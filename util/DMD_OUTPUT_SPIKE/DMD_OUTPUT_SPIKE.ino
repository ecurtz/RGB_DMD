
// Simulated SPIKE DMD output
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

void setup()
{
  pinMode(DMD_ROW_DATA, OUTPUT);
  pinMode(DMD_ROW_CLK, OUTPUT);
  pinMode(DMD_DOT_LATCH, OUTPUT);
  pinMode(DMD_OE, OUTPUT);
  pinMode(DMD_DOTS, OUTPUT);
  pinMode(DMD_DOT_CLK, OUTPUT);

  digitalWriteFast(DMD_ROW_DATA, HIGH);

  for (int row = 0; row < ROW_COUNT; row++)
  {
    for (int col = 0; col < 64; col++)
    {
      if (((row + col) % 8) == 0)
      {
          dots[row][col] = row % 15;
      }
      else
      {
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

uint16_t delay_loops[] = {28, 348, 1000, 2250};

void loop()
{
  uint8_t frame_mask = 0;
  for (int frame = 0; frame < 4; frame++)
  {
    frame_mask = 1 << frame;

    for (int row = 0; row < ROW_COUNT; row++)
    {

      for (int col = 0; col < 12; col++)
      {
        digitalWriteFast(DMD_DOT_CLK, LOW);
        asm volatile("nop\n nop\n nop\n");
        if (dots[row][col] & frame_mask)
        {
          digitalWriteFast(DMD_DOTS, HIGH);
        }
        else
        {
          digitalWriteFast(DMD_DOTS, LOW);
        }
        asm volatile("nop\n nop\n nop\n");
        
        digitalWriteFast(DMD_DOT_CLK, HIGH);
        asm volatile("nop\n nop\n nop\n nop\n");
      }
        
      digitalWriteFast(DMD_ROW_CLK, HIGH);
     
      for (int col = 12; col < 24; col++)
      {       
        digitalWriteFast(DMD_DOT_CLK, LOW);
        asm volatile("nop\n nop\n nop\n");
        if (dots[row][col] & frame_mask)
        {
          digitalWriteFast(DMD_DOTS, HIGH);
        }
        else
        {
          digitalWriteFast(DMD_DOTS, LOW);
        }
        asm volatile("nop\n nop\n nop\n");
        
        digitalWriteFast(DMD_DOT_CLK, HIGH);
        asm volatile("nop\n nop\n nop\n nop\n");
      }
      
      digitalWriteFast(DMD_OE, HIGH);
      
      for (int col = 24; col < 128; col++)
      {
        digitalWriteFast(DMD_DOT_CLK, LOW);
        asm volatile("nop\n nop\n nop\n");
        if (dots[row][col] & frame_mask)
        {
          digitalWriteFast(DMD_DOTS, HIGH);
        }
        else
        {
          digitalWriteFast(DMD_DOTS, LOW);
        }
        asm volatile("nop\n nop\n nop\n");
        
        digitalWriteFast(DMD_DOT_CLK, HIGH);
        asm volatile("nop\n nop\n nop\n nop\n");
      }

      // 3.6uS    
      // 36uS    
      // 102uS    
      // 233uS
      for (int i = 0; i < delay_loops[frame]; i++)
      {
         asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n");
      }
      
       digitalWriteFast(DMD_OE, LOW);
      // 875 ns
      for (int i = 0; i < 6; i++)
      {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n");

      if (row == 0)
      {
        digitalWriteFast(DMD_ROW_DATA, HIGH);
      }
      else
      {
        digitalWriteFast(DMD_ROW_DATA, LOW);
      }
      // 164 ns
      asm volatile("nop\n nop\n nop\n nop\n");
      
      digitalWriteFast(DMD_ROW_CLK, LOW);
      
      // 210 ns
      for (int i = 0; i < 2; i++)
      {
        asm volatile("nop\n nop\n nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n nop\n nop\n");

      digitalWriteFast(DMD_DOTS, HIGH);

      digitalWriteFast(DMD_DOT_LATCH, HIGH);
      
      // 910 ns
      for (int i = 0; i < 7; i++)
      {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_LATCH, LOW);

      // 1080 ns
      for (int i = 0; i < 9; i++)
      {
        asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      }
      asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
      digitalWriteFast(DMD_DOT_CLK, LOW);
    
      asm volatile("nop\n nop\n nop\n");
   } // end row loop
  } // end frame loop
}
