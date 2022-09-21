# whpp-play

## OSX

Requirements:
- XCode command line tools installs
- Install additional dependencies using homebrew
- setenv("GST_PLUGIN_PATH","PATH/TO/LIBS",0);

```
brew install gstreamer gst-plugins-good gst-plugins-bad libsoup@2 icu4c cmake gst-libav
```

Build:

```
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
make
```

## Debug

Before gst init add the following to generate .dot graph

```
g_setenv("GST_DEBUG_DUMP_DOT_DIR", "YOUR/TARGET/DIR", 0);
setenv("GST_DEBUG", "4", 0);
```

Generate png from .dot
```
dot -Tpng input.dot > output.png
```
