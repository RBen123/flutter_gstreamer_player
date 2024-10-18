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
    //gst_element_set_state(pipeline, GST_STATE_NULL);
    //gst_element_set_state(pipeline, GST_STATE_PAUSED);
    // Create a bus to listen for messages from the pipeline
    GstBus *bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)onGstBusMessage, this);
    gst_object_unref(bus);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to set pipeline to PLAYING state.\n");
        removeRtmpSink();
        return;
    }
    g_printerr("start playing.");

//    GstStateChangeReturn ret = gst_element_get_state(pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
//    if (ret != GST_STATE_CHANGE_SUCCESS) {
//        g_printerr("Failed to set pipeline to PLAYING state.");
//        freeGst();
//    }
}

// Handle GStreamer bus messages (e.g., errors)
gboolean GstPlayer::onGstBusMessage(GstBus *bus, GstMessage *msg, gpointer user_data) {
    GstPlayer *self = static_cast<GstPlayer *>(user_data);
    g_printerr("new message receive");
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);
            g_printerr("GStreamer Error: %s\nDebug Info: %s\n", err->message, debug);

            // Check if the error relates to RTMP sink
            if (g_strrstr(err->message, "rtmpsink")) {
                g_printerr("RTMP sink error detected. Removing RTMP sink...\n");
                self->removeRtmpSink();
            }

            g_error_free(err);
            g_free(debug);
            break;
        }
        default:
            break;
    }
    return TRUE;  // Continue receiving messages
}

// Remove the RTMP sink and continue playback without it
void GstPlayer::removeRtmpSink() {
    GstElement *rtmp_sink = gst_bin_get_by_name(GST_BIN(pipeline), "rtmpsink");
    if (rtmp_sink) {
        gst_element_set_state(rtmp_sink, GST_STATE_NULL);  // Stop RTMP sink
        gst_bin_remove(GST_BIN(pipeline), rtmp_sink);      // Remove it from the pipeline
        gst_object_unref(rtmp_sink);
        g_printerr("RTMP sink removed from the pipeline.\n");
        // Restart the pipeline without the RTMP sink
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }
}

void GstPlayer::freeGst(void) {
    //if (pipeline != nullptr) {
        // Stop the pipeline
        gst_element_set_state(pipeline, GST_STATE_NULL);

        // Unref the sink and pipeline
        //if (sink_ != nullptr) {
            // Disconnect signal handler
            //g_signal_handlers_disconnect_by_data(sink_, (gpointer)this);

            gst_object_unref(sink_);
            sink_ = nullptr;
            g_printerr("free sink");
        //}

        gst_object_unref(pipeline);
        pipeline = nullptr;
        g_printerr("free pipeline");
    //}
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
