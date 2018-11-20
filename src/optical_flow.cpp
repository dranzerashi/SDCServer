
#include "optical_flow.hpp"
#include "eventsApi.hpp"
#include <time.h>

using namespace std;
using namespace cv;
using namespace cv::cuda;

inline bool isFlowCorrect(Point2f u)
{
    return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

static Vec3b computeColor(float fx, float fy)
{
    static bool first = true;

    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow
    //  than between yellow and green)
    const int RY = 15;
    const int YG = 6;
    const int GC = 4;
    const int CB = 11;
    const int BM = 13;
    const int MR = 6;
    const int NCOLS = RY + YG + GC + CB + BM + MR;
    static Vec3i colorWheel[NCOLS];

    if (first)
    {
        int k = 0;

        for (int i = 0; i < RY; ++i, ++k)
            colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

        for (int i = 0; i < YG; ++i, ++k)
            colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

        for (int i = 0; i < GC; ++i, ++k)
            colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

        for (int i = 0; i < CB; ++i, ++k)
            colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

        for (int i = 0; i < BM; ++i, ++k)
            colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

        for (int i = 0; i < MR; ++i, ++k)
            colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);

        first = false;
    }

    const float rad = sqrt(fx * fx + fy * fy);
    const float a = atan2(-fy, -fx) / (float) CV_PI;

    const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
    const int k0 = static_cast<int>(fk);
    const int k1 = (k0 + 1) % NCOLS;
    const float f = fk - k0;

    Vec3b pix;

    for (int b = 0; b < 3; b++)
    {
        const float col0 = colorWheel[k0][b] / 255.0f;
        const float col1 = colorWheel[k1][b] / 255.0f;

        float col = (1 - f) * col0 + f * col1;

        if (rad <= 1)
            col = 1 - rad * (1 - col); // increase saturation with radius
        else
            col *= .75; // out of range

        pix[2 - b] = static_cast<uchar>(255.0 * col);
    }

    return pix;
}

static void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion = -1)
{
    dst.create(flowx.size(), CV_8UC3);
    dst.setTo(Scalar::all(0));

    // determine motion range:
    float maxrad = maxmotion;

    if (maxmotion <= 0)
    {
        maxrad = 1;
        for (int y = 0; y < flowx.rows; ++y)
        {
            for (int x = 0; x < flowx.cols; ++x)
            {
                Point2f u(flowx(y, x), flowy(y, x));

                if (!isFlowCorrect(u))
                    continue;

                maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
            }
        }
    }

    for (int y = 0; y < flowx.rows; ++y)
    {
        for (int x = 0; x < flowx.cols; ++x)
        {
            Point2f u(flowx(y, x), flowy(y, x));

            if (isFlowCorrect(u)){
                dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
                //cout<<dst.at<Vec3b>(y,x)<<" ";
            }
        }
        //cout<<endl;
    }
}

static void showFlow(const char* name, const GpuMat& d_flow)
{
    GpuMat planes[2];
    cuda::split(d_flow, planes);

    Mat flowx(planes[0]);
    Mat flowy(planes[1]);

    Mat out;
    drawOpticalFlow(flowx, flowy, out, 10);

    imshow(name, out);
    waitKey();
}


static GpuMat preprocessImage(GpuMat img, CamConfig &cfg){
    cv:Rect ROI  = cfg.getROICoords();
    try{   
            img = img(ROI);

        }catch(...){
            cout <<"Warning :: GIVEN CO-ORDINATES ARE OUT OF RANGE "<<endl;
            stringstream customString;
            customString<<"(0,0);("<<img.size().width<<","<<img.size().height<<");";
            cfg.setROI(customString.str());
            Rect customCordinates = cfg.getROICoords();
            img = img(customCordinates);
       }

    GpuMat frame_HSV, frame_threshold, frame_res, out_img;
    Mat frame_HSV_inrange, frame_threshold_inrange, frame_bit, toSplitFrame;
    cv::cuda::cvtColor(img, frame_HSV, CV_BGR2HSV);

    cv::Vec3b min_color_range = cfg.getMinColorRange();
    cv::Vec3b max_color_range = cfg.getMaxColorRange();

    frame_HSV.download(frame_HSV_inrange);
    inRange(frame_HSV_inrange, min_color_range, max_color_range, frame_threshold_inrange);
    frame_threshold.upload(frame_threshold_inrange);
    
    Mat kernel = getStructuringElement(MORPH_RECT, cv::Size(8,8));//, cv::Point(4,4));
    morphologyEx(frame_threshold_inrange, frame_threshold_inrange, MORPH_OPEN, kernel);
    morphologyEx(frame_threshold_inrange, frame_threshold_inrange, MORPH_CLOSE, kernel);

    cv::cuda::cvtColor(img, frame_res, CV_BGR2YUV);
    vector<Mat> channels(3);
    frame_res.download(toSplitFrame);
    split(toSplitFrame, channels);
    cv::bitwise_and(channels[0], channels[0], frame_bit, frame_threshold_inrange);

    // cv::imshow("Preprocessed image cpp",frame_bit);
    // waitKey(25);
    out_img.upload(frame_bit);
    return out_img;

}

