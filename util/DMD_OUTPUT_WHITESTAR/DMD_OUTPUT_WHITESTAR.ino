
// Simulated SAM DMD output
// Timings valid for 96MHz

// Output pins
const static uint8_t  DMD_ROW_DATA = 17;
const static uint8_t  DMD_ROW_CLK = 16;
const static uint8_t  DMD_DOT_LATCH = 18;
const static uint8_t  DMD_OE = 19;
const static uint8_t  DMD_DOTS = 12;
const static uint8_t  DMD_DOT_CLK = 13;

// DMD display size
#define ROW_COUNT 32
#define COL_COUNT 128
uint8_t dots[ROW_COUNT][COL_COUNT];

void setup() {
  pinMode(DMD_ROW_DATA, OUTPUT);
  pinMode(DMD_ROW_CLK, OUTPUT);
  pinMode(DMD_DOT_LATCH, OUTPUT);
  pinMode(DMD_OE, OUTPUT);
  pinMode(DMD_DOTS, OUTPUT);
  pinMode(DMD_DOT_CLK, OUTPUT);

  for (int row = 0; row < ROW_COUNT; row++) {
    for (int col = 0; col < 64; col++) {
      if (((row + col) % 8) == 0) {
        dots[row][col] = 15;
      }
      else {
        dots[row][col] = 0;
      }
      if (row < 16)
      {
        dots[row][col + 64] = col >> 2;
      }
      else
      {
        dots[row][col + 64] = 15 - (col >> 2);
      }

      if (col < 16)
      {
        dots[row][col] = col;
      }
    }
  }
}

void loop() {
  uint8_t frame_mask = 0;
  for (int row = 0; row < ROW_COUNT; row++) {
    if (row == 0) {
      digitalWriteFast(DMD_ROW_DATA, HIGH);
    }
    else {
      digitalWriteFast(DMD_ROW_DATA, LOW);
    }
    digitalWriteFast(DMD_DOT_LATCH, HIGH);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
    }
    digitalWriteFast(DMD_ROW_CLK, HIGH);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
    }
    digitalWriteFast(DMD_DOT_LATCH, LOW);
    asm volatile("nop\n");
    digitalWriteFast(DMD_OE, HIGH);
    asm volatile("nop\n nop\n");
    
    // Low bit
    for (int col = 0; col < 128; col++) {
      if (dots[row][col] & 0x04) {
        digitalWriteFast(DMD_DOTS, HIGH);
      }
      else {
        digitalWriteFast(DMD_DOTS, LOW);
      }
      asm volatile("nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_CLK, HIGH);
      for (int i = 0; i < 4; i++) {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_CLK, LOW);
      for (int i = 0; i < 3; i++) {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
    }
    
    digitalWriteFast(DMD_OE, LOW);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
    }
    digitalWriteFast(DMD_DOT_LATCH, HIGH);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
    }
    digitalWriteFast(DMD_ROW_CLK, LOW);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
    }
    digitalWriteFast(DMD_DOT_LATCH, LOW);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
    }
    digitalWriteFast(DMD_OE, HIGH);
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n");
    }
 
    // High bit
    for (int col = 0; col < 128; col++) {
      if (dots[row][col] & 0x08) {
        digitalWriteFast(DMD_DOTS, HIGH);
      }
      else {
        digitalWriteFast(DMD_DOTS, LOW);
      }
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_CLK, HIGH);
      for (int i = 0; i < 8; i++) {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_CLK, LOW);
      for (int i = 0; i < 7; i++) {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
    }
    
    for (int i = 0; i < 4; i++) {
      asm volatile("nop\n nop\n nop\n nop\n");
    }
    
    digitalWriteFast(DMD_OE, LOW);
  }
}
