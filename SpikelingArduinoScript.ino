////////////////////////////////////////////////////////////////////////////
// Spikeling v1.0. By T Baden, Sussex Neuroscience, UK (www.badenlab.org) //
// 2017                                                             //
// Izhikevich model taken from original paper (2003 IEEE)                 //
////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// KEY PARAMETERS TO SET BY USER  /////////////////////////////////////////     
///////////////////////////////////////////////////////////////////////////

int   nosound         = 0;    // 0 default, click on spike + digi out port active. 1 switches both off
int   noled           = 0;    // 0 default, 1 switches the LED off
int   AnalogInActive  = 1;  // PORT 3 setting: Is Analog In port in use? Note that this shares the dial with the Syn2 (PORT 2) dial

float PD_Scaling      = 0.5;  // the lower the more sensitive.                       Default = 0.5
int   SynapseScaling  = 50;   // The lower, the stronger the synapse.                Default = 50
int   VmPotiScaling   = 2;    // the lower, the stronger the impact of the Vm poti.  Default = 2
int   AnalogInScaling = 2500; // the lower, the stronger the impact of Analog Input. Default = 2500
int   NoiseScaling    = 10;   // the lower, the higher the default noise level.      Default = 10

float Synapse_decay = 0.995; // speed of synaptic decay.The difference to 1 matters - the smaller the difference, the slower the decay. Default  = 0.995
float PD_gain_min   = 0.0;   // the photodiode gain cannot decay below this value
float timestep_ms   = 0.1;  // default 0.1, so the model runs at 10 kHz at default (0.1 ms steps). Changing this will speed up / slow down everything. This can be set a lot faster if desired, but individual functions will start to fail so test carefully

// set up Neuron behaviour array parameters
int   nModes = 5; // set this to number of entries in each array. Entries 1 define Mode 1, etc..

  // Izhikevich model parameters - for some pre-tested behaviours from the original paper, see bottom of the script
float Array_a[]           =  { 0.02,  0.02, 0.02, 0.02, 0.02 }; // time scale of recovery variable u. Smaller a gives slower recovery
float Array_b[]           =  { 0.20,  0.20, 0.25, 0.20, -0.1 }; // recovery variable associated with u. greater b coules it more strongly (basically sensitivity)
int   Array_c[]           =  {  -65,   -50,  -55,  -55,  -55 }; // after spike reset value
float Array_d[]           =  {  6.0,   2.0, 0.05,  4.0,  6.0 }; // after spike reset of recovery variable

float Array_PD_decay[]    =  { 0.00005,  0.001, 0.00005,  0.001, 0.00005  }; // slow/fast adapting Photodiode - small numbers make diode slow to decay
float Array_PD_recovery[] =  {   0.001,   0.01,   0.001,   0.01,   0.001  }; // slow/fast adapting Photodiode - small numbers make diode recover slowly
int   Array_DigiOutMode[] =  {       1,      1,       1,      1,       1  }; // PORT 1 setting. 0: Synapse 1 In, 1: Stimulus out, 2: 50 Hz binary noise out (for reverse correlation)
int   Array_PD_polarity[] =  {       1,     -1,      -1,      1,       1  }; // 1 or -1, flips photodiode polarity, i.e. 1: ON cell, 2: OFF cell

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN PROGRAMME - ONLY CHANGE IF YOU KNOW WHAT YOU ARE DOING !                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// set up pins
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

////////////////////////////////////////////////////////////////////////////
// Setup variables required to drive the model
float I_total;           // Total input current to the model
float I_PD;              // Photodiode current
float I_Vm;              // Vm setting current
float I_Synapse;         // Total synaptic current of both synapses
float I_AnalogIn;        // Current from analog Input
float I_Noise;           // Noise current
float Synapse1Ampl;      // Synapse 1 efficacy
float Synapse2Ampl;      // Synapse 2 efficacy
float AnalogInAmpl;      // Analog In efficacy
float NoiseAmpl;         // Added Noise level

//Set read intervals for different inputs
long PDinterval            = 100;    // how often do we read the photodiode? (in us)
long PDpreviousMicros      = 0;  
long SpikeIninterval       = 1000; // how often do we read the Digi Ins? (in us)
long SpikeInpreviousMicros = 0; 
long Outputinterval        = 1000; // how often do update the output LEDs/serial (in us). Note this also defines Digi Out "spike" duration
long OutputpreviousMicros  = 0;   
long ModelpreviousMicros   = 0;
long DigiOutInterval       = 1000;    // how often do consider the DigiOut port in stimulation mode (in us) - this will affect the "noise" frequency as well as Step frequency
long DigiOutpreviousMicros = 0;  

