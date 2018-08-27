// -----------------------------------------------------------------------------      
// Settings for the Arduino Pro (Mini)
//
// -----------------------------------------------------------------------------      
#define   USES_FAST_ADC
//#define USES_PLOTTING
//#define USES_FULL_REDRAW
//#define USES_HOUSEKEEPING
//#define USES_DAC

// -----------------------------------------------------------------------------      
// Pin definitions (simulation-related)
// -----------------------------------------------------------------------------      
#define PhotoDiodePin A0 // Photodiode
#define LEDOutPin 9      // LED
#define ButtonPin 2      // Push button to switch spike modes
#define VmPotPin A3      // Resting membrane potential
#define Syn1PotPin A7    // efficacy synapse 1
#define Syn2PotPin A5    // efficacy synapse 2
#define NoisePotPin A6   // scaling of Noise level 
#define DigitalIn1Pin 4  // Synapse 1 Input - expects 5V pulses
#define DigitalIn2Pin 5  // Synapse 2 input - expects 5V pulses
#define AnalogInPin A2   // Analog in- takes 0-5V (positive only)
#define DigitalOutPin 3  // "Axon" - generates 5V pulses
#define AnalogOutPin 11  // Analog out for full spike waveform

// Digital and analog I/O helper macros
//
#define pinModeHelper(pin, mode) pinMode(pin, mode)
#define digitalReadHelper(pin) digitalRead(pin) 
#define digitalWriteHelper(pin, val) digitalWrite(pin, val)
#define analogReadHelper(pin) analogRead(pin)
#define analogWriteHelper(pin, val) analogWrite(pin, val)

// Serial out
//
#define SerOutBAUD 230400

// -----------------------------------------------------------------------------      
// Pin definitions (hardware add-ons)
// -----------------------------------------------------------------------------      

// -----------------------------------------------------------------------------      
// Other hardware-related definitions
// -----------------------------------------------------------------------------      
// Setting and clearing register bits
//
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// -----------------------------------------------------------------------------      
// Helpers
// -----------------------------------------------------------------------------      
void initializeHardware() 
{
  pinMode(LED_BUILTIN, OUTPUT); // 13 digital
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

  // Change ADC prescaler 16, which slighly decreases the ADC precision but 
  // speeds up the time per loop by ~20% 
  //
  #ifdef USES_FAST_ADC
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);
  #endif
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
void plot(output_t Output)
{
}

// -----------------------------------------------------------------------------

