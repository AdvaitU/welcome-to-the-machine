
//------------------------------------------ GLOBAL VARIABLES AND #INCLUDES ----------------------------------------------------------//


/* PINOUT

BARE CONDUCTIVE TOUCH BOARD PIN 0  -------------------------------------------------------------> NEOPIXEL
                            PIN 2 --------------------------------------------------------------> RESET BUTTON
                            MICRO USB -------------------- SERIAL COMMUNICATION ----------------> PC
                            3.5MM AUDIO --------------------------------------------------------> SPEAKER
                            PIN GND ------------> GND PCA9685 SERVO DRIVER ---------------------| 
                            PIN 5V  ------------> VCC PCA9685 SERVO DRIVER ---------------------|
                            PIN SDA ------------> SDA PCA9685 SERVO DRIVER ---------------------|
                            PIN SCL ------------> SCL PCA9685 SERVO DRIVER ---------------------|
                                                      PCA9685 SERVO DRIVER PIN SET 0 -----------> SERVO MOTOR 0
                                                                           PIN SET 1 -----------> SERVO MOTOR 1
                                                                           PIN SET 2 -----------> SERVO MOTOR 2
                                                                           PIN SET 3 -----------> SERVO MOTOR 3
                                                                           PIN SET 4 -----------> SERVO MOTOR 4
                                                                           PIN SET 5 -----------> SERVO MOTOR 5


All the servos start at 0 degrees. But they are not aligned correctly in the starting state. 
The aim is to get all of them to 180 degrees.
Each guitar note is tied to servo rotations in one or more servos - one way or the other
*/


// MP3 INCLUDES --------------------------------------------------------------------------------
#include "Compiler_Errors.h"  // compiler error handling
#include <MPR121.h>           // touch includes
#include <MPR121_Datastream.h>
#include <Wire.h>
#include <SPI.h>  // MP3 includes
#include <SdFat.h>
#include <FreeStack.h>
#include <SFEMP3Shield.h>


// SERVO DRIVER INCLUDES ------------------------------------------------------------------------
#include "HCPCA9685.h"        // Include the HCPCA9685 library
#define I2CAdd 0x40           // I2C slave address for the device/module. For the HCMODU0097 the default I2C address is 0x40
HCPCA9685 HCPCA9685(I2CAdd);  // Create an instance of the library


// NEOPIXEL INCLUDES ----------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif


// MP3 GLOBAL VARIABLES -------------------------------------------------------------------------
const uint32_t BAUD_RATE = 115200;  // touch constants
const uint8_t MPR121_ADDR = 0x5C;
const uint8_t MPR121_INT = 4;
const bool WAIT_FOR_SERIAL = false;           // serial monitor behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;  // MPR121 datastream behaviour constants
uint8_t result;                               // MP3 variables
uint8_t lastPlayed = 0;
SFEMP3Shield MP3player;         // MP3 constants
const bool REPLAY_MODE = true;  // Touching an electrode repeatedly will play the track again from the start each time
SdFat sd;                       // SD card instantiation


// PROGRAMME GLOBAL VARIABLES --------------------------------------------------------------------
int sequence[] = { 7, 7, 7, 7, 7 };        // Sequence that the user plays. Initialised with 7s as they are not present in any possible target sequence
int lastSeq = 0;                           // Variable storing index to add to sequence[i] as necessary
int counter = 0;                           // Counter variable stores values deciding whether Sequence of 5 notes has been input and if the problem should lead to the Win/Lose state
int targetSequence[] = { 0, 1, 3, 4, 5 };  //Sequence required to win. The player must skip the note 2 and play the others in series.



// SERVO DRIVER GLOBAL VARIABLES ------------------------------------------------------------------
int curPos[] = { 700, 700, 700, 700, 700, 700 };  // Initiating an array to save the current positions of all the servos individually

const int sMAX = 700;  //Setting Servo Max, Min and Mid positions to get 0, 90 and 180 degrees.
const int sMID = 360;
const int sMIN = 10;
const int sSPD = 2;  // Servo speed


//NEOPIXEL VARIABLES ------------------------------------------------------------------------------
#define LED_PIN 0
#define LED_COUNT 10
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);  //Declare Neopixel Strip object


//------------------------------------------------------------------------------------------- SETUP ------------------------------------------------------------------------------------------------------//




void setup() {

  // MP3 SETUP -------------------------------------------------------------------------------------
  Serial.begin(9600);            //Begin Serial Monitor
  pinMode(LED_BUILTIN, OUTPUT);  // Set Built-in LED's mode to output
  Serial.print("begin");
  if (WAIT_FOR_SERIAL) {
    while (!Serial)
      ;
  }
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }
  if (!MPR121.begin(MPR121_ADDR)) {  // Error Printing Loop
    while (1)
      ;
  }
  MPR121.setFFI(FFI_10);  // Setup for the MPR121 Chip
  MPR121.setSFI(SFI_10);
  MPR121.setGlobalCDT(CDT_4US);  // reasonable for larger capacitances
  MPR121.autoSetElectrodes();    // autoset all electrode settings
  MP3player.setVolume(10, 10);   // Setting volume of the MP3 Player


  // SERVO DRIVER SETUP ----------------------------------------------------------------------------
  HCPCA9685.Init(SERVO_MODE);  // Initialise the library and set it to 'servo mode'
  HCPCA9685.Sleep(false);      // Wakes the device up
  Serial.begin(9600);          // Begin Serial Communication
  resetAllServos();            // Reset All the Servos to 0 at the beginning once


//LED SETUP --------------------------------------------------------------------------------------
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)


  // RESET BUTTON SETUP -----------------------------------------------------------------------------
  pinMode(2, INPUT);  // Reset button at Pin number 1


  // RESET ALL SERVOS ONCE BEFORE WE BEGIN ----------------------------------------------------------
  resetAllServos();
}



//----------------------------------------------------------------------------------------------- LOOP ---------------------------------------------------------------------------------------------------//




void loop() {

  // RESET BUTTON BEHAVIOUR --------------------------------------------------------------------------------
  if (digitalRead(2) == LOW) {
    counter = 0;       // Reset counter variable to 0
    lastSeq = 0;       // Reset lastSeq variable used to save index in sequence[] to 0
    resetAllServos();  // Reset all servo motors to initial position
    for (int i = 0; i < 10; i++) {  // Reset all neopixels to OFF
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
  }

  // MAIN LOOP FOR GAMEPLAY LOOP ----------------------------------------------------------------------------
  if (counter <= 4) {
    playMusic();
  }

  // RESULTS LOOP --------------------------------------------------------------------------------------------

  if (counter > 4) {
    if (checkWin()) {
      resultLoop(0,255,0);    // Play with green lights
    } else {
      resultLoop(255,0,0);    // Play with red lights
    }
  }
}
