//**************************************************************//
//  Name    : shiftOutCode, Hello World                                
//  Author  : Carlyn Maw,Tom Igoe, David A. Mellis 
//  Date    : 25 Oct, 2006    
//  Modified: 23 Mar 2010                                 
//  Version : 2.0                                             
//  Notes   : Code for using a 74HC595 Shift Register           //
//          : to count from 0 to 255                           
//****************************************************************
#include <limits.h>;

int rowLatch = 13;
int rowClock = 12;
int rowData = 11;

int colLatch = 8;
int colClock = 9;
int colData = 10;

#define row_0 B00000001
#define row_1 B00000010
#define row_2 B00000100
#define row_3 B00001000
#define row_4 B00010000
#define row_5 B00100000
#define row_6 B01000000
#define row_7 B10000000

char  rows[] = {row_0, row_1 ,row_2 ,row_3 ,row_4 ,row_5 ,row_6 ,row_7};

// voy a dibujar la data por filas (en vertical)
#define numColumns 32
unsigned char line = 255;
char data[numColumns] = {};
// char buffe[numColumns] = {};

unsigned long lines[8] = {};

// int rowMux = 250;


void setup() {
	pinMode(colLatch, OUTPUT);
	pinMode(colClock, OUTPUT);
	pinMode(colData, OUTPUT);

	pinMode(rowLatch, OUTPUT);
	pinMode(rowClock, OUTPUT);
	pinMode(rowData, OUTPUT);
}


unsigned long iterations = 0;
// unsigned long last = 0;
// int frame = 10;
unsigned long l;

static inline char viznut(long t)
{
	unsigned ut = unsigned(t);
	return ut*(((ut>>12)|(ut>>8))&(63&(ut>>4)));
}

void verticalColumn(char i){
	digitalWrite(rowLatch, LOW);            
	shiftOut(rowData, rowClock, MSBFIRST,  rows[i]);           
	digitalWrite(rowLatch, HIGH);
}

void verticalRow(unsigned long data){		
	
	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, MSBFIRST, data  );        
	shiftOut(colData, colClock, MSBFIRST, data >> 8 );        
	shiftOut(colData, colClock, MSBFIRST, data >> 16 );        
	shiftOut(colData, colClock, MSBFIRST, data >> 24 );  
	digitalWrite(colLatch, HIGH);    

}
void loop() {   
	
	// if(millis() - last  > frame){
	// 	iterations = (iterations + 1);
	// 	last = millis();		
	// }
	
	iterations = (iterations + 1);
	line = viznut(iterations); //bytebeat(iterations);

	for(int r = 0; r < 8; r++){
		verticalColumn(r);	
		lines[r] <<= 1;
		lines[r] |= line >> r & 1;
		verticalRow(lines[r]);	
		delay(2);
	}
	 
}

static inline char crowd(long t)
{
  // unoptimized formula:
  // ((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7
  unsigned ut = unsigned(t);
  char t1 = char(ut) << 1;
  unsigned t7 = ut >> 7;
  long t12 = t >> 12;
  return (t1 ^ (t1 + t7 & t12)) | ut >> (4 - (1 ^ 7 & char(t12 >> 7))) | t7;
}

static inline char sierpinski(long t)
{
	unsigned ut = unsigned(t);
	return ut & ut >> 8;
}




/*
void verticalRowOn(char line){		
	unsigned long l = 1L << line;
	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, MSBFIRST, l  );        
	shiftOut(colData, colClock, MSBFIRST, l >> 8 );        
	shiftOut(colData, colClock, MSBFIRST, l >> 16 );        
	shiftOut(colData, colClock, MSBFIRST, l >> 24 );  
	digitalWrite(colLatch, HIGH);    

}


void rowMultiplex(unsigned long iterations){
	for (int row = 0 ; row < 8 ; row++)
	{
		// --------------------------
    	// aca dibujo la columna		
		// drawCheckerboard(row + iterations);
		// drawRandom();
		drawMarquee(iterations);
		// drawBytebeat(iterations, row);

		//  luego actualizco la fila
		digitalWrite(rowLatch, LOW);            
		shiftOut(rowData, rowClock, MSBFIRST,  rows[row]);           
		digitalWrite(rowLatch, HIGH);


    	// --------------------------
    	// no pierdo tiempo aca no hace falta
    	// delayMicroseconds(rowMux);
    }
}

void drawColumn(){
	unsigned long l = ULONG_MAX;
	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, MSBFIRST, l  );        
	digitalWrite(colLatch, HIGH);      
}

void drawCheckerboard(char row){
	unsigned long l = 0L;

	for(int i = 0; i < numColumns; i++){
		l |= (i % 2 == 0) ? 1L << i : 0L << i; 
	}
	l = row % 2 == 0 ? l << 1 : l << 0;

	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, LSBFIRST, l  );        
	shiftOut(colData, colClock, LSBFIRST, l >> 8 );        
	shiftOut(colData, colClock, LSBFIRST, l >> 16 );        
	shiftOut(colData, colClock, LSBFIRST, l >> 24 );  
	digitalWrite(colLatch, HIGH);    


}

void drawRandom(){
	unsigned long l = 0L;
	for(int i = 0; i < numColumns; i++){
		l |= (random(0, 1000) > 500 ) ? 1L << i : 0L << i; 
	}
	
	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, LSBFIRST, l  );        
	shiftOut(colData, colClock, LSBFIRST, l >> 8 );        
	shiftOut(colData, colClock, LSBFIRST, l >> 16 );        
	shiftOut(colData, colClock, LSBFIRST, l >> 24 );  
	digitalWrite(colLatch, HIGH);      	

}

void drawMarquee(long iteration){
	unsigned long l = 0;
	iteration = iteration % numColumns;

	for(char i = 0; i < numColumns; i++){
		l |= (i == iteration) ? 1L << i : 0; 
	}
	
	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, MSBFIRST, l  );        
	shiftOut(colData, colClock, MSBFIRST, l >> 8 );        
	shiftOut(colData, colClock, MSBFIRST, l >> 16 );        
	shiftOut(colData, colClock, MSBFIRST, l >> 24 );  
	digitalWrite(colLatch, HIGH);      	

}




void blank()
{
	digitalWrite(colLatch, LOW);
	shiftOut(colData, colClock, MSBFIRST, 0 );       
	digitalWrite(colLatch, HIGH);  
}

*/



