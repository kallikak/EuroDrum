#include "Synth.h"

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlayMemory          midtom; //xy=110.71426773071289,553.2857027053833
AudioPlayMemory          hightom; //xy=110.71426773071289,602.2857027053833
AudioPlayMemory          lowtom; //xy=111.71426773071289,491.2857027053833
AudioPlayMemory          bass;       //xy=116.71426773071289,172.2857027053833
AudioPlayMemory          snare; //xy=116.71426773071289,229.2857027053833
AudioPlayMemory          openhat; //xy=116.71426773071289,295.2857027053833
AudioPlayMemory          closedhat; //xy=118.71426773071289,354.2857027053833
AudioPlayMemory          cymbal; //xy=120.71426773071289,418.2857027053833
AudioEffectEnvelope      hightomenv; //xy=252.5,602.5
AudioEffectEnvelope      snareenv; //xy=261.25000381469727,230.00000381469727
AudioEffectEnvelope      bassenv;      //xy=262.50000381469727,172.50000286102295
AudioEffectEnvelope      lowtomenv; //xy=263.75000381469727,492.50000762939453
AudioEffectEnvelope      midtomenv; //xy=263.75000381469727,553.7500076293945
AudioEffectEnvelope      cymbalenv; //xy=265.00000381469727,417.5000057220459
AudioEffectEnvelope      openhatenv; //xy=272.50000381469727,297.5000047683716
AudioEffectEnvelope      closedhatenv; //xy=278.75000381469727,355.0000057220459
AudioMixer4              mixer1;         //xy=447.21427154541016,246.28570938110352
AudioMixer4              mixer2;         //xy=448.21427154541016,501.5357131958008
AudioMixer4              mixer3;         //xy=553.9642715454102,377.7857131958008
AudioFilterStateVariable filter;        //xy=685.0000076293945,380.00001430511475
AudioEffectWaveshaper    waveshaper;     //xy=864.4643020629883,319.03571796417236
AudioMixer4              driveMixer;     //xy=907.7143058776855,409.535719871521
AudioSynthWaveformDc     dc;            //xy=752,462
AudioAmplifier           amp;           //xy=987.7143058776855,359.535719871521
AudioOutputAnalog        dac;           //xy=1079.7143096923828,383.53571796417236
AudioOutputUSB           usb;           //xy=1087.500015258789,458.75001430511475
AudioConnection          patchCord1(midtom, midtomenv);
AudioConnection          patchCord2(hightom, hightomenv);
AudioConnection          patchCord3(lowtom, lowtomenv);
AudioConnection          patchCord4(bass, bassenv);
AudioConnection          patchCord5(snare, snareenv);
AudioConnection          patchCord6(openhat, openhatenv);
AudioConnection          patchCord7(closedhat, closedhatenv);
AudioConnection          patchCord8(cymbal, cymbalenv);
AudioConnection          patchCord9(hightomenv, 0, mixer2, 3);
AudioConnection          patchCord10(snareenv, 0, mixer1, 1);
AudioConnection          patchCord11(bassenv, 0, mixer1, 0);
AudioConnection          patchCord12(lowtomenv, 0, mixer2, 1);
AudioConnection          patchCord13(midtomenv, 0, mixer2, 2);
AudioConnection          patchCord14(cymbalenv, 0, mixer2, 0);
AudioConnection          patchCord15(openhatenv, 0, mixer1, 2);
AudioConnection          patchCord16(closedhatenv, 0, mixer1, 3);
AudioConnection          patchCord17(mixer1, 0, mixer3, 0);
AudioConnection          patchCord18(mixer2, 0, mixer3, 1);
AudioConnection          patchCord19(mixer3, 0, filter, 0);
AudioConnection          patchCord20(filter, 0, driveMixer, 1);
AudioConnection          patchCord21(filter, 0, waveshaper, 0);
AudioConnection          patchCord22(waveshaper, 0, driveMixer, 0);
AudioConnection          patchCord23(dc, 0, driveMixer, 2);
AudioConnection          patchCord24(waveshaper, 0, driveMixer, 0);
AudioConnection          patchCord25(driveMixer, dac);
AudioConnection          patchCord26(driveMixer, 0, usb, 0);
AudioConnection          patchCord27(driveMixer, 0, usb, 1);
// GUItool: end automatically generated code

#include "EuroDrum.h"
#include "LEDMatrix.h"
#include "drum_defs.h"
#include "drum_samples.h"

void setLevels()
{
  for (int i = 0; i < 4; ++i)
  {
    mixer1.gain(i, getVolumeFactor(i));
    mixer2.gain(i, getVolumeFactor(4 + i));
  }
}

