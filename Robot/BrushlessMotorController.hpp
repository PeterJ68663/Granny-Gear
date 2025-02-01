#ifndef _MOTOR_CONTROLLER_
#define _MOTOR_CONTROLLER_

#include <Servo.h>


class MotorController {

    public:
        static const int SPEED_TOP = 600;
        static const int SPEED_BOTTOM = -600;

        MotorController(int esc_pin){
            _esc_pin = esc_pin;
            _forward = 0;
        };
        void begin(){
            _controller.attach(_esc_pin);
            _controller.writeMicroseconds(ZERO_THROTTLE);
        }
        void drive(int speed){
            _forward = speed;
            // Serial.printf("Writing %d to ESC\n", ZERO_THROTTLE + _forward);
            speed = constrain(speed, SPEED_BOTTOM, SPEED_TOP);
            _controller.writeMicroseconds(ZERO_THROTTLE + speed);
        };
        void stop(){
            _controller.writeMicroseconds(ZERO_THROTTLE);
        };

    private:
        static const int MAX_THROTTLE = 2100;
        static const int ZERO_THROTTLE = 1500;
        static const int MIN_THROTTLE = 900;

        int _esc_pin;
        Servo _controller;
        int _forward; //Number between 600 and -600 determining how hard and in what direction to drive the motor.
};

#endif