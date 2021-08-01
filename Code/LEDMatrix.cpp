#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include <EEPROM.h>

#include "LEDMatrix.h"
#include "EuroDrum.h"
#include "drum_samples.h"

static Adafruit_BicolorMatrix ledmatrix = Adafruit_BicolorMatrix();
static uint8_t PROGMEM defleds[] = {0b00001000,
                                    0b01000000,
                                    0b00100000,
                                    0b00010000,
                                    0b00001000,
                                    0b00000100,
                                    0b00000010,
                                    0b00000001 };
static uint8_t PROGMEM bkleds[] = { 0b00001000,
                                    0b01000000,
                                    0b00100000,
                                    0b00010000,
                                    0b00001000,
                                    0b00000100,
                                    0b00000010,
                                    0b00000001 };

static int cursor_x = 0;
static int cursor_y = 0;
static bool hidecursor = true;

#define SIGNATURE_BYTE 0xA3

bool isSet(int x, int y)
{
  if (x < 0 || y < 0 || x > 7 || y > 7)
    return false;
  x = 7 - x;
  return (bkleds[y] & (0x1 << x)) != 0;
}

void hideCursor()
{
  hidecursor = true;
  ledmatrix.drawPixel(cursor_x, cursor_y, isSet(cursor_x, cursor_y) ? (active ? LED_GREEN : LED_YELLOW) : LED_OFF);
}

void drawCursor()
{
  hidecursor = false;
  ledmatrix.drawPixel(cursor_x, cursor_y, isSet(cursor_x, cursor_y) ? (active ? LED_YELLOW : LED_RED) : LED_RED);                                        
}

void updateForActivation()
{
  if (curcontrol != LEVELS)
  {
    redraw();
  }
}

void save(int mode)
{
  EEPROM.update(0, SIGNATURE_BYTE);
  EEPROM.update(mode == BARMODE ? 1 : 21, curkit);
  EEPROM.put(mode == BARMODE ? 2 : 22, bkleds);
}

void loadLast(int mode)
{
  char sig = EEPROM.read(0);
  if (sig == SIGNATURE_BYTE)  // appears valid
  {
    curkit = EEPROM.read(mode == BARMODE ? 1 : 21);
    curkit = RANGE(curkit, 0, numKits() - 1);
    EEPROM.get(mode == BARMODE ? 2 : 22, bkleds);
  }
}

void setPixel(int x, int y, bool set)
{
  if (x < 0 || y < 0 || x > 7 || y > 7)
    return;
  x = 7 - x;
  if (set)
    bkleds[y] |= (0x1 << x);
  else
    bkleds[y] &= ~(0x1 << x);
}

void setCurPixel(bool set)
{
  setPixel(cursor_x, cursor_y, set);
  ledmatrix.writeDisplay();
}

void togglePixel(int x, int y)
{
  if (x < 0 || y < 0 || x > 7 || y > 7)
    return;
  x = 7 - x;
  bkleds[y] ^= (0x1 << x);
}

void toggleCurPixel()
{
  if (hidecursor)
  {
    drawCursor(); // restore the cursor
  }
  else
  {
    togglePixel(cursor_x, cursor_y);  // toggle the pixel and hide the cursor
    save(curMode);
    hideCursor();
  }
  redraw();
  ledmatrix.writeDisplay();
}

void clear()
{
  for (int i = 0; i < 8; ++i)
    bkleds[i] = 0b00000000;
  ledmatrix.clear();    
  ledmatrix.writeDisplay();
}

void setdefault()
{
  for (int i = 0; i < 8; ++i)
    bkleds[i] = defleds[i];
  redraw();
  hidecursor = true;
  ledmatrix.writeDisplay();
}

void redraw()
{
  if (curcontrol == LEVELS)
  {
    showLevels();
  }
  else
  {
    ledmatrix.clear();
    int l = curMode == BARMODE ? barlength : 8;
    ledmatrix.drawBitmap(0, 0, bkleds, l, l, active ? LED_GREEN : LED_YELLOW);
    for (int i = l; i < 8; ++i)
      ledmatrix.drawLine(i, 0, i, 7, LED_RED);
    if (!hidecursor)
      drawCursor();
    ledmatrix.writeDisplay();
  }
}

void switchMode(int oldmode, int newmode)
{
  save(oldmode);
  loadLast(newmode);
  hideCursor();
  redraw();
}

void showLevels()
{
  hideCursor();
  cursor_y = 0;
  uint8_t PROGMEM lvlleds[8];
  for (int i = 0; i < 8; ++i)
    lvlleds[i] = 0xFF << (8 - getLevel(i));
  ledmatrix.clear();    
  ledmatrix.drawBitmap(0, 0, lvlleds, 8, 8, LED_GREEN);
  updateCursor(0, 0);
}

void showSampleSet()
{
  hideCursor();
  ledmatrix.clear();
  ledmatrix.setCursor(1, 0);
  ledmatrix.print((char)('A' + curkit));
  ledmatrix.writeDisplay();
  save(curMode);
//  ledmatrix.clear();    
//  ledmatrix.drawBitmap(0, 0, bleds, 8, 8, LED_GREEN);
//  ledmatrix.writeDisplay();
}

void showSequencer()
{
  hideCursor();
  ledmatrix.clear();
  ledmatrix.setCursor(1, 0);
  ledmatrix.setTextColor(playsequencer ? LED_GREEN : LED_RED);
  ledmatrix.print(major ? '+' : '-');
  ledmatrix.writeDisplay();
}

void updateCursor(int dx, int dy)
{
  if (curcontrol == LEVELS)
  {
    if (dx)
    {
      adjLevel(cursor_y, dx);
    }
    
    int i = 0;
    int lvl = getLevel(cursor_y);
    while (i < 8)
    {
      ledmatrix.drawPixel(i, cursor_y, i < lvl ? LED_GREEN : LED_OFF);
      i++;
    }
    cursor_y = (8 + cursor_y + dy) % 8;
    lvl = getLevel(cursor_y);
    i = 0;
    while (i < lvl)
    {
      ledmatrix.drawPixel(i, cursor_y, LED_RED);
      i++;
    }
    if (lvl == 0)
      ledmatrix.drawPixel(0, cursor_y, LED_YELLOW);
    ledmatrix.writeDisplay();
  }
  else if (curcontrol == BARLENGTH)
  {
    barlength += dx;
    if (barlength < 2)
      barlength = 2;
    else if (barlength > 8)
      barlength = 8;
    hideCursor();
    redraw();
  }
  else
  {
    ledmatrix.drawPixel(cursor_x, cursor_y, isSet(cursor_x, cursor_y) ? (active ? LED_GREEN : LED_YELLOW) : LED_OFF);
    cursor_x = (barlength + cursor_x + dx) % barlength;
    cursor_y = (8 + cursor_y + dy) % 8;
    drawCursor();
    ledmatrix.writeDisplay();
  }
}

void setupMatrix()
{
  loadLast(curMode);
  hideCursor();
  ledmatrix.begin(0x70);
  ledmatrix.setTextColor(LED_ON);
  ledmatrix.setRotation(2);
  ledmatrix.setBrightness(8);
  ledmatrix.clear();
  redraw();
}
