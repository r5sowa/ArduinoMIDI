/* 10 Dec 2013
Rachel Sowa
CCS CS 1L

This code is for use with an Arduino micro-controller, Tripple Axis LIS3LV02DQ Accelerometer, and a Sparkfun Musical Instrument Shield.

Original example code for the accelerometer is by Troy Nachtigall and can be found here: http://nearfuturelaboratory.com/2006/09/22/arduino-and-the-lis3lv02dq-triple-axis-accelerometer/

Orginal example code for the Musical Instrument Shield is by Nathan Seidle and can be found here: http://www.sparkfun.com/Code/MIDI_Example.pde

*/

#include <SoftwareSerial.h>

#define DATAOUT 11//MOSI
#define DATAIN  12//MISO 
#define SPICLOCK 13//sck
#define SLAVESELECT 10//ss for /LIS3LV02DQ, active low
#define RTC_CHIPSELECT 9// chip select (ss/ce) for RTC, active high

SoftwareSerial mySerial(2, 3); // RX, TX

byte clr;
int incomingByte = 0;

byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line
int  instrument = 0;

void setup() {
  
  char in_byte;
  clr = 0;
  in_byte = clr;
  
  Serial.begin(9600);

  //Setup soft serial for MIDI control
  mySerial.begin(31250);

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)
  
  // set direction of pins
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device
  pinMode(10, OUTPUT);
  // SPCR = 01010000
  // Set the SPCR register to 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA);
  clr=SPSR;
  clr=SPDR;
  // query the WHO_AM_I register of the LIS3LV02DQ
  // this should return 0x3A, a factory setting
  in_byte = read_register(15);
  Serial.print("WHO_AM_I [");
  Serial.print(in_byte, HEX);
  Serial.println("]");
  
  // start up the device
  // this activates the device, powers it on, enables all axes, and turn off the self test
  // CTRL_REG1 set to B10000111=135 per LIS3LV02DQ datasheet
  write_register(0x20, 135);

  Serial.println("----");
  delay(250);
}

char spi_transfer(volatile char data)
{
  /*
  Writing to the SPDR register begins an SPI transaction
  */
  SPDR = data;
  /*
  Loop right here until the transaction is complete. the SPIF bit is 
  the SPI Interrupt Flag. When interrupts are enabled, and the 
  SPIE bit is set enabling SPI interrupts, this bit will set when
  the transaction is finished.
  */
  while (!(SPSR & (1<<SPIF)))     
  {};
  // received data appears in the SPDR register
  return SPDR;                    
}

// reads a register
char read_register(char register_name)
{
   char in_byte;
   // need to set bit 7 to indicate a read
   register_name |= 128;
   // SS is active low
   digitalWrite(SLAVESELECT, LOW);
   // send the address of the register we want to read first
   spi_transfer(register_name);
   // send nothing, but here's when the device sends back the register's value as an 8 bit byte
   in_byte = spi_transfer(0);
   // deselect the device
   digitalWrite(SLAVESELECT, HIGH); 
   return in_byte;
}

// write to a register
void write_register(char register_name, byte data)
{
   // clear bit 7 to indicate we're doing a write
   register_name &= 127;
   // SS is active low
   digitalWrite(SLAVESELECT, LOW);
   // send the address of the register we want to write
   spi_transfer(register_name);
   // send the data we're writing
   spi_transfer(data);
   digitalWrite(SLAVESELECT, HIGH);
}


void loop() {
  
  // with LIS3LV02DQ in +/-2g resolution mode, with 12-bits of resolution + sign bits
  // (high four bits are just the sign in 12 bit mode)
  // then 2g = 2^12/2 = 2048
  // 1g = 1024
  // a range of +/-2g should output +/-2048 

  // Must assemble signed integer from high and low bytes

  int x_val, y_val, z_val;
  byte x_val_l, x_val_h, y_val_l, y_val_h, z_val_l, z_val_h;
  
  x_val_h = read_register(0x29); // Read outx_h register
  x_val_l  = read_register(0x28); // Read outx_l
  x_val = x_val_h;
  x_val <<= 8;
  x_val += x_val_l; // shift high byte by 8 bits and add low byte
  Serial.print(x_val, DEC);
  Serial.print(",");

  y_val_h = read_register(0x2B); //Read outy_h register
  y_val_l = read_register(0x2A); //Read outy_l register
  y_val = y_val_h;
  y_val <<= 8;
  y_val += y_val_l; // shift high byte by 8 bits and add low byte
  Serial.print(y_val, DEC);
  Serial.print(",");


  z_val_h = read_register(0x2D); //Read outz_h register
  z_val_l = read_register(0x2C); //Read outz_l register
  z_val = z_val_h;
  z_val <<= 8;
  z_val += z_val_l; // shift high byte by 8 bits and add low byte
  Serial.println(z_val, DEC);

  //Demo Basic MIDI instruments, GM1
  //=================================================================
  Serial.println("Basic Instruments");
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1

  instrument = 123;

  Serial.print(" Instrument: ");
  Serial.println(instrument, DEC);

  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

  //sets the first note of the scale
  int startingNote = 60; // 60 is middle C.

  //different musical scales represented as integer arrays
  //each half step is represented by an increase of 1
  int ionian[] = {0,2,4,5,7,9,11,12,14,16,17,19,21,23,24,26,28,29,31,33,35,36}; //3 Octave Major Scale
  int dorian[] = {0,2,3,5,7,9,11,12};
  int phrygian[] = {0,1,3,5,7,8,10,12};
  int lydian[] = {0,2,4,6,7,9,11,12};
  int mixolydian[] = {0,2,4,5,7,9,10,12};
  int aeolian[] = {0,2,3,5,7,8,10,12};
  int locrian[] = {0,1,3,5,6,8,10,12};
  int blues[] = {0,3,5,6,7,10,12};
  int maj7[] = {0,4,7,11}; //a Major 7th chord
  int chromatic[] = {0,1,2,3,4,5,6,7,8,9,10,11,12};
  
  //converting accelermoter x value into the 0-127 MIDI range
  int convert = 13; /*This value needs to be changed depending on how many elements there are in the chosen scale array.
  Although the acceleromter values technically range from -2048 to 2048, those extreme values really only occur when shaking the
  acceleromter really hard, but not when simply moving it back and forth. Therefore, it's still a bit of a guess-and-check method
  in terms of trying to figure out the right conversion number based on how many notes there are in the scale array.
  */
  double dblAcc = (double) x_val;
  double dblNote = (((dblAcc)*convert)/2048);
  int intNote = (int) dblNote;
  int accNote = abs(intNote);
 
  Serial.print("MIDI Note Value: ");
  Serial.println(accNote, DEC); //prints the note/pitch value to the serial monitor
  
  //makes a call to the playScaleAcc function and passes into it the starting note that was set earlier, a chosen scale array, and the converted acceleromter value
  playScaleAcc(startingNote, ionian, accNote);
  
  //creates a 100ms rest between notes
  delay(100);

}


void playScaleAcc(int startingNote, int scaleValues[], int accNote) {
  int noteLengthMs = 100;
  int velocity = 120; // 80 is the velocity of striking the key, or releasing

  noteOn(0, startingNote + scaleValues[accNote], velocity); delay(noteLengthMs); noteOff(0, startingNote + scaleValues[accNote], velocity); 
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

}

