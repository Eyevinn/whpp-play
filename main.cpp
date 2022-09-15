#define GST_USE_UNSTABLE_API 1 //Removes compile warning

#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>
#include "nlohmann/json.hpp"

static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
  "src_%u",
  GST_PAD_SRC,
  GST_PAD_SOMETIMES,
  GST_STATIC_CAPS ("ANY")
);

typedef struct _CustomData {
    GstElement* webrtc_source;
    GstElement* pipeline;
    GstElement* fakeSinkElement;
    GstPad* source_pad;
    std::string sdpOffer;
    std::string sdpAnswer;
    std::string location;
} CustomData;
CustomData data;

static void pad_added_handler (GstElement* src, GstPad *pad, CustomData *data);
static void onAnswerCreatedCallback(GstPromise* promise, gpointer userData);
static void onRemoteDescSetCallback(GstPromise* promise, gpointer userData);
static void onNegotiationNeededCallback(gpointer userData);

static void handleSDPs () {

    //Set remote description as received sdp offer
    g_print("Handling SDPs... ");

    GstSDPMessage* offerMessage;
    GstSDPMessage* answerMessage;
    GstWebRTCSessionDescription* offerDesc;
    GstWebRTCSessionDescription* answerDesc;

    if (gst_sdp_message_new_from_text (data.sdpOffer.c_str(), &offerMessage) != GST_SDP_OK)
        {
            g_print("Unable to create SDP object from offer");
        }

    /**/offerDesc = gst_webrtc_session_description_new (GST_WEBRTC_SDP_TYPE_OFFER, offerMessage);
        if (!offerDesc)
        {
            g_print("Unable to create SDP object from offer msg");
        }


    GstPromise* promiseRemote = gst_promise_new_with_change_func (onRemoteDescSetCallback, NULL, NULL);
    g_assert_nonnull (data.webrtc_source);
    g_signal_emit_by_name (data.webrtc_source, "set-remote-description", offerDesc, promiseRemote);

}


static void getPostOffer(){

const char url[1024] = "https://broadcaster.lab.sto.eyevinn.technology:8443/broadcaster/channel/sthlm"; 

    SoupSession *session = soup_session_new ();
    const char *location;

    gsize readBytes;
    SoupMessage *msg = soup_message_new (SOUP_METHOD_POST, url);
    GError *error = NULL;
    soup_session_send_message(session, msg);

if (error) {
        g_printerr ("Failed to download: %s\n", error->message);
        g_error_free (error);
        g_object_unref (msg);
        g_object_unref (session);
    }

    location = soup_message_headers_get_one (msg->response_headers, "location");

    nlohmann::json responseJson = nlohmann::json::parse(msg->response_body->data);
    data.sdpOffer = responseJson["offer"].get<std::string>();
//    //Cleanup
    g_object_unref (msg);
    g_object_unref (session);

    data.location = location;

}


static void putAnswer () {

    g_print("empty");

}


int32_t main(int32_t argc, char **argv) {

    //Set env
    //Dump graph .dot
    g_setenv("GST_DEBUG_DUMP_DOT_DIR", "/Users/olivershin/Documents/", 0);
    //setenv("GST_DEBUG", "6", 0);
    setenv("GST_PLUGIN_PATH","/usr/local/lib/gstreamer-1.0", 0);
    getPostOffer();
    //g_print("%s", data.sdpOffer);
    //g_print("%s", data.location);

    gst_init(NULL, NULL);
    GMainLoop* mainLoop;

    //Make elements
    data.webrtc_source = gst_element_factory_make ("webrtcbin", "source");
    if (!data.webrtc_source) {
        g_print("Failed to make element source");
        return 1;
    }   
    
    data.fakeSinkElement = gst_element_factory_make ("fakesink", "sink");
    if (!data.fakeSinkElement) {
        g_print("Failed to make element sink");
        return 1;
    }
    
    data.pipeline = gst_pipeline_new ("test-pipeline");
    if (!data.pipeline) {
        g_print("Failed to make element pipeline");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(data.pipeline), data.webrtc_source)){
        g_print("Failed to add element source");
        return 1;
    }
    
    if (!gst_bin_add(GST_BIN(data.pipeline), data.fakeSinkElement)) {
        g_print("Failed to add element sink");
        return 1;
    }
    
    g_print ("Connecting... ");
    //Signals
    g_signal_connect (data.webrtc_source, "pad-added", G_CALLBACK (pad_added_handler), &data);
    g_signal_connect(data.webrtc_source, "on-negotiation-needed", G_CALLBACK(onNegotiationNeededCallback), &data);
    
    //Create pads
    data.source_pad = gst_pad_new_from_static_template (&src_factory, "source_pad");
    
    //gst_element_add_pad emits pad_added signal
    if (!gst_element_add_pad (data.webrtc_source, data.source_pad)) {
        g_print("Failed to add pad to source");
        return 1;
    }
    
    /* Start playing */
    g_print ("Start playing... ");
    if (gst_element_set_state(data.pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
            printf("Unable to set the pipeline to the playing state.\n");
            return 1;
        }

    g_print ("main loop... ");
    mainLoop = g_main_loop_new(NULL, FALSE);
    //TO EXPORT RUN IN TERMINAL//dot -Tpng pipeline.dot -o graf.png
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

    // Will loop forever
    g_print ("Looping... ");
    g_main_loop_run(mainLoop);
    
   /* Free resources */
    g_main_loop_unref(mainLoop);
    gst_element_set_state (data.pipeline, GST_STATE_NULL);
    gst_object_unref (data.pipeline);
    gst_deinit();
    g_print("Returning... ");
    return 0;

}

