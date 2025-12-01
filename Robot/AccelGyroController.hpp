#ifndef _ACCELGYRO_CONTROLLER_
#define _ACCELGYRO_CONTROLLER_

#include <Arduino.h>
#include <Wire.h>


const int accelInteruptPin = 5; //D1
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const int ACCEL_CONFIG_REGISTER = 0x1C;
const int GYRO_CONFIG_REGISTER = 0x1B;
const int ACCEL_READ_START_REGISTER = 0x3B;
const int GYRO_READ_START_REGISTER = 0x43;
const int GYRO_SCALING_FACTOR = 16.4; // LSB/deg/s for +-2000 deg/s.   //65.5; // 65.5 LSB/deg/s for a full scale range of +/- 500deg/s
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
            //Configure:          
            //Accel Config
            Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
            Wire.write(ACCEL_CONFIG_REGISTER); // Accelerometer config register
            Wire.write(0x11); // set range to +/- 16g
            Wire.endTransmission(true);
            //Gyro Config
            Wire.beginTransmission(MPU_ADDR);
            Wire.write(GYRO_CONFIG_REGISTER);                   // Talk to the GYRO_CONFIG register (1B hex)
            Wire.write(0x11); // I think this gives me +/- 2000 degrees/second              (0x10);    // Set the register bits as 00001000 (1000deg/s full scale)... I think.
            Wire.endTransmission(true);
            delay(20);

            read_accelerometer();
            read_gyroscope();
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

        int16_t get_gyro_x(){
            return _gyro_x;
        };
        int16_t get_gyro_y(){
            return _gyro_y;
        };
        int16_t get_gyro_z(){
            return _gyro_z;
        };

        void print_accelerometer_config(){
            print_config(ACCEL_CONFIG_REGISTER);
        }

        void print_gyroscope_config(){
            print_config(GYRO_CONFIG_REGISTER);
        }    

        void read_accelerometer() {
            Wire.beginTransmission(MPU_ADDR);
            Wire.write(ACCEL_READ_START_REGISTER); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
            Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
            Wire.requestFrom(MPU_ADDR, 6, true); // request a total of 7*2=14 registers

            // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
            int16_t accelerometer_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
            int16_t accelerometer_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
            int16_t accelerometer_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
            // accelerometer_x += aX_cal; accelerometer_y += aY_cal; accelerometer_z += aZ_cal;
            // Serial.printf("Ax = %d\tAy = %d\tAz = %d\n", accelerometer_x, accelerometer_y, accelerometer_z);
            _accel_x = accelerometer_x;
            _accel_y = accelerometer_y;
            _accel_z = accelerometer_z;

            _accel_last_read_time = millis();
        }

        void read_gyroscope() {
            Wire.beginTransmission(MPU_ADDR);
            Wire.write(GYRO_READ_START_REGISTER); // starting with register 0x43 (GYRO_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
            Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
            Wire.requestFrom(MPU_ADDR, 6, true); 

            // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
            int16_t gyroscope_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
            int16_t gyroscope_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
            int16_t gyroscope_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

            Serial.printf("%d\t%d\t%d\n", gyroscope_x, gyroscope_y, gyroscope_z);

            // gyroscope_x += 93, gyroscope_y -= 37, gyroscope_z -= 30;
            // gyroscope_x -= 65462, gyroscope_y -= 37, gyroscope_z -= 30;
            // Serial.printf("%d\t%d\t%d\n", gyroscope_x, gyroscope_y, gyroscope_z);

            _gyro_x = gyroscope_x / GYRO_SCALING_FACTOR;
            _gyro_y = gyroscope_y / GYRO_SCALING_FACTOR;
            _gyro_z = gyroscope_z / GYRO_SCALING_FACTOR;

            Serial.printf("%d\t%d\t%d\n\n", _gyro_x, _gyro_y, _gyro_z);


            // dividing by 65.5 because we configured the gyroscope range to +/- 500deg/s which has a sensitivity of 65.5 LSB/deg/s.
            // Serial.print("Gx raw: "); Serial.print(gyroscope_x);
            // Serial.print("\tGy raw: "); Serial.print(gyroscope_y);
            // Serial.print("\tGz raw: "); Serial.println(gyroscope_z);
            // Serial.printf("Gx = %f\tGy = %f\tGz = %f\n", gyroscope_x, gyroscope_y, gyroscope_z);
            // Serial.printf("Gx = %f\tGy = %f\tGz = %f\n", _gyro_x, _gyro_y, _gyro_z);

            _gyro_last_read_time = millis();
        }

        unsigned long time_since_accel_last_read(){
            return millis() - _accel_last_read_time;
        }

        unsigned long time_since_gyro_last_read(){
            return millis() - _gyro_last_read_time;
        }

    private:
        int16_t _accel_x, _accel_y, _accel_z;
        unsigned long _accel_last_read_time = 0;
        int _gyro_x, _gyro_y, _gyro_z;
        unsigned long _gyro_last_read_time = 0;

        void print_config(int config_register){
             Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
             Wire.write(config_register); // Accelerometer config register
             int AFS_SEL = Wire.read();
             Wire.endTransmission(true);
             Serial.println(AFS_SEL);
        }
};

#endif