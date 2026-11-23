// Завдання 2.1 (п.5): асинхронний сервісний клієнт з обробкою таймауту
#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rcl/service_introspection.h"
#include "lab2_pkg/srv/reset_filter.hpp"

using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("reset_client");
  auto client = node->create_client<lab2_pkg::srv::ResetFilter>("reset_filter");
  client->configure_introspection(
    node->get_clock(), rclcpp::SystemDefaultsQoS(), RCL_SERVICE_INTROSPECTION_CONTENTS);

  // Окремий потік крутить executor, щоб main міг блокуватись на future.wait_for()
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spinner([&executor]() { executor.spin(); });

  int ret = 0;
  if (!client->wait_for_service(5s)) {
    RCLCPP_ERROR(node->get_logger(), "Сервіс reset_filter недоступний (таймаут 5 с)");
    ret = 1;
  } else {
    auto request = std::make_shared<lab2_pkg::srv::ResetFilter::Request>();
    request->reset_counter = true;

    auto pending = client->async_send_request(request);

    if (pending.future.wait_for(2s) == std::future_status::ready) {
      auto response = pending.future.get();
      RCLCPP_INFO(
        node->get_logger(), "Відповідь: success=%s, оброблено до скидання=%u, повідомлення: %s",
        response->success ? "true" : "false", response->processed_before_reset,
        response->message.c_str());
    } else {
      RCLCPP_ERROR(node->get_logger(), "Таймаут очікування відповіді (2 с)");
      client->remove_pending_request(pending.request_id);
      ret = 2;
    }
  }

  executor.cancel();
  rclcpp::shutdown();
  if (spinner.joinable()) {
    spinner.join();
  }
  return ret;
}
