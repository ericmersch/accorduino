/* this code is used for programming an Arduino Pro Micro as a USB MIDI controller for playing chords according to magics of the circle of fifths
 * Author: Eric Mersch
 * MIDIUSB.h by Gary Grewal
 * Adafruit_MPR121.h by Adafruit
 * 
 * uses the i2c protocol to communicate with four Adafruit MPR121 capacitive sensors
 * 
 * CONNECTIONS:
 * VCC -> 3.3V
 * 2 -> SDA (pin 2 = SDA for Arduino Pro Micro)
 * 3 -> SCL (pin 3 = SCL for Arduino Pro Micro)
 * GND -> GND
 * 
 * A: Default mpr121 address is 0x5A (ADD connected to GND)
 * B: if pin ADD is connected to 3.3V the i2c address becomes 0x5B
 * C: if pin ADD is connected to SDA the address becomes 0x5C and if 
 * D: if pin ADD is connected to SCL the address becomes 0x5D then 0x5D
 * /!\ Do not forget to cut the link betweeen GND and ADD on the MPR121 B,C,D boards.
 * 
 * with 4*12 pins, one has 48 electrodes to press: 12 for playing the notes and 36 for chosing the chord : 12 Major, 12 minor, 12 diminished
 */

#include "MIDIUSB.h"
#include "Adafruit_MPR121.h"

Adafruit_MPR121 capA = Adafruit_MPR121();//mpr121 A has 12 electrodes acting as capacitive sensors
Adafruit_MPR121 capB = Adafruit_MPR121();//mpr121 B has 12 electrodes acting as capacitive sensors
Adafruit_MPR121 capC = Adafruit_MPR121();//mpr121 C has 12 electrodes acting as capacitive sensors
Adafruit_MPR121 capD = Adafruit_MPR121();//mpr121 D has 12 electrodes acting as capacitive sensors

uint16_t lasttouchedA = 0; //last state of register A, no pin pressed by default
uint16_t currtouchedA = 0; //current state of register A: //returns pin0*2^0+pin1*2^1+pin2*2^2+...+pin11*2^11 where pini = 0 if pin is pressed and 0 otherwise: will take a value between 0 (no pin is pressed) and  4095 (all pins are pressed)
uint16_t lasttouchedB = 0;
uint16_t currtouchedB = 0;
uint16_t lasttouchedC = 0;
uint16_t currtouchedC = 0;
uint16_t lasttouchedD = 0;
uint16_t currtouchedD = 0;

#ifndef _BV
#define _BV(bit) (1 << (bit)) //returns 2^bit
#endif

//change detections
#ifndef _pushA //has a pin just been pressed?
#define _pushA(uint8_t) ((currtouchedA & _BV(uint8_t)) && !(lasttouchedA & _BV(uint8_t))) //ex: returns 256=2^8 if pin 8 was just PRESSED  of capacitive senor A, ((1,0,0,1,0,0,0,0,1,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) && not((1,0,0,1,0,0,0,0,0,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) = ((0,0,0,0,0,0,0,0,1,0,0,0)) && (1,1,1,1,1,1,1,1,1,1,1,1) = (0,0,0,0,0,0,0,1,0,0,0) = 256
#endif

#ifndef _releaseA //has a pin just been released?
#define _releaseA(uint8_t) (!(currtouchedA & _BV(uint8_t)) && (lasttouchedA & _BV(uint8_t))) //ex: returns 256=2^8 if pin 8 was just RELEASED of capacitive senor A, not((1,0,0,1,0,0,0,0,0,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) && ((1,0,0,1,0,0,0,0,1,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) = ((1,1,1,1,1,1,1,1,1,1,1,1)) && (0,0,0,0,0,0,0,0,1,0,0,0) = (0,0,0,0,0,0,0,1,0,0,0) = 256
#endif

#ifndef _pushB
#define _pushB(uint8_t) ((currtouchedB & _BV(uint8_t)) && !(lasttouchedB & _BV(uint8_t)))
#endif

#ifndef _releaseB
#define _releaseB(uint8_t) (!(currtouchedB & _BV(uint8_t)) && (lasttouchedB & _BV(uint8_t)))
#endif

#ifndef _pushC
#define _pushC(uint8_t) ((currtouchedC & _BV(uint8_t)) && !(lasttouchedC & _BV(uint8_t)))
#endif

#ifndef _releaseC
#define _releaseC(uint8_t) (!(currtouchedC & _BV(uint8_t)) && (lasttouchedC & _BV(uint8_t)))
#endif

