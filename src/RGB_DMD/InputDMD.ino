// Copyright (c) 2015 Eli Curtz

#include <DMAChannel.h>
#include <FastCRC.h>

#include "InputDMD.h"
#include "MatrixSetup.h"
#include "Palettes.h"

// Modified input method using DMA completion
//#define DMA_COMPLETION_ISR 1

// Comparison constants
#define SMALLER -1
#define EQUAL 0
#define LARGER 1

// Dot data
#define PLANE_COUNT 4
#define PLANE_COUNT_WHITESTAR 2
#define PLANE_BLEND_WPC 3
#define PLANE_SIZE ROW_COUNT * ROW_LENGTH
#define PLANE_BYTES PLANE_SIZE * sizeof(uint16_t)
uint16_t plane_buffer[ROW_LENGTH * 2];
uint16_t planes[PLANE_SIZE * PLANE_COUNT];
typedef uint16_t wpc_plane[ROW_COUNT][ROW_LENGTH];
wpc_plane* wpc_planes = (wpc_plane*) planes;
typedef uint16_t whitestar_plane[PLANE_COUNT_WHITESTAR][ROW_LENGTH];
whitestar_plane* whitestar_planes = (whitestar_plane*) planes;
typedef uint16_t sam_plane[PLANE_COUNT][ROW_LENGTH];
sam_plane* sam_planes = (sam_plane*) planes;

// Checksums
FastCRC32 CRC32;
uint32_t checksums[ROW_COUNT];
uint8_t checksumRowFirst = 12;
uint8_t checksumRowLast = 18;
uint8_t checksumByteFirst = 0;
uint8_t checksumByteLast = 5;
uint32_t lastCheck;

// Variables used in interrupts
volatile uint32_t plane = 0;
volatile uint32_t row = 0;
volatile bool rowZero = false;
volatile bool gotRow = false;
volatile uint32_t receivedRows = 0;

// DMD signal
uint8_t dmdDataFormat = FORMAT_UNKNOWN;
uint8_t planesPerRow = 1;
uint8_t planeOrder[4] = {0, 1, 2, 3};
unsigned long planeStart;
unsigned long planeTimes[PLANE_COUNT];
unsigned long activePlaneTimes[PLANE_COUNT];
bool timingChanged = false;

// Input pins
const static uint8_t  DMD_ROW_DATA = 17;
#define DMD_ROW_DATA_BITMASK CORE_PIN17_BITMASK
const static uint8_t  DMD_ROW_CLK = 16;
#define DMD_ROW_CLK_BITMASK CORE_PIN16_BITMASK
const static uint8_t  DMD_DOT_LATCH = 18;
#define DMD_DOT_LATCH_BITMASK CORE_PIN18_BITMASK
const static uint8_t  DMD_OE = 19;
#define DMD_OE_BITMASK CORE_PIN19_BITMASK

const static uint8_t  DMD_CS = MOSI;

// DMA channel
DMAChannel* dmaSPI0rx;

/*
 Compare two elapsed times with some slop to allow minor variation
*/
int8_t compareIntervals(uint32_t interval1, uint32_t interval2)
{
  uint32_t intervalFraction;
  if (interval1 > 32)
    intervalFraction = interval1 >> 3;
  else
    intervalFraction = interval1 >> 2;
  
  if ((interval1 + intervalFraction) < interval2)
    return LARGER;
  if ((interval1 - intervalFraction) > interval2)
    return SMALLER;
  return EQUAL;
}

/*
 Set the pin modes used by the DMD input signals
*/
void initializeInputDMD()
{
  pinMode(DMD_ROW_DATA, INPUT);
  pinMode(DMD_ROW_CLK, INPUT);
  pinMode(DMD_DOT_LATCH, INPUT/*_PULLDOWN*/);
  *portConfigRegister(DMD_DOT_LATCH) |= PORT_PCR_PE;
  *portConfigRegister(DMD_DOT_LATCH) &= ~PORT_PCR_PS;
  pinMode(DMD_OE, INPUT/*_PULLDOWN*/);
  *portConfigRegister(DMD_OE) |= PORT_PCR_PE;
  *portConfigRegister(DMD_OE) &= ~PORT_PCR_PS;
  pinMode(DMD_CS, OUTPUT);
}

