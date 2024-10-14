#include "gst_player.h"

#include <gst/video/video.h>

#define GstPlayer_GstInit_ProgramName   "gstreamer"
#define GstPlayer_GstInit_Arg1          "/home/1.ogg"

GstPlayer::GstPlayer(const std::vector <std::string> &cmd_arguments) {
    if (cmd_arguments.empty()) {
        char arg0[] = GstPlayer_GstInit_ProgramName;
        char arg1[] = GstPlayer_GstInit_Arg1;
        char *argv[] = {&arg0[0], &arg1[0], NULL};
        int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;
        gst_init(&argc, (char ***) &argv);
    } else {
        // TODO handle this case, pass command line arguments to gstreamer
    }
}

GstPlayer::~GstPlayer() {
    // TODO Should free GStreamers stuff in destructor,
    // but when implemented, flutter complains something about texture
    // when closing application
    freeGst();
}

void GstPlayer::onVideo(VideoFrameCallback callback) {
    video_callback_ = callback;
}

void GstPlayer::play(const gchar* pipelineString, const gchar* rtmpString) {
    pipelineString_ = pipelineString;

    // Check and free previous playing GStreamers if any
    if (sink_ != nullptr || pipeline != nullptr) {
        freeGst();
    }

    // Create new pipeline
    pipeline = gst_parse_launch(pipelineString_.c_str(), nullptr);
    if (!pipeline) {
        g_printerr("Failed to create pipeline.");
        return;
    }

    // Get the sink element (assuming the sink is named "sink" in the pipeline)
    sink_ = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (!sink_) {
        g_printerr("Failed to get sink element from pipeline.");
        freeGst(); // Clean up if there's an error
        return;
    }

    // Connect the new-sample signal to the sink
    gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
    g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)this);

    // Set the pipeline to PLAYING state and wait for the transition
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
//    GstStateChangeReturn ret = gst_element_get_state(pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
//    if (ret != GST_STATE_CHANGE_SUCCESS) {
//        g_printerr("Failed to set pipeline to PLAYING state.");
//        freeGst();
//    }
}


//void GstPlayer::play(const gchar* pipelineString, const gchar* rtmpString) {
//  pipelineString_ = pipelineString;
//
//  // Check and free previous playing GStreamers if any
//  if (sink_ != nullptr || pipeline != nullptr) {
//    freeGst();
//  }
//
//  pipeline = gst_parse_launch(
//       pipelineString_.c_str(),
//      nullptr);
//
//  sink_ = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
//  gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
//  g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)this);
//
//  gst_element_set_state(pipeline, GST_STATE_PLAYING);
//}

//void GstPlayer::play(const gchar* pipelineString, const gchar* rtmpString) {
//    // Check and free previous playing GStreamers if any
//    if (sink_ != nullptr || pipeline != nullptr) {
//        freeGst();
//    }
//
//    // Create the pipeline using the provided pipeline string
//    pipeline = gst_parse_launch(pipelineString, nullptr);
//    if (!pipeline) {
//        g_printerr("Failed to create pipeline from string.\n");
//        return;
//    }
//
//    // Get the appsink element
//    sink_ = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
//    if (!sink_) {
//        g_printerr("Failed to get appsink element from pipeline.\n");
//        gst_object_unref(pipeline);
//        return;
//    }
//
//    // Set properties for appsink
//    gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
//    g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)this);
//
//    // Get the rtmpsink element and set its properties
//    GstElement *rtmpsink = gst_bin_get_by_name(GST_BIN(pipeline), "rtmpsink");
//    if (rtmpsink) {
//        g_object_set(rtmpsink, "location", rtmpString, NULL);  // Set the RTMP location
//    } else {
//        g_printerr("Failed to get rtmpsink element from pipeline.\n");
//        gst_object_unref(pipeline);
//        return;
//    }
//
//    // Start playing the pipeline
//    gst_element_set_state(pipeline, GST_STATE_PLAYING);
//}

