## 流程介绍
### 1 相机/视频 获取图片帧

---
### 2 手点四点获得透视变换矩阵

---
### 3 Yolov5-v6.0模型对图片进行目标识别检测

---
### 4 透视变换矩阵和误差矫正方法获取全场车辆定位

---
### 5 主哨岗、副哨岗、车1、车2 信息整合

---
### 6 ZeroMQ库--车辆与哨岗之间的无线消息传输

---
### 7 可视化

---

## 环境
### 硬件
- 有Nvidia显卡的电脑，笔记本 / 台式机 皆可
- MV-SUA134GC-T 工业相机
- MV-LD-4-4M-G 广角镜头
### 软件
- Ubuntu20.04
- OpenCV4
- CUDA 11.3
- cuDNN 8.2
- TensorRT 8.0


## 使用说明
### 主哨岗
```shell
cd v6.0
mkdir build
cd build
cmake ..
make
./main
```

## 其他院校哨岗开源供参考
[2020 中科院](https://github.com/DRL-CASIA/RMAI2020-Perception)  
[2020 哈工大](https://github.com/MengXiangBo/ICRA2020_RM_IHiter_Perception)  
[2020 西北工业](https://github.com/nwpu-v5-team/ICRA-RoboMaster-2020-Perception)  
[2020 吉大](https://github.com/Junking1/ICRA2020-JLU-TARS_GO-Perception)  
[2022 哈工大深圳](https://github.com/Critical-HIT-hitsz/RMUA2022)  
[2022 武技大(貌似)](https://github.com/chinaheyu/whistle)  