#ifndef _MESSAGE_HPP_
#define _MESSAGE_HPP_

#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/opencv.hpp>
#include <Correct.hpp>
#include <algorithm>

// typedef struct
// {
//     cv::Rect    img_r;              // `图像`上的矩形框信息
//     cv::Point2f carPosition;        // 矫正前的`世界地图`上的坐标点
//     cv::Point2f carPositionFixed;   // 矫正后的`世界地图`上的坐标点
//     int         color;              // 0蓝 1红 2黑
//     int         num;                // 1 / 2
// } car;

typedef struct 
{
    cv::Point2f carPosition;    // 车当前坐标
    bool        hasAlly;        // 存在队友?  FALSE / TRUE
} RobotCarPositionSend;


typedef struct {			// 套接字内容

    int gray_num;           // 当前帧灰车的数量

    int swapColorModes;     // 交换颜色模式: 交换后异号异色车--0  交换后同号同色车--1

    // 在潜伏模式后, 穿山甲的颜色会变成敌方的颜色
    // 在潜伏模式后, 己方车辆发送 [自身的地图坐标&队友是否死亡的信息] 给主哨岗, 主哨岗取其信息进行卧底车牌号判断
    int pangolin;           // 穿山甲
    
    // 占着茅坑不拉屎 1 2
    bool a_dog_in_the_toilet_on_shit_1;
    bool a_dog_in_the_toilet_on_shit_2;

    cv::Point2f blue1;
    cv::Point2f blue2;
    cv::Point2f red1;
    cv::Point2f red2;

    cv::Point2f blue1_2;
    cv::Point2f blue2_2;
    cv::Point2f red1_2;
    cv::Point2f red2_2;

    cv::Point2f gray_1;
    cv::Point2f gray_2;
    cv::Point2f gray_3;
    cv::Point2f gray_4;

} CarInfoSend;

class Message {
    private:
        CarInfoSend     PC_1;

        // 查交换颜色情况
        int             sameColorNumFrames = 0;         // 车同号同色出现的帧数记数, 用于判断 交换颜色模式 0/1
        int             singleFrameSameColorNumSroce;   // 单帧出现同车同号的得分，出现一次+1分，出现两次则可以确认并且给 sameColorNumFrames += 1

        // 查卧底
        int             pangolinIs_1Frames = 0;         // 判断卧底为1的帧数记数, 用于判断卧底号数
        int             pangolinIs_2Frames = 0;         // 判断卧底为2的帧数记数, 用于判断卧底号数
        int             pangolinIs_1;                   // 单帧判断卧底为1的投票, 使得 pangolinIs_1Frames ++
        int             pangolinIs_2;                   // 单帧判断卧底为2的投票, 使得 pangolinIs_1Frames ++

        const std::string whoAmI    = WHO_AM_I; // blue or red 现在是蓝方还是红方
        // 检测同色同号主要靠主哨岗检测，检测同色同号条件为 [严格] or [宽松]
        //      [严格]需要场上四台车都在, 但条件还是很严格(主哨岗连续检测+条件严格), 主哨岗检测不完全就不行;
        //      [宽松]不限场上多少台车, 主哨岗能连续检测到一对同色同号即可, 但主哨岗检测不到还是不行;
        const std::string swapColorCondition = SWAP_COLOR_CONDITION; // strict or relaxed

    public:
        Message();
        ~Message();
        void init();
        void restart();

        CarInfoSend operator()(std::vector<car>& result, CarInfoSend& PC_2, bool& receive_sentry, bool& sentry_online, RobotCarPositionSend& car1Info, bool& receive_car1, RobotCarPositionSend& car2Info, bool& receive_car2);

        void CarPlaceMerge(cv::Point2f& CarLocation1, cv::Point2f& CarLocation1_2, cv::Point2f& CarLocation2, cv::Point2f& CarLocation2_2);
        void swapPointCheck(cv::Point2f& point1, cv::Point2f& point2);

        void sameColorCheckPoint(cv::Point2f& point_1, cv::Point2f& point_2, const cv::Point2f& carPoint);
};

Message::Message() {
    init();
}

Message::~Message() {

}

void Message::restart() {
    Message::sameColorNumFrames = 0;
    Message::pangolinIs_1Frames = 0;
    Message::pangolinIs_2Frames = 0;
}

void Message::init() {
    Message::singleFrameSameColorNumSroce = 0;
    Message::pangolinIs_1 = 0;
    Message::pangolinIs_2 = 0;

    PC_1.pangolin = -1;  // 穿山甲-1初始化

    PC_1.a_dog_in_the_toilet_on_shit_1 = false;
    PC_1.a_dog_in_the_toilet_on_shit_2 = false;

    PC_1.gray_num = 0;
    PC_1.swapColorModes = 0;

    PC_1.blue1.x = -1;
    PC_1.blue1.y = -1;

    PC_1.blue2.x = -1;
    PC_1.blue2.y = -1;

    PC_1.red1.x = -1;
    PC_1.red1.y = -1;

    PC_1.red2.x = -1;
    PC_1.red2.y = -1;

    PC_1.blue1_2.x = -1;
    PC_1.blue1_2.y = -1;

    PC_1.blue2_2.x = -1;
    PC_1.blue2_2.y = -1;

    PC_1.red1_2.x = -1;
    PC_1.red1_2.y = -1;

    PC_1.red2_2.x = -1;
    PC_1.red2_2.y = -1;

    PC_1.gray_1.x = -1;
    PC_1.gray_1.y = -1;

    PC_1.gray_2.x = -1;
    PC_1.gray_2.y = -1;

    PC_1.gray_3.x = -1;
    PC_1.gray_3.y = -1;

    PC_1.gray_4.x = -1;
    PC_1.gray_4.y = -1;
}

