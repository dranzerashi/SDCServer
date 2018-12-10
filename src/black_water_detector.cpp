#include "black_water_detector.hpp"
#include <time.h>

using namespace std;
using namespace cv;
using namespace cv::cuda;

void BlackWaterDetector::detect(cv::Mat input_frame)
{
    // cout<<"BlackWaterDetector::detect"<<endl;
    cv::Mat frame, original_frame;
    input_frame.copyTo(frame);
    input_frame.copyTo(original_frame);

    if (frame.empty() || original_frame.empty())
    {
        cout << "Video Ended Successfully" << endl;
    }

    frame.convertTo(frame, CV_32F);
    // Upload to gpu
    g.upload(frame);

    // convert to normalized float
    g.convertTo(g, CV_32F, 1.f / 255);
    cv::cuda::GpuMat image(g);

    //Croping the image containing region of interest [top_left_X,top_left_Y, Width, Height]
    // cv::Rect regionOfInterest(X_CORDINATE, Y_CORDINATE, WIDTH, HEIGHT);

    cv::cuda::GpuMat croppedImage = image(regionOfInterest);
    Mat cropped_frame;
    croppedImage.download(cropped_frame);

    vector<Mat> channels(3);
    split(cropped_frame, channels);

    // mean() gives the percentage mean of pixels of particular channel , to make it in 0-255 range, we multiply it by MAX_PIXEL_VAL
    float red_channel_mean = mean(channels[0])[0];
    float green_channel_mean = mean(channels[1])[0];
    float blue_channel_mean = mean(channels[2])[0];
    float rgb_channel_mean = (red_channel_mean + green_channel_mean + blue_channel_mean) * MAX_PIXEL_VAL;

    dominant_pixel = (rgb_channel_mean) / float(RGB_CHANNELS);

    // cv::rectangle(original_frame, regionOfInterest, cv::Scalar(255, 0, 0), THICKNESS);

    if (dominant_pixel > THRESHOLD)
    {
        // cout<<"Dominant Pixel"<<endl;
        int64 current_duration = timer_duration();
        if (detection_start_time > 0 && current_duration > buffer_time)
        {
            stop_timer();
        }
        // cv::rectangle(original_frame, regionOfInterest, cv::Scalar(0, 0, 255), THICKNESS);
    }
    else
    {
        // cout<<"Not Dominant Pixel"<<dominant_pixel<<" - "<<THRESHOLD<<endl;
        cv::rectangle(original_frame, regionOfInterest, cv::Scalar(0, 0, 255), THICKNESS);
        check_and_notify(original_frame);
    }

    // cv::imshow("Black Water Detection", original_frame);
    // waitKey(25);
}

void BlackWaterDetector::start_timer()
{
    if (detection_start_time == 0)
        detection_start_time = getTickCount();

    // buffer_time = detection_start_time/getTickFrequency() + snooze_timeout;//cfg.getThreshold()/float(2);
}

int64 BlackWaterDetector::timer_duration()
{
    if (detection_start_time > 0)
        return float(getTickCount() - detection_start_time) / float(getTickFrequency());
    else
        return 0;
}

void BlackWaterDetector::stop_timer()
{
    detection_start_time = 0;
    // buffer_time = 0;
}

void BlackWaterDetector::check_and_notify(cv::Mat snapshot)
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