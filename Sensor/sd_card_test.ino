#include<SPI.h>
#include <SD.h>
#include "String.h"

int cs=7;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(cs, OUTPUT);
if (!SD.begin(cs)) 
  {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("Card successfully found");

}

void loop() {
  // put your main code here, to run repeatedly:
dataLog();
}
void dataLog(){
  File dataFile = SD.open("Hello.txt", FILE_WRITE);
   String dataString="";
   for(int r=0;r<1000;r++){
   dataString=String(r);
   dataFile.println(dataString);
   }
 dataFile.close(); 
  }
