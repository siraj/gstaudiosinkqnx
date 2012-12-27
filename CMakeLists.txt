project(GSTQNXAUDIOSINK)

set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_C_COMPILER ntox86-gcc CACHE STRING "C compiler")
set(CMAKE_C_FLAGS_RELEASE -Os -fPIC -fvisibility=hidden -fno-strict-aliasing -Wcast-align -W -Wall -Wextra -Wchar-subscripts -Wformat -Wformat-security -Wmissing-format-attribute -Wno-format-y2k -Wno-parentheses -Wno-unused-parameter -Wpointer-arith -Wreturn-type -Wundef -Wwrite-strings)

set(GSTQNXAUDIO_LIB_NAME gstqnxaudiosink)
set(GSTQNXAUDIO_LIBRARIES glib-2.0 gstreamer-0.10 gstaudio-0.10 asound)

add_library(${GSTQNXAUDIO_LIB_NAME} SHARED src/qnxaudiosink.c)

target_link_libraries(${GSTQNXAUDIO_LIB_NAME} ${GSTQNXAUDIO_LIBRARIES})