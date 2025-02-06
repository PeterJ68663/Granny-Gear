#ifndef _MOTOR_CONTROLLER_
#define _MOTOR_CONTROLLER_

#include <Servo.h>


class MotorController {

    public:
        static const int INPUT_SPEED_TOP = 100;
        static const int INPUT_SPEED_BOTTOM = -100;

        MotorController(int esc_pin, int servo_min, int servo_mid, int servo_max){
            _esc_pin = esc_pin;
            _servo_min = servo_min;
            _servo_mid = servo_mid;
            _servo_max = servo_max;
        };
        void begin(){
            _pwm_writer.attach(_esc_pin);
            _pwm_writer.writeMicroseconds(_servo_mid);
        }
        void drive(int speed){
            _speed = constrain(speed, INPUT_SPEED_BOTTOM, INPUT_SPEED_TOP);
            int servo_speed = map(speed, INPUT_SPEED_BOTTOM, INPUT_SPEED_TOP, _servo_min, _servo_max);
            Serial.printf("Drive called with %d;\tWriting %d to ESC on pin\t%d\n", speed, servo_speed, _esc_pin);
            _pwm_writer.writeMicroseconds(servo_speed);
        };
        void stop(){
            _pwm_writer.writeMicroseconds(_servo_mid);
        };
        int get_speed(){
            return _speed;
        };

    private:
        int _servo_min;
        int _servo_mid;
        int _servo_max;
        int _speed = 0;
        int _esc_pin;
        Servo _pwm_writer;
};

#endif