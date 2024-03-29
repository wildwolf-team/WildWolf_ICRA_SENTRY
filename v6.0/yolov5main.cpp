#include "yolov5.hpp"

// 标4点位置切换
// 0 点四边玻璃罩子
// 1 点中间各障碍物左下点
#define CHECK_4_OPT 0

// 哨岗相机离地高度 (mm)
#define WATCH_DOG_H 1818.0
// 车半身高度 (mm)
#define CAR_HALF_H 250.0
// 相机偏移量 X (cm)
#define OFFSET_X 15
// 相机偏移量 Y (cm)
#define OFFSET_Y 15

// 数据矫正函数模式
// 0  蓝方[主哨岗] (D1)
// 1  蓝方[副哨岗] (D3)
// 2  红方[主哨岗] (D2)
// 3  红方[副哨岗] (D4)
#define MOTHED 2
// 当前身份 蓝/红
// "blue"   现在为蓝方    记得一定要小写,并且内容无误
// "red"    现在为红方    记得一定要小写,并且内容无误
#define WHO_AM_I "red"

// 同色同号判断严格程度
// "strict"     [严格]需要场上四台车都在, 但条件还是很严格(主哨岗连续检测+条件严格), 主哨岗检测不完全就不行;
// "relaxed"    [宽松]不限场上多少台车, 主哨岗能连续检测到一对同色同号即可, 但主哨岗检测不到还是不行;
#define SWAP_COLOR_CONDITION "relaxed"

// 录制
// 0 录制关闭
// 1 录制开启
#define WRITER 1
// 录制视频目标路径
#define WRITER_DIR "../vedio/"
// 05_17_drak_exp40000_sencond_3

// 接收 副哨岗IP
#define SENTRY_IP "tcp://192.168.1.154:5556"
// 接收 CAR1 IP
#define CAR1_IP "tcp://192.168.1.66:5555"
// 接收 CAR2 IP
#define CAR2_IP "tcp://192.168.1.88:5555"

// 显示虚拟地图
// 0 关闭
// 1 开启
#define MAPINFO_OFF 1
// 显示相机图片(并显示 |帧数|标志数据|坐标数据| )
// 0 关闭
// 1 开启
#define SHOW_IMG 1

// 数据融合函数距离阈值
// distanceCar  主副哨岗数据融合, 得出相同身份的两点, 相距大于该数值(60cm), 则认为不是同一台车
#define DISTANCE_CAR 60

// 同色同号情况投票阈值
#define SAME_COLOR_NUM_FRAMES_THRESHOLD 35
// 卧底识别情况投票阈值
#define PANGOLIN_FRAMES_THRESHOLD 35

// 卧底识别
// 当Car1,Car2给哨岗发数据的时候,可利用哨岗检测到的同样号数的坐标数据
// 求两个坐标数据之间的距离,若距离小于当前设定的距离阈值, 则判断哨岗检测到的该敌方颜色车辆为己方
#define PANGOLIN_DISTANCE_THRESHOLD 65
// 同色同号后, 依旧可以发送敌方坐标
// 同色同号后, 根据哨岗检测到的车辆坐标数据(同一个类别) 和 Car自身发给我的坐标数据 进行对比, 判断出那个坐标为敌方坐标
// POINT_CAR_DISTANCE_THRESHOLD 判断离Car坐标数据最远的那个坐标数据是否超过设定的该距离阈值(70cm), 若超, 则可以判断该数据为敌方坐标
#define POINT_CAR_DISTANCE_THRESHOLD 70

#include "Monitoring.hpp"
#include "Mapinfo.hpp"
#include "Message.hpp"
#include <thread>
#include <mutex>
#include <chrono>
#include <zmq.hpp>
#include "mv_video_capture.hpp"
#include <time.h>
#include <sstream>
// #include "fps.hpp"

struct Send{			// 套接字内容

    // int gray_num;           // 当前帧灰车的数量

    int swapColorModes;     // 交换颜色模式: 交换后异号异色车--0  交换后同号同色车--1

    int pangolin;          // 卧底

    // 占着茅坑不拉屎 1 2
    bool buff_1_;  // a_dog_in_the_toilet_on_shit_1 (F7)
    bool buff_2_;  // a_dog_in_the_toilet_on_shit_2 (F1)

    cv::Point2f blue1;
    cv::Point2f blue2;
    cv::Point2f red1;
    cv::Point2f red2;

