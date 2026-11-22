# 视觉组校内赛 · ChArUco 板识别与位姿解算

## 任务

在 WSL 中使用 ROS2 Humble、C++、OpenCV 和 CMake 编写识别节点。订阅相机图像和内参，识别 ChArUco 板，计算板中心相对于相机的位置和朝向。平时用视频调试，现场用海康相机验收。检测和解算方法自行选择。

- 在调试图上标出标记 ID 和边框，用红、绿、蓝箭头表示板子的 X、Y、Z 轴，显示板中心的 XYZ，单位为米。
- 位姿以板中心为原点，相对于相机光学坐标系 `camera_frame`（X 向右、Y 向下、Z 向前）。
- 建议在板子离开画面或解算失败时保留上一次有效位姿，显示 `STALE` 和经过的秒数。

## 输入与输出

| 方向 | 话题 | 消息类型 |
| --- | --- | --- |
| 订阅图像 | `/camera/front/capture` | `sensor_msgs/msg/Image` |
| 订阅标定 | `/camera/front/camera_info` | `sensor_msgs/msg/CameraInfo` |
| 发布有效位姿 | `/recognition/board_pose` | `geometry_msgs/msg/PoseStamped` |
| 发布调试图 | `/recognition/debug_image` | `sensor_msgs/msg/Image` |

图像编码为 `bgr8`。图像和标定用 Sensor Data QoS 订阅。位姿沿用对应图像的时间戳，仅在本帧解算成功时发布。调试窗口可以关闭，图像话题继续输出。

## 提供的文件

| 路径 | 内容 |
| --- | --- |
| `src/camera_frame/` | 视频回放节点及 launch，按帧发布图像与 `CameraInfo` |
| `config/board.yaml` | `DICT_4X4_50`，6 × 6 方格，格长 150 mm，标记边长 120 mm，ID 为 0–17 |
| `config/DA3113952.yaml` | 海康相机 DA3113952 的 1440 × 1080 标定 |
| `src/interfaces/msg/BoardObservation.msg` | 可选的识别状态消息 |
| `media/` | 测试视频和图片待补。视频由验收相机录制，使用配套标定 |

## 启动与提交

在本目录编译回放和接口包：

```bash
source /opt/ros/humble/setup.bash
colcon build
source install/setup.bash
```

素材提供后，从本目录用配套标定启动回放，将 `test.mp4` 换成实际文件名：

```bash
ros2 launch camera_frame source.launch.py source:=video \
  video_path:="$PWD/media/test.mp4" \
  camera_info_url:="file://$PWD/config/DA3113952.yaml"
```

在自己的 launch 中提供 `source:=video` 和 `source:=live`。视频模式启动回放和识别节点，实车模式直接订阅车端相机话题。两种模式共用一份识别代码。

提交源码、构建文件、launch 和配置。README 写清编译与启动方法，以及板坐标轴的方向。附加项可选 Foxglove 三维展示、组件化通信或发布 `interfaces/msg/BoardObservation`。
