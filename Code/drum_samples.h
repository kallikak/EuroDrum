#pragma once

#include <stdint.h>

// A basic set of 808 style drum kit samples

uint8_t getLevel(int type);
void adjLevel(int type, int d);
void setLevel(int type, uint8_t lvl);
void setDefaultLevels();
void setAltMask(uint8_t altsamples);
uint8_t getAltMask();
int numKits();
void setKit(int kit);
float getVolumeFactor(int type);
const unsigned int *getSample(int type);

extern const unsigned int AudioSampleBassdrum[1249];
#define DK_BASS_ARRAY AudioSampleBassdrum

extern const unsigned int AudioSampleCymbal[3041];
#define DK_CYMBAL_ARRAY AudioSampleCymbal

extern const unsigned int AudioSampleClosedhihat[353];
#define DK_HAT_C_ARRAY AudioSampleClosedhihat

extern const unsigned int AudioSampleOpenhihat[1057];
#define DK_HAT_O_ARRAY AudioSampleOpenhihat

extern const unsigned int AudioSampleSnare[609];
#define DK_SNARE_ARRAY AudioSampleSnare

extern const unsigned int AudioSampleTomtom_high[3361];
#define DK_TOM_H_ARRAY AudioSampleTomtom_high

extern const unsigned int AudioSampleTomtom_low[5153];
#define DK_TOM_L_ARRAY AudioSampleTomtom_low

extern const unsigned int AudioSampleTomtom_med[3841];
#define DK_TOM_M_ARRAY AudioSampleTomtom_med

extern const unsigned int AudioSampleClap[1345];
#define DK_CLAP_ARRAY AudioSampleClap

extern const unsigned int AudioSampleClaves[289];
#define DK_CLAVES_ARRAY AudioSampleClaves

extern const unsigned int AudioSampleHighconga[737];
#define DK_CONGA_H_ARRAY AudioSampleHighconga

extern const unsigned int AudioSampleLowconga[1505];
#define DK_CONGA_L_ARRAY AudioSampleLowconga

extern const unsigned int AudioSampleMedconga[865];
#define DK_CONGA_M_ARRAY AudioSampleMedconga

extern const unsigned int AudioSampleMaracas[129];
#define DK_MARACAS_ARRAY AudioSampleMaracas

extern const unsigned int AudioSampleRidecymbal[449];
#define DK_RIDECYMBAL_ARRAY AudioSampleRidecymbal

extern const unsigned int AudioSampleRimshot[161];
#define DK_RIMSHOT_ARRAY AudioSampleRimshot
