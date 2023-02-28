#pragma once
#include "WindNetPredictDetect.h"

WindNetDetect::WindNetDetect() {};
WindNetDetect::~WindNetDetect() {};

class YoloV5Focus : public ncnn::Layer
{
public:
    YoloV5Focus()
    {
        one_blob_only = true;
    }

    virtual int forward(const ncnn::Mat& bottom_blob, ncnn::Mat& top_blob, const ncnn::Option& opt) const
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;
        int channels = bottom_blob.c;

        int outw = w / 2;
        int outh = h / 2;
        int outc = channels * 4;

        top_blob.create(outw, outh, outc, 4u, 1, opt.blob_allocator);
        if (top_blob.empty())
            return -100;

#pragma omp parallel for num_threads(opt.num_threads)
        for (int p = 0; p < outc; p++)
        {
            const float* ptr = bottom_blob.channel(p % channels).row((p / channels) % 2) + ((p / channels) / 2);
            float* outptr = top_blob.channel(p);

            for (int i = 0; i < outh; i++)
            {
                for (int j = 0; j < outw; j++)
                {
                    *outptr = *ptr;

                    outptr += 1;
                    ptr += 2;
                }

                ptr += w;
            }
        }

        return 0;
    }
};
DEFINE_LAYER_CREATOR(YoloV5Focus)

int WindNetDetect::init1(std::string net_bin, std::string net_para) {

    /*
    // init config
    {
        rr::RrConfig config;
        config.ReadConfig(config_path);
        saveVideoPath = config.ReadString("IS_SAVEVIDEO", "saveVideoPath", "");
        video_path = config.ReadString("IMGORVIDEO", "img_video", "");
        img_path = config.ReadString("IMGORVIDEO", "img_path", "");
        net_para = config.ReadString("NET_MODEL", "net_para", "");
        net_bin = config.ReadString("NET_MODEL", "net_bin", "");
        is_save = config.ReadInt("IS_SAVEVIDEO", "is_save", 0);
    }
    */

    ncnn::Option opt;
    //net_p = (void*)(new ncnn::Option());
    opt.lightmode = true;
    opt.num_threads = 4;
    opt.blob_allocator = &g_blob_pool_allocator;
    opt.workspace_allocator = &g_workspace_pool_allocator;
    opt.use_packing_layout = true;
    opt.use_vulkan_compute = true;

    yolov5.opt = opt;

    /*
    ncnn::create_gpu_instance();
    ncnn::VulkanDevice vkdev(1);
    yolov5.opt.use_packing_layout = true;
    yolov5.opt.use_vulkan_compute = true;
    yolov5.opt.use_bf16_storage = true;
    yolov5.set_vulkan_device(&vkdev);
    */


    const char* param_path_ = net_para.c_str();
    const char* bin_path_ = net_bin.c_str();

    yolov5.register_custom_layer("YoloV5Focus", YoloV5Focus_layer_creator);
    // init param
    {
        int ret = yolov5.load_param(param_path_);

        if (ret != 0)
        {
            std::cout << "ret= " << ret << std::endl;
            fprintf(stderr, "YoloV5Ncnn, load_param failed");
            return -301;
        }
    }
    // init bin
    {
        int ret = yolov5.load_model(bin_path_);
        if (ret != 0)
        {
            fprintf(stderr, "YoloV5Ncnn, load_model failed");
            return -301;
        }
    }
    
    return 0;
}



//Object WindNetDetect::get_one_Object(std::vector<Object> object)
//{
//
//    if (object.size() <= 1) {
//        return object[0];
//    }
//    else {
//        Object ObjectMax;
//        float prob = object[0].prob;
//        for (int i = 1; i < object.size(); i++)
//        {
//            const auto obj = object[i];
//            float prob1 = obj.prob;
//            if (prob1 > prob) {
//                ObjectMax = obj;
//                prob = prob1;
//            }
//            else {
//                ObjectMax = object[0];
//            }
//        }
//    }
//}



