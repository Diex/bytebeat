#include <TVout.h>
#include <fontALL.h>
#include "wiring_private.h"
#undef round

TVout TV;

long t = 0;
char i = 0;
int button = 5;

const int buffer_log_size = 7;
static unsigned char buffer[1 << buffer_log_size];
typedef unsigned char bufidx;
static bufidx front = 0;
static volatile bufidx rear = 0;

//static inline bufidx next(bufidx cur)
static inline unsigned char next(unsigned char cur)
{
  return (cur + 1) & ((1 << buffer_log_size) - 1);
}

static inline char put(char v)
{
  if (rear == next(front)) return 0;
  buffer[unsigned(front)] = v;
  front = next(front);
  return 1;
}

static inline char get(char *where)
{
  register bufidx r = rear;
  if (r == front) return 0;
  *where = buffer[unsigned(r)];
  rear = next(r);
  return 1;
}

// a place to stick a number to debug with
static unsigned char samples_spat_out_by_asm;

void our_hbi_hook() {
  if (++i != 2) return;
  i = 0;
  char c;
  if (get(&c)) {
    OCR2A = c;  // set PWM duty cycle
  } 
  else {
    samples_spat_out_by_asm++;
  }
}

// This stays interesting for ten minutes or so at least,
// and repeats about every twenty:
static inline char crowd()
{
  // unoptimized formula:
  // ((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7
  unsigned ut = unsigned(t);
  char t1 = char(ut) << 1;
  unsigned t7 = ut >> 7;
  long t12 = t >> 12;
  return (t1 ^ (t1 + t7 & t12)) | ut >> (4 - (1 ^ 7 & char(t12 >> 7))) | t7;
}

static inline char triangle_bells()
{
  // original by kragen
  //  char f = (unsigned char)
  //            ((unsigned char)
  //            ((unsigned char) (t >> 11) % (unsigned char) 63 ^ (0x15 + t >> 12)) %10*2)%13*4;


  char f = (unsigned char)
    ((unsigned char)
      ((unsigned char) (t >> 11) % (unsigned char) 63 ^ (0x15 + t >> 12)) % 10*2) % 12*4;
  // unoptimized formula:
  // return (f = ((t >> 11) % 63 ^ (0x15 + t >> 12)) %10*2%13*4, ((((t * f & 256) == 0) - 1 ^ t * f) & 255) >> ((t >> 7 + (t >> 13 & 1)) & 7));
  unsigned short ust = t;
  // Some notes on frequency.
  // ust >> 7 & 7 uses the last 10 bits of ust.  (Well, the top three of them, anyway.)
  // So it should repeat every 1024 samples.
  // ust >> 8 & 7 should repeat every 2048 samples.
  // ust >> 13 & 1 should, by the same logic, repeat every 16384 samples.
  // So the beat structure should be eight beats of 1024 samples followed by four beats of 2048 samples.
  // Audacity tells me that one of these 16384-sample beats runs from 1.000s in my recording to 3.341s.
  // So it's taking 2.341 seconds instead of the 2.048 seconds it should take, which is really fairly large.
  // I mean, that's 14%, about 2.3 semitones.  I'm only getting 6999 samples per second.
  // I'm not sure what to do about that.  Maybe skip calculating 14% of the samples?  Play 28% of the samples
  // for only a single scan line instead of two scan lines?
  // Audacity also shows a disturbing amount of high-frequency energy, including a peak at 12726Hz (0.55 samples)
  // at -37dB, louder than any of the easily audible sounds!
  // I see what looks like a regular oscillation with a period of about 3.3 of my 44.1kHz samples in Audacity,
  // which would be about 13kHz; I had chalked it up to PWM but I think it's too low-frequency.
  // And I may be imagining things but I think I can hear the 13kHz whine on the TV as well.
  return (((((ust * f & 256) == 0) - 1 ^ ust * f) & 255) >> ((ust >> 7 + (ust >> 13 & 1)) & 7));
}

int formula = 0;