/*
 Look for a DMD signal on the input pins
*/
bool hasInputDMD()
{
  // Enable counter interrupts to check for DMD signal
  row = 0; plane = 0;
  attachInterrupt(digitalPinToInterrupt(DMD_ROW_CLK), row_count_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(DMD_DOT_LATCH), plane_count_isr, FALLING);
  // Wait 1/8th of a second
  delay(128);
  // Disable counter interrupts
  detachInterrupt(digitalPinToInterrupt(DMD_ROW_CLK));
  detachInterrupt(digitalPinToInterrupt(DMD_DOT_LATCH));
  
  if (row < ROW_COUNT)
    return false;

  planesPerRow = (plane + (ROW_COUNT >> 3)) / row;
  Serial.print(plane); Serial.print(" : "); Serial.println(row);
  Serial.print("Planes per row: "); Serial.println(planesPerRow);
  
  if ((planesPerRow == 0) || (planesPerRow > PLANE_COUNT))
    return false;

  // Enable timer interrupts to calculate plane order
  row = 0; plane = PLANE_COUNT;
  attachInterrupt(digitalPinToInterrupt(DMD_ROW_CLK), row_clk_isr_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(DMD_DOT_LATCH), dot_latch_isr_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(DMD_OE), oe_isr_timing, FALLING);
  // Wait 1/16th of a second
  delay(64);
  // Disable timer interrupts
  detachInterrupt(digitalPinToInterrupt(DMD_ROW_CLK));
  detachInterrupt(digitalPinToInterrupt(DMD_DOT_LATCH));
  detachInterrupt(digitalPinToInterrupt(DMD_OE));
  
  calculatePlaneTimings();
  Serial.print("Format: "); Serial.println(dmdDataFormat);
 
  row = 0; plane = 0;
  return true;
}