int WindNetDetect::get_one_label(Object obj) {
    //cv::rectangle(bgr, cv::Point(obj.x, obj.y), cv::Point(obj.w, obj.h), cv::Scalar(0, 255, 0), 3, 8, 0);
    int labels = obj.label;
    return labels;
}

/*
cv::Mat WindNetDetect::drawImg_detect(cv::Mat& img, std::string strtime, int results, Object one_obj) {
    cv::Point2f first_point(10, 200);
    cv::Point2f last_point(700, 460);
    //cv::Point2f first_point(10, 10);
    //cv::Point2f last_point(400, 400);
    cv::Point2f showFistPoint(380, 0);
    cv::Point2f showLastPoint(720, 122);

    if (results == 1)
    {
        //cv::rectangle(img, showFistPoint, showLastPoint, (255, 0, 0), -1);
        cv::rectangle(img, cv::Point(one_obj.x, one_obj.y), cv::Point(one_obj.w, one_obj.h), cv::Scalar(0, 255, 0), 3, 8, 0);
        //putText(img, "show result: Disorderly", cvPoint(390, 25), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
        //putText(img, "show result: excessive", cvPoint(390, 25), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
        //cv::rectangle(img, first_point, last_point, cv::Scalar(255, 0, 0), 3);
    }
    else {
        cv::rectangle(img, cv::Point(one_obj.x, one_obj.y), cv::Point(one_obj.w, one_obj.h), cv::Scalar(0, 255, 0), 3, 8, 0);
        //cv::rectangle(img, showFistPoint, showLastPoint, (0, 255, 0), -1);
        //putText(img, "show result: Normal", cvPoint(390, 25), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
        //cv::rectangle(img, first_point, last_point, cv::Scalar(0, 255, 0), 3);
        //cvPutText(iplimg, "Normal", cvPoint(200, 300), &font, CV_RGB(0, 255, 0));
    }
    strtime = "run    time: " + strtime;
    putText(img, strtime, cvPoint(390, 65), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
    return img;
}
*/