void generate_samples()
{

  for (;;) {
    char ft;
    char sample;

    switch(formula){

    case 0:
      sample = t*(((t>>12)|(t>>8))&(63&(t>>4)));
      break;

    case 1:
      // This works okay and has an interesting rhythm, but I
      // wonder if it may be better off without the second line
      // and corresponding mixing:
      sample =  (((((t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1) & t>>12) ^ t>>16) & 255) >> 1 | ((t<<1&t>>9|t+1023>>9) & 255) >> 1;
      break;    

    case 2:
      // This innocent-looking formula used to be really slow until I added char().
      // Should repeat after about 4 hours.  Too bad it sounds terrible.
      sample = char(char(t)<<(7&(t>>12)))+(t<<1)&t>>9|t+(t>>10)>>9;
      break;

    case 3:
      // too slow, except when t was a 16-bit int:    
      //sample =  t^t%255;
      //break;

    case 4:
      // This is a microcontroller-friendly way to do t^t%255:
      sample = t ^ (char(t) + char(int(t) >> 8));
      break;

    case 5:
      // and a little more interesting:
      sample =  t ^ (char(t) + char(int(t) >> 8)) | short(t) >> 6 | t >> 9;
      break;

    case 6:
      // and more interesting still:
      sample = crowd();
      break;


    case 7:
      // skurk-raer: one of these two branches is too slow with longs:
      //sample =  ((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));
      //break;

    case 8:

      // optimized for microcontroller:
      sample = ((t&4096)?((t*(t^(char(t)+char(int(t)>>8)))|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));
      break;

    case 9:
      // this one ("laser boots") is still too slow:
      // sample = 255-((1L<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10);
      // break;

    case 10:
      // This bowdlerized version supposedly worked with the Arduino compiler on the NAML
      // machine, but on inexorable it's too slow:
      //      sample = 255-((1<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10);
      //    break;

    case 11:
      // Explore the space of all possible 8-beat rhythms in less than an hour.
      // It had some minor vsync problems.
      sample = (t<<1 ^ (t + (t >> 8))) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);
      break;

    case 12:
      // The same rhythm generator, with Ryg's Chaos Theory melody instead:
      sample = t*2*(char((t>>10)^(t>>10)-2)%11) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);
      break;

    case 13:
      // a sort of salsa beat; this one has some kind of incompatibility with JS:
      sample = (t*t>>(4-((t>>14)&7)))*t|(t&t-(2047&~(t>>7)))>>5|t>>3;
      break;

    case 14:
      // A triangle wave!
      sample = 8*t & 0x100 ? -8*t-1 : 8*t;      
      break;

    case 15:
      sample = t*(((t>>12)|(t>>8))&(63&(t>>4)));      
      break;

    case 16:
      sample = (t*5&t>>7)|(t*3&t>>10);      
      break;

    case 17:
      // Nice and rhythmic and interesting:
      sample = (t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1;
      break;

    case 18:
      // Simple reflected binary Gray code:
      sample = t ^ t >> 1;
      break;

    case 19:
      // pseudo-sinewave:
      sample = ((t & 15) * (-t & 15) ^ !(t & 16) - 1) + 128;
      break;

    case 20:
      sample = triangle_bells();
      break;


    case 21:
      // pseudo-sinewave bells:
      sample = (ft = t * (1 + (t >> 13 & 3 ^ t >> 18 & 3 ^ 1) << (t >> 15 & 1)), (((ft & 15) * (-ft & 15) ^ !(ft & 16) - 1) >> (t >> (7 + (t >> 13 & 3 ^ 1)) & 7)) + 128);
      break;

    case 22:
      // mmamm 01
      sample = (t & (t >> 7)) % 100 + (t  ^ (t >> 1 ) ) % 80; // + (t  | (t >> 113) ) % 75 + (t  ^ (t >> 81 ) ) % 80;
      break;

    case 23:
      // mmamm 02
      sample = (t << 31) * ( 1 + ( (t >> 11) & 7) ) * ((t>>13) % 13 * 31);
      break;

    case 24:
      //mmamm 03    
      sample = t*27*(13&t>>10) % 128;
      break;

    case 25:
      //mmamm 04
      sample = (t * 9 *(39 & (t >> 13))) % 127;
      break;

    case 26:
      //mmamm 05         
      sample =  (t * 9 *(39 & (t >> 13))) % 127 | t * 27 * (38 & (t >> 12 )) % 64 ;         
      break;


    case 27:
      //mmamm 06      
      sample =  (((t  * (t>>8 | t >> 5)) ) ^ 21 * (t>>1 | t >>2 | t>>3 | t>>5 | t >> 8 | t >> 13 | t >> 21) | 21  & 65 ) & (t>>11);
      break;
    }

    if (!put(sample)) break;
    t++;
  }
  //if (BUFFER_SIZE < 128) t += 128 - BUFFER_SIZE;
}

prog_char string_0[] PROGMEM =  "t*(((t>>12)|(t>>8))&(63&(t>>4)));";
prog_char string_1[] PROGMEM =  "(((((t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1) & t>>12) ^ t>>16) & 255) >> 1 | ((t<<1&t>>9|t+1023>>9) & 255) >>1;";
prog_char string_2[] PROGMEM =  "char(char(t)<<(7&(t>>12)))+(t<<1)&t>>9|t+(t>>10)>>9;";
prog_char string_3[] PROGMEM =  "t^t%255;";
prog_char string_4[] PROGMEM =  "t ^ (char(t) + char(int(t) >> 8));";
prog_char string_5[] PROGMEM =  "t ^ (char(t) + char(int(t) >> 8)) | short(t) >> 6 | t >> 9;";
prog_char string_6[] PROGMEM =  "crowd();";
prog_char string_7[] PROGMEM =  "((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));";
prog_char string_8[] PROGMEM =  "((t&4096)?((t*(t^(char(t)+char(int(t)>>8)))|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));";
prog_char string_9[] PROGMEM =  "255-((1L<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10);";
prog_char string_10[] PROGMEM =  "//null;";
prog_char string_11[] PROGMEM =  "(t<<1 ^ (t + (t >> 8))) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);";
prog_char string_12[] PROGMEM =  "t*2*(char((t>>10)^(t>>10)-2)%11) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);";
prog_char string_13[] PROGMEM =  "(t*t>>(4-((t>>14)&7)))*t|(t&t-(2047&~(t>>7)))>>5|t>>3;";
prog_char string_14[] PROGMEM =  "8*t & 0x100 ? -8*t-1 : 8*t;";
prog_char string_15[] PROGMEM =  "t*(((t>>12)|(t>>8))&(63&(t>>4)));";
prog_char string_16[] PROGMEM =  "(t*5&t>>7)|(t*3&t>>10);";
prog_char string_17[] PROGMEM =  "(t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1;";
prog_char string_18[] PROGMEM =  "t ^ t >> 1;";
prog_char string_19[] PROGMEM =  "((t & 15) * (-t & 15) ^ !(t & 16) - 1) + 128;";
prog_char string_20[] PROGMEM =  "triangle_bells();";
prog_char string_21[] PROGMEM = "ft = t * (1 + (t >> 13 & 3 ^ t >> 18 & 3 ^ 1) << (t >> 15 & 1)), (((ft & 15) * (-ft & 15) ^ !(ft & 16) - 1)>> (t >> (7 + (t >> 13 & 3 ^ 1)) & 7)) + 128);";
prog_char string_22[] PROGMEM =  "(t & (t >> 7)) % 100 + (t  ^ (t >> 1 ) ) % 80; // + (t  | (t >> 113) ) % 75 + (t  ^ (t >> 81 ) ) % 80;";
prog_char string_23[] PROGMEM =  "(t << 31) * ( 1 + ( (t >> 11) & 7) ) * ((t>>13) % 13 * 31);";
prog_char string_24[] PROGMEM =  "t*27*(13&t>>10) % 128;";
prog_char string_25[] PROGMEM =  "(t * 9 *(39 & (t >> 13))) % 127;";
prog_char string_26[] PROGMEM =  "(t * 9 *(39 & (t >> 13))) % 127 | t * 27 * (38 & (t >> 12 )) % 64 ;         ";
prog_char string_27[] PROGMEM =  "(((t  * (t>>8 | t >> 5)) ) ^ 21 * (t>>1 | t >>2 | t>>3 | t>>5 | t >> 8 | t >> 13 | t >> 21) | 21  & 65 ) & (;>>11);";

PROGMEM const char *names[] = 	   // change "string_table" name to suit
{ string_0,
string_1,
string_2,
string_3,
string_4,
string_5,
string_6,
string_7,
string_8,
string_9,
string_10,
string_11,
string_12,
string_13,
string_14,
string_15,
string_16,
string_17,
string_18,
string_19,
string_20,
string_21,
string_22,
string_23,
string_24,
string_25,
string_26,
string_27};

char formString[120];    // make sure this is large enough for the largest string it must hold

const int height = 86;
const int width = 120;

void setup()
{
  // audio setup
  pinMode(11, OUTPUT);
  pinMode(button, INPUT);
  TV.set_hbi_hook(&our_hbi_hook);

  // connect pwm to pin on timer 2
  sbi(TCCR2A, COM2A1);
  TCCR2B = TCCR2B & 0xf8 | 0x01; // no prescaling on clock select

  /////////////////////////
  TV.begin(NTSC, width, height);
  TV.select_font(font6x8);

  randomSeed(analogRead(0));
}


boolean button_on = false;
boolean showTitle = false;

int time = 2000; // ms
int lastTime = 0;
void loop()
{
  //  if(random(1, 1000) < 10)  {
  //    TV.fill(1); 
  //    delay(30);
  //    TV.clear_screen(); 
  //    delay(500);
  //    formula = random(0,27);
  //  }

  if(!digitalRead(button) && !button_on){
    button_on = true;
    formula = (formula + 1) % 27;    
    showTitle = true;    
  }

  if(digitalRead(button))  {
    button_on = false;
  }
  else
  {
    TV.println("...");
  }

  if(showTitle){
    if(!button_on){
      strcpy_P(formString, (char*)pgm_read_word(&(names[i]))); // 
      TV.println(formString);
      TV.delay(6000);
      showTitle = false;
      TV.clear_screen();
    }
  }
  else{
    for (unsigned char i = 10; i < height; i++) {
      generate_samples();
      unsigned char sample = buffer[i];
      TV.fill_line(i, 0, width, 0);
      TV.screen[i * width/8] = sample;
      TV.fill_line(i,
      width/2 - sample*5/32,
      width/2 + sample*5/32,
      1);
    }
  }
}