/*
 Configure DMD input
*/
void configureInputDMD()
{  
  // Configure pins for use by SPI0
  CORE_PIN10_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS;
  /*
   Don't enable MOSI for SPI0 because we use it to replace CS on
   platforms where the shift register timings aren't valid for SPI
  */
  // CORE_PIN11_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2);
  CORE_PIN12_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_PE;
  CORE_PIN13_CONFIG = PORT_PCR_MUX(2);
  digitalWriteFast(DMD_CS, HIGH);
  
  // Enable clock to SPI0
  SIM_SCGC6 |= SIM_SCGC6_SPI0;
  SPI0_MCR = SPI_MCR_HALT | SPI_MCR_MDIS | SPI_MCR_PCSIS(1<<0);
  
  // 16 bit transfers, on rising edge
  SPI0_CTAR0_SLAVE = SPI_CTAR_FMSZ(15);

  // Enable FIFO Drain Request DMA
  SPI0_RSER = SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS;
  dmaSPI0rx = new DMAChannel();
  dmaSPI0rx->source((volatile uint16_t&) SPI0_POPR);
  dmaSPI0rx->destinationBuffer(plane_buffer, ROW_LENGTH * sizeof(uint16_t));
  dmaSPI0rx->triggerAtHardwareEvent(DMAMUX_SOURCE_SPI0_RX);
#ifndef DMA_COMPLETION_ISR
  dmaSPI0rx->disableOnCompletion();
  dmaSPI0rx->disable();
#endif
  
  // Make sure FIFO drain preempts display stuff
  DMAPriorityOrder(*dmaSPI0rx, dmaOutputAddress);
  DMAPriorityOrder(*dmaSPI0rx, dmaUpdateAddress);
  DMAPriorityOrder(*dmaSPI0rx, dmaUpdateTimer);
  DMAPriorityOrder(*dmaSPI0rx, dmaClockOutData);
  enableDMAPreemption(dmaOutputAddress);
  enableDMAPreemption(dmaUpdateAddress);
  enableDMAPreemption(dmaUpdateTimer);
  enableDMAPreemption(dmaClockOutData);
  
  // Enable interrupts for DMD inputs
  if (dmdDataFormat == FORMAT_WPC) {
#if DMA_COMPLETION_ISR
    // Enable DMA completion interrupt
    dmaSPI0rx->attachInterrupt(dma_isr_wpc);
    dmaSPI0rx->interruptAtCompletion();
    // Enable Output Enable interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_OE), portb_isr_wpc, RISING);
#else
    // Enable Row Zero interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_ROW_DATA), portb_isr_wpc, RISING);
    // Enable Output Enable interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_OE), portb_isr_wpc, RISING);
#endif
    // Replace global PORTB isr
    attachInterruptVector(IRQ_PORTB, portb_isr_wpc);
  }
  else if (dmdDataFormat == FORMAT_WHITESTAR) {
    // Enable Row Zero interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_ROW_DATA), portb_isr_whitestar, RISING);
    // Enable Dot Latch interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_DOT_LATCH), portb_isr_whitestar, RISING);
    // Replace global PORTB isr
    attachInterruptVector(IRQ_PORTB, portb_isr_whitestar);
  }
  else if (dmdDataFormat == FORMAT_SAM) {
    // Enable Row Zero interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_ROW_DATA), portb_isr_sam, RISING);
    // Enable Output Enable interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_OE), portb_isr_sam, FALLING);
    // Replace global PORTB isr
    attachInterruptVector(IRQ_PORTB, portb_isr_sam);
  }
  else if (dmdDataFormat == FORMAT_PREMIER) {
#if DMA_COMPLETION_ISR
    // Enable DMA completion interrupt
    dmaSPI0rx->attachInterrupt(dma_isr_premier);
    dmaSPI0rx->interruptAtCompletion();
    // Enable Row Zero interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_ROW_DATA), portb_isr_premier, RISING);
#else
    // Enable Row Zero interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_ROW_DATA), portb_isr_premier, RISING);
    // Enable Dot Latch interrupt
    attachInterrupt(digitalPinToInterrupt(DMD_DOT_LATCH), portb_isr_premier, RISING);
#endif
    // Replace global PORTB isr
    attachInterruptVector(IRQ_PORTB, portb_isr_premier);
  }
  // Adjust isr priorities so SPI0 input preempts PORTB input
  NVIC_SET_PRIORITY(IRQ_PORTB, 32);
  NVIC_SET_PRIORITY(IRQ_SPI0, 0);
  NVIC_DISABLE_IRQ(IRQ_PORTB);
  
  // Stall waiting for row 0 alignment
  while (!digitalReadFast(DMD_ROW_DATA));
  while (!digitalReadFast(DMD_ROW_CLK));
  do {
    SPI0_MCR |= SPI_MCR_CLR_RXF;
  } while (digitalReadFast(DMD_ROW_CLK));
  if (dmdDataFormat == FORMAT_WPC) {
    while (!digitalReadFast(DMD_OE));
  }
  rowZero = true;
 
  // Turn on the SPI
  SPI0_MCR &= ~SPI_MCR_HALT & ~SPI_MCR_MDIS;
  digitalWriteFast(DMD_CS, LOW);
  NVIC_ENABLE_IRQ(IRQ_PORTB);
  dmaSPI0rx->enable();
}