void setupSynth()
{
  AudioMemory(50);
  setLevels();
  mixer3.gain(0, 0.5);
  mixer3.gain(1, 0.5);
  amp.gain(2);

  filter.octaveControl(4);
  filter.frequency(12000);
  filter.resonance(0.7);
  
  float waveShapeArray[17] = {
    -0.588, -0.579, -0.549, -0.320, -0.488, -0.228, -0.122, -0.396, 0,
    0.122, 0.320, 0.396, 0.228, 0.549, 0.488, 0.588, 0.579
  };
  waveshaper.shape(waveShapeArray, 17);
  
//  float overdriveShaper[9] = { 
//    -0.5, -0.45, -0.4, 0.2, 0, 0.2, 0.4, 0.45, 0.5
//  };
//  waveshaper.shape(overdriveShaper, 9);

  bassenv.attack(0);
  snareenv.attack(0);
  openhatenv.attack(0);
  closedhatenv.attack(0);
  hightomenv.attack(0);
  cymbalenv.attack(0);
  midtomenv.attack(0);
  hightomenv.attack(0);
  
  bassenv.sustain(0);
  snareenv.sustain(0);
  openhatenv.sustain(0);
  closedhatenv.sustain(0);
  hightomenv.sustain(0);
  cymbalenv.sustain(0);
  midtomenv.sustain(0);
  hightomenv.sustain(0);

  dc.amplitude(0);
  driveMixer.gain(2, 0);
  
  dac.analogReference(EXTERNAL);
}

void setDAC(bool onoff)
{
  mixer3.gain(0, onoff ? 0.5 : 0);
  mixer3.gain(1, onoff ? 0.5 : 0);
  driveMixer.gain(2, onoff ? 0 : 1.0);
}

void setDACLevel(float level)
{
  DBG(level)
  dc.amplitude(level);
}

float convertCV(int v)
{
#if EURORACK
  // -5V maps to 3.136V
  // 0V maps to 1.636V
  // +5V maps to 0.136V
  // over 5V maps to 0V 
  return map(v, MAXPOT, 0, -MIDPOT, MIDPOT);
#else
  return 0;
#endif    
}
// CV from -5 to +5
// 
void handleSynthDecay(int d, int dcv)
{
//  DBG2n("Decay", d)DBGn(dcv)
  d = RANGE(d + convertCV(dcv), 0, MAXPOT);
//  DBG(d)
  int newdecay = (int)round(2000.0 * d / MAXPOT * d / MAXPOT);
  
  bassenv.decay(newdecay);
  snareenv.decay(newdecay);
  openhatenv.decay(newdecay);
  closedhatenv.decay(newdecay);
  hightomenv.decay(newdecay);
  cymbalenv.decay(newdecay);
  midtomenv.decay(newdecay);
  hightomenv.decay(newdecay);
}

void handleSynthTone(int t, int tcv)
{
//  DBG2n("Tone", t)DBG(tcv)
  t = RANGE(t + convertCV(tcv), 0, MAXPOT);
  float cutoff, q;
  if (t < MIDPOT)
  {
    // exp curve from about 50Hz to 12000 Hz
    cutoff = 50 * exp(11.0 * t / MAXPOT);
    q = 0.7;
  }
  else
  {
    t -= MIDPOT;
    cutoff = 12000 - 40 * exp(11.0 * t / MAXPOT);
    q = t * 4.3 / MIDPOT + 0.7;
  }
  filter.frequency(cutoff);
  filter.resonance(q);
}

void handleSynthDrive(int d, int dcv)
{ 
//  DBG2n("Tone", d)DBG(dcv)
  d = RANGE(d + convertCV(dcv), 0, MAXPOT);
  float df = 1.0 * d / MAXPOT;
  driveMixer.gain(0, df);
  driveMixer.gain(1, 1 - df);
}

void playSample(int index, AudioPlayMemory *sample, AudioEffectEnvelope *env)
{
  env->noteOn();
  sample->play(getSample(index)); 
}

void playIfSet(int i, bool accent)
{
  if (accent)
  {
    if (i < 4)
      mixer1.gain(i, 4);
    else
      mixer2.gain(i - 4, 4);
  }
  
  if (isSet(i, DK_BASS))
    playSample(DK_BASS, &bass, &bassenv);
    
  if (isSet(i, DK_SNARE))
    playSample(DK_SNARE, &snare, &snareenv);
    
  if (isSet(i, DK_HAT_O))
    playSample(DK_HAT_O, &openhat, &openhatenv);
    
  if (isSet(i, DK_HAT_C))
    playSample(DK_HAT_C, &closedhat, &closedhatenv);
    
  if (isSet(i, DK_CYMBAL))
    playSample(DK_CYMBAL, &cymbal, &cymbalenv);
    
  if (isSet(i, DK_TOM_L))
    playSample(DK_TOM_L, &lowtom, &lowtomenv);
    
  if (isSet(i, DK_TOM_M))
    playSample(DK_TOM_M, &midtom, &midtomenv);
    
  if (isSet(i, DK_TOM_H))
    playSample(DK_TOM_H, &hightom, &hightomenv);
    
  if (accent)
  {
    if (i < 4)
      mixer1.gain(i, getLevel(i));
    else
      mixer2.gain(i - 4, getLevel(i));
  }
}