    Send(int& swapColorModes, int& pangolin, cv::Point2f& blue1, cv::Point2f& blue2, cv::Point2f& red1, cv::Point2f& red2, bool& buff_1_, bool& buff_2_) {
        this->swapColorModes = swapColorModes;
        this->pangolin = pangolin;
        this->blue1 = blue1;
        this->blue2 = blue2;
        this->red1  = red1;
        this->red2  = red2;
        this->buff_1_ = buff_1_;
        this->buff_2_ = buff_2_;
    }
};

// 旋转矩阵
cv::Mat warpmatrix(3, 3, CV_64FC1);
// 回调函数点击事件
static void onMouse1(int event, int x, int y, int, void* userInput) {
    static int          times   = 0;
    static cv::Point2f  fourPoint[4];
	if (event != cv::EVENT_LBUTTONDOWN) {
        return;
    }
    else {
        times++;
	    if (times <= 4) {
            std::cout << x << "  " << y << std::endl;
            fourPoint[times-1].x = x;
            fourPoint[times-1].y = y;
        }
        else if (times == 5) {
		    cv::Point2f god_view[4];

#if CHECK_4_OPT == 0
            if (WHO_AM_I == "blue") {
                god_view[0] = cv::Point2f(100, 808-638);
                god_view[1] = cv::Point2f(348, 100);
                god_view[2] = cv::Point2f(100, 708);
                god_view[3] = cv::Point2f(348, 808-150);
                // god_view[] 	= { cv::Point2f(100, 808-638), cv::Point2f(348, 100), cv::Point2f(100, 708), cv::Point2f(348, 808-150) };
            }
		    if (WHO_AM_I == "red") {
                god_view[0] = cv::Point2f(100,      808-638);
                god_view[1] = cv::Point2f(348,      100);
                god_view[2] = cv::Point2f(100+20,   708);
                god_view[3] = cv::Point2f(348,      808-150);
                // god_view[] 	    = { cv::Point2f(100, 808-638), cv::Point2f(348, 100), cv::Point2f(100+20, 708), cv::Point2f(348, 808-150) };
            }
#endif
#if CHECK_4_OPT == 1
                god_view[0] = cv::Point2f(214,      808-578);
                god_view[1] = cv::Point2f(93.5,     808-354);
                god_view[2] = cv::Point2f(334.5,    808-354);
                god_view[3] = cv::Point2f(214,      808-150);
#endif

		    // 计算变换矩阵
		    warpmatrix = cv::getPerspectiveTransform(fourPoint, god_view);
		    std::cout << warpmatrix << std::endl;
        }
    }
}
// 获取透视变换矩阵
void getWarpMatrix(cv::Mat& img) {
    while(true) {
        cv::imshow("test", img);
        cv::setMouseCallback("test", onMouse1); // 调用回调函数
        if (cv::waitKey(1) == 'q') {
            cv::destroyWindow("test");
            break;
        }
    }
}

