//-------------------------------------------------------------------------------------  FUNCTIONS -------------------------------------------------------------------------------------------------------//

// Contains all the functions used in the programme.

/* In order, they are as follows:

NEOPIXEL FUNCTIONS --> colourWipe(colour, delay), resultLoop()
SERVO FUNCTIONS --> move0To90(servo_number), move180To90(servo_number), moveTo0(servo_number), moveTo180(servo_number), resetAllServos()
DECISION TREE (SERVO) --> moveDecision()
TOUCH ELECTRODE AND MUSIC FUNCTIONS --> playMusic(), checkWin()

*/

//--------------------------------------------------------------------------------------- NEOPIXEL -------------------------------------------------------------------------------------------------------//


// COLOUR WIPE (colour) FUNCTION -------------------------------------------------------------------------

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show(); 
    delay(wait);                           //  Pause for a moment
  }
}

// LED AND SOUND WHEN A PLAYER WINS OR LOSES --------------------------------------------------------------
void resultLoop(int r, int g, int b) {  // Takes 3 int values making up RGB spectrum
  for (int c = 0; c <5; c++) {
    colorWipe(strip.Color(r,g,b),100);
    for (int i = 0; i <10; i++) {
      strip.setPixelColor(i, strip.Color(0,0,0)); 
    }
    strip.show(); 
  }
  
  counter = 0;
  delay(3000);
  resetAllServos();

}


//------------------------------------------------------------------------------------- SERVO FUNCTIONS -------------------------------------------------------------------------------------------//

/*--------- FUNCTIONS --------------/

move0to90 --> Move servo from 0 degree (sMAX) to 90 degrees (sMID)
move180to90 --> Move servo from 180 degree (sMIN) to 90 degrees (sMID)
moveTo180 --> Move from wherever the servo is (curPos) to 180 (sMIN)
moveTo0 --> Move from wherever the servo is (curPos) to 180 (sMAX)

/-----------------------------------*/

//************* MOVE FROM 0 to 90 ***************/

void move0To90(int servoNo) {                   // Use like moveTo90(0); - Will move Servo 0 to 90 degrees; Will set curPos[0] to final position after movement i.e. ~ 360
  unsigned int Pos;                             // Initialising variable Pos to set position outside the for-loop's scope
  for (Pos = sMAX; Pos >= sMID; Pos -= sSPD) {  // for loop for gradual movement
    HCPCA9685.Servo(servoNo, Pos);              // Move servoNo i.e Servo 0 for eg to Pos
    delay(10);                                  // Small delay to make movement slower and gradual
  }
  curPos[servoNo] = Pos;  // Saves Pos into curPos array at the index
}

//*********** MOVE FROM 180 TO 90 *****************//

void move180To90(int servoNo) {  // Use like move180To90(0); - Will move Servo 0 which is at 180 degrees to 90 degrees; Will set curPos[0] to final position after movement i.e. ~ 360
  unsigned int Pos;
  for (Pos = sMIN; Pos <= sMID; Pos += sSPD) {
    HCPCA9685.Servo(servoNo, Pos);
    delay(10);
  }
  curPos[servoNo] = Pos;            // Saves Pos into curPos array at the index
}

//************ MOVE FROM ANYWHERE TO 180 *************//

void moveTo180(int servoNo) {  // Use like moveTo180(0); - Will move Servo 0 to 180 degrees; Will set curPos[0] to final position after movement i.e. ~ 10
  unsigned int Pos;
  for (Pos = curPos[servoNo]; Pos >= sMIN; Pos -= sSPD) {
    HCPCA9685.Servo(servoNo, Pos);
    delay(10);
  }
  curPos[servoNo] = Pos;
}

//************ MOVE FROM ANYWHERE TO 0 *************//

