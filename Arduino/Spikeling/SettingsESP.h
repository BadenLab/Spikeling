//#define USE_FAST_ADC
#define USES_PLOTTING
#define NEEDS_HOUSEKEEPING

// -----------------------------------------------------------------------------      
// Pin definitions (simulation-related)
// -----------------------------------------------------------------------------      
#define PhotoDiodePin 0  // Photodiode
#define LEDOutPin 1      // LED
#define ButtonPin 2      // Push button to switch spike modes
#define VmPotPin 3       // Resting membrane potential
#define Syn1PotPin 4     // efficacy synapse 1
#define Syn2PotPin 5     // efficacy synapse 2
#define NoisePotPin 6    // scaling of Noise level 
#define DigitalIn1Pin 7  // Synapse 1 Input - expects 5V pulses
#define DigitalIn2Pin 8  // Synapse 2 input - expects 5V pulses
#define AnalogInPin 9    // Analog in- takes 0-5V (positive only)
#define DigitalOutPin 10 // "Axon" - generates 5V pulses
#define AnalogOutPin 11  // Analog out for full spike waveform

// Digital and analog I/O helper macros
//
#define pinModeHelper(pin, mode) pinModeNew(pin, mode)
#define digitalReadHelper(pin) digitalReadNew(pin) 
#define digitalWriteHelper(pin, val) digitalWriteNew(pin, val)
#define analogReadHelper(pin) analogReadNew(pin)
#define analogWriteHelper(pin, val) analogWriteNew(pin, val)

// Serial out 
//
#define SerOutBAUD 921600
//#define SerOutBAUD 230400

// -----------------------------------------------------------------------------      
// Pin definitions (hardware add-ons)
// -----------------------------------------------------------------------------      
#include <SPI.h>
#include "MiniGrafx.h" // General graphic library
#include "ILI9341_SPI.h" // Hardware-specific library

#define TFT_DC D2
#define TFT_CS D1
#define TFT_LED D8

// Define colors usable in the paletted 16 color frame buffer
//
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_NAVY, // 2
                      ILI9341_DARKCYAN, // 3
                      ILI9341_DARKGREEN, // 4
                      ILI9341_MAROON, // 5
                      ILI9341_PURPLE, // 6
                      ILI9341_OLIVE, // 7
                      ILI9341_LIGHTGREY, // 8
                      ILI9341_DARKGREY, // 9
                      ILI9341_BLUE, // 10
                      ILI9341_GREEN, // 11
                      ILI9341_CYAN, // 12
                      ILI9341_RED, // 13
                      ILI9341_MAGENTA, // 14
                      ILI9341_YELLOW}; // 15

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;
const int BITS_PER_PIXEL = 4; // 2^4 = 16 colors
const int FONT_HEIGHT = 14; // Standard font

// Initialize the driver
//
ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);

// Definitions and variables for plotting
//
#define INFO_DY 18 // Height of info panel
#define MAX_TRACES 3 // Maximal number of traces shown in parallel
#define MAX_VALUES 320 // Maximal trace length
#define PLOT_UPDATE 16 // Redraw screen every # values

#define V_MIN -100
#define V_MAX 40

const char* OutputStr[]= {"V_mem [V] ", "I_tot [pA]", "I_PD [pA] ", "I_AIn [pA]", 
                          "I_Syn [pA]", "Stim_State", "SpIn1State", "SpIn2State", 
                          "time [us] "};

int TraceCols[MAX_TRACES] = {13,11,15};
int Traces[MAX_TRACES][MAX_VALUES];
int TracesStrIndex[MAX_TRACES];
int iPnt, dyPlot, dxInfo;
char timeStr[16];

// -----------------------------------------------------------------------------      
// Other hardware-related definitions
// -----------------------------------------------------------------------------      

