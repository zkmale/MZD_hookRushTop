//
// Created by xx on 2022/11/5.
//

#ifndef HTTP_DEMO_IMGINFO_H
#define HTTP_DEMO_IMGINFO_H

#include <utility>

#include "cJSON.h"
#include "base64.h"


#define RTSP_URL "rtsp://admin:a123456789@192.168.8.31:554/h264/ch1/main/av_stream/1"
#define AlgorithmType "RushTop"
#define AlgorithmType_people "PeopleDetection"

class ImgInfo {
private:
    std::string img;
    std::vector<std::string> label;
    //std::string resp;
    std::string level;
public:
    ImgInfo(std::string img, std::vector<std::string> label, 
            std::string level) : img(std::move(img)), label(std::move(label)),
                                 level(std::move(level)) {}

    std::string to_json() {
        auto *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "image", img.c_str());
        cJSON_AddStringToObject(root, "level", level.c_str());
        cJSON_AddStringToObject(root, "rtsp", RTSP_URL);
        cJSON_AddStringToObject(root, "type", AlgorithmType);
        cJSON *label_array = cJSON_CreateArray();

        for (auto &i: label) {
            cJSON_AddItemToArray(label_array, cJSON_CreateString(i.c_str()));
        }
        cJSON_AddItemToObject(root, "label", label_array);
        char *out = cJSON_Print(root);
        std::string res = out;
        free(out);
        cJSON_Delete(root);
        return res;
    }

    std::string to_json_people() {
        auto *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "image", img.c_str());
        cJSON_AddStringToObject(root, "level", level.c_str());
        cJSON_AddStringToObject(root, "rtsp", RTSP_URL);
        cJSON_AddStringToObject(root, "type", AlgorithmType_people);
        cJSON *label_array = cJSON_CreateArray();

        for (auto &i: label) {
            cJSON_AddItemToArray(label_array, cJSON_CreateString(i.c_str()));
        }
        cJSON_AddItemToObject(root, "label", label_array);
        char *out = cJSON_Print(root);
        std::string res = out;
        free(out);
        cJSON_Delete(root);
        return res;
    }

};

#endif //HTTP_DEMO_IMGINFO_H