/* *
 * To be removed 
 * */
void processFrameWithOpticalFlow(Mat frame0, Mat frame1)
{

    if (frame0.empty())
    {
        return;
    }
    if (frame1.empty())
    {
        return;
    }

    if (frame1.size() != frame0.size())
    {
        cerr << "Images should be of equal sizes" << endl;
        return;
    }

    GpuMat td_frame0(frame0);
    GpuMat td_frame1(frame1);
    GpuMat d_frame0;
    GpuMat d_frame1;
    
    //d_frame0 = preprocessImage(td_frame0, cfg);
    //d_frame1 = preprocessImage(td_frame1, cfg);

    GpuMat d_flow(frame0.size(), CV_32FC2);

    //Ptr<cuda::BroxOpticalFlow> brox = cuda::BroxOpticalFlow::create(0.197f, 50.0f, 0.8f, 10, 77, 10);
    //Ptr<cuda::DensePyrLKOpticalFlow> lk = cuda::DensePyrLKOpticalFlow::create(Size(7, 7));
    Ptr<cuda::FarnebackOpticalFlow> farn = cuda::FarnebackOpticalFlow::create(1,0.5,false,5,8,5,1.1,0);
    //Ptr<cuda::OpticalFlowDual_TVL1> tvl1 = cuda::OpticalFlowDual_TVL1::create();

    // {
    //     GpuMat d_frame0f;
    //     GpuMat d_frame1f;

    //     d_frame0.convertTo(d_frame0f, CV_32F, 1.0 / 255.0);
    //     d_frame1.convertTo(d_frame1f, CV_32F, 1.0 / 255.0);

    //     const int64 start = getTickCount();

    //     brox->calc(d_frame0f, d_frame1f, d_flow);

    //     const double timeSec = (getTickCount() - start) / getTickFrequency();
    //     cout << "Brox : " << timeSec << " sec" << endl;

    //     showFlow("Brox", d_flow);
    // }

    // {
    //     const int64 start = getTickCount();

    //     lk->calc(d_frame0, d_frame1, d_flow);

    //     const double timeSec = (getTickCount() - start) / getTickFrequency();
    //     cout << "LK : " << timeSec << " sec" << endl;

    //     showFlow("LK", d_flow);
    // }

    {
        const int64 start = getTickCount();

        farn->calc(d_frame0, d_frame1, d_flow);

        const double timeSec = (getTickCount() - start) / getTickFrequency();
        cout << "Farn : " << timeSec << " sec" << endl;

        showFlow("Farn", d_flow);
    }

    // {
    //     const int64 start = getTickCount();

    //     tvl1->calc(d_frame0, d_frame1, d_flow);

    //     const double timeSec = (getTickCount() - start) / getTickFrequency();
    //     cout << "TVL1 : " << timeSec << " sec" << endl;

    //     showFlow("TVL1", d_flow);
    // }

    //imshow("Frame 0", frame0);
    Mat frame_show;
    d_frame0.download(frame_show);
    imshow("Frame 1", frame_show);
    waitKey();

    
}