/*
 Determine settings used to drive the DMD
*/
void calculatePlaneTimings()
{
  // Determine the output enable timings for the DMD
  // These could be:
  // 1 plane per row, enable times equal < 128 fps - WPC
  // 1 plane per row, enable times equal > 200 fps - Premier
  // 2 planes per row, varying enable times - WhiteStar 
  // 4 planes per row, enable times equal - P-ROC boot
  // 4 planes per row, varying enable times - SAM, P-ROC normal
  dmdDataFormat = FORMAT_WPC;
  if (planesPerRow > 1) {
    uint8_t t;
    dmdDataFormat = FORMAT_WHITESTAR;
    if (compareIntervals(planeTimes[planeOrder[1]], planeTimes[planeOrder[0]]) > 0)
      { t = planeOrder[0]; planeOrder[0] = planeOrder[1]; planeOrder[1] = t; }
    if (planesPerRow > 2) {
      dmdDataFormat = FORMAT_SAM;
      if (compareIntervals(planeTimes[planeOrder[3]], planeTimes[planeOrder[2]]) > 0)
        { t = planeOrder[2]; planeOrder[2] = planeOrder[3]; planeOrder[3] = t; }
      if (compareIntervals(planeTimes[planeOrder[2]], planeTimes[planeOrder[1]]) > 0)
        { t = planeOrder[1]; planeOrder[1] = planeOrder[2]; planeOrder[2] = t; }
      if (compareIntervals(planeTimes[planeOrder[1]], planeTimes[planeOrder[0]]) > 0)
        { t = planeOrder[0]; planeOrder[0] = planeOrder[1]; planeOrder[1] = t; }
      if (compareIntervals(planeTimes[planeOrder[3]], planeTimes[planeOrder[2]]) > 0)
        { t = planeOrder[2]; planeOrder[2] = planeOrder[3]; planeOrder[3] = t; }
      if (compareIntervals(planeTimes[planeOrder[2]], planeTimes[planeOrder[1]]) > 0)
        { t = planeOrder[1]; planeOrder[1] = planeOrder[2]; planeOrder[2] = t; }

//      if (compareIntervals(planeTimes[planeOrder[0]], planeTimes[planeOrder[3]]) == 0) {
//        dmdDataFormat = FORMAT_PROC_BOOT;
//      }
    }
  }
  else if (planeTimes[0] < 100) {
    dmdDataFormat = FORMAT_PREMIER;
  }

  for (int i = 0; i < PLANE_COUNT; i++) {
    activePlaneTimes[i] = planeTimes[i];
  }
 
  Serial.println("Plane order: ");
  for (int i = 0; i < planesPerRow; i++) {
    Serial.print(planeOrder[i]);
    Serial.print(":");
    Serial.println(planeTimes[planeOrder[i]]);
  }

   /*
   BUG!

   Need to be able to synchronize with received screens.
   Having the matrix refresh out of sync can cause uneven
   use of front and back buffers causing screen stutter.
  */
  if (dmdDataFormat == FORMAT_WPC) {
    matrix.setRefreshRate(120);
  }
  else if (dmdDataFormat == FORMAT_WHITESTAR) {
    matrix.setRefreshRate(80);
  }
  else if (dmdDataFormat == FORMAT_SAM) {
    matrix.setRefreshRate(60);
  }
  else if (dmdDataFormat == FORMAT_PREMIER) {
    matrix.setRefreshRate(120);
  }
}

/*
 Copy received row data into back buffer, and swap if screen complete
*/
bool handleInputDMD()
{
  if (receivedRows != 0) {
    uint32_t rowFlag = 1;
    uint8_t currentRow = 0;
    while ((receivedRows & rowFlag) == 0) {
      currentRow++;
      rowFlag = 1 << currentRow;
    }
    receivedRows &= ~rowFlag;
    
    gotRow = false;
   
    switch (dmdDataFormat) {
      case FORMAT_WPC:
        updateWPC(currentRow);
        break;
        
      case FORMAT_WHITESTAR:
        updateWhitestar(currentRow);
        break;
        
      case FORMAT_SAM:
        updateSAM(currentRow);
        break;
        
       case FORMAT_PREMIER:
        updatePremier(currentRow);
        break;
   }
    
//    if ((signed long) planeTimes[currentPlane] < 0) {
//      planeTimes[currentPlane] = activePlaneTimes[currentPlane];
//    }
//    else if (compareIntervals(activePlaneTimes[currentPlane], planeTimes[currentPlane]) != EQUAL) {
//      timingChanged = true;
//      activePlaneTimes[currentPlane] = planeTimes[currentPlane];
//    }

      if (currentRow == (ROW_COUNT - 1)) {
//      if (timingChanged) {
//        calculatePlaneTimings();
//        timingChanged = false;
//      }

      backgroundLayer.swapBuffers(false);
      return true;
    }
  }

  return false;
}

void updateWPC(uint8_t currentRow)
{
  rgb24* pixelBuffer = backgroundLayer.backBuffer();
  pixelBuffer += currentRow * COL_COUNT;

  for (int j = 0; j < ROW_LENGTH; j++) {
    uint16_t dots0 = wpc_planes[0][currentRow][j];
    uint16_t dots1 = wpc_planes[1][currentRow][j];
    uint16_t dots2 = wpc_planes[2][currentRow][j];
 
    uint16_t dotMask = 0x8000;
    while (dotMask) {
      rgb24* colorPtr = activePalette;
      // WPC style equally weighted planes
      if (dots0 & dotMask) { colorPtr += 5; }
      if (dots1 & dotMask) { colorPtr += 5; }
      if (dots2 & dotMask) { colorPtr += 5; }
      
      *pixelBuffer = *colorPtr;
      pixelBuffer++;
      dotMask >>= 1;
    }
  }
}

