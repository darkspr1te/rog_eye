

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <Wire.h>
#define ROG_VERSION 0.15
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

#define ROG_EXT 0x4a
//currently only known 
#define ADDRESSES 11




byte buf1, buf2;
#define MEM_SIZE 0x1fe
byte memory[MEM_SIZE] {};
byte alt_memory[MEM_SIZE] {};


/*
 * pin Outs At ROG Connector
1 - grn - 18 cs 
2 - red - 21 clk
3 - org - 20 data/MOSI
4 - yel - 19 DC
5 - blk - 17 ?rst maybe? 
6 - pur - 16 backlight 
 */
/*
 * for AT MEGA use pins
#define TFT_MOSI 51  // Data out
#define TFT_SCLK 52  // Clock out
#define TFT_CS 53 // CS
#define TFT_RST 17
#define TFT_DC 48
//#define TFT_BACKLIGHT 46
#define TFT_BACKLIGHT 8

for uno etc use :- clk = 13,12 MISO,11 MOSI, cs = 10 
for ESP use 
D5,D6,D7,D8 plus one for back light like D1
*/

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  7
#define TFT_BACKLIGHT 8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC,TFT_RST);
//i2cdump 1 0x4a
/*
 * example dump found online
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 00 0f 00 00 00 00 00 04 00 00 00 00 00 00 00 00    .?.....?........
10: 00 00 06 00 00 00 00 00 00 00 00 00 00 00 00 00    ..?.............
20: 25 25 3f 20 00 00 00 00 03 e8 03 e8 0b b8 01 90    %%? ....????????
30: 01 90 00 c8 01 68 00 96 01 68 01 68 02 d0 01 2c    ??.??h.??h?h???,
40: 01 15 01 14 01 54 00 96 02 2a 00 f0 01 e0 00 64    ?????T.??*.???.d
50: 22 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    "...............
60: 00 fc 00 00 00 00 00 00 00 00 00 00 00 00 00 00    .?..............
70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
d0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
*/
typedef struct {
    int addr;
    char bytes;
    char type;
    char * name;
} addrStruct;

addrStruct addresses[ADDRESSES] = {
    {0x00, 1, TOPEID, "OPEID"},
//    {0x01, 1
 //   [0x06 ,1, POSTMODE,"POST MODE"},
//    {0x07, 1
//0x10 location Valid for Intel Only it seems or C6H and prior boards. possible candidate is 0x22
    {0x10, 1, TQCODE, "QCODE"},
//    {0x12, 1
    {0x20, 1, TRATIO, "CPU Ratio"},
//    {0x22, 1
// 0x22 could be AMD qcode location 
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
// {0xff,1,UNKWN,"UNKWN"},
};


float p = 3.1415926;

