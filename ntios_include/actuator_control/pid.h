
#ifndef PID_H
#define PID_H

class PIDControl{
  public:
    double p_gain, i_gain, d_gain;

    PIDControl(double p, double i, double d);
    double run(double real, double sp);

  private:
    double i_val;
    double d_last;
    long last_run_micros;
};

class PIDControlLimit{
  public:
    double p_gain, i_gain, d_gain;

    PIDControlLimit(double p, double i, double d);
    double run(double real, double sp);

  private:
    double i_val;
    double d_last;
    long last_run_micros;
};

#endif
