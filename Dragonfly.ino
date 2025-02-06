#include <Arduino.h>
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <WiFiUdp.h>
#include <Servo.h>
// #include <Wire.h>
#include "Robot/MotorController.hpp"
#include "Robot/MotorController.hpp"
#include "Robot/AccelGyroController.hpp"
#include "Robot/LEDController.hpp"
#include "Robot/UdpController.hpp"
#include "Robot/BotController.hpp"
#include "secrets.h"
// #include <ESPmDNS.h>

const int LEFT_SPINNER_ESC_PIN = 12; //D6
const int RIGHT_SPINNER_ESC_PIN = 14; //D5
const int DRIVE_ESC_PIN = 16; //D0
const int SPINNER_SERVO_MIN = 1000;
const int SPINNER_SERVO_MID = 1500;
const int SPINNER_SERVO_MAX = 2000;
const int DRIVE_SERVO_MIN = 1000;
const int DRIVE_SERVO_MID = 1500;
const int DRIVE_SERVO_MAX = 2000;

const int LED_PIN = 2;

int forward = 0, right = 0;


unsigned long time_this_message_sent, time_last_message_sent_to_logger = 0, time_since_last_message_sent;
unsigned long time_this_accel_reading, time_last_accel_reading = 0, time_since_last_accel_reading;


//  Set up Controllers:
AccelGyroController accel_gyro{};
LEDController led_controller{LED_PIN};
MotorController left_spinner_controller{LEFT_SPINNER_ESC_PIN, SPINNER_SERVO_MIN, SPINNER_SERVO_MID, SPINNER_SERVO_MAX};
MotorController right_spinner_controller{RIGHT_SPINNER_ESC_PIN, SPINNER_SERVO_MIN, SPINNER_SERVO_MID, SPINNER_SERVO_MAX};
MotorController drive_motor_controller{DRIVE_ESC_PIN, DRIVE_SERVO_MIN, DRIVE_SERVO_MID, DRIVE_SERVO_MAX};

UdpInterface udp_interface{WIFI_SSID, WIFI_PASSWORD};
PS2_ControllerInterface ps2_controller{udp_interface};
DataLoggerInterface data_logger{udp_interface};

BotController bot_controller{led_controller, left_spinner_controller, right_spinner_controller, drive_motor_controller, ps2_controller, accel_gyro};


void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  udp_interface.begin();
  delay(10);
  accel_gyro.begin();
  delay(10);
  bot_controller.begin();
  bot_controller.stop();
}

void loop() {
    ps2_controller.ReadController();
    if (ps2_controller.ReceivingFromController()){
      // Serial.println("Received signal, driving.");
      bot_controller.drive();
    }
    else{
      // Serial.println("No signal, stopping.");
      bot_controller.stop();
    }
    delay(10);
}



// Just write 1600 um pwm to all GPIO pins: 
// #include <Servo.h>

// Servo esc1, esc2, esc3, esc4, esc5, esc6, esc7, esc8, esc9;

// void setup() {
//   esc1.attach(12);
//   // esc2.attach(14);
//   esc3.attach(16);
//   // esc4.attach(5);
//   // esc5.attach(4);
//   // esc6.attach(0);
//   // esc7.attach(2);
//   // esc8.attach(15);
//   // esc9.attach(13);
//   // esc1.writeMicroseconds(2000);
//   // delay(2000);
//   // esc1.writeMicroseconds(1000);
//   // delay(2000);
//   // esc1.writeMicroseconds(1500);
//   // delay(2000);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   esc1.writeMicroseconds(1700);
//   // esc2.writeMicroseconds(1000);
//   esc3.write(1400);
//   // esc4.writeMicroseconds(1000);
//   // esc5.writeMicroseconds(1000);
//   // esc6.writeMicroseconds(1000);
//   // esc7.writeMicroseconds(1000);
//   // esc8.writeMicroseconds(1000);
//   // esc9.writeMicroseconds(1000);
//   delay(10);
// }

