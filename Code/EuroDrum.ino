#include "Bounce2.h"
#include <IntervalTimer.h>
#include <Encoder.h>

#include "Synth.h"

#include "EuroDrum.h"
#include "LEDMatrix.h"
#include "drum_defs.h"
#include "drum_samples.h"

typedef enum { NONE, SHORT, PENDINGLONG, LONG, VERYLONG, ROSE } buttonpress;

int curMode = BARMODE;

// CVs
#define GATECV 6
#define CLOCKCV 7
#define DRIVECV A6
#define DECAYCV A7
#define TONECV A8
#define DAC A14

// pots
#define TONE A0
#define DECAY A1
#define DRIVE A2
#define RATE A3

typedef struct 
{
  int pin;
  int value;
} pot;

pot drivePot = {DRIVE, 0};
pot decayPot = {DECAY, 0};
pot tonePot = {TONE, 0};
pot ratePot = {RATE, 0};

// buttons
#define SET 5
#define ADJUST 9
#define RESET 12

#define LONG_PRESS_MILLIS 300

typedef struct
{
  int pin;
  bool longpress;
  bool handled;
  Bounce *debouncer;
} button;

button setBtn = {SET, false, false, NULL};
button adjBtn = {ADJUST, false, false, NULL};

#define RATELED 8
#define MODE 10

// encoders
typedef struct
{
  int pinA;
  int pinB;
  Encoder *enc;
  int pos;
} encoder;

encoder lr = {1, 2, NULL, 0};
encoder ud = {3, 4, NULL, 0};

IntervalTimer timer;

volatile bool active = false;

volatile bool rateLEDstate = false;
volatile bool trigger = false;
volatile int beat = 0;
volatile int barlength = 8;
volatile unsigned long totalbeat = 0;

bool major = true;
bool playsequencer = false;

controlmode curcontrol = NORMAL;
int curkit = 0;

void setupButtons()
{
  pinMode(setBtn.pin, INPUT_PULLUP);
  setBtn.debouncer = new Bounce();
  setBtn.debouncer->attach(setBtn.pin);
  setBtn.debouncer->interval(50);
  pinMode(adjBtn.pin, INPUT_PULLUP);
  adjBtn.debouncer = new Bounce();
  adjBtn.debouncer->attach(adjBtn.pin);
  adjBtn.debouncer->interval(50);
}

void setupEncoders()
{
  pinMode(lr.pinA, INPUT_PULLUP);
  pinMode(lr.pinB, INPUT_PULLUP);
  lr.enc = new Encoder(lr.pinA, lr.pinB);
  
  pinMode(ud.pinA, INPUT_PULLUP);
  pinMode(ud.pinB, INPUT_PULLUP);
  ud.enc = new Encoder(ud.pinA, ud.pinB);
}

int16_t movingaverage(int16_t newvalue, int16_t avg, int n)
{
  if (newvalue < 3)
    return 0;
  if (newvalue > MAXPOT - 3)
    return MAXPOT;
  else
    return (n * avg + newvalue) / (n + 1);
}

void handleRatePot()
{
  // linear tempo from 30 to 900 bpm  - pad a bit so extremes don't drop out
  int d = max(30, min(900, map(ratePot.value, 0, MAXPOT, 28, 902)));
  timer.update((int)(60000.0 / d * 1000 / 2));  // two updates each interval for off and on
}

void checkTempo()
{
  int newv = movingaverage(analogRead(ratePot.pin), ratePot.value, 5);
//  int newv = analogRead(ratePot.pin);
  if (abs(newv - ratePot.value) > 10)
  {
    ratePot.value = newv;
    handleRatePot();
  }
}

void timerFired()
{
  rateLEDstate = !rateLEDstate;

// with normalled circuit this is no longer necessary
//#if !EURORACK
//  // only flash the LED if internal
//  digitalWrite(RATELED, rateLEDstate);
//  if (rateLEDstate) // on trigger
//  {
//    updateBeats();
//  }
//#endif

  // gate the output anyway
  digitalWrite(CLOCKCV, active && rateLEDstate ? HIGH : LOW);
}

void updateBeats()
{
  trigger = active;
  ++beat;
  beat %= barlength;
  totalbeat++; 
}

