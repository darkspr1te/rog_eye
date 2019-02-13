

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <Wire.h>
#define ROG_VERSION 0.18
/*
#define CPU_TYPE AMDKK

#if CPU_TYPE == INTEL_X
  #define VDIV1 0.6475
  #define VDIV2 0.0025
#else
  #define VDIV1 1.28
  #define VDIV2 0.005
#endif
*/

//Docs online say vdiv 0.6475/0.00025 for amd but i found 1.28/0.005 displayed correct for C7H/2700x AMD 
//Voltage divider for CPUV/DRAMV
#define VDIV1 1.28
#define VDIV2 0.005
//clock divider fro BLCLK 
#define CDIV1 25.6
#define CDIV2 0.1

#define TVOLT    0x00
#define TCLOCK   0x01
#define TRATIO   0x02
#define TFAN     0x03
#define TTEMP    0x04
#define TQCODE   0x05
#define TOPEID   0x06

//this is the i2c address the system writes to, there are others on the bus but for saftey , DONT WRITE TO THE BUS 
#define ROG_EXT 0x4a
//currently only known addresses, this is legacy from github.com/kevinlekiller , i dont understand its usage in code along with the struct but it's layout has been handy for notes
#define ADDRESSES 11




byte buf1, buf2;
//memory so far is only writing up to 0xff , option here thought ot double that for testing purposes only , save ram, it's a-rduino 
//#define MEM_SIZE 0x1fe
#define MEM_SIZE 0xff
byte memory[MEM_SIZE] {};
byte alt_memory[MEM_SIZE] {};
byte REQ_REC;

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
#define ROG_ENABLE 17 

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

/*
Chipset   Motherboard Model   Full Function Partial Function  Remark
Intel Z97 Maximus VII Formula           P         *V1=In put Voltage; V2=Cache Voltage
Intel Z97 Maximus VII Gene              P           *V1=In put Voltage; V2=Cache Voltage
Intel Z97 Maximus VII Hero              P           *No V1/V2, Only have BCLK, Hotwire.
Intel Z97 Maximus VII Impact            P         *V1=In put Voltage; V2=Cache Voltage
Intel Z97 Maximus VII Ranger            P         *No V1/V2, Only have BCLK, Hotwire.
Intel X79 Rampage IV Black Edition      P         *V1=VTT_CPU; V2=DRAM_AB; DRAM=DRAM_CD
Intel X99 Rampage V Extreme 　          P           *V1=In put Voltage; V2=DRAM_AB; DRAM=DRAM_CD
Intel Z170  Maximus VIII Extreme        V             　 *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Extreme Assembly V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Hero       V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Hero Alpha     V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Ranger       V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Gene       V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Impact       V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Intel Z170  Maximus VIII Formula      V 　             *V1=SA(System Agent Voltage); V2=IO(VCCIO)
Function differs between OC/Panel 1/2, Front Base, OC Panel(silver buttons) devices.
Seems Mobo firmware related and not really device related 

My Dump on differs at byte 0x6 (1 during post) and 0xff which is random 
other than that they are all the same apart from variable mem locations, eg fan speeds at 0x60 High byte and 0x61 lower byte 




addrStruct addresses[ADDRESSES] = {
    {0x00, 1, TOPEID, "OPEID"},
//    {0x01, 1
 //   [0x06 ,1, POSTMODE,"POST MODE"},
//    {0x07, 1
//0x10 location Valid for Intel Only it seems or C6H and prior boards. possible candidate is 0x??
    {0x10, 1, TQCODE, "QCODE"},
//    {0x12, 1
    {0x20, 1, TRATIO, "CPU Ratio"},
//  {0x21,1,unknwn,"SAME AS CPU RATIO"},
//    {0x22, 1
// 0x22 ,1, ??could be AMD qcode location 
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
*/

float p = 3.1415926;

