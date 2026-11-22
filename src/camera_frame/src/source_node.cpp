#include "source_node.hpp"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <camera_info_manager/camera_info_manager.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <std_msgs/msg/header.hpp>

namespace camera_frame
{
class SourceNode : public rclcpp::Node
{
public:
  explicit SourceNode(const rclcpp::NodeOptions &options) : Node("camera_frame", options)
  {
    const auto path = declare_parameter<std::string>("video_path", "");
    const auto calibration_url = declare_parameter<std::string>("camera_info_url", "");
    loop_ = declare_parameter<bool>("loop", true);

    if (path.empty())
    {
      throw std::runtime_error("Set video_path to the video file.");
    }
    if (!std::filesystem::is_regular_file(path) || !capture_.open(path) ||
        !capture_.read(frame_))
    {
      throw std::runtime_error("Could not open or decode video: " + path);
    }

    if (!calibration_url.empty())
    {
      camera_info_manager::CameraInfoManager info_manager(this, "video_camera");
      if (!info_manager.loadCameraInfo(calibration_url) || !info_manager.isCalibrated())
      {
        throw std::runtime_error("Could not load camera calibration: " + calibration_url);
      }
      camera_info_ = info_manager.getCameraInfo();
      calibrated_ = true;
      validate_frame_size(frame_);
    }
    else
    {
      RCLCPP_INFO(get_logger(), "Video has no calibration. CameraInfo K is zero.");
    }

    double fps = capture_.get(cv::CAP_PROP_FPS);
    if (!std::isfinite(fps) || fps < 0.001 || fps > 1000.0)
    {
      fps = 30.0;
      RCLCPP_WARN(get_logger(), "Video frame rate is unavailable or invalid. Replaying at 30 FPS.");
    }
    RCLCPP_INFO(
      get_logger(), "Replaying %s (%d x %d at %.2f FPS)", path.c_str(), frame_.cols,
      frame_.rows, fps);

    const auto qos = rclcpp::SensorDataQoS().keep_last(1);
    image_pub_ = create_publisher<sensor_msgs::msg::Image>("/camera/front/capture", qos);
    info_pub_ = create_publisher<sensor_msgs::msg::CameraInfo>("/camera/front/camera_info", qos);
    const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(1.0 / fps));
    timer_ = create_wall_timer(period, [this]() {
      try
      {
        publish_frame();
      }
      catch (const std::exception &error)
      {
        timer_->cancel();
        RCLCPP_ERROR(get_logger(), "Playback stopped: %s", error.what());
      }
    });
  }

private:
  void validate_frame_size(const cv::Mat &frame) const
  {
    if (calibrated_ &&
        (camera_info_.width != static_cast<unsigned int>(frame.cols) ||
         camera_info_.height != static_cast<unsigned int>(frame.rows)))
    {
      throw std::runtime_error(
        "Calibration dimensions " + std::to_string(camera_info_.width) + "x" +
        std::to_string(camera_info_.height) + " do not match video frame " +
        std::to_string(frame.cols) + "x" + std::to_string(frame.rows) + ".");
    }
  }

  void publish_frame()
  {
    if (first_frame_)
    {
      first_frame_ = false;
    }
    else if (!capture_.read(frame_))
    {
      if (!loop_)
      {
        timer_->cancel();
        RCLCPP_INFO(get_logger(), "Playback finished.");
        return;
      }
      if (!capture_.set(cv::CAP_PROP_POS_FRAMES, 0) || !capture_.read(frame_))
      {
        throw std::runtime_error("Could not restart playback.");
      }
    }

    validate_frame_size(frame_);
    cv::Mat bgr = frame_;
    if (frame_.type() == CV_8UC1)
    {
      cv::cvtColor(frame_, bgr, cv::COLOR_GRAY2BGR);
    }
    else if (frame_.type() == CV_8UC4)
    {
      cv::cvtColor(frame_, bgr, cv::COLOR_BGRA2BGR);
    }
    else if (frame_.type() != CV_8UC3)
    {
      throw std::runtime_error("Video frame cannot be published as bgr8.");
    }

    std_msgs::msg::Header header;
    header.stamp = now();
    header.frame_id = "camera_frame";

    auto info = camera_info_;
    info.header = header;
    info.width = static_cast<unsigned int>(frame_.cols);
    info.height = static_cast<unsigned int>(frame_.rows);

    auto image = std::make_unique<sensor_msgs::msg::Image>();
    cv_bridge::CvImage(header, "bgr8", bgr).toImageMsg(*image);

    info_pub_->publish(info);
    image_pub_->publish(std::move(image));
  }

  cv::VideoCapture capture_;
  cv::Mat frame_;
  bool first_frame_ = true;
  bool loop_ = true;
  bool calibrated_ = false;
  sensor_msgs::msg::CameraInfo camera_info_{};
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr info_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

std::shared_ptr<rclcpp::Node> make_source_node(const rclcpp::NodeOptions &options)
{
  return std::make_shared<SourceNode>(options);
}
}  // namespace camera_frame

RCLCPP_COMPONENTS_REGISTER_NODE(camera_frame::SourceNode)