// -----------------------------------------------------------------------------      
// Helpers
// -----------------------------------------------------------------------------      
void initializeHardware() 
{
/*pinMode(LED_BUILTIN, OUTPUT); // 13 digital
  pinMode(AnalogOutPin, OUTPUT); // 11 "digital" PWM
  pinMode(DigitalOutPin, OUTPUT); // 3 digital
  pinMode(LEDOutPin, OUTPUT); // 9 digital PWM
  pinMode(DigitalIn1Pin, INPUT); // 4 digital
  pinMode(DigitalIn2Pin, INPUT); // 5 digital
  pinMode(ButtonPin, INPUT); // 2 digital
  pinMode(PhotoDiodePin, INPUT); // 0 analog
  pinMode(AnalogInPin, INPUT); // 5 analog // same as Synapse 2
  pinMode(VmPotPin,INPUT); // 3 analog
  pinMode(Syn1PotPin,INPUT); // 7 analog
  pinMode(Syn2PotPin,INPUT); // 5 analog // this one also controls the Analog In gain!
  pinMode(NoisePotPin,INPUT); // 6 analog
  
  TCCR2B = TCCR2B & 0b11111000 | 0x01; // sets PWM pins 3 and 11 (timer 2) to 31250 Hz    
*/  
  // Turn on the background LED of TFT
  //
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  // Initialise a few variables
  //
  iPnt = 0;
  dyPlot = SCREEN_HEIGHT -INFO_DY;
  dxInfo = SCREEN_WIDTH /(MAX_TRACES +1);
  timeStr[0] = 0;
  
  // Set rotation and clear screen
  //
  gfx.init();
  gfx.setRotation(3); // landscape, USB port up
  gfx.setFastRefresh(true);
  gfx.fillBuffer(0);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.commit();
}

// -----------------------------------------------------------------------------      
void pinModeNew(int pin, int mode) 
{
}

void digitalWriteNew(int pin, int val) 
{
} 

void analogWriteNew(int pin, int val) 
{
}

int digitalReadNew(int pin) 
{
  return LOW;
}

int analogReadNew(int pin) 
{
  switch(pin) {
    case VmPotPin:
      return 512;
    case Syn1PotPin:
      return 300;
    case Syn2PotPin:
      return 128;
    case NoisePotPin:
      return 100;
    case AnalogInPin:
      return 200;
    case PhotoDiodePin:
      return 0;
  }
  Serial.println("ERROR: analogIn pin not defined");
}  

// -----------------------------------------------------------------------------      
// Housekeeping routine, to be called once per loop
// -----------------------------------------------------------------------------      
void housekeeping() 
{
}

// -----------------------------------------------------------------------------
// Graphics
// -----------------------------------------------------------------------------   
int getYCoord(float vMin, float vMax, float v) 
{
  return SCREEN_HEIGHT -1 -map(round(v), vMin, vMax, 0, dyPlot);
}


void plot(output_t* Output)
{
  int y, iTr;

  // Add data to traces and save index of output variable string
  //
  Traces[0][iPnt] = getYCoord(V_MIN, V_MAX, Output->v);
  TracesStrIndex[0] = ID_V;
  Traces[1][iPnt] = getYCoord(V_MIN*2, V_MAX*2, Output->I_total);  
  TracesStrIndex[1] = ID_I_TOTAL;
  Traces[2][iPnt] = getYCoord(V_MIN*2, V_MAX*2, Output->Stim_State);    
  TracesStrIndex[2] = ID_I_STIM_STATE;

  // Draw new piece of each trace
  //
  if(iPnt > 0) {
    for(iTr=0; iTr<MAX_TRACES; iTr++) {
      gfx.setColor(TraceCols[iTr]);
      gfx.drawLine(iPnt-1, Traces[iTr][iPnt-1], iPnt, Traces[iTr][iPnt]);
    }
  }

  iPnt++;
  if(iPnt >= MAX_VALUES) {
    // Set trace data pointer to the beginning of the array and clear screen
    //
    iPnt = 0;
    gfx.fillBuffer(0);

    // Redraw info area
    //
    for(iTr=0; iTr<MAX_TRACES; iTr++) {
      gfx.setColor(TraceCols[iTr]);
      gfx.drawString(dxInfo/2 +(iTr+1)*dxInfo, dyPlot, OutputStr[TracesStrIndex[iTr]]);
    }
  }
  if((((iPnt-1) % PLOT_UPDATE) == 0) || (iPnt == 0)) {
    // Redraw time (first old string in black, then new string in white)
    //
    gfx.setColor(0);
    gfx.drawString(dxInfo/2, dyPlot, timeStr);
    sprintf(timeStr, "%3.1f [s]\n", Output->currentMicros /1E6);
    gfx.setColor(1);
    gfx.drawString(dxInfo/2, dyPlot, timeStr);
    
    // Commit graph commands
    //
    gfx.commit();
  }
}

// -----------------------------------------------------------------------------




