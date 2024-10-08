#include "gst_player.h"

#include <gst/video/video.h>

#define GstPlayer_GstInit_ProgramName   "gstreamer"
#define GstPlayer_GstInit_Arg1          "/home/1.ogg"

GstPlayer::GstPlayer(const std::vector<std::string>& cmd_arguments) {
  if (cmd_arguments.empty()) {
    char  arg0[] = GstPlayer_GstInit_ProgramName;
    char  arg1[] = GstPlayer_GstInit_Arg1;
    char* argv[] = { &arg0[0], &arg1[0], NULL };
    int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    gst_init(&argc, (char ***)&argv);
  } else {
    // TODO handle this case, pass command line arguments to gstreamer
  }
}

GstPlayer::~GstPlayer() {
  // TODO Should free GStreamers stuff in destructor,
  // but when implemented, flutter complains something about texture
  // when closing application
  // freeGst();
}

void GstPlayer::onVideo(VideoFrameCallback callback) {
  video_callback_ = callback;
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

//void GstPlayer::play(const gchar* pipelineString, const gchar* rtmpUri) {
//  pipelineString_ = pipelineString;
//
//  // Check and free previous playing GStreamers if any
//  if (sink_ != nullptr || pipeline != nullptr) {
//    freeGst();
//  }
//
//  // Create the pipeline from the provided pipeline string
//  pipeline = gst_parse_launch(pipelineString_.c_str(), nullptr);
//
//  // Create the necessary GStreamer elements
//  GstElement *tee = gst_element_factory_make("tee", "tee");  // To split the stream
//  GstElement *queue1 = gst_element_factory_make("queue", "queue1");  // For the AppSink branch
//  GstElement *queue2 = gst_element_factory_make("queue", "queue2");
//  GstElement *x264enc = gst_element_factory_make("x264enc", "encoder");
//  GstElement *flvmux = gst_element_factory_make("flvmux", "muxer");// For the RTMP branch
//  GstElement *rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");  // For RTMP output
//
//  // Set RTMP sink properties
//  g_object_set(rtmpsink, "location", rtmpUri, NULL);  // Set the RTMP server URL
//  g_object_set(x264enc, "bitrate", 500, NULL);
//  g_object_set(flvmux, "streamable", true, NULL);
//
//  // Get the existing sink (AppSink)
//  sink_ = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
//  gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
//  g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)this);
//
//  // Add new elements (tee, queues, rtmpsink) to the pipeline
//  gst_bin_add_many(GST_BIN(pipeline), tee, queue1, queue2, x264enc, flvmux, rtmpsink, NULL);
//
//  // Link the tee to the AppSink and RTMP sink branches
//  GstPad *tee_src_pad1 = gst_element_get_request_pad(tee, "src_%u");
//  GstPad *tee_src_pad2 = gst_element_get_request_pad(tee, "src_%u");
//
//  // Create a ghost pad from the source pad of the pipeline to tee
//  GstElement *source = gst_bin_get_by_name(GST_BIN(pipeline), "source");  // Get the source element
//  GstPad *source_pad = gst_element_get_static_pad(source, "src");
//  GstPad *tee_sink_pad = gst_element_get_static_pad(tee, "sink");
//
//  gst_pad_link(source_pad, tee_sink_pad);  // Link the source to the tee
//  gst_pad_link(tee_src_pad1, gst_element_get_static_pad(queue1, "sink"));  // First tee output to queue1 (AppSink branch)
//  gst_pad_link(tee_src_pad2, gst_element_get_static_pad(queue2, "sink"));  // Second tee output to queue2 (RTMP branch)
//
//  // Link the queue1 (AppSink branch)
//  gst_element_link(queue1, sink_);
//
//  // Link the queue2 to rtmpsink (RTMP branch)
//  gst_element_link(queue2, x264enc);
//  gst_element_link(x264enc, flvmux);
//  gst_element_link(flvmux, rtmpsink);
//
//  // Set the pipeline to the playing state
//  gst_element_set_state(pipeline, GST_STATE_PLAYING);
//}

void GstPlayer::play(const gchar* pipelineString, const gchar* rtmpString) {
  pipelineString_ = pipelineString;

  // Check and free previous playing GStreamers if any
  if (sink_ != nullptr || pipeline != nullptr) {
    freeGst();
  }

  // Create the pipeline elements
  GstElement* udpsrc = gst_element_factory_make("udpsrc", "source");
  GstElement* rtpdepay = gst_element_factory_make("rtph264depay", "rtpdepay");
  GstElement* avdec = gst_element_factory_make("avdec_h264", "avdec");
  GstElement* videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  GstElement* tee = gst_element_factory_make("tee", "tee");
  GstElement* queue1 = gst_element_factory_make("queue", "queue1"); // For appsink
  GstElement* appsink = gst_element_factory_make("appsink", "sink");
  GstElement* queue2 = gst_element_factory_make("queue", "queue2"); // For RTMP
  GstElement* x264enc = gst_element_factory_make("x264enc", "encoder");
  GstElement* flvmux = gst_element_factory_make("flvmux", "muxer");
  GstElement* rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");

  // Check if all elements were created successfully
  if (!udpsrc || !rtpdepay || !avdec || !videoconvert || !tee ||
      !queue1 || !appsink || !queue2 || !x264enc || !flvmux || !rtmpsink) {
    g_printerr("Not all elements could be created.\n");
    return;
  }

  // Set properties
  g_object_set(udpsrc, "port", 53262, NULL);
  g_object_set(appsink, "sync", FALSE, "max-buffers", 1, "drop", TRUE, NULL);
  g_object_set(rtmpsink, "location", rtmpString, NULL);
  g_object_set(x264enc, "bitrate", 500, NULL);
  g_object_set(flvmux, "streamable", TRUE, NULL);

  // Create the pipeline
  pipeline = gst_pipeline_new("pipeline");

  // Add elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), udpsrc, rtpdepay, avdec, videoconvert, tee, queue1, appsink, queue2, x264enc, flvmux, rtmpsink, NULL);

  // Link the elements
  gst_element_link(udpsrc, rtpdepay);
  gst_element_link(rtpdepay, avdec);
  gst_element_link(avdec, videoconvert);
  gst_element_link(videoconvert, tee);

  // Link the tee to appsink and rtmpsink
  GstPad *tee_src_pad1 = gst_element_get_request_pad(tee, "src_0");
  GstPad *tee_src_pad2 = gst_element_get_request_pad(tee, "src_1");
  gst_element_link(queue1, appsink);
  gst_element_link(queue2, x264enc);
  gst_element_link(x264enc, flvmux);
  gst_element_link(flvmux, rtmpsink);

  // Link the tee's source pads to the queues
  if (gst_pad_link(tee_src_pad1, gst_element_get_static_pad(queue1, "sink")) != GST_PAD_LINK_OK) {
    g_printerr("Error: Failed to link tee src pad 1 to queue 1 sink pad.\n");
    return;
  }

  if (gst_pad_link(tee_src_pad2, gst_element_get_static_pad(queue2, "sink")) != GST_PAD_LINK_OK) {
    g_printerr("Error: Failed to link tee src pad 2 to queue 2 sink pad.\n");
    return;
  }

  // Set appsink to emit signals
  sink_ = appsink;
  gst_app_sink_set_emit_signals(GST_APP_SINK(sink_), TRUE);
  g_signal_connect(sink_, "new-sample", G_CALLBACK(newSample), (gpointer)this);

  // Start playing the pipeline
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void GstPlayer::freeGst(void) {
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(sink_);
  gst_object_unref(pipeline);
}

GstFlowReturn GstPlayer::newSample(GstAppSink *sink, gpointer gSelf) {
  GstSample* sample = NULL;
  GstMapInfo bufferInfo;

  GstPlayer* self = static_cast<GstPlayer* >(gSelf);
  sample = gst_app_sink_pull_sample(GST_APP_SINK(self->sink_));

  if(sample != NULL) {
    GstBuffer *buffer_ = gst_sample_get_buffer(sample);
    if(buffer_ != NULL) {
      gst_buffer_map(buffer_, &bufferInfo, GST_MAP_READ);

      // Get video width and height
      GstVideoFrame vframe;
      GstVideoInfo video_info;
      GstCaps* sampleCaps = gst_sample_get_caps(sample);
      gst_video_info_from_caps(&video_info, sampleCaps);
      gst_video_frame_map (&vframe, &video_info, buffer_, GST_MAP_READ);

      self->video_callback_(
          (uint8_t*)bufferInfo.data,
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

GstPlayer* GstPlayers::Get(int32_t id, std::vector<std::string> cmd_arguments) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto [it, added] = players_.try_emplace(id, nullptr);
  if (added) {
    it->second = std::make_unique<GstPlayer>(cmd_arguments);
  }
  return it->second.get();
}

void GstPlayers::Dispose(int32_t id) {
  std::lock_guard<std::mutex> lock(mutex_);
  players_.erase(id);
}
