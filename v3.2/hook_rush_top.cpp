//#include "opencv2/opencv.hpp"

//#include <QDir>
#include <fstream>
#include <unistd.h>
//#include "auto_entercs.h"

#include "HCNetSDK.h"
#include "PlayM4.h"
#include "LinuxPlayM4.h"
#include "WindNetPredictDetect.h"
#include <X11/Xlib.h>
#include <queue>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <json.h>

using namespace cv;
using namespace std;
using namespace std::chrono; // calc fps
LONG lUserID;

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
    //球机信息
    char IP[] = "192.168.8.32";   //����������������ͷ��ip
    char UName[] = "admin";                 //����������������ͷ���û���
    char PSW[] = "a123456789";           //����������������ͷ������
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(1000, true);
    NET_DVR_SetLogToFile(3, "./sdkLog");
    NET_DVR_DEVICEINFO_V30 struDeviceInfo = { 0 };
    NET_DVR_SetRecvTimeOut(5000);
    lUserID = NET_DVR_Login_V30(IP, 8000, UName, PSW, &struDeviceInfo);
    //NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

    Mat input, blob;
    VideoCapture capture;
    capture.open("rtsp://admin:a123456789@192.168.8.32:554/h264/ch1/main/av_stream/1");
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
			if (frame_buffer.size() < 10000000){
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


int  HexToDecMa(int wHex)//ʮ������תʮ����
{
    return (wHex / 4096) * 1000 + ((wHex % 4096) / 256) * 100 + ((wHex % 256) / 16) * 10 + (wHex % 16);
}

int DEC2HEX_doc(int x)//ʮ����תʮ������
{
    return (x / 1000) * 4096 + ((x % 1000) / 100) * 256 + ((x % 100) / 10) * 16 + x % 10;
}

void contral_cam(int wPanPos1, int wTiltPos1, int wZoomPos1, DWORD dwtmp, NET_DVR_PTZPOS m_ptzPosCurrent){

    bool a = NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_PTZPOS, 0, &m_ptzPosCurrent, sizeof(NET_DVR_PTZPOS), &dwtmp);
    //转换相机信息到十进制
    int m_iPara1 = HexToDecMa(m_ptzPosCurrent.wPanPos);
    int m_iPara2 = HexToDecMa(m_ptzPosCurrent.wTiltPos);
    int m_iPara3 = HexToDecMa(m_ptzPosCurrent.wZoomPos);
    std::vector<int> TempPosture(3);
    TempPosture[0] = m_iPara1 / 10 ;    //P水平方向
    TempPosture[1] = m_iPara2 / 10 ;    //T仰角
    TempPosture[2] = m_iPara3 / 10 ;    //Z焦距
    
	std::cout << "TempPosture[2] = " << TempPosture[2] << std::endl;
    
    //如果大于1将焦距调到1
    //将俯仰角调到90°
    m_ptzPosCurrent.wPanPos = DEC2HEX_doc(wPanPos1*10);
    m_ptzPosCurrent.wTiltPos = DEC2HEX_doc(wTiltPos1*10);
    m_ptzPosCurrent.wZoomPos = DEC2HEX_doc(wZoomPos1*10);
    
    bool b = NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_PTZPOS, 0, &m_ptzPosCurrent, sizeof(NET_DVR_PTZPOS));
    sleep(2);
}

int display_wTiltPos(DWORD dwtmp, NET_DVR_PTZPOS m_ptzPosCurrent){
    bool a = NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_PTZPOS, 0, &m_ptzPosCurrent, sizeof(NET_DVR_PTZPOS), &dwtmp);
    //转换相机信息到十进制
    int m_iPara1 = HexToDecMa(m_ptzPosCurrent.wPanPos);
    int m_iPara2 = HexToDecMa(m_ptzPosCurrent.wTiltPos);
    int m_iPara3 = HexToDecMa(m_ptzPosCurrent.wZoomPos);
    std::vector<int> TempPosture(3);
    TempPosture[0] = m_iPara1 / 10 ;    //P水平方向
    TempPosture[1] = m_iPara2 / 10 ;    //T仰角
    TempPosture[2] = m_iPara3 / 10 ;    //Z焦距
	std::cout << "TempPosture[2] = " << TempPosture[2] << std::endl;
    return TempPosture[1];
}

