#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#ifndef F_CPU
#define F_CPU 8000000UL
#endif



volatile unsigned long timer0_millis = 0;

char PreviousIRsensor = 'c';

unsigned long BuzzertoActive = 3000;
unsigned long CartoStop = 6000;
unsigned long DurationIR;
bool Carstate = false;


const int intervalSensor = 50;
unsigned long PreReadingtime;

unsigned long Eyeclosedtime;




#define UBRRVAL 51

void usart_init(void);
void sendbyte(unsigned char);
void sendstr(unsigned char *);
unsigned char receivebyte(void);
void receivestr(unsigned char*);

unsigned char onmsg[] = "ON\n";  
unsigned char offmsg[] = "OFF\n";
unsigned char defaultmsg[] = "LED Status:\n";
unsigned char rxdata;


#define E (1<<PB3)
#define RS (1<<PB4)

void lcdinit();
void lcdcmd(uint8_t);
void lcdchar(uint8_t);
void lcdstr(unsigned char *);
void latch(void);
void lcdgoto(uint8_t , uint8_t );



#define THRESHOLD_DISTANCE 30 // Define the distance threshold in centimeters

// Function to generate a pulse on the trigger pin of the ultrasonic sensor
void trigger_pulse() {
    PORTD |= (1 << PD6);   // Set PD6 to HIGH
    _delay_us(10);         // Wait for 10 microseconds
    PORTD &= ~(1 << PD6);  // Set PD6 to LOW
   }

uint16_t measure_pulse_width() {
    uint16_t pulse_width = 0;
    
    // Wait for the echo pin to go HIGH
    while (!(PIND & (1 << PD7)));

    // Start the timer
    TCCR1B |= (1 << CS10);
    
    // Wait for the echo pin to go LOW
    while (PIND & (1 << PD7)) 
    {
        pulse_width++;
        _delay_us(1);
    }

    // Stop the timer
    TCCR1B = 0;

    return pulse_width;
}

unsigned long currentMillis;

void readSensorState(){
  if(currentMillis - PreReadingtime > intervalSensor){
    char SensorState = rxdata;
    
    if (SensorState == 'c' && PreviousIRsensor == 'o' && !Carstate) {
      DurationIR = currentMillis;
      PreviousIRsensor = 'c';
    }
    Eyeclosedtime = currentMillis - DurationIR;
    
    if (SensorState == 'c' && !Carstate && Eyeclosedtime >= BuzzertoActive){
      Carstate = true;
      PORTC |= (1<<PORTC0); //digitalWrite(buzzer,HIGH);
    
    }
    
    if (SensorState == 'c' && Carstate && Eyeclosedtime >= CartoStop) {
      

        uint16_t pulse_width;
        float distance_cm;

        trigger_pulse();                // Generate the trigger pulse
        pulse_width = measure_pulse_width(); // Measure the pulse width of the echo signal

        // Calculate the distance using the speed of sound (343 m/s)
        // and the formula: distance = (pulse_width * speed_of_sound) / (2 * clock_frequency)
        distance_cm = (pulse_width * 343) / (2 * F_CPU);

        // Check if the distance is less than or equal to the threshold
        if (distance_cm >= THRESHOLD_DISTANCE) {
              PORTB &= ~(1<<PORTB0); //digitalWrite(Relay,LOW);
              Carstate = false;
        }

      
    }
    if (SensorState == 'o' && PreviousIRsensor == 'c') {
      PreviousIRsensor = 'o';
      Carstate = false;
      PORTC &= ~(1<<PORTC0);  //digitalWrite(buzzer,LOW);
    }
    PreReadingtime = currentMillis;
  }
  
}

unsigned long millis()
{
  unsigned long m;
  uint8_t oldSREG = SREG;

  // disable interrupts while we read timer0_millis or we might get an
  // inconsistent value (e.g. in the middle of a write to timer0_millis)
  cli();
  m = timer0_millis;
  SREG = oldSREG;

  return m;
}

int main(void)
{
  
   DDRC |= (1<<DDC0);
   DDRB |= (1<<DDB0);
   PORTB |= (1<<PORTB0);

   DDRD |= (1 << PD6);  // Set PD6 (trigger pin) as an output
   DDRD &= ~(1 << PD7); // Set PD7 (echo pin) as an input

   DDRB |= (1<<PB0);
  usart_init();


  
  
    while (1)
    {
    currentMillis = millis();    // store the current time



    rxdata = receivebyte();
    
  
 
    readSensorState();           // read the sensor state

    }


    
    
}


void usart_init(void){
  UBRR0H= (unsigned char)(UBRRVAL>>8);   //high byte
    UBRR0L=(unsigned char)UBRRVAL;          //low byte
    UCSR0B |= (1<<TXEN0) | (1<<RXEN0);    //Enable Transmitter and Receiver
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);  //Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size
}

void sendbyte(unsigned char MSG){
    while((UCSR0A&(1<<UDRE0)) == 0);     // Wait if a byte is being transmitted
    UDR0 = MSG; 
}

void sendstr(unsigned char *s){
  unsigned char i = 0;
  while(s[i] != '\0'){
   sendbyte(s[i]);
   i++;
   }
}

unsigned char receivebyte(void){
  while(!(UCSR0A & (1<<RXC0)));
    return UDR0;
} 




void lcdinit(){
   
   //initialize PORTs for LCD
   DDRD |= (1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5);
    DDRB |= (1<<PB4) | (1<<PB3);
   
   _delay_ms(50);
   PORTD &= ~E;    //send low
   _delay_ms(50);  //delay for stable power
   lcdcmd(0x33);
   //_delay_us(100);
  lcdcmd(0x32);
  //_delay_us(100);
   lcdcmd(0x28);  // 2 lines 5x7 matrix dot
  // _delay_us(100);
    lcdcmd(0x0C);  // display ON, Cursor OFF
 // _delay_us(100);
   lcdcmd(0x01);  //clear LCD
 //  _delay_us(2000);
   lcdcmd(0x06);  //shift cursor to right
   _delay_us(1000);
   }
   
   void lcdcmd(unsigned char cmd){
      PORTD = (PORTD & 0x0F) | (cmd & 0xF0);  // send high nibble
     // PORTD &= ~RW; //send 0 for write operation
      PORTD &= ~RS; //send 0 to select command register
      PORTD |= E;   //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
   //   _delay_us(20);    //wait
      
      PORTD = (PORTD & 0x0F) | (cmd<<4);  //send low nibble 
       PORTD |= E;    //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
//_delay_us(20);    //wait
      }
      
  void lcdchar(unsigned char data){
      
      PORTD = (PORTD & 0x0F) | (data & 0xF0);  // send high nibble
      //PORTD &= ~RW; //send 0 for write operation
      PORTD |= RS;  //send 1 to select data register
      PORTD |= E;   //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
     
      PORTD = (PORTD & 0x0F) | (data<<4);  // send low nibble
      PORTD |= E;   //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
      
      }
      
 void lcdstr(unsigned char *str){
    unsigned char k=0;
    while(str[k] != 0){
   lcdchar(str[k]);
       k++;
       }
    }
 
 void lcdgoto(unsigned char x, unsigned char y){
      unsigned char firstcharadr[] = {0x80, 0xC0, 0x94, 0xD4};
      lcdcmd(firstcharadr[y-1] + x-1);
      _delay_us(100);
    }

void lcdclear(){
  lcdcmd(0x01);
  _delay_ms(1);
} 