float relu(float _frame) {
    static float zero = 0.0;
    // return std::max(_frame, zero);
    if (_frame < zero) {
        return zero;
    }
    return _frame;
}

float getDistance(const cv::Point2f& point1, const cv::Point2f& point2) {
    static double distance;
    distance = sqrtf(powf((point1.x - point2.x),2) + powf((point1.y - point2.y),2));
    return distance;
}


// 当出现同号同色的两车时才调用该函数
/*
    4个类别 blue1, blue2, red1, red2
      point_1 和 point_2 为同一个类别
        point_1为当前类别检测到的第一个坐标
        point_2为当前类别检测到的第二个坐标
      当point_2有数据时, point_1必有数据
    
      CarInfo为车辆传过来的坐标信息
*/
void Message::sameColorCheckPoint(cv::Point2f& point_1, cv::Point2f& point_2, const cv::Point2f& carPoint) {
    static cv::Point2f _nothing = cv::Point2f(-1,-1);
    float dis_point1_carPoint = 0;    // cm
    float dis_point2_carPoint = 0;    // cm

    // 若point_1不为空数据, 求point_1与车传来的坐标数据进行距离计算
    if (point_1 != _nothing) {
        dis_point1_carPoint = getDistance(point_1, carPoint);
    }
    if (point_2 != _nothing) {
        dis_point2_carPoint = getDistance(point_2, carPoint);
    }
    
    static float point_Car_DistanceThreshold = POINT_CAR_DISTANCE_THRESHOLD; // cm
    // point_2离carPoint更远
    if (dis_point1_carPoint < dis_point2_carPoint) {
        // 若该point_2与carPoint的距离>=阈值, 判断point_2为敌方坐标
        if (dis_point2_carPoint >= point_Car_DistanceThreshold) { 
            std::swap(point_1, point_2);
        }
        // 两个距离都未超过阈值, 三个坐标靠的很近, 无须哨岗提供敌方坐标信息
        else {
            point_1.x=-1,point_1.y=-1;
            point_2.x=-1,point_2.y=-1;
        }
    }
    // point_1 和 point_2  距离 carPoint 同样远
    else if (dis_point1_carPoint == dis_point2_carPoint) {
        // 可能point_1和point_2都没有数据, 无须操作
        // 可能point_1和point_2距离carPoint同样远, 很刁钻, 没什么办法
    }
    // 可能只检测到了point_1, 或者刚好point_1为敌方坐标
    else if (dis_point1_carPoint > dis_point2_carPoint) {
        // 若该point_1与carPoint的距离>=阈值, 判断point_1为敌方坐标
        if (dis_point1_carPoint >= point_Car_DistanceThreshold) {
            // 无须操作
        }
        // 两个距离都未超过阈值, 
        //   或许三个坐标靠的很近, 无须哨岗提供敌方坐标信息
        //   又或许只检测到自方车辆, 也无须哨岗提供敌方坐标信息
        else {
            point_1.x=-1,point_1.y=-1;
            point_2.x=-1,point_2.y=-1;
        }
    }
}

// 当出现同号同色的两车时才调用该函数
/*      需要 Point1 的点为靠近哨岗的点
        判断 y轴值相近 fasb<=20 (以主哨岗为参考点测距)
        选择面积小的点为point1
*/
void Message::swapPointCheck(cv::Point2f& point1, cv::Point2f& point2) {
    static cv::Point2f myPosition;
    if (Message::whoAmI == "blue") {
        myPosition = cv::Point2f(0.0, 808.0);  // 当前地图中主哨岗的全局坐标
    }
    else if (Message::whoAmI == "red") {
        myPosition = cv::Point2f(0.0, 0.0);    // 当前地图中主哨岗的全局坐标
    }

    // static cv::Point2f myPosition = cv::Point2f(0.0, 0.0);  // 当前哨岗全局坐标, 若换坐标系表示的时候，需要改

    if ( ( std::fabs(point1.y-point2.y) <= 20 ) &&          // 808的边用y表示, 若这两个坐标的y轴相近,则需要取判断那个比较靠近主哨岗, !如果要更换坐标系表示,y不再表示808这条边,则这段代码需要改动为x
         ( getDistance(point1, myPosition) > getDistance(point2, myPosition) )
    ) {
        std::swap(point1, point2);
    }
}

