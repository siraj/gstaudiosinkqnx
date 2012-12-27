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

#include "qnxaudiosink.h"

static void     qnxaudiosink_finalize (GObject * object);
static GstCaps* qnxaudiosink_getcaps (GstBaseSink * bsink);
static gboolean qnxaudiosink_open (GstAudioSink * asink);
static gboolean qnxaudiosink_close (GstAudioSink * asink);
static gboolean qnxaudiosink_prepare (GstAudioSink * asink, GstRingBufferSpec * spec);
static gboolean qnxaudiosink_unprepare (GstAudioSink * asink);
static guint    qnxaudiosink_write (GstAudioSink * asink, gpointer data, guint length);
static gint     get_qnx_format (GstBufferFormat fmt);


gboolean plugin_init (GstPlugin * plugin)
{
    INFO_LOG("Plugin loaded");
    if ( !gst_element_register (plugin, "qnxaudiosink", GST_RANK_PRIMARY, GST_TYPE_QNXAUDIOSINK)) {
        ERROR_LOG("Could not register element!");
        return FALSE;
    }

    return TRUE;
}

GST_PLUGIN_DEFINE (0, 10, "qnxaudiosink", "QNX audio sink", plugin_init, "0.10.33", "LGPL", "Gstreamer", "GStreamer community")

static GstStaticPadTemplate qnxaudiosink_sink_factory =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) { " G_STRINGIFY (G_BYTE_ORDER) " }, "
        "signed = (boolean) { TRUE, FALSE }, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "rate = (int) {8000, 11025, 22050, 44100}, "
        "channels = (int) [ 1, 2 ]; "
        "audio/x-raw-int, "
        "endianness = (int) { " G_STRINGIFY (G_BYTE_ORDER) " }, "
        "signed = (boolean) { TRUE, FALSE }, "
        "width = (int) 8, "
        "depth = (int) 8, "
        "rate = (int) {8000, 11025, 22050, 44100}, " "channels = (int) [ 1, 2 ]")
    );

GST_BOILERPLATE (QNXAudioSink, qnxaudiosink, GstAudioSink, GST_TYPE_AUDIO_SINK);

static void qnxaudiosink_base_init (gpointer g_class)
{
    INFO_LOG("qnxaudiosink_base_init");
    GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

    gst_element_class_set_details_simple (element_class, "QNX audio sink",
        "Sink/Audio",
        "Output for QNX sound system",
        "Alexander Shashkevych <alexander@stunpix.com>");

    gst_element_class_add_pad_template (element_class, gst_static_pad_template_get (&qnxaudiosink_sink_factory));
}

static void qnxaudiosink_class_init (QNXAudioSinkClass * klass)
{
    INFO_LOG("qnxaudiosink_class_init");
    GObjectClass *gobject_class;
    GstBaseSinkClass *gstbasesink_class;
    GstAudioSinkClass *gstaudiosink_class;

    gobject_class = (GObjectClass *) klass;
    gstbasesink_class = (GstBaseSinkClass *) klass;
    gstaudiosink_class = (GstAudioSinkClass *) klass;

    gobject_class->finalize = GST_DEBUG_FUNCPTR (qnxaudiosink_finalize);

    gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (qnxaudiosink_getcaps);

    gstaudiosink_class->open = GST_DEBUG_FUNCPTR (qnxaudiosink_open);
    gstaudiosink_class->close = GST_DEBUG_FUNCPTR (qnxaudiosink_close);
    gstaudiosink_class->prepare = GST_DEBUG_FUNCPTR (qnxaudiosink_prepare);
    gstaudiosink_class->unprepare = GST_DEBUG_FUNCPTR (qnxaudiosink_unprepare);
    gstaudiosink_class->write = GST_DEBUG_FUNCPTR (qnxaudiosink_write);
}

static void qnxaudiosink_init (QNXAudioSink * qnxaudiosink, QNXAudioSinkClass * g_class)
{
    INFO_LOG("Initializing qnxaudiosink");
    qnxaudiosink->eos = FALSE;
    qnxaudiosink->mutex = g_mutex_new();
}

