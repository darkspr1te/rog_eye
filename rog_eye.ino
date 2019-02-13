

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

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

#define ROG_EXT 0x4a
#define ADDRESSES 11

//50 (MISO), 51 (MOSI), 52 (SCK), 53 (SS).
/*
1 - grn - 18 cs 
2 - red - 21 clk
3 - org - 20 data/MOSI
4 - yel - 19 DC
5 - blk - 17 ?rst maybe? 
6 - pur - 16 backlight 
 */
/*
 * for mega use pins
#define TFT_MOSI 51  // Data out
#define TFT_SCLK 52  // Clock out
#define TFT_CS 53 // CS
#define TFT_RST 17
#define TFT_DC 48
//#define TFT_BACKLIGHT 46
#define TFT_BACKLIGHT 8

for uno etc use :- clk = 13,12 MISO,11 MOSI, cs = 10 
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

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  7
#define TFT_BACKLIGHT 8


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC,TFT_RST);

byte buf1, buf2;
#define MEM_SIZE 0xff
byte memory[MEM_SIZE] {};
byte alt_memory[MEM_SIZE] {};

float p = 3.1415926;

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
void dump_memory()
{
  Serial.println("  -=0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F=-");
 Serial.print("0");//that one dodgy zero, will fix later but this dirty fix i hope
   tft.fillScreen(ST77XX_BLACK);  
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
  for (int row=0;row<0x10;row++)
  {
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

void draw_grid()
{
  tft.setTextSize(1);
  tft.setCursor(0, 0);
//tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
/*
  for (int x=0;x > tft.width();x+=6)
  {
    tft.drawFastVLine( x, 1, x*5,ST77XX_WHITE);  
  }
*/
  delay(1000);
    for (int16_t x=0; x < tft.width(); x+=11) {
    //tft.drawLine(0, 0, x, tft.height()-1, ST77XX_WHITE);
    tft.drawFastVLine( x, 1, tft.height()-1,ST77XX_RED); 
    delay(0);
  }

    for (int row=0;row<9;row++)
  {

  tft.setTextWrap(true);
  tft.print(memory[row],HEX);
  tft.print(row*0x10,HEX);
  //tft.print(":");

  }
}

void display_knowns()
{
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.print("CPU V"); tft.print(((memory[0x40] * VDIV1) + (memory[0x41] * VDIV2)), 4);
 
  tft.setCursor(0,10);
  tft.print("DRAM V");tft.print(((memory[0x48] * VDIV1) + (memory[0x49] * VDIV2)), 4);
  
  tft.setCursor(0,20);
   tft.print("CPU FAN RPM ");tft.print((memory[0x60] << 8 | memory[0x61]));

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
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  
   else 
   {
   
      tft.setCursor(0,30);
      tft.print("CPU Temp C ");
      tft.print(memory[0x50]);
      
    }
  
  tft.setCursor(0,40);
  tft.print("BLCLK MHz ");tft.print(((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2)), 2);

  tft.setCursor(0,50);
  tft.print("CPU Mhz ");tft.print((memory[0x20]*((memory[0x28] * CDIV1) + (memory[0x29] * CDIV2))));

  tft.setCursor(0,60);
  tft.print("Clk Multiplier x");tft.print(memory[0x20]);
  
  tft.setCursor(0,70);
  //intel QCODE location tft.print("QCODE #");tft.print(memory[0x10]);
  tft.print("QCODE 1#");tft.print(memory[0x22],HEX);
  tft.setCursor(0,80);
  if (memory[0x6]==1)
    {
      tft.print("EUFI Mode: ");
    }
  else
      tft.print("BOOT Mode: ");
  for (int xn=0;xn<7;xn++)
  {
    tft.print(memory[xn],HEX);
    tft.print(" ");
  }
  tft.setCursor(0,90);
  tft.print("MEM 0x7:");tft.print(memory[0x7]);
  //tft.print(" 0x22:");tft.print(memory[0x22]);
  tft.print(" 0xff:");tft.print(memory[0xff]);

  tft.setCursor(0,100);
  
    for (int xn=0x20;xn<0x30;xn++)
  {
    tft.print(memory[xn]),HEX;
    tft.print(":");
  }
}

void setup(void) {
  Serial.begin(115200);
  Serial.print(F("RO_EXT test one"));

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

  testdrawtext("Power On - activating I2C Bridge ", ST77XX_WHITE);
  delay(1000);
  for (int count=0;count < MEM_SIZE;count++){memory[count]=0;}
  Wire.begin(ROG_EXT);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //Serial.begin(115200);           // start serial for output
  pinMode(SCL, INPUT_PULLUP);  // remove internal pullup
  pinMode(17,OUTPUT);
  digitalWrite(17, HIGH);
  tft.fillScreen(ST77XX_BLACK);
  
}

void loop() {
//  tft.invertDisplay(true);
//  delay(500);
 // tft.invertDisplay(false);
 
  delay(50);
 //  tft.fillScreen(ST77XX_BLACK);
  //tftPrintTest();
 // dump_memory();
 //draw_grid();
 
 display_knowns();
}

void demo()
{
  // tft print function!
 // tftPrintTest();
  //delay(4000);


  // a single pixel
  tft.drawPixel(tft.width()/2, tft.height()/2, ST77XX_GREEN);
  delay(500);

  // line draw test
  testlines(ST77XX_YELLOW);
  delay(500);

  // optimized lines
  testfastlines(ST77XX_RED, ST77XX_BLUE);
  delay(500);

  testdrawrects(ST77XX_GREEN);
  delay(500);

  testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  delay(500);

  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);

  testroundrects();
  delay(500);

  testtriangles();
  delay(500);

  mediabuttons();
  delay(500);

  //Serial.println("done");
  delay(1000);
  
}
void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, 0, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, 0, 0, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
    delay(0);
  }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(2);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}