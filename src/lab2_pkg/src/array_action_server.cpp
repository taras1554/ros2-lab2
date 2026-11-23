// Завдання 2.1 (п.6): Action-сервер ProcessArray
//  - виконання у окремому потоці
//  - періодичний feedback (state = PROCESSING / SUCCEEDED)
//  - підтримка скасування цілі
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab2_pkg/action/process_array.hpp"

using namespace std::chrono_literals;
using ProcessArray = lab2_pkg::action::ProcessArray;
using GoalHandle = rclcpp_action::ServerGoalHandle<ProcessArray>;

class ArrayActionServer : public rclcpp::Node
{
public:
  ArrayActionServer() : Node("array_action_server")
  {
    using std::placeholders::_1;
    using std::placeholders::_2;
    server_ = rclcpp_action::create_server<ProcessArray>(
      this, "process_array",
      std::bind(&ArrayActionServer::handle_goal, this, _1, _2),
      std::bind(&ArrayActionServer::handle_cancel, this, _1),
      std::bind(&ArrayActionServer::handle_accepted, this, _1));
    RCLCPP_INFO(get_logger(), "Action-сервер /process_array готовий");
  }

private:
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &, std::shared_ptr<const ProcessArray::Goal> goal)
  {
    if (goal->data.empty()) {
      RCLCPP_WARN(get_logger(), "Порожній масив -> ціль відхилено");
      return rclcpp_action::GoalResponse::REJECT;
    }
    RCLCPP_INFO(get_logger(), "Прийнято ціль: %zu елементів", goal->data.size());
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle>)
  {
    RCLCPP_INFO(get_logger(), "Отримано запит на скасування");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    // Не блокуємо executor: виконання йде в окремому потоці
    std::thread{std::bind(&ArrayActionServer::execute, this, std::placeholders::_1), goal_handle}
    .detach();
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<ProcessArray::Feedback>();
    auto result = std::make_shared<ProcessArray::Result>();
    const size_t total = goal->data.size();

    rclcpp::Rate rate(2.0);  // імітація важкої обробки: 2 елементи/с
    for (size_t i = 0; i < total && rclcpp::ok(); ++i) {
      if (goal_handle->is_canceling()) {
        result->succeeded = false;
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Ціль скасовано на елементі %zu", i);
        return;
      }
      result->processed.push_back(goal->data[i] * goal->data[i]);

      feedback->processed_count = static_cast<uint32_t>(i + 1);
      feedback->progress_percent = 100.0f * static_cast<float>(i + 1) / static_cast<float>(total);
      feedback->state = "PROCESSING";
      goal_handle->publish_feedback(feedback);
      rate.sleep();
    }

    if (rclcpp::ok()) {
      // Останній feedback: досягнуто стану SUCCEEDED
      feedback->state = "SUCCEEDED";
      goal_handle->publish_feedback(feedback);
      result->succeeded = true;
      goal_handle->succeed(result);
      RCLCPP_INFO(get_logger(), "Ціль виконано (SUCCEEDED)");
    }
  }

  rclcpp_action::Server<ProcessArray>::SharedPtr server_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ArrayActionServer>());
  rclcpp::shutdown();
  return 0;
}
