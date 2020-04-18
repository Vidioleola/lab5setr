cmake_minimum_required(VERSION 3.0)
project(SETR_TP5_PC)

set(SOURCE_TP5_PC main.c opusUtils.c bluetoothServer.c bluetoothClient.c alsaUtils.c)
add_executable(SETR_TP5_PC ${SOURCE_TP5_PC})
TARGET_LINK_LIBRARIES(SETR_TP5_PC opus)
TARGET_LINK_LIBRARIES(SETR_TP5_PC bluetooth)
TARGET_LINK_LIBRARIES(SETR_TP5_PC pthread)
TARGET_LINK_LIBRARIES(SETR_TP5_PC asound)
