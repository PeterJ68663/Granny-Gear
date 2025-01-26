#include <Arduino.h>
#include <WiFi.h>        // Include the Wi-Fi library
#include <WiFiUdp.h>
#include <ESP32Servo.h>
// #include <Wire.h>
#include "MotorController.hpp"
#include "AccelGyroController.hpp"
#include "LEDController.hpp"
#include "UdpController.hpp"
#include "BotController.hpp"
// #include <ESPmDNS.h>



const int leftESCPin = 15; //D8
const int rightESCPin = 2; //D7

const int LEDpin = 5;

int forward = 0, right = 0;
int leftWheelForward = 0, rightWheelForward = 0;
int leftWheelSpin = 0, rightWheelSpin = 0, leftWheelTranslate = 0, rightWheelTranslate = 0;
bool inverted = false;




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

BotController bot_controller{led_controller, left_motor_controller, right_motor_controller, udp_interface, accel_gyro};


void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');
  //  Serial.printf("%f\t%f\t%f\t%f\t", desired_angular_velocity_rad_per_s, desired_centripetal_acceleration_ms2, desired_centripetal_acceleration_g, desired_measured_accel_x);

  udp_interface.begin();
  accel_gyro.begin();
  bot_controller.begin();
}

void loop() {
    ps2_controller.ReadController();
    if (ps2_controller.ReceivingFromController()){
      Serial.println("Received signal, driving.");
      bot_controller.drive();
    }
    else{
      Serial.println("No signal, stopping.");
      bot_controller.stop();
    }
    delay(10);

//     TankOrMelty();
  
//   if (udp_interface.ReceivingFromController()) {
//     read_accel_this_cycle = false;
//     time_this_accel_reading = millis();
//     time_since_last_accel_reading = time_this_accel_reading - time_last_accel_reading;
//     if (time_since_last_accel_reading >= accel_period) {
//       read_accel_this_cycle = true;
//       time_last_accel_reading = time_this_accel_reading;
//       accel_gyro.read_accelerometer();
//       // check_inverted();
//     }
    
//     if (controlMode == 'T') {
//       TankDrive();
//       just_switched_to_melty_mode = true; // Means if we're in melty mode we'll have just switched from tank mode. Needs a better name.
//     }
    
//     else if (controlMode == 'M') {
//       if (just_switched_to_melty_mode) {
//         right = melty_start_right;
//         just_switched_to_melty_mode = false;
//       }

//       if(read_accel_this_cycle){
// //      PID();
// //      simple_feedback_loop();
// //        degrees_per_second = DPS_from_accel(accelerometer_x);
// //        LogData();
//       }
//       constant_motor_control();

//       //Calculate Heading:
//       last_time = current_time;
//       current_time = micros();
//       mu_s_elapsed = current_time - last_time;
//       heading += (measured_degrees_per_second * mu_s_elapsed) / 10000;
// //      Serial.printf("Micoseconds Elapsed = %d\tHeading change = %d\n", mu_s_elapsed, (measured_degrees_per_second * mu_s_elapsed) / 10000;
//       if (heading >= 36000) {
//         heading = heading % 36000;
//       }
//       udp_interface.LogData();
      
//       //Heading light
//       if (heading <= 2000 or heading >= (36000 - 2000)) {
//         led_controller.on();
//       }
//       else {
//         led_controller.off();
//       }

//       // //Trim:
//       // if (Left_pressed){
//       //   measured_degrees_per_second -= 6; // 6 dps = 1 rpm
//       // }
//       // if(Right_pressed){
//       //   measured_degrees_per_second += 6;
//       // }

//       // Steer:
// //      Serial.println(LX);
// //      steer = map(LX, 0, 255, -127, 127);
// //      Serial.println(steer);
// //      if(steer > 10){
// ////        Serial.println("Steering");
// //        heading_steer_time = millis();
// //        heading += (heading_steer_time - last_heading_steer_time) * 1;
// //        last_heading_steer_time = heading_steer_time;
// //      }
// //      else if(steer < -10){
// ////        Serial.println("Steering");
// //        heading_steer_time = millis();
// //        heading -= (heading_steer_time - last_heading_steer_time) * 1;
// //        last_heading_steer_time = heading_steer_time;
// //      }
      
// //      if(Right_pressed){
// //        heading += 800;
// //      }
// //      if(Left_pressed){
// //        heading -= 800;
// //      }

//       Translate();
//       leftWheelForward = leftWheelSpin + leftWheelTranslate;
//       rightWheelForward = rightWheelSpin + rightWheelTranslate;
//     }
    
    // Drive();
  // }
}











// //Helpers

// char tmp_str[7]; // temporary variable used in convert function
// char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
//   sprintf(tmp_str, "%6d", i);
//   return tmp_str;
// }










void TankDrive() {
  
}




// int abs_max(int input, int max_abs_value) {
//   if (input >= 0) {
//     input = min(input, max_abs_value);
//   }
//   else {
//     input = max(input, max_abs_value * -1);
//   }
//   return input;
// }