#ifndef _pushD
#define _pushD(uint8_t) ((currtouchedD & _BV(uint8_t)) && !(lasttouchedD & _BV(uint8_t)))
#endif

#ifndef _releaseD
#define _releaseD(uint8_t) (!(currtouchedD & _BV(uint8_t)) && (lasttouchedD & _BV(uint8_t)))
#endif

//TO DO: define _pushA&B((uint8_t,uint8_t): trigger another kind of chord if two pins are pressed at the same time or change the instrument if three pins are pressed at the same time.

uint8_t MInterval[] = {0,4,7,12,16,19,24,28,31,36,40,43};//interval of the major chord notes in semitones (4 octaves, one triad per octave)
uint8_t mInterval[] = {0,3,7,12,15,19,24,27,31,36,39,43};//inteval of the minor chord notes in semitones
uint8_t DimInterval[] = {0,3,6,12,15,18,24,27,30,36,39,42};//interval of the diminished chord notes in semitones
uint8_t susInterval[] = {0,2,7,12,14,19,24,26,31,36,38,43};//inteval of the sus2 chord notes in semitones Isus2 <-> Vsus4
uint8_t DomInterval[] = {0,4,10,12,16,22,24,28,34,36,40,46};//interval of the major chord notes in semitones (4 octaves, one triad per octave)
uint8_t Mdegree[] = {0,-5,2,-3,4,-1,6,1,-4,3,-2,5}; //cirlce of fifths for major :  0 -> I (0), 1 -> V (7 % 12 = -5 % 12), 2 -> II (3), 3 -> VI (-3), etc...
uint8_t mdegree[] = {-3,4,-1,6,1,-4,3,-2,5,0,-5,2}; //cirlce of fifths for minor :  0 -> vi (-3),1 -> iii (4),2 -> vii (-1), etc... 
uint8_t Dimdegree[] = {-1,6,1,-4,3,-2,5,0,-5,2,-3,4}; //cirlce of fifths for diminished : 0 -> vii° (-1),1 -> vib° (6),2 -> i#° (1), etc...
uint8_t Domdegree[] = {5, 0,-5,2,-3,4,-1,6,1,-4,3,-2}; //cirlce of fifths for dominant7
uint8_t root = 48;//MIDI note 36 = C2

uint8_t chord[] = {0,0,0,0,0,0,0,0,0,0,0,0};

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void playNote(uint8_t i){
  noteOn(0, i, 100);
  MidiUSB.flush();
  }

void releaseNote(uint8_t i){
  noteOff(0, i, 100);
  MidiUSB.flush();
  }

void release_all(){
  for (uint8_t k = 0; k < 12; k++) {
        noteOff(0, chord[k], 100);
      }
      MidiUSB.flush();
}

void changeChordM(uint8_t j){//change to major chord with position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = MInterval[i]+Mdegree[j]+root;
      }
  }

void changeChordm(uint8_t j){//change to minor chord with position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = mInterval[i]+mdegree[j]+root;
      }
  }

void changeChordDim(uint8_t j){//change to diminished chord of position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = DimInterval[i]+Dimdegree[j]+root;
      }
  }

void changeChordSus(uint8_t j){//change to diminished chord of position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = susInterval[i]+Mdegree[j]+root;
      }
  }

void changeChordDom(uint8_t j){//change to diminished chord of position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = DomInterval[i]+Domdegree[j]+root;
      }
  }

void setup() {

  changeChordM(0);//by default, set chord to C Major
  
  //initialise the 4 capacitive sensors
  capA.begin(0x5A);
  capB.begin(0x5B);
  capC.begin(0x5C);
  capD.begin(0x5D);

  //set the sensor thresholds
  capA.setThresholds(25, 5);// 25 for push, 2 for release
  capB.setThresholds(25, 5);
  capC.setThresholds(25, 5);
  capD.setThresholds(25, 5);
}

