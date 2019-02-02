### ROG_EYE
/* Author: darkspr1te
 * This program and it's files are free open source under gnu 2 or what ever
 * so long as you dont blame me, sell for profit without sending me a little thank you , blow up the world or anything silly
 * 
 * Project ROG_EYE
 *  
 * ***CAUTION CAUTION***
 * SENDING DATA OUT ON THIS BUS RANDOMLY WILL KILL YOUR CPU, trust me i have already done it once, symptoms are it will only boot windows/linux with cores disabled
 * Use only 3.3v I/O on your arduino device or modify it for 3.3V operation 
 *
 *
 * Purpose: Read ROG_EXT on asus motherboards, possible write values too, we shall see.  
 * original work by elmor labs and some over clocker with a personal hate for intel heat spreaders
 * Working confirmed so far :-
    printTemp();
    printFanSpeed();
    printClockRatio();

 * How this works
 * The Arduino's I2c Pins A5/PC5 (CLK) and A4/PC4 (DATA) plus a ENABLE A5/PC3/PIN 17 for pinmode pin is connected to the ROG_EXT pins
 * bring ENABLE high and the motherboard exsposes the internal i2c bus,

 * Anyway, so once ENABLE is HIGH then the arduino responds to I2C slave addres 0x4A, the motherboard writes to this at regular intervals, some values get updated more often
 * Sometimes high cpu usage will delay updates, use avereaging for sensors that may jump large values if delayed
 * 
 * currently the Arduino device act's as a I2C memory write only device, two memmory arrays up to 0xff are available, one can be compare to the other for simple value change monitoring
 * Data is written to the memory[MEM_SIZE] buffer, current set at MEM_SIZE=0xff 
 * Known Addresses are :-

Address conversions Between Busppirate sniffer and Arduino I2C, Address 0x4A in arduino code with slave mode is seen as 0x94 write and 0x95 read on a buspirate sniff

typedef struct {
    int addr;
    char bytes;
    char type;
    char * name;
} addrStruct;
 
addrStruct addresses[] = {
    {0x00, 1, TOPEID, "OPEID"}, // No idea yet

    {0x10, 1, TQCODE, "QCODE"}, // Bios Error codes, eg. 99 is boot ok followed by 24 is EUFI boot ok for ROG Hero VII board

    {0x20, 1, TRATIO, "CPU Ratio"}, // CPU multiplier , my ADM 2700x reports 37 when no cpu overide in place, does not show boot clocks

    {0x24, 1, TRATIO, "Cache Ratio"},//not sure
    {0x28, 2, TCLOCK, "BCLK"},// Actual BCLK in Mhz, default 100Mhz, Formula is ((memory[0x28] * 25.6) + (memory[0x29] * 0.1)) for CPU speed in mhz multiply result by TRATIO, 37000mhz on example 2700x/Hero VII
    {0x2a, 2 ; PCIEBCLK?},

    {0x30, 2, TVOLT,  "V1"},

    {0x38, 2, TVOLT,  "V2"},

    {0x3c, 2 ; 1.8v
    {0x40, 2, TVOLT,  "VCORE"},
    {0x48, 2, TVOLT,  "VDRAM"},
    {0x50, 1, TTEMP,  "CPU Temp"},//Cpu Temp direct value 
    {0x60, 2, TFAN,   "CPU Fanspeed"},//CPU Fan speed Formula is (memory[0x60] << 8 | memory[0x61])
}; 
Math on cpu voltage is known to be different, hence the CPU_TYPE define (host motherboard cpu not the arduino MCU device) 
QCode location also different between AMD and intel, possible reason is firmware limits of ROG_EXT device or OEM wanting to sell more panels like intel hide the fact motherboards can be compatible between generations 


    =========UP========

 1 3 5 7 gap 1  2  3  4  5
 2 4 6 8 gap 6  7  8  9  nc
 
Asus Front Base Pins/Colours At Motherboard Side
Functions supplied where possible, 

Left //I2C Bus and ENABLE
1 Green 1
3 Red 1  
5 Yellow (SDA i2c)
7 gray  (SCL i2c)

2 White 1/black (splits too front base) 
4 orange ENABLE (activate i2c output)
6 blue
8 shield 

Right Side (USB11) //Note often only the upper USB 1-4 row is connected to anything
1 USB 5+v red 
2 data wh
3 data gn
4 gnd
5 Unknown

6 USB 5+v red
7 Data wh
8 Data gn
9 Gnd
10 n/c (key pin) 

*/

```
CPU:Arduino Compatible Device Modded for 3.3V

```

###To Do

- [x] Basic bring up and working theory
- [x] Blow up CPU in testing so other dont have too
- [ ] Add more sensors
- [ ] Add lcd code 

- [ ] add schematics in pdf format 
- [ ] Add additional protocols
  - [ ] Switch PE
  - [ ] Switch CPU ratio etc See https://www.asus.com/Motherboard-Accessories/OC-Panel-II/

- [ ] EEPROM saving preference




Tested device 
Asus Crosshair Hero VII [URL]https://www.asus.com/Motherboards/ROG-CROSSHAIR-VII-HERO/overview/ [/URL]
Asus Front Panel/Front Base [URL]https://www.asus.com/Motherboard-Accessories/ROG_Front_Base/[/URL]
			  
---

```
About Author
Name: darkspr1te
location: Zambia Africa
Hobbies: Reverse engineer products faulty code so the user can fix it

This reasearch is not cheap, this git commit alone cost me a motherboard and cpu with my mistakes. How ever I have done that so you dont have too, 
Out of the box the program is safe to use 

If you wish to contibute anything the please contact me on igeekcrg AT gmail.com , usual replace AT with @ and combine. 

Happy Overclock watching 
```
 
