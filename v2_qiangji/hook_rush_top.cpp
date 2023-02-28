//#include "opencv2/opencv.hpp"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <thread>
#include <queue>
#include <chrono>
#include <functional>
#include "WindNetPredictDetect.h"
#include <X11/Xlib.h>

using namespace cv;
using namespace std;
using namespace std::chrono; // calc fps

double fps()
{
    static double fps = 0.0;
    static int frameCount = 0;
    static auto lastTime = system_clock::now();
    static auto curTime = system_clock::now();

    curTime = system_clock::now();

    auto duration = duration_cast<microseconds>(curTime - lastTime);
    double duration_s = double(duration.count()) * microseconds::period::num / microseconds::period::den;

    if (duration_s > 2)//2秒之后开始统计FPS
    {
        fps = frameCount / duration_s;
        frameCount = 0;
        lastTime = curTime;
    }
    ++frameCount;
    return fps;
}

void frame_write(vector<Mat>& frame_buffer) {
    cout << "this is write" << endl;
    Mat input, blob;
    VideoCapture capture;
    capture.open("rtsp://admin:a123456789@192.168.8.33:554/h264/ch1/main/av_stream/1");
    if (capture.isOpened())
    {
        cout << "Capture is opened" << endl;
        for (;;)
        {
            capture >> input;
            if (input.empty()){
				std::cout << "image is empty " << std::endl;
                continue;
				}
			if (frame_buffer.size() < 100){
				//std::cout << "frame_buffer get input" << std::endl;
                frame_buffer.push_back(input);
			}
			else {
                cout << "thread ==============> after read stop, frame_buffer.size() > 100 , write stop";
                return;
            }
        }
    }
    else {
        cout << "open camera failed" << endl;
    }
}



