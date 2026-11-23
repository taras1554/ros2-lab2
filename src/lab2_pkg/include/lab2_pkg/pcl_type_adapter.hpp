// Завдання 2.1 (п.7*): TypeAdapter між pcl::PointCloud<pcl::PointXYZ> та sensor_msgs/PointCloud2
#ifndef LAB2_PKG__PCL_TYPE_ADAPTER_HPP_
#define LAB2_PKG__PCL_TYPE_ADAPTER_HPP_

#include <atomic>
#include <type_traits>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/type_adapter.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

namespace lab2_ns
{
// Лічильники реальних перетворень (щоб довести, що в intra-process їх немає)
inline std::atomic<int> g_to_ros_count{0};
inline std::atomic<int> g_to_custom_count{0};
}  // namespace lab2_ns

template<>
struct rclcpp::TypeAdapter<pcl::PointCloud<pcl::PointXYZ>, sensor_msgs::msg::PointCloud2>
{
  using is_specialized = std::true_type;
  using custom_type = pcl::PointCloud<pcl::PointXYZ>;
  using ros_message_type = sensor_msgs::msg::PointCloud2;

  static void convert_to_ros_message(const custom_type & source, ros_message_type & destination)
  {
    int n = ++lab2_ns::g_to_ros_count;
    RCLCPP_WARN(
      rclcpp::get_logger("pcl_type_adapter"),
      "[ПЕРЕТВОРЕННЯ #%d] PCL -> PointCloud2 (%zu точок)", n, source.size());
    pcl::toROSMsg(source, destination);
  }

  static void convert_to_custom(const ros_message_type & source, custom_type & destination)
  {
    int n = ++lab2_ns::g_to_custom_count;
    RCLCPP_WARN(
      rclcpp::get_logger("pcl_type_adapter"),
      "[ПЕРЕТВОРЕННЯ #%d] PointCloud2 -> PCL", n);
    pcl::fromROSMsg(source, destination);
  }
};

namespace lab2_ns
{
using PclAdapted =
  rclcpp::TypeAdapter<pcl::PointCloud<pcl::PointXYZ>, sensor_msgs::msg::PointCloud2>;
}

#endif  // LAB2_PKG__PCL_TYPE_ADAPTER_HPP_
