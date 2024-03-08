// Importing the required libraries

#include<SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include "String.h"
#include <TinyGPSPlus.h>


// GPS MODULE PIN AND BAUD RATE DEFINITIONS
static const int RXPin = 4, TXPin = 3;        // D4 AND D3 OF NANO 
static const uint32_t GPSBaud = 9600;

// INITIALIZING THE SOFTWARESERIAL OBJECT TO HANDLE THE XBEE 
SoftwareSerial zigbee(3,2); 

#define ATITUDE_THRESHOLD 260;    // Defining the threshold altitude to which the main chute must be deployed


int i = 0;

// DEFINING THE CHIP SELECT FOR PERIPHERALS
int CS_BMP = 10;    // CS_BMP
int CS_MPU = 8;  //CS_MPU
int CS_SD = 9;    //CS_SD

// INITIALIZING OTHER DATA VARIABLES
int32_t t_fine;                                                 // t_fine for temperature calculation
int32_t temperature,pressure;                                   // temperature and pressure data variables
double accelX, accelY, accelZ;                                  // acceleration on all three axes
float gForceX, gForceY, gForceZ;                                // gyro values on all three axes
float altiReading,preReading,tempReading;                       
float avgalt,altitude;

float sum_alti = 0;
float alti = 0;
float alti_offset;
int j = 0;

float lastalti;

// INITIALIZING THE tinygps OBJECT
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);                                // softwareserial object for GPS

// SPI PROTOCOL CONFIGURATION
SPISettings mySPISettings(500000, MSBFIRST, SPI_MODE3);         //Defining the configuration of SPI protocol


void setup() {
  ss.begin(GPSBaud);
  pinMode(7,OUTPUT);

  digitalWrite(7,HIGH);

  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);

  // AVIONICS BAY ARMING STATUS
  zigbee.begin(9600);
  zigbee.write("Avionics Bay is Armed!!!!");

  // BMP INITIALIZATION:
  pinMode(CS_BMP, OUTPUT);
  digitalWrite(CS_BMP, LOW);
  delayMicroseconds(1000);

  Serial.begin(9600);
  SPI.begin();

  delayMicroseconds(1000);
  writeReg(0xE0, 0xB6);
  delayMicroseconds(1000);

  //delayMicroseconds(1000);
  //writeReg(0xF4, 0x2F);

  writeReg(0xF4, 0x57);
  delayMicroseconds(1000);

  //delay(100);
  writeReg(0xF5, 0x30);
  delayMicroseconds(1000);


  // MPU INITIALIZATION
  pinMode(CS_MPU, OUTPUT);
  digitalWrite(CS_MPU, LOW);  
  zigbee.println("MPU Initialising.....");
  writeRegister(0x1C, 0x18); 
  delayMicroseconds(1000);
  writeRegister(0x6B, 0x00); 
  delayMicroseconds(1000);
  writeRegister(0x6A, 0x10);  
  delayMicroseconds (1000);


  SPI.endTransaction();
  delayMicroseconds(1000);

  // SD CARD INTIALIZATION. WORKS ON A DIFFERENT SPI INSTANCE
  pinMode(CS_SD, OUTPUT);
  if (!SD.begin(CS_SD)) 
  {
    zigbee.write("Card failed, or not present");
    logdaata();
    Serial.println("Card failed, or not present");
  }
  
  zigbee.write("   Avionics bay is armed!!  ");
  logdaata_testing();
  sum_alti = 0;

  // To compute the average of the initial readings to set the offset
  for (int j = 0; j < 30; j++)
  {
    alti = readAltitudeValue(1013.25);
    delay(500);
    sum_alti = sum_alti + alti;
  }

  alti_offset = (sum_alti / 30) - 2;

  // PRINTING THE BASE ALTITUDE OF ROCKET BEFORE LAUNCH
  Serial.print("Average offset after 30 readings : ");
  Serial.print(alti_offset);

  float basealti = (readAltitudeValue(1013.25) - alti_offset);
  zigbee.print(basealti);
  zigbee.write("  \n   ");

}



