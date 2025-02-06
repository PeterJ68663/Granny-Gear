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

            Serial.begin(115200);
            delay(10);
            Serial.printf("LED Controller Started at %d\n", millis());
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
