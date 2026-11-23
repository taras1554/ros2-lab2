// Завдання 2.1 (п.1-5): вузол data_filter
//  - підписка на вхідну тему (String) -> верхній регістр -> uppercase_topic
//  - "ледача" підписка через matched_callback видавця
//  - QoS видавця: transient_local, depth = 1, з можливістю перевизначення
//  - сервіс ResetFilter з інтроспекцією (RCL_SERVICE_INTROSPECTION_CONTENTS)
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rcl/service_introspection.h"
#include "std_msgs/msg/string.hpp"
#include "lab2_pkg/srv/reset_filter.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

class DataFilter : public rclcpp::Node
{
public:
  DataFilter() : Node("data_filter")
  {
    input_topic_ = this->declare_parameter<std::string>("input_topic", "input_topic");

    // --- QoS: transient_local, depth = 1 (reliable потрібен для transient_local) ---
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.reliable().transient_local();

    rclcpp::PublisherOptions pub_options;

    // --- Перевизначення Depth / Reliability / Durability через параметри ---
    pub_options.qos_overriding_options = rclcpp::QosOverridingOptions({
      rclcpp::QosPolicyKind::Depth,
      rclcpp::QosPolicyKind::Reliability,
      rclcpp::QosPolicyKind::Durability});

    // --- matched_callback: ледача підписка ---
    pub_options.event_callbacks.matched_callback =
      [this](rclcpp::MatchedInfo & info) {
        RCLCPP_INFO(
          get_logger(), "Matched: поточна кількість слухачів uppercase_topic = %zu",
          static_cast<size_t>(info.current_count));
        if (info.current_count > 0 && !sub_) {
          sub_ = create_subscription<std_msgs::msg::String>(
            input_topic_, 10, std::bind(&DataFilter::on_input, this, _1));
          RCLCPP_INFO(get_logger(), "Є слухачі -> підписку на '%s' СТВОРЕНО", input_topic_.c_str());
        } else if (info.current_count == 0 && sub_) {
          sub_.reset();
          RCLCPP_INFO(get_logger(), "Слухачів немає -> підписку на '%s' ВИМКНЕНО", input_topic_.c_str());
        }
      };

    pub_ = create_publisher<std_msgs::msg::String>("uppercase_topic", qos, pub_options);

    // --- Сервіс ResetFilter + інтроспекція ---
    srv_ = create_service<lab2_pkg::srv::ResetFilter>(
      "reset_filter", std::bind(&DataFilter::on_reset, this, _1, _2));
    srv_->configure_introspection(
      get_clock(), rclcpp::SystemDefaultsQoS(), RCL_SERVICE_INTROSPECTION_CONTENTS);

    RCLCPP_INFO(get_logger(), "data_filter запущено. Очікуємо слухачів на uppercase_topic...");
  }

private:
  void on_input(const std_msgs::msg::String::SharedPtr msg)
  {
    std_msgs::msg::String out;
    out.data = msg->data;
    std::transform(
      out.data.begin(), out.data.end(), out.data.begin(),
      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    ++processed_;
    pub_->publish(out);
  }

  void on_reset(
    const std::shared_ptr<lab2_pkg::srv::ResetFilter::Request> request,
    std::shared_ptr<lab2_pkg::srv::ResetFilter::Response> response)
  {
    response->processed_before_reset = processed_;
    if (request->reset_counter) {
      processed_ = 0;
      response->success = true;
      response->message = "Лічильник скинуто";
    } else {
      response->success = false;
      response->message = "reset_counter = false, нічого не змінено";
    }
    RCLCPP_INFO(get_logger(), "ResetFilter: %s", response->message.c_str());
  }

  std::string input_topic_;
  uint32_t processed_{0};
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
  rclcpp::Service<lab2_pkg::srv::ResetFilter>::SharedPtr srv_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DataFilter>());
  rclcpp::shutdown();
  return 0;
}
