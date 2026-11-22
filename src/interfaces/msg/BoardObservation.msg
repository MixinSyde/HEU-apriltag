# header.stamp: current image stamp; on input timeout use current ROS time.
# header.frame_id: camera_frame
std_msgs/Header header
# IDs detected in the current image; empty on timeout.
int32[] tag_ids
# True only if this image produced a valid pose. False on missing input.
bool pose_valid
# Whether a successful pose has ever been obtained.
bool has_last_pose
# True when no new image arrived within the configured input timeout.
bool input_stale
# Last successful pose and ORIGINAL image timestamp; never refresh when stale.
geometry_msgs/Pose last_pose
builtin_interfaces/Time last_pose_stamp
# Seconds since last_pose_stamp in the ROS time domain; -1 before first pose.
float64 pose_age_sec