void loop() {  
  
  altiReading = (readAltitudeValue(1013.25) - alti_offset);
  zigbee.write("Altitude value = ");
  zigbee.print(altiReading);
  Serial.println(altiReading);

  // READING ACCELERATION FROM MPU
  int16_t AcX = read_register (0x3B) << 8 | read_register (0x3C);
  int16_t AcY = read_register (0x3D) << 8 | read_register (0x3E);
  int16_t AcZ = read_register (0x3F) << 8 | read_register (0x40);

  float executed;

  float Ax_offset = 0;
  float Ay_offset = 0;
  float Az_offset = 0;
  
  if (!executed != 1) {
    for (int i = 0; i < 5000; i++) {
      
      int16_t Ax_raw = read_register (0x3B) << 8 | read_register (0x3C);
      int16_t Ay_raw = read_register (0x3D) << 8 | read_register (0x3E);
      int16_t Az_raw = read_register (0x3F) << 8 | read_register (0x40);
           
      Ax_offset += Ax_raw - 2048;
      Ay_offset += Ay_raw;
      Az_offset += Az_raw;
      
      delay(10);
    }
  
    Ax_offset /= 5000;
    Ay_offset /= 5000;
    Az_offset /= 5000;
    
    Serial.println("Calibration complete!");
    Serial.print("Ax offset: "); Serial.println(Ax_offset);
    Serial.print("Ay offset: "); Serial.println(Ay_offset);
    Serial.print("Az offset: "); Serial.println(Az_offset);

    executed = 1;
  }
  
  // G-FORCE COMPUTATION
  gForceX = (AcX - Ax_offset)*(9.81/ 2048);
  gForceY = (AcY - Ay_offset)*(9.81/ 2048);
  gForceZ = (AcZ - Az_offset)*(9.81/ 2048);

  //   
  zigbee.write("  ,  Acceleration X:   ");
  zigbee.print(AcX *(9.81/ 2048));
  Serial.println("acx :");
  Serial.println(AcX *(9.81/ 2048));
  
  zigbee.write(", Y:   ");
  zigbee.print(AcY *(9.81/ 2048));
  Serial.println("acy :");
  Serial.println(AcY *(9.81/ 2048));
  
  zigbee.write(", Z:   ");
  zigbee.print(AcZ *(9.81/ 2048));
  zigbee.write("  \n   ");
  Serial.println("acz :");
  Serial.print(AcZ *(9.81/ 2048));
  Serial.print("\n");

  // LATITUDE AND LONGITUDE FROM NEO6M
  float lat = gps_latitude();                 
  float lng = gps_longitude();
  zigbee.print(lat);
  zigbee.print(lng);
  Serial.println("Latitude");
  Serial.println(lat, 6); 
  //Serial.print("xxxx");                     // "xxxx" is used to differentiate between latitude and longitude while parsing serial input stream in python //Should be changed
  Serial.println("Longitude");
  Serial.println(lng, 6); 
  Serial.println("*********************************************************************************");

  lastalti =  (readAltitudeValue(1013.25) - alti_offset);
  delay(500);
  altitudee = (readAltitudeValue(1013.25) - alti_offset);
  int flag;
  LogData(altitudee, gForceX, gForceY, gForceZ);
  

  if(altitudee-lastalti>0)
  {
    delay(500);
    altitudee = (readAltitudeValue(1013.25) - alti_offset);
    flag=0;
  }
  bool m;
  int i,j;
  
  m = flag==1 && altitudee < ATITUDE_THRESHOLD;
  
  if(m)
  {
        digitalWrite(5,HIGH);
        LogDataParachute1();
        if (i<3)
        {
          zigbee.write("Main Parachute Ejected");
          i=i+1;
        }
  }
  if(altitudee-lastalti<0)
  {
    delay(500);
    altitudee = (readAltitudeValue(1013.25) - alti_offset);
    
  if(altitudee-lastalti<-2)
  {
    delay(500);
     altitudee = (readAltitudeValue(1013.25) - alti_offset);
    if(altitudee-lastalti<-5)
    {
      digitalWrite(6,HIGH);
      flag=1;
    LogDataParachute();
    if(j<3)
    {
      zigbee.write("Drogue Parachute Released");
      j=j+1;
    }
    }
  }
  }
  
}

