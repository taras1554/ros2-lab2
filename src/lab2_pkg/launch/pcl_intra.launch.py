# Обидва компоненти в ОДНОМУ контейнері з intra-process -> перетворень типів немає
from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    extra = [{'use_intra_process_comms': True}]
    return LaunchDescription([
        ComposableNodeContainer(
            name='pcl_container',
            namespace='',
            package='rclcpp_components',
            executable='component_container',
            composable_node_descriptions=[
                ComposableNode(package='lab2_pkg', plugin='lab2_ns::PclPublisher',
                               extra_arguments=extra),
                ComposableNode(package='lab2_pkg', plugin='lab2_ns::PclSubscriber',
                               extra_arguments=extra),
            ],
            output='screen',
        )
    ])