// 原本为 getMean函数, 发现两方哨岗得出的数据距离真实值差的不多,且偏差的位置相反,使用求均获得真实值理论上挺好的;
// 但由于副哨岗和主哨岗之间帧数差距过大,会造成定位数据的抖动, 故而弃掉, 只要确保主副哨岗都检测到的时候, 只返回主哨岗检测的坐标数据就好了
cv::Point2f chooseOne(const cv::Point2f& point1, const cv::Point2f& point2) {
    // static cv::Point2f mean;
    // // 加个判断? 当双点差距过大, return point1;
    // mean = cv::Point2f((point1.x+point2.x)/2.0, (point1.y+point2.y)/2.0);
    // return mean;
    
    return point1;
}

/*
    传入的参数表示的 [同号同色的车] 在主[1]副[2]哨岗上检测到的 [不同位置]
    主哨岗: CarLocation1, CarLocation1_2
    副哨岗: CarLocation2, CarLocation2_2
*/
void Message::CarPlaceMerge(cv::Point2f& CarLocation1, cv::Point2f& CarLocation1_2, cv::Point2f& CarLocation2, cv::Point2f& CarLocation2_2) {
    // 0: 无任何数据
    if ( (CarLocation1   == cv::Point2f(-1,-1) && CarLocation2   == cv::Point2f(-1,-1)) && 
         (CarLocation1_2 == cv::Point2f(-1,-1) && CarLocation2_2 == cv::Point2f(-1,-1))
    ) {
        return ;
    }
    // 1: 主哨岗检测到一个位置, 其他为空
    else if ( (CarLocation1   != cv::Point2f(-1,-1) && CarLocation2   == cv::Point2f(-1,-1)) &&
              (CarLocation1_2 == cv::Point2f(-1,-1) && CarLocation2_2 == cv::Point2f(-1,-1))
    ) {
        // CarLocation1 无需改动
        return ;
    }
    // 1: 副哨岗检测到一个位置, 其他为空
    else if ( (CarLocation1   == cv::Point2f(-1,-1) && CarLocation2   != cv::Point2f(-1,-1)) &&
              (CarLocation1_2 == cv::Point2f(-1,-1) && CarLocation2_2 == cv::Point2f(-1,-1))
    ) {
        CarLocation1 = CarLocation2;
    }
    // 2: 主副哨岗各检测到一个位置
    else if ( (CarLocation1   != cv::Point2f(-1,-1) && CarLocation2   != cv::Point2f(-1,-1)) &&
              (CarLocation1_2 == cv::Point2f(-1,-1) && CarLocation2_2 == cv::Point2f(-1,-1))
    ) {
        float distanceCar = getDistance(CarLocation1, CarLocation2);
        // 两点距离超过 [60] 厘米, 判定为两台不一样的车
        if (distanceCar > DISTANCE_CAR) {
            // CarLocation1 无需改动
            CarLocation1_2 = CarLocation2;  // 副哨岗赋值给主哨岗
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
        // 判定为同一台车的坐标，数据求mean融合
        else {
            CarLocation1 = chooseOne(CarLocation1, CarLocation2);
        }
    }
    // 2: 主哨岗检测到两个位置，副哨岗无
    else if ( (CarLocation1   != cv::Point2f(-1,-1) && CarLocation2   == cv::Point2f(-1,-1)) &&
              (CarLocation1_2 != cv::Point2f(-1,-1) && CarLocation2_2 == cv::Point2f(-1,-1))
    ) {
        // 无需操作
    }
    // 2: 副哨岗检测到两个位置，主哨岗无
    else if ( (CarLocation1   == cv::Point2f(-1,-1) && CarLocation2   != cv::Point2f(-1,-1)) &&
              (CarLocation1_2 == cv::Point2f(-1,-1) && CarLocation2_2 != cv::Point2f(-1,-1))
    ) {
        CarLocation1    = CarLocation2;
        CarLocation1_2  = CarLocation2_2;
    }
    // 3: 主哨岗检测到两个位置, 副哨岗检测到一个位置
    else if ( (CarLocation1   != cv::Point2f(-1,-1) && CarLocation2   != cv::Point2f(-1,-1)) &&
              (CarLocation1_2 != cv::Point2f(-1,-1) && CarLocation2_2 == cv::Point2f(-1,-1))
    ) {
        // 分别求 CarLocation1到CarLocation2的距离 和 CarLocation1_2到CarLocation2的距离
        float distanceCar_1 = getDistance(CarLocation1,   CarLocation2);
        float distanceCar_2 = getDistance(CarLocation1_2, CarLocation2);
        // 若CarLocation1比CarLocation1_2 离 CarLocation2 更近,则判断CarLocation1和CarLocation2判断的为同一台车
        if (distanceCar_1 < distanceCar_2) {
            CarLocation1 = chooseOne(CarLocation1, CarLocation2);
            // CarLocation1_2 无需改动
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
        // 反之
        else {
            // CarLocation1 无需改动
            CarLocation1_2 = chooseOne(CarLocation1_2, CarLocation2);
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
    }
    // 3: 主哨岗检测到一个位置, 副哨岗检测到两个位置
    else if ( (CarLocation1   != cv::Point2f(-1,-1) && CarLocation2   != cv::Point2f(-1,-1)) &&
              (CarLocation1_2 == cv::Point2f(-1,-1) && CarLocation2_2 != cv::Point2f(-1,-1))
    ) { 
        // 分别求 CarLocation1到CarLocation2的距离 和 CarLocation1到CarLocation2_2的距离
        float distanceCar_1 = getDistance(CarLocation1, CarLocation2);
        float distanceCar_2 = getDistance(CarLocation1, CarLocation2_2);
        // 若CarLocation2比CarLocation2_2 离 CarLocation1 更近,则判断CarLocation2和CarLocation1判断的为同一台车
        if (distanceCar_1 < distanceCar_2) {
            CarLocation1    = chooseOne(CarLocation1, CarLocation2);
            CarLocation1_2  = CarLocation2_2;   // 副哨岗赋值给主哨岗
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
        // 反之
        else {
            CarLocation1    = chooseOne(CarLocation1, CarLocation2_2);
            CarLocation1_2  = CarLocation2;
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
    }
    // 4: 主副哨岗都检测到两个位置
    else if ( (CarLocation1   != cv::Point2f(-1,-1) && CarLocation2   != cv::Point2f(-1,-1)) &&
              (CarLocation1_2 != cv::Point2f(-1,-1) && CarLocation2_2 != cv::Point2f(-1,-1))
    ) {
        // 相信单哨岗的 y轴从小到大排序放置 和 位置交换?
        float distanceCar_1  = getDistance(CarLocation1, CarLocation2);
        float distanceCar_2  = getDistance(CarLocation1, CarLocation2_2);
        if (distanceCar_1 < distanceCar_2) {
            CarLocation1    = chooseOne(CarLocation1,   CarLocation2);
            CarLocation1_2  = chooseOne(CarLocation1_2, CarLocation2_2);
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
        // 反之
        else {
            CarLocation1    = chooseOne(CarLocation1,   CarLocation2_2);
            CarLocation1_2  = chooseOne(CarLocation1_2, CarLocation2);
            swapPointCheck(CarLocation1, CarLocation1_2);
        }
    }
}

//自定义排序函数  
bool SortByCarPositionFixedY(const car& _car1_sort, const car& _car2_sort)   //注意：本函数的参数的类型一定要与vector中元素的类型一致  
{   
    return _car1_sort.carPositionFixed.y < _car2_sort.carPositionFixed.y;  //升序排列
}

CarInfoSend Message::operator()(std::vector<car>& result, CarInfoSend& PC_2, bool& receive_sentry, bool& sentry_online, RobotCarPositionSend& car1Info, bool& receive_car1, RobotCarPositionSend& car2Info, bool& receive_car2) {
    init();
//     cv::Point2f carPositionFixed;   // 矫正后的`世界地图`上的坐标点
//     int         color;              // 0蓝 1红 2黑
//     int         num;                // 1 / 2
    
    // 对 std::vector<car> result 以 y轴从小到大排序
    std::sort(result.begin(), result.end(), SortByCarPositionFixedY);

    for (car& info: result) {
        if (info.num == 1) {            // 先车牌 <-- `1`
            if (info.color == 0) {      // 后颜色 <-- `blue`0
                if (PC_1.blue1 == cv::Point2f(-1 ,-1)) {    // 若未出现该 类型[color, num] 的车，则直接赋值
                    PC_1.blue1 = info.carPositionFixed;
                }
                else {                                      // 若已经出现该 类型[color, num] 的车，则为blue1_2赋值
                    PC_1.blue1_2 = info.carPositionFixed;
                    swapPointCheck(PC_1.blue1, PC_1.blue1_2);
                }
            }
            else if (info.color == 1) { // 后颜色 <-- `red` 1
                if (PC_1.red1 == cv::Point2f(-1 ,-1)) {
                    PC_1.red1 = info.carPositionFixed;
                }
                else {
                    PC_1.red1_2 = info.carPositionFixed;
                    swapPointCheck(PC_1.red1, PC_1.red1_2);
                }  
            }
            // else if (info.color == 2) { // 后颜色 <-- `gray`2
            //     PC_1.gray_num += 1;
            // }
        }
        else if (info.num == 2) {       // 先车牌 <-- `2`
            if (info.color == 0) {      // 后颜色 <-- `blue`0
                if (PC_1.blue2 == cv::Point2f(-1 ,-1)) {
                    PC_1.blue2 = info.carPositionFixed;
                }
                else {
                    PC_1.blue2_2 = info.carPositionFixed;
                    swapPointCheck(PC_1.blue2, PC_1.blue2_2);
                }
            }
            else if (info.color == 1) { // 后颜色 <-- `red` 1
                if (PC_1.red2 == cv::Point2f(-1 ,-1)) {
                    PC_1.red2 = info.carPositionFixed;
                }
                else {
                    PC_1.red2_2 = info.carPositionFixed;
                    swapPointCheck(PC_1.red2, PC_1.red2_2);
                }
            }
            // else if (info.color == 2) { // 后颜色 <-- `gray`2
            //     PC_1.gray_num += 1;
            // }
        }
        else {  // 现在只剩下 [灰装甲板](num==-1) / [无装甲板] 车辆了,需要把 灰车数据保存下来
            if (info.color == 2) {
                PC_1.gray_num += 1;

                if (PC_1.gray_num == 1) {
                    PC_1.gray_1 = info.carPositionFixed;
                }
                else if (PC_1.gray_num == 2) {
                    PC_1.gray_2 = info.carPositionFixed;
                }
                else if (PC_1.gray_num == 3) {
                    PC_1.gray_3 = info.carPositionFixed;
                }
                else if (PC_1.gray_num == 4) {
                    PC_1.gray_4 = info.carPositionFixed;
                }
            }
        }
    }
    
    // 占着茅坑不拉屎判断
    // PC_1.a_dog_in_the_toilet_on_shit_1 一号茅坑判断 (F7_BUFF)
    // static cv::Rect buff_1 当前地图的 [一号茅坑]的(左上角位置和宽高大小)
    static cv::Rect buff_1 = cv::Rect(255-40,23 , 48+60,54+30); // x, y, width, height
    if (PC_1.a_dog_in_the_toilet_on_shit_1 == false && 
        PC_1.gray_1 != cv::Point2f(-1,-1)) {
            PC_1.a_dog_in_the_toilet_on_shit_1 = buff_1.contains(PC_1.gray_1);
    }
    if (PC_1.a_dog_in_the_toilet_on_shit_1 == false && 
        PC_1.gray_2 != cv::Point2f(-1,-1)) {
                PC_1.a_dog_in_the_toilet_on_shit_1 = buff_1.contains(PC_1.gray_2);
    }
    if (PC_1.a_dog_in_the_toilet_on_shit_1 == false && 
        PC_1.gray_3 != cv::Point2f(-1,-1)) {
                PC_1.a_dog_in_the_toilet_on_shit_1 = buff_1.contains(PC_1.gray_3);
    }
    if (PC_1.a_dog_in_the_toilet_on_shit_1 == false && 
        PC_1.gray_4 != cv::Point2f(-1,-1)) {
                PC_1.a_dog_in_the_toilet_on_shit_1 = buff_1.contains(PC_1.gray_4);
    }
    // PC_1.a_dog_in_the_toilet_on_shit_2 二号茅坑判断 (F1_BUFF)
    // static cv::Rect buff_2 当前地图的 [二号茅坑]的(左上角位置和宽高大小)
    static cv::Rect buff_2 = cv::Rect(145-20,731-20 , 48+30,54+30); // x, y, width, height
    if (PC_1.a_dog_in_the_toilet_on_shit_2 == false && 
        PC_1.gray_1 != cv::Point2f(-1,-1)) {
            PC_1.a_dog_in_the_toilet_on_shit_2 = buff_2.contains(PC_1.gray_1);
    }
    if (PC_1.a_dog_in_the_toilet_on_shit_2 == false && 
        PC_1.gray_2 != cv::Point2f(-1,-1)) {
                PC_1.a_dog_in_the_toilet_on_shit_2 = buff_2.contains(PC_1.gray_2);
    }
    if (PC_1.a_dog_in_the_toilet_on_shit_2 == false && 
        PC_1.gray_3 != cv::Point2f(-1,-1)) {
                PC_1.a_dog_in_the_toilet_on_shit_2 = buff_2.contains(PC_1.gray_3);
    }
    if (PC_1.a_dog_in_the_toilet_on_shit_2 == false && 
        PC_1.gray_4 != cv::Point2f(-1,-1)) {
                PC_1.a_dog_in_the_toilet_on_shit_2 = buff_2.contains(PC_1.gray_4);
    }

    /* 数据融合
        bool sentry_online 表示是否副哨岗是否还在线(若超过1s未给主哨岗发送数据,则判断为副哨岗掉线, 若重新开始给副哨岗发送信息,则刷新该标志位)
    */
    if (sentry_online) {
        // PC_1.blue1   PC_1.blue1_2    PC_2.blue1  PC_2.blue1_2  ==> 除去 cv::Point2f(-1, -1)的, 剩下的去聚类, 以此类推
        CarPlaceMerge(PC_1.blue1, PC_1.blue1_2, PC_2.blue1, PC_2.blue1_2);
        CarPlaceMerge(PC_1.red1,  PC_1.red1_2,  PC_2.red1,  PC_2.red1_2);
        CarPlaceMerge(PC_1.blue2, PC_1.blue2_2, PC_2.blue2, PC_2.blue2_2);
        CarPlaceMerge(PC_1.red2,  PC_1.red2_2,  PC_2.red2,  PC_2.red2_2);

        // 选两个哨岗检测出的死车max数
        PC_1.gray_num = std::max(PC_1.gray_num, PC_2.gray_num);

        // (F7_buff 区的灰车检测)
        if (PC_2.a_dog_in_the_toilet_on_shit_1) {
            PC_1.a_dog_in_the_toilet_on_shit_1 = true;
        }
        // (F1_buff 区的灰车检测)
        if (PC_2.a_dog_in_the_toilet_on_shit_2) {
            PC_1.a_dog_in_the_toilet_on_shit_2 = true;
        }
    }

    // receive_sentry 表示是否接收到 哨岗消息    接收到true 未接收到false
    // sentry_online  表示哨岗是否在线          在线true   不在线false
    // 若 (!sentry_online)==false, 则 receive_sentry ==true  ==> 副哨岗在线,  且接收到副哨岗消息,  此时进行数据融合后的      [同色同号]&[卧底] 检测
    // 若 (!sentry_online)==ture,  则 receive_sentry ==false ==> 副哨岗掉线,  必收不到副哨岗消息,  此时进行用主哨岗信息的     [同色同号]&[卧底] 检测
    // 若 (!sentry_online)==false, 则 receive_sentry ==false ==> 副哨岗在线,  但未接收到副哨岗消息, 未有数据融合,此时不进行(因缺失完整信息) [同色同号]&[卧底] 检测
    static int sameColorNumFramesThreshold = SAME_COLOR_NUM_FRAMES_THRESHOLD;
    if ((!sentry_online) || receive_sentry) {

        /*同色同号检测
            有同号同色情况, Message::singleFrameSameColorNumSroce += 1
        */
        if (PC_1.blue1   != cv::Point2f(-1, -1) &&
            PC_1.blue1_2 != cv::Point2f(-1, -1)) {
            Message::singleFrameSameColorNumSroce += 1;
        }
        if (PC_1.blue2   != cv::Point2f(-1, -1) &&
            PC_1.blue2_2 != cv::Point2f(-1, -1)) {
            Message::singleFrameSameColorNumSroce += 1;
        }
        if (PC_1.red1    != cv::Point2f(-1, -1) &&
            PC_1.red1_2  != cv::Point2f(-1, -1)) {
            Message::singleFrameSameColorNumSroce += 1;
        }
        if (PC_1.red2    != cv::Point2f(-1, -1) &&
            PC_1.red2_2  != cv::Point2f(-1, -1)) {
            Message::singleFrameSameColorNumSroce += 1;
        }

        if (Message::sameColorNumFrames < sameColorNumFramesThreshold) {
            // 场上四辆车,严格的条件,主哨岗能一直检测到全部车辆(至少三辆车), 才能判断
            if (Message::swapColorCondition == "strict") {
                // 记录同车同号出现的帧数
                /*
                    一: 无车死亡
                        [无灰车 PC_1.gray_num == 0, 单帧 车同号同色 情况出现两次 Message::singleFrameSameColorNumSroce == 2 ] -------> sameColorNumFrames+=1
                    二: 己方未死, 敌方死一台
                        [有灰车 PC_1.gray_num == 1, 单帧 车同号同色 情况出现一次 Message::singleFrameSameColorNumSroce == 1 ] -------> sameColorNumFrames+=1
                    三: 己方死一台, 敌方未死
                        [有灰车 PC_1.gray_num == 1, 单帧 车同号同色 情况出现一次 Message::singleFrameSameColorNumSroce == 1 ] -------> sameColorNumFrames+=1
                    四: 双方各死一台
                        [PC_1.gray_num == 2,       单帧 车同号同色 情况出现一次 Message::singleFrameSameColorNumSroce == 1 ] -------> sameColorNumFrames+=1

                    但由于三四都为己方死一台车为前提, 就不必考虑是否误伤队友, 这时 sameColorNumFrames和socketInfo.swapColorModes 的值是什么已经无所谓了
                */
                if (PC_1.gray_num != 2 && Message::singleFrameSameColorNumSroce + PC_1.gray_num == 2) {
                    Message::sameColorNumFrames += 1;            // 车同号同号色出现的帧数+1
                }
                else if (PC_1.gray_num == 2 && Message::singleFrameSameColorNumSroce==1) {
                    Message::sameColorNumFrames += 1;
                }
                else {
                    Message::sameColorNumFrames -= 2;
                    Message::sameColorNumFrames = relu(Message::sameColorNumFrames);
                }
            }
            // 宽松的条件, 不限场上多少辆车, 只要主哨岗连续紧密地检测到同色同号条件即可
            else if (Message::swapColorCondition == "relaxed") {
                if (Message::singleFrameSameColorNumSroce == 1) {
                    Message::sameColorNumFrames += 1;            // 车同号同号色出现的帧数+1
                }
                else {
                    Message::sameColorNumFrames -= 2;
                    Message::sameColorNumFrames = relu(Message::sameColorNumFrames);
                }
            }
        }
    }
    // 当sameColorNumFrames累积达到某个阈值时，即多次检测到[两辆同车同号]的情况出现时 --> 确认潜伏模式后的车车颜色交换情况
    if (Message::sameColorNumFrames >= sameColorNumFramesThreshold) {
        PC_1.swapColorModes = 1;      // 达到阈值, 判定 1 --> 最麻烦的情况, 敌我车辆同号同色
    }

    
    // receive_sentry 表示是否接收到 哨岗消息    接收到true 未接收到false
    // sentry_online  表示哨岗是否在线          在线true   不在线false
    // 若 (!sentry_online)==false, 则 receive_sentry ==true  ==> 副哨岗在线,  且接收到副哨岗消息,  此时进行数据融合后的      [同色同号]&[卧底] 检测
    // 若 (!sentry_online)==ture,  则 receive_sentry ==false ==> 副哨岗掉线,  必收不到副哨岗消息,  此时进行用主哨岗信息的     [同色同号]&[卧底] 检测
    // 若 (!sentry_online)==false, 则 receive_sentry ==false ==> 副哨岗在线,  但未接收到副哨岗消息, 此时不进行(因缺失完整信息) [同色同号]&[卧底] 检测
    static int pangolinFramesThreshold = PANGOLIN_FRAMES_THRESHOLD;
    if ((!sentry_online) || receive_sentry) {
        /*  查卧底
                bool receive_car1   表示是否接收到car1位置数据
                bool receive_car2   表示是否接收到car2位置数据
        */
        // static std::string whoAmI    = "blue"; // blue or red
        static cv::Point2f nothing   = cv::Point2f(-1,-1);
        static float       pangolinDistanceThreshold = PANGOLIN_DISTANCE_THRESHOLD;
        
        if (Message::whoAmI == "blue") {
            if (receive_car1 == true) {
                float min_dis_car = getDistance(cv::Point2f(808+10, 448+10), nothing);
                float dis_car;

                if (PC_1.red1 != nothing) {
                    dis_car = getDistance(PC_1.red1,   car1Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_1 = 1;
                    }
                }
                if (PC_1.red1_2 != nothing) {
                    dis_car = getDistance(PC_1.red1_2, car1Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_1 = 1;
                    }
                }
                // if (PC_1.red2 != nothing) {
                //     dis_car = getDistance(PC_1.red2,   car1Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_1 = 1;
                //     }
                // }
                // if (PC_1.red2_2 != nothing) {
                //     dis_car = getDistance(PC_1.red2_2, car1Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_1 = 1;
                //     }
                // }

                // 若该距离还大于某个阈值, 则判断为目前哨岗的数据没有一个可以匹配该车辆数据的
                if (min_dis_car > pangolinDistanceThreshold)
                {
                    Message::pangolinIs_1 = 0;
                }
                // 当car1的队友不存在了, 且自己并不是卧底时, 可以有效怀疑死去的队友是卧底
                if (Message::pangolinIs_1 == 0 && car1Info.hasAlly == false) {
                    Message::pangolinIs_2 = 1;
                }
            }
            if (receive_car2 == true) {
                float min_dis_car = getDistance(cv::Point2f(808+10, 448+10), nothing);
                float dis_car;

                // if (PC_1.red1 != nothing) {
                //     dis_car = getDistance(PC_1.red1,   car2Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_2 = 1;
                //     }
                // }
                // if (PC_1.red1_2 != nothing) {
                //     dis_car = getDistance(PC_1.red1_2, car2Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_2 = 1;
                //     }
                // }
                if (PC_1.red2 != nothing) {
                    dis_car = getDistance(PC_1.red2,   car2Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_2 = 1;
                    }
                }
                if (PC_1.red2_2 != nothing) {
                    dis_car = getDistance(PC_1.red2_2, car2Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_2 = 1;
                    }
                }

                // 若该距离还大于某个阈值, 则判断为目前哨岗的数据没有一个可以匹配该车辆数据的
                if (min_dis_car > pangolinDistanceThreshold)
                {
                    Message::pangolinIs_2 = 0;
                }
                // 当car2的队友不存在了, 且自己并不是卧底时, 可以有效怀疑死去的队友是卧底
                if (Message::pangolinIs_2 == 0 && car2Info.hasAlly == false) {
                    Message::pangolinIs_1 = 1;
                }
            }
        }
        else if (Message::whoAmI == "red") {
            if (receive_car1 == true) {
                float min_dis_car = getDistance(cv::Point2f(808+10, 448+10), nothing);
                float dis_car;

                if (PC_1.blue1 != nothing) {
                    dis_car = getDistance(PC_1.blue1,   car1Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_1 = 1;
                    }
                }
                if (PC_1.blue1_2 != nothing) {
                    dis_car = getDistance(PC_1.blue1_2, car1Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_1 = 1;
                    }
                }
                // if (PC_1.blue2 != nothing) {
                //     dis_car = getDistance(PC_1.blue2,   car1Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_1 = 1;
                //     }
                // }
                // if (PC_1.blue2_2 != nothing) {
                //     dis_car = getDistance(PC_1.blue2_2, car1Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_1 = 1;
                //     }
                // }

                // 若该距离还大于某个阈值, 则判断为目前哨岗的数据没有一个可以匹配该车辆数据的
                if (min_dis_car > pangolinDistanceThreshold)
                {
                    Message::pangolinIs_1 = 0;
                }
                // 当car1的队友不存在了, 且自己并不是卧底时, 可以有效怀疑死去的队友是卧底
                if (Message::pangolinIs_1 == 0 && car1Info.hasAlly == false) {
                    Message::pangolinIs_2 = 1;
                }
            }
            if (receive_car2 == true) {
                float min_dis_car = getDistance(cv::Point2f(808+10, 448+10), nothing);
                float dis_car;

                // if (PC_1.blue1 != nothing) {
                //     dis_car = getDistance(PC_1.blue1,   car2Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_2 = 1;
                //     }
                // }
                // if (PC_1.blue1_2 != nothing) {
                //     dis_car = getDistance(PC_1.blue1_2, car2Info.carPosition);
                //     if (dis_car < min_dis_car) {
                //         min_dis_car = dis_car;
                //         Message::pangolinIs_2 = 1;
                //     }
                // }
                if (PC_1.blue2 != nothing) {
                    dis_car = getDistance(PC_1.blue2,   car2Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_2 = 1;
                    }
                }
                if (PC_1.blue2_2 != nothing) {
                    dis_car = getDistance(PC_1.blue2_2, car2Info.carPosition);
                    if (dis_car < min_dis_car) {
                        min_dis_car = dis_car;
                        Message::pangolinIs_2 = 1;
                    }
                }

                // 若该距离还大于某个阈值, 则判断为目前哨岗的数据没有一个可以匹配该车辆数据的
                if (min_dis_car > pangolinDistanceThreshold)
                {
                    Message::pangolinIs_2 = 0;
                }
                // 当car2的队友不存在了, 且自己并不是卧底时, 可以有效怀疑死去的队友是卧底
                if (Message::pangolinIs_2 == 0 && car2Info.hasAlly == false) {
                    Message::pangolinIs_1 = 1;
                }
            }
        }

        // 经过多帧连续的卧底判断, 若达到阈值帧数, 则完成判断
        if ( (Message::pangolinIs_1Frames < pangolinFramesThreshold) &&
            (Message::pangolinIs_2Frames < pangolinFramesThreshold))  {
            // 
            if (Message::pangolinIs_1 == 1)     { Message::pangolinIs_1Frames+=1; Message::pangolinIs_1Frames=relu(Message::pangolinIs_1Frames); }  // 当前帧 判断 卧底是1
            else if (Message::pangolinIs_1 == 0){ Message::pangolinIs_1Frames-=2; Message::pangolinIs_1Frames=relu(Message::pangolinIs_1Frames); }  // 当前帧 判断 卧底不是1
            // 
            if (Message::pangolinIs_2 == 1)     { Message::pangolinIs_2Frames+=1; Message::pangolinIs_2Frames=relu(Message::pangolinIs_2Frames); }  // 当前帧 判断 卧底是1
            else if (Message::pangolinIs_2 == 0){ Message::pangolinIs_2Frames-=2; Message::pangolinIs_2Frames=relu(Message::pangolinIs_2Frames); }  // 当前帧 判断 卧底不是1
        }
    }
    // 判断卧底
    if (Message::pangolinIs_1Frames >= pangolinFramesThreshold)      { PC_1.pangolin = 1; } // 卧底是1
    else if (Message::pangolinIs_2Frames >= pangolinFramesThreshold) { PC_1.pangolin = 2; } // 卧底是2


    /* 在交换颜色为同色同号的情况下, 需要把确保敌方的数据在[blue1][blue2][red1][red2], 而不是在[blue1_2][blue2_2][red1_2][red2_2]
        需要借助Car1,Car2的坐标数据
            
    */
    // 判断为同色同号的情况下, 保证能找到已检测到的敌方坐标, 若未检测到,则给左右的坐标信息置空
    if (PC_1.swapColorModes == 1) {
        // 收到car1的坐标信息
        if (receive_car1 == true) {
            sameColorCheckPoint(PC_1.blue1, PC_1.blue1_2, car1Info.carPosition);
            sameColorCheckPoint(PC_1.red1,  PC_1.red1_2,  car1Info.carPosition);
        }
        // 未收到car1的坐标信息
        else {
            // 所有有关于1车牌号的数据置空
            PC_1.blue1.x=-1,PC_1.blue1.y=-1;
            PC_1.blue1_2.x=-1,PC_1.blue1_2.y=-1;
            PC_1.red1.x=-1, PC_1.red1.y=-1;
            PC_1.red1_2.x=-1, PC_1.red1_2.y=-1;
        }

        // 收到car2的坐标信息
        if (receive_car2 == true) {
            sameColorCheckPoint(PC_1.blue2, PC_1.blue2_2, car2Info.carPosition);
            sameColorCheckPoint(PC_1.red2,  PC_1.red2_2,  car2Info.carPosition);
        }
        // 未收到car2的坐标信息
        else {
            // 所有有关于2车牌号的数据置空
            PC_1.blue2.x=-1,PC_1.blue2.y=-1;
            PC_1.red2.x=-1, PC_1.red2.y=-1;
            PC_1.blue2_2.x=-1,PC_1.blue2_2.y=-1;
            PC_1.red2_2.x=-1, PC_1.red2_2.y=-1;
        }
    }


    return this->PC_1;
}

#endif  // _MESSAGE_HPP_