void writeReg(byte address, byte data) //BMP
{
  digitalWrite(CS_BMP, LOW);
  delay(100); 
  byte reg = address & 0x7F;
  SPI.beginTransaction(mySPISettings);
  SPI.transfer(reg);
  SPI.transfer(data);
  SPI.endTransaction();
  delay(100);
  digitalWrite(CS_BMP, HIGH);
}

byte readReg(byte regAddress) //BMP
{
  digitalWrite(CS_BMP, LOW);
  delay(20);
  byte regAd = regAddress | 0x80;
  SPI.beginTransaction(mySPISettings);
  SPI.transfer(regAd);
  byte val = SPI.transfer(0x00);
  SPI.endTransaction();
  delay(20);
  digitalWrite(CS_BMP, HIGH);
  return (val);
}

float readTemperatureValue() {
  int32_t var1, var2;
  int32_t adc_T = read24(0xFA);
  adc_T >>= 4 ;
  int16_t dig_T1 = 27875; // (int16_t)readReg(0x89)<<8 | (int16_t)readReg(0x88);
  int16_t dig_T2 =  26548 ;//(int16_t)readReg(0x8B)<<8 | (int16_t)readReg(0x8A);
  int16_t dig_T3 =   -1000;//(int16_t)readReg(0x8D)<<8 | (int16_t)readReg(0x8C);
  var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) *((int32_t)dig_T2)) >>11;
  var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *((adc_T >> 4) - ((int32_t)dig_T1))) >>12) *((int32_t)dig_T3)) >>14;
  t_fine = var1 + var2;
  float T = (t_fine * 5 + 128) >> 8;
  return T/100 ; 
}


