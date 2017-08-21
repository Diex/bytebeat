//**************************************************************//
//  Name    : shiftOutCode, Hello World                                
//  Author  : Carlyn Maw,Tom Igoe, David A. Mellis 
//  Date    : 25 Oct, 2006    
//  Modified: 23 Mar 2010                                 
//  Version : 2.0                                             
//  Notes   : Code for using a 74HC595 Shift Register           //
//          : to count from 0 to 255                           
//****************************************************************

//Pin connected to ST_CP of 74HC595
int latchPin = 13;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

int rowDataPin = 10;

int rowClockPin = 9;

int rowLatchPin = 8;



void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(rowLatchPin, OUTPUT);
  pinMode(rowClockPin, OUTPUT);
  pinMode(rowDataPin, OUTPUT);

  Serial.begin(9600);
}

boolean toggle = true;

void loop() { 
    
  toggle = !toggle;    
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    byte a = B01010101;
    byte b = B10101010;
    byte number = toggle ? a : b;
//    Serial.print("latch: "); Serial.println(digitalRead(latchPin));
    for (int b = 0 ; b < 8 ; b++)
    {
     shiftOut(dataPin, clockPin, MSBFIRST, number);       

      vertical();

    }


    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);

    


    // pause before next value:
    //Serial.println(number);
   // Serial.print("latch: "); Serial.println(digitalRead(latchPin));
    
   // Serial.println("--------");
     
  //}
  
}


void vertical()
{
      for (int row = 0 ; row < 8 ; row++)
    {

     digitalWrite(rowLatchPin, LOW);
     shiftOut(rowDataPin, rowClockPin, MSBFIRST, 1 << row );       
     digitalWrite(rowLatchPin, HIGH);
     delay(2 );
    }
}
