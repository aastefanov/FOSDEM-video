cmake_minimum_required(VERSION 3.10)

project(MixerApi C)

file(GLOB mixer_SRC "*.c")
file(GLOB mixer_LIB "mixer.h")


add_library(mixerapi SHARED ${mixer_LIB} ${mixer_SRC})
set_target_properties(mixerapi PROPERTIES PUBLIC_HEADER mixer.h)
set_property(TARGET mixerapi PROPERTY C_STANDARD 90) # C89
