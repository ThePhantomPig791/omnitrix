#include <SPI.h>
#include <TFT_eSPI.h>

#include <math.h>

#include "config.h"
#include "silhouettes.h"


// A and B are for the magnetic rotation system, C is for detecting if the core is up or down
#define SENSOR_A_PIN 15
#define SENSOR_A_GND 16
#define SENSOR_A_VCC 17

#define SENSOR_B_PIN 21
#define SENSOR_B_GND 33
#define SENSOR_B_VCC 34

#define SENSOR_C_PIN 46
#define SENSOR_C_GND 45
#define SENSOR_C_VCC 42

// #define BACKLIGHT_PIN 40


// #define R_PIN 0
// #define G_PIN 4
// #define B_PIN 2

int lastA = 0, lastB = 0;
int position = 0;


uint16_t primaryColor = TFT_GREEN;
uint16_t secondaryColor = TFT_DARKGREY;

#define MAX_PROGRESS 130
int progress = 0;  // [0, MAX_PROGRESS]

uint8_t selectedAlien = 0;


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite buffer = TFT_eSprite(&tft);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Omnitrix Initializing...");


  pinMode(SENSOR_A_GND, OUTPUT);
  digitalWrite(SENSOR_A_GND, LOW);
  pinMode(SENSOR_A_VCC, OUTPUT);
  digitalWrite(SENSOR_A_VCC, HIGH);
  pinMode(SENSOR_A_PIN, INPUT);

  pinMode(SENSOR_B_GND, OUTPUT);
  digitalWrite(SENSOR_B_GND, LOW);
  pinMode(SENSOR_B_VCC, OUTPUT);
  digitalWrite(SENSOR_B_VCC, HIGH);
  pinMode(SENSOR_B_PIN, INPUT);

  pinMode(SENSOR_C_GND, OUTPUT);
  digitalWrite(SENSOR_C_GND, LOW);
  pinMode(SENSOR_C_VCC, OUTPUT);
  digitalWrite(SENSOR_C_VCC, HIGH);
  pinMode(SENSOR_C_PIN, INPUT);


  // pinMode(R_PIN, OUTPUT);
  // pinMode(G_PIN, OUTPUT);
  // pinMode(B_PIN, OUTPUT);


  // analogWrite(R_PIN, 0);
  // analogWrite(G_PIN, 255);

  // pinMode(BACKLIGHT_PIN, OUTPUT);
  // digitalWrite(BACKLIGHT_PIN, LOW);

  tft.init();
  tft.setPivot(120, 120);

  buffer.setColorDepth(8);
  buffer.createSprite(240, 240);
}

void loop() {
  Serial.println(progress);

  buffer.fillSprite(TFT_BLACK);
  buffer.drawLine(120, 120, 220, 120, TFT_WHITE);
  drawDefaultMenu();


  if (digitalRead(SENSOR_C_PIN) == LOW) {
    incProgress(11);
  } else {
    decProgress(11);
  }


  if (progress >= MAX_PROGRESS * 0.9) {
    Serial.println("proog");
    int a = digitalRead(SENSOR_A_PIN);
    int b = digitalRead(SENSOR_B_PIN);

    if (a != lastA || b != lastB) {
      // if A just turned on; rising-edge
      if (lastA == 0 && a == 1) {
        // Check direction
        if (b == 0) position--;  // Counterclockwise
        else position++;         // Clockwise

        Serial.print("position: ");
        Serial.println(position);
      }
    }

    lastA = a;
    lastB = b;

    //Serial.print(b);
    //Serial.print(" | ");
    //Serial.println(a);

    selectedAlien = wrap(position, 0, ALIENCOUNT - 1);

    drawAlien();

    buffer.drawNumber(a, 90, 120);
    buffer.drawNumber(b, 150, 120);
  }

  buffer.pushSprite(0, 0);

  //delay(10);
}


void drawDefaultMenu() {
  buffer.fillSprite(getPrimaryColor());

  const int offset = 15;
  const int subTriangleOffset = 20;
  const int outerTriangleOffset = 40;  // bascially makes the lines longer by this many pixels / basically scales the triangles

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
    buffer.drawWideLine(half + offset - progress + 10, half, half, y, 15, TFT_BLACK);
    buffer.drawWideLine(half, y, half - offset + progress - 10, half, 15, TFT_BLACK);
    buffer.drawWideLine(half - offset + progress - 10, half, half, full - y, 15, TFT_BLACK);
    buffer.drawWideLine(half, full - y, half + offset - progress + 10, half, 15, TFT_BLACK);
  }
}

void drawLine(int xs, int ys, int xe, int ye, unsigned int color, int width) {
  width /= 2;
  for (float i = -width; i <= width; i++) {
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
  // if (state == SCAN) return TFT_YELLOW;
  // if (state == RECAL) return TFT_BLUE;
  return primaryColor;
}


void incProgress(int n) {
  if (progress + n <= MAX_PROGRESS) {
    progress += n;
  } else if (MAX_PROGRESS - progress < n) {
    progress = MAX_PROGRESS;
  }
}

void decProgress(int n) {
  if (progress - n >= 0) {
    progress -= n;
  } else if (n > progress) {
    progress = 0;
  }
}


// wrap(1, 0, 9) returns 1; wrap(11, 0, 9) returns 1; wrap(-3, 0, 9) returns 7
// https://stackoverflow.com/questions/707370/clean-efficient-algorithm-for-wrapping-integers-in-c
int wrap(int kX, int const kLowerBound, int const kUpperBound) {
  int range_size = kUpperBound - kLowerBound + 1;
  if (kX < kLowerBound)
    kX += range_size * ((kLowerBound - kX) / range_size + 1);
  return kLowerBound + (kX - kLowerBound) % range_size;
}