std::mutex  mtx;
CarInfoSend PC_2 = CarInfoSend{};
RobotCarPositionSend car1Info = RobotCarPositionSend{};
RobotCarPositionSend car2Info = RobotCarPositionSend{};
bool receive_sentry = false;    // 是否接收到哨岗信息
bool receive_car1   = false;    // 是否接收到car1信息
bool receive_car2   = false;    // 是否接收到car2信息
bool sentry_online  = true;     // 副哨岗是否在线
auto sentry_last = std::chrono::system_clock::now();    // 最后一次接收到哨岗信息的时间
auto sentry_now  = std::chrono::system_clock::now();    // 当前时间
auto sentry_diff = std::chrono::duration_cast<std::chrono::milliseconds>(sentry_last - sentry_now).count();
void start_2(zmq::socket_t& subscriber_sentry, zmq::socket_t& subscriber_car1, zmq::socket_t& subscriber_car2) {
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 休眠20ms

        // 获取副哨岗的信息
        zmq::message_t recv_message_sentry(sizeof(CarInfoSend));
        // 获取car1的信息
        zmq::message_t recv_message_car1(sizeof(RobotCarPositionSend));
        // 获取car2的信息
        zmq::message_t recv_message_car2(sizeof(RobotCarPositionSend));

mtx.lock();
        receive_sentry = false;
        while ( subscriber_sentry.recv(&recv_message_sentry, ZMQ_DONTWAIT) ) { 
            memcpy(&PC_2, recv_message_sentry.data(), sizeof(PC_2));
            receive_sentry = true;
            sentry_last = std::chrono::system_clock::now(); // 记录副哨岗信息时间戳
        }

        receive_car1 = false;
        while ( subscriber_car1.recv(&recv_message_car1, ZMQ_DONTWAIT) ) { 
            memcpy(&car1Info, recv_message_car1.data(), sizeof(car1Info));
            receive_car1 = true;
        }
        if (receive_car1) {
            car1Info.carPosition.x *= 100;
            car1Info.carPosition.y *= 100;
        }

        receive_car2 = false;
        while ( subscriber_car2.recv(&recv_message_car2, ZMQ_DONTWAIT) ) { 
            memcpy(&car2Info, recv_message_car2.data(), sizeof(car2Info));
            receive_car2 = true;
        }
        if (receive_car2) {
            car2Info.carPosition.x *= 100;
            car2Info.carPosition.y *= 100;
        }

        sentry_now  = std::chrono::system_clock::now();  // 刷新当前时间
        sentry_diff = std::chrono::duration_cast<std::chrono::milliseconds>(sentry_now - sentry_last).count();  // 统计最近一次副哨岗的信息距离现在的时间 (ms)
        // 若最后一次副哨岗信息距今超过1000ms(1s), 判断副哨岗掉线 
        if (sentry_diff < 1000) {
            sentry_online = true;   // 副哨岗在线
        }
        else {
            sentry_online = false;  // 副哨岗掉线
        }
mtx.unlock();
        std::cout << "副哨岗断连时间  :" << sentry_diff << std::endl;
        std::cout << "副哨岗在线状态  :" << sentry_online << std::endl;
        std::cout << "是否接受到副哨岗: " << receive_sentry << std::endl;
        std::cout << "是否接受到car1 : " << receive_car1 << std::endl;
        std::cout << "是否接受到car2 : " << receive_car2 << std::endl;
        std::cout << "car1Info      : " << car1Info.carPosition << "    hasAlly:  " << car1Info.hasAlly << std::endl;
        std::cout << "car2Info      : " << car2Info.carPosition << "    hasAlly:  " << car2Info.hasAlly << std::endl;
    }
}


