
// Simulated WPC DMD output
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
    }
  }
}

void loop() {
  uint8_t frame_cutoff = 0;
  for (int frame = 0; frame < 3; frame++) {
    frame_cutoff = (frame * 4) + 3;
    
    digitalWriteFast(DMD_ROW_DATA, HIGH);
    for (int row = 0; row < ROW_COUNT; row++) {
      for (int i = 0; i < 512; i++) {
        asm volatile("nop\n");
      }
      
      digitalWriteFast(DMD_OE, HIGH);
      for (int i = 0; i < 2592; i++) {
        asm volatile("nop\n");
      }
      
      for (int col = 0; col < 116; col++) {
        if (dots[row][col] > frame_cutoff) {
          digitalWriteFast(DMD_DOTS, HIGH);
        }
        else {
          digitalWriteFast(DMD_DOTS, LOW);
        }
        for (int i = 0; i < 3; i++) {
          asm volatile("nop\n nop\n nop\n");
        }
        digitalWriteFast(DMD_DOT_CLK, HIGH);
        for (int i = 0; i < 8; i++) {
          asm volatile("nop\n nop\n nop\n");
       }
        digitalWriteFast(DMD_DOT_CLK, LOW);
        for (int i = 0; i < 3; i++) {
          asm volatile("nop\n nop\n nop\n");
        }
      }
  
      digitalWriteFast(DMD_OE, LOW);
      
      for (int col = 116; col < 127; col++) {
        if (dots[row][col] > frame_cutoff) {
          digitalWriteFast(DMD_DOTS, HIGH);
        }
        else {
          digitalWriteFast(DMD_DOTS, LOW);
        }
        for (int i = 0; i < 3; i++) {
          asm volatile("nop\n nop\n nop\n");
        }
        digitalWriteFast(DMD_DOT_CLK, HIGH);
        for (int i = 0; i < 8; i++) {
          asm volatile("nop\n nop\n nop\n");
        }
        digitalWriteFast(DMD_DOT_CLK, LOW);
        for (int i = 0; i < 3; i++) {
          asm volatile("nop\n nop\n nop\n");
        }
      }
      
      if (dots[row][127] > frame_cutoff) {
        digitalWriteFast(DMD_DOTS, HIGH);
      }
      else {
        digitalWriteFast(DMD_DOTS, LOW);
      }
      for (int i = 0; i < 3; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      digitalWriteFast(DMD_DOT_CLK, HIGH);
      for (int i = 0; i < 8; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      digitalWriteFast(DMD_ROW_CLK, LOW);
      digitalWriteFast(DMD_DOT_LATCH, HIGH);
      for (int i = 0; i < 3; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      digitalWriteFast(DMD_ROW_CLK, HIGH);
      digitalWriteFast(DMD_DOT_LATCH, LOW);
      digitalWriteFast(DMD_DOT_CLK, LOW);
      digitalWriteFast(DMD_ROW_DATA, LOW);
    }
  }
}
