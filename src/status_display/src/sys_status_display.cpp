#include <QApplication>
#include <QLabel>
#include <QString>

#include <rclcpp/rclcpp.hpp>
#include <status_interfaces/msg/system_status.hpp>

#include <sstream>
#include <thread>

using status_interfaces::msg::SystemStatus;

class SysStatusDisplay : public rclcpp::Node {
private:
  /* data */
  rclcpp::Subscription<SystemStatus>::SharedPtr subscriber_;
  QLabel *label_;

public:
  SysStatusDisplay() : Node("sys_status_display") {
    label_ = new QLabel();
    subscriber_ = this->create_subscription<SystemStatus>(
        "sys_status", 10, [this](const SystemStatus::SharedPtr msg) -> void {
          label_->setText(get_qstr_from_msg(msg));
        });
    label_->setText(get_qstr_from_msg(std::make_shared<SystemStatus>()));
    label_->show();
  }

  QString get_qstr_from_msg(const SystemStatus::SharedPtr msg) {
    std::stringstream show_str;
    show_str << "============ 系统状态可视化显示工具 ============\n"
             << "数 据 时 间:\t" << msg->stamp.sec << "\ts\n"
             << "主 机 名 字:\t" << msg->host_name << "\t\n"
             << "CPU 使用率:\t" << msg->cpu_percent << "\t%\n"
             << "内存使用率:\t" << msg->memory_percent << "\t%\n"
             << "内存总大小:\t" << msg->memory_total << "\tMB\n"
             << "剩余有效内存:\t" << msg->memory_available << "\tMB\n"
             << "网络发送量:\t" << msg->net_sent << "\tMB\n"
             << "网络接收量:\t" << msg->net_recv << "\tMB\n"
             << "================================================\n";

    return QString::fromStdString(show_str.str());
  }
};

int main(int argc, char *argv[])
{
  // QApplication 必须先于 rclcpp::init，否则 --ros-args 这类参数会被 Qt 当成自己的选项
  QApplication app(argc, argv);
  rclcpp::init(argc, argv);

  auto node = std::make_shared<SysStatusDisplay>();

  // ROS 2 的 spin 会阻塞，所以丢到子线程里跑，主线程留给 Qt 事件循环
  std::thread spin_thread([node]() -> void { rclcpp::spin(node); });

  app.exec();

  rclcpp::shutdown();
  spin_thread.join();
  return 0;
}
