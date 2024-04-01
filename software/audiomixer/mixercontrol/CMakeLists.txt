cmake_minimum_required(VERSION 3.10)

project(MixerControl C)

add_subdirectory(mixer)
add_subdirectory(web)

add_executable(mixercontroldemo demo.c)

add_library(mongoose mongoose/mongoose.c mongoose/mongoose.h)

target_link_libraries(mixercontroldemo PRIVATE mixerapi)
set_property(TARGET mixercontroldemo PROPERTY C_STANDARD 90) # C89

