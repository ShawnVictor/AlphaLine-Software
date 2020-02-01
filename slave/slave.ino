/**
 *    Slave.ino
 *    Written by: Shawn Victor & Cole Morgan
 *    Date Last Mod: 1/29/20
 */

#define DEVICE_ID 0
#define SERIAL_MONITOR_BAUD 9600
#define XBEE_BAUD 9600
#define SERIAL_MONITOR Serial
#define SERIAL_XBEE Serial3


#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


//Global Data Variables
Adafruit_BNO055 bno = Adafruit_BNO055(55);
IntervalTimer heartbeatTimer;
IntervalTimer dataPushTimer;

float quat_w = 0.0;
float quat_x = 0.0;
float quat_y = 0.0;
float quat_z = 0.0;

int ledState = LOW;



void setup() 
{
  pinMode(13, OUTPUT);
  SERIAL_MONITOR.begin(9600);
  SERIAL_XBEE.begin(9600);

  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  heartbeatTimer.begin(blinker, 250000);
  dataPushTimer.begin(testSerialPushData, 250000);
}



void loop() 
{
  // put your main code here, to run repeatedly:
  interrupts();
  Serial3.println("Cole likes bad wiring!");
  Serial.println("message sent");
  delay(1000);
}



void testSerialPushData()
{
  imu::Quaternion quat = bno.getQuat();
  
  SERIAL_MONITOR.println();
  SERIAL_MONITOR.print(DEVICE_ID);
  SERIAL_MONITOR.print("{");

  SERIAL_MONITOR.print(quat.w());
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(quat.x());
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(quat.y());
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(quat.z());
  
  SERIAL_MONITOR.print("}");
  
  SERIAL_MONITOR.print("Data Packet Sent @ time ");
  SERIAL_MONITOR.print(micros());
  SERIAL_MONITOR.println(" us");
}


void serialPushData() // PACKEET FORMAT: ID{QUAT_W,QUAT_X,QUAT_Y,QUAT_Z}
{
  imu::Quaternion quat = bno.getQuat();
  
  SERIAL_XBEE.println();
  SERIAL_XBEE.print(DEVICE_ID);
  SERIAL_XBEE.print("{");

  SERIAL_XBEE.print(quat.w());
  SERIAL_XBEE.print(",");
  SERIAL_XBEE.print(quat.x());
  SERIAL_XBEE.print(",");
  SERIAL_XBEE.print(quat.y());
  SERIAL_XBEE.print(",");
  SERIAL_XBEE.print(quat.z());
  
  SERIAL_XBEE.print("}");
  
  SERIAL_MONITOR.print("Data Packet Sent @ time ");
  SERIAL_MONITOR.print(micros());
  SERIAL_MONITOR.println(" us");
}



float quatDiff(float q[4], float r[4]) 
{
  float qDiff[4] = {0};
  float rConj[4] = {r[0], -r[1], -r[2], -r[3]};
  float angle;
  qDiff[0] = (rConj[0]*q[0]-rConj[1]*q[1]-rConj[2]*q[2]-rConj[3]*q[3]);
  qDiff[1] = (rConj[0]*q[1]+rConj[1]*q[0]-rConj[2]*q[3]+rConj[3]*q[2]);
  qDiff[2] = (rConj[0]*q[2]+rConj[1]*q[3]+rConj[2]*q[0]-rConj[3]*q[1]);
  qDiff[3] = (rConj[0]*q[3]-rConj[1]*q[2]+rConj[2]*q[1]+rConj[3]*q[0]);
  
  
  float norm = (qDiff[0]*qDiff[0]+qDiff[1]*qDiff[1]+qDiff[2]*qDiff[2]+qDiff[3]*qDiff[3]);
  norm = sqrt(norm);
  
  qDiff[0] /= norm;
  qDiff[1] /= norm;
  qDiff[2] /= norm;
  qDiff[3] /= norm;
  
  angle = 180*2*acos(qDiff[0])/PI;
  return angle;
}



void blinker()
{
  if(ledState == LOW){ledState = HIGH;}
  else{ledState = LOW;}
  digitalWrite(13, ledState);
}
