#include "source_node.hpp"

#include <exception>

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  try
  {
    rclcpp::spin(camera_frame::make_source_node(rclcpp::NodeOptions{}));
  }
  catch (const std::exception &error)
  {
    RCLCPP_ERROR(rclcpp::get_logger("camera_frame"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
