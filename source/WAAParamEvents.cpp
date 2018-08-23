/*
 * Copyright (c) 2018 Spotify AB.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "WAAParamEvents.h"

#include <cmath>

namespace nativeformat {
namespace param {

ValueAtTimeEvent::ValueAtTimeEvent(float value, double time)
    : ParamEvent(time, ParamEvent::INVALID_TIME, Anchor::START) {
  ParamEvent::start_value = value;
}

ValueAtTimeEvent::~ValueAtTimeEvent() {}

float ValueAtTimeEvent::valueAtTime(double time) {
  return start_value;
}

float ValueAtTimeEvent::cumulativeValue(double start_time, double end_time, double precision) {
  // The integral of a delta function is equivalent to the value of the delta
  return start_value * (end_time - start_time);
}

TargetAtTimeEvent::TargetAtTimeEvent(float value, double time, float time_constant)
    : ParamEvent(time, ParamEvent::INVALID_TIME, Anchor::START),
      time_constant(time_constant),
      target(value) {}

TargetAtTimeEvent::~TargetAtTimeEvent() {}

float TargetAtTimeEvent::valueAtTime(double time) {
  if (time < start_time) {
    return start_value;
  }
  return target + (start_value - target) * std::exp(-1.0 * ((time - start_time) / time_constant));
}

float TargetAtTimeEvent::cumulativeValue(double start_time, double end_time, double precision) {
  // \int_{t1}^{t2} T + (s - T)*exp(-\frac{x-s}{c})dt =
  // Tx - \frac{s-T}{c}exp(-\frac{x-s}{c}) |_{t1}^{t2}
  auto integral = [&](const double &time) {
    auto exp_arg = -1.0 * (time - start_time) / time_constant;
    auto exp_fac = (start_value - target) / time_constant;
    return target * time - exp_fac * std::exp(exp_arg);
  };

  return integral(end_time) - integral(start_time) + start_value * (end_time - start_time);
}

LinearRampEvent::LinearRampEvent(float value, double time)
    : ParamEvent(0.0, time, Anchor::END), target(value) {}
LinearRampEvent::~LinearRampEvent() {}

float LinearRampEvent::valueAtTime(double time) {
  if (time < start_time || start_time == end_time) {
    return start_value;
  }
  if (time > end_time) {
    return target;
  }
  double c = (time - start_time) / (end_time - start_time);
  return start_value + (target - start_value) * c;
}

float LinearRampEvent::cumulativeValue(double start_time, double end_time, double precision) {
  // int_{t1}^{t2}mx + bdx = 1/2 * mx^2 + bx|_{t1}^{t2}
  // neglecting coefficients and initial value on purpose
  auto integral = [](const double &time) { return time * time; };
  auto slope = (target - start_value) / (end_time - start_time);
  return .5 * slope * (integral(end_time) - integral(start_time)) +
         start_value * (end_time - start_time);
}

ExponentialRampEvent::ExponentialRampEvent(float value, double time)
    : ParamEvent(0.0, time, Anchor::END), target(value) {}

ExponentialRampEvent::~ExponentialRampEvent() {}

float ExponentialRampEvent::valueAtTime(double time) {
  if (time < start_time || start_time == end_time) {
    return start_value;
  }
  if (time > end_time) {
    return target;
  }
  double p = (time - start_time) / (end_time - start_time);
  return start_value * std::pow(base(), p);
}

float ExponentialRampEvent::cumulativeValue(double start_time, double end_time, double precision) {
  // int_{t1}^{t2} a^b db = \frac{a^b}{logx} |_{t1}^{t2}
  auto integral = [this](const double &time) { return std::pow(this->base(), time); };
  return (integral(end_time) - integral(start_time)) / std::log(base()) +
         start_value * (end_time - start_time);
}

ValueCurveEvent::ValueCurveEvent(const std::vector<float> &values,
                                 double start_time,
                                 double duration)
    : ParamEvent(start_time, start_time + duration, Anchor::ALL),
      values(values),
      duration(duration) {
  ParamEvent::start_value = values.front();
}

ValueCurveEvent::~ValueCurveEvent() {}

float ValueCurveEvent::valueAtTime(double time) {
  if (time > end_time) {
    return values.back();
  }
  size_t n = values.size();
  size_t k = std::floor((time - start_time) * (n - 1) / duration);
  float v0 = values[k];
  float v1 = values[k + 1];
  return v0 + (v1 - v0) * (time - start_time) / (end_time - start_time);
}

DummyEvent::DummyEvent(float value) : ParamEvent(0.0, ParamEvent::INVALID_TIME, Anchor::NONE) {
  ParamEvent::start_value = value;
}

DummyEvent::~DummyEvent() {}

float DummyEvent::valueAtTime(double time) {
  return start_value;
}

}  // namespace param
}  // namespace nativeformat
