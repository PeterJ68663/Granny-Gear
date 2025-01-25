#ifndef _LED_CONTROLLER_
#define _LED_CONTROLLER_

#include <Arduino.h>


class LEDController {

    public:
        LEDController(int pin){
            _pin = pin;
        };
        void begin(){
            pinMode(_pin, OUTPUT);
            digitalWrite(_pin, LOW);
            _on = false;
        }
        void on(){
            digitalWrite(_pin, HIGH);
            _on = true;
        };
        void off(){
            digitalWrite(_pin, LOW);
            _on = false;
        };
        bool is_on(){
            return _on;
        };

    private:
        int _pin;
        bool _on;
};

#endif
