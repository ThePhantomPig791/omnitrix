#include <SPI.h>
#include <TFT_eSPI.h>

#include <Vector.h> // arraylist >

#include "config.h"
#include "silhouettes.h"


// Screen
const int screenSize = 240;
const int bufferSize = screenSize * screenSize;
const int amountOfBuffers = 1; // match below
short currentBufferIndex = 0;
TFT_eSPI tft = TFT_eSPI();

TFT_eSprite buffer = TFT_eSprite(&tft);
int rotation = 0;


// Omni
uint16_t primaryColor = TFT_GREEN;
uint16_t secondaryColor = TFT_DARKGREY;

int progress = 0; // [0, maxProgress]
const int maxProgress = 130;

enum OmnitrixState {
  INACTIVE,
  SELECT,
  SCAN,
  TRANSFORM,
  RECAL
};
OmnitrixState state = INACTIVE;

const int maxTransformTime = 50;
int transformTime = -1;

// Aliens
uint8_t selectedAlien = 0;


// Rotary encoder
#define ENCODER_CLK 45
#define ENCODER_DT 41

int rotaryCounter = 0;
int currentStateCLK;
int lastStateCLK;

// Button
#define ENCODER_SW 39

int buttonState = 0;
int lastButtonState = 0;
const int maxPresses = 10;
unsigned long presses[maxPresses];
int pressCount = 0;
unsigned int pressLifetime = 300; // lifetime of a press, in ms



void setup(void) {
  Serial.begin(115200);
  Serial.println("Omnitrix initializing...");


  // Screen
  tft.init();
  tft.setPivot(screenSize / 2, screenSize / 2);

  buffer.setColorDepth(8);
  buffer.createSprite(screenSize, screenSize);


  // Rotary encoder
  pinMode(ENCODER_CLK, INPUT);
	pinMode(ENCODER_DT, INPUT);
  
	lastStateCLK = digitalRead(ENCODER_CLK);

  attachInterrupt(ENCODER_CLK, updateEncoder, CHANGE);
	attachInterrupt(ENCODER_DT, updateEncoder, CHANGE);

  // Button
  pinMode(ENCODER_SW, INPUT_PULLUP); // Use internal pullup resistor for the button


  // Done!
  Serial.println("Omnitrix initialized!");
}

void loop() {
  selectedAlien = wrap(rotaryCounter, 0, ALIENCOUNT - 1);
  if (state != SCAN) rotation = 18 * wrap(rotaryCounter, 0, 19); // 18 = 360 / 20 (i believe this is important because 20 is the amount of turns in a full rotation of the encoder)

  // Button
  buttonState = digitalRead(ENCODER_SW);
  if (buttonState == LOW && lastButtonState == HIGH) {
    presses[pressCount++] = millis();
  }
  lastButtonState = buttonState;

  if (presses[0] != 0 && millis() - presses[0] >= pressLifetime) {
    Serial.println("working...");
    if (presses[1] == 0) {
      Serial.println("single press");
      singlePress();
    } else if (presses[2] == 0) {
      Serial.println("double press");
      doublePress();
    } else if (presses[3] == 0) {
      Serial.println("triple press");
      triplePress();
    }
    for (int i = 0; i < maxPresses; i++) {
      presses[i] = 0;
    }
    pressCount = 0;
  }

  // loop depending on state
  switch (state) {
    case INACTIVE:
      drawDefaultMenu();
      decProgress(12);
      //delay(3000);
      //singlePress();
      break;
    case SELECT:
      drawDefaultMenu();
      if (progress >= maxProgress) {
        drawAlien();
      }
      incProgress(12);
      break;
    case TRANSFORM:
      buffer.fillSprite(getPrimaryColor());
      transformTime -= 1;
      if (transformTime <= -1) state = INACTIVE;
      break;
    case SCAN:
      rotation += 15;
      drawDefaultMenu();
      decProgress();
      break;
    case RECAL:
      drawDefaultMenu();
      decProgress();
      break;
  }
  

  // Draw to screen
  buffer.pushRotated(rotation);
}

void singlePress() {
  switch (state) {
    case INACTIVE: 
      state = SELECT;
      break;
    case SELECT:
      state = TRANSFORM;
      transformTime = maxTransformTime;
      progress = 0;
      break;
    case SCAN:
      state = INACTIVE;
      break;
    case RECAL:
      state = INACTIVE;
      break;
    case TRANSFORM:
      // do nothing
      break;
  }
}

void doublePress() {
  switch (state) {
    case INACTIVE: 
      state = SCAN;
      break;
    case SELECT:
      state = INACTIVE;
      break;
    case SCAN:
      state = INACTIVE;
      break;
    case RECAL:
      state = INACTIVE;
      break;
    case TRANSFORM:
      state = INACTIVE;
      break;
  }
}

