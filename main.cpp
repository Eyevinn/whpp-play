#include <gst/gst.h>
#include <cstdio>

#include <glib.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <libsoup/soup.h>

GstElement* pipeline = nullptr;
GstElement* fakeSrcElement = nullptr;
GstElement* fakeSinkElement = nullptr;
GstElement* webrtc = nullptr;
GMainLoop* mainLoop = nullptr;

int main() {
    gst_init(nullptr, nullptr);

    pipeline = gst_pipeline_new("gstdemo-pipeline");
    if (!pipeline) {
        printf("Failed to create pipeline\n");
        return 1;
    }

    webrtc = gst_element_factory_make ("webrtcbin", "Src");
    if (!webrtc) {
        printf("Failed to create webrtcbin\n");
        return 1;
    }

    fakeSrcElement = gst_element_factory_make("fakesrc", "myFakeSrc");
    if (!fakeSrcElement) {
        printf("Failed to create fakeSrcElement\n");
        return 1;
    }

    fakeSinkElement = gst_element_factory_make("fakesink", "myFakeSink");
    if (!fakeSinkElement) {
        printf("Failed to create fakeSinkElement\n");
        return 1;
    }
    

    if (!gst_bin_add(GST_BIN(pipeline), fakeSrcElement)) {
        printf("Failed to add fakeSrcElement\n");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(pipeline), fakeSinkElement)) {
        printf("Failed to add fakeSinkElement\n");
        return 1;
    }

    if (!gst_element_link_many(fakeSrcElement, fakeSinkElement, nullptr)) {
        printf("Failed to link\n");
        return 1;
    }

    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        printf("Unable to set the pipeline to the playing state.\n");
        return 1;
    }


    mainLoop = g_main_loop_new(nullptr, FALSE);

    // Will loop forever
    g_main_loop_run(mainLoop);

    g_main_loop_unref(mainLoop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_deinit();

    return 0;
}