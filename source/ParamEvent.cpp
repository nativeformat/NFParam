#include <NFParam/ParamEvent.h>

namespace nativeformat {
namespace param {

ParamEvent::ParamEvent(double start_time, double end_time, Anchor anchor)
    : start_time(start_time), end_time(end_time), start_value(0), anchor(anchor) {}

ParamEvent::~ParamEvent() {}

float ParamEvent::endValue() {
  return valueAtTime(end_time);
}

float ParamEvent::cumulativeValue(double start_time, double end_time, double precision) {
  double cumulative_value = 0.0;

  // define a function to define the trapezoid rule
  auto trapezoid = [this](const double &time_at, const double &time_diff) {
    return (this->valueAtTime(time_at + time_diff) + this->valueAtTime(time_at)) * time_diff / 2.;
  };
  // now calculate remainder
  double time = start_time;
  for (; time + precision < end_time; time += precision) {
    cumulative_value += trapezoid(time, precision);
  }
  // get the remainder of the integral
  cumulative_value += trapezoid(time, end_time - time);
  return cumulative_value;
}

CustomParamEvent::CustomParamEvent(double start_time,
                                   double end_time,
                                   Anchor anchor,
                                   NF_AUDIO_PARAM_FUNCTION function)
    : ParamEvent(start_time, end_time, anchor), function(function) {}

CustomParamEvent::~CustomParamEvent() {}

float CustomParamEvent::valueAtTime(double time) {
  return function(time);
}

}  // namespace param
}  // namespace nativeformat
