
#include <iostream>
#include "../Core/dolphiimote.h"
#include <array>

void on_data_received(unsigned int wiimote_number, struct dolphiimote_wiimote_data *data, void *userdata)
{
  if(data->button_state & dolphiimote_BUTTON_A)
  {
    std::cout << "A ";
    dolphiimote_brief_rumble(wiimote_number);
  }
  if(data->button_state & dolphiimote_BUTTON_B)
    std::cout << "B ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_DOWN)
    std::cout << "Down ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_RIGHT)
    std::cout << "Right ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_LEFT)
    std::cout << "Left ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_UP)
    std::cout << "Up ";
  if(data -> button_state & dolphiimote_BUTTON_MINUS)
    std::cout << "Minus ";
  if(data -> button_state & dolphiimote_BUTTON_PLUS)
    std::cout << "Plus ";
  if(data -> button_state & dolphiimote_BUTTON_HOME)
    std::cout << "Home ";
  if(data -> button_state & dolphiimote_BUTTON_ONE)
    std::cout << "One ";
  if(data -> button_state & dolphiimote_BUTTON_TWO)
    std::cout << "Two ";

  //if(data->valid_data_flags & dolphiimote_ACCELERATION_VALID)
  //{
  //  std::cout << to_hex((int)data->acceleration.x);
  //  std::cout << to_hex((int)data->acceleration.y);
  //  std::cout << to_hex((int)data->acceleration.z) << std::endl;
  //}

  if(data->valid_data_flags & dolphiimote_MOTIONPLUS_VALID)
  {
    std::cout << to_hex((int)data->motionplus.yaw_down_speed);
    std::cout << to_hex((int)data->motionplus.yaw_down_speed);
    std::cout << to_hex((int)data->motionplus.yaw_down_speed) << std::endl;
  }
}

int main()
{
  int num_wiimotes = dolphiimote_init(on_data_received, NULL);

  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    dolphiimote_update();
  }
}