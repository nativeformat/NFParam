add_library(NFParam
  STATIC
  ${NFPARAM_INCLUDE_DIRECTORY}/NFParam/Param.h
  ${NFPARAM_INCLUDE_DIRECTORY}/NFParam/ParamEvent.h
  ParamEvent.cpp
  WAAParamEvents.h
  WAAParamEvents.cpp
  ParamImplementation.h
  ParamImplementation.cpp)
target_include_directories(
  NFParam
  PUBLIC
  ${NFPARAM_INCLUDE_DIRECTORY})

add_subdirectory(test)
