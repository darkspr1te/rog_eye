/*
 * 
 *See README.md 
 *darkspr1te
 * 
 */
 
#include <Wire.h>

#define CPU_TYPE AMD

#if CPU_TYPE == AMD
  #define VDIV1 0.6475
  #define VDIV2 0.0025
#else
  #define VDIV1 1.28
  #define VDIV2 0.005
#endif

#define CDIV1 25.6
#define CDIV2 0.1

#define TVOLT    0x00
#define TCLOCK   0x01
#define TRATIO   0x02
#define TFAN     0x03
#define TTEMP    0x04
#define TQCODE   0x05
#define TOPEID   0x06

#define ROG_EXT 0x4A

#define ADDRESSES 11

typedef struct {
    int addr;
    char bytes;
    char type;
    char * name;
} addrStruct;
//From https://github.com/kevinlekiller/rogext.git
//for reference only as these structure are for talking to Elmor labs ROG_EXT_OUT module (REOM1B) [URL]Elmor Labs[https://elmorlabs.com/index.php/hardware/{/URL]
addrStruct addresses[ADDRESSES] = {
    {0x00, 1, TOPEID, "OPEID"},
//    {0x01, 1
//    {0x07, 1
    {0x10, 1, TQCODE, "QCODE"},
//    {0x12, 1
    {0x20, 1, TRATIO, "CPU Ratio"},
//    {0x22, 1
    {0x24, 1, TRATIO, "Cache Ratio"},
    {0x28, 2, TCLOCK, "BCLK"},
//    {0x2a, 2 ; PCIEBCLK?
//    {0x2c, 2
//    {0x2e, 2
    {0x30, 2, TVOLT,  "V1"},
//    {0x32, 2
//    {0x34, 2
//    {0x36, 2
    {0x38, 2, TVOLT,  "V2"},
//    {0x3a, 2
//    {0x3c, 2 ; 1.8v
//    {0x3e, 2
    {0x40, 2, TVOLT,  "VCORE"},
//    {0x42, 2
//    {0x44, 2
//    {0x46, 2
    {0x48, 2, TVOLT,  "VDRAM"},
//    {0x4c, 2
    {0x50, 1, TTEMP,  "CPU Temp"},
    {0x60, 2, TFAN,   "CPU Fanspeed"},
};

byte buf1, buf2;
#define MEM_SIZE 0xff
byte memory[MEM_SIZE] {};
byte alt_memory[MEM_SIZE] {};

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
  for (int count=0;count < MEM_SIZE;count++){memory[count]=0;}
  Wire.begin(ROG_EXT);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //Serial.begin(115200);           // start serial for output
  pinMode(SCL, INPUT_PULLUP);  // remove internal pullup
  pinMode(17,OUTPUT);
  digitalWrite(17, HIGH);
   Serial.println("testing");
  //Wire.end();
}

void dump_memory()
{
  Serial.println("  -=0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F=-");
 Serial.print("0");//that one dodgy zero, will fix later but this dirty fix i hope
  for (int row=0;row<0x10;row++)
  {
    //Serial.print(memory[row],HEX);
    Serial.print(row*0x10,HEX);
    Serial.print(" :");
   Serial.print(PrintHex8(memory,row*0x10,0x10));
    Serial.println("");
  }
  Serial.println(); 
}

void printVoltage()
{
  Serial.print(((memory[0x40] * VDIV1) + (memory[0x41] * VDIV2)), 4);
  Serial.print("V-CPU");
}

void printVoltage_ram()
{
  Serial.print(((memory[0x48] * VDIV1) + (memory[0x49] * VDIV2)), 4);
  Serial.print("V-DRAM");
}

void printFanSpeed()
{
  Serial.print((memory[0x60] << 8 | memory[0x61]));
  Serial.print("RPM");
}

void printTemp()
{
  Serial.print(memory[0x50]);
  Serial.print("'C ");
//  Serial.print(memory[0x51]);
//  Serial.print("C ");
}

void printClock()
{
  Serial.print(((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2)), 2);
  Serial.print("MHz");
}

void printClockRatio()
{
  int CPU_SPEED =0;
  Serial.print((memory[0x20]*((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2))));
  
  //Serial.print("x");
  //Serial.print(((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2)), 2);
  Serial.print("MHz");
}
void printRatio()
{
  Serial.print(memory[0x20]);
  Serial.print("x");
}

void printQcode()
{
  Serial.print("#");
  Serial.print(memory[0x10]);
}

void mem_comp()
{
  
for (int compare=0;compare<MEM_SIZE;compare++) 
  {
    if (alt_memory[compare]==memory[compare])
    {}
    else
    {
      Serial.print("0x");
      Serial.print(compare,HEX);
      Serial.print(" 0x");
      Serial.print(memory[compare],HEX);
      Serial.print(" 0x");
      Serial.print(alt_memory[compare],HEX);
      Serial.println("   --");
    }
  }
/*

*/

//  Serial.println();
  delay(100);
  for (int copy=0;copy <MEM_SIZE;copy++){alt_memory[copy]=memory[copy];}
}

void loop() {
  delay(200);
//  Serial.print("loop");
//dump_memory();
//working
Serial.print("CPU Temp ");
printTemp();
Serial.println();
Serial.print("CPU FAN1 ");
printFanSpeed();
Serial.println();
Serial.print("CPU  ");
printClockRatio();
Serial.println();
Serial.println("-------");

//not fully tested or measured
printQcode();
Serial.println();
//printRatio();
//Serial.println();
printVoltage();
Serial.println();
//printVoltage_ram();
//Serial.println();
Serial.println("======");
Serial.println();  
}

void receiveEvent(int howMany) {
  
  int bytes=0;
  int myvars[6]={0,0,0,0,0,0};

  int ROG_addr = 0;
  int ROG_value =0;


   //Serial.println("Start Event "); 
  while(Wire.available())    // slave may send less than requested
    { 
  
    myvars[bytes] = Wire.read();
    

 
    
    bytes++;
    }
  memory[myvars[0]]=myvars[1];
   // Serial.println();
//Serial.println("End Event ");

}

char PrintHex8(uint8_t *data,uint8_t start, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
     char tmp[16];
       for (int i=0; i<length; i++) { 
         sprintf(tmp, "0x%.2X",data[start+i]); 
         Serial.print(tmp); Serial.print(" ");
       }
}

void PrintHex16(uint16_t *data, uint8_t length) // prints 16-bit data in hex with leading zeroes
{
       char tmp[16];
       for (int i=0; i<length; i++)
       { 
         sprintf(tmp, "0x%.4X",data[i]); 
         Serial.print(tmp); Serial.print(" ");
       }
}
