#ifndef _MAPINFO_HPP_
#define _MAPINFO_HPP_

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/opencv.hpp>
#include "Message.hpp"

class MapInfo
{
private:
    cv::Mat warpmatrix;     // (3, 3, CV_64FC1)  透视变换矩阵

    cv::Mat aiMap               = ~ cv::Mat::zeros(808, 448, CV_8UC3);  // 白图(初始化变量)
    cv::Mat B3_B7               = ~ cv::Mat::zeros(20,  100, CV_8UC3);
    cv::Mat B1_B4_B6_B9         = ~ cv::Mat::zeros(100, 20,  CV_8UC3);
    cv::Mat B2_B8               = ~ cv::Mat::zeros(80,  20,  CV_8UC3);
    cv::Mat B5                  = cv::Mat::zeros(26,  26,  CV_8UC3);    // 黑图

    cv::Mat aiMapShow           = ~ cv::Mat::zeros(808, 448, CV_8UC3);  // 要展示的画布
    cv::Mat aiMapShow2          = ~ cv::Mat::zeros(808, 448, CV_8UC3);  // 要展示的画布

public:
   MapInfo(cv::Mat& warpmatrix);
   ~MapInfo();
   void showTransformImg(const cv::Mat& img);
   void showMapInfo(std::vector<car>& result);
   void drawCarPosition(std::vector<car>& result);
   
   void showMapInfo2(const CarInfoSend& sendInfo);
   void drawCarPosition2(const CarInfoSend& sendInfo);
};

// 构造函数
// 给透视变换矩阵赋值 && 画一个虚拟地图
MapInfo::MapInfo(cv::Mat& warpmatrix) {
    this->warpmatrix = warpmatrix;                                  // 初始化透视变换矩阵
    // aiMap = ~aiMap;
    cv::Mat roiB3 = aiMap(cv::Rect(0,       150,      100, 20));    // cv::Rect(左上角的点 和 宽高)
    cv::Mat roiB7 = aiMap(cv::Rect(448-100, 808-170,  100, 20));
    cv::absdiff(roiB3, B3_B7, roiB3);                               // 相减，取绝对值
    cv::absdiff(roiB7, B3_B7, roiB7);

    cv::Mat roiB1 = aiMap(cv::Rect(448-120, 0,        20,  100));
    cv::Mat roiB4 = aiMap(cv::Rect(100,     808-100,  20,  100));
    cv::Mat roiB6 = aiMap(cv::Rect(93,      354,      20,  100));   // 93  -> 93.5
    cv::Mat roiB9 = aiMap(cv::Rect(335,     354,      20,  100));   // 335 -> 334.5
    cv::absdiff(roiB1, B1_B4_B6_B9, roiB1);
    cv::absdiff(roiB4, B1_B4_B6_B9, roiB4);
    cv::absdiff(roiB6, B1_B4_B6_B9, roiB6);
    cv::absdiff(roiB9, B1_B4_B6_B9, roiB9);

    cv::Mat roiB2 = aiMap(cv::Rect(214,     150,      20,  80));
    cv::Mat roiB8 = aiMap(cv::Rect(214,     578,      20,  80));
    cv::absdiff(roiB2, B2_B8, roiB2);
    cv::absdiff(roiB8, B2_B8, roiB8);

    cv::Mat roiB5 = aiMap(cv::Rect(224-13,  404-13,   26,  26));
    // 在B5上画个旋转矩形
    cv::RotatedRect rRect = cv::RotatedRect(cv::Point2f(13,13), cv::Size2f(18, 18), 45);
    cv::Point2f vertices2f[4];                                      // 定义4个点的数组
    rRect.points(vertices2f);                                       // 将四个点存储到 `vertices` 数组中
    cv::Point   vertices[4];
    for (int i = 0; i < 4; ++i) {
        vertices[i] = vertices2f[i];
    }
    cv::fillConvexPoly(B5, vertices, 4, cv::Scalar(255, 255, 255));
    cv::absdiff(roiB5, B5, roiB5);

    aiMap.copyTo(aiMapShow);
    aiMap.copyTo(aiMapShow2);
}

MapInfo::~MapInfo() {
    
}

// 显示 透视变换 后的图像
void MapInfo::showTransformImg(const cv::Mat& img) {
    static cv::Mat result;
    cv::warpPerspective(img, result, this->warpmatrix, cv::Size(448, 808),cv::INTER_LINEAR); //result.size(),
    cv::imshow("result", result);
}

void MapInfo::showMapInfo(std::vector<car>& result) {
    drawCarPosition(result);
    cv::imshow("map", aiMapShow);
    aiMap.copyTo(aiMapShow);
}

void MapInfo::drawCarPosition(std::vector<car>& result) {
    static const cv::Scalar colors[] = {{0,0,0}, {255,0,0}, {0,0,255}, {0,0,0}};
    for (car& p: result) {
        cv::circle(aiMapShow, cv::Point(p.carPosition.x, p.carPosition.y),              25, cv::Scalar(0, 255, 0)); // 矫正前的
        
        cv::circle(aiMapShow, cv::Point(p.carPositionFixed.x, p.carPositionFixed.y),    25, colors[p.color+1], -1); // 矫正后的
        cv::putText(aiMapShow, std::to_string((int)p.num), cv::Point(p.carPositionFixed.x - 6, p.carPositionFixed.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
}

void MapInfo::showMapInfo2(const CarInfoSend& sendInfo) {
    drawCarPosition2(sendInfo);
    cv::imshow("map2", aiMapShow2);
    aiMap.copyTo(aiMapShow2);
}

void MapInfo::drawCarPosition2(const CarInfoSend& sendInfo) {
    static const cv::Scalar colors[] = {{0,0,0}, {255,0,0}, {0,0,255}, {0,0,0}};
    if (sendInfo.blue1 != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.blue1, 25, colors[1], -1);
        cv::putText(aiMapShow2, std::to_string(1), cv::Point(sendInfo.blue1.x - 6, sendInfo.blue1.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.blue2 != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.blue2, 25, colors[1], -1);
        cv::putText(aiMapShow2, std::to_string(2), cv::Point(sendInfo.blue2.x - 6, sendInfo.blue2.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.red1  != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.red1,  25, colors[2], -1);
        cv::putText(aiMapShow2, std::to_string(1), cv::Point(sendInfo.red1.x - 6, sendInfo.red1.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.red2 != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.red2,  25, colors[2], -1);
        cv::putText(aiMapShow2, std::to_string(2), cv::Point(sendInfo.red2.x - 6, sendInfo.red2.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.blue1_2 != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.blue1_2, 25, colors[1], -1);
        cv::putText(aiMapShow2, std::to_string(1), cv::Point(sendInfo.blue1_2.x - 6, sendInfo.blue1_2.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.blue2_2 != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.blue2_2, 25, colors[1], -1);
        cv::putText(aiMapShow2, std::to_string(2), cv::Point(sendInfo.blue2_2.x - 6, sendInfo.blue2_2.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.red1_2  != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.red1_2,  25, colors[2], -1);
        cv::putText(aiMapShow2, std::to_string(1), cv::Point(sendInfo.red1_2.x - 6, sendInfo.red1_2.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    if (sendInfo.red2_2 != cv::Point2f(-1, -1)) {
        cv::circle(aiMapShow2, sendInfo.red2_2,  25, colors[2], -1);
        cv::putText(aiMapShow2, std::to_string(2), cv::Point(sendInfo.red2_2.x - 6, sendInfo.red2_2.y + 6), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
}

#endif  // _MAPINFO_HPP_