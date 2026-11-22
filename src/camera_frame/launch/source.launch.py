from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def start_source(context):
    if LaunchConfiguration('source').perform(context) == 'live':
        return []

    video_path = LaunchConfiguration('video_path').perform(context)
    if not video_path:
        raise RuntimeError('Set video_path to the video file when using source:=video.')

    return [Node(
        package='camera_frame',
        executable='camera_source_node',
        output='screen',
        parameters=[{
            'video_path': ParameterValue(video_path, value_type=str),
            'camera_info_url': ParameterValue(
                LaunchConfiguration('camera_info_url'), value_type=str),
            'loop': ParameterValue(LaunchConfiguration('loop'), value_type=bool),
        }],
    )]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'source', default_value='video', choices=['video', 'live'],
            description='Replay a video or use an existing live camera publisher.'),
        DeclareLaunchArgument(
            'video_path', default_value='',
            description='Path to the video file for replay.'),
        DeclareLaunchArgument(
            'camera_info_url', default_value='',
            description='Optional calibration URL matching the recorded video.'),
        DeclareLaunchArgument(
            'loop', default_value='true', choices=['true', 'false'],
            description='Restart playback from the beginning at the end of the video.'),
        OpaqueFunction(function=start_source),
    ])