//void GstPlayer::play(const gchar *pipelineString, const gchar *rtmpString) {
//    pipelineString_ = pipelineString;
//
//    // Check and free previous playing GStreamers if any
//    if (sink_ != nullptr || pipeline != nullptr) {
//        freeGst();
//    }
//
//    // Create the pipeline from the provided pipeline string
//    pipeline = gst_parse_launch(pipelineString_.c_str(), nullptr);
//
//    // Get the existing appsink element by name
//    sink_ = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
//    gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
//    g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)
//    this);
//
//    // Create additional elements for RTMP streaming
//    GstElement *tee = gst_element_factory_make("tee", "tee");
//    GstElement *queue_rtmp = gst_element_factory_make("queue", "queue_rtmp");
//    GstElement *x264enc = gst_element_factory_make("x264enc", "x264enc");
//    GstElement *flvmux = gst_element_factory_make("flvmux", "flvmux");
//    GstElement *rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");
//
//    if (!tee || !queue_rtmp || !x264enc || !flvmux || !rtmpsink) {
//        g_printerr("Failed to create RTMP elements.\n");
//        return;
//    }
//
//    // Set properties for x264 encoder and rtmpsink
//    g_object_set(x264enc, "bitrate", 500, "tune", 4, NULL);
//    g_object_set(flvmux, "streamable", TRUE, NULL);
//    g_object_set(rtmpsink, "location", rtmpString, NULL);
//
//    // Add new elements to the pipeline
//    gst_bin_add_many(GST_BIN(pipeline), tee, queue_rtmp, x264enc, flvmux, rtmpsink, NULL);
//
//    // Get the last element before appsink (likely videoconvert or similar)
//    GstPad *last_element_before_sink = gst_element_get_static_pad(sink_, "sink");
//    GstPad *tee_pad_sink = gst_element_get_static_pad(tee, "sink");
//
//    // Insert tee into the existing pipeline
//    gst_pad_link(tee_pad_sink, last_element_before_sink);
//
//    // Link the RTMP branch
//    gst_element_link_many(queue_rtmp, x264enc, flvmux, rtmpsink, NULL);
//
//    // Set the pipeline to the playing state
//    gst_element_set_state(pipeline, GST_STATE_PLAYING);
//}

//void GstPlayer::play(const gchar *pipelineString, const gchar *rtmpUri) {
//    pipelineString_ = pipelineString;
//
//    if (sink_ != nullptr || pipeline != nullptr) {
//        freeGst();
//    }
//
//    // Create the pipeline elements
//    GstElement *udpsrc = gst_element_factory_make("udpsrc", "source");
//    GstElement *rtpdepay = gst_element_factory_make("rtph264depay", "rtpdepay");
//    GstElement *avdec = gst_element_factory_make("avdec_h264", "avdec");
//    GstElement *tee = gst_element_factory_make("tee", "tee");
//    GstElement *queue1 = gst_element_factory_make("queue", "queue1"); // For appsink branch
//    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
//    GstElement *appsink = gst_element_factory_make("appsink", "sink");
//    GstElement *queue2 = gst_element_factory_make("queue", "queue2"); // For RTMP branch
//    GstElement *x264enc = gst_element_factory_make("x264enc", "encoder");
//    GstElement *flvmux = gst_element_factory_make("flvmux", "muxer");
//    GstElement *rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");
//
//    // Check if all elements were created successfully
//    if (!udpsrc || !rtpdepay || !avdec || !tee || !queue1 || !videoconvert ||
//        !appsink || !queue2 || !x264enc || !flvmux || !rtmpsink) {
//        g_printerr("Not all elements could be created.\n");
//        return;
//    }
//
//    // Set properties
//    g_object_set(udpsrc, "port", 53262, NULL);
//    g_object_set(appsink, "sync", FALSE, "max-buffers", 1, "drop", TRUE, NULL);
//    g_object_set(x264enc, "bitrate", 500, NULL);
//    g_object_set(flvmux, "streamable", TRUE, NULL);
//    g_object_set(rtmpsink, "location", rtmpUri, NULL);
//
//    // Create the pipeline
//    pipeline = gst_pipeline_new("pipeline");
//
//    // Add elements to the pipeline
//    gst_bin_add_many(GST_BIN(pipeline), udpsrc, rtpdepay, avdec, tee, queue1, videoconvert,
//                     appsink, queue2, x264enc, flvmux, rtmpsink, NULL);
//
//    // Create the RTP caps filter
//    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
//                                        "encoding-name", G_TYPE_STRING, "H264",
//                                        "payload", G_TYPE_INT, 96,
//                                        NULL);
//
//    // Create the RGBA caps filter
//    GstCaps *capsRGBA = gst_caps_new_simple("video/x-raw",
//                                            "format", G_TYPE_STRING, "RGBA",
//                                            NULL);
//
//    // Link udpsrc -> rtph264depay with RTP caps
//    if (!gst_element_link_filtered(udpsrc, rtpdepay, caps)) {
//        g_printerr("Error: Could not link udpsrc to rtph264depay with caps.\n");
//        gst_caps_unref(caps);
//        gst_object_unref(pipeline);
//        return;
//    }
//    gst_caps_unref(caps); // Free the caps after linking
//
//    // Link rtph264depay -> avdec_h264
//    gst_element_link(rtpdepay, avdec);
//
//    // Link the tee's two output branches
//    GstPad *tee_src_pad1 = gst_element_get_request_pad(tee, "src_%u");
//    GstPad *tee_src_pad2 = gst_element_get_request_pad(tee, "src_%u");
//
//    // Queue 1: Video rendering via appsink
//    gst_element_link_many(avdec, tee, NULL); // Link avdec to tee
//    GstPad *queue1_sink_pad = gst_element_get_static_pad(queue1, "sink");
//    if (gst_pad_link(tee_src_pad1, queue1_sink_pad) != GST_PAD_LINK_OK) {
//        g_printerr("Error: Failed to link tee to queue1.\n");
//        gst_object_unref(pipeline);
//        return;
//    }
//    gst_element_link_many(queue1, videoconvert, NULL);
//
//    if (!gst_element_link_filtered(videoconvert, appsink, capsRGBA)) {
//        g_printerr("Error: Could not link videoconvert to appsink with RGBA caps filter.\n");
//        gst_caps_unref(capsRGBA);
//        gst_object_unref(pipeline);
//        return;
//    }
//    gst_caps_unref(capsRGBA); // Free the RGBA caps after linking
//
//    // Queue 2: RTMP streaming branch
//    GstPad *queue2_sink_pad = gst_element_get_static_pad(queue2, "sink");
//    if (gst_pad_link(tee_src_pad2, queue2_sink_pad) != GST_PAD_LINK_OK) {
//        g_printerr("Error: Failed to link tee to queue2.\n");
//        gst_object_unref(pipeline);
//        return;
//    }
//    gst_element_link_many(queue2, x264enc, flvmux, rtmpsink, NULL);
//
//    // Set appsink to emit signals
//    sink_ = appsink;
//    gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
//    g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)this);
//
//    // Set the pipeline to playing state
//    gst_element_set_state(pipeline, GST_STATE_PLAYING);
//}

