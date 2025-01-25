// Class for configuring and reading from an MPU6050 Acceleromter Gyroscope
#ifndef _ACCELGYRO_CONTROLLER_
#define _ACCELGYRO_CONTROLLER_

#include <Arduino.h>
#include <Wire.h>


const int accelInteruptPin = 5; //D1
const int accelSCLPin = 4; //D2
const int accelSDAPin = 0; //D3
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const int aX_cal = -200, aY_cal = 280, aZ_cal = 0; // calibrations for accelerometer

// struct AccelGyroState{
//     int16_t accel_x;
//     int16_t accel_y;
//     int16_t accel_z;
//     // int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
//     // int16_t temperature; // variables for temperature data /TODO: rm if unused
// };

class AccelGyroController{

    public:
        // AccelGyroController(){
        //     _setup_accel_gyro();
        // };

        void begin() {
            Wire.begin();
            Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
            Wire.write(0x6B); // PWR_MGMT_1 register
            Wire.write(0); // set to zero (wakes up the MPU-6050)
            Wire.endTransmission(true);
            delay(50);
            //Configure acclerometer:
            Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
            Wire.write(0x1C); // Accelerometer config register
            Wire.write(0x11); // set range to +/- 16g
            Wire.endTransmission(true);

            //  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
            //  Wire.write(0x1C); // Accelerometer config register
            //  int AFS_SEL = Wire.read(); // set range to +/- 16g
            //  Wire.endTransmission(true);
            //  Serial.println(AFS_SEL);

            read_accelerometer();
    }

        int16_t get_accel_x(){
            return _accel_x;
        };
        int16_t get_accel_y(){
            return _accel_y;
        };
        int16_t get_accel_z(){
            return _accel_z;
        };

        void read_accelerometer() {
            Wire.beginTransmission(MPU_ADDR);
            Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
            Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
            Wire.requestFrom(MPU_ADDR, 7 * 2, true); // request a total of 7*2=14 registers

            // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
            int16_t accelerometer_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
            int16_t accelerometer_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
            int16_t accelerometer_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
            accelerometer_x += aX_cal; accelerometer_y += aY_cal; accelerometer_z += aZ_cal;
            // Serial.printf("Ax = %d\tAy = %d\tAz = %d\n", accelerometer_x, accelerometer_y, accelerometer_z);
            _accel_x = accelerometer_x;
            _accel_y = accelerometer_y;
            _accel_z = accelerometer_z;

            _last_read_time = millis();
        }
        unsigned long time_since_last_read(){
            return millis() - _last_read_time;
        }

    private:
        int16_t _accel_x, _accel_y, _accel_z;
        unsigned long _last_read_time;
};

#endif