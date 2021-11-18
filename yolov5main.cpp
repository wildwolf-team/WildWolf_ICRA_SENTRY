#include "yolov5.hpp"
#include "TRTModule.hpp"
#include "warpmatrixImg.hpp"
#include "map_info.hpp"
#include "message.hpp"

#define MAPINFO_OFF

int main(int argc, char **argv) {
    std::string engine_name = "/home/wsl/wolf_workspace/tensorrtx_v5.0/v5.0/build/engine/m43best_25epochs.engine";
    std::string img_dir     = "/home/wsl/下载/哨岗2亮度稍暗.avi";

    cv::VideoCapture cap(img_dir);  // cap捕捉图片流

    start(engine_name);             // yolov5.hpp

    TRTModule model("/home/wsl/wolf_workspace/tensorrtx_v5.0/v5.0/opt4/model-opt-4.onnx");

    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
	// cap.set(cv::CAP_PROP_FRAME_HEIGHT, 800);

    // 检测是否有相机
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    cv::Mat img;
    cap.read(img);
    cv::Mat warpmatrix = getTransformMask(img);             // ++ 获得透视变换矩阵      car_position
    MapInfo mapInfo;                                            // ++ 初始化模拟地图        map_info

    while (true) {
        auto start = std::chrono::system_clock::now();

        cap.read(img);
        auto res = yolov5_v5_Rtx_start(img);                // 中科院数据集 yolov5模型 推理
        auto detections = model(img);                       // 上交 四点模型推理

#ifndef MAPINFO_OFF
        showTransformImg(warpmatrix, img);                  // ++ 显示透视变换后的图片   car_position
        mapInfo.showMapInfo();                              // ++ 显示化模拟地图        map_info
        
        // 绘制 矩形(rectangle) 和 类编号(class_id)
        for (size_t j = 0; j < res.size(); j++)  {          // res.size() 该图检测到多少个class
            if( res[j].class_id == 0) {
                cv::Rect r = get_rect(img, res[j].bbox);
                cv::rectangle(img, r, cv::Scalar(0x27, 0xC1, 0x36), 2);
                cv::putText(img, std::to_string((int)res[j].class_id), cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
            }
            mapInfo.drawCarPosition(res[j], warpmatrix, img);       // ++ 在模拟地图上画车车     map_info
        }

        // 在原图上画装甲板opt4
        const cv::Scalar colors[3] = {{255, 0, 0}, {0, 0, 255}, {0, 255, 0}};
        for (const auto &b : detections) {
            cv::line(img, b.pts[0], b.pts[1], colors[2], 2);
            cv::line(img, b.pts[1], b.pts[2], colors[2], 2);
            cv::line(img, b.pts[2], b.pts[3], colors[2], 2);
            cv::line(img, b.pts[3], b.pts[0], colors[2], 2);
            cv::putText(img, std::to_string(b.tag_id), b.pts[0], cv::FONT_HERSHEY_SIMPLEX, 1, colors[b.color_id]);
        }
#endif  // MAPINFO_OFF

        classify(res, detections, img, warpmatrix);
        carColorProcess();
        showCarInfo();

#ifndef MAPINFO_OFF
        cv::imshow("yolov5", img);
#endif  // MAPINFO_OFF

        if (cv::waitKey(1) == 'q') {
            destroy();                                      // yolov5.hpp
            break;
        }

        auto end = std::chrono::system_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    }
}