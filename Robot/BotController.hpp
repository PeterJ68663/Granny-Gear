#ifndef _BOT_CONTROLLER_
#define _BOT_CONTROLLER_

#include "LEDController.hpp"
#include "MotorController.hpp"
#include "UdpController.hpp"
#include "AccelGyroController.hpp"
#include <Arduino.h>


class BotController{    
    public:
        BotController(
            LEDController& led_controller,
            MotorController& left_wheel_controller,
            MotorController& right_wheel_controller,
            PS2_ControllerInterface& ps2_controller,
            AccelGyroController& accel_gyro_controller
        ) : 
            led(led_controller),
            left_wheel(left_wheel_controller),
            right_wheel(right_wheel_controller),
            ps2_controller(ps2_controller),
            accel_gyro(accel_gyro_controller)
        {
            motor_input_speed_top = MotorController::SPEED_TOP;
            motor_input_speed_bottom = MotorController::SPEED_BOTTOM;
        };

        void begin(){
            led.begin();
            left_wheel.begin();
            right_wheel.begin();
        };

        void stop(){
            led.off();
            left_wheel.stop();
            right_wheel.stop();
        };

        void drive(){
            if (melty_mode){
                Serial.println("Melty mode not yet implemented.");
            }
            else{
                led.on();
                _tank_drive();
            }
            left_wheel.drive(left_wheel_forward);
            right_wheel.drive(right_wheel_forward);
        }

    private:
        LEDController &led;
        MotorController &left_wheel;
        MotorController &right_wheel;
        PS2_ControllerInterface &ps2_controller;
        AccelGyroController &accel_gyro;

        bool melty_mode = false; //If false, tank mode.
        int forward = 0;
        int right = 0;
        int motor_input_speed_top;
        int motor_input_speed_bottom;
        int left_wheel_forward = 0;
        int right_wheel_forward = 0;

        void _tank_drive(){
            int LX = ps2_controller.get_LX(), LY = ps2_controller.get_LY();
            // Serial.printf("LX = %d\t LY = %d\n", LX, LY);
            forward = map(ps2_controller.get_LY(), 254, 0, -1024, 1024);
            right = map(ps2_controller.get_LX(), 0, 254, -256, 256); //mapping from 254 instead of 255 as using 255 gives 127.5 as midpoint and resulting rounding error leads to small motor signal at rest.
            // printf("Forward = %d\t Right = %d\n", forward, right);
            //Mixing:
            left_wheel_forward = constrain(forward + right, -1024, 1024);
            right_wheel_forward = constrain(forward - right, -1024, 1024);
            // Serial.printf("LeftForward = %d\tRightForward = %d\n", left_wheel_forward, right_wheel_forward);
            // right_wheel_forward *= -1; // because I wired the ESC up wrong :P
            left_wheel_forward = map(left_wheel_forward, -1024, 1024, MotorController::SPEED_BOTTOM, MotorController::SPEED_TOP);
            right_wheel_forward = map(right_wheel_forward, -1024, 1024, MotorController::SPEED_BOTTOM, MotorController::SPEED_TOP);
            // Serial.printf("Left Forward: %d\t Right Forward %d\n", left_wheel_forward, right_wheel_forward);
        }
};


#endif