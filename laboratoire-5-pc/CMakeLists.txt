cmake_minimum_required(VERSION 3.0)
project(SETR_TP5_PC)

set(SOURCE_TP5_PC main.c opusUtils.c bluetoothClient.c alsaUtils.c)
add_executable(SETR_TP5_PC ${SOURCE_TP5_PC})
#target_compile_options(SETR_TP5_PC PRIVATE -O1)
TARGET_LINK_LIBRARIES(SETR_TP5_PC opus)
TARGET_LINK_LIBRARIES(SETR_TP5_PC bluetooth)
TARGET_LINK_LIBRARIES(SETR_TP5_PC pthread)
TARGET_LINK_LIBRARIES(SETR_TP5_PC asound)

add_executable(AUDIO audio.c)
TARGET_LINK_LIBRARIES(AUDIO asound)

add_executable(COMPRESS compress.c opusUtils.c)
TARGET_LINK_LIBRARIES(COMPRESS opus asound)