void updateWhitestar(uint8_t currentRow)
{
  rgb24* pixelBuffer = backgroundLayer.backBuffer();
  pixelBuffer += currentRow * COL_COUNT;

  for (int j = 0; j < ROW_LENGTH; j++) {
    uint16_t dots5 = whitestar_planes[currentRow][planeOrder[0]][j];
    uint16_t dots10 = whitestar_planes[currentRow][planeOrder[1]][j];
 
    uint16_t dotMask = 0x8000;
    while (dotMask) {
      rgb24* colorPtr = activePalette;
      // Whitestar style variably weighted planes
      if (dots5 & dotMask) { colorPtr += 5; }
      if (dots10 & dotMask) { colorPtr += 10; }
      
      *pixelBuffer = *colorPtr;
      pixelBuffer++;
      dotMask >>= 1;
    }
  }
}

void updateSAM(uint8_t currentRow)
{
  rgb24* pixelBuffer = backgroundLayer.backBuffer();
  pixelBuffer += currentRow * COL_COUNT;

  for (int j = 0; j < ROW_LENGTH; j++) {
    uint16_t dots1 = sam_planes[currentRow][planeOrder[0]][j];
    uint16_t dots2 = sam_planes[currentRow][planeOrder[1]][j];
    uint16_t dots4 = sam_planes[currentRow][planeOrder[2]][j];
    uint16_t dots8 = sam_planes[currentRow][planeOrder[3]][j];
 
    uint16_t dotMask = 0x8000;
    while (dotMask) {
      rgb24* colorPtr = activePalette;
      // SAM style variably weighted planes
      if (dots1 & dotMask) { colorPtr += 1; }
      if (dots2 & dotMask) { colorPtr += 2; }
      if (dots4 & dotMask) { colorPtr += 4; }
      if (dots8 & dotMask) { colorPtr += 8; }
      
      *pixelBuffer = *colorPtr;
      pixelBuffer++;
      dotMask >>= 1;
    }
  }
}

void updatePremier(uint8_t currentRow)
{
  rgb24* pixelBuffer = backgroundLayer.backBuffer();
  pixelBuffer += currentRow * COL_COUNT;

  for (int j = 0; j < ROW_LENGTH; j++) {
    uint16_t dots0 = wpc_planes[0][currentRow][j];
    uint16_t dots1 = wpc_planes[1][currentRow][j];
    uint16_t dots2 = wpc_planes[2][currentRow][j];
    uint16_t dots3 = wpc_planes[3][currentRow][j];
 
    uint16_t dotMask = 0x8000;
    while (dotMask) {
      uint8_t colorEntry = 0;
      // Premier style equally weighted planes
      if (dots0 & dotMask) { colorEntry += 4; }
      if (dots1 & dotMask) { colorEntry += 4; }
      if (dots2 & dotMask) { colorEntry += 4; }
      if (dots3 & dotMask) { colorEntry += 4; }
      if (colorEntry >= PALETTE_ENTRIES) { colorEntry = (PALETTE_ENTRIES - 1); }
      
      *pixelBuffer = activePalette[colorEntry];
      pixelBuffer++;
      dotMask >>= 1;
    }
  }
}

/*
 Minimal interrupts for testing configuration
*/
void plane_count_isr(void) {
  plane++;
}

void row_count_isr(void) {
  row++;
}


#ifdef DMA_COMPLETION_ISR
/*
 Combined interrupt for all Port B inputs, Williams WPC version
*/
void portb_isr_wpc(void)
{  
  uint32_t isfr = PORTB_ISFR;
  PORTB_ISFR = isfr;

  // Check for output enable
  if (isfr & DMD_OE_BITMASK) {
    rowZero = digitalReadFast(DMD_ROW_DATA);
  }
}

/*
 DMA completion interrupt, Williams WPC version
*/
void dma_isr_wpc(void)
{
  dmaSPI0rx->clearInterrupt();
  
  if (rowZero) {
    rowZero = false;
    row = 0;
    plane++;
    if (plane >= PLANE_BLEND_WPC) {
      plane = 0;
    }
  }
  else {
    row++;
    if (row >= ROW_COUNT) {
      row = 0;
    }
  }
  
  memcpy(wpc_planes[plane][row], plane_buffer, ROW_LENGTH * sizeof(uint16_t));
  receivedRows |= (1 << row);
  
  gotRow = true;    
}

#else

