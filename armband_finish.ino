#include <SoftwareSerial.h>


#include <stdio.h>
#include <stdlib.h>

#include <Wire.h>
#include "Kalman.h"
SoftwareSerial HM10(12,11);




Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;
Kalman kalmanZ;
typedef union {
  float floatingPoint;
  byte binary[4];
} binaryFloat;
uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

int16_t accX, accY, accZ;
int16_t tempRaw;
int16_t gyroX, gyroY, gyroZ;


double accXangle, accYangle, accZangle; // Angle calculate using the accelerometer
double temp_xynu; // Temperature
double gyroXangle, gyroYangle, gyroZangle; // Angle calculate using the gyro
double compAngleX, compAngleY, compAngleZ; // Calculate the angle using a complementary filter
double kalAngleX, kalAngleY, kalAngleZ; // Calculate the angle using a Kalman filter



int count=0;


void setup()
{
  // delay(1000);
  Serial.begin(38400);
  HM10.begin(9600);
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

  // Serial.write("\nTest Start\n");
}

void loop()
{

  srand(analogRead(0)); 



  long start;

while(1){
 
    
  



  u8 real_data[40];//function코드 포함
  u8 receive_data[40];




  if (HM10.available())
  {
    unsigned char temp[2];
    delay(22);
    temp[0] = (unsigned char)HM10.read();

    temp[1] = (unsigned char)HM10.read();

    Serial.print("syn data:");

    Serial.print(temp[0], HEX);
    Serial.print(temp[1], HEX);
    Serial.println();

Serial.print("count:");
Serial.print(count,DEC);
Serial.println();
    if (temp[0] == 0xf3 && temp[1] == 0x12)
    {

        
      
count++;
      if(count==1)
      {
start=millis();
}
if(count==100)
{
long endtime=millis();
Serial.print("-------------endtime-------:");
Serial.print((endtime-start)/100,DEC);
Serial.print("(단위:100MS)");

Serial.println();
  
}
      char receive_size = 0;
      char received_size = 0;
      unsigned char function_code;
      
    unsigned char au_msgLen = 0;
  
      receive_size = (unsigned char)HM10.read();
      Serial.print("receive_size:");
      Serial.println(receive_size, DEC);

   
      
   


      while (received_size < receive_size + 1)
      {

        if (HM10.available()) {
          unsigned char sendunchar;
          sendunchar = (unsigned char)HM10.read();
         
          receive_data[received_size] = sendunchar;

          received_size++;
        }
      }
      function_code=receive_data[0];
      memcpy(real_data,receive_data+1,receive_size);
      Serial.println();
      Serial.print("function code:");
      Serial.println(function_code, DEC);
  
   Serial.print("real_receive data:");
    for(int k=0;k<receive_size;k++)
    {
Serial.print(real_data[k],HEX);
Serial.print("  ");      
    }
    Serial.println();

    
      switch (function_code)
      {


     
        break;
      case 0x04:
    {
        bool stop_flag=false;
      while(!stop_flag){


        
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

  temp_xynu = ((double)tempRaw + 12412.0) / 340.0;

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
int send_angle=int(kalAngleX)/2;

if(send_angle<0)
send_angle=0;


HM10.write(int(send_angle/2));




  Serial.print(send_angle);
  Serial.print("\r\n");

if (HM10.available())
  {
    unsigned char stopbyte[3];
    delay(30);
    char receive_data_stop=0;
while(receive_data_stop<5)
  { 
     
      if (HM10.available())
      {

        stopbyte[receive_data_stop]=(unsigned char)HM10.read();
        receive_data_stop++;
      }

  }
   

 
    if((stopbyte[0]==0xf3) &&(stopbyte[1]==0x12)&&(stopbyte[2]==0x01)&&(stopbyte[3]==0x05))
         stop_flag=true;
 

  } 


      }
           break;
           Serial.println("loop get out");
      }
        break;
        
   

      case 0x06:
      {
        bool stop_flag=false;
      while(!stop_flag){


        
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

  temp_xynu = ((double)tempRaw + 12412.0) / 340.0;

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

if (HM10.available())
  {
    unsigned char stopbyte[3];
    delay(30);
    char receive_data_stop=0;
while(receive_data_stop<5)
  { 
     
      if (HM10.available())
      {

        stopbyte[receive_data_stop]=(unsigned char)HM10.read();
        receive_data_stop++;
      }

  }
   

 
    if((stopbyte[0]==0xf3) &&(stopbyte[1]==0x12)&&(stopbyte[2]==0x01)&&(stopbyte[3]==0x07))
         stop_flag=true;
 

  } 


      }
           break;
           Serial.println("loop get out");
      }
          break;
      }

      /*
      u8 plain[50]={0x00,};
      Serial.println();
      Serial.print("received:");
      for(int k=0;k<receive_size;k++)
      Serial.print(real_data[k],HEX);

      //받은값 확인

      //시
      decipherLen=0;
      for (unsigned char i = 0; i <receive_size; i += CRYPTO_LEA_BLOCKSIZE)
      {
      unsigned char blockCipher[CRYPTO_LEA_BLOCKSIZE] = {0,};
      unsigned char blockDecipher[CRYPTO_LEA_BLOCKSIZE] = {0,};

      memcpy(blockCipher, real_data + i, CRYPTO_LEA_BLOCKSIZE);


      LEA_DecryptBlk(blockDecipher, blockCipher,rndkeys); //blockDecipher 는 블록 복호화
      memcpy(real_data + i, blockDecipher, CRYPTO_LEA_BLOCKSIZE);

      decipherLen += CRYPTO_LEA_BLOCKSIZE;//decipherLen은 전체 블록
      }

      decipherLen=unpad(real_data, decipherLen, CRYPTO_LEA_BLOCKSIZE);
      //끝
      Serial.println();
      Serial.println("decipherLen:");
      Serial.println(decipherLen,DEC);

      Serial.println("decry:");
      for(int k=0;k<decipherLen;k++)
      Serial.print(real_data[k],HEX);
      //복호화
      Serial.println();
      */


    }


  }
}
}

