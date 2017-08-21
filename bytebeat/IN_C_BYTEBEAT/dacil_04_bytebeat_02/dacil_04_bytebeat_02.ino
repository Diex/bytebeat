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
int r_l = 13;
int r_c = 12;
int r_d = 11;

int c_l = 8;
int c_c = 9;
int c_d = 10;


char row_0 = B00000001;
char row_1 = B00000010;
char row_2 = B00000100;
char row_3 = B00001000;
char row_4 = B00010000;
char row_5 = B00100000;
char row_6 = B01000000;
char row_7 = B10000000;

char  rows[] = {
  row_0, row_1 ,row_2 ,row_3 ,row_4 ,row_5 ,row_6 ,row_7 };

char buf[] = 
{
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,255,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0
};




void setup() {

  //  buffer = malloc(size * sizeof(char));
  //set pins to output so you can control the shift register
  pinMode(c_l, OUTPUT);
  pinMode(c_c, OUTPUT);
  pinMode(c_d, OUTPUT);

  pinMode(r_l, OUTPUT);
  pinMode(r_c, OUTPUT);
  pinMode(r_d, OUTPUT);

  digitalWrite(r_l, LOW);
  shiftOut(r_d, r_c, MSBFIRST, row_0 );       
  digitalWrite(r_l, HIGH);
}

unsigned char t = 1;
unsigned char count = 1;
unsigned char speed = 10;
int rowMux = 1;

void loop() { 

// delay(100);
 if(count == speed)  {
   t += 8;
   count = 0;
 }else
 {
   count ++;
 }
   

 bytebeat(  t );
 pushBuffer();
// delay(100);

// checkers();
}

void bytebeat(unsigned char t){  

  char col = 0;  
  char data = 0;
  unsigned char internal_t = t;

  for (int row = 0 ; row < 8 ; row++)
  {

    internal_t += row * 8; ;
    //    data += t  * row;

    for( int col = 0; col < 8 ; col++)
    {
//      data += t * col;
      internal_t += col;

//      data =  internal_t * (((internal_t>>12)|(internal_t>>8))&(63&(internal_t>>4)));
//       data =  internal_t & internal_t << 8; //
      data = (internal_t*(internal_t>>5|internal_t>>8))>>(internal_t>>16) ;
      loadBuffer(col, row, data);    
    }
  }
}

void loadBuffer(char x, char  y, char data)
{
  buf[y * 8 + x] = data;   
}

void pushBuffer()
{
  for (int row = 0 ; row < 8 ; row++)
  {
    digitalWrite(r_l, LOW);            
    shiftOut(r_d, r_c, MSBFIRST,  rows[row]);           

    digitalWrite(c_l, LOW);            
    for(int col = 0; col < 8; col++)
    {
      shiftOut(c_d, c_c, MSBFIRST,  getData(col,row));                 
    }
    
    digitalWrite(c_l, HIGH);
    digitalWrite(r_l, HIGH);
    delay(rowMux);
  }
}

char getData(char x, char y)
{
  return buf[x + y * 8];
}


void randomPattern()
{
  digitalWrite(c_l, LOW);
  for(int i = 0; i < 8; i ++)
  {
    shiftOut(c_d, c_c, MSBFIRST, random(255)  );             
  }  
  digitalWrite(c_l, HIGH);
}

void blank()
{
  digitalWrite(c_l, LOW);
  shiftOut(c_d, c_c, MSBFIRST, 0 );       
  digitalWrite(c_l, HIGH);  
}

void checkers()
{
  for (int row = 0 ; row < 8 ; row++)
  {
    digitalWrite(r_l, LOW);            
    shiftOut(r_d, r_c, MSBFIRST, rows[row] );           

    if(row % 2 == 0 ) 
    {
      patternA();
    }
    else
    {
      patternB();
    }

    digitalWrite(r_l, HIGH);
  }
}
void patternA()
{

  digitalWrite(c_l, LOW);
  for(int i = 0; i < 8; i ++)
  {
    shiftOut(c_d, c_c, MSBFIRST, 170  );             
  }  
  digitalWrite(c_l, HIGH);

}
void patternB()
{

  digitalWrite(c_l, LOW);

  for(int i = 0; i < 8; i ++)

  {
    shiftOut(c_d, c_c, MSBFIRST, 85  );
  }  
  digitalWrite(c_l, HIGH);
}


void scroll(){
  unsigned long l = 1;
  unsigned long n = 0;

  //lesss
  for (int b = 0 ; b < 16 ; b++)
  {

    digitalWrite(c_l, LOW);

    shiftOut(c_d, c_c, MSBFIRST, l  );        
    shiftOut(c_d, c_c, MSBFIRST, l >> 8   );       
    shiftOut(c_d, c_c, MSBFIRST, l >> 16  );
    shiftOut(c_d, c_c, MSBFIRST, l >> 24);                  
    digitalWrite(c_l, HIGH);    
    l <<= 1;
    delay(15);
  }
}