void moveTo0(int servoNo) {  // Use like resetServo(0); - Will move Servo 0 to 0 degrees; Will set curPos[0] to final position after movement i.e. ~ 700
  unsigned int Pos;
  for (Pos = curPos[servoNo]; Pos <= sMAX; Pos += sSPD) {
    HCPCA9685.Servo(servoNo, Pos);  //Move to 0 degrees
    delay(10);
  }
  curPos[servoNo] = Pos;
}

//-------------------------- RESET ALL SERVOS --------------------------------//


void resetAllServos() {

  for (int j = 0; j < 6; j++) {
    moveTo0(j);
  }
}

// --------------------------------------------------------------------------------------- MOVE DECISION TREE --------------------------------------------------------------------------------------------//


void moveDecision(int note) {    // Moves the servos using a switch-case statement based on which note of the guitar is played

  switch(note) {

    case 0: 
    moveTo180(0);
    move0To90(2);
    move0To90(4);
    break;

    case 1:
    move180To90(0);
    moveTo180(3);
    moveTo180(4);
    move0To90(5);
    break;

    case 2:
    moveTo0(0);
    moveTo0(2);
    moveTo0(3);
    moveTo0(5);
    break;

    case 3:
    move0To90(1);
    moveTo0(2);
    moveTo0(5);
    break;

    case 4:
    moveTo180(0);
    moveTo180(1);
    moveTo0(4);
    break;

    case 5:
    moveTo180(2);
    moveTo180(4);
    moveTo180(5);
    break;

  }
}



//---------------------------------------------------------------------------------------- MP3 FUNCTIONS ------------------------------------------------------------------------------------------------//

/*--------- FUNCTIONS --------------

playMusic --> Plays the music for the electrode touched
writeSequence --> Writes the sequence[] to the Serial Monitor
checkWin--> Checks if winning condition is reached

-----------------------------------*/


void playMusic() {

  MPR121.updateAll();

  if (MPR121.getNumTouches() <= 1) {  // If there are multiple touches, it does absolutely nothing at all.
    for (int i = 0; i < 12; i++) {    // Cycle through all electrode numbers from 0 to 11
      if (MPR121.isNewTouch(i)) {     // If there is a touch detected at i, Turn LED on.

        digitalWrite(LED_BUILTIN, HIGH);
        MP3player.playTrack(i);                  // Plays track for button i
        moveDecision(i);                         // Moves servos according to game design

        sequence[lastSeq] = i;                   //Updates value at index of lastSeq (starting at 0) with i i.e. the Servo number moved

        strip.setPixelColor(lastSeq, strip.Color(255,255,255));
        strip.show();

        if (lastSeq > 3) {                                      // If all 5 LEDs are white
          for (int px = 0; px < 10; px++) {
            strip.setPixelColor(px, strip.Color(255,255,255)); // Set all 10 LEDs white
            strip.show();                                      // Indicating the end of this round
          }
        }       
        lastSeq++;                               //Adds to lastSeq value for the next electrode press
        
        if (lastSeq > 4) {                       // If lastSeq exceeds 4 i.e. 5 total button presses, it is reset to zero for the next run at the game
          lastSeq = 0;
        }
        counter++;               // Add to counter value indicating another value has been added to the array of 5. Once the array is complete, the mode is broken

        lastPlayed = i;
      }
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}


//----------------------------------------------------------------------- FUNCTION - CHECKS IF sequence[]] MATCHES targetSequence[] ----------------------------------------------------------------------//



bool checkWin() {  // Use as if (checkWin()) { Do this; } else { Do that; }

  bool state = true;

  for (int i = 0; i < 5; i++) {              // Go through values of i from 0 to 3
    if (sequence[i] != targetSequence[i]) {  // If value at i in sequence[] is not equal to value at i in targetSequence[] i.e. if any of the values don't match
      return false;                          // Return (false)
    } else {
      state = true;  // If all the values match, return true
    }
  }

  return state;
}


//---------------------------------------------------------------------------------------------- END OF FUNCTIONS ----------------------------------------------------------------------------------------//