cv::Mat WindNetDetect::drawImg(cv::Mat& img, std::string strtime, int results) {
    cv::Point2f first_point(10, 200);
    cv::Point2f last_point(700, 460);
    //cv::Point2f first_point(10, 10);
    //cv::Point2f last_point(400, 400);
    cv::Point2f showFistPoint(380, 0);
    cv::Point2f showLastPoint(720, 122);

    if (results == 1)
    {
        cv::rectangle(img, showFistPoint, showLastPoint, (255, 0, 0), -1);
        putText(img, "show result: Disorderly", cvPoint(390, 25), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
        //putText(img, "show result: excessive", cvPoint(390, 25), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
        cv::rectangle(img, first_point, last_point, cv::Scalar(255, 0, 0), 3);
    }
    else {
        cv::rectangle(img, showFistPoint, showLastPoint, (0, 255, 0), -1);
        putText(img, "show result: Normal", cvPoint(390, 25), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
        cv::rectangle(img, first_point, last_point, cv::Scalar(0, 255, 0), 3);
        //cvPutText(iplimg, "Normal", cvPoint(200, 300), &font, CV_RGB(0, 255, 0));
    }
    strtime = "run    time: " + strtime;
    putText(img, strtime, cvPoint(390, 65), cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(0, 0, 255), 3, CV_AA);
    return img;
}

/*
void WindNetDetect::get_video_result(std::string videoPath, std::string saveVideoPath, int is_save) {
    cv::VideoCapture video;
    clock_t start, end;
    video.open(videoPath);
    if (!video.isOpened()) {//ÅÐ¶ÏÊÇ·ñµ÷ÓÃ³É¹Š
        std::cout << "¶ÁÈ¡ÊÓÆµÊ§°Ü";
    }
    cv::Mat img;
    //init(net_bin, net_para, configPath);
    if (is_save == 1) {
        video >> img;//»ñÈ¡ÍŒÏñ
        cv::VideoWriter writer;
        double fps = 20.0;//ÉèÖÃÊÓÆµÖ¡ÂÊ
        bool isColor = (img.type() == CV_8UC3);//ÅÐ¶ÏÊÓÆµÊÇ·ñÎª²ÊÉ«
        int coder = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');//Ñ¡Ôñ±àÂëžñÊœ
        writer.open(saveVideoPath, coder, fps, img.size(), isColor);//ŽŽœš±£ŽæÊÓÆµÎÄŒþµÄÊÓÆµÁ÷
        if (!writer.isOpened()) {
            std::cout << "Žò¿ªÊÓÆµÎÄŒþÊ§°Ü£¬ÇëÈ·ÈÏÊÇ·ñÎªºÏ·šÊäÈë." << std::endl;
        }
        //video >> img;//»ñÈ¡ÍŒÏñ
        //video.read(img)

        while (video.read(img)) {
            start = clock();
            //std::vector<Object> objects;
            Object one_obj;
            //detect(img, objects);
            one_obj = get_one_Object(img);
            int lab = get_one_label(one_obj);
            
            //int result = get_one_result(img);
            std::cout << "result = " << lab << std::endl;
            end = clock();
            float rumtime = (float)(end - start) / CLOCKS_PER_SEC;
            std::stringstream buf;
            buf.precision(3);//ž²žÇÄ¬ÈÏŸ«¶È
            buf.setf(std::ios::fixed);//±£ÁôÐ¡ÊýÎ»
            buf << rumtime;
            std::string strtime;
            strtime = buf.str();
            drawImg(img, strtime, lab);
            cv::imshow("video", img);
            cv::waitKey(10);
            //writer.write(img);//°ÑÍŒÏñÐŽÈëÊÓÆµÁ÷
            writer << img;
            cv::waitKey(10);
        }
        //cv::waitKey(0);
        video.release();
        writer.release();
    }
    else {
        while (video.read(img)) {
            start = clock();
            //video >> img;//»ñÈ¡ÍŒÏñ
            //int result = get_one_result(img, bin_path, param_path);
            //std::vector<Object> objects;
            Object one_obj;
            //detect(img, objects);
            one_obj = get_one_Object(img);
            int lab = get_one_label(one_obj);
            //int result = get_one_result(img);
            std::cout << "result = " << lab << std::endl;
            end = clock();
            float rumtime = (float)(end - start) / CLOCKS_PER_SEC;
            //float rumtime = end - start;
            std::cout << "rumtime = " << rumtime << std::endl;
            std::stringstream buf;
            buf.precision(3);//ž²žÇÄ¬ÈÏŸ«¶È
            buf.setf(std::ios::fixed);//±£ÁôÐ¡ÊýÎ»
            buf << rumtime;
            std::string strtime;
            strtime = buf.str();
            //drawImg(img, strtime, lab);
            //drawImg_detect(img, strtime, lab, one_obj);
            cv::imshow("video", img);
            cv::waitKey(10);
        }
        video.release();
    }
}
*/

inline float WindNetDetect::intersection_area(const Object& a, const Object& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

void WindNetDetect::qsort_descent_inplace1(std::vector<Object>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j) qsort_descent_inplace1(faceobjects, left, j);
        }
#pragma omp section
        {
            if (i < right) qsort_descent_inplace1(faceobjects, i, right);
        }
    }
}

void WindNetDetect::qsort_descent_inplace(std::vector<Object>& faceobjects)
{
    if (faceobjects.empty())
        return;
    qsort_descent_inplace1(faceobjects, 0, faceobjects.size() - 1);
}

