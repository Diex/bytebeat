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
int r_latch = 8;
//Pin connected to SH_CP of 74HC595
int r_clock = 9;
////Pin connected to DS of 74HC595
int r_data = 10;

//Pin connected to ST_CP of 74HC595
int c_latch = 13;
//Pin connected to SH_CP of 74HC595
int c_clock = 12;
////Pin connected to DS of 74HC595
int c_data = 11;


void setup() {
  //set pins to output so you can control the shift register
  pinMode(c_latch, OUTPUT);
  pinMode(c_clock, OUTPUT);
  pinMode(c_data, OUTPUT);

  pinMode(r_latch, OUTPUT);
  pinMode(r_clock, OUTPUT);
  pinMode(r_data, OUTPUT);

}


void loop() {

  for(int i = 0 ; i < 8 ; i ++){    
    digitalWrite(c_latch , LOW);

    
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    shiftOut(c_data, c_clock, MSBFIRST, 255);  
    
    digitalWrite(c_latch, HIGH);   
  
    digitalWrite(r_latch, LOW);
    shiftOut(r_data, r_clock, MSBFIRST, 1 << i);      
    digitalWrite(r_latch, HIGH);    

//    delay(1);
  }

}

