<img alt="NFParam" src="NFParam.png" width="100%" max-width="888">

[![Build Status](https://api.travis-ci.org/spotify/NFParam.svg)](https://travis-ci.org/spotify/NFParam)
[![License](https://img.shields.io/github/license/spotify/NFParam.svg)](LICENSE)
[![Spotify FOSS Slack](https://slackin.spotify.com/badge.svg)](https://slackin.spotify.com)
[![Readme Score](http://readme-score-api.herokuapp.com/score.svg?url=https://github.com/spotify/nfparam)](http://clayallsopp.github.io/readme-score?url=https://github.com/spotify/nfparam)

A C++ library for defining and evaluating piecewise functions, inspired by the Web Audio API [AudioParam](https://webaudio.github.io/web-audio-api/#AudioParam) interface.

## Platform support
`NFParam` should compile and run on any platform with a C++11 compiler, but the following are tested in CI.

- [x] 📱 iOS 9.0+
- [x] 💻 OS X 10.11+
- [x] 🐧 Ubuntu Trusty 14.04+

## Develop

### Dependencies
CMake 3.5 or later is required to generate the build.
```shell
brew install cmake
```
or
```shell
sudo apt-get install cmake
```

`NFParam` is a [CMake](https://cmake.org/) project, so to use it within a larger CMake project, simply add the following line to your `CMakeLists.txt` file:
```
add_subdirectory(NFParam)
``` 
Alternatively, you can use the CMake generator of your choice to compile or develop NFParam as a standalone library.

### For iOS/OSX
Generate an Xcode project from the Cmake project like so:

```shell
$ mkdir build
$ cd build
$ cmake .. -GXcode
```

### For Linux
Generate a Ninja project from the Cmake project like so:

```shell
$ mkdir build
$ cd build
$ cmake .. -GNinja
```

## Examples

### Create the "Audio Param Example" curve pictured below 

#### Create the `Param`
Specify the default value, min, max, and a name.
Until events are added, its value will be the default value for all times.
```
auto p = nativeformat::param::createParam(0, 1, -1, "testParam");
```

#### Add some events
The `setValueAtTime` command behaves as a step function.
The value specified will be maintained until the next event anchor.
```
p->setValueAtTime(0.2f, 0.0);
p->setValueAtTime(0.3f, 0.1);
p->setValueAtTime(0.4f, 0.2);
```

The linearRampToValueAtTime event computes the slope necessary to 
reach the target value from the previous anchor point by the specified time.
```
p->linearRampToValueAtTime(1.0f, 0.3);
p->linearRampToValueAtTime(0.8f, 0.325);
```

The setTargetAtTime command will expoenentially approach the target value
from the previous anchor with a rate specified by the time constant.
It does not have a fixed end time.
```
p->setTargetAtTime(0.5f, 0.325, time_constant);
p->setValueAtTime(0.552f, 0.5);
```

The `exponentialRampToValueAtTime` event computes the constant necessary to
reach the target value from the previous anchor point by the specified time.
```
p->exponentialRampToValueAtTime(0.75f, 0.6);
p->exponentialRampToValueAtTime(0.05f, 0.7);
```

The `setValueCurveAtTime` event linearly interpolates between the points provided on the user-defined curve.
It cannot overlap with any other command anchors.
```
size_t curve_len = 44100;
std::vector<float> curve(curve_len);
for (int i = 0; i < curve_len; ++i) {
  curve[i] = std::sin((3.14159265 * i) / curve_len);
}
p->setValueCurveAtTime(curve, 0.7, 0.3);
```
#### Retrieve some values from the `Param`
Finally, let's sample some values from the param we have defined!
```
size_t points = 1001;
std::vector<float> x(points), y(points);
p->valuesForTimeRange(y.data(), points, 0.0, 1.0);

double ts = 1.0 / (points - 1);
double t = 0;
for (int i = 0; i < x.size(); ++i) {
  x[i] = t;
  t += ts;
}
```

## Tests
[`NFParamTests.cpp`](source/test/NFParamTests.cpp) contains a number of test cases,
two of which generate TSV output files that should match the
[AudioParam automation example](https://webaudio.github.io/web-audio-api/#example1-AudioParam)
from the WAA spec. In the `resources` directory, there is a [script](resources/plot.sh) that will plot the unit test output data
if you have [gnuplot](http://gnuplot.info/) installed. The path where `paramAutomation.txt` is generated when the tests are run by the 
ci scripts varies by platform.

### Generating a plot
```
sh plot.sh ../build/source/test/Debug/paramAutomation.txt out.png
```
![](resources/paramAutomationExpected.png?raw=true)

