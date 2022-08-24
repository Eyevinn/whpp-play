# whpp-play

## OSX

Requirements:
- XCode command line tools installs
- Install additional dependencies using homebrew

```
brew install gstreamer gst-plugins-good gst-plugins-bad curl cmake gst-libav
```

Build:

```
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
make
```