/*
 Combined interrupt for all Port B inputs, Williams WPC version
*/
void portb_isr_wpc(void)
{  
  uint32_t isfr = PORTB_ISFR;
  PORTB_ISFR = isfr;

  // Check for row zero marker
  if (isfr & DMD_ROW_DATA_BITMASK) {
    rowZero = true;
  } 
  
  // Check for output enable
  if (isfr & DMD_OE_BITMASK) {
    // Use fake chip select to end plane
    digitalWriteFast(DMD_CS, HIGH);
    
    memcpy(wpc_planes[plane][row], plane_buffer, ROW_LENGTH * sizeof(uint16_t));
    receivedRows |= (1 << row);
    
    if (rowZero) {
      rowZero = false;
      row = 0;
      plane++;
      if (plane >= PLANE_BLEND_WPC) {
        plane = 0;
      }
    }
    else {
      row++;
      if (row >= ROW_COUNT) {
        row = 0;
      }
    }
    
    // Flush the receive buffer, in case we're out of sync
    SPI0_MCR |= SPI_MCR_HALT;
    SPI0_MCR |= SPI_MCR_CLR_RXF;

    // Reset the DMA transfer
    dmaSPI0rx->disable();
    dmaSPI0rx->clearComplete();
    dmaSPI0rx->destinationBuffer(plane_buffer, ROW_LENGTH * sizeof(uint16_t));
    dmaSPI0rx->enable();
 
    gotRow = true;

    // Start new SPI plane
    SPI0_MCR &= ~SPI_MCR_HALT;
    digitalWriteFast(DMD_CS, LOW);
   
    // Start the plane timer
    // planeStart = micros();
  }
}
#endif

/*
 Combined interrupt for all Port B inputs, Stern Whitestar version
*/
void portb_isr_whitestar(void)
{  
  uint32_t isfr = PORTB_ISFR;
  PORTB_ISFR = isfr;

  // Check for row zero marker
  if (isfr & DMD_ROW_DATA_BITMASK) {
    rowZero = true;
  } 
  
  // Check for latch
  if (isfr & DMD_DOT_LATCH_BITMASK) {
    // Use fake chip select to end plane
    if (digitalReadFast(DMD_DOT_LATCH)) { 
      digitalWriteFast(DMD_CS, HIGH);
      while (digitalReadFast(DMD_DOT_LATCH));
    }
    
    memcpy(whitestar_planes[row][plane], plane_buffer, ROW_LENGTH * sizeof(uint16_t));

    // Start new SPI plane
    digitalWriteFast(DMD_CS, LOW);

    if (rowZero) {
      rowZero = false;
      row = 0;
      plane = 0;
    }
    else {
      plane++;
      if (plane >= PLANE_COUNT_WHITESTAR) {
        receivedRows |= (1 << row);
        gotRow = true;
        plane = 0;
        row++;
        if (row >= ROW_COUNT) {
          row = 0;
        }
      }
    }
    
    // Reset the DMA transfer
    dmaSPI0rx->disable();
    dmaSPI0rx->clearComplete();
    dmaSPI0rx->destinationBuffer(plane_buffer, ROW_LENGTH * sizeof(uint16_t));
    dmaSPI0rx->enable();
 
    // Start the plane timer
    // planeStart = micros();
  }
}

/*
 Combined interrupt for all Port B inputs, Stern SAM version
*/
void portb_isr_sam(void)
{
  uint32_t isfr = PORTB_ISFR;
  PORTB_ISFR = isfr;

  // Check for row zero marker
  if (isfr & DMD_ROW_DATA_BITMASK) {
    rowZero = true;
  } 
  
  // Check for latch
  if (isfr & DMD_OE_BITMASK) {
    // Use fake chip select to end plane
    digitalWriteFast(DMD_CS, HIGH);
    
    memcpy(sam_planes[row][plane], plane_buffer, ROW_LENGTH * sizeof(uint16_t));

    // Start new SPI plane
    digitalWriteFast(DMD_CS, LOW);

    // This is sort of a hack because we're getting the OE edge before
    // the row 0 flag, so we begin on plane 1 rather than 0.
    if (rowZero) {
      rowZero = false;
      row = 0;
      plane = 1;
    }
    else {
      plane++;
      if (plane >= PLANE_COUNT) {
        receivedRows |= (1 << row);
        gotRow = true;
        plane = 0;
        row++;
        if (row >= ROW_COUNT) {
         row = 0;
        }
      }
    }
    
    // Reset the DMA transfer
    dmaSPI0rx->disable();
    dmaSPI0rx->clearComplete();
    dmaSPI0rx->destinationBuffer(plane_buffer, ROW_LENGTH * sizeof(uint16_t));
    dmaSPI0rx->enable();

    // Start the plane timer
    // planeStart = micros();
  }
}

