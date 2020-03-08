/*
    FileName: master.ino
    Developed by: Cole Morgan & Shawn Victor
    Last Modified: 3/8/2020
*/



//Adding in Library Files
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>



//Setting up Serial Interfaces
#define BAUD_RATE 9600
#define BLE_SERIAL Serial2
#define BLE_SERIAL_TX 9 // Can be 9, 26
#define BLE_SERIAL_RX 10 // Can be 10, 31
#define ZIGBEE_SERIAL Serial3
#define ZIGBEE_SERIAL_TX 7 // Can be 0, 21
#define ZIGBEE_SERIAL_RX 8 // Can be 1, 5



//Master ID
#define MODULE_ID 2



//Global Data
Adafruit_BNO055 bno = Adafruit_BNO055(55);
float quats[7][4];
float angles[7];
uint8_t systemCal, gyro, accel, mag;
imu::Quaternion quat;
int minDataLength = 22;
String dataLine;
int ledState;



//Interval Timers
IntervalTimer bnoSample;
IntervalTimer bleTx;



void setup() 
{
  //Setup pin modes
  pinMode(13, OUTPUT);


  //Setups the Serial 
  Serial.begin(115200);
  ZIGBEE_SERIAL.begin(BAUD_RATE); 
  BLE_SERIAL.begin(BAUD_RATE); 


  //Checking IMUs for connection
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.write("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
 
  
  //Waiting for the IMU to stabilize
  delay(1000);
  bno.setExtCrystalUse(true);


  //Waiting for IMU to Calibrate
  Serial.write("Calibrating\n");
  do 
  {
    bno.getCalibration(&systemCal, &gyro, &accel, &mag);
    Serial.print("Gyro: ");
    Serial.print(gyro);
    Serial.print(", Mag: ");
    Serial.println(mag);
    delay(100);
  } while(gyro != 3 || mag != 3);
  digitalWrite(13, HIGH);


  //Set Interval Timer Settings
  bnoSample.priority(1);
  bleTx.priority(0);
  bnoSample.begin(sampleBNO, hzToMicro(3));
  bleTx.begin(bleTransmit, hzToMicro(3));
}



void loop() 
{
  //Locals
  char c;


  noInterrupts();
  
  if(ZIGBEE_SERIAL.available())
  {
    //Serial.println(ZIGBEE_SERIAL.available());    //Used to output Current Buffer Size
    c = ZIGBEE_SERIAL.read();
    //Serial.write(c);                              //Used to output the Current character being read
    interrupts();
    

    if(c == '\n')
    {
      //Heartbeat Indicating the end of a packet
      if(ledState == LOW){ledState = HIGH;}
      else{ledState = LOW;}
      digitalWrite(13, ledState);


      Serial.println(dataLine);                     //Used to output the whole packet to the Serial Buffer
      

      noInterrupts();
      parseZigbeeData(dataLine);
      interrupts();


      //Clear dataLine String for next packet to be stored
      dataLine = "";
    }
    else
    {
      dataLine += c;                                //append text to end of command
    }
  }  

  interrupts();
}



//Parses the packet into the Corresponding Global Quaternion Data
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
  //Serial.print("This is the substring: ");        //can be used to see what an individual Packet Looks like
  
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

  if(moduleID == 0) {
    angles[0] = quatDiff(quats[0], quats[1]);
  }
  else if(moduleID == 4) {
    angles[3] = quatDiff(quats[4], quats[5]);
  }
  else {
    angles[moduleID-1] = quatDiff(quats[moduleID-1], quats[moduleID]);
    angles[moduleID] = quatDiff(quats[moduleID], quats[moduleID+1]);
  }
}



//Calculates the angles based on Two Quaternion inputs
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
  if(norm == 0)
    return 0;
  norm = sqrt(norm);
  
  qDiff[0] /= norm;
  qDiff[1] /= norm;
  qDiff[2] /= norm;
  qDiff[3] /= norm;
  
  angle = 180*2*acos(qDiff[0])/PI;
  // Get principal angle
  if(angle > 90 && angle < 180) {
    angle = 180-angle;
  }
  else if(angle > 180 && angle < 270) {
    angle = 180-angle;
  }
  else if(angle > 270 && angle < 360) {
    angle = angle-360;
  }
  
  return angle;
}



