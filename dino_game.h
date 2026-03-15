#ifndef DINO_GAME_H
#define DINO_GAME_H

#include "OLEDDisplay.h"

// Button pin — GPIO3 (RX) on ESP01
// GPIO0/2 are used for I2C, so the board button is likely on GPIO3
#define BUTTON_PIN 3

#define GROUND_Y     54
#define DINO_X       10

// --- T-Rex sprites (20x22, XBM format) ---

// Standing / in-air pose
const uint8_t dino_stand[] PROGMEM = {
  // Row 0-4: head
  0x00, 0xFC, 0x03,  //          XXXXXXXX
  0x00, 0xFE, 0x07,  //         XXXXXXXXXX
  0x00, 0xFE, 0x07,  //         XXXXXXXXXX
  0x00, 0xDE, 0x07,  //         XXXX X XXXX  (eye)
  0x00, 0xFE, 0x07,  //         XXXXXXXXXX
  // Row 5-6: jaw
  0x00, 0xFE, 0x01,  //         XXXXXXXX
  0x00, 0xFE, 0x03,  //         XXXXXXXXX
  // Row 7-9: arm + body
  0x10, 0xFF, 0x03,  //     X   XXXXXXXXXX
  0x10, 0xFF, 0x01,  //     X   XXXXXXXXX
  0x30, 0xFF, 0x01,  //     XX  XXXXXXXXX
  // Row 10-13: body
  0xF0, 0xFF, 0x01,  //     XXXXXXXXXXXXX
  0xF8, 0xFF, 0x01,  //    XXXXXXXXXXXXXX
  0xF8, 0xFF, 0x00,  //    XXXXXXXXXXXXX
  0xF0, 0x7F, 0x00,  //     XXXXXXXXXXX
  // Row 14-17: body taper
  0xE0, 0x7F, 0x00,  //      XXXXXXXXXX
  0xC0, 0x3F, 0x00,  //       XXXXXXXX
  0x80, 0x1F, 0x00,  //        XXXXXX
  0x00, 0x0F, 0x00,  //         XXXX
  // Row 18-21: legs (standing)
  0x00, 0x0B, 0x00,  //         XX X
  0x00, 0x0B, 0x00,  //         XX X
  0x00, 0x09, 0x00,  //         X  X
  0x00, 0x09, 0x00,  //         X  X
};

// Running frame 1 (left leg forward)
const uint8_t dino_run1[] PROGMEM = {
  0x00, 0xFC, 0x03,
  0x00, 0xFE, 0x07,
  0x00, 0xFE, 0x07,
  0x00, 0xDE, 0x07,
  0x00, 0xFE, 0x07,
  0x00, 0xFE, 0x01,
  0x00, 0xFE, 0x03,
  0x10, 0xFF, 0x03,
  0x10, 0xFF, 0x01,
  0x30, 0xFF, 0x01,
  0xF0, 0xFF, 0x01,
  0xF8, 0xFF, 0x01,
  0xF8, 0xFF, 0x00,
  0xF0, 0x7F, 0x00,
  0xE0, 0x7F, 0x00,
  0xC0, 0x3F, 0x00,
  0x80, 0x1F, 0x00,
  0x00, 0x0F, 0x00,
  // Legs: left forward, right back
  0x00, 0x06, 0x00,  //          XX
  0x00, 0x06, 0x00,  //          XX
  0x00, 0x01, 0x00,  //         X
  0x00, 0x01, 0x00,  //         X
};

// Running frame 2 (right leg forward)
const uint8_t dino_run2[] PROGMEM = {
  0x00, 0xFC, 0x03,
  0x00, 0xFE, 0x07,
  0x00, 0xFE, 0x07,
  0x00, 0xDE, 0x07,
  0x00, 0xFE, 0x07,
  0x00, 0xFE, 0x01,
  0x00, 0xFE, 0x03,
  0x10, 0xFF, 0x03,
  0x10, 0xFF, 0x01,
  0x30, 0xFF, 0x01,
  0xF0, 0xFF, 0x01,
  0xF8, 0xFF, 0x01,
  0xF8, 0xFF, 0x00,
  0xF0, 0x7F, 0x00,
  0xE0, 0x7F, 0x00,
  0xC0, 0x3F, 0x00,
  0x80, 0x1F, 0x00,
  0x00, 0x0F, 0x00,
  // Legs: right forward, left back
  0x00, 0x09, 0x00,  //         X  X
  0x00, 0x08, 0x00,  //            X
  0x00, 0x18, 0x00,  //            XX
  0x00, 0x00, 0x00,
};

