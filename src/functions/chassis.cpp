#include "main.h"
#include "define.hpp"
#include "functions/chassis.hpp"
#include "functions/odometry.hpp"
#include "functions/pid.hpp"
#include "functions/math.hpp"


Chassis::Chassis(){}
Chassis::~Chassis(){}

Chassis chassis;
static Math math;
static PID pid;
static Odom odom;

bool Chassis::isRunning = false,
     Chassis::isSettled = true,
     Chassis::isTurning = false,
     Chassis::isDriving = false;

float Chassis::targetTheta = 0,
      Chassis::targetTicks = 0,
      Chassis::targetVoltage = 0;

float Chassis::leftVoltage = 0,
      Chassis::rightVoltage = 0,
      Chassis::timeOut = 0;

float targetX = 0, 
      targetY = 0,
      finalVoltage = 0,
      ticks = 0,
      theta = 0,
      drift = 0;

/**
 * Stops the drivetrain
 */
Chassis& Chassis::stop() {
  LFD.move_velocity(0);
  LBD.move_velocity(0);
  RFD.move_velocity(0);
  RBD.move_velocity(0);
  return *this;
}

/**
 * Sets the brake mode of the drivetrain to brake
 */
void Chassis::brake() {
  LFD.set_brake_mode(MOTOR_BRAKE_BRAKE);
  LBD.set_brake_mode(MOTOR_BRAKE_BRAKE);
  RFD.set_brake_mode(MOTOR_BRAKE_BRAKE);
  RBD.set_brake_mode(MOTOR_BRAKE_BRAKE);
}

/**
 * Sets the brake mode of the drivetrain to hold
 */
void Chassis::hold() {
  LFD.set_brake_mode(MOTOR_BRAKE_HOLD);
  LBD.set_brake_mode(MOTOR_BRAKE_HOLD);
  RFD.set_brake_mode(MOTOR_BRAKE_HOLD);
  RBD.set_brake_mode(MOTOR_BRAKE_HOLD);
}

/**
 * Sets the brake mode of the drivetrain to coast
 */
void Chassis::coast() {
  LFD.set_brake_mode(MOTOR_BRAKE_COAST);
  LBD.set_brake_mode(MOTOR_BRAKE_COAST);
  RFD.set_brake_mode(MOTOR_BRAKE_COAST);
  RBD.set_brake_mode(MOTOR_BRAKE_COAST);
}

void Chassis::reset() {
  isSettled = true;
  targetX = 0, targetY = 0, finalVoltage = 0, ticks = 0, theta = 0, drift = 0;
  leftVoltage = rightVoltage = timeOut = 0;
  targetTheta = targetTicks = targetVoltage = 0;
  chassis.stop().brake();
}

void Chassis::startTask(void* param) {
  delay(300);
  isRunning = true;
  chassis.reset();
  cout<<"Chassis Task Started"<<endl;
  while(isRunning) {
    LFD.move_voltage(leftVoltage);
    RFD.move_voltage(rightVoltage);
    LBD.move_voltage(leftVoltage);
    RBD.move_voltage(rightVoltage);
    if (!isSettled) {
      if (isTurning) {
        finalVoltage = pid.turn(math.angleWrap(theta), targetVoltage);
        leftVoltage = finalVoltage;
        rightVoltage = -finalVoltage;
      } else if (isDriving) {
        finalVoltage = pid.drive(ticks, targetVoltage);
        drift = pid.drift();
        leftVoltage = finalVoltage - drift;
        rightVoltage = finalVoltage + drift;
      }
      if (targetTheta <= odom.getTheta()) {
        isTurning = false;
      } else if (targetTicks <= (LEnc.get_value() + REnc.get_value())/2) {
        isDriving = false;
      } else if (millis() >= timeOut) {
        chassis.reset();
        isDriving = isTurning = false;
      } else if (!isTurning && !isDriving) {
        chassis.reset();
      }
    }
    delay(10);
  }
}

void Chassis::endTask() {
  isRunning = false;
  chassis.reset();
  cout<<"Chassis task ended"<<endl;
}

Chassis& Chassis::drive(float distance, float targetVoltage_, float timeOut_) {
  if (isRunning) {
    ticks = math.inchToTicks(distance);
    targetTicks = ticks + (LEnc.get_value() + REnc.get_value())/2;
    targetVoltage = math.percentToVoltage(targetVoltage_);
    timeOut = math.secToMillis(timeOut_) + millis();
    isDriving = true;
    isTurning = false;
    isSettled = false;
  }
  return *this;
}

Chassis& Chassis::turn(float theta, float targetVoltage_, float timeOut_) {
  if (isRunning) {
    theta = math.angleWrap(theta);
    targetTheta = theta + odom.getTheta();
    targetVoltage = math.percentToVoltage(targetVoltage_);
    timeOut = math.secToMillis(timeOut_) + millis();
    isTurning = true;
    isDriving = false;
    isSettled = false;
  }
  return *this;
}

Chassis& Chassis::driveToPoint(float x, float y, float targetVoltage_, float timeOut_) {
  if (isRunning) {
    targetX = x + odom.getX();
    targetY = y + odom.getY();
    if (targetY != 0) {
      theta = math.angleWrap(atan(targetX/targetY)*(180/M_PI)) - odom.getTheta();
      targetTheta = math.angleWrap(atan(targetX/targetY)*(180/M_PI)) + odom.getTheta();
    }
    ticks = math.inchToTicks(sqrt(pow(targetX, 2) + pow(targetY, 2)));
    targetTicks = math.inchToTicks(sqrt(pow(targetX, 2) + pow(targetY, 2))) + (LEnc.get_value() + REnc.get_value())/2;
    targetVoltage = math.percentToVoltage(targetVoltage_);
    timeOut = math.secToMillis(timeOut_) + millis();
    isTurning = true;
    isDriving = true;
    isSettled = false;
  }
  return *this;
}

Chassis& Chassis::turnToPoint(float x, float y, float targetVoltage_, float timeOut_) {
  if (isRunning) {
    targetX = x + odom.getX();
    targetY = y + odom.getY();
    if (targetY != 0) {
      theta = math.angleWrap(atan(targetX/targetY)*(180/M_PI)) - odom.getTheta();
      targetTheta = math.angleWrap(atan(targetX/targetY)*(180/M_PI)) + odom.getTheta();
    }
    targetVoltage = math.percentToVoltage(targetVoltage_);
    timeOut = math.secToMillis(timeOut_) + millis();
    isTurning = true;
    isDriving = false;
    isSettled = false;
  }
  return *this;
}

Chassis& Chassis::turnToAngle(float theta, float targetVoltage_, float timeOut_) {
  if (isRunning) {
    theta = math.angleWrap(theta) - odom.getTheta();
    targetTheta = math.angleWrap(theta) + odom.getTheta();
    targetVoltage = math.percentToVoltage(targetVoltage_);
    timeOut = math.secToMillis(timeOut_) + millis();
    isTurning = true;
    isDriving = false;
    isSettled = false;
  }
  return *this;
}

void Chassis::waitUntilSettled() {
  while(!isSettled) {
    delay(10);
  }
}