void quatDiffRaw(float q[4], float r[4], float dest[4]) 
{
  float rConj[4] = {r[0], -r[1], -r[2], -r[3]};
  dest[0] = (rConj[0]*q[0]-rConj[1]*q[1]-rConj[2]*q[2]-rConj[3]*q[3]);
  dest[1] = (rConj[0]*q[1]+rConj[1]*q[0]-rConj[2]*q[3]+rConj[3]*q[2]);
  dest[2] = (rConj[0]*q[2]+rConj[1]*q[3]+rConj[2]*q[0]-rConj[3]*q[1]);
  dest[3] = (rConj[0]*q[3]-rConj[1]*q[2]+rConj[2]*q[1]+rConj[3]*q[0]);
}



void printQuats(void) 
{
  for(int i = 0; i < 7; i++) {
    Serial.print("Device #"); Serial.println(i);
    for(int j = 0; j < 4; j++) { 
      Serial.print("Q");Serial.print(j); Serial.print(": "); Serial.println(quats[i][j]);
    }
  }
}



int checkData(String s) {
  String subStr;
  String curStr = s;

  // Check id length
  id = curStr.substring(0, curStr.indexOf("{"));
  if(id.length() != 1)
    return 0;
  // Update current string
  curStr = curStr.substring(curStr.indexOf("{")+1);

  // Verify angle formatting
  for(int i = 0; i < 6; i++) {
    subStr = curStr.substring(0, curStr.indexOf(","));
    subStr = subStr.replace("-", "");
    if(subStr.substring(0, ".").length() != 1)
      return 0;
    if(subStr.substring(subStr.indexOf(".")+1).length() != 2)
      return 0;
    curStr = curStr.substring(curStr.indexOf(",")+1);
  }

  // Verify packet id format and '}'
  if(subStr.indexOf("}") == -1)
    return 0;
  if(subStr.substring(0, subStr.indexOf("}")).toInt() == 0) {
    return 0;
  }
  
  return 1;
}



void sampleBNO()
{
  Serial.write("in BNO routine\n");
  bno.getCalibration(&systemCal, &gyro, &accel, &mag);
  if(gyro != 3 || mag != 3) {
    Serial.println("Did not read IMU");
    return;
  }
  
  quat = bno.getQuat();
  quats[MODULE_ID][0] = quat.w();
  quats[MODULE_ID][1] = quat.x();
  quats[MODULE_ID][2] = quat.y();
  quats[MODULE_ID][3] = quat.z();
  angles[MODULE_ID-1] = quatDiff(quats[MODULE_ID-1], quats[MODULE_ID]);
  angles[MODULE_ID] = quatDiff(quats[MODULE_ID], quats[MODULE_ID+1]);
}

int packetCnt = 1;
void bleTransmit() {
  Serial.write("in BLE routine\n");
  // Calculate angle between slave and master
//  float localQuat[4];
//  localQuat[0] = quat.w();
//  quats[MODULE_ID][0] = quat.w();
//  localQuat[1] = quat.x();
//  quats[MODULE_ID][1] = quat.x();
//  localQuat[2] = quat.y();
//  quats[MODULE_ID][2] = quat.y();
//  localQuat[3] = quat.z();
//  quats[MODULE_ID][3] = quat.z();
//  float angle = quatDiff(localQuat, quats[0]);
//  Serial.println(angle);
  //newQuatData = false;
  // Transmit angles through BLE
  String uartTx = "{";
  for(int i = 0; i < 5; i++) {
    uartTx += String(angles[i]);
//      uartTx += String(quatDiff(quats[0], quats[1]));
    uartTx += ",";
  }
  uartTx += angles[5];
  uartTx += ",";
  uartTx += packetCnt++;
//    uartTx += quatDiff(quats[1], quats[2]);
  uartTx += "}";
  uartTx += "\n";
//    Serial.write(uartTx.c_str());
  BLE_SERIAL.write(uartTx.c_str());

}



int hzToMicro(int hz)
{
  return (1000000/hz);
}
