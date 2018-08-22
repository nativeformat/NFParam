/*
 * Copyright (c) 2017 Spotify AB.
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

#include <memory>
#include <vector>

namespace nativeformat {
namespace param {

class Param {
 public:
  // from WAA spec
  virtual float defaultValue() const = 0;
  virtual float maxValue() const = 0;
  virtual float minValue() const = 0;
  virtual void setValue(float value) = 0;
  virtual void setValueAtTime(float value, double time) = 0;
  virtual void linearRampToValueAtTime(float end_value, double end_time) = 0;
  virtual void setTargetAtTime(float target, double start_time, float time_constant) = 0;
  virtual void exponentialRampToValueAtTime(float value, double end_time) = 0;
  virtual void setValueCurveAtTime(std::vector<float> values,
                                   double start_time,
                                   double duration) = 0;

  // other methods
  virtual void addCustomEvent(double start_time,
                              double end_time,
                              Anchor anchor,
                              NF_AUDIO_PARAM_FUNCTION function) = 0;
  virtual float valueForTime(double time) = 0;
  virtual void valuesForTimeRange(float *values,
                                  size_t values_count,
                                  double start_time,
                                  double end_time) = 0;
  virtual std::string name() = 0;
  virtual float smoothedValueForTimeRange(double start_time,
                                          double end_time,
                                          size_t samples = 5) = 0;
  virtual float cumulativeValueForTimeRange(double start_time,
                                            double end_time,
                                            double precision = 0.1) = 0;
};

std::shared_ptr<Param> createParam(float default_value,
                                   float max_value,
                                   float min_value,
                                   const std::string &name);

}  // namespace param
}  // namespace nativeformat
