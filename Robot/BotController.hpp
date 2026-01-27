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
            _drive_motor_input_speed_top = drive_motor.get_max_input();
            _drive_motor_input_speed_bottom = drive_motor.get_min_input();
            _spinner_input_speed_top = left_spinner.get_max_input();
            _spinner_input_speed_bottom = left_spinner.get_min_input();
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
            left_spinner.disarm();
            right_spinner.disarm();
        };

        void fight(){
            led.on();
            int LX = ps2_controller.get_LX(), LY = ps2_controller.get_LY();
            int RX = ps2_controller.get_RX(), RY = ps2_controller.get_RY();
            // Serial.printf("LX = %d\tLY = %d\tRX = %d\tRY = %d\n", LX, LY, RX, RY);
            bool up_pressed = ps2_controller.up_state();
            bool down_pressed = ps2_controller.down_state();
            bool left_pressed = ps2_controller.left_state();
            bool right_pressed = ps2_controller.right_state();
            // if (up_pressed) {
            //     _up_count++;
            // }
            // if (down_pressed) {
            //     _down_count++;
            // }
            // if (left_pressed) {
            //     _left_count++;
            // }
            // if (right_pressed) {
            //     _right_count++;
            // }
            // Serial.printf("Up = %d\t Down = %d\t Left = %d\t Right = %d\n", _up_count, _down_count, _left_count, _right_count);
            // Serial.printf("LX = %d\t LY = %d\n", LX, LY);
            // return;

            if (RY < 20) {
                drive_motor.arm();
                left_spinner.arm();
                right_spinner.arm();
            }

            if (left_pressed) {
                inverted = !inverted;
            }

            accel_gyro.read_accelerometer();
            _current_rightwards_acceleration = accel_gyro.get_accel_y() * -1;
            accel_gyro.read_gyroscope();
            Serial.printf("Accel: %d\tGyro: %d\n", accel_gyro.get_accel_y(), accel_gyro.get_gyro_x());
            _current_angular_speed = accel_gyro.get_gyro_x() * -1;
            if (inverted){
                _current_angular_speed *= -1;
                _current_rightwards_acceleration *= -1;
            }

            // _jink(RX, RY);

            drive_wheel(LY);
            int time_since_last_steer = millis() - _time_of_last_steer;
            if (time_since_last_steer >= 10) { //ms
                steer(LX);
                _time_of_last_steer = millis();
            }

            // Handle direct commands for spinner speed:
            if (up_pressed) {
                left_spinner.drive(200);
                right_spinner.drive(-200);
            }
            if (down_pressed) {
                left_spinner.drive(0);
                right_spinner.drive(0);
            }
        }

        void drive_wheel(int LY){
            _forward = map(LY, 0, 254, _drive_motor_input_speed_top, _drive_motor_input_speed_bottom);
            if (inverted) {
                _forward = -_forward;
            }
            drive_motor.drive(_forward);
        }

        void steer(int LX){
            int _desired_angular_speed = map(LX, 0, 254, _fastest_angular_speed, -_fastest_angular_speed); // Degrees per second; Clockwise. //mapping from 254 instead of 255 as using 255 gives 127.5 as midpoint and resulting rounding error leads to small motor signal at rest.
            if (abs(_desired_angular_speed) <= 1) {
                _desired_angular_speed = 0;
            }
            // Serial.printf("Current Angular Speed: %d\n", _current_angular_speed);
            int P = _desired_angular_speed - _current_angular_speed;
            int D = _current_rightwards_acceleration;
            Serial.printf("D: %d\tP: %d\n", D, P);
            float eps_p = 0.005; // 0.05 for single spinner steering.
            float eps_d = 0;

            int _speed_change = (eps_p * P) + (eps_d * D);
            if (inverted){
                _speed_change *= -1;  // Because the motors are upside down, the effect of the speed change is flipped.
            }

            // Serial.printf("LX %d\tSpeed change right = %d\n", LX, _speed_change);
            right_spinner.drive(right_spinner.get_speed() + _speed_change);
            left_spinner.drive(left_spinner.get_speed() + _speed_change);
            // Serial.printf("right speed %d\t left speed %d\t", right_spinner.get_speed(), left_spinner.get_speed());

            // float turn_per_ms = (float)_desired_angular_speed * _turning_sensitivity;
            // int current_spinner_speed = left_spinner.get_speed();
            // int time_since_last_spinner_update = millis() - _time_of_last_spinner_update;
            // int spinner_increase = turn_per_ms * time_since_last_spinner_update;
            // int new_spinner_speed = current_spinner_speed + spinner_increase;
            // Serial.printf("LX %d\tRight %d\t turning_sensitivity %f\t right min %d\t right max %d\t\n", LX, _desired_angular_speed, _turning_sensitivity, _desired_angular_speed_min, _desired_angular_speed_max);
            // Serial.printf("Time elapsed %d\t turn per ms %f\tspinner_incrase %d\tnew_spinner_speed %d\n", time_since_last_spinner_update, turn_per_ms, spinner_increase, new_spinner_speed);
            // _time_of_last_spinner_update = millis();



            // if (_desired_angular_speed > 50) {
            //     Serial.println("Steering Right"); 
            //     left_spinner.drive(left_spinner.get_speed() - 1);
            //     right_spinner.drive(right_spinner.get_speed() - 1);
            // }
            // else if (_desired_angular_speed < -50) {
            //     Serial.println("Steering Left");
            //     left_spinner.drive(left_spinner.get_speed() + 1);
            //     right_spinner.drive(right_spinner.get_speed() + 1);
            // }
            // else {
            //     Serial.println("Not Steering");
            //     left_spinner.stop();
            //     right_spinner.stop();
            // }
        }

    private:
        LEDController &led;
        MotorController &left_spinner;
        MotorController &right_spinner;
        MotorController &drive_motor;
        PS2_ControllerInterface &ps2_controller;
        AccelGyroController &accel_gyro;

        int _forward = 0;
        int _spinner_input_speed_top;
        int _spinner_input_speed_bottom;
        int _drive_motor_input_speed_top;
        int _drive_motor_input_speed_bottom;
        int _wheel_forward = 0;
        int _time_of_last_steer = 0;  //ms
        int _current_rightwards_acceleration = 0;
        int _current_angular_speed = 0; // Degrees per second.
        int _fastest_angular_speed = 1000; // Degrees per second.
        int _time_of_last_jink = 0;  //ms
        // int _time_of_last_spinner_update = 0; //ms
        // const float _turning_sensitivity = 0.01;
        // const int _right_min = -100;
        // const int _right_max = 100;
        int _up_count = 0, _down_count = 0, _left_count = 0, _right_count = 0;
        bool inverted = false;

        void _jink(int RX, int RY){
            int jink_rate = 40;
            int time_since_last_jink = millis() - _time_of_last_jink;
            if (time_since_last_jink >= 100) { //ms
                _time_of_last_jink = millis();
                if (RX > 180 && RY < 60){
                    // Serial.println("Top Right");
                    right_spinner.drive(right_spinner.get_speed() - jink_rate);
                }
                else if (RX > 180 && RY > 180){
                    // Serial.println("Bottom Right");
                    right_spinner.drive(right_spinner.get_speed() + jink_rate);
                }
                else if (RX < 60 && RY < 60){
                    // Serial.println("Top Left");
                    left_spinner.drive(left_spinner.get_speed() - jink_rate);
                }
                else if (RX < 60 && RY > 180){
                    // Serial.println("Bottom Left");
                    left_spinner.drive(left_spinner.get_speed() + jink_rate);
                }
            }
        }

        void _back_emf_test(){
            delay(5000);
            left_spinner.drive(100);
            right_spinner.drive(-100);
            delay(5000);
            left_spinner.drive(0);
            right_spinner.drive(0);
            delay(5000);
            left_spinner.drive(200);
            right_spinner.drive(-200);
            delay(5000);
            left_spinner.drive(0);
            right_spinner.drive(0);
            delay(5000);
            left_spinner.drive(300);
            right_spinner.drive(-300);
            delay(5000);
            left_spinner.drive(0);
            right_spinner.drive(0);
            delay(5000);
            left_spinner.drive(400);
            right_spinner.drive(-400);
            delay(5000);
            left_spinner.drive(0);
            right_spinner.drive(0);
        }
};


#endif