#ifndef _UDP_CONTROLLER_
#define _UDP_CONTROLLER_

#include <Arduino.h>
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <WiFiUDP.h>


unsigned int localUdpPort = 4210;  // local port to listen on
const char* data_logger_ip = "255.255.255.255"; //"192.168.1.110";
const int max_receive_misses_before_cutout = 750;



class UdpInterface{

    public:  

        UdpInterface(
            const char* subnet_ssid,
            const char* subnet_password
        )
        {
            _subnet_ssid = subnet_ssid;
            _subnet_password = subnet_password;
        }
        void begin(){
            _setup_for_udp();
        }

        WiFiUDP Udp;


    private:

        void _setup_for_udp(){
            WiFi.mode(WIFI_AP);
            _fix_ip();
            Udp.begin(localUdpPort);
        }

        void _fix_ip() {
            IPAddress staticIP(192, 168, 1, 11);
            IPAddress gateway(192, 168, 1, 9);
            IPAddress subnet(255, 255, 255, 0);

            //Fix the ip
            WiFi.softAPConfig(staticIP, gateway, subnet);
            WiFi.softAP(_subnet_ssid, _subnet_password);
            Serial.print("Access Point \"");
            Serial.print(_subnet_ssid);
            Serial.println("\" started");

            Serial.print("IP address:\t");
            Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP to the computer
            delay(20);
        }

        const char* _subnet_ssid;
        const char* _subnet_password;
};


class DataLoggerInterface{

    public:
        DataLoggerInterface(UdpInterface& udp) : _udp(udp.Udp) {}
        
        void LogData() {
            int time_this_message_sent = millis();
            int time_since_last_message_sent = time_this_message_sent - _time_last_message_sent_to_logger;
            _time_last_message_sent_to_logger = time_this_message_sent;
            // Send acclerometer data to data logger
            char data[80];
            
            // Need to work out how I'm getting the data in here.
            // sprintf(data, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", accelerometer_x, time_since_last_message_sent, error, diff_error, integral_error, p_change, d_change, i_change, change, right, leftWheelForward, measured_degrees_per_second, heading);
            
            //Serial.printf(data);
            //Serial.println("Attempting to send udp packet");
            _udp.beginPacket(data_logger_ip, 4210);
            _udp.print(data);
            _udp.endPacket();
        }

    private:
        WiFiUDP& _udp;
        int _time_last_message_sent_to_logger = 0;
};


class PS2_ControllerInterface{
     
        public:
            PS2_ControllerInterface(UdpInterface& udp) : _udp(udp.Udp), _receiving_from_controller(false) {}

            void ReadController() {
                // Serial.println("Attempting to read controller");
                _packetSize = _udp.parsePacket();
                if (_packetSize) {
                    _receiving_from_controller = true;
                    _receive_misses = 0;
                    // Serial.printf("Received %d bytes from %s, port %d\n", _packetSize, _udp.remoteIP().toString().c_str(), _udp.remotePort());
                    _read_incoming_packet();
                    _packetSize = 0;
                }
                else {
                    HandleNoSignal();
                }
            }
            unsigned char get_LX(){
                return _LX;
            }
            unsigned char get_LY(){
                return _LY;
            }
            unsigned char get_RX(){
                return _RX;
            }
            bool LeftPressed(){
                return _left_pressed;
            }
            bool RightPressed(){
                return _right_pressed;
            }
            bool UpPressed(){
                return _up_pressed;
            }
            bool DownPressed(){
                return _down_pressed;
            }
            bool ReceivingFromController(){
                return _receiving_from_controller;
            }

        private:
            void _buttons_from_byte(unsigned char buttons){
                _left_pressed = (buttons & (1<<0)) != 0;
                _right_pressed = (buttons & (1<<1)) != 0;
                _up_pressed = (buttons & (1<<2)) != 0;
                _down_pressed = (buttons & (1<<3)) != 0;
            }

            void _read_incoming_packet(){
                int len = _udp.read(_incomingPacket, 9);
                if (len > 0){
                    unsigned char* command = _incomingPacket;
                    _LX = command[0];
                    _LY = command[1];
                    _RX = command[2];
                    unsigned char buttons_byte = command[3];
                    _buttons_from_byte(buttons_byte);
                    //  measured_rpm = 500 + command[4] + command[5] + command[6] + command[7];
                    _max_melty_throttle = 1500 + command[8];
                    // Serial.printf("%d\t%d\t%d\t%d\t%d\n", command[4], command[5], command[6], command[7], command[8]);
                    //  Serial.printf("Spin speed: %d\tMax throttle: %d\n", measured_rpm, _max_melty_throttle);
                    // Serial.printf("UDP packet contents: %d\t%d\t%d\t%d\t%d\n", _LX, _LY, _RX, _left_pressed, _right_pressed);
                }
                else {
                    Serial.println("Failed to read packet.");
                    HandleNoSignal();
                }
            }

            void HandleNoSignal() {
                Serial.printf("No signal. Receive misses = %d\n", _receive_misses);
                _receive_misses++;
                if (_receive_misses >= max_receive_misses_before_cutout) {
                    _receiving_from_controller = false;
                    // _receive_misses = 0;
                }
            }
           
            WiFiUDP& _udp;     
            unsigned char _incomingPacket[9];  // buffer for incoming packets
            bool _receiving_from_controller;
            int _packetSize;
            unsigned char _LX = 127;
            unsigned char _LY = 127;
            unsigned char _RX = 127;
            bool _left_pressed = false;
            bool _right_pressed = false;
            bool _up_pressed = false;
            bool _down_pressed = false;
            int _receive_misses = 0;
            int _max_melty_throttle;
};

#endif