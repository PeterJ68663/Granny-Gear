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
// #include <ESPmDNS.h>



const int leftESCPin = 15; //D8
const int rightESCPin = 2; //D7

const int LEDpin = 5;

int forward = 0, right = 0;




//Accelerometer:
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
// AccelGyroState accelGyroState;
const int ssf = 2048; //sensitivity scale factor. 1g = ssf.
const float g = 9.81;
const float pi = 3.1416;
const float accelerometer_radius = 0.0025; //meters

const int desired_angular_velocity_rpm = 400;
const float desired_angular_velocity_rad_per_s = desired_angular_velocity_rpm * (2 * pi / 60);
const float desired_centripetal_acceleration_ms2 = pow(desired_angular_velocity_rad_per_s, 2) * accelerometer_radius; //a = rw^2
const float desired_centripetal_acceleration_g = desired_centripetal_acceleration_ms2 / g;
const int desired_measured_accel_x = 3000; // desired_centripetal_acceleration_g * ssf;
int max_melty_throttle = 1600;
int measured_rpm = 1674;//1539;//880;//976;//1480;//1535;//1627; // The angular velocity we actually measure for this desired angular velocity. rpm
int measured_degrees_per_second = measured_rpm * 360 / 60;
int degrees_per_second = 0;

int melty_right_max = 100000;
int melty_start_motor_output = 1500;
int melty_start_right = map(1630, 1500, 2100, 0, melty_right_max);

float last_angular_velocity = 0, current_angular_velocity = 0;
float mean_angular_velocity;
unsigned long current_time, last_time;
unsigned long mu_s_elapsed;
int heading = 0; //Degrees * 100
char controlMode = 'T'; //T = Tank, M = Melty


unsigned long time_this_message_sent, time_last_message_sent_to_logger = 0, time_since_last_message_sent;
unsigned long time_this_accel_reading, time_last_accel_reading = 0, time_since_last_accel_reading;
int accel_period = 10; //ms
int p_error, last_p_error = 0, d_error;
int change, p_change, i_change, d_change;
unsigned long five = 0;
bool just_switched_to_melty_mode = true;
const int num_recent_p_errors = 25;
int last_n_p_errors[num_recent_p_errors] = {0};
bool read_accel_this_cycle = false;
const float K_p = 0.001;
const float K_i = 0;//0.00001;
const float K_d = 0;//0.001;
int error;
int integral_error;
int diff_error;

unsigned long heading_steer_time, last_heading_steer_time = 0, time_since_last_heading_steer;
int steer;


//  Set up Controllers:
AccelGyroController accel_gyro{};
LEDController led_controller{LEDpin};
MotorController left_motor_controller{leftESCPin};
MotorController right_motor_controller{rightESCPin};

UdpInterface udp_interface{};
PS2_ControllerInterface ps2_controller{udp_interface};
DataLoggerInterface data_logger{udp_interface};

BotController bot_controller{led_controller, left_motor_controller, right_motor_controller, ps2_controller, accel_gyro};


void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  //  Serial.printf("%f\t%f\t%f\t%f\t", desired_angular_velocity_rad_per_s, desired_centripetal_acceleration_ms2, desired_centripetal_acceleration_g, desired_measured_accel_x);

  udp_interface.begin();
  accel_gyro.begin();
  bot_controller.begin();
}

void loop() {
    ps2_controller.ReadController();
    Serial.printf("%d\n", ps2_controller.ReceivingFromController());
    if (ps2_controller.ReceivingFromController()){
      Serial.println("Received signal, driving.");
      bot_controller.drive();
    }
    else{
      Serial.println("No signal, stopping.");
      bot_controller.stop();
    }
    delay(10);
}

