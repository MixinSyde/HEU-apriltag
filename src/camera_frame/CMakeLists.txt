cmake_minimum_required(VERSION 3.16)
project(camera_frame)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(camera_info_manager REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)

add_library(camera_source SHARED src/source_node.cpp)
target_compile_features(camera_source PRIVATE cxx_std_20)
target_compile_options(camera_source PRIVATE -Wall -Wextra -Wpedantic)
ament_target_dependencies(camera_source
  rclcpp
  rclcpp_components
  sensor_msgs
  std_msgs
  camera_info_manager
  cv_bridge
)
target_include_directories(camera_source PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(camera_source ${OpenCV_LIBS})
rclcpp_components_register_nodes(camera_source "camera_frame::SourceNode")

add_executable(camera_source_node src/main.cpp)
target_compile_features(camera_source_node PRIVATE cxx_std_20)
ament_target_dependencies(camera_source_node rclcpp)
target_link_libraries(camera_source_node camera_source)

install(TARGETS camera_source LIBRARY DESTINATION lib)
install(TARGETS camera_source_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
