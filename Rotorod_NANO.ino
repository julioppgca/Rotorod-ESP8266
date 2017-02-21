// Rotorod Sampler Test
// Author: Júlio Santos - julio.ppgca@gmail.com

/* -- Software parameters --*/
#define DEFAULT_MOTOR_ON_TIME       6         //in minutes
#define DEFAULT_MOTOR_OFF_TIME      54        //in minutes
#define DEFAULT_TOTAL_SAMPLE_TIME   2         //in hours   
#define DEFAULT_MOTOR_SPEED         200       // need to be validated 0..255 -> 0..2440RPM
#define MOTOR_STOP                  0
#define ONE_SEC                     1000000   //in microseconds, 1e6 is one second.
#define UART_BAUDRATE               115200    //uart baudrate setting

/* -- Hardware pins definition  --*/
#define MOTOR_PIN   11
#define LED         13

/* -- Debug timer over serial port --*/
//#define DEBUG_TIMER

/* -- Included library --*/
#include "TimerOne.h"
#include <U8glib.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

/* -- Global variables and structs --*/
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

timer Time;     // Software clock variable
sample Motor;   // Motor timer parameters

bool intFlag;   //control flags
bool motorFlag;

/*  --display update function--  */
void draw(int hour, int minu, int sec, int endS) 
{
  u8g.drawRFrame(0,16, 128, 48, 4);
  u8g.setFont(u8g_font_8x13B);
  u8g.drawStr(4,12,"PPGCA - Rotorod");
  u8g.drawStr(5,44,"Motor: ");
  u8g.drawStr(5,30,"Run.: ");
  if(endS)  u8g.drawStr(5,58,"Sampling...");
  else      u8g.drawStr(40,58,"..END..");

  u8g.setFont(u8g_font_8x13);  
  char buf[10];
  sprintf (buf, "%.2d:%.2d:%.2d", hour,minu,sec); 
  u8g.drawStr(55,30,buf);

  if(motorFlag)  u8g.drawStr(54,44,"Spinning");
  else           u8g.drawStr(54,44,"Stopped");
}


/* -- Initial Settings --*/
void setup() 
{
  Serial.begin(UART_BAUDRATE);                              //serial comm. initialize
  Serial.println("Rotorod system init started...\n");       //first message
  pinMode(MOTOR_PIN, OUTPUT);                               //motor pin GPIO as output used as PWM
  pinMode(LED, OUTPUT);                                     //led pin GPIO as output
  noInterrupts();                                           //disable all interrupts to initialize timer
  intFlag=false;                                            //no interrupt has been ocurred
  motorFlag=false;                                          //motor is off
  Timer1.initialize(ONE_SEC);                               //one interrupt every second
  Timer1.attachInterrupt(timer1_Int_handler);               //interrupt function handler definition
  Time.seconds=0;                                           //clock variables init  
  Time.minute=0;
  Time.hour=0;
  Time.day=0;
  Motor.on.minute=DEFAULT_MOTOR_ON_TIME;                    //motor default time initialize
  Motor.off.minute=DEFAULT_MOTOR_OFF_TIME;
  Motor.s_time.hour=DEFAULT_TOTAL_SAMPLE_TIME;

if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  // PWM setting, look at reference datasheet to understand code below.
  // http://www.atmel.com/Images/Atmel-42735-8-bit-AVR-Microcontroller-ATmega328-328P_Datasheet.pdf
  // pg.206
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20); 
  
  Serial.println("System running.\n\n");                    //all done
  interrupts();                                             //timer interrupt enabled
}

/* -- Timer1 interrupt handler --*/
void timer1_Int_handler()
{
digitalWrite(LED,digitalRead(LED)^1);

//if(on++>=Motor.on.minute) off++; // seconds increment, just for test purpouses..

if(Time.seconds++>=59)
{
// one minute has been elapsed.
if(Motor.on.minute--<=0) Motor.off.minute--;// minutes increment on motor timer.
//Motor.s_time.hour--; // minutes increment, just for test pourposes..
  Time.seconds=0;
  if(Time.minute++>=59)
  {
  // one hour has been elapesd.
    Motor.s_time.hour--;
    Time.minute=0;
    if(Time.hour++>=23)
    {
    // one day has been past.
      Time.hour=0;
      Time.day++;
    }
  }
}

if(Motor.s_time.hour>0)  // sample not finished.
{
  if(Motor.on.minute<=0) //motor control based on timers
  {
    OCR2A = MOTOR_STOP;
    motorFlag=false;
    Serial.println("Motor off");
    if(Motor.off.minute<=0)
    {
      OCR2A = DEFAULT_MOTOR_SPEED; 
      motorFlag=true;
      Serial.println("Motor on");
      Motor.on.minute=DEFAULT_MOTOR_ON_TIME;
      Motor.off.minute=DEFAULT_MOTOR_OFF_TIME;
    }
  }
  else
  {
    OCR2A = DEFAULT_MOTOR_SPEED; 
    motorFlag=true;
    Serial.println("Motor on");
  }
}


#ifdef DEBUG_TIMER
 Serial.print("Current on time (H:M:S): ");
 Serial.print(Time.hour);
 Serial.print(':');
 Serial.print(Time.minute);
 Serial.print(':');
 Serial.println(Time.seconds);
 Serial.print("On timer: ");
 Serial.println(Motor.on.minute);
 Serial.print("Off timer: ");
 Serial.println(Motor.off.minute);
#endif

intFlag=true;
}

void loop() 
{
    while(intFlag)
    {
    //noInterrupts();
    int endS=Motor.s_time.hour;
    int sec=Time.seconds;
    int minu=Time.minute;
    int hour=Time.hour;
    //interrupts();
    
    u8g.firstPage();
    do
      {
        draw(hour,minu,sec,endS);
      }while( u8g.nextPage() );

   if(endS<=0)
    {
      OCR2A=MOTOR_STOP;
      motorFlag=false;
      digitalWrite(LED,HIGH);
      Serial.println("\n End of the sample time");
      delay(200);
      noInterrupts();
      while(true);
    }
    intFlag=false;
    }
}
