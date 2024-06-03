#include "motor.h"

// Motor Array ; first 2 for Motor1, next 2 for Motor 2 and last 2 for motor 3  (M1a,M1b, M2a,M2b, M3a,M3b )
const uint8_t MOTOR_IO_PIN[] = {D1,D2, D5,D6, D7,D8};

unsigned long motorCountDown[] = {0, 0, 0};
int motorStatus[]={0, 0, 0};

Ticker motorStopTimer;


/** #########################################################################
   @brief   // set all motor pins to OUTPUT & low
*/
void initMotorPins(void){

  for (int i = 0; i < 6; i++){
    pinMode(MOTOR_IO_PIN[i], OUTPUT);
    digitalWrite(MOTOR_IO_PIN[i], LOW);
  }
}



/** #########################################################################
   @brief Switch IO-Pins to execute motor commands
   @param motor motor number as int 1,2 or 3 // not 0!
   @param dir  direction as int : 0=stop

*/
void setMotor(int motor, int dir)
{

#ifdef SERIAL_VERBOSE
  Serial.print(motor); Serial.print("="); Serial.println(dir);
#endif

  bool a = LOW, b = LOW; // Default=STOP

  if (dir == 1) // RECHTS
  {
    a = HIGH;
  } else if (dir == 2) // LINKS
  {
    b = HIGH;
  } else {
    dir = 0; // must be 0 if not 1 and not 2 !
  }

  motorStatus[motor - 1] = dir;

#ifdef SERIAL_VERBOSE
  Serial.print(MOTOR_IO_PIN[ motor * 2 - 2 ]); Serial.print("="); Serial.println(a);
  Serial.print(MOTOR_IO_PIN[ motor * 2 - 1 ]); Serial.print("="); Serial.println(b);
#endif

  digitalWrite( MOTOR_IO_PIN[ motor * 2 - 2 ] , a);
  digitalWrite( MOTOR_IO_PIN[ motor * 2 - 1 ] , b);

  if (dir != 0) // set stop time
  {
    motorCountDown[motor - 1] = millis() + 10000 ; /// 10 Sekunden!
  } else {
    motorCountDown[motor - 1] = 0;
  }

} // setMotor


void checkRunningMotors()
{

  for (int i = 0; i < 3; i++) {
    #ifdef SERIAL_VERBOSE
    Serial.println(motorCountDown[i]);
    #endif

    if (motorCountDown[i] > 0) { // is motor still running?
      if (millis() > motorCountDown[i] ) { // time up?
        setMotor(i + 1, 0);
      }
    }
  }

} // checkRunningMotors