// initialise state variables for different inputs
boolean spike = false;
int     buttonState    = 0;
int     SpikeIn1State  = 0;
int     SpikeIn2State  = 0;
int     VmPotVal       = 0;
float   Syn1PotVal     = 0.0;
float   Syn2PotVal     = 0.0;
float   NoisePotVal    = 0.0;
float   AnalogInPotVal = 0.0;
float   AnalogInVal    = 0.0;
int     PDVal          = 0;
int     Stimulator_Val = 0;

// for PD smoothing // this is a dirty hack to smooth the photodiode current which in raw form generates ugly noise spikes
int PDVal_Array[]         =  {0,0,0,0,0,0,0,0,0,0}; 
int PD_integration_counter= 0;
float PDVal_smoothed      = 0;

float PD_gain = 1.0;
int NeuronBehaviour = 0; // 0:8 for different modes, cycled by button
int DigiOutStep = 0;     // stimestep counter for stimulator mode
int Stim_State = 0;      // State of the internal stimulator
float v; // voltage in Iziekevich model
float u; // recovery variable in Iziekevich model

int startMicros = micros();  

////////////////////////////////////////////////////////////////////////////
// SETUP (this only runs once at when the Arduino is initialised) //////////
////////////////////////////////////////////////////////////////////////////
 
void setup(void) {
  Serial.begin(230400); // 
  // Set all the PINs
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

  if (Array_DigiOutMode[NeuronBehaviour]==0) {
      pinMode(DigitalIn1Pin, INPUT); // SET SYNAPSE 1 IN AS INTENDED
    } else {
      pinMode(DigitalIn1Pin, OUTPUT); // SET SYNAPSE 1 IN AS STIMULATOR OUT CHANNEL
    }
  
  TCCR2B = TCCR2B & 0b11111000 | 0x01; // sets PWM pins 3 and 11 (timer 2) to 31250 Hz
}

////////////////////////////////////////////////////////////////////////////
// MAIN ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
 
