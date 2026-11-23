// Завдання 2.1 (п.7*): два компоненти (видавець і підписник) з використанням TypeAdapter
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "lab2_pkg/pcl_type_adapter.hpp"

using namespace std::chrono_literals;

namespace lab2_ns
{

class PclPublisher : public rclcpp::Node
{
public:
  explicit PclPublisher(const rclcpp::NodeOptions & options)
  : Node("pcl_publisher", options)
  {
    pub_ = create_publisher<PclAdapted>("pcl_cloud", 10);
    timer_ = create_wall_timer(1s, [this]() {
      // unique_ptr -> у режимі intra-process передається без копіювання (zero-copy)
      auto cloud = std::make_unique<pcl::PointCloud<pcl::PointXYZ>>();
      for (int i = 0; i < 1000; ++i) {
        cloud->push_back(pcl::PointXYZ(0.01f * i, 0.0f, 0.0f));
      }
      cloud->header.frame_id = "map";
      RCLCPP_INFO(get_logger(), "Публікую хмару з %zu точок", cloud->size());
      pub_->publish(std::move(cloud));
    });
  }

private:
  rclcpp::Publisher<PclAdapted>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

class PclSubscriber : public rclcpp::Node
{
public:
  explicit PclSubscriber(const rclcpp::NodeOptions & options)
  : Node("pcl_subscriber", options)
  {
    // Колбек приймає безпосередньо pcl::PointCloud, а не PointCloud2
    sub_ = create_subscription<PclAdapted>(
      "pcl_cloud", 10,
      [this](const pcl::PointCloud<pcl::PointXYZ> & cloud) {
        RCLCPP_INFO(
          get_logger(), "Отримано хмару з %zu точок (всього перетворень: to_ros=%d, to_pcl=%d)",
          cloud.size(), g_to_ros_count.load(), g_to_custom_count.load());
      });
  }

private:
  rclcpp::Subscription<PclAdapted>::SharedPtr sub_;
};

}  // namespace lab2_ns

RCLCPP_COMPONENTS_REGISTER_NODE(lab2_ns::PclPublisher)
RCLCPP_COMPONENTS_REGISTER_NODE(lab2_ns::PclSubscriber)