void receiveEvent(int howMany) {
  
  byte bytes=0;
  byte ROG_Bytes[6]={0,0,0,0,0,0};

//  byte ROG_addr = 0;
//  byte ROG_value =0;
//The ROG_EXT enable boards write to device 0x4A with the values of cpu v,temp etc. from 0x00 to 0xff , if it's two bit i2c address then i've not figured the write past 0xff trigger yet
//normally receive two bytes from i2c , first is memory location and rest is data
//arduino acts as I2c memory device and then we read that memory[x] back and display it, simple really , dont need the Elmor/Overlay.live labs snake oil  
 
 //For Debug, send serial command every time the Motherboard makes a request 
 //Serial.println("Start Event "); 
  while(Wire.available())    // master may send less than requested
    { 
      ROG_Bytes[bytes] = Wire.read();
      bytes++;
    }
  //first byte is memory location that the Motherboard is writing to , second is value 
  memory[ROG_Bytes[0]]=ROG_Bytes[1];

  //debug output
    Serial.print(ROG_Bytes[0],HEX); Serial.print(" ");Serial.print(ROG_Bytes[1],HEX);Serial.print(" ");Serial.println(bytes);
  //Serial.println("End Event ");

}

void requestEvent() {
  
  REQ_REC=1;
  Serial.println("req Event ");

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
  tft.print("CPU V"); tft.print(((memory[0x40] * VDIV1) + (memory[0x41] * VDIV2)),4);
  tft.print(" ¯ ");
  switch (memory[0x2]) {
        case 0x1:
              tft.print("M6E Intel");
              break;
        case 0x2:
              tft.print("C6H AMD");
              break;
        case 0xF:
              tft.print("C7H AMD");
              break;
        default:
              tft.print("reboot");
              break;
  }   
              
               
   
 
  tft.setCursor(0,10);
  tft.print("DRAM V");tft.print(((memory[0x48] * VDIV1) + (memory[0x49] * VDIV2)),4);
  
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
    tft.print("CPU Mhz ");tft.print((memory[0x20]*((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2))));


  tft.setCursor(0,50);
  //tft.print("BLCLK MHz ");tft.print(((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2)), 2);
  //TVolt 1
  //CPU SOC on AMD C7H board
  tft.print("CPU SOC ");tft.print(((memory[0x30] * VDIV1) + (memory[0x31] * VDIV2)),3);

  tft.setCursor(0,60);
  //tft.print("Clk Multiplier x");tft.print(memory[0x20]);
  //VCCIO = Tvolt 2
  tft.print("VCCIO  ");tft.print(((memory[0x38] * VDIV1) + (memory[0x39] * VDIV2)),3);
  
  tft.setCursor(0,70);
  //intel QCODE location tft.print("QCODE #");tft.print(memory[0x10]);
  tft.print("QCODE 1#");
  tft.setTextColor(ST77XX_RED, ST77XX_WHITE);
  tft.print(memory[0x10],HEX);
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
  tft.print("MEM ");
  //Specific memory locations to be monitored, reduces screen clutter
  tft.print("0x40:");tft.print(memory[0x40]);
  tft.print(" 0x41:");tft.print(memory[0x41]);
  tft.setCursor(0,100);
    tft.print("MEM ");
  //Specific memory locations to be monitored, reduces screen clutter
  tft.print("0x48:");tft.print(memory[0x48]);
  tft.print(" 0x49:");tft.print(memory[0x49]);
  /*
   #define VDIV1 0.6475
   #define VDIV2 0.0025
   */
  //tft.print(" 0xff:");tft.print(memory[0xff]);

  tft.setCursor(0,110);
  //this section just displays a memory dump,few locations , monitor during adjustment/usage/boot process 
  
  int start =0x0;
    for (int xn=start;xn<(start+0x10);xn++)
  {
    tft.print(memory[xn],HEX);
    tft.print(":");
  }
  /*
  tft.setCursor(0,110);
   int starttwo =0x30;
    for (int xn=starttwo;xn<(starttwo+0xf);xn++)
  {
    tft.print(memory[xn],HEX);
    tft.print(":");
  } 
  */
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("Rog Eye, Open Source ROG_EXT adapter V");
  Serial.println(ROG_VERSION);
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1); //MPT-7210A rotation
  for (int count=0;count < MEM_SIZE;count++){memory[count]=0;}
  Wire.begin(ROG_EXT);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  REQ_REC=0;
  pinMode(SCL, INPUT_PULLUP);  // remove internal pullup
  //On my screen i have to enable backlight
  pinMode(ROG_ENABLE,OUTPUT);
  digitalWrite(ROG_ENABLE, HIGH);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

}

void loop() {
  delay(5);
  if (REQ_REC==1)
  {
    tft.invertDisplay(true);
  }
 display_knowns();
}