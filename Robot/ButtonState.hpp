#ifndef _BUTTON_STATE_
#define _BUTTON_STATE_

#include <Arduino.h>


class ButtonState{
    /*
    This class is set up to to allow for acting exactly once on a button press.
    When asked for a button state the first time, the class will return the current state as reported by the controller class.
    After a true value is returned, subsequent queries within some cool-down period, it will return false.
    After this period, the current state from the controller class will be returned again.

    When a button is pressed, the controller will continue to send true for that button for X ms.
    This will be read every B seconds by the robot (once per loop).
    So long as the cool-down period is less than X, the robot will only act once on a button press.
    */
    public:
        ButtonState(bool& controller_reported_button_state) : _button_state(controller_reported_button_state) {}
        bool get_button_state(){
            int time_since_true_last_returned = millis() - _time_true_last_returned;
            if (_button_state == true){                    
                if (time_since_true_last_returned > _cool_down_period){
                    _time_true_last_returned = millis();
                    return _button_state;
                }
                else{
                    Serial.println("Button pressed too recently.");
                }
            }
            return false;
        }
    
    private:
        bool& _button_state;
        int _time_true_last_returned = -999;  // ms
        const int _cool_down_period = 40;//26;  // ms
};


#endif