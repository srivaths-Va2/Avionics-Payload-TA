float i=2;
void setup() {
  Serial.begin(9600);

}

void loop() {
  Serial.print(i,1);
  Serial.print("xxxx");
  Serial.println(i+1,1);
  delay(5000);
  i++;
}