//void getframe1(std::vector<cv::Mat>& frame_buffer) {
//void *getframe1(void *arg) {
void frame_read(vector<Mat>& frame_buffer) {
	//std::vector<cv::Mat> frame_buffer = *(std::vector<cv::Mat>*)arg;
	std::cout << "this is read" << std::endl;
    clock_t start, end;

    //***************************************************************
    //***************************************************************
    //读取配置文件
    Json::Reader reader;
	Json::Value root;
    std::ifstream in("./output.json", std::ios::binary);
    if (!in.is_open())
	{
		std::cout << "Error opening file\n";
	}
    std::string net_bins;
    std::string net_paras;

    int smoothing_factor = 4;
    std::string warns;
    std::string early_warns;
    std::string abnormals;
    std::string labels;

    //const char* warns_;
    //const char* early_warns_;
    //const char* abnormals_;
    //const char* labels_;

    
    int is_save;
    int interval;
    int Pans;
    int Tilts;
    int Zooms;
    int is_read_rtsp_sign;

    int is_change_threadsholds;
    int serious_punch_tops;
    int moderate_punch_tops;
    int mild_punch_tops;

    int is_contral_cams;            //是否跟踪目标。
    int contral_cam_threadsholds;

    float img_ratio = 0.5;
    int send_interval = 450;

    

    if (reader.parse(in, root))
	{
		net_bins = root["net_bin"].asString();
		net_paras = root["net_param"].asString();
		Pans = root["Pan"].asInt();
        Tilts = root["Tilt"].asInt();
        Zooms = root["Zoom"].asInt();
		is_save = root["is_save_img"]["is_save"].asInt();
		interval = root["is_save_img"]["interval"].asInt();

        smoothing_factor = root["level"]["smoothing_factor"].asInt();
        warns = root["level"]["warn"].asString();
        early_warns = root["level"]["early_warn"].asString();
        abnormals = root["level"]["abnormal"].asString();
        labels = root["label"].asString();

        img_ratio = root["img_ratio"].asFloat();
        send_interval = root["send_interval"].asInt();

        is_contral_cams = root["contral_cam"]["is_contral_cam"].asInt();
        contral_cam_threadsholds = root["contral_cam"]["contral_cam_threadshold"].asInt();
        //labels_ = labels.c_str();

        is_change_threadsholds = root["threshold_punch_top"]["is_change_threadshold"].asInt();
        if(is_change_threadsholds == 1){
            serious_punch_tops = root["threshold_punch_top"]["serious_punch_top"].asInt();
		    moderate_punch_tops = root["threshold_punch_top"]["moderate_punch_top"].asInt();
		    mild_punch_tops = root["threshold_punch_top"]["mild_punch_top"].asInt();
        }
        else{
            serious_punch_tops = 2000;
            moderate_punch_tops = 1600;
            mild_punch_tops = 1200;
        }
        //const char* net_params = net_para.c_str();
        //const char* net_bins = net_bin.c_str();

		std::cout << "net_bins =  " << net_bins << std::endl;
		std::cout << "net_params =  " << net_paras << std::endl;
		std::cout << "Pans =  " << Pans << std::endl;
        std::cout << "Tilts =  " << Tilts << std::endl;
		std::cout << "Zooms =  " << Zooms << std::endl;
		std::cout << "interval =  " << interval << std::endl;
        std::cout << "is_contral_cams =  " << is_contral_cams << std::endl;
        //std::cout << "is_contral_cams =  " << is_contral_cams << std::endl;
        std::cout << "contral_cam_threadsholds =  " << contral_cam_threadsholds << std::endl;
        std::cout << "serious_punch_tops =  " << serious_punch_tops << std::endl;
        std::cout << "moderate_punch_tops =  " << moderate_punch_tops << std::endl;
        std::cout << "mild_punch_tops =  " << mild_punch_tops << std::endl;
        std::cout << "img_ratio =  " << img_ratio << std::endl;
        std::cout << "send_interval =  " << send_interval << std::endl;
    }
    else
	{
        net_bins = "./models/hook_run103.bin";
        net_paras = "./models/hook_run103.param";
        Pans = 55;
        Tilts = 47;
        Zooms = 1;
        is_save = 0;
        interval = 1500;

        warns = "warn";
        early_warns = "early_warn";
        abnormals = "abnormal";
        labels = "label";
        is_contral_cams = 0;
        contral_cam_threadsholds = 60;
        serious_punch_tops = 2000;
        moderate_punch_tops = 1600;
        mild_punch_tops = 1200;
        

		std::cout << "parse error\n" << std::endl;
	}
	in.close();
    //*****************************************************************
    //*****************************************************************
    
    void* p_algorithm;
    p_algorithm = (void*)(new WindNetDetect());

    //std::string net_bins = "./models/hook_run103.bin";
    //std::string net_paras = "./models/hook_run103.param";

    int init_res = ((WindNetDetect*)p_algorithm)->init1(net_bins, net_paras);
    WindNetDetect* tmp = (WindNetDetect*)(p_algorithm);

	cv::Mat frame1;
    cv::Mat send_frame;

	int image_name = 1;
	
	int x_head = 1280;
	int y_head = 720;
	int center_point_x;
	int center_point_y;

    //调焦参数定义
    NET_DVR_PTZPOS m_ptzPosCurrent;
    DWORD dwtmp;
    //std::vector<int> TempPosture(3);

    //输出hook参数
    std::string level;
    int x_hook;
    int y_hook;
    int w_hook;
    int h_hook;

    //控制球机焦距和俯仰角
    contral_cam(Pans, Tilts, Zooms, dwtmp, m_ptzPosCurrent);
    int contral_num = 0;
    
    //cv::namedWindow("people");
    //cv::namedWindow("hook");
    int k = 0;
    int smoothing_warn_it = 0;
    int smoothing_early_warn_it = 0;
    int smoothing_abnormal_it = 0;

    while(1){
        sleep(1);
        if (!frame_buffer.empty()) {
            frame1 = frame_buffer.back();
			//pthread_cond_signal(&cond);
            if(!frame1.empty()){
                frame_buffer.pop_back();

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
                    
                        
                    std::cout << "obj_max.rect.width + obj_max.rect.height = " << obj_max.rect.width + obj_max.rect.height << std::endl;
                    
                    center_point_x = obj_max.rect.x + obj_max.rect.width/2;
                    center_point_y = obj_max.rect.y + obj_max.rect.height/2;
                    //*************************************************************
                    //输出吊钩冲顶参数
                    x_hook = obj_max.rect.x;
                    y_hook = obj_max.rect.y;
                    w_hook = obj_max.rect.width;
                    h_hook = obj_max.rect.height;

                    //**********************************************
                    // std::vector<std::string> label_hook;
                    // label_hook.push_back(std::to_string(x_hook));
                    // label_hook.push_back(std::to_string(y_hook));
                    // label_hook.push_back(std::to_string(w_hook));
                    // label_hook.push_back(std::to_string(h_hook));
                    //**********************************************

                    if(obj_max.rect.width + obj_max.rect.height > serious_punch_tops ){
                        smoothing_warn_it++;
                        smoothing_early_warn_it = 0;
                        smoothing_abnormal_it = 0;
                        if (smoothing_warn_it >= smoothing_factor){
                            cv::resize(frame1, send_frame, cv::Size(1280 * img_ratio, 720 * img_ratio));
                            std::cout << "level = " << warns << std::endl;
                            if(smoothing_warn_it % send_interval == smoothing_factor){
                                tmp->send_json(send_frame, labels, warns);
                            }
                        }
                        //level = "warn";
                    }
                    else if(obj_max.rect.width + obj_max.rect.height > moderate_punch_tops){
                        smoothing_warn_it = 0;
                        smoothing_early_warn_it++;
                        smoothing_abnormal_it = 0;
                        if (smoothing_early_warn_it >= smoothing_factor){
                            cv::resize(frame1, send_frame, cv::Size(1280 * img_ratio, 720 * img_ratio));
                            std::cout << "level = " << early_warns << std::endl;
                            //level = "early_warn";
                            if(smoothing_early_warn_it % send_interval == smoothing_factor){
                                tmp->send_json(send_frame, labels, early_warns);
                            }
                        }
                    }
                    else if(obj_max.rect.width + obj_max.rect.height > mild_punch_tops){
                        smoothing_warn_it = 0;
                        smoothing_early_warn_it = 0;
                        smoothing_abnormal_it++;
                        if (smoothing_abnormal_it >= smoothing_factor){
                            cv::resize(frame1, send_frame, cv::Size(1280 * img_ratio, 720 * img_ratio));
                            
                            std::cout << "level = " << abnormals << std::endl;
                            std::cout << "smoothing_abnormal_it = " << smoothing_abnormal_it << std::endl;
                            //level = "abnormal";
                            if(smoothing_abnormal_it % send_interval == smoothing_factor){
                                tmp->send_json(send_frame, labels, abnormals);
                            }
                        }
                    }

                    else{
                        //std::cout << "cccccc" << std::endl;
                        
                        smoothing_warn_it = 0;
                        smoothing_early_warn_it = 0;
                        smoothing_abnormal_it = 0;
                        level = "normal";
                        //tmp->send_json(frame1, label_hook, level);
                    }

                    if(is_contral_cams == 1){
                        if((std::abs(1280 - center_point_x) > 800 || std::abs(720 - center_point_y) > 480)){
                            NET_DVR_POINT_FRAME posdata;
                            int std_cols = 2560;
                            int std_rows = 1440;
                            //posdata.xTop = (int)(obj_max.rect.x * 255 / std_cols);	// 坐标归一化到(255, 255)
                            posdata.xTop = (int)(center_point_x * 255 / std_cols);	// 坐标归一化到(255, 255)
                            posdata.xBottom = posdata.xTop;

                            posdata.yTop = (int)(center_point_y * 255 / std_rows);
                            posdata.yBottom = posdata.yTop;
                            posdata.bCounter = 3;   // 没用
                            if (!NET_DVR_PTZSelZoomIn_EX(0, 1, &posdata)){
                                printf("3D定位失败, %d\n", NET_DVR_GetLastError());
                                //return -1;
                            }
                            sleep(3);
                            if (display_wTiltPos(dwtmp, m_ptzPosCurrent) > contral_cam_threadsholds){
                                contral_cam(Pans, Tilts, Zooms, dwtmp, m_ptzPosCurrent);
                            }
                            //rect(960, 540, 640, 360);
                        }
                    }
                    
                    //*****************************************************************************  
                    //sleep(100);
                }

                if(k%interval == 2 && is_save == 1){
                    cv::imwrite("./data/images/" + std::to_string(image_name) + ".jpg", frame1);
                    image_name++;
                }
                k++;

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
            }
            else{
                contral_num++;
                if (contral_num == 5000){
                    contral_cam(Pans, Tilts, Zooms, dwtmp, m_ptzPosCurrent);
                }
            }

            if (frame_buffer.size() > 10) { // 隔帧抽取一半删除
				auto iter = frame_buffer.erase(frame_buffer.begin(), frame_buffer.end() - 5);
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

