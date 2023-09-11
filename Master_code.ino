#define IRsensor 9
int state = 0;
int potValue = 0;

void setup() {
  pinMode(IRsensor, INPUT);
  Serial.begin(38400); // Default communication rate of the Bluetooth module
}

void loop() {
 if(Serial.available() > 0){ // Checks whether data is comming from the serial port
    state = Serial.read(); // Reads the data from the serial port
 }


 potValue = digitalRead(IRsensor);
 Serial.write(potValue);
 delay(10);



}