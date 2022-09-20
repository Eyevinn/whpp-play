#define GST_USE_UNSTABLE_API 1 //Removes compile warning

#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <libsoup/soup.h>
#include "nlohmann/json.hpp"

typedef struct _CustomData {
    GstElement* webrtc_source;
    GstElement* pipeline;
    GstElement* rtp_depay_vp8;
    GstElement* vp8_decoder;
    GstElement* sinkElement;
    std::string sdpOffer;
    std::string sdpAnswer;
    std::string location;
    std::string whppURL;
} CustomData;
CustomData data;

//const char* url = "https://broadcaster.lab.sto.eyevinn.technology:8443/broadcaster/channel/sthlm";

static void pad_added_handler (GstElement* src, GstPad* pad, CustomData* data);
static void onAnswerCreatedCallback(GstPromise* promise, gpointer userData);
static void onRemoteDescSetCallback(GstPromise* promise, gpointer userData);
static void onNegotiationNeededCallback(gpointer userData);

static void handleSDPs () {

    GstSDPMessage* offerMessage;
    GstWebRTCSessionDescription* offerDesc;

    if (gst_sdp_message_new_from_text (data.sdpOffer.c_str(), &offerMessage) != GST_SDP_OK)
        {
            g_print("Unable to create SDP object from offer\n");
        }

    offerDesc = gst_webrtc_session_description_new (GST_WEBRTC_SDP_TYPE_OFFER, offerMessage);
        if (!offerDesc)
        {
            g_print("Unable to create SDP object from offer msg\n");
        }

    GstPromise* promiseRemote = gst_promise_new_with_change_func (onRemoteDescSetCallback, NULL, NULL);
    g_assert_nonnull (data.webrtc_source);
    g_signal_emit_by_name (data.webrtc_source, "set-remote-description", offerDesc, promiseRemote);

}

static void getPostOffer(){

    SoupSession* session = soup_session_new ();
    const char* location;

    SoupMessage* msg = soup_message_new ("POST", data.whppURL.c_str());
    if(!msg){
        g_print("ERROR: NULL msg in getPostOffer()\n");
        exit (EXIT_FAILURE);
    }
    GError* error = NULL;
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
    std::string str(location);
    data.location = location;

    //Cleanup
    g_object_unref (msg);
    g_object_unref (session);

}

static void putAnswer() {

    SoupSession* session = soup_session_new ();
    SoupMessage* msg = soup_message_new ("PUT", data.location.c_str());
    if (!msg)
    {
        g_print("ERROR: when creating msg in putAnswer()\n");
        exit (EXIT_FAILURE);
    }
    GError* error = NULL;
    nlohmann::json req_body = "{}"_json;
    const char* sdp = data.sdpAnswer.c_str();

    req_body.push_back(nlohmann::json::object_t::value_type("answer", sdp));

    soup_message_set_request (msg , "application/whpp+json", SOUP_MEMORY_COPY, req_body.dump().c_str(), req_body.dump().length());
   
    if (error) {
        g_printerr ("Failed put generation: %s\n", error->message);
        g_error_free (error);
        g_object_unref (msg);
        g_object_unref (session);
    }

    auto statusCode = soup_session_send_message(session, msg);

    //Cleanup
    g_object_unref (msg);
    g_object_unref (session);

    if (statusCode != 204)
    {
        g_print("%s", "ERROR: ");
    }

    g_print("%i \n", statusCode);

}

