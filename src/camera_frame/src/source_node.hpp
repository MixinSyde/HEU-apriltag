#ifndef CAMERA_FRAME_SOURCE_NODE_HPP_
#define CAMERA_FRAME_SOURCE_NODE_HPP_

#include <memory>

#include <rclcpp/rclcpp.hpp>

namespace camera_frame
{
std::shared_ptr<rclcpp::Node> make_source_node(const rclcpp::NodeOptions &options);
}  // namespace camera_frame

#endif  // CAMERA_FRAME_SOURCE_NODE_HPP_
