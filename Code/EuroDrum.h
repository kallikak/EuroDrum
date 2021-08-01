#pragma once

#define EURORACK 1

#define MSG(v) {Serial.println(v);}
#define DBG(v) {Serial.print(#v " = ");Serial.println(v);}
#define DBG2(s,v) {Serial.print(#s " = ");Serial.println(v);}
#define DBG3(s,v) {Serial.print(#s ", " #v " = ");Serial.println(v);}
#define MSGn(v) {Serial.print(v);}
#define DBGn(v) {Serial.print(#v " = ");Serial.print(v);Serial.print(" ");}
#define DBG2n(s,v) {Serial.print(#s " = ");Serial.print(v);Serial.print(" ");}
#define DBG3n(s,v) {Serial.print(#s ", " #v " = ");Serial.print(v);Serial.print(" ");}
#define DBGLINE {Serial.println("----------------");}

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define RANGE(x, l, h) (max(min(x, h), l))

typedef enum { NORMAL, LEVELS, SAMPLESEL, SEQUENCER, BARLENGTH } controlmode;

#define BARMODE 0
#define POLYMODE 1

#define MAXPOT 1023 //4095
#define MIDPOT 512  //2048

extern volatile bool active;
extern controlmode curcontrol;
extern int curMode;
extern int curkit;
extern volatile int barlength;
extern bool major;
extern bool playsequencer;