buttonpress getPress(button *btn)
{
  btn->debouncer->update();
  int d = btn->debouncer->duration();
  int pd = btn->debouncer->previousDuration();
  if (!btn->handled && !btn->longpress && !btn->debouncer->read())
  {
    if (d >= 3 * LONG_PRESS_MILLIS)
    {
      btn->longpress = true;
      return VERYLONG;
    }
    else if (d >= LONG_PRESS_MILLIS)
    {
      return PENDINGLONG;
    }
  }
  else if (btn->debouncer->rose())
  {
    if (!btn->handled)
    {
      if (!btn->longpress)
      {
        if (pd > LONG_PRESS_MILLIS && pd < 3 * LONG_PRESS_MILLIS)
        {
          return LONG;
        }
        else
        {
          return SHORT;
        }
      }
    }
    btn->longpress = false;
    btn->handled = false;
    return ROSE;
  }
  return NONE;
}

void checkButtons()
{
  buttonpress setpress = getPress(&setBtn);
  switch (setpress)
  {
    case SHORT:
      if (setBtn.longpress)
        setBtn.longpress = false;
      else if (curcontrol == NORMAL)
        toggleCurPixel();
//      else if (curcontrol == LEVELS && cursamplemode == MIXED)
//        showLevels();
      else if (curcontrol == SAMPLESEL)
      {
        curkit = (curkit + 1) % numKits();
        setKit(curkit);
        showSampleSet();
      }
      else if (curcontrol == SEQUENCER)
      {
        major = !major;
        showSequencer();
      }
      break;
    case LONG:
      active = !active;
      updateForActivation();
      break;
    case VERYLONG:
      if (curcontrol == NORMAL)
        curcontrol = BARLENGTH;
      break;
    case ROSE:
      if (curcontrol == BARLENGTH)
        curcontrol = NORMAL;
      break;
    case PENDINGLONG:
    case NONE:
      break;
  }
  
  buttonpress adjpress = getPress(&adjBtn);
  switch (adjpress)
  {
    case SHORT:
      curcontrol = (controlmode)((curcontrol + 1) % 4);
      if (curcontrol == LEVELS)
        showLevels();
      else if (curcontrol == SAMPLESEL)
        showSampleSet();
      else if (curcontrol == SEQUENCER)
        showSequencer();
      else
        redraw();
      break;
    case VERYLONG:
      if (curcontrol == NORMAL)
      {
        setdefault();
        adjBtn.handled = true;
      }
      break;
    case PENDINGLONG:
      if (curcontrol == LEVELS)
      {
        setDefaultLevels();
        setLevels();
        showLevels();
        adjBtn.handled = true;
      }
      else if (curcontrol == SEQUENCER)
      {
        playsequencer = !playsequencer;
        setDAC(!playsequencer);
        adjBtn.handled = true;
        showSequencer();
      }
      else if (curcontrol == NORMAL)
      {
        clear();
      }
      break;
    case LONG:
    case ROSE:
    case NONE:
      break;
  }
}

void checkMode()
{
  int newMode = digitalRead(MODE);
  if (newMode != curMode)
  {
    int prevMode = curMode;
    curMode = newMode;
    curcontrol = NORMAL;
    switchMode(prevMode, newMode);
  }
}

void checkEncoders()
{
  if (curcontrol == SAMPLESEL || curcontrol == SEQUENCER)
    return;
  int pos, dx = 0, dy = 0;
  
  pos = lr.enc->read();
  if (pos != lr.pos && pos % 8 == 0) 
  {  
    dx = pos > lr.pos ? -1 : 1;
    lr.pos = pos;
  }
  
  pos = ud.enc->read();
  if (pos != ud.pos && pos % 8 == 0) 
  {  
    dy = pos > ud.pos ? -1 : 1;
    ud.pos = pos;
  }

  if (dx || dy)
  {
    updateCursor(dx, dy);
    if (curcontrol == LEVELS)
      setLevels();
  }
}

static bool laststate = false;
void gateCVint()
{
  bool state = digitalReadFast(GATECV);
  if (state != laststate)
  {
    laststate = state;
    digitalWrite(RATELED, state);
    if (state)
    {
      updateBeats();
    }
  }
}

void handleDecay()
{
  handleSynthDecay(analogRead(DECAY), analogRead(DECAYCV));
}

void handleTone()
{
  handleSynthTone(analogRead(TONE), analogRead(TONECV));
}

void handleDrive()
{ 
  handleSynthDrive(analogRead(DRIVE), analogRead(DRIVECV));
}

void pin_reset() 
{
  SCB_AIRCR = 0x05FA0004;
}

