//#include <SoftwareSerial.h>
#include<stdio.h>
#include <Wire.h>
#include "Kalman.h"

//SoftwareSerial HM10(12,11);//3번이 BLE RXD
#define HM10 Serial3


Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;
Kalman kalmanZ;
/* IMU Data */
int16_t accX, accY, accZ;
int16_t tempRaw;
int16_t gyroX, gyroY, gyroZ;
typedef union {
  float floatingPoint;
  byte binary[4];
} binaryFloat;

double accXangle, accYangle, accZangle; // Angle calculate using the accelerometer
double temp; // Temperature
double gyroXangle, gyroYangle, gyroZangle; // Angle calculate using the gyro
double compAngleX, compAngleY, compAngleZ; // Calculate the angle using a complementary filter
double kalAngleX, kalAngleY, kalAngleZ; // Calculate the angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

void setup() {
  Serial.begin(9600);
  
  HM10.begin(9600); //arduino <-> hm10
  Wire.begin();
  i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode
  while (i2cRead(0x75, i2cData, 1));


  delay(30); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = ((i2cData[0] << 8) | i2cData[1]);
  accY = ((i2cData[2] << 8) | i2cData[3]);
  accZ = ((i2cData[4] << 8) | i2cData[5]);
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accYangle = (atan2(accX, accZ) + PI) * RAD_TO_DEG;
  accXangle = (atan2(accY, accZ) + PI) * RAD_TO_DEG;
  accZangle = (atan2(accX, accZ) + PI) * RAD_TO_DEG;


  kalmanX.setAngle(accXangle); // Set starting angle
  kalmanY.setAngle(accYangle);
  kalmanZ.setAngle(accZangle);

  gyroXangle = accXangle;
  gyroYangle = accYangle;
  gyroZangle = accZangle;

  compAngleX = accXangle;
  compAngleY = accYangle;
  compAngleZ = accZangle;

  timer = micros();
}

void loop() {

if(HM10.available())
{
  
unsigned char a=HM10.read();
Serial.print("=======");
Serial.print(a,HEX);
Serial.println("=======");

  if(a==0xf1){
     while(1){
  /* Update all the values */
  while (i2cRead(0x3B, i2cData, 14));
  accX = ((i2cData[0] << 8) | i2cData[1]);
  accY = ((i2cData[2] << 8) | i2cData[3]);
  accZ = ((i2cData[4] << 8) | i2cData[5]);
  tempRaw = ((i2cData[6] << 8) | i2cData[7]);
  gyroX = ((i2cData[8] << 8) | i2cData[9]);
  gyroY = ((i2cData[10] << 8) | i2cData[11]);
  gyroZ = ((i2cData[12] << 8) | i2cData[13]);

  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accXangle = (atan2(accY, accZ) + PI) * RAD_TO_DEG;
  accYangle = (atan2(accX, accZ) + PI) * RAD_TO_DEG;
  accZangle = (atan2(accX, accY) + PI) * RAD_TO_DEG;



  double gyroXrate = (double)gyroX / 131.0;
  double gyroYrate = -((double)gyroY / 131.0);
  double gyroZrate = -((double)gyroZ / 131.0);

  gyroXangle += gyroXrate * ((double)(micros() - timer) / 1000000); // Calculate gyro angle without any filter
  gyroYangle += gyroYrate * ((double)(micros() - timer) / 1000000);
  gyroZangle += gyroZrate * ((double)(micros() - timer) / 1000000);

  //gyroXangle += kalmanX.getRate()*((double)(micros()-timer)/1000000); // Calculate gyro angle using the unbiased rate
  //gyroYangle += kalmanY.getRate()*((double)(micros()-timer)/1000000);

  compAngleX = (0.93 * (compAngleX + (gyroXrate * (double)(micros() - timer) / 1000000))) + (0.07 * accXangle); // Calculate the angle using a Complimentary filter
  compAngleY = (0.93 * (compAngleY + (gyroYrate * (double)(micros() - timer) / 1000000))) + (0.07 * accYangle);
  compAngleZ = (0.93 * (compAngleZ + (gyroZrate * (double)(micros() - timer) / 1000000))) + (0.07 * accZangle);

  kalAngleX = kalmanX.getAngle(accXangle, gyroXrate, (double)(micros() - timer) / 1000000); // Calculate the angle using a Kalman filter
  kalAngleY = kalmanY.getAngle(accYangle, gyroYrate, (double)(micros() - timer) / 1000000);
  kalAngleZ = kalmanZ.getAngle(accZangle, gyroZrate, (double)(micros() - timer) / 1000000);

  timer = micros();

  temp = ((double)tempRaw + 12412.0) / 340.0;

  /* Print Data */
  //display_formatted_float(accX, 5, 0, 3, false);
  //display_formatted_float(accY, 5, 0, 3, false);
  //display_formatted_float(accZ, 5, 0, 3, false);
  //display_formatted_float(gyroX, 5, 0, 3, false);
  //display_formatted_float(gyroY, 5, 0, 3, false);
  //display_formatted_float(gyroZ, 5, 0, 3, false);

  //Serial.print("\t");


  //display_formatted_float(compAngleX, 5, 2, 3, false);
  //display_formatted_float(kalAngleX, 5, 2, 3, false);

  // display_formatted_float(compAngleY, 5, 2, 3, false);//윗몸일으키기
  // display_formatted_float(kalAngleY, 5, 2, 3, false);//윗몸일으키기
int send_angle=int(kalAngleY)/2;

if(send_angle<0)
send_angle=0;


HM10.write(send_angle);



  Serial.print(int(kalAngleY/2));
  Serial.print("\r\n");
  // display_formatted_float(kalAngleZ, 5, 2, 3, false);//윗몸일으키기


      }
    }
  }
}


