import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'package:flutter_gstreamer_player/flutter_gstreamer_player.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      title: 'Test',
      home: StreamPage(),
    );
  }
}

class StreamPage extends StatefulWidget {
  const StreamPage({Key? key}) : super(key: key);

  @override
  State<StreamPage> createState() => _StreamPageState();
}

class _StreamPageState extends State<StreamPage> {
  String pipeline = "";
  String pipelineJPG =
      '''udpsrc port=53262 buffer-size=65535 ! application/x-rtp,encoding-name=JPEG,payload=26 ! rtpjitterbuffer latency=50 ! rtpjpegdepay ! jpegdec ! videoconvert ! tee name=t t. ! queue ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink sync=FALSE max-buffers=1 drop=TRUE''';
  String pipelineH264Streaming =
      '''udpsrc port=53262 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! avdec_h264 ! tee name=t
         t. ! queue ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink sync=FALSE max-buffers=1 drop=TRUE
         t. ! queue ! x264enc bitrate=1000 tune=zerolatency ! flvmux streamable=true ! rtmpsink name=rtmpsink location="rtmp://" sync=FALSE''';

  String pipelineH264 =
      '''udpsrc port=53262 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink sync=FALSE max-buffers=1 drop=TRUE''';

  @override
  void initState() {
    super.initState();
    pipeline = pipelineH264;
  }

  @override
  void dispose() {
    super.dispose();
  }

  void _enableFullScreen() {
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.manual,
        overlays: []); // Hides all system UI
  }

  void _disableFullScreen() {
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.manual,
        overlays: SystemUiOverlay.values); // Shows all system UI
  }

  @override
  Widget build(BuildContext context) {
    Orientation orientation = MediaQuery.of(context).orientation;
    if (orientation == Orientation.landscape) {
      _enableFullScreen();
    } else {
      _disableFullScreen();
    }
    return Scaffold(
      body: SafeArea(
        child: Stack(
          children: [
            Center(
              child: AspectRatio(
                aspectRatio: 8 / 5,
                child: Stack(
                  children: [
                    GstPlayer(
                      pipeline: pipeline,
                    ),
                  ],
                ),
              ),
            ),
            Row(
              children: [
                ElevatedButton(
                    onPressed: () {
                      setState(() {
                        pipeline = pipelineH264;
                      });
                    },
                    child: Text("h264")),
                const SizedBox(
                  width: 16,
                ),
                ElevatedButton(
                    onPressed: () {
                      setState(() {
                        pipeline = pipelineH264Streaming;
                      });
                    },
                    child: Text("h264RTMP"))
              ],
            )
          ],
        ),
      ),
    );
  }
}
