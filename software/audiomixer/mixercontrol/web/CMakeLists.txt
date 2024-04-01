cmake_minimum_required(VERSION 3.10)

project(MixerControlWeb C)

add_executable(mixercontrolweb main.c)
target_link_libraries(mixercontrolweb PRIVATE mixerapi)
target_link_libraries(mixercontrolweb PRIVATE mongoose)

set_property(TARGET mixercontrolweb PROPERTY C_STANDARD 99) # C99
