#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define BAUD_RATE 9600
#define ZIGBEE_SERIAL Serial3
#define ZIGBEE_SERIAL_TX 7 // Can be 0, 21
#define ZIGBEE_SERIAL_RX 8 // Can be 1, 5
#define BLE_SERIAL Serial2
#define BLE_SERIAL_TX 9 // Can be 9, 26
#define BLE_SERIAL_RX 10 // Can be 10, 31
#define MODULE_ID 2

// Current quaternion data
//     0
//     |
//     1 
//     |
//     2
//     |
//     3
//     |
// 4 - 5 - 6

float quats[7][4];
bool newQuatData = false;

Adafruit_BNO055 bno = Adafruit_BNO055(55);

float quatDiff(float q[4], float r[4]) {
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
void quatDiffRaw(float q[4], float r[4], float dest[4]) {
  float rConj[4] = {r[0], -r[1], -r[2], -r[3]};
  dest[0] = (rConj[0]*q[0]-rConj[1]*q[1]-rConj[2]*q[2]-rConj[3]*q[3]);
  dest[1] = (rConj[0]*q[1]+rConj[1]*q[0]-rConj[2]*q[3]+rConj[3]*q[2]);
  dest[2] = (rConj[0]*q[2]+rConj[1]*q[3]+rConj[2]*q[0]-rConj[3]*q[1]);
  dest[3] = (rConj[0]*q[3]-rConj[1]*q[2]+rConj[2]*q[1]+rConj[3]*q[0]);
}

int minDataLength = 22;

void printQuats(void) {
  for(int i = 0; i < 7; i++) {
    Serial.print("Device #"); Serial.println(i);
    for(int j = 0; j < 4; j++) { 
      Serial.print("Q");Serial.print(j); Serial.print(": "); Serial.println(quats[i][j]);
    }
  }
}

void parseZigbeeData(String s)
{
  if(s.length() == 0 || s.length() < minDataLength) 
  {
    s = "";
    return;
  }

  String currentString = s;
  String subarray = "";
  int moduleID;
  //Serial.print("This is the substring: ");
  
  // Parse Module ID
  subarray = currentString.substring(0, s.indexOf("{"));
  moduleID = subarray.toInt();
  
  // Parse q0
//  Serial.print("q0:");
  subarray = currentString.substring(currentString.indexOf("{")+1, currentString.indexOf(","));
//  Serial.println(subarray);
  quats[moduleID][0] = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);
//  Serial.print(currentString);

  // Parse q1
//  Serial.print("q1:");
  subarray = currentString.substring(0, currentString.indexOf(","));
//  Serial.println(subarray);
  quats[moduleID][1] = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);
//  Serial.print(currentString);
  
  // Parse q2
//  Serial.print("q2:");
  subarray = currentString.substring(0, currentString.indexOf(","));
//  Serial.println(subarray);
  quats[moduleID][2] = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);
//  Serial.print(currentString);

  // Parse q3
//  Serial.print("q3:");
  subarray = currentString.substring(0, currentString.indexOf("}"));
//  Serial.println(subarray);
  quats[moduleID][3] = subarray.toFloat();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ZIGBEE_SERIAL.begin(BAUD_RATE); 
  BLE_SERIAL.begin(BAUD_RATE); 

  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.write("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);
}

String dataLine;
void loop() {
  // put your main code here, to run repeatedly:
  // Get local quaternion
  imu::Quaternion quat = bno.getQuat();
//  Serial.print("qW: ");
//  Serial.print(quat.w(), 4);
//  Serial.print(" qX: ");
//  Serial.print(quat.x(), 4);
//  Serial.print(" qY: ");
//  Serial.print(quat.y(), 4);
//  Serial.print(" qZ: ");
//  Serial.print(quat.z(), 4);
//  Serial.print("\n");
  // Read from Zigbee serial if available
  char c;
  
  if(ZIGBEE_SERIAL.available())
  {
    c = ZIGBEE_SERIAL.read();
    if(c == '\n')
    {
      //Serial.write(dataLine.c_str());
      //Serial.write("\n");
//      Serial.println(dataLine);
      parseZigbeeData(dataLine);
//      printQuats();
      dataLine = "";
      newQuatData = true;
    }
    else
    {
      dataLine += c; //append text to end of command
    }
  }
  // Calculate angle between slave and master
  float localQuat[4];
  localQuat[0] = quat.w();
  quats[MODULE_ID][0] = quat.w();
  localQuat[1] = quat.x();
  quats[MODULE_ID][1] = quat.x();
  localQuat[2] = quat.y();
  quats[MODULE_ID][2] = quat.y();
  localQuat[3] = quat.z();
  quats[MODULE_ID][3] = quat.z();
  float angle = quatDiff(localQuat, quats[0]);
//  Serial.println(angle);
  // Transmit angles through BLE
  if(newQuatData) {
    String uartTx = "{";
    for(int i = 0; i < 5; i++) {
//      uartTx += String(quatDiff(quats[i], quats[i+1]));
      uartTx += String(quatDiff(quats[0], quats[1]));
      uartTx += ",";
    }
//    uartTx += quatDiff(quats[5], quats[6]);
    uartTx += quatDiff(quats[1], quats[2]);
    uartTx += "}";
    uartTx += "\n";
    Serial.write(uartTx.c_str());
    BLE_SERIAL.write(uartTx.c_str());

    newQuatData = false;
  }
}
