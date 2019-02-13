### ROG_EYE
/* Author: darkspr1te
 * This program and it's files are free open source under gnu 2 or what ever
 * so long as you dont blame me, sell for profit without sending me a little thank you , blow up the world or anything silly
   
 * Project ROG_EYE
    
 * ***CAUTION CAUTION***
 * SENDING DATA OUT ON THIS BUS RANDOMLY WILL KILL YOUR CPU, trust me i have already done it once, symptoms are it will only boot windows/linux with cores disabled
 * Use only 3.3v I/O on your arduino device or modify it for 3.3V operation 

 
 * Purpose: Read ROG_EXT on asus motherboards, possible write values too, we shall see.  
   original work by elmor labs and some over clocker with a personal hate for intel heat spreaders
 * Working confirmed so far :-
    printTemp();
	printFanSpeed();
    printClockRatio();
	printRatio();
	printClock();

 * How this works
 
  The Arduino's I2c Pins A5/PC5 (CLK) and A4/PC4 (DATA) plus a ENABLE A5/PC3/PIN 17 for pinmode, this pin is connected to the ROG_EXT pins 5(SDA),7(SCL),4(ENABLE)
  See Diagram rog_ext_bb.png
  bring ROG_ENABLE(pin 4/con1) high and the motherboard exsposes the internal i2c bus,

  Anyway, so once ROG_ENABLE is HIGH then the arduino responds to I2C slave addres 0x4A, the motherboard writes to this at regular intervals, some values get updated more often
  Sometimes high cpu usage will delay updates, use avereaging for sensors that may jump large values if delayed

  currently the Arduino device act's as a I2C memory write only device, two memory arrays up to Size(0xff) are available, one can be compare to the other for simple value change monitoring (CPU heavy, restrict to 0xf in size 
  during comparison but full memory can be done) 
  Data is written to the memory[MEM_SIZE] buffer, current set at MEM_SIZE=0xff 


Address conversions Between Busppirate sniffer and Arduino I2C, Address 0x4A in arduino code with slave mode is seen as 0x94 write and 0x95 read on a buspirate sniff
 
 * Known Addresses are :-

 
 ```
typedef struct {
    int addr;
    char bytes;
    char type;
    char * name;
} addrStruct;
 
addrStruct addresses[ADDRESSES] = {
    {0x00, 1, TOPEID, "OPEID"},
//    {0x01, 1
 //   [0x06 ,1, POSTMODE,"SYSTEM POSTING"},
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
```
Math on cpu voltage is known to be different, hence the CPU_TYPE define (host motherboard cpu not the arduino MCU device) 
QCode location also different between AMD and intel, possible reason is firmware limits of ROG_EXT device or OEM wanting to sell more panels like intel hide the fact motherboards can be compatible between generations 


Rog Connector pinouts  (see rog_ext_maximus image ) 
```
Con1 (rog)   Con2 (USB)
    =========UP========

 1 3 5 7 gap 1  3  5  7  9
 
 2 4 6 8 gap 2  4  6  8  nc
 ```
 
 
Asus Front Base Pins/Colours At Motherboard Side
Functions supplied where possible, 

Left Connector :-
```
1 Green 1

3 Red 1  

5 Yellow (SDA i2c)

7 gray  (SCL i2c)

2 White 1/black (splits too front base) 

4 orange ENABLE (activate i2c output)

6 blue

8 shield 
``` 

Right Connector :-
```
1 +5ve

2 +5ve

3 usb A

4 NC

5 usb B

6 NC

7 GND

8 GND

9 Not connect/KEY for rotation of plug 

10 Same As pin 9 but not present
```

On Actual Front_Base and OC/Panel units the system detects USB and switches to update mode , thats why manuals request you connect to normal USB pin header to update the devices. 
when USB A/B is missing the device switches to normal function

```
CPU:Arduino Compatible Device Modded for 3.3V (old screen from a broken RGB Gobi lamp with DMX 512 function !!!, screen has built in arduino chip and buttons , bonus )
SCREEN #2: from some random MMPT solar charger, broke the previous one , dropped the bugger and stood on it.


```

###To Do

- [x] Basic bring up and working theory
- [x] Blow up CPU in testing so other's dont have too :-(
- [x] Prove theory and provide working test code & schematic 
- [ ] Add more sensors
- [x] Add lcd code 

- [x] add schematics in png format 
- [ ] Add additional protocols
  - [ ] Switch PE ratio
  - [ ] Switch CPU ratio etc See https://www.asus.com/Motherboard-Accessories/OC-Panel-II/
  - [ ] Add power on via event 

- [ ] EEPROM saving preference

- [x] Tidy up the code , remove testing data - Added more example code - removed more test code - tested more code 


Tested device 
Asus Crosshair Hero VII [URL]https://www.asus.com/Motherboards/ROG-CROSSHAIR-VII-HERO/overview/ [/URL]
Asus Front Panel/Front Base [URL]https://www.asus.com/Motherboard-Accessories/ROG_Front_Base/[/URL]
			  
---

```
About Author
Name: darkspr1te
location: Zambia,  Africa
Hobbies: Reverse engineer products faulty code so the user can fix it, less e-waste , re-use old products no longer supported 

This research is not cheap, this git commit alone cost me a motherboard and cpu with my mistakes. How ever I have done that so you dont have too, 
Out of the box the program is safe to use 

If you wish to contibute anything the please contact me on igeekcrg AT gmail.com , usual replace AT with @ and combine. 

Happy Overclock watching 
```
 
