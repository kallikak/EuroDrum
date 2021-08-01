#pragma once

void setupMatrix();

void showLevels();

void showSampleSet();

void showSequencer();

void updateCursor(int dx, int dy);

void setPixel(int x, int y, bool set);

void setCurPixel(bool set);

void togglePixel(int x, int y);

void toggleCurPixel();

void clear();

void redraw();

void switchMode(int oldmode, int newmode);

void setdefault();

void updateForActivation();

bool isSet(int x, int y);