static void processFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, bool &is_smoke){
    int valid_angle_count = 0;
    float valid_angle_percentage = 0.0;
    Mat magnitude, angle;
    cv::cartToPolar(flowx, flowy, magnitude, angle, true);
    Mat mag;
    normalize(magnitude, mag, 0, 255, NORM_MINMAX);
    dst.create(flowx.size(), CV_8UC3);
    dst.setTo(Scalar::all(0));
    for (int y = 0; y < angle.rows; ++y)
    {
        for (int x = 0; x < angle.cols; ++x)
        {
            Point2f u(flowx(y, x), flowy(y, x));

            if (isFlowCorrect(u)){
            uchar a = static_cast<uchar>(angle.at<float>(y, x)/2);
            uchar m = static_cast<uchar>(mag.at<float>(y, x));
            Vec3b pixel;
            pixel[0] = static_cast<uchar>(a);
            pixel[1] = static_cast<uchar>(255);
            pixel[2] = static_cast<uchar>(m);
            dst.at<Vec3b>(y, x) = pixel;

            valid_angle_count = (angle.at<float>(y, x) >= 30 && angle.at<float>(y, x) <= 150 ) ? valid_angle_count + 1: valid_angle_count;
            }
        }

    }
    valid_angle_percentage = float(valid_angle_count)/(angle.rows * angle.cols) * 100;
    // cout<<"valid_angle_percentage: "<<valid_angle_percentage<<endl;
    is_smoke = ( valid_angle_percentage >= MOVEMENT_INTENSITY_THRESHOLD) ? true : false;
    
    cv::cvtColor(dst, dst, CV_HSV2BGR);

}

static bool showProcessFlow(const char* name, const GpuMat& d_flow)
{
    bool is_smoke = false;
    GpuMat planes[2];
    cuda::split(d_flow, planes);

    Mat flowx(planes[0]);
    Mat flowy(planes[1]);

    Mat out;
    processFlow(flowx, flowy, out, is_smoke);
    // if(is_smoke)
    //     cout<<"Smoke Detected!!"<<endl;
    // imshow(name, out);
    // waitKey(25);
    return is_smoke;
}

void OpticalFlowProcess::start_timer(){
    if(detection_start_time == 0)
        detection_start_time = getTickCount();
    
    buffer_time = detection_start_time + snooze_timeout;//cfg.getThreshold()/float(2);
}

int64 OpticalFlowProcess::timer_duration(){
    if(detection_start_time > 0)
        return (getTickCount() - detection_start_time)/getTickFrequency();
    else
        return 0;
}

void OpticalFlowProcess::stop_timer(){
    detection_start_time = 0;
    buffer_time = 0;
}

void OpticalFlowProcess::process(Mat frame){
    if (frame.empty())
    {
        return;
    }
    if (prev_frame.empty())
    {
        GpuMat temp_frame;
        temp_frame.upload(frame);
        prev_frame = preprocessImage(temp_frame, cfg);
        return;
    }
    else{
        GpuMat temp_frame;
        temp_frame.upload(frame);
        curr_frame = preprocessImage(temp_frame, cfg);
    }
    if (prev_frame.size() != curr_frame.size())
    {
        cerr << "Images should be of equal sizes" << endl;
        return;
    }

    cv::cuda::GpuMat d_flow(curr_frame.size(), CV_32FC2);
    
    std::string source_abs_path = cfg.getSource();

    {
        const int64 start = getTickCount();

        farn->calc(prev_frame, curr_frame, d_flow);

        const double timeSec = (getTickCount() - start) / getTickFrequency();
        // cout << "Farn : " << timeSec << " sec " <<cfg.getKey()<< endl;
        bool is_smoke_detected = showProcessFlow(source_abs_path.c_str(), d_flow);

        if( is_smoke_detected ){
            if (detection_start_time == 0)
                start_timer();
            else
                buffer_time = detection_start_time + cfg.getThreshold()/float(2);
            
            int64 current_duration = timer_duration();
            if( current_duration > cfg.getThreshold() )
            {
                //make POST call
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
            }
        } else {
            int64 current_duration = timer_duration();
            if( detection_start_time > 0 && current_duration > buffer_time )
                stop_timer();
        }

        

        //showFlow("Farn", d_flow);
    }
    // {
    //     Mat frame_show;
    //     curr_frame.download(frame_show);
    //     imshow("Frame 1", frame_show);
    //     waitKey();
    // }

    return;
}