//void getframe1(std::vector<cv::Mat>& frame_buffer) {
void frame_read(vector<Mat>& frame_buffer) {
    cout << "this is read" << endl;
    Mat frame;

    clock_t start, end, end1, end2;
    void* p_algorithm;
    p_algorithm = (void*)(new WindNetDetect());


    std::string net_bins = "./models/hook_run103.bin";
    std::string net_paras = "./models/hook_run103.param";


    int init_res = ((WindNetDetect*)p_algorithm)->init1(net_bins, net_paras);
    WindNetDetect* tmp = (WindNetDetect*)(p_algorithm);

	cv::Mat frame1;

	int image_name = 1;
	
	int x_head = 1280;
	int y_head = 720;
	int center_point_x;
	int center_point_y;

    //输出hook参数
    std::string level;
    int x_hook;
    int y_hook;
    int w_hook;
    int h_hook;

    int k = 0;

    while(1){
        if (!g_frameList.empty()) {
            frame1 = g_frameList.back();
			//pthread_cond_signal(&cond);
            if(!frame1.empty()){
                g_frameList.pop_back();

                std::vector<Object> objects;
                start = clock();
                tmp->detect(frame1, objects);
                
                //tmp->draw_objects(frame1, objects);
                //cv::imwrite("./data/a39/" + std::to_string(image_name) + ".jpg", frame1);

                if (objects.size() > 0){
                    contral_num = 0;
                    int max_i = 0;
                    int max_prob = 0;
                    for (size_t i = 0; i < objects.size(); i++)
                    {
                        const Object& obj = objects[i];
                        if (obj.prob > max_prob){
                            max_prob = obj.prob;
                            max_i = i;
                        }
                    }
                    
                    const Object& obj_max = objects[max_i];

                    fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj_max.label, obj_max.prob,
                        obj_max.rect.x, obj_max.rect.y, obj_max.rect.width, obj_max.rect.height);

                    cv::rectangle(frame1, obj_max.rect, cv::Scalar(255, 0, 0), 3);

                    //detection label
                    char text[256];
                    //sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);
                    sprintf(text, "%s", class_names[obj_max.label]);

                    int baseLine = 0;
                    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

                    int x = obj_max.rect.x;
                    int y = obj_max.rect.y - label_size.height - baseLine;
                    if (y < 0)
                        y = 0;
                    if (x + label_size.width > frame1.cols)
                        x = frame1.cols - label_size.width;

                    cv::rectangle(frame1, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                        cv::Scalar(255, 255, 255), -1);

                    cv::putText(frame1, text, cv::Point(x, y + label_size.height),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
                        
                    //**********************************************
                    //cv::Mat reframe;
                    //cv::resize(frame1, reframe, cv::Size(1028, 720));
                    //cv::imshow("hook", reframe);
                    //cv::waitKey(10);
                    //**********************************************
                    if(k%1500 == 5){
                        cv::imwrite("./data/images/" + std::to_string(image_name) + ".jpg", frame1);
                        image_name++;
                    }
                    k++;
                        
                    std::cout << "obj_max.rect.width + obj_max.rect.height = " << obj_max.rect.width + obj_max.rect.height << std::endl;
                    
                    //*************************************************************
                    //输出吊钩冲顶参数
                    x_hook = obj_max.rect.x;
                    y_hook = obj_max.rect.y;
                    w_hook = obj_max.rect.width;
                    h_hook = obj_max.rect.height;
                    
                    std::vector<std::string> label_hook;
                    label_hook.push_back(std::to_string(x_hook));
                    label_hook.push_back(std::to_string(y_hook));
                    label_hook.push_back(std::to_string(w_hook));
                    label_hook.push_back(std::to_string(h_hook));

                    if(obj_max.rect.width + obj_max.rect.height > 2000 ){
                        std::cout << "level = " << 1 << std::endl;
                        level = "1:serious_hook_punch_top";
                        tmp->send_json(frame1, label_hook, level);
                    }
                    else if(obj_max.rect.width + obj_max.rect.height > 1600){
                        std::cout << "level = " << 2 << std::endl;
                        level = "2:moderate_hook_punch_top";
                        tmp->send_json(frame1, label_hook, level);

                    }
                    else if(obj_max.rect.width + obj_max.rect.height > 1200){
                        std::cout << "level = " << 3 << std::endl;
                        level = "3:mild_hook_punch_top";
                        tmp->send_json(frame1, label_hook, level);
                    }

                    else{
                        level = "normal";
                        tmp->send_json(frame1, label_hook, level);
                    }
                    
                    //*****************************************************************************  
                    
                    //********************************************
                    //cv::imshow("people", img_roi);
                    //cv::waitKey(10);
                    //**********************************************  

                    //cv::imwrite("./data/images/" + std::to_string(image_name) + ".jpg", img_roi);
                    //image_name++;
                    //tmp->draw_objects(frame1, objects);
                    objects.clear();
                    end = clock();
                    float rumtime = (float)(end - start) / CLOCKS_PER_SEC;
                    std::stringstream buf;
                    buf.precision(3);	//瑕嗙洊榛樿?ょ簿搴?
                    buf.setf(std::ios::fixed);	//淇濈暀灏忔暟浣?
                    buf << rumtime;
                    std::string strtime;
                    strtime = buf.str();
                    std::cout << "strtime22222 = " << strtime << std::endl;
                    //sleep(100);
                }
            }
            if (g_frameList.size() > 10) { // 隔帧抽取一半删除
				auto iter = g_frameList.erase(g_frameList.begin(), g_frameList.end() - 5);
                //auto iter = frame_buffer.begin();
                //for (int inde = 0; inde < frame_buffer.size() / 2; inde++)
                //    frame_buffer.erase(iter++);
            }
        }
    }
	 //std::cout << "thread ==============> read stop" << std::endl;

}

int main(int argc, char** argv)
{
    vector<Mat> frame_buffer;
    XInitThreads();
    std::thread tw(frame_write, ref(frame_buffer)); // pass by value
    std::thread tr(frame_read, ref(frame_buffer)); // pass by value

    tw.join();
    tr.join();

    return 0;
}