float readPressureValue() {
  int64_t var1, var2, p;
  int64_t adc_P = read24(0xF7);
  adc_P >>= 4;
  uint16_t dig_P1 =36376;//(int16_t)readReg(0x8F) << 8 | readReg(0x8E);
  int16_t dig_P2 = -10627;//(int16_t)readReg(0x91) << 8 | readReg(0x90);
  int16_t dig_P3 = 3024;//(int16_t)readReg(0x93) << 8 | readReg(0x92);
  int16_t dig_P4 = 3703;//(int16_t)readReg(0x95) << 8 | readReg(0x94);
  int16_t dig_P5 = 134;//(int16_t)readReg(0x97) << 8 | readReg(0x96);
  int16_t dig_P6 = -7;//(int16_t)readReg(0x99) << 8 | readReg(0x98);
  int16_t dig_P7 = 15500;//(int16_t)readReg(0x9B) << 8 | readReg(0x9A);
  int16_t dig_P8 = -14600;//(int16_t)readReg(0x9D) << 8 | readReg(0x9C);
  int16_t dig_P9 = 6000;//(int16_t)readReg(0x9F) << 8 | readReg(0x9E);
 
  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)dig_P6;
  var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
  var2 = var2 + (((int64_t)dig_P4) << 35);
  var1 = ((var1 * var1 * dig_P3) >> 8) +((var1 * (int64_t)dig_P2) << 12);
  var1 =(((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
  return (float)p / 256;
}

int32_t spixfer(int32_t x) {
  return SPI.transfer(x);
}

int32_t read24(byte reg) {
  int32_t value;
  SPI.beginTransaction(mySPISettings);
  digitalWrite(CS_BMP, LOW);
  spixfer(reg | 0x80); // read, bit 7 high
  value = spixfer(0);
  value <<= 8;
  value |= spixfer(0);
  value <<= 8;
  value |= spixfer(0);
  digitalWrite(CS_BMP, HIGH);
  SPI.endTransaction();
  return value;
}

float readAltitudeValue(float seaLevelhPa) {
  //tempReading = readTemperatureValue();  
  preReading = readPressureValue();
  float temp = preReading/100;
  float altitudeValue = 44330 * (1.0 - pow(temp / seaLevelhPa, 0.1903));
  return altitudeValue;
}

// WRITE REGISTER FOR MPU
void writeRegister(byte thisRegister, byte thisValue) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  digitalWrite(CS_MPU, LOW);
  SPI.transfer(thisRegister);                           //Send register address
  SPI.transfer(thisValue);                              //Send register data

  digitalWrite(CS_MPU, HIGH);

  SPI.endTransaction();
}

//READ REGISTER FOR THE MPU
unsigned int read_register(byte thisRegister)
{
  byte dataToSend = thisRegister | 0x80;
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  digitalWrite(CS_MPU, LOW);
  SPI.transfer(dataToSend);
  byte result = SPI.transfer(0x00);
  digitalWrite(CS_MPU, HIGH);

  SPI.endTransaction();
  return (result);
}


// RETURNS LATITUDE FROM THE NEO6M-GPS
float gps_latitude(){
  float latitude;

  while (ss.available() > 0)
    Serial.print("DEBUG STATUS : 1");

    if (gps.encode(ss.read()))
      Serial.print("DEBUG STATUS : 2");

      if (gps.location.isValid())
        {
          Serial.print("DEBUG STATUS : 3");
          latitude=(gps.location.lat());
        }
      else
      {
        Serial.print("DEBUG STATUS : 4");
      }
    else
    {
      Serial.print("DEBUG STATUS : 5");      
    }
  
  Serial.print("DEBUG STATUS : 6");

  return latitude;  
}


//RETURNS LONGITUDE FROM THE NEO6M-GPS
float gps_longitude(){
  float longitude;

  while (ss.available() > 0)
    Serial.print("DEBUG STATUS : 7");

    if (gps.encode(ss.read()))
      Serial.print("DEBUG STATUS : 8");

      if (gps.location.isValid())
      {
        Serial.print("DEBUG STATUS : 9");
        longitude=gps.location.lng();
      }

      else
      {
        Serial.print("DEBUG STATUS : 10");
      }
    
    else
    {
      Serial.print("DEBUG STATUS : 11");
    }
  
  Serial.print("DEBUG STATUS : 12");

  return longitude;
}


void LogData(float altitudeValue,  float gForceX, float gForceY, float gForceZ){
  File dataFile = SD.open("rfcccc.txt", FILE_WRITE);
   String dataString="";
    if (dataFile) {
      dataString += "ALtitude : ";
      dataString += String(altitudeValue);
      dataString += "\n";
      dataString += "Acc_X : ";
      dataString += String(gForceX);
      dataString+=",";
      dataString += "Acc_Y : ";
      dataString+=String(gForceY);
      dataString+=",";
      dataString += "Acc_Z : ";
      dataString+=String(gForceZ);
      dataString += "\n";
      dataString += "*****************************************************************"; 
    }
    dataFile.println(dataString);
    dataFile.close(); 
}

void LogDataParachute()
{
   File dataFile = SD.open("rfcccc.txt", FILE_WRITE);
   String dataString="";
    if (dataFile) {
      dataString+=("drogue parachute deployed");
      }
      dataFile.println(dataString);
      dataFile.close();
}

void LogDataParachute1()
{
   File dataFile = SD.open("rfcccc.txt", FILE_WRITE);
   String dataString="";
    if (dataFile) {
      dataString+=("Main parachute deployed");
      }
      dataFile.println(dataString);
      dataFile.close();
}
void logdaata_testing(){
     File dataFile = SD.open("rfcccc.txt", FILE_WRITE);
   String dataString="";
    if (dataFile) {
      dataString+=("Card failed, or not present");
      }
      dataString+=("lessgo");
      dataFile.println(dataString);
      dataFile.close();
     
  }

void logdaata(){
     File dataFile = SD.open("rfcccc.txt", FILE_WRITE);
   String dataString="";
    if (dataFile) {
      dataString+=("Card failed, or not present");
      }
      dataFile.println(dataString);
      dataFile.close();
  }

  void mpu_config(){

  writeRegister(0x3B, 0x00);  
  delayMicroseconds(1000);
  writeRegister(0x3C, 0x00);  
  delayMicroseconds (1000);
  writeRegister(0x3D, 0x00);  
  delayMicroseconds(1000);
  writeRegister(0x3E, 0x00);  
  delayMicroseconds (1000);
    writeRegister(0x40, 0x00);   
  delayMicroseconds (1000);
}