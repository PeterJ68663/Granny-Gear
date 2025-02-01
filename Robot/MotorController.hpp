#ifndef _MOTOR_CONTROLLER_
#define _MOTOR_CONTROLLER_

#include <Servo.h>


class MotorController {

    public:
        static const int SPEED_TOP = 100;
        static const int SPEED_BOTTOM = -100;

        MotorController(int esc_pin, int servo_min, int servo_mid, int servo_max){
            _esc_pin = esc_pin;
            _servo_min = servo_min;
            _servo_mid = servo_mid;
            _servo_max = servo_max;
            _forward = 0;
        };
        void begin(){
            _controller.attach(_esc_pin);
            _controller.writeMicroseconds(_servo_mid);
        }
        void drive(int speed){
            _forward = speed;
            // Serial.printf("Writing %d to ESC\n", _servo_mid + _forward);
            speed = constrain(speed, SPEED_BOTTOM, SPEED_TOP);
            _controller.writeMicroseconds(_servo_mid + speed);
        };
        void stop(){
            _controller.writeMicroseconds(_servo_mid);
        };

    private:
        int _servo_min;
        int _servo_mid;
        int _servo_max;

        int _esc_pin;
        Servo _controller;
        int _forward; //Number between 600 and -600 determining how hard and in what direction to drive the motor.
};

#endif