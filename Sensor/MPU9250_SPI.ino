 //ARDUINO:           MPU6500:
// MOSI-11           SDA/SDI
//MISO-12            ADO/SDO
//SCK-13             SCL/SCLK
//GPIO - 10           NCS


#include <SPI.h>

int CS = 8;
bool executed = false;
void setup(){
  pinMode (CS,OUTPUT);
  Serial.begin (9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000,MSBFIRST,SPI_MODE0));
  Serial.println("MPU Initialising.....");
  write_register(0x1C, 0x18); 
  delayMicroseconds(1000);
  write_register(0x6B, 0x00); 
  delayMicroseconds(1000);
  write_register(0x6A, 0x10);  
  delayMicroseconds (1000);
  mpu_config();
  SPI.endTransaction();
  delayMicroseconds(1000);
}


void loop() {
  float Ax_offset = 0;
  float Ay_offset = 0;
  float Az_offset = 0;
  if (!executed) {
    for (int i = 0; i < 5000; i++) {
      digitalWrite(CS, LOW);
      
      int16_t Ax_raw = read_register (0x3B) << 8 | read_register (0x3C);
      int16_t Ay_raw = read_register (0x3D) << 8 | read_register (0x3E);
      int16_t Az_raw = read_register (0x3F) << 8 | read_register (0x40);
      
      digitalWrite(CS, HIGH);
      
      Ax_offset += Ax_raw;
      Ay_offset += Ay_raw;
      Az_offset = Az_raw - 2048;
      
      delay(10);
    }
  
    Ax_offset /= 5000;
    Ay_offset /= 5000;
    Az_offset /= 5000;
    
    Serial.println("Calibration complete!");
    Serial.print("Ax offset: "); Serial.println(Ax_offset);
    Serial.print("Ay offset: "); Serial.println(Ay_offset);
    Serial.print("Az offset: "); Serial.println(Az_offset);

    executed = true;
  }
  
  int16_t accelX = read_register (0x3B) << 8 | read_register (0x3C);
  int16_t accelY = read_register (0x3D) << 8 | read_register (0x3E);
  int16_t accelZ = read_register (0x3F) << 8 | read_register (0x40);
    Serial.print ("     accel_X:  "); 
    Serial.print((accelX-Ax_offset)*(9.8/2048));
    Serial.print ("     accel_Y:  "); 
    Serial.print((accelY-Ay_offset)*(9.8/2048));
    Serial.print ("  accel_Z:  "); 
    Serial.println((accelZ-Az_offset)*(9.8/2048));
    Serial.print("pitch =");
    Serial.print(180 * atan (accelX/sqrt(accelY*accelY + accelZ*accelZ))/M_PI);
    Serial.print("roll =");
    Serial.print(180 * atan (accelY/sqrt(accelX*accelX + accelZ*accelZ))/M_PI);
    Serial.print("yaw =");
    Serial.println(180 * atan (accelZ/sqrt(accelX*accelX + accelZ*accelZ))/M_PI);
  delayMicroseconds(1000000);
  }


void write_register(uint8_t reg, uint8_t value){
  digitalWrite(CS, LOW);
  SPI.transfer(reg);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}


uint8_t read_register(uint8_t reg){
  uint8_t value;
  digitalWrite(CS, LOW);
  SPI.transfer(reg | 0x80);
  value = SPI.transfer(0x00);
  digitalWrite(CS,HIGH);
  return value;
}
void mpu_config(){
  
  write_register(0x3B, 0x00);  
  delayMicroseconds(1000);
  write_register(0x3C, 0x00);  
  delayMicroseconds (1000);
  write_register(0x3D, 0x00);  
  delayMicroseconds(1000);
  write_register(0x3E, 0x00);  
  delayMicroseconds (1000);
    write_register(0x40, 0x00);   
  delayMicroseconds (1000);
}