void loop(void) {

  // check system time in microseconds
  unsigned long currentMicros = micros() - startMicros;  
  
  // read button to change spike model
  buttonState = digitalRead(ButtonPin);
  if (buttonState == HIGH) {
    NeuronBehaviour+=1;
    if (NeuronBehaviour>=nModes) {NeuronBehaviour=0;}
    Serial.print("Neuron Mode:");
    Serial.println(NeuronBehaviour);
    for (int modeblink = 0; modeblink < NeuronBehaviour +1; modeblink++) {
      digitalWrite(LED_BUILTIN, HIGH); // Blinks the onboard LED according to which programme is selected
      delay(150);    
      digitalWrite(LED_BUILTIN, LOW);
      delay(150);
    }
    if (Array_DigiOutMode[NeuronBehaviour]==0) {
      pinMode(DigitalIn1Pin, INPUT); // SET SYNAPSE 1 IN AS INTENDED
    } else {
      pinMode(DigitalIn1Pin, OUTPUT); // SET SYNAPSE 1 IN AS STIMULATOR OUT CHANNEL
    }
  }

  // read Vm potentiometer to calculate I_Vm
  VmPotVal = analogRead(VmPotPin); // 0:1023, Vm
  I_Vm = -1 * (VmPotVal-512) / VmPotiScaling;
  
  // read analog In potentiometer to set Analog in and Noise scaling
  AnalogInPotVal = analogRead(Syn2PotPin); // 0:1023, Vm - reads same pot as Synapse 2!
  AnalogInAmpl = (AnalogInPotVal - 512) / AnalogInScaling;

  NoisePotVal = analogRead(NoisePotPin); // 0:1023, Vm 
  NoiseAmpl = -1 * ((NoisePotVal-512) / NoiseScaling);
  if (NoiseAmpl<0) {NoiseAmpl = 0;}

  // read analog in to calculate I_AnalogIn and I_Noise
  AnalogInVal = analogRead(AnalogInPin); // 0:1023
  I_AnalogIn = -1 * (AnalogInVal) * AnalogInAmpl;
  if (AnalogInActive == 0) {I_AnalogIn = 0;}
  
  I_Noise+=random(-NoiseAmpl/2,NoiseAmpl/2);
  I_Noise*=0.9;
  

  // read Photodiode every PDinterval microseconds
  if(currentMicros - PDpreviousMicros > PDinterval) {
    PDpreviousMicros = currentMicros;   
  
    int PDVal = analogRead(PhotoDiodePin); // 0:1023

    // PD integration over 5 points
    if (PD_integration_counter<10) {
      PD_integration_counter+=1;
    } else {
      PD_integration_counter=0;
    }
    PDVal_Array[PD_integration_counter]=PDVal;
    PDVal_smoothed=(PDVal_Array[0]+PDVal_Array[1]+PDVal_Array[2]+PDVal_Array[3]+PDVal_Array[4]+PDVal_Array[5]+PDVal_Array[6]+PDVal_Array[7]+PDVal_Array[8]+PDVal_Array[9])/10; // dirty hack to smooth PD current - could be a lot more elegant

    I_PD = ((PDVal_smoothed) / PD_Scaling) * PD_gain; // input current
    
    if (PD_gain>PD_gain_min){
      PD_gain-=Array_PD_decay[NeuronBehaviour]*I_PD; // adapts proportional to I_PD
       if (PD_gain<PD_gain_min){
        PD_gain=PD_gain_min;
      }
    }
    if (PD_gain<1.0) {
      PD_gain+=Array_PD_recovery[NeuronBehaviour]; // recovers by constant % per iteration
    }
  }
  
  // Read the two synapses to calculate Synapse Ampl parameters
  Syn1PotVal = analogRead(Syn1PotPin); // 0:1023, Vm
  Synapse1Ampl = -1* (Syn1PotVal-512) / SynapseScaling; // 
  Syn2PotVal = analogRead(Syn2PotPin); // 0:1023, Vm
  Synapse2Ampl = -1 * (Syn2PotVal-512) / SynapseScaling; // 

  // read Synapse digital inputs every SpikeIninterval microseconds
  if(currentMicros - SpikeInpreviousMicros > SpikeIninterval) {
    SpikeInpreviousMicros = currentMicros; 
    SpikeIn1State = digitalRead(DigitalIn1Pin);
    if (Array_DigiOutMode[NeuronBehaviour]>0){ 
      SpikeIn1State = LOW;
    }
    SpikeIn2State = digitalRead(DigitalIn2Pin);
    if (SpikeIn1State == HIGH) {I_Synapse+=Synapse1Ampl;}
    if (SpikeIn2State == HIGH) {I_Synapse+=Synapse2Ampl;}
  }
  // Decay all synaptic current towards zero
  I_Synapse*=Synapse_decay; 
  
  // calculate Izhikevich model
  float I_total = I_PD*Array_PD_polarity[NeuronBehaviour] + I_Vm + I_Synapse + I_AnalogIn + I_Noise; // Add up all current sources
  
  if(currentMicros - ModelpreviousMicros > timestep_ms*1000) {
    ModelpreviousMicros = currentMicros;     
    v = v + timestep_ms*(0.04 * v * v + 5*v + 140 - u + I_total);
    u = u + timestep_ms*(Array_a[NeuronBehaviour] * ( Array_b[NeuronBehaviour]*v - u));
    if (v>=30.0){v=Array_c[NeuronBehaviour]; u+=Array_d[NeuronBehaviour];}
    if (v<=-90) {v=-90.0;} // prevent from analog out (below) going into overdrive - but also means that it will flatline at -90. Change the "90" in this line and the one below if want to
    int AnalogOutValue = (v+90) * 2;
    analogWrite(AnalogOutPin,AnalogOutValue);
    if (noled==0) {
      analogWrite(LEDOutPin,AnalogOutValue);
    }
  }
  if  (v>-30.0) {spike=true;}   // check if there has been a spike for digi out routine (below)

  // trigger audio click and Digi out 5V pulse if there has been a spike
  if(currentMicros - OutputpreviousMicros > Outputinterval) {
    OutputpreviousMicros = currentMicros;   
    if (spike==true) { 
      if (nosound==0) {
        digitalWrite(DigitalOutPin, HIGH);
      }    
      spike=false;
    }
    else {
      digitalWrite(DigitalOutPin, LOW);
    }
  }
  
  // Set DigiOut level if Array_DigiOutMode[NeuronBehaviour] is not 0
  if (Array_DigiOutMode[NeuronBehaviour]==1){ // if in Step Mode
    if(currentMicros - DigiOutpreviousMicros > DigiOutInterval) {
      DigiOutpreviousMicros = currentMicros; 
      if (DigiOutStep<Stimulator_Val){
         digitalWrite(DigitalIn1Pin, HIGH); // use synapse 1 pin as stimulator
         Stim_State = 1;
      } else {
         digitalWrite(DigitalIn1Pin, LOW); 
         Stim_State = 0;
      }  
      DigiOutStep+=1;
      if (DigiOutStep>=Stimulator_Val*2) {
        DigiOutStep = 0;
        Stimulator_Val = round((Syn1PotVal/1023) * 100); // also calculate 0-100 range variable in case Synapse 1 input is used as stimulator instead
        
        Stimulator_Val = Stimulator_Val * 10;
        if (Stimulator_Val<10) {Stimulator_Val=10;}
      
      
      } // the *2 sets duty cycle to 50 %. higher multipliers reduce duty cycle
    }
  }
  if (Array_DigiOutMode[NeuronBehaviour]==2){ // if in Noise Mode
    if(currentMicros - DigiOutpreviousMicros > DigiOutInterval) {
      DigiOutpreviousMicros = currentMicros; 
      int randNumber = random(100); 
      if (randNumber<50) {digitalWrite(DigitalIn1Pin, LOW); Stim_State = 0;} 
      if (randNumber>=50) {digitalWrite(DigitalIn1Pin, HIGH); Stim_State = 1;}       
    }
  }

  // Serial output in order

  // Oscilloscope 1
  Serial.print(v);              // Ch1: voltage
  Serial.print(", ");
  Serial.print(I_total);        // Ch2: Total input current
  Serial.print(", ");
  Serial.print(Stim_State);     // Ch3: Internal Stimulus State (if Synapse 1 mode >0)
  Serial.print(", ");

  // Oscilloscope 2
  Serial.print(SpikeIn1State);  // Ch4: State of Synapse 1 (High/Low)
  Serial.print(", ");
  Serial.print(SpikeIn2State);  // Ch5: State of Synapse 2 (High/Low)
  Serial.print(", ");
  Serial.print(I_PD);           // Ch6: Total Photodiode current
  Serial.print(", ");

  // Oscilloscope 3
  Serial.print(I_AnalogIn);     // Ch7: Total Analog In current
  Serial.print(", ");
  Serial.print(I_Synapse);      // Ch8: Total Synaptic Current
  Serial.print(", ");
  Serial.print(currentMicros);   // Ch9: System Time in us
  Serial.println("\r");

}

