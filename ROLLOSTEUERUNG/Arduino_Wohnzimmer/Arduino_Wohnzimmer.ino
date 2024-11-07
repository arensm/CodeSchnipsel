/////////////////////////////////////////////////////////
// Rollosteuerung
// Arduino mit 433Mhz und ESP8266 (mit Arduino IDE Integration)
// Ari 2017.01 Teil 2von2 Arduino Teil
// Funktion:
// 1. WLAN ueber den Teil1von2 ESP
// 2. 433Mhz Funkempfänger
//
// Free for anybody - no warranties
// Thanks for Stefan Thesen
/////////////////////////////////////////////////////////

// use this as debug pin - the LED is on, if Motor1 is set down
#define LEDPIN 13

// Receive-Buffer - for Softserial / WIFI reading
#define RECV_BUFFER_SIZE 20
char RECV_Buffer[RECV_BUFFER_SIZE];

// assign serial ports
#define WIFI_Serial mySerial
#define Debug_Serial Serial

#include <SoftwareSerial.h>
SoftwareSerial mySerial(3,4); // RX, TX

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
unsigned long lRFOldValue=0;  // to check for consecutive reads on 433MHz
int iRFCodeRepeat=0; // how many repeated codes receiver of 433MHz?

// Number of Motors to control
#define NO_MOTOR 3
// at which pin is motor 0's 1st pin? - A0 is pin 14, A1=15 ...
#define MOTOR_PIN_OFFSET 14

// ESP8266 reset pin
#define ESP_RESET 5

// array for the fc receive codes
unsigned long RFCode[NO_MOTOR][2];

// state of the motors
int iMotorState[NO_MOTOR];
#define MOTOR_UP    -1
#define MOTOR_STOP   0
#define MOTOR_DOWN   1
#define MOTOR_INVAL -999

// time to wait when turning motor off or changing direction
// recommended to allow motor capacitor to unload over motor coil 
#define MOTOR_WAIT_MS 300

// time for direction relay to swtich before power is applied
// recommended to avoid implicit switch of direction with power on
#define RELAIS_WAIT_MS 150

// duration of a motor event
//#define EVENT_MS 120000
#define EVENT_MS 30000
#define DEBOUNCE_MS 1000
unsigned long ulMotorEventEnd[NO_MOTOR];  // end of current motor event
unsigned long ulRFEventDeBounce;  // timer for RF event on 433MHz (needed for debouncing)


///////////////////////////////////////
// sets a motor into up/down/stop state
///////////////////////////////////////
void SetMotor(int iMotor, int iState)
{ 
  // calculate output pins
  int iMotorPwr=2*iMotor   +MOTOR_PIN_OFFSET;
  int iMotorDir=2*iMotor+1 +MOTOR_PIN_OFFSET;
  
  // define output state
  pinMode(iMotorPwr,OUTPUT); 
  pinMode(iMotorDir,OUTPUT); 
  
  // pin n is motor power; pin n+1 is motor direction for motor 0; etc...
  // relais board INVERTS!!! - HIGH = OFF
  if (iState==MOTOR_STOP)
  {
    digitalWrite(iMotorPwr,HIGH);
    delay(MOTOR_WAIT_MS); // wait some time to allow motor capacitor to unload over motor coil
    digitalWrite(iMotorDir,HIGH);
    ulMotorEventEnd[iMotor]=millis(); 
    Debug_Serial.print("Motor STOP:");
    Debug_Serial.println(iMotor);
  }
  else if (iState==MOTOR_UP)
  {
    // if still driving, stop first (incl delay)
  	if (iMotorState[iMotor]!=MOTOR_STOP) {SetMotor(iMotor,MOTOR_STOP);}

    digitalWrite(iMotorDir,HIGH);
    delay(RELAIS_WAIT_MS);
    digitalWrite(iMotorPwr,LOW);
    ulMotorEventEnd[iMotor]=millis()+EVENT_MS;
    Debug_Serial.print("Motor UP:");
    Debug_Serial.println(iMotor);
  }
  else if (iState==MOTOR_DOWN)
  {
  	// if still driving, stop first 
  	if (iMotorState[iMotor]!=MOTOR_STOP) {SetMotor(iMotor,MOTOR_STOP);}
  	
    digitalWrite(iMotorDir,LOW);
    delay(RELAIS_WAIT_MS);
    digitalWrite(iMotorPwr,LOW);
    ulMotorEventEnd[iMotor]=millis()+EVENT_MS;
    Debug_Serial.print("Motor DOWN:");
    Debug_Serial.println(iMotor);
  }
  
  // debug output: motor 0 to LED pin
  if(iMotor==0)
  {
    if (iState==MOTOR_DOWN) {digitalWrite(LEDPIN,HIGH);}
    else {digitalWrite(LEDPIN,LOW);}
  }
  
  // remember state
  iMotorState[iMotor]=iState;
}


