import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';

import 'package:flutter_gstreamer_player/flutter_gstreamer_player.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  MyApp({Key? key}) : super(key: key);

  String rtmp = "rtmp://";

  Widget initMultipleGstPlayer(List<String> pipelines) {
    // Try this: https://stackoverflow.com/a/66421214
    List<Widget> videoListWidget = [];
    for (var i = 0; i < pipelines.length - 1; i = i + 2){
      videoListWidget.add(
        Expanded(
          child: Row(
            children: <Widget> [
              Expanded(
                child: GstPlayer(
                  pipeline: pipelines[i],
                  rtmp: rtmp,
                ),
              ),
              Expanded(
                child: GstPlayer(
                  pipeline: pipelines[i+1],
                  rtmp: rtmp,
                ),
              ),
            ],
          ),
        ),
      );
    }

    if (pipelines.length.isOdd) {
      videoListWidget.add(
        Expanded(
          child: Row(
            children: <Widget> [
              Expanded(
                child: GstPlayer(
                  pipeline: pipelines.last,
                  rtmp: rtmp,
                ),
              ),
            ],
          ),
        ),
      );
    }

    Widget multipleGstPlayer = Column(
          children: videoListWidget,
        );

    return multipleGstPlayer;
  }

  @override
  Widget build(BuildContext context) {
    List<String> pipelines = [];

    switch (defaultTargetPlatform) {
      case (TargetPlatform.linux):
      case (TargetPlatform.android):
        pipelines.addAll([
          //'''udpsrc port=53262 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink sync=FALSE max-buffers=1 drop=TRUE''',
    '''udpsrc port=53262 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! avdec_h264 ! tee name=t "
        "t. ! queue ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink sync=FALSE max-buffers=1 drop=TRUE "
        "t. ! queue ! x264enc bitrate=500 tune=zerolatency ! flvmux streamable=true ! rtmpsink name=rtmpsink location=\"rtmp://your_rtmp_server\"'''
        ]);
        break;
      case (TargetPlatform.iOS):
        pipelines.addAll([
          '''udpsrc port=53262 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink sync=FALSE max-buffers=1 drop=TRUE''',
        ]);
        break;
      default:
        break;
    }

    Widget multipleGstPlayer = initMultipleGstPlayer(pipelines);

    return MaterialApp(
      home: Scaffold(
        body: multipleGstPlayer,
      ),
    );
  }
}
