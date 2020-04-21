cmake_minimum_required(VERSION 3.0)
set(CMAKE_TOOLCHAIN_FILE $ENV{HOME}/arm-cross-comp-env/rpi-zero-w-toolchain.cmake)
project(SETR_TP5)

SET(GCC_WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wjump-misses-init -Wshadow")
SET(GCC_ADDRESS_SANITIZER_FLAGS "-fno-omit-frame-pointer")
add_definitions("${GCC_WARNING_FLAGS} ${GCC_ADDRESS_SANITIZER_FLAGS}")

set(SOURCE_TP5 main.c opusUtils.c bluetoothServer.c)
add_executable(SETR_TP5 ${SOURCE_TP5})
target_compile_options(SETR_TP5 PRIVATE -O1)
TARGET_LINK_LIBRARIES(SETR_TP5 opus)
TARGET_LINK_LIBRARIES(SETR_TP5 bluetooth)
TARGET_LINK_LIBRARIES(SETR_TP5 pthread)
