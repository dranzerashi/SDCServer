#include "flame_detection.hpp"
#include <time.h>

using namespace std;
using namespace cv;
using namespace cv::cuda;

void FlameDetector::detect(cv::Mat input_frame)
{
    // cout<<"BlackWaterDetector::detect"<<endl;
    cv::Mat frame, original_frame;
    input_frame.copyTo(frame);
    input_frame.copyTo(original_frame);

    if (frame.empty() || original_frame.empty())
    {
        cout << "Video Ended Successfully" << endl;
    }

    frame_HSI = frame;
    frame.convertTo(frame, CV_32F);
    // Upload to gpu
    gpu_frame.upload(frame);
    gpu_frame.convertTo(gpu_frame,CV_32F, 1.f/ 255);

    cv::cuda::GpuMat image(gpu_frame);

    gpu_frame = image(regionOfInterest);

    cv::cuda::cvtColor(gpu_frame, next_frame, CV_BGR2GRAY);

    next_frame.download(greyMat);

    frame_HSI(regionOfInterest).copyTo(croppedImg);
    static cv::Mat prevFrame = greyMat;

    r1 = Motion_Detection( prevFrame , greyMat);
    r2 = get_HSI(croppedImg);
    r3 = Flame_Area_Detection(greyMat);
     /* if you want to check which frame is givng what response then uncomment below code::*/

    // if(r1 && r2 && r3){
    //     cv::rectangle(frame_HSI ,regionOfInterest, cv::Scalar(0, 0, 255),THICKNESS);
    // }else{
    //         cout<<"###############################################"<<endl;
    //         cout<<"Is Frame showing movement :::::::::::::::: "<<r1<<endl;
    //         cout<<"Is Flame content valid ::::::::::::::::::: "<<r2<<endl;
    //         cout<<"Is Flame size valid :::::::::::::::::::::: "<<r3<<endl;
    //         cout <<"###############################################"<<endl;
    //     cv::rectangle(frame_HSI ,regionOfInterest, cv::Scalar(0, 255, 0), THICKNESS);
    // }

    //prev_frame = next_frame;
    
    next_frame.download(prevFrame);
    
    if( r1 && r2 && r3 ){
            if (detection_start_time == 0)
                start_timer();
            // else
            //     buffer_time = detection_start_time/getTickFrequency() + snooze_timeout;
            
            int64 current_duration = timer_duration();
            // cout<< getTickCount()/getTickFrequency() - latest_post_timestamp<<"--:--"<<buffer_time<<endl;
            if( current_duration > cfg.getThreshold() && getTickCount()/getTickFrequency() - latest_post_timestamp > buffer_time)
            {
                cout<<"--------------------------------------------------------------making POST call-----------------------------------------------"<<endl;
                time_t curr_time;
                tm * curr_tm;
                time(&curr_time);
                curr_tm = gmtime(&curr_time);
                char time_str[51];
                Mat snapshot = frame(cfg.getROICoords());
                strftime(time_str, 50, "%FT%T.000Z", curr_tm);
                eventns::Event event(cfg.getCamID(), time_str, snapshot, "image/jpg","SMOKE_DETECTION");
                // eventns::Event event(cfg.getCamID(), "2018-09-26T01:17:56.787Z", frame, "image/jpg","SMOKE_DETECTION");

                postEvent(event);
                stop_timer();

                latest_post_timestamp = getTickCount()/getTickFrequency();
                buffer_time = snooze_timeout;
            }
        } else {
            int64 current_duration = timer_duration();
            if( detection_start_time > 0 && current_duration > buffer_time ){
                stop_timer();
            }
        }
    //waitKey(25);
}

void FlameDetector::start_timer()
{
    if (detection_start_time == 0)
        detection_start_time = getTickCount();

    // buffer_time = detection_start_time/getTickFrequency() + snooze_timeout;//cfg.getThreshold()/float(2);
}

int64 FlameDetector::timer_duration()
{
    if (detection_start_time > 0)
        return float(getTickCount() - detection_start_time) / float(getTickFrequency());
    else
        return 0;
}

void FlameDetector::stop_timer()
{
    detection_start_time = 0;
    // buffer_time = 0;
}

void FlameDetector::check_and_notify(cv::Mat snapshot)
{
    // cout<<"check_and_notify"<<endl;
    if (detection_start_time == 0)
        start_timer();
    // else
    //     buffer_time = detection_start_time/getTickFrequency() + snooze_timeout;

    int64 current_duration = timer_duration();
    // cout<< getTickCount()/getTickFrequency() - latest_post_timestamp<<" --:-- "<<buffer_time<<endl;
    // cout<< current_duration<<" --:-- "<<cfg.getThreshold()<<endl;

    if (current_duration > cfg.getThreshold() && getTickCount() / getTickFrequency() - latest_post_timestamp > buffer_time)
    {
        cout << "--------------------------------------------------------------making POST call-----------------------------------------------" << endl;
        time_t curr_time;
        tm *curr_tm;
        time(&curr_time);
        curr_tm = gmtime(&curr_time);
        char time_str[51];
        // Mat snapshot = frame(cfg.getROICoords());
        strftime(time_str, 50, "%FT%T.000Z", curr_tm);
        eventns::Event event(cfg.getCamID(), time_str, snapshot, "image/jpg", "SMOKE_DETECTION");
        // eventns::Event event(cfg.getCamID(), "2018-09-26T01:17:56.787Z", frame, "image/jpg","SMOKE_DETECTION");

        postEvent(event);
        stop_timer();

        latest_post_timestamp = getTickCount() / getTickFrequency();
        buffer_time = snooze_timeout;
    }
}