static gboolean qnxaudiosink_open (GstAudioSink * asink)
{
    INFO_LOG("qnxaudiosink_open");

    QNXAudioSink *qnxaudio;
    qnxaudio = GST_QNXAUDIOSINK (asink);
    int rtn;

    char cardName[512];
    int i;
    int cardsNum = snd_cards(); // Get number of sound cards in system
    if (cardsNum < 1) {
        ERROR_LOG("Number of sound cards in system is less than 1!!!");
        return FALSE;
    } else {
        INFO_LOG("Cards = %d", cardsNum);
    }

    int foundCardsDevicesNum;
    int* foundCards = malloc( cardsNum * sizeof(int)); // Create array of ints which will hold card numbers
    int* foundDevices = malloc( cardsNum * sizeof(int)); // Create array of ints which will hold card numbers

    if (snd_pcm_find( get_qnx_format(spec->format), &foundCardsDevicesNum, foundCards, foundDevices, SND_PCM_OPEN_PLAYBACK) < 1) {
        ERROR_LOG("Didn't found any compatible PCM devices and cards!");
        return FALSE;
    }

    INFO_LOG("Available cards in system:");
    for (i = 0; i < foundCardsDevicesNum; i++) {
        INFO_LOG("Card ID: %d", foundCards[i]);
        INFO_LOG("Device ID: %d", foundDevices[i]);

        snd_card_get_longname(foundCards[i], &cardName[0], sizeof(cardName));
        INFO_LOG("Card: %s", cardName);

        snd_card_get_name(foundCards[i], &cardName[0], sizeof(cardName));
        INFO_LOG("Short name: %s", cardName);
    }

    // Get first found compatible card
    qnxaudio->card = foundCards[0];
    qnxaudio->device = foundDevices[0];

    free(foundCards);
    free(foundDevices);

    if ((rtn = snd_pcm_open_preferred (&qnxaudio->pcm_handle, &qnxaudio->card, &qnxaudio->device, SND_PCM_OPEN_PLAYBACK)) < 0) {
        ERROR_LOG("snd_pcm_open_preferred() failed");
        return FALSE;
    }

    return TRUE;
}

static gboolean qnxaudiosink_prepare (GstAudioSink * asink, GstRingBufferSpec * spec)
{
    INFO_LOG("qnxaudiosink_prepare");
    QNXAudioSink *qnxaudio;
    qnxaudio = GST_QNXAUDIOSINK (asink);
    int rtn;

    memset (&qnxaudio->pluginfo, 0, sizeof (qnxaudio->pluginfo));
    memset (&qnxaudio->plugparams, 0, sizeof (qnxaudio->plugparams));
    memset (&qnxaudio->setup, 0, sizeof (qnxaudio->setup));
    memset (&qnxaudio->group, 0, sizeof (qnxaudio->group));

    if (spec->width != 16 && spec->width != 8) {
        ERROR_LOG("unexpected sample bit size: %d", spec->width);
        return FALSE;
    }

    qnxaudio->pluginfo.channel = SND_PCM_CHANNEL_PLAYBACK;
    if ((rtn = snd_pcm_plugin_info (qnxaudio->pcm_handle, &qnxaudio->pluginfo)) < 0) {
        ERROR_LOG("snd_pcm_plugin_info failed: %s", snd_strerror (rtn));
        return FALSE;
    }
    memset (&qnxaudio->plugparams, 0, sizeof (qnxaudio->plugparams));

    qnxaudio->plugparams.mode = SND_PCM_MODE_BLOCK;
    qnxaudio->plugparams.channel = SND_PCM_CHANNEL_PLAYBACK;
    qnxaudio->plugparams.start_mode = SND_PCM_START_FULL;
    qnxaudio->plugparams.stop_mode = SND_PCM_STOP_STOP;
    qnxaudio->plugparams.buf.block.frag_size = qnxaudio->pluginfo.max_fragment_size;
    qnxaudio->plugparams.buf.block.frags_max = 1;
    qnxaudio->plugparams.buf.block.frags_min = 1;

    qnxaudio->plugparams.format.interleave = 1;
    qnxaudio->plugparams.format.rate = spec->rate;
    qnxaudio->plugparams.format.voices = spec->channels;
    qnxaudio->plugparams.format.format = get_qnx_format(spec->format);

    INFO_LOG("Audio specifications");
    INFO_LOG("Format:%s", snd_pcm_get_format_name (qnxaudio->plugparams.format.format));
    INFO_LOG("Rate:%d", qnxaudio->plugparams.format.rate);
    INFO_LOG("Channels:%d", qnxaudio->plugparams.format.voices);

    if ((rtn = snd_pcm_plugin_params (qnxaudio->pcm_handle, &qnxaudio->plugparams)) < 0) {
        ERROR_LOG("snd_pcm_plugin_params failed: %s", snd_strerror (rtn));
        return FALSE;
    }

    if ((rtn = snd_pcm_plugin_prepare (qnxaudio->pcm_handle, SND_PCM_CHANNEL_PLAYBACK)) < 0) {
        ERROR_LOG("snd_pcm_plugin_prepare failed: %s", snd_strerror (rtn));
    }

    qnxaudio->setup.channel = SND_PCM_CHANNEL_PLAYBACK;
    qnxaudio->setup.mixer_gid = &qnxaudio->group.gid;

    if ((rtn = snd_pcm_plugin_setup(qnxaudio->pcm_handle, &qnxaudio->setup)) < 0) {
        ERROR_LOG("snd_pcm_plugin_setup failed: %s", snd_strerror (rtn));
        return FALSE;
    }

    qnxaudio->bufSize = spec->segsize = qnxaudio->setup.buf.block.frag_size;
    INFO_LOG("Buffer:%d", qnxaudio->bufSize);

    if (qnxaudio->group.gid.name[0] == 0) {
        ERROR_LOG("Mixer Pcm Group [%s] Not Set", qnxaudio->group.gid.name);
        return FALSE;
    }

    INFO_LOG("Mixer PCM Group [%s]", qnxaudio->group.gid.name);

    if ((rtn = snd_mixer_open (&qnxaudio->mixer_handle, qnxaudio->card, qnxaudio->setup.mixer_device)) < 0) {
        ERROR_LOG("snd_mixer_open failed: %s", snd_strerror (rtn));
        return FALSE;
    }

    spec->bytes_per_sample = spec->channels * ( spec->width >> 3);
    memset (spec->silence_sample, 0, spec->bytes_per_sample);

    return TRUE;
}

