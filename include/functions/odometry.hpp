#include "main.h"

#ifndef ODOMETRY_HPP
#define ODOMETRY_HPP

class Odom {
public:
  Odom();
  ~Odom();

  void reset();
  float getX();
  float getY();
  float getTheta();
  static void startTask(void* param);
  void endTask();

private:
  float x, y, theta;
  static bool isRunning;
};

#endif
