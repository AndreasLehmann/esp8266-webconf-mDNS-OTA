#include <Arduino.h>
#include <Ticker.h>

extern int motorStatus[3];
extern Ticker motorStopTimer;

void setMotor(int, int);
void initMotorPins(void);
void checkRunningMotors(void);