static gboolean qnxaudiosink_close (GstAudioSink * asink)
{
    INFO_LOG("qnxaudiosink_close");
    QNXAudioSink *qnxaudio = GST_QNXAUDIOSINK (asink);
    snd_mixer_close (qnxaudio->mixer_handle);
    snd_pcm_close (qnxaudio->pcm_handle);
    return TRUE;
}

static void qnxaudiosink_finalize (GObject * object)
{
    INFO_LOG("qnxaudiosink_finalize");
    QNXAudioSink *qnxaudiosink = GST_QNXAUDIOSINK(object);
    g_mutex_free(qnxaudiosink->mutex);
    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean qnxaudiosink_unprepare (GstAudioSink * asink)
{
    INFO_LOG("qnxaudiosink_unprepare");
    QNXAudioSink *qnxaudio = GST_QNXAUDIOSINK (asink);
    qnxaudio->eos = TRUE;
    snd_pcm_plugin_flush (qnxaudio->pcm_handle, SND_PCM_CHANNEL_PLAYBACK);
    return TRUE;
}

static guint qnxaudiosink_write (GstAudioSink * asink, gpointer data, guint length)
{
    //INFO_LOG("qnxaudiosink_write");
    QNXAudioSink *qnxaudio = GST_QNXAUDIOSINK (asink);
    snd_pcm_channel_status_t status;
    int written = 0;

    if (qnxaudio->bufSize != length) {
        ERROR_LOG("ring buffer segment length (%u) != qnx sound buffer len (%u)", length, qnxaudio->bufSize);
    }

    g_mutex_lock(qnxaudio->mutex);

    do {
        if (!qnxaudio->eos) {
            written = snd_pcm_plugin_write (qnxaudio->pcm_handle, data, qnxaudio->bufSize);
            if ((guint)written < length) {
                memset (&status, 0, sizeof (status));
                status.channel = SND_PCM_CHANNEL_PLAYBACK;
                if (snd_pcm_plugin_status(qnxaudio->pcm_handle, &status) < 0) {
                    ERROR_LOG("underrun: playback channel status error");
                    written = 0;
                    break;
                }

                if (status.status == SND_PCM_STATUS_READY ||
                    status.status == SND_PCM_STATUS_UNDERRUN) {
                    if (snd_pcm_plugin_prepare (qnxaudio->pcm_handle, SND_PCM_CHANNEL_PLAYBACK) < 0) {
                        ERROR_LOG("underrun: playback channel prepare error");
                        written = 0;
                        break;
                    }
                }
                if (written < 0)
                    written = 0;
                written += snd_pcm_plugin_write(qnxaudio->pcm_handle, ((char*)data) + written, length - written);
            }
        }
    } while(0);

    g_mutex_unlock(qnxaudio->mutex);
    return written;
}


static GstCaps* qnxaudiosink_getcaps (GstBaseSink * bsink)
{
    ERROR_LOG("qnxaudiosink_getcaps");
    return gst_caps_copy (gst_pad_get_pad_template_caps (GST_BASE_SINK_PAD (bsink)));
}

static gint get_qnx_format (GstBufferFormat fmt)
{
    gint result = GST_UNKNOWN;

    switch (fmt) {
        case GST_U8:
            result = SND_PCM_SFMT_U8;
            break;
        case GST_S8:
            result = SND_PCM_SFMT_S8;
            break;
        case GST_S16_LE:
            result = SND_PCM_SFMT_S16_LE;
            break;
        case GST_S16_BE:
            result = SND_PCM_SFMT_S16_BE;
            break;
        case GST_U16_LE:
            result = SND_PCM_SFMT_U16_LE;
            break;
        case GST_U16_BE:
            result = SND_PCM_SFMT_U16_BE;
            break;
        default:
            break;
    }
    return result;
}