void GstPlayer::freeGst(void) {
    if (pipeline != nullptr) {
        // Stop the pipeline
        gst_element_set_state(pipeline, GST_STATE_NULL);

        // Unref the sink and pipeline
        if (sink_ != nullptr) {
            // Disconnect signal handler
            //g_signal_handlers_disconnect_by_func(sink_, G_CALLBACK(newSample), this);

            gst_object_unref(sink_);
            sink_ = nullptr;
        }

        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
}


//void GstPlayer::freeGst(void) {
//    gst_element_set_state(pipeline, GST_STATE_NULL);
//    gst_object_unref(sink_);
//    gst_object_unref(pipeline);
//}

GstFlowReturn GstPlayer::newSample(GstAppSink *sink, gpointer gSelf) {
    GstSample *sample = NULL;
    GstMapInfo bufferInfo;

    GstPlayer *self = static_cast<GstPlayer * >(gSelf);
    sample = gst_app_sink_pull_sample(GST_APP_SINK(self->sink_));

    if (sample != NULL) {
        GstBuffer *buffer_ = gst_sample_get_buffer(sample);
        if (buffer_ != NULL) {
            gst_buffer_map(buffer_, &bufferInfo, GST_MAP_READ);

            // Get video width and height
            GstVideoFrame vframe;
            GstVideoInfo video_info;
            GstCaps *sampleCaps = gst_sample_get_caps(sample);
            gst_video_info_from_caps(&video_info, sampleCaps);
            gst_video_frame_map(&vframe, &video_info, buffer_, GST_MAP_READ);

            self->video_callback_(
                    (uint8_t *) bufferInfo.data,
                    video_info.size,
                    video_info.width,
                    video_info.height,
                    video_info.stride[0]);

            gst_buffer_unmap(buffer_, &bufferInfo);
            gst_video_frame_unmap(&vframe);
        }
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}

GstPlayer *GstPlayers::Get(int32_t id, std::vector <std::string> cmd_arguments) {
    std::lock_guard <std::mutex> lock(mutex_);
    auto [it, added] = players_.try_emplace(id, nullptr);
    if (added) {
        it->second = std::make_unique<GstPlayer>(cmd_arguments);
    }
    return it->second.get();
}

void GstPlayers::Dispose(int32_t id) {
    std::lock_guard <std::mutex> lock(mutex_);
    players_.erase(id);
}