void start_1() {
    // std::string engine_name = "/home/wsl/wolf_workspace/tensorrtx/yolov5/imgsz1280_1280_65_b.engine";
    // std::string engine_name = "/home/wsl/wolf_workspace/tensorrtx/yolov5/75last.engine";
    std::string engine_name = "/home/wsl/wolf_workspace/tensorrtx/yolov5/77_last.engine";
    // std::string img_dir     = "/home/wsl/myWolfTeamWork/v6.0/vedio/18_54_52.avi";
    std::string img_dir     = "/home/wsl/桌面/lwr_vedio/24_one.avi";

    cv::VideoCapture    cap(img_dir);  // cap捕捉图片流
    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
	// cap.set(cv::CAP_PROP_FRAME_HEIGHT, 800);

    std::vector<car>    result;        // 捕获结果 [模型推理后的信息处理结果 --> 详情看car结构体]
    TRTX                wsl(engine_name);
    cv::Mat             img;

    mindvision::VideoCapture mv_capture_ = mindvision::VideoCapture(
                                                    mindvision::CameraParam(0,          // 0--工业相机, 1--其他免驱相机
                                                                            mindvision::RESOLUTION_1280_X_1024,
                                                                            mindvision::EXPOSURE_NONE
                                                                            )
                                                                        );
    // fps::FPS       global_fps_;

#if WRITER == 1
    time_t vedio_name = time(NULL);
    tm* tm_t = localtime(&vedio_name);
    std::stringstream vns;
    vns << std::to_string(tm_t->tm_hour) << "_" << std::to_string(tm_t->tm_min) << "_" << std::to_string(tm_t->tm_sec);
    // 录制
    cv::VideoWriter writer;
    std::string out_path = WRITER_DIR + vns.str() +".avi";    // 目标路径
    cv::Size size(1280, 1024);                              // 重要! 要求与摄像头参数一致
    // int fourcc = writer.fourcc('X', 'V', 'I', 'D');      // 设置avi文件对应的编码格式 66 67
    int fourcc = writer.fourcc('M', 'J', 'P', 'G');     // 33 30 48Flv1
    // int fourcc = writer.fourcc('F', 'L', 'V', '1');      // 33 30 48Flv1
    writer.open(out_path, fourcc, 30, size, true);      // CAP_DSHOW = true
    if (writer.isOpened()) {
        std::cout << "正在录制" << std::endl;
    }
    else {
        std::cout << "录制失败" << std::endl;
    }
#endif  // WRITER_OFF

    // 检测是否有免驱相机
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return ;
    }
    if (mv_capture_.isindustryimgInput()) {
        img = mv_capture_.image();
    }
    else {
        cap.read(img);
    }
    // img *= 1.5;
    getWarpMatrix(img);                     // 手动点四点透视变换

    Monitoring wolfEye(warpmatrix);         // 哨岗类
    MapInfo mapInfo(wolfEye.getmatrix());   // 俯视图显示类
    Message chong;                          // 传输数据处理类
    static CarInfoSend PC_1;                // 要传输的数据结构变量
    
    // 初始化 发送
    zmq::context_t  ip_context(1);
    zmq::socket_t   publisher(ip_context, zmq::socket_type::pub);
    publisher.bind("tcp://*:5556");


    while (true) {
        auto start = std::chrono::system_clock::now();

        if (mv_capture_.isindustryimgInput()) {
            img = mv_capture_.image();
        }
        else {
            cap.read(img);
        }

#if WRITER == 1
        writer << img;
#endif  // WRITER_OFF

        auto car = wsl(img);                    // 检测 车车

        wolfEye.run(car, img, result);   // 得出结果 ————> result 

#if MAPINFO_OFF == 1
        // mapInfo.showTransformImg(img);                          // ++ 显示透视变换后的图片
        mapInfo.showMapInfo(result);                            // ++ 显示模拟地图
#endif  // MAPINFO_OFF

mtx.lock();
        // 获取需要发送的消息内容 // receive: 接收到数据,则融合 || 未接收到,则跳过
        PC_1 = chong(result, PC_2, receive_sentry, sentry_online, car1Info, receive_car1, car2Info, receive_car2);
        // // 重置接收数据flag
        // receive_sentry = false;
        // receive_car1   = false;
        // receive_car2   = false;
mtx.unlock();

#if MAPINFO_OFF == 1
        mapInfo.showMapInfo2(PC_1); // 显示融合后的数据
#endif  // MAPINFO_OFF

        Send PC_1_Send(PC_1.swapColorModes, PC_1.pangolin, PC_1.blue1, PC_1.blue2, PC_1.red1, PC_1.red2, PC_1.a_dog_in_the_toilet_on_shit_1, PC_1.a_dog_in_the_toilet_on_shit_2); // 要传输的给RoboCar的消息

        std::cout << "发送的内容是" << std::endl;
        std::cout << "swapColorModes:" << PC_1_Send.swapColorModes  << std::endl;
        std::cout << "pangolin:      " << PC_1_Send.pangolin        << std::endl;
        std::cout << "buff_1_:       " << PC_1_Send.buff_1_         << std::endl;
        std::cout << "buff_2_:       " << PC_1_Send.buff_1_         << std::endl;
        std::cout << "blue1:         " << PC_1_Send.blue1           << std::endl;
        std::cout << "blue2:         " << PC_1_Send.blue2           << std::endl;
        std::cout << "red1:          " << PC_1_Send.red1            << std::endl;
        std::cout << "red2:          " << PC_1_Send.red2            << std::endl;

        // 发送消息
        zmq::message_t  send_message(sizeof(Send));
        memcpy(send_message.data(), &PC_1_Send, sizeof(PC_1_Send));
        publisher.send(send_message);

        mv_capture_.cameraReleasebuff();   // 释放这一帧的内容

        auto end = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "整体时间" << diff << "ms" << std::endl << std::endl;

#if SHOW_IMG == 1
    // 在原图上绘制检测结果 start
        // 绘制 矩形(rectangle) 和 类编号(class_id)
        for (size_t j = 0; j < car.size(); j++) {               // car.size() 该图检测到多少个class
            // if( car[j].class_id == 0) {
                cv::Rect r  = get_rect(img, car[j].bbox);
                // r           = r + cv::Point(0, r.height/2);       //平移，左上顶点的 `x坐标`不变，`y坐标` +r.height/2
                // r           = r + cv::Size (0, -r.height/2);      //缩放，左上顶点不变，宽度不变，高度减半
                cv::rectangle(img, r, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
                cv::putText(img, std::to_string((int)car[j].class_id), cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0x00, 0xFF, 0x00), 2);
            // }
        }
        cv::putText(img, "swapColorModes : " + std::to_string(PC_1_Send.swapColorModes),  cv::Point(50, 50),  cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "pangolin       : " + std::to_string(PC_1_Send.pangolin),        cv::Point(50, 100), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "buff_1_        : " + std::to_string(PC_1_Send.buff_1_),         cv::Point(50, 150), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "buff_2_        : " + std::to_string(PC_1_Send.buff_2_),         cv::Point(50, 200), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "FPS            : " + std::to_string(1000.0/diff),               cv::Point(50, 250), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "sentry_online  : " + std::to_string(sentry_online),             cv::Point(50, 350), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "receive_car1   : " + std::to_string(receive_car1),              cv::Point(50, 400), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "receive_car2   : " + std::to_string(receive_car2),              cv::Point(50, 450), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);

        cv::putText(img, "car1      :", cv::Point(450, 400), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(car1Info.carPosition.x)),    cv::Point(650, 400), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(car1Info.carPosition.y)),    cv::Point(750, 400), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "car2      :", cv::Point(450, 450), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(car2Info.carPosition.x)),    cv::Point(650, 450), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(car2Info.carPosition.y)),    cv::Point(750, 450), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);

        cv::putText(img, "blue1          :", cv::Point(50, 550), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.blue1.x)),         cv::Point(350, 550), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.blue1.y)),         cv::Point(450, 550), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "blue2:         :", cv::Point(50, 600), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.blue2.x)),         cv::Point(350, 600), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.blue2.y)),         cv::Point(450, 600), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "red1           :", cv::Point(50, 650), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.red1.x)),          cv::Point(350, 650), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.red1.y)),          cv::Point(450, 650), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, "red2           :", cv::Point(50, 700), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.red2.x)),          cv::Point(350, 700), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
        cv::putText(img, std::to_string(int(PC_1_Send.red2.y)),          cv::Point(450, 700), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0x00, 0x00, 0xFF), 2);
