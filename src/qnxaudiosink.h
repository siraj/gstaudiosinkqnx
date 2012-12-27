/* GStreamer
 * Copyright (C) <2011> Alexander Shashkevych <alexander@stunpix.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more 
 */

#ifndef __GST_QNXAUDIOSINK_H__
#define __GST_QNXAUDIOSINK_H__

#include <stdio.h>
#include <stdlib.h>

#include <gst/gst.h>
#include <gst/audio/gstaudiosink.h>
#include <sys/asoundlib.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#define PACKAGE "gstreamer"

G_BEGIN_DECLS

#define GST_TYPE_QNXAUDIOSINK            (qnxaudiosink_get_type())
#define GST_QNXAUDIOSINK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_QNXAUDIOSINK,QNXAudioSink))
#define GST_QNXAUDIOSINK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_QNXAUDIOSINK,QNXAudioSinkClass))
#define GST_IS_QNXAUDIOSINK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_QNXAUDIOSINK))
#define GST_IS_QNXAUDIOSINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_QNXAUDIOSINK))

#define INFO_LOG(MSG, ...) { slogf( _SLOG_SETCODE(_SLOGC_3RDPARTY_START, 0), _SLOG_INFO, "INFO: GStreamer QNX audio sink: " MSG, ##__VA_ARGS__); }
#define ERROR_LOG(MSG, ...) { slogf( _SLOG_SETCODE(_SLOGC_3RDPARTY_START, 0), _SLOG_ERROR, "ERROR: GStreamer audio plugin: " MSG, ##__VA_ARGS__); }

#define LIBRARY_API __attribute__ ((visibility ("default")))

LIBRARY_API GstPluginDesc gst_plugin_desc ;

typedef struct _QNXAudioSink QNXAudioSink;
typedef struct _QNXAudioSinkClass QNXAudioSinkClass;

struct _QNXAudioSink
{
    GstAudioSink sink;
    guint bufSize;
    gboolean eos;
    gint card;
    gint device;
    snd_pcm_t *pcm_handle;
    snd_mixer_t *mixer_handle;
    snd_mixer_group_t group;
    snd_pcm_channel_params_t plugparams;
    snd_pcm_channel_info_t  pluginfo;
    snd_pcm_channel_setup_t setup;
    GMutex *mutex;
};

struct _QNXAudioSinkClass
{
    GstAudioSinkClass parent_class;
};

GType qnxaudiosink_get_type(void);

G_END_DECLS

#endif /* __GST_QNXAUDIOSINK_H__ */