bool FlameDetector::Flame_Area_Detection(const Mat &src){
    // Converting grayscale to black n white
    cv::Mat bw = src > .5;
    cv::blur( bw, bw, cv::Size(3,3) );
   
    int count_white = 0;
    for(int i =0 ; i< bw.rows; i++){
        for(int j =0 ; j< bw.cols; j++){
            if(bw.at<float>(i,j) != 0){
              count_white++;
            }
        }
    }

    float per_b_pixel =((float)count_white/(float)(bw.rows*bw.cols))* 100;
    if(per_b_pixel < CONST_AREA_THRESHOLD)
        return false;
    return true;
}

bool FlameDetector::get_HSI(const Mat &pic){

    cv::Mat hsi(pic.rows, pic.cols, pic.type());
    int H = pic.rows;
    int W = pic.cols;
    bool grab = true;
    for (int j=0;j<H;j++)
    for (int i=0;i<W;i++) {

        double B =(double) pic.at<cv::Vec3b>(j,i)[0];
        double G =(double) pic.at<cv::Vec3b>(j,i)[1];
        double R =(double) pic.at<cv::Vec3b>(j,i)[2];
        double intensity = 0;
        double hue = 0;
        double saturation = 0;
    
        intensity = (double) (R + G + B) / (3.0);
        double tmp = min(R, min(G, B));

        saturation = 1 - 3*(tmp/(R + G + B));
        if(saturation < 0.00001){
            saturation = 0;
        }else if(saturation > 0.99999){
                saturation = 1;
        }

        double rb = R - B;
		double rg = R - G;
		double denom = sqrt(( rg * rg) + (rb*(G - B)));
		if( saturation != 0 and denom!=0){
            hue = 0.5 * (rg - rb) /sqrt(( rg * rg) + (rb*(G - B)));
			hue = acos(hue);
			if( B <= G){
                    hue = hue;
            }
			else{
                    hue = 360 -hue;
            }
        }		
		else{
                    hue = 180;
        }
        saturation = saturation * 100;
        hsi.at<cv::Vec3b>(j, i)[2] = hue;
        hsi.at<cv::Vec3b>(j, i)[1] = saturation;
        hsi.at<cv::Vec3b>(j, i)[0] = intensity;
        }

    int count = 0;
	for (int j=0;j< H;j++)
       for (int i=0;i<W;i++){

        double I =(double) hsi.at<cv::Vec3b>(j,i)[0];
        double S =(double) hsi.at<cv::Vec3b>(j,i)[1];
        double H =(double) hsi.at<cv::Vec3b>(j,i)[2];
		//Tune values of H,S,I as per demand 
		if (!((H<=120) and (S<=50) and (I>=180))){
                hsi.at<cv::Vec3b>(j, i)[2] = 0;
                hsi.at<cv::Vec3b>(j, i)[1] = 0;
                hsi.at<cv::Vec3b>(j, i)[0] = 0;
				count+=1;
        }
    }

	int size = (H * W);
	float flame_per = 100 - (count*100)/size;

	if (flame_per < CONST_FLAME_THRESHOLD){
              return false;
    }

    //cout<< flame_per<<endl;
    //cv::imshow("HSI Format",hsi);
    return true;
}


bool FlameDetector::Motion_Detection(const Mat &prev ,const  Mat &next) {
    Mat flow;
    cv::calcOpticalFlowFarneback(prev,next,flow, 0.5,3,15,3,5,1.2,0);
    cv::Mat xy[2]; //X,Y
    cv::split(flow, xy);

    //calculate angle and magnitude
    cv::Mat magnitude, angle;
    cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);

    double mag_max;
    cv::minMaxLoc(magnitude, 0, &mag_max);
    Mat mag;
    
    normalize(magnitude, mag, 0, 255, NORM_MINMAX);
    cv::Mat _hsv[3], hsv;
    _hsv[0] = angle ; //((angle*180)/3.14)/2;
    _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
    _hsv[1] = 255;
    _hsv[2] = mag;
    cv::merge(_hsv, 3, hsv);

    //convert to BGR and show
    cv::Mat bgr;//CV_32FC3 matrix
    cuda::GpuMat hsv_cuda, bgr_cuda;
    hsv_cuda.upload(hsv);
    cuda::cvtColor(hsv_cuda, bgr_cuda, cv::COLOR_HSV2BGR);
    bgr_cuda.download(bgr);

    int count = 0;
    for(int i =0 ; i<bgr.size[0]; i++ ){
        for(int j =0 ; j<bgr.size[1]; j++ ){
            if(bgr.at<float>(i,j) != 0){
                    //cout<< bgr.at<Vec3b>(i,j)[0];
                    count++;
            }
        }
    }   
     int thresh_hold = (bgr.size[0] * bgr.size[1])/3;
     if(count >= thresh_hold){
         //cv::imshow("optical flow", bgr);
         return (true) ;
     }
    //cv::imshow("optical flow", bgr);
    return false;
}