int32_t main(int32_t argc, char **argv) {

    if (argc < 2) { 
        g_print("Usage: ./whpp-play WHPP-URL\n");
        return 1;
    }
    data.whppURL = argv[1];
    getPostOffer();

    gst_init(NULL, NULL);
    GMainLoop* mainLoop;

    //Make elements
    data.webrtc_source = gst_element_factory_make ("webrtcbin", "source");
    if (!data.webrtc_source) {
        g_print("Failed to make element source\n");
        return 1;
    }   
    
    data.sinkElement = gst_element_factory_make ("glimagesink", "gli_sink");
    if (!data.sinkElement) {
        g_print("Failed to make element gli sink\n");
        return 1;
    }

    data.rtp_depay_vp8 = gst_element_factory_make ("rtpvp8depay", "rtp_depayloader_vp8");
    if (!data.rtp_depay_vp8) {
        g_print("Failed to make element rtp depayloader\n");
        return 1;
    }

    data.vp8_decoder = gst_element_factory_make ("vp8dec", "vp8_decoder");
    if (!data.vp8_decoder) {
        g_print("Failed to make element vp8 decoder\n");
        return 1;
    }
  
    data.pipeline = gst_pipeline_new ("test-pipeline");
    if (!data.pipeline) {
        g_print("Failed to make element pipeline\n");
        return 1;
    }

    //Add elements
    if (!gst_bin_add(GST_BIN(data.pipeline), data.webrtc_source)){
        g_print("Failed to add element source\n");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(data.pipeline), data.rtp_depay_vp8)) {
        g_print("Failed to add element rtp depayloader\n");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(data.pipeline), data.vp8_decoder)) {
        g_print("Failed to add element decoder\n");
        return 1;
    }


    if (!gst_bin_add(GST_BIN(data.pipeline), data.sinkElement)) {
        g_print("Failed to add element sink\n");
        return 1;
    }

    //Signals
    g_signal_connect (data.webrtc_source, "pad-added", G_CALLBACK (pad_added_handler), &data);
    g_signal_connect(data.webrtc_source, "on-negotiation-needed", G_CALLBACK(onNegotiationNeededCallback), &data);
    
    //Start playing
    g_print ("Start playing...\n");
    if (gst_element_set_state(data.pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
            printf("Unable to set the pipeline to the playing state.\n");
            return 1;
        }

    g_print ("main loop...\n");
    mainLoop = g_main_loop_new(NULL, FALSE);

    //Will loop forever
    g_print ("Looping...\n");
    g_main_loop_run(mainLoop);
    
    //Free resources 
    g_main_loop_unref(mainLoop);
    gst_element_set_state (data.pipeline, GST_STATE_NULL);
    gst_object_unref (data.pipeline);
    gst_deinit();
    g_print("Returning...\n");
    return 0;

}

static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
        
    g_print ("Received new pad '%s' from '%s'\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

    if (!gst_element_link_many(src, data->rtp_depay_vp8, data->vp8_decoder, data->sinkElement, nullptr)) {
            printf("Failed to link source to sink\n");
    }
}

static void onNegotiationNeededCallback (gpointer userData) {

    handleSDPs();

}

static void onRemoteDescSetCallback(GstPromise* promise, gpointer userData) {

    g_assert_cmphex (gst_promise_wait (promise), ==, GST_PROMISE_RESULT_REPLIED);
    gst_promise_unref(promise);
    
    GstPromise* promiseAnswer = gst_promise_new_with_change_func(onAnswerCreatedCallback, NULL, NULL);
    g_signal_emit_by_name(data.webrtc_source , "create-answer", NULL, promiseAnswer);

}

void onAnswerCreatedCallback (GstPromise* promise, gpointer userData) {

    GstWebRTCSessionDescription* answerPointer = NULL;
    const GstStructure* reply;

    g_assert_cmphex (gst_promise_wait (promise), ==, GST_PROMISE_RESULT_REPLIED);
    reply = gst_promise_get_reply(promise);

    gst_promise_unref(promise);

    gst_structure_get (reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answerPointer, NULL);
    if (!answerPointer->sdp) {
        g_print("ERROR: No answer sdp!   \n");
    }
    
    g_signal_emit_by_name(data.webrtc_source, "set-local-description", answerPointer, NULL);
    data.sdpAnswer = gst_sdp_message_as_text (answerPointer->sdp);
    putAnswer();
    
}


