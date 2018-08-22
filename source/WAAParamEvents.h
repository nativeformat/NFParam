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
#pragma once

#include <NFParam/ParamEvent.h>

#include <vector>

namespace nativeformat {
namespace param {

struct ValueAtTimeEvent : ParamEvent {
  ValueAtTimeEvent(float value, double time);
  virtual ~ValueAtTimeEvent();

  float valueAtTime(double time) override;

  virtual float cumulativeValue(double start_time, double end_time, double precision = .1) override;
};

struct TargetAtTimeEvent : ParamEvent {
  const float time_constant;
  const float target;

  TargetAtTimeEvent(float value, double time, float time_constant);
  virtual ~TargetAtTimeEvent();

  float valueAtTime(double time) override;
  virtual float cumulativeValue(double start_time, double end_time, double precision = .1) override;
};

struct LinearRampEvent : ParamEvent {
  const float target;

  LinearRampEvent(float value, double time);
  virtual ~LinearRampEvent();

  float valueAtTime(double time) override;
  virtual float cumulativeValue(double start_time, double end_time, double precision = .1) override;
};

struct ExponentialRampEvent : ParamEvent {
  const float target;

  ExponentialRampEvent(float value, double time);
  virtual ~ExponentialRampEvent();

  float valueAtTime(double time) override;
  virtual float cumulativeValue(double start_time, double end_time, double precision = .1) override;

 private:
  double base() { return target / start_value; }
};

struct ValueCurveEvent : ParamEvent {
  const std::vector<float> values;
  const double duration;

  ValueCurveEvent(const std::vector<float> &values, double start_time, double duration);
  virtual ~ValueCurveEvent();

  float valueAtTime(double time) override;
};

struct DummyEvent : ParamEvent {
  DummyEvent(float value);
  virtual ~DummyEvent();

  float valueAtTime(double time) override;
};

template <typename EventClass, typename... Args>
std::unique_ptr<EventClass> createEvent(Args... args) {
  return std::unique_ptr<EventClass>(new EventClass(args...));
}

}  // namespace param
}  // namespace nativeformat