#ifdef DMA_COMPLETION_ISR
/*
 Combined interrupt for all Port B inputs, Gottlieb/Premier version
*/
void portb_isr_premier(void)
{  
  uint32_t isfr = PORTB_ISFR;
  PORTB_ISFR = isfr;

  // Check for output enable
  if (isfr & DMD_ROW_DATA_BITMASK) {
    if (!digitalReadFast(DMD_ROW_DATA)) return;
    rowZero = true;
  }
}

/*
 DMA completion interrupt, Gottlieb/Premier version
*/
void dma_isr_premier(void)
{
  dmaSPI0rx->clearInterrupt();
  
  if (rowZero) {
    rowZero = false;
    row = 0;
    plane++;
    if (plane >= PLANE_COUNT) {
      plane = 0;
    }
  }

  if (row < ROW_COUNT) {
    memcpy(wpc_planes[plane][row], plane_buffer, ROW_LENGTH * sizeof(uint16_t));
    if (plane == (PLANE_COUNT - 1)) {
      receivedRows |= (1 << row);
      gotRow = true;
    }
    row++;
  }
}

#else
/*
 Combined interrupt for all Port B inputs, Gottlieb/Premier version
*/
void portb_isr_premier(void)
{  
  uint32_t isfr = PORTB_ISFR;
  PORTB_ISFR = isfr;

  // Check for row zero marker
  if (isfr & DMD_ROW_DATA_BITMASK) {
    rowZero = true;
  } 
  
  // Check for output enable
  if (isfr & DMD_DOT_LATCH_BITMASK) {   
    if (rowZero) {
      rowZero = false;
      row = 0;
      plane++;
      if (plane >= PLANE_COUNT) {
        plane = 0;
      }
    }
    
    // Use fake chip select to end plane
    digitalWriteFast(DMD_CS, HIGH);

    if (row < ROW_COUNT) {
      memcpy(wpc_planes[plane][row], plane_buffer, ROW_LENGTH * sizeof(uint16_t));
      if (plane == (PLANE_COUNT - 1)) {
        receivedRows |= (1 << row);
      }
    }
    row++;

    // Start new SPI plane
    digitalWriteFast(DMD_CS, LOW);
   
    // Reset the DMA transfer
    dmaSPI0rx->disable();
    dmaSPI0rx->clearComplete();
    dmaSPI0rx->destinationBuffer(plane_buffer, ROW_LENGTH * sizeof(uint16_t));
    dmaSPI0rx->enable();
   
    // Start the plane timer
//    planeStart = micros();
  }
}
#endif

/*
 Interrupts for timing output enable
*/
void row_clk_isr_timing(void)
{
  plane = 0;
}

void dot_latch_isr_timing(void)
{
  planeStart = micros();
}

void oe_isr_timing(void)
{
  if ((plane >= PLANE_COUNT) || (planeStart == 0)) return;
  
  unsigned long planeTime = micros() - planeStart;
  if (planeTime > planeTimes[plane]) {
    planeTimes[plane] = planeTime;
  }
  plane++;
}

/* 
 Set DMAChannel to allow preemption
*/
void enableDMAPreemption(DMAChannel &dma)
{
  switch (dma.channel) {
    case 0:
      DMA_DCHPRI0 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 1:
      DMA_DCHPRI1 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 2:
      DMA_DCHPRI2 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 3:
      DMA_DCHPRI3 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
#if DMA_NUM_CHANNELS >= 16
    case 4:
      DMA_DCHPRI4 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 5:
      DMA_DCHPRI5 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 6:
      DMA_DCHPRI6 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 7:
      DMA_DCHPRI7 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 8:
      DMA_DCHPRI8 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 9:
      DMA_DCHPRI9 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 10:
      DMA_DCHPRI10 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 11:
      DMA_DCHPRI11 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 12:
      DMA_DCHPRI12 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 13:
      DMA_DCHPRI13 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 14:
      DMA_DCHPRI14 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
    case 15:
      DMA_DCHPRI15 |= DMA_DCHPRI_ECP | DMA_DCHPRI_DPA; break;
#endif
  }
}
