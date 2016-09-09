#define DEFAULT_MOTOR_ON_TIME       5         //in minutes
#define DEFAULT_MOTOR_OFF_TIME      5        //in minutes
#define DEFAULT_TOTAL_SAMPLE_TIME   3         //in hours   
#define ONE_SEC                     1000000   //in microseconds
#define UART_BAUDRATE               115200    //uart baudrate setting

#define MOTOR_PIN   1
#define LED 13

#define DEBUG_TIMER

#include "TimerOne.h"

int inChar=0;
String inputString = "";         // a string to hold incoming data
boolean bln_string_arrived = false;  // whether the string is complete

typedef struct{
  int seconds;
  int minute;
  int hour;
  int day;
}timer;

typedef struct{
  timer on;
  timer off;
  timer s_time;
}sample;

timer Time;
sample Motor;

void setup() 
{
  Serial.begin(UART_BAUDRATE);
  Serial.println("Rotorod system init started...\n");
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  noInterrupts();
  Timer1.initialize(ONE_SEC);
  Timer1.attachInterrupt(timer1_Int_handler);
  Time.seconds=0;
  Time.minute=0;
  Time.hour=0;
  Time.day=0;
  Motor.on.minute=DEFAULT_MOTOR_ON_TIME;
  Motor.off.minute=DEFAULT_MOTOR_OFF_TIME;
  Motor.s_time.hour=DEFAULT_TOTAL_SAMPLE_TIME;
  inputString.reserve(200);                         // reserve 200 bytes for the inputString:
  Serial.println("System running.\n\n");
  interrupts();
}

void timer1_Int_handler()
{
static int on=0,off=0;

digitalWrite(LED,digitalRead(LED)^1);

if(on++>=Motor.on.minute) off++;
if(Time.seconds++>=59)
{
//  if(on++>=Motor.on.minute) off++;
 
  Time.seconds=0;
  if(Time.minute++>=59)
  {
    Time.minute=0;
    if(Time.hour++>=23)
    {
      Time.hour=0;
      Time.day++;
    }
  }
}

if(on>=Motor.on.minute)
{
  digitalWrite(MOTOR_PIN,LOW);
  Serial.println("Motor off");
  if((off)>=Motor.off.minute)
  {
    digitalWrite(MOTOR_PIN,HIGH);
    Serial.println("Motor on");
    on=0;
    off=0;
  }
}
else
{
  digitalWrite(MOTOR_PIN,HIGH);
  Serial.println("Motor on");
}


#ifdef DEBUG_TIMER
 Serial.print("Current on time (H:M:S): ");
 Serial.print(Time.hour);
 Serial.print(':');
 Serial.print(Time.minute);
 Serial.print(':');
 Serial.println(Time.seconds);

 Serial.print("Motor on timer: ");
 Serial.println(on);
 Serial.print("Motor off timer: ");
 Serial.println(off);

#endif
}

void loop() 
{
while(true);
}

