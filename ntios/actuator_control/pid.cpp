
#include <math.h>
#include "ntios.h"
#include "actuator_control/pid.h"


PIDControl::PIDControl(double p, double i, double d){
  p_gain = p;
  i_gain = i;
  d_gain = d;
  last_run_micros = -1;
  i_val = 0;
}

double PIDControl::run(double real, double sp){
  double error = sp - real;
  double sum = p_gain * error;

  /*Serial.print("Err=");
  Serial.print(error);
  Serial.print(" Sum=");
  Serial.println(sum);*/

  long time = micros();
  
  if (last_run_micros !=-1){
    double dt = (time - last_run_micros) / 1000000.0;
    /*Serial.print("dT = ");
    Serial.print(dt);
    Serial.println("s");*/
    i_val += error * dt;
    //Serial.print("i=");
    //Serial.println(i_val);
    sum += i_gain * i_val; 
    //Serial.print("Rd=");
    //Serial.println(d_gain * (error - d_last) / dt);
    sum += d_gain * (error - d_last) / dt;
  }
  d_last = error;
  
  last_run_micros = time;
  return sum;
}

PIDControlLimit::PIDControlLimit(double p, double i, double d){
  p_gain = p;
  i_gain = i;
  d_gain = d;
  last_run_micros = -1;
  i_val = 0;
}

double PIDControlLimit::run(double real, double sp){
  double error = tanh(sp - real);
  double sum = p_gain * error;

  /*Serial.print("Err=");
  Serial.print(error);
  Serial.print(" Sum=");
  Serial.println(sum);*/

  long time = micros();
  
  if (last_run_micros !=-1){
    double dt = (time - last_run_micros) / 1000000.0;
    /*Serial.print("dT = ");
    Serial.print(dt);
    Serial.println("s");*/
    i_val += error * dt;
    //Serial.print("i=");
    //Serial.println(i_val);
    sum += i_gain * i_val; 
    //Serial.print("Rd=");
    //Serial.println(d_gain * (error - d_last) / dt);
    sum += d_gain * (error - d_last) / dt;
  }
  d_last = error;
  
  last_run_micros = time;
  return sum;
}
