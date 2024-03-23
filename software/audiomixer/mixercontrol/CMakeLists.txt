cmake_minimum_required(VERSION 3.10)

project(MixerControlDemo C)

add_subdirectory(mixer)
add_executable(mixercontroldemo demo.c)

target_link_libraries(mixercontroldemo PRIVATE mixerapi)
set_property(TARGET mixercontroldemo PROPERTY C_STANDARD 90) # C89
