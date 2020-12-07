#include "main.h"

#ifndef DEFINE_HPP
#define DEFINE_HPP

/* ********** Declare Motors ********** */

extern Motor LFD, LBD , RFD, RBD; // Drive motors
extern Motor LI, RI; // Intakes
extern Motor LU, RU; // Uptakes

/* ********** Declare Sensors ********** */

extern Imu IMU; // Inertial measurement unit
extern Optical Intake_Optical; // Optical sensor
extern ADILineSensor Bottom_Uptake_Line, Middle_Uptake_Line, Top_Uptake_Line; // Line sensors
extern ADIEncoder HEnc, VEnc; // Encoders

/* ********** Declare Controller ********** */

extern Controller Main; // Main controller

#endif
