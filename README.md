GStreamer audio sink plugin for QNX
===================================

This is GStreamer audio sink plugin for QNX systems. Project was started as part of webkit browser for QNX systems on top of glib and gstreamer frameworks. During browser development I searched for a solution for gstreamer that could play audio natively on qnx systems, but I found only SDL plugin that can be compiled for QNX. SDL is excellent project, but for for me it's a big overhead for such simple task as audio output. I started writing own implementation. As reference I took the ALSA plugin for gstreamer since ALSA is fairly similar to qnx audio subsystem.

Dependencies
===============

To build this plugin you need a Linux machine with installed QNX SDK on it. You can obtain your copy of SDK for Linux systems from official [QNX] site: browse to downloads section and download .bin file with name wich looks similar to `QNX® Software Development Platform 6.5.0 [Build XXXXXXXX] - Linux Hosts`. QNX SDK requires licensing for usage, but you can obtain personal non-commercial license for hobbyists. For this you need to register on QNX site and follow the instructions.

To build plugin you need at least:
- glib-2.8
- gstreamer-0.10.33
- cmake-2.8.0
- make

Since we are crosscompiling, therefore cmake and make are linux tools, but glib and gstreamer are qnx binaries! Don't miss this.

Compilation
===========

    $> ./configure
    $> make

Notes
=====

Sources are provided "as is" and they can contain some minor bugs, because they were taken from another big project with pretty complex build environment, so please help me to improve sources by forking the project.

[QNX]: http://qnx.com
