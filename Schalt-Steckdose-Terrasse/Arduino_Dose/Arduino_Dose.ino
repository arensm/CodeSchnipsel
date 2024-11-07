/////////////////////////////////////////////////////////
// Project ESP8266 with Arduino
// Communication over serial
//
// Part 2 Arduino to deploy more GPIOs
//
// mail: droidbox.ma () gmail.com
//
// Free for anybody - no warranties
/////////////////////////////////////////////////////////

#define RECV_BUFFER_SIZE 20       // Receive-Buffer - for Softserial / WIFI reading
char RECV_Buffer[RECV_BUFFER_SIZE];

#define WIFI_Serial mySerial      // assign serial ports
#define Debug_Serial Serial

#include <SoftwareSerial.h>
SoftwareSerial mySerial(3,4); // RX, TX

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
unsigned long lRFOldValue=0;      // to check for consecutive reads on 433MHz
int iRFCodeRepeat=0;              // how many repeated codes receiver of 433MHz?
#define ESP_RESET 5               // ESP8266 reset pin
#define NO_DOSE 4                 // Numbers of DOSE to control, 4 Dosen = 0-3
unsigned long RFCode[NO_DOSE][2]; // array for the fc receive codes
int iDOSEState[NO_DOSE];          // state of the DOSES
#define DOSE_ON     1             // -1
#define DOSE_OFF    0             // 1
#define DOSE_INVAL -999
#define DEBOUNCE_MS 1000
unsigned long ulDOSEEventEnd[NO_DOSE];  // end of current motor event
unsigned long ulRFEventDeBounce;  // timer for RF event on 433MHz (needed for debouncing)

////////////////
// setup routine
void setup()
  {
    pinMode(14, OUTPUT); digitalWrite(14, HIGH);
    pinMode(15, OUTPUT); digitalWrite(15, HIGH);
    pinMode(16, OUTPUT); digitalWrite(16, HIGH);
    pinMode(17, OUTPUT); digitalWrite(17, HIGH);
    
    // init serials - make sure ESP8266 on WIFI_Serial is also at 9600
    Debug_Serial.begin(9600);
    WIFI_Serial.begin(9600);

    // set-up rf receiver
    mySwitch.enableReceive(0);  // 433MHz Receiver on interrupt 0 => that is pin #2
    ulRFEventDeBounce=0;
    Debug_Serial.println("Aussensteckdosen Steuerung 2018.05");
  
    ////////////////////////////
    // define RF receiver codes  -  place your codes here, if you want 433MHz support
    ////////////////////////////
    // Pollin FB Outdoor
    // OFF OFF OFF OFF OFF
    // 00000
    // Nr  ON    OFF
    // 1 5588305    5588308
    // 2 5591377    5591380
    // 3 5592145    5592148
    // 4 5592337    5592340
    // left button  ON
    // right button OFF
    RFCode[0][0]=5588308;  // Switch A left ON
    RFCode[0][1]=5588305;  // Switch A left OFF
    RFCode[1][0]=5591380;  // Switch B left ON
    RFCode[1][1]=5591377;  // Switch B left OFF
    RFCode[2][0]=5592148;  // Switch C left ON
    RFCode[2][1]=5592145;  // Switch C left OFF
    RFCode[3][0]=5592340;  // Switch D left ON
    RFCode[3][1]=5592337;  // Switch D left OFF
   
    pinMode(ESP_RESET, OUTPUT);   // reset ESP8266
    digitalWrite(ESP_RESET, LOW);
    delay(500);
    digitalWrite(ESP_RESET, HIGH);
  
  }

////////////
// main loop
void loop()
  {
    //////////////
    // handle WIFI
    if(WIFI_Serial.available())
      {
        if (WIFI_Serial.findUntil("DOSE", "\n"))
          {
            if (WIFI_Serial.readBytesUntil('\n', RECV_Buffer, RECV_BUFFER_SIZE))
          {
         // we should now have the motor number in ascii in receive buffer
         int iDOSE = RECV_Buffer[0] -48 -1; // -1 as webinterface calls DOSE0 .... DOSE9
         int iCommand=DOSE_INVAL;
         if (iDOSE>=0 && iDOSE<=9)
           {
             Debug_Serial.print("DOSE ");
             Debug_Serial.print(iDOSE);
              if (strncmp(&RECV_Buffer[1], "ON", 2) == 0)
                {
                 iCommand=DOSE_ON;
                }
                  else if (strncmp(&RECV_Buffer[1], "OFF", 3) == 0)
                    {
                    iCommand=DOSE_OFF;
                    }
              Debug_Serial.print(" Komando ");
              Debug_Serial.println(iCommand);
          
              if (iDOSE==0 && iCommand==0)
                { digitalWrite(14, HIGH); }
                  else if (iDOSE==0 && iCommand==1)
                { digitalWrite(14, LOW);  }

              if (iDOSE==1 && iCommand==0)
                { digitalWrite(15, HIGH); }
                  else if (iDOSE==1 && iCommand==1)
                { digitalWrite(15, LOW);  }

              if (iDOSE==2 && iCommand==0)
                { digitalWrite(16, HIGH); }
                  else if (iDOSE==2 && iCommand==1)
                { digitalWrite(16, LOW);  }

              if (iDOSE==3 && iCommand==0)
                { digitalWrite(17, HIGH); }
                  else if (iDOSE==3 && iCommand==1)
                { digitalWrite(17, LOW);  }
             }
          }
        }
    }
////////////////
  // handle 433MHz
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
      for (int ii=0;ii<NO_DOSE;ii++)
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
              
              if (ii==0 && ij==0)
                { digitalWrite(14, HIGH); }
                  else if (ii==0 && ij==1)
                { digitalWrite(14, LOW);  }

              if (ii==1 && ij==0)
                { digitalWrite(15, HIGH); }
                  else if (ii==1 && ij==1)
                { digitalWrite(15, LOW);  }

              if (ii==2 && ij==0)
                { digitalWrite(16, HIGH); }
                  else if (ii==2 && ij==1)
                { digitalWrite(16, LOW);  }

              if (ii==3 && ij==0)
                { digitalWrite(17, HIGH); }
                  else if (ii==3 && ij==1)
                { digitalWrite(17, LOW);  }
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
  if (ulRFEventDeBounce<millis())
    {
      iRFCodeRepeat=0;
      lRFOldValue=0;
    }
 }

