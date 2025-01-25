#ifndef _UDP_CONTROLLER_
#define _UDP_CONTROLLER_

#include <Arduino.h>
#include <WiFi.h>        // Include the Wi-Fi library
#include <WiFiUDP.h>
// #include "BotController.hpp"


unsigned int localUdpPort = 4210;  // local port to listen on
const char *subnet_ssid = "Moustache AP"; // The name of the Wi-Fi network that will be created
const char *subnet_password = "thereentnospoon";   // The password required to connect to it, leave blank for an open network
const char* data_logger_ip = "255.255.255.255"; //"192.168.1.110";
const int max_receive_misses_before_cutout = 750;


// TODO: Should probably have a small class/funcition to set up the udp object and then pass it by reference to the controller and data logger classes, which should be split out from this big one.


class UdpController{

    public:        
        WiFiUDP Udp;

        void begin(){
            _setup_for_udp();
        }

        // PS2 Controller Stuff:     
        void ReadController() {
            Serial.println("Attempting to read controller");
            packetSize = Udp.parsePacket();
            if (packetSize) {
                receiving_from_controller = true;
                receive_misses = 0;
                Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
                _read_incoming_packet();
                packetSize = 0;
            }
            else {
                HandleNoSignal();
            }
        }
        unsigned char get_LX(){
            return LX;
        }
        unsigned char get_LY(){
            return LY;
        }
        unsigned char get_RX(){
            return RX;
        }
        bool LeftPressed(){
            return left_pressed;
        }
        bool RightPressed(){
            return right_pressed;
        }
        bool UpPressed(){
            return up_pressed;
        }
        bool DownPressed(){
            return down_pressed;
        }
        bool ReceivingFromController(){
            return receiving_from_controller;
        }

        //Data Logger Stuff:
        void LogData() {
            int time_this_message_sent = millis();
            int time_since_last_message_sent = time_this_message_sent - time_last_message_sent_to_logger;
            time_last_message_sent_to_logger = time_this_message_sent;
            // Send acclerometer data to data logger
            char data[80];
            
            // Need to work out how I'm getting the data in here.
            // sprintf(data, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", accelerometer_x, time_since_last_message_sent, error, diff_error, integral_error, p_change, d_change, i_change, change, right, leftWheelForward, measured_degrees_per_second, heading);
            
            //Serial.printf(data);
            //Serial.println("Attempting to send udp packet");
            Udp.beginPacket(data_logger_ip, 4210);
            Udp.print(data);
            Udp.endPacket();
        }


    private:
        int _timeSinceLastPacketSent = 0;

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
            WiFi.softAP(subnet_ssid, subnet_password);
            Serial.print("Access Point \"");
            Serial.print(subnet_ssid);
            Serial.println("\" started");

            Serial.print("IP address:\t");
            Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer
            delay(20);
        }

        // PS2 Controller Stuff:
        unsigned char _incomingPacket[9];  // buffer for incoming packets
        bool receiving_from_controller;
        int packetSize;
        unsigned char LX = 127;
        unsigned char LY = 127;
        unsigned char RX = 127;
        bool left_pressed = false;
        bool right_pressed = false;
        bool up_pressed = false;
        bool down_pressed = false;
        int receive_misses = 0;
        int max_melty_throttle;

        void HandleNoSignal() {
            Serial.printf("No signal. Receive misses = %d\n", receive_misses);
            receive_misses++;
            if (receive_misses >= max_receive_misses_before_cutout) {
                receiving_from_controller = false;
                //Lost signal. Stop LED and Freeze all motors:
                Serial.println("No signal.");// Freezing motors\n");
                // delay(400);
                receive_misses = 0;
            }
        }

        void _read_incoming_packet(){
                int len = Udp.read(_incomingPacket, 9);
                if (len > 0){
                    _incomingPacket[len] = 0;
                    unsigned char* command = _incomingPacket;
                    LX = command[0];
                    LY = command[1];
                    RX = command[2];
                    unsigned char buttons_byte = command[3];
                    ButtonsFromByte(buttons_byte);
                    //  measured_rpm = 500 + command[4] + command[5] + command[6] + command[7];
                    max_melty_throttle = 1500 + command[8];
                    //  Serial.printf("%d\t%d\t%d\t%d\t%d\n", command[4], command[5], command[6], command[7], command[8]);
                    //  Serial.printf("Spin speed: %d\tMax throttle: %d\n", measured_rpm, max_melty_throttle);
                    Serial.printf("UDP packet contents: %d\t%d\t%d\t%d\t%d\n", LX, LY, RX, left_pressed, right_pressed);
                }
                else {
                    HandleNoSignal();
                }
        }

        void ButtonsFromByte(unsigned char buttons){
            //  left_pressed = false; right_pressed = false;
            left_pressed = (buttons & (1<<0)) != 0;
            right_pressed = (buttons & (1<<1)) != 0;
            up_pressed = (buttons & (1<<2)) != 0;
            down_pressed = (buttons & (1<<3)) != 0;
        }

        int time_last_message_sent_to_logger = 0;

};

// class Ps2Controller{
//     public:  

        

    
//     private:
//         WiFiUDP Udp;


// };

// class DataLogger{
    
//     

//     public:
//         void DataLogger(WiFiUDP udp){
//             Udp = udp;
//         }

        
//     private:
//         WiFiUDP Udp;
// };

#endif