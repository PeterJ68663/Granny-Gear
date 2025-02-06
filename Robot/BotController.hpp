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
            _drive_motor_input_speed_top = MotorController::INPUT_SPEED_TOP;
            _drive_motor_input_speed_bottom = MotorController::INPUT_SPEED_BOTTOM;
            _spinner_input_speed_top = MotorController::INPUT_SPEED_TOP;
            _spinner_input_speed_bottom = MotorController::INPUT_SPEED_BOTTOM;
        };

        void begin(){
            led.begin();
            delay(10);
            left_spinner.begin();
            delay(10);
            right_spinner.begin();
            delay(10);
            drive_motor.begin();

            Serial.begin(115200);
            delay(10);
            Serial.printf("Bot Controller Started at %d\n", millis());

            // One time setup for drive motor speed controller:
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
            const int spinner_full_change_time = 10; //seconds
            const float spinner_rate_of_change = MotorController::INPUT_SPEED_TOP / (spinner_full_change_time * 1000); //units per millisecond
            led.on();
            int LX = ps2_controller.get_LX(), LY = ps2_controller.get_LY();
            // Serial.printf("LX = %d\t LY = %d\n", LX, LY);
            _forward = map(ps2_controller.get_LY(), 254, 0, MotorController::INPUT_SPEED_BOTTOM, MotorController::INPUT_SPEED_TOP);
            _right = map(ps2_controller.get_LX(), 0, 254, -256, 256); //mapping from 254 instead of 255 as using 255 gives 127.5 as midpoint and resulting rounding error leads to small motor signal at rest.
            // printf("Forward = %d\t Right = %d\n", _forward, _right);
            drive_motor.drive(_forward);
            // if (abs(_right) > 10) {
            //     Serial.println("Steering");
                // Steer by changing spinner velocity.
                // int millis_since_spinner_last_changed = millis() - _time_of_last_spinner_change;
                // int change = millis_since_spinner_last_changed * spinner_rate_of_change;
                // if (change > 0) {
                //     if (_right < 0) {
                //         change *= -1;
                //     }
                //     left_spinner.drive(left_spinner.get_speed() - change);
                //     right_spinner.drive(right_spinner.get_speed() - change);
                //     _time_of_last_spinner_change = millis();
                //     }
                // }
            Serial.printf("LX = %d\n", ps2_controller.get_LX());
            Serial.printf("Right = %d\n", _right);
            if (_right > 100) {
                Serial.println("Steering Right"); 
                left_spinner.drive(left_spinner.get_speed() - 1);
                right_spinner.drive(right_spinner.get_speed() - 1);
            }
            else if (_right < -100) {
                Serial.println("Steering Left");
                left_spinner.drive(left_spinner.get_speed() + 1);
                right_spinner.drive(right_spinner.get_speed() + 1);
            }
            else {
                Serial.println("Not Steering");
                left_spinner.stop();
                right_spinner.stop();
            }
            delay(100);
        }

    private:
        LEDController &led;
        MotorController &left_spinner;
        MotorController &right_spinner;
        MotorController &drive_motor;
        PS2_ControllerInterface &ps2_controller;
        AccelGyroController &accel_gyro;

        int _forward = 0;
        int _right = 0;
        int _spinner_input_speed_top;
        int _spinner_input_speed_bottom;
        int _drive_motor_input_speed_top;
        int _drive_motor_input_speed_bottom;
        int _wheel_forward = 0;
        int _time_of_last_spinner_change = 0; //ms
};


#endif