static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
    
  g_print("pad handler callback... ");
  GstPad *sink_pad = gst_element_get_static_pad (data->fakeSinkElement, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps;
  //GstStructure *new_pad_struct;
  const gchar *new_pad_type;
    
  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps (new_pad);
  //new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  //new_pad_type = gst_structure_get_name (new_pad_struct);

  /* Attempt the link */
  if (gst_pad_can_link (new_pad, sink_pad)) {
        g_print("Compatible pads... ");
    }
    
  //ret = gst_pad_link (new_pad, sink_pad);
  ret = static_cast<GstPadLinkReturn>(gst_element_link_pads(data->webrtc_source, GST_PAD_NAME (new_pad), data->fakeSinkElement, GST_PAD_NAME (sink_pad)));
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("Link failed.\n");
  } else {
    g_print ("Pad link succeeded.\n");
  }
    
}

static void onNegotiationNeededCallback (gpointer userData) {

    g_print("Negotiation needed callback... ");
    handleSDPs();

}

static void onRemoteDescSetCallback(GstPromise* promise, gpointer userData) {

    g_print("Set Remote description callback triggered... ");
    //Create answer by calling g_signal emit "create answer", receive callback response which contains answer
    g_assert_cmphex (gst_promise_wait (promise), ==, GST_PROMISE_RESULT_REPLIED);
    gst_promise_unref(promise);
    

    /*TEST print remote sdp*/
    
    const gchar* message;
    GstWebRTCSessionDescription* remote = NULL;

    g_object_get (data.webrtc_source, "remote-description", &remote, NULL);
    if(!remote->sdp){
        g_print("ERROR: remote sdp not set  ");
    }
    
    else {
        message = gst_sdp_message_as_text (remote->sdp);
        g_print("%s", message);
    }
    


    g_print("Promising... ");
    GstPromise* promiseAnswer = gst_promise_new_with_change_func(onAnswerCreatedCallback, NULL, NULL);
    g_print("Create answer... ");
    g_signal_emit_by_name(data.webrtc_source , "create-answer", NULL, promiseAnswer);

}

void onAnswerCreatedCallback (GstPromise* promise, gpointer userData) {

    GstWebRTCSessionDescription* answerPointer = NULL;
    //GstWebRTCSessionDescription* remote = NULL;
    const GstStructure* reply;
    const gchar* message;

    g_print("onAnswerCallback... ");

/*
    g_object_get (data.webrtc_source, "remote-description", &remote, NULL);
    if(!remote->sdp){
        g_print("ERROR: remote sdp not set  ");
    }



    else {
        message = gst_sdp_message_as_text (remote->sdp);
        g_print("%s", message);
    }
*/


    g_assert_cmphex (gst_promise_wait (promise), ==, GST_PROMISE_RESULT_REPLIED);
    //reply struct has no member answer
    reply = gst_promise_get_reply(promise);

    gst_promise_unref(promise);
    //gst_structure_to_string (reply);

    gst_structure_get (reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answerPointer, NULL);
    if (!answerPointer->sdp) {
        g_print("ERROR: No answer sdp!   ");
    }
    
    message = gst_sdp_message_as_text (answerPointer->sdp);
    g_print("%s", message);
    g_signal_emit_by_name(data.webrtc_source, "set-local-description", answerPointer, NULL);
    

    }