/*
        // 在原图上画装甲板opt4
        const cv::Scalar colors[3] = {{255, 0, 0}, {0, 0, 255}, {0, 255, 0}};
        for (const auto &b : armor) {
            cv::line(img, b.pts[0], b.pts[1], colors[2], 2);
            cv::line(img, b.pts[1], b.pts[2], colors[2], 2);
            cv::line(img, b.pts[2], b.pts[3], colors[2], 2);
            cv::line(img, b.pts[3], b.pts[0], colors[2], 2);
            cv::putText(img, std::to_string(b.tag_id), b.pts[0], cv::FONT_HERSHEY_SIMPLEX, 1, colors[b.color_id]);
        }
*/
    // 在原图上绘制检测结果 end
        cv::imshow("yolov5", img);
        static char _key;
        _key = cv::waitKey(1);
        if (_key == 'q') {
            break;
        }
        else if (_key == 'c') {
            chong.restart();
        }
#endif // SHOW_IMG

    }

#if WRITER == 1
    writer.release();
#endif  // WRITER_OFF

    cap.release();
}

int main(int argc, char **argv) {
    // 初始化 接收 副哨岗
    zmq::context_t receive_context_sentry(1);
    zmq::socket_t subscriber_sentry(receive_context_sentry, ZMQ_SUB);
    subscriber_sentry.connect(SENTRY_IP);    // "tcp://192.168.1.154:5556"
    subscriber_sentry.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // 初始化 接收 car1Info
    zmq::context_t receive_context_car1(1);
    zmq::socket_t subscriber_car1(receive_context_car1, ZMQ_SUB);
    subscriber_car1.connect(CAR1_IP);  // "tcp://192.168.1.66:5555"
    subscriber_car1.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // 初始化 接收 car2Info
    zmq::context_t receive_context_car2(1);
    zmq::socket_t subscriber_car2(receive_context_car2, ZMQ_SUB);
    subscriber_car2.connect(CAR2_IP);  // "tcp://192.168.1.89:5555"
    subscriber_car2.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    std::thread t2 = std::thread(start_2, std::ref(subscriber_sentry), std::ref(subscriber_car1), std::ref(subscriber_car2)); // 接收 副哨岗,car1Info,car2Info 信息
    std::thread t1 = std::thread(start_1);

    t2.detach();
    t1.join();

    return 0;
}