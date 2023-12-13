#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "autonoma_msgs/msg/vehicle_inputs.hpp"

class Joystick : public rclcpp::Node{
  public:
    Joystick()
    : Node("gamepad_controller_cpp_node"),
      gear_position(1),
      shiftup_pressed(false),
      shiftdown_pressed(false)
    {
      subscription_ = create_subscription<sensor_msgs::msg::Joy>("joy", 10, std::bind(&Joystick::callback, this, std::placeholders::_1));
      publisher_ = create_publisher<autonoma_msgs::msg::VehicleInputs>("vehicle_inputs", 10);
      load_parameters();
    }

  private:
    void callback(const sensor_msgs::msg::Joy::SharedPtr msg){
      auto cmd = process_message(*msg);
      publisher_->publish(cmd);
    }
    autonoma_msgs::msg::VehicleInputs process_message(const sensor_msgs::msg::Joy& msg){
      autonoma_msgs::msg::VehicleInputs cmd;
      if(!throttle_inverse) cmd.throttle_cmd = (msg.axes[throttle_axis_index] + 1.0) * 50.0;
      else cmd.throttle_cmd = (-1.0 * msg.axes[throttle_axis_index] + 1.0) * 50.0;
      if(!brake_inverse) cmd.brake_cmd = (msg.axes[brake_axis_index] + 1.0) * 3000.0;
      else cmd.brake_cmd = (-1.0 * msg.axes[brake_axis_index] + 1.0) * 3000.0;
      cmd.steering_cmd = msg.axes[steering_axis_index] * 450.0;
      if(msg.buttons[shiftup_button_index] == 1 && gear_position < 6 && !shiftup_pressed){
        gear_position = gear_position + 1;
        shiftup_pressed = true;
      }
      if(msg.buttons[shiftdown_button_index] == 1 && gear_position > 1 && !shiftdown_pressed){
        gear_position = gear_position - 1;
        shiftdown_pressed = true;
      }
      if(msg.buttons[shiftup_button_index] == 0) shiftup_pressed = false;
      if(msg.buttons[shiftdown_button_index] == 0) shiftdown_pressed = false;
      cmd.gear_cmd = gear_position;
      return cmd;
    }
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    rclcpp::Publisher<autonoma_msgs::msg::VehicleInputs>::SharedPtr publisher_;
    int gear_position;
    bool shiftup_pressed;
    bool shiftdown_pressed;
    int throttle_axis_index;
    int brake_axis_index;
    int steering_axis_index;
    int shiftup_button_index;
    int shiftdown_button_index;
    bool throttle_inverse;
    bool brake_inverse;
    void load_parameters(){
      declare_parameter("throttle_axis_index", 2);
      declare_parameter("brake_axis_index", 1);
      declare_parameter("steering_axis_index", 0);
      declare_parameter("shiftup_button_index", 1);
      declare_parameter("shiftdown_button_index", 0);
      declare_parameter("throttle_inverse", false);
      declare_parameter("brake_inverse", false);
      throttle_axis_index = get_parameter("throttle_axis_index").as_int();
      brake_axis_index = get_parameter("brake_axis_index").as_int();
      steering_axis_index = get_parameter("steering_axis_index").as_int();
      shiftup_button_index = get_parameter("shiftup_button_index").as_int();
      shiftdown_button_index = get_parameter("shiftdown_button_index").as_int();
      throttle_inverse = get_parameter("throttle_inverse").as_bool();
      brake_inverse = get_parameter("brake_inverse").as_bool();
    }
};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Joystick>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
