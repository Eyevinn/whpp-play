# Player for WebRTC HTTP Playback Protocol

Video player for WebRTC streams using WHPP as SDP exchange protocol

## OSX

Requirements:
- XCode command line tools installs
- Install additional dependencies using homebrew

```
brew install gstreamer gst-plugins-good gst-plugins-bad libsoup@2 icu4c cmake gst-libav
```

Build:

```
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
make
```

Run:

```
GST_PLUGIN_PATH=/opt/homebrew/lib/gstreamer-1.0 ./whpp-play <WHPP URL>
```

## Debug

Run with the following environment variables set:

```
GST_DEBUG_DUMP_DOT_DIR=<target-dir> \
GST_DEBUG=4 \
GST_PLUGIN_PATH=/opt/homebrew/lib/gstreamer-1.0 \
./whpp-play <WHPP URL>
```

Generate png from .dot
```
dot -Tpng input.dot > output.png
```