////////////////////////////////////////////////////////////////////////////////////////////////

// From Iziekevich.org - see also https://www.izhikevich.org/publications/figure1.pdf:
//      0.02      0.2     -65      6       14 ;...    % tonic spiking
//      0.02      0.25    -65      6       0.5 ;...   % phasic spiking
//      0.02      0.2     -50      2       15 ;...    % tonic bursting
//      0.02      0.25    -55     0.05     0.6 ;...   % phasic bursting
//      0.02      0.2     -55     4        10 ;...    % mixed mode
//      0.01      0.2     -65     8        30 ;...    % spike frequency adaptation
//      0.02      -0.1    -55     6        0  ;...    % Class 1
//      0.2       0.26    -65     0        0  ;...    % Class 2
//      0.02      0.2     -65     6        7  ;...    % spike latency
//      0.05      0.26    -60     0        0  ;...    % subthreshold oscillations
//      0.1       0.26    -60     -1       0  ;...    % resonator
//      0.02      -0.1    -55     6        0  ;...    % integrator
//      0.03      0.25    -60     4        0;...      % rebound spike
//      0.03      0.25    -52     0        0;...      % rebound burst
//      0.03      0.25    -60     4        0  ;...    % threshold variability
//      1         1.5     -60     0      -65  ;...    % bistability
//        1       0.2     -60     -21      0  ;...    % DAP
//      0.02      1       -55     4        0  ;...    % accomodation
//     -0.02      -1      -60     8        80 ;...    % inhibition-induced spiking
//     -0.026     -1      -45     0        80];       % inhibition-induced bursting


