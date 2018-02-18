
// Simulated SAM DMD output
// Timings valid for 96MHz

// Output pins
/*
const static uint8_t  DMD_ROW_DATA = 17;
const static uint8_t  DMD_ROW_CLK = 16;
const static uint8_t  DMD_DOT_LATCH = 18;
const static uint8_t  DMD_OE = 19;
const static uint8_t  DMD_DOTS = 12;
const static uint8_t  DMD_DOT_CLK = 13;
*/
// RGB_DMD Mini v0.5 10/29/2015
const static uint8_t  DMD_ROW_DATA = 16;
const static uint8_t  DMD_ROW_CLK = 17;
const static uint8_t  DMD_DOT_LATCH = 18;
const static uint8_t  DMD_OE = 19;
const static uint8_t  DMD_DOTS = 12;
const static uint8_t  DMD_DOT_CLK = 13;

// DMD display size
#define ROW_COUNT 32
#define COL_COUNT 128
uint8_t dots[ROW_COUNT][COL_COUNT];

uint8_t sprite[8][8] = {
  {0, 0, 0, 5, 5, 0, 0, 0},
  {0, 0, 5, 5, 5, 5, 0, 0},
  {0, 5, 5, 5, 5, 5, 5, 0},
  {5, 5, 5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5, 5, 5},
  {0, 5, 5, 5, 5, 5, 5, 0},
  {0, 0, 5, 5, 5, 5, 0, 0},
  {0, 0, 0, 5, 5, 0, 0, 0}
};
int16_t sprite_x;
int16_t sprite_y;
int16_t sprite_dx;
int16_t sprite_dy;

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

/*
   for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
        dots[row][col] ^= sprite[row][col];
      }
    }
  sprite_x = 0;
  sprite_y = 0;
  sprite_dx = 3;
  sprite_dy = 1;
*/
 }

void loop() {
  uint8_t frame_cutoff = 0;
  uint8_t draw_x = sprite_x >> 3;
  uint8_t draw_y = sprite_y >> 3;

/*  
  if (draw_x <= 0) sprite_dx = 3;
  if (draw_x >= 120) sprite_dx = -3;
  if (draw_y <= 0) sprite_dy = 1;
  if (draw_y >= 24) sprite_dy = -1;
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
        dots[row + draw_y][col + draw_x] ^= sprite[row][col];
    }
  }
  sprite_x += sprite_dx;
  sprite_y += sprite_dy;
  draw_x = sprite_x >> 3;
  draw_y = sprite_y >> 3;
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
        dots[row + draw_y][col + draw_x] ^= sprite[row][col];
    }
  }
 */
   
  for (int frame = 0; frame < 3; frame++) {
    frame_cutoff = frame * 5;
    
    digitalWriteFast(DMD_ROW_DATA, HIGH);
    for (int row = 0; row < ROW_COUNT; row++) {
      digitalWriteFast(DMD_OE, HIGH);
      asm volatile("nop\n nop\n nop\n");  
          
      for (int col = 0; col < COL_COUNT; col++) {
        for (int i = 0; i < 4; i++) {
          asm volatile("nop\n nop\n nop\n");
        }
        asm volatile("nop\n nop\n nop\n");
        digitalWriteFast(DMD_DOT_CLK, LOW);
        for (int i = 0; i < 2; i++) {
          asm volatile("nop\n nop\n nop\n nop\n");
        }
        if (dots[row][col] > frame_cutoff) {
          digitalWriteFast(DMD_DOTS, HIGH);
        }
        else {
          digitalWriteFast(DMD_DOTS, LOW);
        }
        for (int i = 0; i < 2; i++) {
          asm volatile("nop\n nop\n nop\n nop\n");
        }
        digitalWriteFast(DMD_DOT_CLK, HIGH);
     }
       asm volatile("nop\n");
      digitalWriteFast(DMD_OE, LOW);
      for (int i = 0; i < 36; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      
      digitalWriteFast(DMD_ROW_CLK, LOW);
      digitalWriteFast(DMD_DOT_LATCH, HIGH);
      for (int i = 0; i < 18; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      
      digitalWriteFast(DMD_ROW_CLK, HIGH);
      digitalWriteFast(DMD_DOT_LATCH, LOW);
      for (int i = 0; i < 16; i++) {
        asm volatile("nop\n nop\n nop\n");
      }

      digitalWriteFast(DMD_ROW_DATA, LOW);
      asm volatile("nop\n");    
    }
     
    // Goofy Premier has extra rows!
    for (int row = 0; row < 3; row++) {       
    for (int col = 0; col < COL_COUNT; col++) {
      for (int i = 0; i < 4; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_CLK, LOW);
      for (int i = 0; i < 2; i++) {
        asm volatile("nop\n nop\n nop\n nop\n");
      }
      if (dots[0][col] > frame_cutoff) {
        digitalWriteFast(DMD_DOTS, HIGH);
      }
      else {
        digitalWriteFast(DMD_DOTS, LOW);
      }
      for (int i = 0; i < 2; i++) {
        asm volatile("nop\n nop\n nop\n nop\n");
      }
    }
      digitalWriteFast(DMD_ROW_CLK, LOW);
      digitalWriteFast(DMD_DOT_LATCH, HIGH);
      for (int i = 0; i < 18; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
      
      digitalWriteFast(DMD_ROW_CLK, HIGH);
      digitalWriteFast(DMD_DOT_LATCH, LOW);
      for (int i = 0; i < 16; i++) {
        asm volatile("nop\n nop\n nop\n");
      }
    }
    }
}

