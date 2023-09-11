#include <LiquidCrystal.h>


static const int IRsensor = 10;
#define buzzer A1
#define trigPin 6 
#define echoPin 7
#define relayPin 8 


int state = 1;
int buttonState = 0;




int PreviousIRsensor = LOW;
unsigned long BuzzertoActive = 3000;
unsigned long CartoStop = 6000;
unsigned long DurationIR;
bool Carstate = true;


const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);



const int intervalSensor = 50;
unsigned long PreReadingtime;
unsigned long Eyeclosedtime;
unsigned long currentTime;




void setup() {

  Serial.begin(38400); // Default communication rate of the Bluetooth module
  pinMode(IRsensor, INPUT);
  Serial.println("Driver wake");
  pinMode(buzzer, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  lcd.begin(16, 2);

}

void ReadSensor() {

  if (currentTime - PreReadingtime > intervalSensor) {
    int SensorState = state;

    if (SensorState == LOW && PreviousIRsensor == HIGH && Carstate) {
       DurationIR = currentTime;
      PreviousIRsensor = LOW;
      Serial.println("Driver eyes closed");
    }

    Eyeclosedtime = currentTime -  DurationIR;

    if (SensorState == LOW && Carstate && Eyeclosedtime >= BuzzertoActive) {
      digitalWrite(buzzer, HIGH);
      delay(20);
      Serial.println("Driver sleep     ");


      lcd.setCursor(0, 0);
      lcd.print("Buzzer active    ");
      lcd.display();


    }

    if (SensorState == LOW && Carstate && Eyeclosedtime >= CartoStop) {

     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Danger!          ");

     lcd.setCursor(0, 1);
     lcd.print("Driver Sleep     ");
     lcd.display();
  
     Serial.println("Driver sleep");


     long duration, distance_cm;
     digitalWrite(trigPin, LOW);
     delayMicroseconds(2);
     digitalWrite(trigPin, HIGH);
     delayMicroseconds(10);
     digitalWrite(trigPin, LOW);
     duration = pulseIn(echoPin, HIGH);


     distance_cm = (duration * 343) / (2 * 10000);

     Serial.print("Distance: ");
     Serial.print(distance_cm);
     Serial.println(" cm");
     delay(50);


     if (distance_cm >= 30) {

       digitalWrite(relayPin, LOW);
       Serial.println("Stop");
       Carstate = false;

       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("Car Stopped...!!!");
       lcd.display();
       }
    }


    if (SensorState == HIGH && PreviousIRsensor == LOW) {
      PreviousIRsensor = HIGH;
      Carstate = true;
      digitalWrite(buzzer, LOW);
      Serial.println("Driver awake");
      lcd.clear();
    }

    PreReadingtime = currentTime;
  }
}

void loop() {


  if(Serial.available() > 0)
  {
  state = Serial.read();
  }
 

 Serial.println(state);
 delay(20);


 currentTime = millis();
 ReadSensor();




}


