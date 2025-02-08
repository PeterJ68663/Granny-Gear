#ifndef _MOTOR_CONTROLLER_
#define _MOTOR_CONTROLLER_

#include <Servo.h>


class MotorController {

    public:

        MotorController(const int esc_pin, const int servo_min, const int servo_mid, const int servo_max, const int input_range){
            _esc_pin = esc_pin;
            _servo_min = servo_min;
            _servo_mid = servo_mid;
            _servo_max = servo_max;
            _input_min = - input_range / 2;
            _input_max = - _input_min;
            
        };
        void begin(){
            _pwm_writer.attach(_esc_pin);
            _pwm_writer.writeMicroseconds(_servo_mid);

            Serial.begin(115200);
            delay(10);
            Serial.printf("Motor Controller Started at %d\n", millis());
        }
        void drive(int speed){
            _speed = constrain(speed, _input_min, _input_max);
            int servo_speed = map(speed, _input_min, _input_max, _servo_min, _servo_max);
            // Serial.printf("Drive called with %d;\tWriting %d to ESC on pin\t%d\n", speed, servo_speed, _esc_pin);
            _pwm_writer.writeMicroseconds(servo_speed);
        };
        void stop(){
            _pwm_writer.writeMicroseconds(_servo_mid);
        };
        int get_speed(){
            return _speed;
        };

        int get_max_input(){
            return _input_max;
        }

        int get_min_input(){
            return _input_min;
        }


    private:
        int _servo_min;
        int _servo_mid;
        int _servo_max;
        int _input_min;
        int _input_max;
        int _esc_pin;
        int _speed = 0;
        Servo _pwm_writer;
};

#endif