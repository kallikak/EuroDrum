#include <Audio.h>

void setLevels();

void setupSynth();

void setDAC(bool onoff);

void setDACLevel(float level);

void handleSynthDecay(int d, int dcv);

void handleSynthTone(int t, int tcv);

void handleSynthDrive(int d, int dcv);

void playSample(int index, AudioPlayMemory *sample, AudioEffectEnvelope *env);

void playIfSet(int i, bool accent);
