#define DEFAULT_MOTOR_ON_TIME       7         //in minutes
#define DEFAULT_MOTOR_OFF_TIME      53        //in minutes
#define DEFAULT_TOTAL_SAMPLE_TIME   3         //in hours   
#define ONE_SEC                     100000   //in microseconds
#define UART_BAUDRATE               115200    //uart baudrate setting

#define MOTOR_PIN   1
#define LED 13

//#define DEBUG_TIMER

#include "TimerOne.h"

typedef struct{
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
  Time.minute=0;
  Time.hour=0;
  Time.day=0;
  Motor.on.minute=DEFAULT_MOTOR_ON_TIME;
  Motor.off.minute=DEFAULT_MOTOR_OFF_TIME;
  Motor.s_time.hour=DEFAULT_TOTAL_SAMPLE_TIME;
  Serial.println("System running.\n\n");
  interrupts();
}

void timer1_Int_handler()
{
static int sec=0;
digitalWrite(LED,digitalRead(LED)^1);

if(sec++>=59)
{
  sec=0;
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

#ifdef DEBUG_TIMER
 Serial.print("Tempo: ");
 Serial.print(Time.hour);
 Serial.print(':');
 Serial.print(Time.minute);
 Serial.print(':');
 Serial.println(sec);
#endif
}

void loop() 
{
  while (Serial.available() > 0)
  {
    switch(Serial.read())
    {
      case 'H':
      Serial.println("\n\n---- Help Menu ----");
      Serial.println("H: This menu");
      Serial.println("S: Print current settings");
      Serial.println("A: Adjust motor parameters\n");
      break;

      case 'S':
      Serial.println("\n\n---- System Status ----");
      Serial.print("Current on time: ");
      Serial.print(Time.hour);
      Serial.print(" Hour(s) ");
      Serial.print(Time.minute);
      Serial.print(" Minute(s)\n");
      Serial.print("Motor On Time: ");
      Serial.println(Motor.on.minute);
      Serial.print("Motor Off Time: ");
      Serial.println(Motor.off.minute);
      Serial.print("Total sample time: ");
      Serial.println(Motor.s_time.hour);
      break;

      case 'A':
      Serial.println("\n\n---- System Adjustment ----");
      Serial.println("Are you sure? (Y/N)");
      while((Serial.read()!='Y')|(Serial.read()!='N'))
      {
      if(Serial.read()=='Y')
        {
          Serial.println("Set the motor on time (minutes): ");
          int temp=Serial.parseInt();
          noInterrupts();
          Motor.on.minute=temp;
          interrupts();
          break;
        }
      else if(Serial.read()=='N')
        {
          Serial.println("Cancelled.");
          break;
        }
      }
      break;
    }
  }
}