void WindNetDetect::nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold, bool agnostic)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++)
    {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Object& b = faceobjects[picked[j]];

            if (!agnostic && a.label != b.label)
                continue;

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

inline float WindNetDetect::sigmoid(float x)
{
    return static_cast<float>(1.f / (1.f + exp(-x)));
}

void WindNetDetect::generate_proposals(const ncnn::Mat& anchors, int stride, const ncnn::Mat& in_pad, const ncnn::Mat& feat_blob, float prob_threshold, std::vector<Object>& objects)
{
    const int num_grid = feat_blob.h;

    int num_grid_x;
    int num_grid_y;
    if (in_pad.w > in_pad.h)
    {
        num_grid_x = in_pad.w / stride;
        num_grid_y = num_grid / num_grid_x;
    }
    else
    {
        num_grid_y = in_pad.h / stride;
        num_grid_x = num_grid / num_grid_y;
    }

    const int num_class = feat_blob.w - 5;

    const int num_anchors = anchors.w / 2;

    for (int q = 0; q < num_anchors; q++)
    {
        const float anchor_w = anchors[q * 2];
        const float anchor_h = anchors[q * 2 + 1];

        const ncnn::Mat feat = feat_blob.channel(q);

        for (int i = 0; i < num_grid_y; i++)
        {
            for (int j = 0; j < num_grid_x; j++)
            {
                const float* featptr = feat.row(i * num_grid_x + j);
                float box_confidence = sigmoid(featptr[4]);
                if (box_confidence >= prob_threshold)
                {
                    // find class index with max class score
                    int class_index = 0;
                    float class_score = -FLT_MAX;
                    for (int k = 0; k < num_class; k++)
                    {
                        float score = featptr[5 + k];
                        if (score > class_score)
                        {
                            class_index = k;
                            class_score = score;
                        }
                    }
                    float confidence = box_confidence * sigmoid(class_score);
                    if (confidence >= prob_threshold)
                    {
                        // yolov5/models/yolo.py Detect forward
                        // y = x[i].sigmoid()
                        // y[..., 0:2] = (y[..., 0:2] * 2. - 0.5 + self.grid[i].to(x[i].device)) * self.stride[i]  # xy
                        // y[..., 2:4] = (y[..., 2:4] * 2) ** 2 * self.anchor_grid[i]  # wh

                        float dx = sigmoid(featptr[0]);
                        float dy = sigmoid(featptr[1]);
                        float dw = sigmoid(featptr[2]);
                        float dh = sigmoid(featptr[3]);

                        float pb_cx = (dx * 2.f - 0.5f + j) * stride;
                        float pb_cy = (dy * 2.f - 0.5f + i) * stride;

                        float pb_w = pow(dw * 2.f, 2) * anchor_w;
                        float pb_h = pow(dh * 2.f, 2) * anchor_h;

                        float x0 = pb_cx - pb_w * 0.5f;
                        float y0 = pb_cy - pb_h * 0.5f;
                        float x1 = pb_cx + pb_w * 0.5f;
                        float y1 = pb_cy + pb_h * 0.5f;

                        Object obj;
                        obj.rect.x = x0;
                        obj.rect.y = y0;
                        obj.rect.width = x1 - x0;
                        obj.rect.height = y1 - y0;
                        obj.label = class_index;
                        obj.prob = confidence;

                        objects.push_back(obj);
                    }
                }
            }
        }
    }
}


//visdrone Ð¡Ä¿±êdetect

//int WindNetDetect::detect(cv::Mat img, std::vector<Object>& objects)
//{
//    double start_time = ncnn::get_current_time();
//    const int target_size = 640;
//
//    // letterbox pad to multiple of 32
//    const int width = img.cols;//1280
//    const int height = img.rows;//720
//    int w = img.cols;//1280
//    int h = img.rows;//720
//    float scale = 1.f;
//    //float scale = 1.0;
//    if (w > h)
//    {
//        scale = (float)target_size / w;//640/1280
//        w = target_size;//640
//        h = h * scale;//360
//        //h = target_size;
//    }
//    else
//    {
//        scale = (float)target_size / h;
//        h = target_size;
//        w = w * scale;
//    }
//    cv::resize(img, img, cv::Size(w, h));
//    ncnn::Mat in = ncnn::Mat::from_pixels(img.data, ncnn::Mat::PIXEL_BGR2RGB, w, h);
//
//    // pad to target_size rectangle
//    // yolov5/utils/datasets.py letterbox
//    int wpad = (w + 31) / 32 * 32 - w;
//    int hpad = (h + 31) / 32 * 32 - h;
//    ncnn::Mat in_pad;
//    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 114.f);
//    // yolov5
//    //std::vector<Object> objects;
//    {
//        const float prob_threshold = 0.4f;
//        const float nms_threshold = 0.51f;
//
//        const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
//        in_pad.substract_mean_normalize(0, norm_vals);
//
//        ncnn::Extractor ex = yolov5.create_extractor();
//        //ex.set_vulkan_compute(use_gpu);
//
//        ex.input("images", in_pad);
//        std::vector<Object> proposals;
//
//        // anchor setting from yolov5/models/yolov5s.yaml
//
//
//        //stride 4
//        {
//            ncnn::Mat out;
//            ex.extract("output", out);
//            ncnn::Mat anchors(6);
//            anchors[0] = 5.f;
//            anchors[1] = 6.f;
//            anchors[2] = 8.f;
//            anchors[3] = 14.f;
//            anchors[4] = 15.f;
//            anchors[5] = 11.f;
//
//            std::vector<Object> objects8;
//            generate_proposals(anchors, 4, in_pad, out, prob_threshold, objects8);
//            proposals.insert(proposals.end(), objects8.begin(), objects8.end());
//        }
//
//        // stride 8
//        {
//            ncnn::Mat out;
//            ex.extract("1534", out);
//            ncnn::Mat anchors(6);
//            anchors[0] = 10.f;
//            anchors[1] = 13.f;
//            anchors[2] = 16.f;
//            anchors[3] = 30.f;
//            anchors[4] = 33.f;
//            anchors[5] = 23.f;
//
//            std::vector<Object> objects8;
//            generate_proposals(anchors, 8, in_pad, out, prob_threshold, objects8);
//            proposals.insert(proposals.end(), objects8.begin(), objects8.end());
//        }
//
//        // stride 16
//        {
//            ncnn::Mat out;
//            ex.extract("1548", out);
//
//            ncnn::Mat anchors(6);
//            anchors[0] = 30.f;
//            anchors[1] = 61.f;
//            anchors[2] = 62.f;
//            anchors[3] = 45.f;
//            anchors[4] = 59.f;
//            anchors[5] = 119.f;
//
//            std::vector<Object> objects16;
//            generate_proposals(anchors, 16, in_pad, out, prob_threshold, objects16);
//
//            proposals.insert(proposals.end(), objects16.begin(), objects16.end());
//        }
//        // stride 32
//        {
//            ncnn::Mat out;
//            ex.extract("1562", out);
//            ncnn::Mat anchors(6);
//            anchors[0] = 116.f;
//            anchors[1] = 90.f;
//            anchors[2] = 156.f;
//            anchors[3] = 198.f;
//            anchors[4] = 373.f;
//
//            anchors[5] = 326.f;
//
//            std::vector<Object> objects32;
//            generate_proposals(anchors, 32, in_pad, out, prob_threshold, objects32);
//
//            proposals.insert(proposals.end(), objects32.begin(), objects32.end());
//        }
//
//        // sort all proposals by score from highest to lowest
//        qsort_descent_inplace(proposals);
//        // apply nms with nms_threshold
//        std::vector<int> picked;
//        nms_sorted_bboxes(proposals, picked, nms_threshold);
//
//        int count = picked.size();
//        objects.resize(count);
//        for (int i = 0; i < count; i++)
//        {
//            objects[i] = proposals[picked[i]];
//
//            // adjust offset to original unpadded
//            float x0 = (objects[i].rect.x - (wpad / 2)) / scale;
//            float y0 = (objects[i].rect.y - (hpad / 2)) / scale;
//            float x1 = (objects[i].rect.x + objects[i].rect.width - (wpad / 2)) / scale;
//            float y1 = (objects[i].rect.y + objects[i].rect.height - (hpad / 2)) / scale;
//
//            // clip
//            x0 = maxs(mins(x0, (float)(width - 1)), 0.f);
//            y0 = maxs(mins(y0, (float)(height - 1)), 0.f);
//            x1 = maxs(mins(x1, (float)(width - 1)), 0.f);
//            y1 = maxs(mins(y1, (float)(height - 1)), 0.f);
//            objects[i].rect.x = x0;
//            objects[i].rect.y = y0;
//            objects[i].rect.width = x1 - x0;
//            objects[i].rect.height = y1 - y0;
//        }
//    }
//    return 0;
//}


int WindNetDetect::detect(cv::Mat img, std::vector<Object>& objects)
{
    double start_time = ncnn::get_current_time();
    const int target_size = 640;

    // letterbox pad to multiple of 32
    const int width = img.cols;//1280
    const int height = img.rows;//720
    int w = img.cols;//1280
    int h = img.rows;//720
    //int w = 2560;//1280
    //int h = 1440;//720

    //std::cout << "xxxx" << std::endl;

    float scale = 1.f;
    //float scale = 1.0;
    if (w > h)
    {
        scale = (float)target_size / w;//640/1280
        w = target_size;//640
        h = h * scale;//360
        //h = target_size;
        //std::cout << "scale = " << scale << std::endl;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
    }

    cv::resize(img, img, cv::Size(w, h));
    ncnn::Mat in = ncnn::Mat::from_pixels(img.data, ncnn::Mat::PIXEL_BGR2RGB, w, h);

    // pad to target_size rectangle
    // yolov5/utils/datasets.py letterbox
    int wpad = (w + 31) / 32 * 32 - w;
    int hpad = (h + 31) / 32 * 32 - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 114.f);
    // yolov5
    //std::vector<Object> objects;
    {
        const float prob_threshold = 0.4f;
        const float nms_threshold = 0.51f;

        const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
        in_pad.substract_mean_normalize(0, norm_vals);

        ncnn::Extractor ex = yolov5.create_extractor();
        //ex.set_vulkan_compute(use_gpu);

        ex.input("images", in_pad);
        std::vector<Object> proposals;

        // anchor setting from yolov5/models/yolov5s.yaml


        //stride 4
        {
            ncnn::Mat out;
            ex.extract("output", out);
            ncnn::Mat anchors(6);
            anchors[0] = 10.f;
            anchors[1] = 13.f;
            anchors[2] = 16.f;
            anchors[3] = 30.f;
            anchors[4] = 33.f;
            anchors[5] = 23.f;

            std::vector<Object> objects8;
            generate_proposals(anchors, 8, in_pad, out, prob_threshold, objects8);
            proposals.insert(proposals.end(), objects8.begin(), objects8.end());
        }

        // stride 8
        {
            ncnn::Mat out;
            ex.extract("715", out);
            ncnn::Mat anchors(6);
            anchors[0] = 30.f;
            anchors[1] = 61.f;
            anchors[2] = 62.f;
            anchors[3] = 45.f;
            anchors[4] = 59.f;
            anchors[5] = 119.f;

            std::vector<Object> objects8;
            generate_proposals(anchors, 16, in_pad, out, prob_threshold, objects8);
            proposals.insert(proposals.end(), objects8.begin(), objects8.end());
        }

        // stride 16
        {
            ncnn::Mat out;
            ex.extract("729", out);

            ncnn::Mat anchors(6);
            anchors[0] = 116.f;
            anchors[1] = 90.f;
            anchors[2] = 156.f;
            anchors[3] = 198.f;
            anchors[4] = 373.f;
            anchors[5] = 326.f;

            std::vector<Object> objects16;
            generate_proposals(anchors, 32, in_pad, out, prob_threshold, objects16);

            proposals.insert(proposals.end(), objects16.begin(), objects16.end());
        }

        // sort all proposals by score from highest to lowest
        qsort_descent_inplace(proposals);
        // apply nms with nms_threshold
        std::vector<int> picked;
        nms_sorted_bboxes(proposals, picked, nms_threshold);

        int count = picked.size();
        objects.resize(count);
        for (int i = 0; i < count; i++)
        {
            objects[i] = proposals[picked[i]];

            // adjust offset to original unpadded
            float x0 = (objects[i].rect.x - (wpad / 2)) / scale;
            float y0 = (objects[i].rect.y - (hpad / 2)) / scale;
            float x1 = (objects[i].rect.x + objects[i].rect.width - (wpad / 2)) / scale;
            float y1 = (objects[i].rect.y + objects[i].rect.height - (hpad / 2)) / scale;

            // clip
            x0 = maxs(mins(x0, (float)(width - 1)), 0.f);
            y0 = maxs(mins(y0, (float)(height - 1)), 0.f);
            x1 = maxs(mins(x1, (float)(width - 1)), 0.f);
            y1 = maxs(mins(y1, (float)(height - 1)), 0.f);
            objects[i].rect.x = x0;
            objects[i].rect.y = y0;
            objects[i].rect.width = x1 - x0;
            objects[i].rect.height = y1 - y0;
        }
    }
    return 0;
}