// void check_inverted() {
// //  if (accelerometer_z >= 0) {
// //    Serial.printf("Uninverted\t%d\n", accelerometer_z);
// //    inverted = true;
// //  }
// //  else {
// //    Serial.printf("Inverted\t%d\n", accelerometer_z);
// //    inverted = false;
// //  }
// }

void TankOrMelty() {
  if (udp_interface.get_RX() >= 150 | udp_interface.get_RX() <= 104) {
    controlMode = 'M';
  }
  else {
    controlMode = 'T';
  }
}

// void simple_feedback_loop() {
//   change = 1;
//   if (accelerometer_x < desired_measured_accel_x) {
//     // Serial.print("Accelerating\t");
//     right += change;
//   }
//   else if (accelerometer_x > desired_measured_accel_x and right >= 0) {
//     // Serial.print("Deccelerating\t");
//     right -= change;
//   }
//   leftWheelForward = -right;
//   rightWheelForward = -right; // More post-soldering hackery.
//   leftWheelForward = map(leftWheelForward, 0, melty_right_max, 1500, 2100);
//   rightWheelForward = map(rightWheelForward, 0, melty_right_max, 1500, 2100); // both the same because I wired the ESC up wrong :P

// }

// void update_last_ten_p_errors(int new_p_error){
//   for(int i=0; i<num_recent_p_errors - 1 ;i++){
//     last_n_p_errors[i] = last_n_p_errors[i+1];
//     last_n_p_errors[num_recent_p_errors - 1] = new_p_error;
//   }
// }

void constant_motor_control(){
  leftWheelSpin = 1385;
  rightWheelSpin = 1385;
}

// void PID() {
//   error = desired_measured_accel_x - accelerometer_x;
// //   int p_error_max = 50000;
// //   p_error = abs_max(p_error, p_error_max);
//   // Serial.println(p_error);
//   integral_error += error;
//   update_last_ten_p_errors(error);
//   diff_error = get_diff_error();

//   p_change = K_p * error;
//   i_change = K_i * integral_error;
//   d_change = K_d * diff_error;
    
//   change = p_change + i_change + d_change;

//   right += change;
//   right = max(0, right);
//   right = min(right, melty_right_max);
 
//   leftWheelForward = -right;
//   rightWheelForward = -right; // More post-soldering hackery.
//   leftWheelForward = map(leftWheelForward, 0, melty_right_max, 1500, 2100);
//   rightWheelForward = map(rightWheelForward, 0, melty_right_max, 1500, 2100); // both the same because I wired the ESC up wrong :P

//   leftWheelForward = min(leftWheelForward , max_melty_throttle);
//   rightWheelForward = min(rightWheelForward, max_melty_throttle);
//   // Serial.printf("left_wheel: %d\tright_wheel: %d\n", leftWheelForward, rightWheelForward);
// }

void Translate(){
  forward = map(udp_interface.get_LY(), 0, 255, 127, -127);
  leftWheelTranslate = 0;
  rightWheelTranslate = 0;
  int translate_throttle = 40;
  int adjusted_heading = heading / 100;
  if(forward > 50){
    if((adjusted_heading < 45 or adjusted_heading > 360 - 45)){
      leftWheelTranslate = translate_throttle;
      rightWheelTranslate = translate_throttle;
//      Serial.println("Translating");
      }
    else if (adjusted_heading > 180 - 45 and adjusted_heading < 180 + 45){
      leftWheelTranslate = -translate_throttle;
      rightWheelTranslate = -translate_throttle;
//      Serial.println("Translating");
    }
  }
}

// int get_diff_error(){
//   long sum_first_five = 0;  // sum will be larger than an item, long for safety.
//   long sum_second_five = 0;
//   for (int i = 0 ; i < 10 ; i++)
//     if (i < 10){
//       sum_first_five += last_n_p_errors [i] ;
//     }
//     else{
//       sum_second_five += last_n_p_errors [i];
//     }
//   return (sum_first_five - sum_second_five) / 5;
// }

// int DPS_from_accel(int accel_reading){
//   return (380.09840356 + 0.0460694 * accel_reading) * 60;
// }

// void Drive(){
// //    Serial.printf("leftWheelForward: %d\trightWheelForward: %d\n", leftWheelForward, rightWheelForward);
//     if(inverted){
//       leftWheelForward = 1500 + (1500 - leftWheelForward);
//       rightWheelForward = 1500 + (1500 - rightWheelForward);
//     }
//     leftWheelForward = 1500 + 1500 - leftWheelForward;
// //    Serial.printf("leftWheelForward: %d\trightWheelForward: %d\t heading: %d\n", leftWheelForward, rightWheelForward, heading);
    
//     left_motor_controller.drive();
    
    
    // L_esc.writeMicroseconds(leftWheelForward);
    // R_esc.writeMicroseconds(rightWheelForward);
    
    //Serial.printf("Right Wheel Forward: %d\n", rightWheelForward);
    //Serial.printf("Updated motors after: %d\n", millis() - five);
    //five = millis();
// }