#define DINO_W 20
#define DINO_H 22

// --- Cactus types ---
// Small single (5x12)
const uint8_t cactus_small[] PROGMEM = {
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x15,  // X X X
  0x15,  // X X X
  0x0E,  //  XXX
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
};

// Tall single (5x18)
const uint8_t cactus_tall[] PROGMEM = {
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x05,  // X X
  0x05,  // X X
  0x07,  // XXX
  0x14,  //   X X
  0x14,  //   X X
  0x1C,  //   XXX
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
  0x04,  //   X
};

// Double cactus (11x12)
const uint8_t cactus_double[] PROGMEM = {
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
  0x15, 0x05,  // X X X  X X X
  0x15, 0x05,  // X X X  X X X
  0x0E, 0x03,  //  XXX    XX
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
  0x04, 0x01,  //   X      X
};

// --- Game constants ---
#define MAX_OBSTACLES  4
#define MAX_CLOUDS     3
#define LONG_PRESS_MS  800

// Chrome-like physics (positive = upward)
// Jump peaks at ~30px (clears tall cacti at 18px with room to spare)
const float GRAVITY = -1.2;
const float JUMP_FORCE = 8.0;
// Faster gravity on descent for snappier feel
const float GRAVITY_DOWN = -1.8;

// --- Game state ---
enum GameState { GAME_IDLE, GAME_PLAY, GAME_OVER };
GameState gameState = GAME_IDLE;

float dinoY = 0;
float dinoVel = 0;
bool dinoOnGround = true;
int animFrame = 0;
bool nightMode = false;

int score = 0;
int highScore = 0;
int gameSpeed = 2;
int spawnTimer = 0;
int groundOffset = 0;

// Obstacle types
enum CactusType { CACTUS_SMALL, CACTUS_TALL, CACTUS_DOUBLE };

struct Obstacle {
  int x;
  CactusType type;
  bool active;
};
Obstacle obstacles[MAX_OBSTACLES];

// Clouds
struct Cloud {
  int x;
  int y;
  bool active;
};
Cloud clouds[MAX_CLOUDS];
int cloudTimer = 0;

// Button state
volatile bool btnPressed = false;  // set by interrupt
bool btnDown = false;
unsigned long btnPressStart = 0;
bool longPressHandled = false;

void IRAM_ATTR buttonISR() {
  btnPressed = true;
}

// --- Functions ---

void gameInit() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void spawnCloud() {
  for (int i = 0; i < MAX_CLOUDS; i++) {
    if (!clouds[i].active) {
      clouds[i].x = 128 + random(0, 40);
      clouds[i].y = random(5, 25);
      clouds[i].active = true;
      cloudTimer = random(40, 100);
      return;
    }
  }
}

void gameExitToWeather() {
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
  gameState = GAME_IDLE;
}

void gameReset() {
  // Attach interrupt for reliable short press detection
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  btnPressed = false;
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
  dinoY = 0;
  dinoVel = 0;
  dinoOnGround = true;
  animFrame = 0;
  nightMode = false;
  score = 0;
  gameSpeed = 3;
  spawnTimer = 40;
  groundOffset = 0;
  for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
  for (int i = 0; i < MAX_CLOUDS; i++) clouds[i].active = false;
  cloudTimer = 20;
  gameState = GAME_PLAY;
}

// Returns: 0=nothing, 1=short press, 2=long press
int readButton() {
  bool pressed = digitalRead(BUTTON_PIN) == LOW;
  int result = 0;

  if (pressed && !btnDown) {
    btnDown = true;
    btnPressStart = millis();
    longPressHandled = false;
  } else if (pressed && btnDown) {
    if (!longPressHandled && (millis() - btnPressStart > LONG_PRESS_MS)) {
      result = 2;
      longPressHandled = true;
    }
  } else if (!pressed && btnDown) {
    if (!longPressHandled) result = 1;
    btnDown = false;
  }

  return result;
}

int getCactusWidth(CactusType t) {
  switch (t) {
    case CACTUS_SMALL:  return 5;
    case CACTUS_TALL:   return 5;
    case CACTUS_DOUBLE: return 11;
  }
  return 5;
}

