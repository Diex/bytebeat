// Based on http://arduino.cc/en/Tutorial/Fading by David A. Mellis 
// and Tom Igoe, http://www.arduino.cc/playground/Code/PwmFrequency,
// and viznut's, skurk's, and raer's discoveries documented in 
// http://countercomplex.blogspot.com/2011/10/algorithmic-symphonies-from-one-line-of.html

int speakerPin = 9;    // Speaker connected to digital pin 9

void setup()  { 
  // use 0x01 for divisor of 1, i.e. 31.25kHz
  // or 0x02 for divisor of 8, or 0x03 for divisor of 64 (default)
  TCCR1B = TCCR1B & 0xf8 | 0x01;
} 

long t = 0;
int i = 0;

void loop() {
  if (++i != 64) return;
  i = 0;
  t++;
  
  analogWrite(speakerPin, 255-((1<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10));
}