void triplePress() {
  switch (state) {
    case INACTIVE: 
      state = RECAL;
      break;
    case SELECT:
      state = INACTIVE;
      break;
    case SCAN:
      state = INACTIVE;
      break;
    case RECAL:
      state = INACTIVE;
      break;
    case TRANSFORM:
      state = INACTIVE;
      break;
  }
}


void incProgress(int n) {
  if (progress < maxProgress) {
    progress += n;
    if (progress > maxProgress) progress = maxProgress;
  }
}
void incProgress() {
  if (progress < maxProgress) {
    progress += 1;
  }
}
void decProgress() {
  if (progress > 0) {
    progress -= 1;
  }
}
void decProgress(int n) {
  if (progress > 0) {
    progress -= n;
    if (progress < 0) progress = 0;
  }
}


// thanks https://lastminuteengineers.com/rotary-encoder-arduino-tutorial/
void updateEncoder() {
  if (state == RECAL) return;

	currentStateCLK = digitalRead(ENCODER_CLK);

	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
		if (digitalRead(ENCODER_DT) != currentStateCLK) {
			rotaryCounter--;
		  Serial.print("ccw");
		} else {
			rotaryCounter++;
		  Serial.print("cw");
		}

		Serial.print("rotation: ");
		Serial.println(rotaryCounter);
	}

	lastStateCLK = currentStateCLK;
}


void drawDefaultMenu() {
  buffer.fillSprite(getPrimaryColor());

  const int offset = 15;
  const int subTriangleOffset = 20;
  const int outerTriangleOffset = 40; // bascially makes the lines longer by this many pixels / basically scales the triangles
  
  const int half = 120;
  const int full = 240;

  // black triangles
  buffer.fillTriangle(0 + progress - outerTriangleOffset, 0 - outerTriangleOffset, half - offset + progress, half, 0 + progress - outerTriangleOffset, full + outerTriangleOffset, TFT_BLACK);
  buffer.fillTriangle(full - progress + outerTriangleOffset, 0 - outerTriangleOffset, half + offset - progress, half, full - progress + outerTriangleOffset, full + outerTriangleOffset, TFT_BLACK);
  // secondary color triangles
  buffer.fillTriangle(0 + progress - subTriangleOffset, 0, half - offset + progress - subTriangleOffset, half, 0 + progress - subTriangleOffset, full, secondaryColor);
  buffer.fillTriangle(full - progress + subTriangleOffset, 0, half + offset - progress + subTriangleOffset, half, full - progress + subTriangleOffset, full, secondaryColor);
  int y = (progress - offset + half);
  if (progress > offset) {
    // outside secondary color rects
    buffer.fillRect(0, 0, progress - offset, full, secondaryColor);
    buffer.fillRect(full + offset - progress, 0, half, full, secondaryColor);

    // the rectangle (square? two triangles?? idk) making the diamond when alien selection is active
    buffer.fillTriangle(half + offset - progress + 10, half, half, y, half, full - y, getPrimaryColor());
    buffer.fillTriangle(half - offset + progress - 10, half, half, y, half, full - y, getPrimaryColor());
    // the black lines doing the green diamond's outline
    drawLine(half + offset - progress + 10, half, half, y, TFT_BLACK, 15);
    drawLine(half, y, half - offset + progress - 10, half, TFT_BLACK, 15);
    drawLine(half - offset + progress - 10, half, half, full - y, TFT_BLACK, 15);
    drawLine(half, full - y, half + offset - progress + 10, half, TFT_BLACK, 15);
  }
}

void drawLine(int xs, int ys, int xe, int ye, unsigned int color, int width) {
  width /= 2;
  for (int i = -width; i <= width; i++) {
    buffer.drawLine(xs + i, ys, xe + i, ye, color);
    buffer.drawLine(xs, ys + i, xe, ye + i, color);
  }
}

void drawAlien() {
  for (int i = 0; i < 240 * 238; i++) {
    int x = i % 240, y = i / 238;
    if (alienSilhouettes[selectedAlien][i]) buffer.drawPixel(x, y, TFT_BLACK);
  }
}


uint16_t getPrimaryColor() {
  if (state == SCAN) return TFT_YELLOW;
  if (state == RECAL) return TFT_BLUE;
  return primaryColor;
}


// wrap(1, 0, 9) returns 1; wrap(11, 0, 9) returns 1; wrap(-3, 0, 9) returns 7
// https://stackoverflow.com/questions/707370/clean-efficient-algorithm-for-wrapping-integers-in-c
int wrap(int kX, int const kLowerBound, int const kUpperBound) {
    int range_size = kUpperBound - kLowerBound + 1;
    if (kX < kLowerBound)
        kX += range_size * ((kLowerBound - kX) / range_size + 1);
    return kLowerBound + (kX - kLowerBound) % range_size;
}