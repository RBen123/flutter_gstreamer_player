#ifndef GST_PLAYER_H_
#define GST_PLAYER_H_

#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include <string>
#include <functional>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

typedef std::function<void(uint8_t*, uint32_t, int32_t, int32_t, int32_t)> VideoFrameCallback;

class GstPlayer {
  public:
    GstPlayer(const std::vector<std::string>& cmd_arguments);

    ~GstPlayer(void);

    void onVideo(VideoFrameCallback callback);

    void play(const gchar* pipelineString, const gchar* rtmpString);

  private:
    std::string pipelineString_;
    std::string rtmpString_;
    VideoFrameCallback video_callback_;

    GstElement *pipeline = nullptr;
    GstElement *sink_ = nullptr;

    void freeGst(void);
    void removeRtmpSink();

    static GstFlowReturn newSample(GstAppSink *sink, gpointer gSelf);
    static gboolean onGstBusMessage(GstBus *bus, GstMessage *msg, gpointer user_data);
};

class GstPlayers {
  public:
    GstPlayer* Get(int32_t id, std::vector<std::string> cmd_arguments = {});

    void Dispose(int32_t id);

  private:
    std::mutex mutex_;
    std::map<int32_t, std::unique_ptr<GstPlayer>> players_;
};

static std::unique_ptr<GstPlayers> g_players = std::make_unique<GstPlayers>();

#endif