int getCactusHeight(CactusType t) {
  switch (t) {
    case CACTUS_SMALL:  return 12;
    case CACTUS_TALL:   return 18;
    case CACTUS_DOUBLE: return 12;
  }
  return 12;
}

void drawCactusSprite(OLEDDisplay *d, int x, CactusType t) {
  int h = getCactusHeight(t);
  int w = getCactusWidth(t);
  int y = GROUND_Y - h;
  switch (t) {
    case CACTUS_SMALL:
      d->drawXbm(x, y, w, h, cactus_small);
      break;
    case CACTUS_TALL:
      d->drawXbm(x, y, w, h, cactus_tall);
      break;
    case CACTUS_DOUBLE:
      d->drawXbm(x, y, w, h, cactus_double);
      break;
  }
}

void drawDino(OLEDDisplay *d) {
  int baseY = GROUND_Y - (int)dinoY - DINO_H;
  const uint8_t *sprite;

  if (!dinoOnGround || gameState == GAME_OVER) {
    sprite = dino_stand;
  } else {
    // Alternate legs every 4 frames
    sprite = (animFrame / 4) % 2 == 0 ? dino_run1 : dino_run2;
  }
  d->drawXbm(DINO_X, baseY, DINO_W, DINO_H, sprite);
}

void drawCloud(OLEDDisplay *d, int x, int y) {
  // Simple cloud shape
  d->drawHorizontalLine(x + 2, y, 8);
  d->drawHorizontalLine(x + 1, y + 1, 10);
  d->drawHorizontalLine(x, y + 2, 12);
  d->drawHorizontalLine(x + 1, y + 3, 10);
  d->drawHorizontalLine(x + 3, y + 4, 6);
}

bool checkCollision() {
  // Dino hitbox (slightly smaller than sprite for fair gameplay)
  int dy = GROUND_Y - (int)dinoY;
  int dinoLeft = DINO_X + 3;
  int dinoRight = DINO_X + DINO_W - 2;
  int dinoTop = dy - DINO_H + 3;
  int dinoBottom = dy;

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    int cw = getCactusWidth(obstacles[i].type);
    int ch = getCactusHeight(obstacles[i].type);
    int cx = obstacles[i].x;

    if (dinoRight > cx && dinoLeft < cx + cw &&
        dinoBottom > GROUND_Y - ch && dinoTop < GROUND_Y) {
      return true;
    }
  }
  return false;
}

void spawnObstacle() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) {
      obstacles[i].x = 128 + random(0, 30);
      // Weighted random: more small cacti early, bigger ones later
      int r = random(0, 10);
      if (gameSpeed < 3 || r < 5) {
        obstacles[i].type = CACTUS_SMALL;
      } else if (r < 8) {
        obstacles[i].type = CACTUS_TALL;
      } else {
        obstacles[i].type = CACTUS_DOUBLE;
      }
      obstacles[i].active = true;
      // Faster speed = shorter gaps, but with minimum distance
      spawnTimer = random(50, 110) - (gameSpeed * 6);
      if (spawnTimer < 25) spawnTimer = 25;
      return;
    }
  }
}

void gameUpdate() {
  // Jump physics — faster fall than rise (Chrome-like)
  if (!dinoOnGround) {
    float g = dinoVel < 0 ? GRAVITY_DOWN : GRAVITY;
    dinoVel += g;
    dinoY += dinoVel;
    if (dinoY <= 0) {
      dinoY = 0;
      dinoVel = 0;
      dinoOnGround = true;
    }
  }

  // Move obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    obstacles[i].x -= gameSpeed;
    if (obstacles[i].x < -getCactusWidth(obstacles[i].type) - 5) {
      obstacles[i].active = false;
    }
  }

  // Move clouds (slower than ground)
  for (int i = 0; i < MAX_CLOUDS; i++) {
    if (!clouds[i].active) continue;
    clouds[i].x -= 1;
    if (clouds[i].x < -14) clouds[i].active = false;
  }

  // Spawn obstacles
  spawnTimer--;
  if (spawnTimer <= 0) spawnObstacle();

  // Spawn clouds
  cloudTimer--;
  if (cloudTimer <= 0) spawnCloud();

  // Ground scroll
  groundOffset = (groundOffset + gameSpeed) % 128;

  // Collision
  if (checkCollision()) {
    gameState = GAME_OVER;
    if (score > highScore) highScore = score;
    return;
  }

  // Score & difficulty (Chrome-like: speed up every 100 points)
  score++;
  if (score % 300 == 0 && gameSpeed < 8) {
    gameSpeed++;
  }

  // Night mode toggle every 700 points (like Chrome)
  if (score % 3500 == 0 && score > 0) {
    nightMode = !nightMode;
  }

  animFrame++;
}