// pentatonic:   0 2 5 7 9
// pentatonic_m: 0 3 5 7 10
int notesmajor[] = {12, 9, 7, 5, 2, 0, -3, -5};
int notesminor[] = {12, 10, 7, 5, 3, 0, -2, -5};

void sendNote_direct(int n)
{
  static float semitone = 4096 / 3.3 / 4 / 12; // 4096 DAC resolution, 3.3V, amp gain of 4, 12 steps in 1V octave
  // convert to CV and write to pin 14
  // adjust the trimmer pot so 1.65V goes to 0V out
  // 1.65V is 2048
  // 1 octave up is then 1V out
  // since gain is 4, that means an output of 0.25V
  // full range is 4096 => 3.3V, so 0.25V is 4096 / 3.3 / 4
  int v = round(2048 - n * semitone);
//  Serial.print(beat);
//  Serial.print(": ");
//  Serial.print(v);
//  Serial.print("V for note ");
//  Serial.println(n);
  analogWrite(DAC, v);
}

void sendNote(int n)
{
  /* 
   * 0 to 1 into AudioOutputAnalog maps from 0 to 3.3V (analog ref voltage) 
   * the following amp has gain of 4 (and offset to make it AC)
   * so 0.5 to dac is 0V output from the amp
   * 0.65152 to dac will be 1.65 + 0.25 = 1.9V from the DAC, 1.0V from the amp
   * so a semitone step is 0.25 / 1.65 / 12 = 0.1263
   */
  static float semitone = 0.25 / 1.65 / 12;
  float v = 0.5 + n * semitone;
//  Serial.print(beat);
//  Serial.print(": ");
//  Serial.print(v);
//  Serial.print("V for note ");
//  Serial.println(n);
  setDACLevel(v);
}

#define NO_NOTE -1000

int playDACIfSet(int i)
{
  int note = 0;
  bool update = false;
  
  for (int j = 0; j < 8; ++j)
  {
    if (isSet(i, j))
    {
      update = true;
      note += (major ? notesmajor : notesminor)[7 - j];
    }
  }

  return update ? note : NO_NOTE;
}

void playDAC()
{
  int note = 0;
  if (curMode == BARMODE)
  {
    note = playDACIfSet(beat);
  }
  else
  {
    for (int j = 1; j <= 8; ++j)
    {
      if (totalbeat % j == 0)
      {
        int note2 = playDACIfSet(j - 1);
        if (note2 != NO_NOTE)
          note += note2;
      }
    }
  }
  if (note != NO_NOTE)
    sendNote(note);
}

void play()
{
  if (playsequencer)
  {
    playDAC();
  }
  else
  {
    if (curMode == BARMODE)
    {
      playIfSet(beat, beat == 0);
    }
    else
    {
      for (int j = 1; j <= 8; ++j)
      {
        if (totalbeat % j == 0)
        {
          playIfSet(j - 1, false);
        }
      }
    }
  }
}

void setup() 
{
//  analogReadResolution(12);
  analogWriteResolution(12);

  pinMode(DRIVE, INPUT);
  pinMode(DECAY, INPUT);
  pinMode(TONE, INPUT);
  pinMode(RATE, INPUT);
  
  pinMode(DRIVECV, INPUT);
  pinMode(DECAYCV, INPUT);
  pinMode(TONECV, INPUT);
  pinMode(GATECV, INPUT_PULLUP);
  pinMode(CLOCKCV, OUTPUT);
  
  pinMode(RESET, INPUT_PULLUP);
  
  pinMode(RATELED, OUTPUT);
  pinMode(MODE, INPUT_PULLUP);
  
  setupEncoders();
  setupButtons();

  setupMatrix();

  setupSynth();
  
  attachInterrupt(GATECV, gateCVint, CHANGE);
  attachInterrupt(RESET, pin_reset, FALLING);
  
  timer.begin(timerFired, 1000);
  checkTempo();
}

int loopcount = 0;

void loop() 
{
  loopcount++;

  if (trigger)
  {
    trigger = false;
    play();
    return;
  }
  
  if (loopcount % 10 == 0)
  {
    checkTempo();
//    gateCVint();
    checkButtons();
  }
  
  handleDecay();
  handleTone();
  handleDrive();

  checkEncoders();
  
  if (loopcount % 100 == 0)
  {
    checkMode();
  }
  
  if (loopcount % 100000 == 0)
  {
    DBGn(loopcount)Serial.println(AudioMemoryUsageMax());    
  }
}