////////////////
// setup routine
////////////////
void setup() 
{
  // init serials - make sure ESP8266 on WIFI_Serial is also at 9600
  Debug_Serial.begin(9600);
  WIFI_Serial.begin(9600);

  // set-up rf receiver
  mySwitch.enableReceive(0);  // 433MHz Receiver on interrupt 0 => that is pin #2
  ulRFEventDeBounce=0;

  Debug_Serial.println("Rollosteuerung - Shorty und Ari 2018.05");

  ////////////////////////////
  // define RF receiver codes  -  place your codes here, if you want 433MHz support
  ////////////////////////////
   // Pollin FB Wohnzimmer
  // OFF OFF OFF OFF OFF
  // 00000
  // Nr  ON    OFF
  // 1 5588305   5588308
  // 2 5591377   5591380
  // 3 5592145   5592148
  // 4 5592337   5592340
  // linke Taste RUNTER
  // rechte Tarse HOCH

  RFCode[0][0]=5588308;  // left switch off
  RFCode[0][1]=5588305;  // left switch on
  RFCode[1][0]=5591380;  // mid switch off
  RFCode[1][1]=5591377;  // mid switch on
  RFCode[2][0]=5592148;  // right switch off
  RFCode[2][1]=5592145;  // right switch on

  
        
  // set-up motor states
  for (int ii=0;ii<NO_MOTOR;ii++)
  {    
    SetMotor(ii,MOTOR_STOP);
  }
  
  // set-up output
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  
  // reset ESP8266
  pinMode(ESP_RESET, OUTPUT);
  digitalWrite(ESP_RESET, LOW);
  delay(500);
  digitalWrite(ESP_RESET, HIGH);
}


////////////
// main loop
////////////
void loop() 
{
  //////////////
  // handle WIFI
  //////////////
  if(WIFI_Serial.available())
  {
    if (WIFI_Serial.findUntil("MOTOR", "\n"))
    {
      if (WIFI_Serial.readBytesUntil('\n', RECV_Buffer, RECV_BUFFER_SIZE))
      {
        // we should now have the motor number in ascii in receive buffer
        int iMotor = RECV_Buffer[0] -48 -1; // -1 as webinterface calls Motor0 MOTOR1
        int iCommand=MOTOR_INVAL;
        if (iMotor>=0 && iMotor<=9)
        {
          Debug_Serial.print("Motor ");
          Debug_Serial.print(iMotor);
          
          if (strncmp(&RECV_Buffer[1], "UP", 2) == 0)
          {
            iCommand=MOTOR_UP;
          }
          else if (strncmp(&RECV_Buffer[1], "DOWN", 4) == 0)
          {
            iCommand=MOTOR_DOWN;
          }
          else if (strncmp(&RECV_Buffer[1], "STOP", 4) == 0)
          {
            iCommand=MOTOR_STOP;
          }
          Debug_Serial.print(" Kommando ");
          Debug_Serial.println(iCommand);
          
          if (iCommand!=MOTOR_INVAL)
          {
            // drive motor
            SetMotor(iMotor,iCommand);
                       
            // reset RF code repeat (we might want to stop via RF what WIFI did)
            iRFCodeRepeat=0;
            lRFOldValue=0;
          }
        }
      }
    }
  }
  
  ////////////////
  // handle 433MHz
  ////////////////
  if (mySwitch.available()) 
  {
    unsigned long value = mySwitch.getReceivedValue();
    
    if (value == 0) 
    {
      Debug_Serial.print("Unknown encoding");
    } 
    else
    {
      Debug_Serial.print("Received ");
      Debug_Serial.print( value );
      Debug_Serial.print(" / ");
      Debug_Serial.print( mySwitch.getReceivedBitlength() );
      Debug_Serial.print("bit ");
      Debug_Serial.print("Protocol: ");
      Debug_Serial.println( mySwitch.getReceivedProtocol() );

      // find code in list
      for (int ii=0;ii<NO_MOTOR;ii++)
      {
        for (int ij=0;ij<2;ij++)
        {
          if (value == RFCode[ii][ij])
          {
            if (value == lRFOldValue)
            {
              iRFCodeRepeat++;
            }
            else  // use only new commands
            {
              iRFCodeRepeat=0;
              ulRFEventDeBounce=millis()+DEBOUNCE_MS;
              
              Debug_Serial.print("Using RF Command:");
              Debug_Serial.print(ii);
              Debug_Serial.print(",");
              Debug_Serial.println(ij);
              
              // drive motor
              // motor already running? - then stop
              if (iMotorState[ii]!=MOTOR_STOP)
              {
                SetMotor(ii,MOTOR_STOP);
              }
              else if (ij==0) { SetMotor(ii,MOTOR_UP); }
              else if (ij==1) { SetMotor(ii,MOTOR_DOWN); }
            }

            // store current command as old now
            lRFOldValue = value;              
          }
        }
      }            
    }
    mySwitch.resetAvailable();    
  }
 
  ///////////////////////////
  // debounce 433MHz RF input
  ///////////////////////////
  if (ulRFEventDeBounce<millis())
  {
    iRFCodeRepeat=0;
    lRFOldValue=0;
  }
 
  //////////////////////////////////////////
  // check for end timestamp of motor events
  //////////////////////////////////////////
  for (int ii=0;ii<NO_MOTOR;ii++)
  {
    if (ulMotorEventEnd[ii]<millis())
    {   
      // stop motor if required
      if (iMotorState[ii]!=MOTOR_STOP)
      {
        SetMotor(ii,MOTOR_STOP);
        iRFCodeRepeat=0;
        lRFOldValue=0;
      }
    }
  }
}

