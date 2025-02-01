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
            MotorController& left_spinner_controller,
            MotorController& right_spinner_controller,
            MotorController& drive_motor_controller,
            PS2_ControllerInterface& ps2_controller,
            AccelGyroController& accel_gyro_controller
        ) : 
            led(led_controller),
            drive_motor(drive_motor_controller),
            left_spinner(left_spinner_controller),
            right_spinner(right_spinner_controller),
            ps2_controller(ps2_controller),
            accel_gyro(accel_gyro_controller)
        {
            drive_motor_input_speed_top = MotorController::INPUT_SPEED_TOP;
            drive_motor_input_speed_bottom = MotorController::INPUT_SPEED_BOTTOM;
            spinner_input_speed_top = MotorController::INPUT_SPEED_TOP;
            spinner_input_speed_bottom = MotorController::INPUT_SPEED_BOTTOM;
        };

        void begin(){
            led.begin();
            left_spinner.begin();
            right_spinner.begin();
            drive_motor.begin();
            // drive_motor.drive(100);
            // delay(2000);
            // drive_motor.drive(-100);
            // delay(2000);
            // drive_motor.drive(0);
            // delay(2000);
        };

        void stop(){
            led.off();
            left_spinner.stop();
            right_spinner.stop();
            drive_motor.stop();
        };

        void drive(){
            led.on();
            int LX = ps2_controller.get_LX(), LY = ps2_controller.get_LY();
            // Serial.printf("LX = %d\t LY = %d\n", LX, LY);
            forward = map(ps2_controller.get_LY(), 254, 0, MotorController::INPUT_SPEED_BOTTOM, MotorController::INPUT_SPEED_TOP);
            right = map(ps2_controller.get_LX(), 0, 254, -256, 256); //mapping from 254 instead of 255 as using 255 gives 127.5 as midpoint and resulting rounding error leads to small motor signal at rest.
            // printf("Forward = %d\t Right = %d\n", forward, right);
            drive_motor.drive(forward);
            
            
        }

    private:
        LEDController &led;
        MotorController &left_spinner;
        MotorController &right_spinner;
        MotorController &drive_motor;
        PS2_ControllerInterface &ps2_controller;
        AccelGyroController &accel_gyro;

        int forward = 0;
        int right = 0;
        int spinner_input_speed_top;
        int spinner_input_speed_bottom;
        int drive_motor_input_speed_top;
        int drive_motor_input_speed_bottom;
        int wheel_forward = 0;

};


#endif