void loop() {
  // Get the currently touched pads
  currtouchedA = capA.touched();//get register A state
  currtouchedB = capB.touched();
  currtouchedC = capC.touched();
  currtouchedD = capD.touched();

////////////////////////////////////////////////////////////////
// mapping of the pins according to the pcb design /////////////
// change the mapping if you change the design//////////////////

/////////////////////////////////////////////////////////////////
// play notes////////////////////////////////////////////////////
  if(_pushD(11)){//if pin 11 of mpr121 D was just pressed
      playNote(chord[0]);//play the lowest note of the chord
  }
  if(_pushD(10)){
      playNote(chord[1]);
  }
  if(_pushD(9)){
      playNote(chord[2]);
  }
  if(_pushD(8)){
      playNote(chord[3]);
  }
  if(_pushD(7)){
      playNote(chord[4]);
  }
  if(_pushD(2)){
      playNote(chord[5]);
  }
  if(_pushD(3)){
      playNote(chord[6]);
  }
  if(_pushD(1)){
      playNote(chord[7]);
  }
  if(_pushD(0)){
      playNote(chord[8]);
  }
  if(_pushD(6)){
      playNote(chord[9]);
  }
  if(_pushD(5)){
      playNote(chord[10]);
  }
  if(_pushD(4)){
      playNote(chord[11]);
  }

/////////////////////////////////////////////////////////////////
// release notes/////////////////////////////////////////////////

  if(_releaseD(11)){//if pin 11 of mpr121 D was just released
      releaseNote(chord[0]);// mute the lowest note of the chord
  }
  if(_releaseD(10)){
      releaseNote(chord[1]);
  }
  if(_releaseD(9)){
      releaseNote(chord[2]);
  }
  if(_releaseD(8)){
      releaseNote(chord[3]);
  }
  if(_releaseD(7)){
      releaseNote(chord[4]);
  }
  if(_releaseD(2)){
      releaseNote(chord[5]);
  }
  if(_releaseD(3)){
      releaseNote(chord[6]);
  }
  if(_releaseD(1)){
      releaseNote(chord[7]);
  }
  if(_releaseD(0)){
      releaseNote(chord[8]);
  }
  if(_releaseD(6)){
      releaseNote(chord[9]);
  }
  if(_releaseD(5)){
      releaseNote(chord[10]);
  }
  if(_releaseD(4)){
      releaseNote(chord[11]);
  }

/////////////////////////////////////////////////////////////////
//change to major chord//////////////////////////////////////////

  if(_pushB(0)){//if pin 0 of mpr121 B was just pressed
      changeChordM(0);// set the chord to C Major
  }
  if(_pushC(9)){
      changeChordM(1);
  }
  if(_pushC(8)){
      changeChordM(2);
  }
  if(_pushC(5)){
      changeChordM(3);
  }
  if(_pushC(0)){
      changeChordM(4);
   }
   if(_pushA(11)){
      changeChordM(5);
   }
  if(_pushA(8)){
      changeChordM(6);
   }
  if(_pushA(3)){
   changeChordM(7);
  }
  if(_pushA(0)){
      changeChordM(8);
  }
  if(_pushB(9)){
      changeChordM(9);
  }
  if(_pushB(8)){
      changeChordM(10);
  }
  if(_pushB(5)){
      changeChordM(11);
  }
/////////////////////////////////////////////////////////////////
//change to minor chord//////////////////////////////////////////


  if(_pushB(1)){//if pin 1 of mpr121 B was just pressed
    changeChordm(0);// set the chord to A minor
  }
  if(_pushC(10)){
    changeChordm(1);
  }
  if(_pushC(7)){
    changeChordm(2);
  }
  if(_pushC(4)){
    changeChordm(3);
  }
  if(_pushC(1)){
    changeChordm(4);
  }
  if(_pushA(10)){
    changeChordm(5);
  }
  if(_pushA(7)){
   changeChordm(6);
  }
  if(_pushA(4)){
    changeChordm(7);
    }
  if(_pushA(1)){
    changeChordm(8);
    }
   if(_pushB(10)){
      changeChordm(9);
      }
   if(_pushB(7)){
    changeChordm(10);
    }
  if(_pushB(4)){
    changeChordm(11);
  }
      
/////////////////////////////////////////////////////////////////
//change to diminished chord/////////////////////////////////////

  if(_pushB(2)){//if pin 0 of mpr121 B was just pressed
    changeChordDim(0);// set the chord to B diminished 
  }
  if(_pushC(11)){
    changeChordDim(1);
  }
  if(_pushC(6)){
    changeChordDim(2);
  }
  if(_pushC(3)){
    changeChordDim(3);
  }
  if(_pushC(2)){
      changeChordDim(4);
  }
  if(_pushA(9)){
    changeChordDim(5);
  }
   if(_pushA(6)){
      changeChordDim(6);
  }
  if(_pushA(5)){
      changeChordDim(7);
  }
  if(_pushA(2)){
      changeChordDim(8);
  }
  if(_pushB(11)){
      changeChordDim(9);
  }
  if(_pushB(6)){
      changeChordDim(10);
  }
  if(_pushB(3)){
      changeChordDim(11);
  }

  /////////////////////////////////////////////////////////////////
//change to sus chord//////////////////////////////////////////

  if(_pushB(0) && (currtouchedC & _BV(9)) || _pushC(9) && (currtouchedB & _BV(0))){
      changeChordSus(0);// set the chord to Csus2
  }
  if(_pushC(9) && (currtouchedC & _BV(8)) || _pushC(8) && (currtouchedC & _BV(9))){
      changeChordSus(1);
  }
  if(_pushC(8) && (currtouchedC & _BV(5)) || _pushC(5) && (currtouchedC & _BV(8))){
      changeChordSus(2);
  }
  if(_pushC(5) && (currtouchedC & _BV(0)) || _pushC(0) && (currtouchedC & _BV(5))){
      changeChordSus(3);
  }
  if(_pushC(0) && (currtouchedA & _BV(11)) || _pushA(11) && (currtouchedC & _BV(0))){
      changeChordSus(4);
   }
  if(_pushA(11) && (currtouchedA & _BV(8)) || _pushA(8) && (currtouchedA & _BV(11))){
      changeChordSus(5);
   }
  if(_pushA(8) && (currtouchedA & _BV(3)) || _pushA(3) && (currtouchedA & _BV(8))){
      changeChordSus(6);
   }
  if(_pushA(3) && (currtouchedA & _BV(0)) || _pushA(0) && (currtouchedA & _BV(3))){
   changeChordSus(7);
  }
  if(_pushA(0) && (currtouchedB & _BV(9)) || _pushB(9) && (currtouchedA & _BV(0))){
      changeChordSus(8);
  }
  if(_pushB(9) && (currtouchedB & _BV(8)) || _pushB(8) && (currtouchedB & _BV(9))){
      changeChordSus(9);
  }
  if(_pushB(8) && (currtouchedB & _BV(5)) || _pushB(5) && (currtouchedB & _BV(8))){
      changeChordSus(10);
  }
  if(_pushB(5) && (currtouchedB & _BV(0)) || _pushB(0) && (currtouchedB & _BV(5))){
      changeChordSus(11);
  }

  /////////////////////////////////////////////////////////////////
//change to Dominant 7 chord//////////////////////////////////////////

  if(_pushB(0) && (currtouchedC & _BV(8)) || _pushC(8) && (currtouchedB & _BV(0))){
      changeChordDom(0);// set the chord to Csus2
  }
  if(_pushC(9) && (currtouchedC & _BV(5)) || _pushC(5) && (currtouchedC & _BV(8))){
      changeChordDom(1);
  }
  if(_pushC(8) && (currtouchedC & _BV(0)) || _pushC(0) && (currtouchedC & _BV(8))){
      changeChordDom(2);
  }
  if(_pushC(5) && (currtouchedA & _BV(11)) || _pushA(11) && (currtouchedC & _BV(5))){
      changeChordDom(3);
  }
  if(_pushC(0) && (currtouchedA & _BV(8)) || _pushA(8) && (currtouchedC & _BV(0))){
      changeChordDom(4);
   }
  if(_pushA(11) && (currtouchedA & _BV(3)) || _pushA(3) && (currtouchedA & _BV(11))){
      changeChordDom(5);
   }
  if(_pushA(8) && (currtouchedA & _BV(0)) || _pushA(0) && (currtouchedA & _BV(8))){
      changeChordDom(6);
   }
  if(_pushA(3) && (currtouchedB & _BV(9)) || _pushB(9) && (currtouchedA & _BV(3))){
   changeChordDom(7);
  }
  if(_pushA(0) && (currtouchedB & _BV(8)) || _pushB(8) && (currtouchedA & _BV(0))){
      changeChordDom(8);
  }
  if(_pushB(9) && (currtouchedB & _BV(5)) || _pushB(5) && (currtouchedB & _BV(9))){
      changeChordDom(9);
  }
  if(_pushB(8) && (currtouchedB & _BV(0)) || _pushB(0) && (currtouchedB & _BV(8))){
      changeChordDom(10);
  }
  if(_pushB(5) && (currtouchedC & _BV(9)) || _pushC(9) && (currtouchedB & _BV(5))){
      changeChordDom(11);
  }

  // keep the register last state in order to detect push and release actions
  lasttouchedA = currtouchedA;
  lasttouchedB = currtouchedB;
  lasttouchedC = currtouchedC;
  lasttouchedD = currtouchedD;
}