void receiveEvent(int howMany) {
  
  int bytes=0;
  int myvars[6]={0,0,0,0,0,0};

  int ROG_addr = 0;
  int ROG_value =0;
//The ROG_EXT enable boards write to device 0x4A with the values of cpu v,temp etc. from 0x00 to 0xff , if it's two bit i2c address then i've not figured the write past 0xff trigger yet
//normally receive two bytes from i2c , first is memory location and rest is data
//arduino acts as I2c memory device and then we read that memory[x] back and display it, simple really , dont need the Elmor/Overlay.live labs snake oil  
 
 //For Debug, send serial command every time the Motherboard makes a request 
 //Serial.println("Start Event "); 
  while(Wire.available())    // slave may send less than requested
    { 
      myvars[bytes] = Wire.read();
      bytes++;
    }
  //first byte is memory location that the Motherboard is writing to , second is value 
  memory[myvars[0]]=myvars[1];

  //debug output
   // Serial.println();
  //Serial.println("End Event ");

}
void dump_memory()
{
  //not really usefull function on small screen, works best on larger screen to see all/most of memory locations change or use serial and a good terminal app that support ansi functions (BBS RULEZ)
  
  tft.fillScreen(ST77XX_BLACK);  
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  //my screen did not have enought space, but it's here for other screens
  tft.print("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
  for (int row=0;row<0x10;row++)
  {
    //some debug
    //Serial.print(memory[row],HEX);
   // Serial.print(row*0x10,HEX);
   //  Serial.print(" :");
   //   Serial.print(PrintHex8(memory,row*0x10,0x10));
   //   Serial.println("");

  tft.setTextWrap(true);
  tft.print(memory[row],HEX);
  tft.print(row*0x10,HEX);
  tft.print(":");

  }
  
}


void display_knowns()
{
  //test routine to display most known values
  //lower section displays unknown memeory locations 
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.print("CPU V"); tft.print(((memory[0x40] * VDIV1) + (memory[0x41] * VDIV2)), 4);
 
  tft.setCursor(0,10);
  tft.print("DRAM V");tft.print(((memory[0x48] * VDIV1) + (memory[0x49] * VDIV2)), 4);
  
  tft.setCursor(0,20);
  tft.print("CPU FAN RPM ");tft.print((memory[0x60] << 8 | memory[0x61]));
//a little fun with some colours start with highest number then work down if using lazy IF chains, bad programmer btw :-) 
  if (memory[0x50]>70)
  {
     tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setCursor(0,30);
  tft.print("CPU Temp C ");
  tft.print(memory[0x50]);
  tft.print(" ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  else 
  if (memory[0x50]>50)
  {
    tft.setTextColor(ST77XX_ORANGE, ST77XX_BLACK);
    tft.setCursor(0,30);
    tft.print("CPU Temp C ");
    tft.print(memory[0x50]);
    tft.print(" ");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  else 
   {
    tft.setCursor(0,30);
    tft.print("CPU Temp C ");
    tft.print(memory[0x50]);
    tft.print("    ");
    }
  
  tft.setCursor(0,40);
  tft.print("BLCLK MHz ");tft.print(((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2)), 2);

  tft.setCursor(0,50);
  tft.print("CPU Mhz ");tft.print((memory[0x20]*((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2))));

  tft.setCursor(0,60);
  tft.print("Clk Multiplier x");tft.print(memory[0x20]);
  
  tft.setCursor(0,70);
  //intel QCODE location tft.print("QCODE #");tft.print(memory[0x10]);
  tft.print("QCODE 1#");
  tft.setTextColor(ST77XX_RED, ST77XX_WHITE);
  tft.print(memory[0x22],HEX);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  
  tft.setCursor(0,80);
  //memory location 0x06 is 1 when system is in POST, 0 when booting
  if (memory[0x6]==1)
    {
      //System is still in EUFI setup mode
      tft.setTextColor(ST77XX_ORANGE, ST77XX_BLACK);
      tft.print("EUFI Mode: ");
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    }
  else
  {
    // System has completed POST and is now booting 
    tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
    tft.print("BOOT Mode: ");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  for (int xn=0;xn<7;xn++)
  {
    tft.print(memory[xn],HEX);
    tft.print(" ");
  }
  tft.setCursor(0,90);
  //this is looking at specific memory locations 
  //tft.print("MEM ");
  //Specific memory locations to be monitored, reduces screen clutter
  //tft.print("0x7:");tft.print(memory[0x7]);
  //tft.print(" 0x22:");tft.print(memory[0x22]);
  //tft.print(" 0xff:");tft.print(memory[0xff]);

  //tft.setCursor(0,100);
  //this section just displays a memory dump,few locations , monitor during adjustment/usage/boot process 
  int start =0x20;
    for (int xn=start;xn<(start+0xf);xn++)
  {
    tft.print(memory[xn],HEX);
    tft.print(":");
  }
  tft.setCursor(0,110);
   int starttwo =0x30;
    for (int xn=starttwo;xn<(starttwo+0xf);xn++)
  {
    tft.print(memory[xn],HEX);
    tft.print(":");
  } 
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("Rog Eye, Open Source ROG_EXT adapter V");
  Serial.println(ROG_VERSION);

  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
  

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  // large block of text
  //setRoation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1); //MPT-7210A rotation
  for (int count=0;count < MEM_SIZE;count++){memory[count]=0;}
  Wire.begin(ROG_EXT);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //Serial.begin(115200);           // start serial for output
  pinMode(SCL, INPUT_PULLUP);  // remove internal pullup
  //On my screen i have to enable backlight
  pinMode(17,OUTPUT);
  digitalWrite(17, HIGH);

}

void loop() {
  delay(50);
 display_knowns();
}