    #include <MIDI.h>
    #include <MsTimer2.h>
    #include <SoftwareSerial.h>
    #include <limits.h>;


    
    // -----------------------------------------------------------------------------
    // midi
    // -----------------------------------------------------------------------------    
    #define MIDI_RX 8
    #define MIDI_TX 9

    SoftwareSerial SoftSerial(MIDI_RX, MIDI_TX);
    MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, MIDI);

    byte clockOut 	= 0;
    byte counter 	= 0;
    byte beat 		= 3; // cuento en fusas

    // -----------------------------------------------------------------------------
    // display
    // -----------------------------------------------------------------------------    

    #define colLatch 10
    #define colClock 11
    #define colData  12


    byte displayRows [8] = {1,1,1,1,1,1,1,1};





    void handleNoteOn(byte channel, byte pitch, byte velocity)
    {
        // Do whatever you want when a note is pressed.
        char str [255];
        sprintf( str, "channel: %d, \tpitch: %d, \tveloc: %d", channel, pitch, velocity );
        // Serial.println(str);        
    }

    void handleNoteOff(byte channel, byte pitch, byte velocity)
    {
        // Do something when the note is released.
        // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
    }

    void handleClock()
    {
        counter++;   

        if(counter % beat == 0){
            counter = 0;
            clockOut ++;
            for(int w = 0; w < 8; w++){
                displayRows[w] = (clockOut % (1 << w) == 0 ? displayRows[w] << 1 : displayRows[w]) % 255; 
            }            

            trigger(clockOut);
            renderLeds(displayRows);
        }
    }


    long lastTrigger = 0;

    void trigger(byte clockOut){

        digitalWrite(colLatch, LOW);
        shiftOut(colData, colClock, LSBFIRST, clockOut);        
        digitalWrite(colLatch, HIGH);    
        lastTrigger = millis();
    }


    





    // -----------------------------------------------------------------------------
    // gui

    int dataIn = 4;
    int load = 5;
    int clock = 6;

    int maxInUse = 1;    //change this variable to set how many MAX7219's you'll use

    int e = 0;           // just a variable

    // define max7219 registers
    byte max7219_reg_noop        = 0x00;
    byte max7219_reg_digit0      = 0x01;
    byte max7219_reg_digit1      = 0x02;
    byte max7219_reg_digit2      = 0x03;
    byte max7219_reg_digit3      = 0x04;
    byte max7219_reg_digit4      = 0x05;
    byte max7219_reg_digit5      = 0x06;
    byte max7219_reg_digit6      = 0x07;
    byte max7219_reg_digit7      = 0x08;
    byte max7219_reg_decodeMode  = 0x09;
    byte max7219_reg_intensity   = 0x0a;
    byte max7219_reg_scanLimit   = 0x0b;
    byte max7219_reg_shutdown    = 0x0c;
    byte max7219_reg_displayTest = 0x0f;

    void putByte(byte data) {
      byte i = 8;
      byte mask;
      while(i > 0) {
        mask = 0x01 << (i - 1);      // get bitmask
        digitalWrite( clock, LOW);   // tick
        if (data & mask){            // choose bit
          digitalWrite(dataIn, HIGH);// send 1
          }else{
          digitalWrite(dataIn, LOW); // send 0
      }
        digitalWrite(clock, HIGH);   // tock
        --i;                         // move to lesser bit
    }
}

void maxSingle( byte reg, byte col) {    
    //maxSingle is the "easy"  function to use for a single max7219

      digitalWrite(load, LOW);       // begin     
      putByte(reg);                  // specify register
      putByte(col);//((data & 0x01) * 256) + data >> 1); // put data   
      digitalWrite(load, LOW);       // and load da stuff
      digitalWrite(load,HIGH); 
  }

    void maxAll (byte reg, byte col) {    // initialize  all  MAX7219's in the system
    int c = 0;
      digitalWrite(load, LOW);  // begin     
      for ( c =1; c<= maxInUse; c++) {
      putByte(reg);  // specify register
      putByte(col);//((data & 0x01) * 256) + data >> 1); // put data
}
digitalWrite(load, LOW);
digitalWrite(load,HIGH);
}

void maxOne(byte maxNr, byte reg, byte col) {    
    //maxOne is for addressing different MAX7219's, 
    //while having a couple of them cascaded

    int c = 0;
      digitalWrite(load, LOW);  // begin     

      for ( c = maxInUse; c > maxNr; c--) {
        putByte(0);    // means no operation
        putByte(0);    // means no operation
    }

      putByte(reg);  // specify register
      putByte(col);//((data & 0x01) * 256) + data >> 1); // put data 

    for ( c =maxNr-1; c >= 1; c--) {
        putByte(0);    // means no operation
        putByte(0);    // means no operation
    }

      digitalWrite(load, LOW); // and load da stuff
      digitalWrite(load,HIGH); 
  }


  void setupMatrix () {

      pinMode(dataIn, OUTPUT);
      pinMode(clock,  OUTPUT);
      pinMode(load,   OUTPUT);

      digitalWrite(13, HIGH);  

    //initiation of the max 7219
    maxAll(max7219_reg_scanLimit, 0x07);      
      maxAll(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
      maxAll(max7219_reg_shutdown, 0x01);    // not in shutdown mode
      maxAll(max7219_reg_displayTest, 0x00); // no display test
       for (e=1; e<=8; e++) {    // empty registers, turn all LEDs off 
        maxAll(e,0);
        }
      maxAll(max7219_reg_intensity, 0x0f & 0x0f);    // the first 0x0f is the value you can set
                                                      // range: 0x00 to 0x0f
    }  


void renderLeds(byte* outputs){

        //if you use just one MAX7219 it should look like this
       maxSingle(1,outputs[0]);                       //  + - - - - - - -
       maxSingle(2,outputs[1]);                       //  - + - - - - - -
       maxSingle(3,outputs[2]);                       //  - - + - - - - -
       maxSingle(4,outputs[3]);                       //  - - - + - - - -
       maxSingle(5,outputs[4]);                      //  - - - - + - - -
       maxSingle(6,outputs[5]);                      //  - - - - - + - -
       maxSingle(7,outputs[6]);                      //  - - - - - - + -
       maxSingle(8,outputs[7]);                     //  - - - - - - - +

   }



 // ---------------------------------------
 // controls
       

    // ---------------------------------------
    void setup()
    {
        // Connect the handleNoteOn function to the library,
        // so it is called upon reception of a NoteOn.
        MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
        MIDI.setHandleNoteOff(handleNoteOff);
        MIDI.setHandleClock(handleClock);


        // Initiate MIDI communications, listen to all channels
        MIDI.begin(MIDI_CHANNEL_OMNI);
        MIDI.turnThruOn();

        pinMode(colLatch, OUTPUT);
        pinMode(colClock, OUTPUT);
        pinMode(colData, OUTPUT);

        setupMatrix();


        Serial.begin(115200);

        Serial.println("Setting up");

    }

    boolean useMidiClock = true;
    long bpmPeriod = 5208; //us/1/96 ticks


    void loop()
    {
        // Call MIDI.read the fastest you can for real-time performance.
        if(useMidiClock) {
    	    while(MIDI.read()); // engania pichanga
          
    	}else{
    		  handleClock();	
          delayMicroseconds(bpmPeriod);
    	}
        
    }