// Format score as 5-digit string like Chrome
String formatScore(int s) {
  int pts = s / 5;
  if (pts > 99999) pts = 99999;
  if (pts < 0) pts = 0;
  char buf[6];
  buf[0] = '0' + (pts / 10000) % 10;
  buf[1] = '0' + (pts / 1000) % 10;
  buf[2] = '0' + (pts / 100) % 10;
  buf[3] = '0' + (pts / 10) % 10;
  buf[4] = '0' + pts % 10;
  buf[5] = '\0';
  return String(buf);
}

void gameDraw(OLEDDisplay *d) {
  d->clear();

  if (nightMode) {
    // Fill screen white, then draw in black (inverted)
    d->setColor(WHITE);
    d->fillRect(0, 0, 128, 64);
    d->setColor(BLACK);
  }

  // Clouds
  for (int i = 0; i < MAX_CLOUDS; i++) {
    if (clouds[i].active) {
      drawCloud(d, clouds[i].x, clouds[i].y);
    }
  }

  // Ground line
  d->drawHorizontalLine(0, GROUND_Y, 128);

  // Ground texture (bumps scrolling with ground)
  for (int i = -groundOffset % 6; i < 128; i += 6) {
    if (i >= 0) d->setPixel(i, GROUND_Y + 2);
  }
  for (int i = -groundOffset % 15 + 3; i < 128; i += 15) {
    if (i >= 0) d->drawHorizontalLine(i, GROUND_Y + 3, 2);
  }
  for (int i = -groundOffset % 10 + 7; i < 128; i += 10) {
    if (i >= 0) d->setPixel(i, GROUND_Y + 4);
  }

  // Obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      drawCactusSprite(d, obstacles[i].x, obstacles[i].type);
    }
  }

  // Dino
  drawDino(d);

  // Score (top right, Chrome-style)
  d->setColor(nightMode ? BLACK : WHITE);
  d->setFont(ArialMT_Plain_10);
  d->setTextAlignment(TEXT_ALIGN_RIGHT);
  d->drawString(127, 0, formatScore(score));

  if (highScore > 0) {
    d->setTextAlignment(TEXT_ALIGN_RIGHT);
    d->drawString(74, 0, "HI " + formatScore(highScore));
  }

  // Game over overlay
  if (gameState == GAME_OVER) {
    // Invert a box behind text for readability
    d->setColor(nightMode ? BLACK : WHITE);
    d->setTextAlignment(TEXT_ALIGN_CENTER);
    d->setFont(ArialMT_Plain_16);
    d->drawString(64, 16, "GAME OVER");
    d->setFont(ArialMT_Plain_10);
    // Blink "press to restart"
    if ((millis() / 500) % 2 == 0) {
      d->drawString(64, 36, "Taste drücken");
    }
  }

  d->setColor(WHITE);
  d->display();
}

// Main game tick — call from loop(), returns true while in game mode
bool gameLoop(OLEDDisplay *d) {
  int btn = readButton();  // polling for long press detection

  switch (gameState) {
    case GAME_IDLE:
      if (btn == 2) {
        gameReset();
        return true;
      }
      return false;

    case GAME_PLAY:
      if (btn == 2) {
        gameExitToWeather();
        return false;
      }
      // Jump via interrupt flag (reliable for quick presses)
      if (btnPressed && dinoOnGround) {
        dinoVel = JUMP_FORCE;
        dinoOnGround = false;
      }
      btnPressed = false;
      gameUpdate();
      gameDraw(d);
      return true;

    case GAME_OVER:
      if (btn == 2) {
        gameExitToWeather();
        return false;
      }
      if (btnPressed) {
        btnPressed = false;
        gameReset();
        return true;
      }
      gameDraw(d);
      return true;
  }
  return false;
}

#endif