void WindNetDetect::draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects)
{
    
    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < objects.size(); i++)
    {
        const Object& obj = objects[i];

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
            obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::rectangle(image, obj.rect, cv::Scalar(255, 0, 0), 2);

        char text[256];
        //sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
            cv::Scalar(255, 255, 255), -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    //cv::imwrite("./result20.jpg", image);
    cv::imshow("image", image);
    cv::waitKey(10);
}


/**********************************************************
//mat转base64，并将数据以json格式发送
***********************************************************/
std::string WindNetDetect::Mat2Base64(const cv::Mat &image, std::string imgType){
    std::vector<uchar> buf;
    cv::imencode(imgType, image, buf);
    //uchar *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
    std::string img_data = base64_encode(buf.data(), buf.size(), false);
    return img_data;
}

void WindNetDetect::send_json(cv::Mat img, std::string label, std::string level){
    std::string out = Mat2Base64(img,".jpg");
    //std::cout << out << std::endl;

#if 1
    ImgInfo imgInfo(out, label, level);
    static auto client = httplib::Client("127.0.0.1", 18080);
    //static auto client = httplib::Client("192.168.8.11", 18080);
    auto result = client.Post("/uploadAlgorithmResult", imgInfo.to_json(), "application/json");
    
    //auto client = httplib::Client("192.168.1.105", 8080);
    //std::cout << imgInfo.to_json() << std::endl;
    //client.set_connection_timeout(1);
#else
    auto client = httplib::Client("127.0.0.1", 8080);
    //auto client = httplib::Client("192.168.8.11", 8080);
    auto result = client.Post("/update", imgInfo.to_json(), "application/json");
#endif

    if (result != nullptr && result->status == 200) {
        std::cout << result->body << std::endl;
    }
}
/*
void WindNetDetect::send_json_people(cv::Mat img, std::vector<std::string> label, std::string level){
    std::string out = Mat2Base64(img,".jpg");
    //std::cout << out << std::endl;

#if 1
    ImgInfo imgInfo(out, label, level);
    static auto client = httplib::Client("127.0.0.1", 18080);
    auto result = client.Post("/uploadAlgorithmResult", imgInfo.to_json(), "application/json");
    
    //auto client = httplib::Client("192.168.1.105", 8080);
    //std::cout << imgInfo.to_json() << std::endl;
    //client.set_connection_timeout(1);
#else
    auto client = httplib::Client("127.0.0.1", 8080);
    auto result = client.Post("/update", imgInfo.to_json(), "application/json");
#endif
    if (result != nullptr && result->status == 200) {
        std::cout << result->body << std::endl;
    }
}
*/



