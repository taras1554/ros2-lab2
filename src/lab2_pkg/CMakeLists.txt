cmake_minimum_required(VERSION 3.8)
project(lab2_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED COMPONENTS common)
find_package(rosidl_default_generators REQUIRED)

# --- Власні інтерфейси: сервіс та дія ---
rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/ResetFilter.srv"
  "action/ProcessArray.action"
)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
ament_export_dependencies(rosidl_default_runtime)

# --- Завдання 2.1 (п.1-5): вузол data_filter ---
add_executable(data_filter src/data_filter.cpp)
target_link_libraries(data_filter PRIVATE
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  "${cpp_typesupport_target}"
)

# --- Завдання 2.1 (п.5): сервісний клієнт ---
add_executable(reset_client src/reset_client.cpp)
target_link_libraries(reset_client PRIVATE
  rclcpp::rclcpp
  "${cpp_typesupport_target}"
)

# --- Завдання 2.1 (п.6): Action-сервер ---
add_executable(array_action_server src/array_action_server.cpp)
target_link_libraries(array_action_server PRIVATE
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  "${cpp_typesupport_target}"
)

# --- Завдання 2.1 (п.7*): компоненти з Type Adaptation ---
add_library(pcl_components SHARED src/pcl_components.cpp)
target_include_directories(pcl_components PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PCL_INCLUDE_DIRS}
)
target_link_libraries(pcl_components PUBLIC
  rclcpp::rclcpp
  rclcpp_components::component
  ${sensor_msgs_TARGETS}
  pcl_conversions::pcl_conversions
  ${PCL_LIBRARIES}
)
rclcpp_components_register_nodes(pcl_components
  "lab2_ns::PclPublisher"
  "lab2_ns::PclSubscriber"
)

install(TARGETS pcl_components
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(TARGETS data_filter reset_client array_action_server
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
