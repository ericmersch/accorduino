/* this code is used for programming an Arduino Pro Micro as a USB MIDI controller for playing chords according to magics of the circle of fifths
 * Author: Eric Mersch
 * MIDIUSB.h by Per Tillisch
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
 * C: if pin ADD is connected to SDA the address becomes 0x5C
 * D: if pin ADD is connected to SCL the address becomes 0x5D
 * /!\ Do not forget to cut the link betweeen GND and ADD on the MPR121 B,C,D boards to avoid shorts.
 * 
 * with 4*12 pins, one has 48 electrodes to press: 12 for playing the notes and 36 for chosing the chords
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
#define _pushA(uint8_t) (currtouchedA & _BV(uint8_t)) //ex: returns 256=2^8 if pin 8 was just PRESSED  of capacitive sensor A, ((1,0,0,1,0,0,0,0,1,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) & not((1,0,0,1,0,0,0,0,0,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) = ((0,0,0,0,0,0,0,0,1,0,0,0)) & (1,1,1,1,1,1,1,1,1,1,1,1) = (0,0,0,0,0,0,0,1,0,0,0) = 256
#endif

#ifndef _lastA //has a pin just been pressed?
#define _lastA(uint8_t) !(lasttouchedA & _BV(uint8_t)) //ex: returns 256=2^8 if pin 8 was just PRESSED  of capacitive sensor A, ((1,0,0,1,0,0,0,0,1,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) & not((1,0,0,1,0,0,0,0,0,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) = ((0,0,0,0,0,0,0,0,1,0,0,0)) & (1,1,1,1,1,1,1,1,1,1,1,1) = (0,0,0,0,0,0,0,1,0,0,0) = 256
#endif

#ifndef _releaseA //has a pin just been released?
#define _releaseA(uint8_t) (!(currtouchedA & _BV(uint8_t)) && (lasttouchedA & _BV(uint8_t))) //ex: returns 256=2^8 if pin 8 was just RELEASED of capacitive senor A, not((1,0,0,1,0,0,0,0,0,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) & ((1,0,0,1,0,0,0,0,1,0,0,0) & (0,0,0,0,0,0,0,0,1,0,0,0)) = ((1,1,1,1,1,1,1,1,1,1,1,1)) & (0,0,0,0,0,0,0,0,1,0,0,0) = (0,0,0,0,0,0,0,1,0,0,0) = 256
#endif

#ifndef _pushB
#define _pushB(uint8_t) (currtouchedB & _BV(uint8_t))
#endif

#ifndef _lastB
#define _lastB(uint8_t) !(lasttouchedB & _BV(uint8_t))
#endif

#ifndef _releaseB
#define _releaseB(uint8_t) (!(currtouchedB & _BV(uint8_t)) && (lasttouchedB & _BV(uint8_t)))
#endif

#ifndef _pushC
#define _pushC(uint8_t) (currtouchedC & _BV(uint8_t))
#endif

#ifndef _lastC
#define _lastC(uint8_t) !(lasttouchedC & _BV(uint8_t))
#endif

#ifndef _releaseC
#define _releaseC(uint8_t) (!(currtouchedC & _BV(uint8_t)) && (lasttouchedC & _BV(uint8_t)))
#endif

#ifndef _pushD
#define _pushD(uint8_t) currtouchedD & _BV(uint8_t)
#endif

#ifndef _lastD
#define _lastD(uint8_t) !(lasttouchedD & _BV(uint8_t))
#endif

#ifndef _releaseD
#define _releaseD(uint8_t) (!(currtouchedD & _BV(uint8_t)) && (lasttouchedD & _BV(uint8_t)))
#endif

//TO DO: define _pushA&&B((uint8_t,uint8_t): trigger another kind of chord if two pins are pressed at the same time or change the instrument if three pins are pressed at the same time.

uint8_t MInterval[] = {0,4,7,12,16,19,24,28,31,36,40,43};//interval of the major chord notes in semitones (4 octaves, one triad per octave)
uint8_t mInterval[] = {0,3,7,12,15,19,24,27,31,36,39,43};//inteval of the minor chord notes in semitones
uint8_t DimInterval[] = {0,3,6,12,15,18,24,27,30,36,39,42};//interval of the diminished chord notes in semitones
uint8_t susInterval[] = {0,2,7,12,14,19,24,26,31,36,38,43};//inteval of the sus2 chord notes in semitones Isus2 <-> Vsus4
uint8_t M7mInterval[] = {0,4,10,12,16,22,24,28,34,36,40,46};//interval of the 7MM chord notes in semitones (4 octaves, one triad per octave)
uint8_t M7MInterval[] = {0,4,11,12,16,23,24,28,35,36,40,47};//interval of the 7MM chord notes in semitones (4 octaves, one triad per octave)
uint8_t m7mInterval[] = {0,3,10,12,15,22,24,27,34,36,39,46};//interval of the 7MM chord notes in semitones (4 octaves, one triad per octave)

int8_t Mdegree[] = {0,-5,2,-3,4,-1,6,1,-4,3,-2,5}; //cirlce of fifths for major :  0 -> I (0), 1 -> V (7 % 12 = -5 % 12), 2 -> II (3), 3 -> VI (-3), etc...
int8_t mdegree[] = {-3,4,-1,6,1,-4,3,-2,5,0,-5,2}; //cirlce of fifths for minor :  0 -> vi (-3),1 -> iii (4),2 -> vii (-1), etc... 
int8_t Dimdegree[] = {-1,6,1,-4,3,-2,5,0,-5,2,-3,4}; //cirlce of fifths for diminished : 0 -> vii° (-1),1 -> vib° (6),2 -> i#° (1), etc...
int8_t M7mdegree[] = {-5,2,-3,4,-1,6,1,-4,3,-2,5,0}; //degrees for 7th chords with major third and minor 7th
int8_t M7Mdegree[] = {0,-5,2,-3,4,-1,6,1,-4,3,-2,5}; //degrees for 7th chords with major third and major 7th
int8_t m7mdegree[] = {-3,4,-1,6,1,-4,3,-2,5,0,-5,2}; //degrees for 7th chords with minor third and minor 7th
uint8_t root = 48;//MIDI note 48 = C3

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

void changeChordM7m(uint8_t j){//change to diminished chord of position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = M7mInterval[i]+M7mdegree[j]+root;
      }
  }

void changeChordM7M(uint8_t j){//change to diminished chord of position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = M7MInterval[i]+M7Mdegree[j]+root;
      }
  }
  
void changeChordm7m(uint8_t j){//change to diminished chord of position j on the cirlce of fifths
  release_all();
    for(uint8_t i = 0;i<12;i++){
      chord[i] = m7mInterval[i]+m7mdegree[j]+root;
      }
  }

void setup() {

  changeChordM(0);//by default, set chord to C Major

  //Wire.begin();
  
  //initialise the 4 capacitive sensors
  capA.begin(0x5A, &Wire);
  capB.begin(0x5B, &Wire);
  capC.begin(0x5C, &Wire);
  capD.begin(0x5D, &Wire);


  capA.setAutoconfig(true);
  capB.setAutoconfig(true);
  capC.setAutoconfig(true);
  capD.setAutoconfig(true);
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
  if(_pushD(11) && _lastD(11)){//if pin 11 of mpr121 D was just pressed
      playNote(chord[0]);//play the lowest note of the chord
  }
  if(_pushD(10) && _lastD(10)){
      playNote(chord[1]);
  }
  if(_pushD(9) && _lastD(9)){
      playNote(chord[2]);
  }
  if(_pushD(8) && _lastD(8)){
      playNote(chord[3]);
  }
  if(_pushD(7) && _lastD(7)){
      playNote(chord[4]);
  }
  if(_pushD(2) && _lastD(2)){
      playNote(chord[5]);
  }
  if(_pushD(3) && _lastD(3)){
      playNote(chord[6]);
  }
  if(_pushD(1) && _lastD(1)){
      playNote(chord[7]);
  }
  if(_pushD(0) && _lastD(0)){
      playNote(chord[8]);
  }
  if(_pushD(6) && _lastD(6)){
      playNote(chord[9]);
  }
  if(_pushD(5) && _lastD(5)){
      playNote(chord[10]);
  }
  if(_pushD(4) && _lastD(4)){
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
    if(_lastB(0)){changeChordM(0);}// set the chord to C Major
    else if(_pushC(9) && (_lastB(0) || _lastC(9))){
      changeChordSus(0);// set the chord to Csus2
    }
    else if(_pushB(3) && (_lastB(0) || _lastB(3))){
      changeChordM7m(11);
    }
    else if(_pushC(10) && (_lastB(0) || _lastC(10))){
      changeChordM7M(0);// set the chord to Csus2
    }
    else if(_pushB(1) && (_lastB(0) || _lastB(1))){
      changeChordm7m(0);// set the chord to Csus2
    }
  }
  else if(_pushC(9)){
    if(_lastC(9)){changeChordM(1);}
    else if(_pushC(8) && (_lastC(9) || _lastC(8))){
      changeChordSus(1);
    }
    else if(_pushB(2) && (_lastC(9) || _lastB(2))){
      changeChordM7m(0);// set the chord to Dom7
    }
    else if(_pushC(7) && (_lastC(9) || _lastC(7))){
      changeChordM7M(1);
    }
    else if(_pushC(10) && (_lastC(9) || _lastC(10))){
      changeChordm7m(1);
    }
  }
  else if(_pushC(8)){
    if(_lastC(8)){changeChordM(2);}
    else if(_pushC(5) && (_lastC(8) || _lastC(5))){
      changeChordSus(2);
    }
    else if(_pushC(11) && (_lastC(8) || _lastC(11))){
      changeChordM7m(1);
    }
    else if(_pushC(4) && (_lastC(8) || _lastC(4))){
      changeChordM7M(2);
    }
    else if(_pushC(7) && (_lastC(8) || _lastC(7))){
      changeChordm7m(2);
    }
  }
  else if(_pushC(5)){
    if(_lastC(5)) changeChordM(3);
    else if(_pushC(0) && (_lastC(5) || _lastC(0))){
      changeChordSus(3);
    }
    else if(_pushC(6) && (_lastC(5) || _lastC(6))){
      changeChordM7m(2);
    }
    else if(_pushC(1) && (_lastC(5) || _lastC(1))){
      changeChordM7M(3);
    }
    else if(_pushC(4) && (_lastC(5) || _lastC(4))){
      changeChordm7m(3);
    }
  }
  else if(_pushC(0)){
    if(_lastC(0)){changeChordM(4);}
    else if(_pushA(11) && (_lastC(0) || _lastA(11))){
      changeChordSus(4);
    }
    else if(_pushC(3) && (_lastC(0) || _lastC(3))){
      changeChordM7m(3);
    }
    else if(_pushA(10) && (_lastC(0) || _lastA(10))){
      changeChordM7M(4);
    }
    else if(_pushC(1) && (_lastC(0) || _lastC(1))){
      changeChordm7m(4);
    }
   }
   else if(_pushA(11)){
    if(_lastA(11)) changeChordM(5);
    else if(_pushA(8) && (_lastA(11) || _lastA(8))){
      changeChordSus(5);
    }
    else if(_pushC(2) && (_lastA(11) || _lastC(2))){
      changeChordM7m(4);
    }
    else if(_pushA(7) && (_lastA(11) || _lastA(7))){
      changeChordM7M(5);
    }
    else if(_pushA(10) && (_lastA(11) || _lastA(10))){
      changeChordm7m(5);
    }
   }
  else if(_pushA(8)){
    if(_lastA(8)){changeChordM(6);}
    else if(_pushA(3) && (_lastA(8) || _lastA(3))){
      changeChordSus(6);
    }
    else if(_pushA(9) && (_lastA(8) || _lastA(9))){
      changeChordM7m(5);
    }
    else if(_pushA(4) && (_lastA(8) || _lastA(4))){
      changeChordM7M(6);
    }
    else if(_pushA(7) && (_lastA(8) || _lastA(7))){
      changeChordm7m(6);
    }
   }
  else if(_pushA(3)){
    if(_lastA(3)){changeChordM(7);}
    else if(_pushA(0) && (_lastA(3) || _lastA(0))){
      changeChordSus(7);
    }
    else if(_pushA(6) && (_lastA(3) || _lastA(6))){
      changeChordM7m(6);
    }
    else if(_pushA(1) && (_lastA(3) || _lastA(1))){
      changeChordM7M(7);
    }
    else if(_pushA(4) && (_lastA(3) || _lastA(4))){
      changeChordm7m(7);
    }
  }
  else if(_pushA(0)){
    if(_lastA(0)){changeChordM(8);}
    else if(_pushB(9) && (_lastA(0) || _lastB(9))){
      changeChordSus(8);
    }
    else if(_pushA(5) && (_lastA(0) || _lastA(5))){
      changeChordM7m(7);
    }
    else if(_pushB(10) && (_lastA(0) || _lastB(10))){
      changeChordM7M(8);
    }
    else if(_pushA(1) && (_lastA(0) || _lastA(1))){
      changeChordm7m(8);
    }
  }
  else if(_pushB(9)){
    if(_lastB(9)){changeChordM(9);}
    else if(_pushB(8) && (_lastB(9) || _lastB(8))){
      changeChordSus(9);
    }
    else if(_pushA(2) && (_lastB(9) || _lastA(2))){
      changeChordM7m(8);
    }
    else if(_pushB(7) && (_lastB(9) || _lastB(7))){
      changeChordM7M(9);
    }
    else if(_pushB(10) && (_lastB(9) || _lastB(10))){
      changeChordm7m(9);
    }
  }
  else if(_pushB(8)){
    if(_lastB(8)){changeChordM(10);}
    else if(_pushB(5) && (_lastB(8) || _lastB(5))){
      changeChordSus(10);
    }
    else if(_pushB(11) && (_lastB(8) || _lastB(11))){
      changeChordM7m(9);
    }
    else if(_pushB(4) && (_lastB(8) || _lastB(4))){
      changeChordM7M(10);
    }
    else if(_pushB(7) && (_lastB(8) || _lastB(7))){
      changeChordm7m(10);
    }
  }
  else if(_pushB(5)){
    if(_lastB(5)){changeChordM(11);}
    else if(_pushB(0) && (_lastB(5) || _lastB(0))){
      changeChordSus(11);
    }
    else if(_pushB(6) && (_lastB(5) || _lastB(6))){
      changeChordM7m(10);
    }
    else if(_pushB(1) && (_lastB(5) || _lastB(1))){
      changeChordM7M(11);
    }
    else if(_pushB(4) && (_lastB(5) || _lastB(04))){
      changeChordm7m(11);
    }
  }
/////////////////////////////////////////////////////////////////
//change to minor chord//////////////////////////////////////////


  else if(_pushB(1) && _lastB(1)){//if pin 1 of mpr121 B was just pressed
    changeChordm(0);// set the chord to A minor
  }
  else if(_pushC(10) && _lastC(10)){
    changeChordm(1);
  }
  else if(_pushC(7) && _lastC(7)){
    changeChordm(2);
  }
  else if(_pushC(4) && _lastC(4)){
    changeChordm(3);
  }
  else if(_pushC(1) && _lastC(1)){
    changeChordm(4);
  }
  else if(_pushA(10) && _lastA(10)){
    changeChordm(5);
  }
  else if(_pushA(7) && _lastA(7)){
   changeChordm(6);
  }
  else if(_pushA(4) && _lastA(4)){
    changeChordm(7);
    }
  else if(_pushA(1) && _lastA(1)){
    changeChordm(8);
    }
  else if(_pushB(10) && _lastB(10)){
      changeChordm(9);
      }
  else if(_pushB(7) && _lastB(7)){
    changeChordm(10);
    }
  else if(_pushB(4) && _lastB(4)){
    changeChordm(11);
  }
      
/////////////////////////////////////////////////////////////////
//change to diminished chord/////////////////////////////////////

  else if(_pushB(2)  && _lastB(2)){//if pin 2 of mpr121 B was just pressed
    changeChordDim(0);// set the chord to B diminished 
  }
  else if(_pushC(11) && _lastC(11)){
    changeChordDim(1);
  }
  else if(_pushC(6) && _lastC(6)){
    changeChordDim(2);
  }
  else if(_pushC(3) && _lastC(3)){
    changeChordDim(3);
  }
  else if(_pushC(2) && _lastC(2)){
      changeChordDim(4);
  }
  else if(_pushA(9) && _lastA(9)){
    changeChordDim(5);
  }
  else if(_pushA(6) && _lastA(6)){
      changeChordDim(6);
  }
  else if(_pushA(5) && _lastA(5)){
      changeChordDim(7);
  }
  else if(_pushA(2) && _lastA(2)){
      changeChordDim(8);
  }
  else if(_pushB(11) && _lastB(11)){
      changeChordDim(9);
  }
  else if(_pushB(6) && _lastB(6)){
      changeChordDim(10);
  }
  else if(_pushB(3) && _lastB(3)){
      changeChordDim(11);
  }
 
  // keep the register last state in order to detect push and release actions
  lasttouchedA = currtouchedA;
  lasttouchedB = currtouchedB;
  lasttouchedC = currtouchedC;
  lasttouchedD = currtouchedD;
}
