#define GST_USE_UNSTABLE_API 1 // Removes compile warning

#include "nlohmann/json.hpp"
#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <iostream>
#include <libsoup/soup.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void pad_added_handler(GstElement* src, GstPad* pad, CustomData* data);
void onAnswerCreatedCallback(GstPromise* promise, gpointer userData);
void onRemoteDescSetCallback(GstPromise* promise, gpointer userData);
void onNegotiationNeededCallback(gpointer userData);

void handleSDPs()
{

    GstSDPMessage* offerMessage;
    GstWebRTCSessionDescription* offerDesc;

    if (gst_sdp_message_new_from_text(data.sdpOffer.c_str(), &offerMessage) != GST_SDP_OK) {
        printf("Unable to create SDP object from offer\n");
    }

    offerDesc = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, offerMessage);
    if (!offerDesc) {
        printf("Unable to create SDP object from offer msg\n");
    }

    GstPromise* promiseRemote = gst_promise_new_with_change_func(onRemoteDescSetCallback, nullptr, nullptr);
    g_assert_nonnull(data.webrtc_source);
    g_signal_emit_by_name(data.webrtc_source, "set-remote-description", offerDesc, promiseRemote);
}

void getPostOffer()
{

    SoupSession* session = soup_session_new();
    const char* location;

    SoupMessage* msg = soup_message_new("POST", data.whppURL.c_str());
    if (!msg) {
        printf("ERROR: nullptr msg in getPostOffer()\n");
        exit(EXIT_FAILURE);
    }
    GError* error = nullptr;
    soup_session_send_message(session, msg);

    if (error) {
        printf("Failed to download: %s\n", error->message);
        g_error_free(error);
        g_object_unref(msg);
        g_object_unref(session);
    }

    location = soup_message_headers_get_one(msg->response_headers, "location");
    nlohmann::json responseJson = nlohmann::json::parse(msg->response_body->data);
    data.sdpOffer = responseJson["offer"].get<std::string>();
    std::string str(location);
    data.location = location;

    // Cleanup
    g_object_unref(msg);
    g_object_unref(session);
}

void putAnswer()
{

    SoupSession* session = soup_session_new();
    SoupMessage* msg = soup_message_new("PUT", data.location.c_str());
    if (!msg) {
        printf("ERROR: when creating msg in putAnswer()\n");
        exit(EXIT_FAILURE);
    }
    GError* error = nullptr;
    nlohmann::json req_body;
    const char* sdp = data.sdpAnswer.c_str();
    req_body["answer"] = sdp;

    soup_message_set_request(msg, "application/whpp+json", SOUP_MEMORY_COPY, req_body.dump().c_str(), req_body.dump().length());

    if (error) {
        printf("Failed put generation: %s\n", error->message);
        g_error_free(error);
        g_object_unref(msg);
        g_object_unref(session);
    }

    auto statusCode = soup_session_send_message(session, msg);

    // Cleanup
    g_object_unref(msg);
    g_object_unref(session);

    if (statusCode != 204) {
        printf("%s", "ERROR: ");
    }

    printf("%i \n", statusCode);
}

int32_t main(int32_t argc, char** argv)
{

    if (argc < 2) {
        printf("Usage: ./whpp-play WHPP-URL\n");
        return 1;
    }
    data.whppURL = argv[1];
    getPostOffer();

    gst_init(nullptr, nullptr);
    GMainLoop* mainLoop;

    // Make elements
    data.webrtc_source = gst_element_factory_make("webrtcbin", "source");
    if (!data.webrtc_source) {
        printf("Failed to make element source\n");
        return 1;
    }

    data.sinkElement = gst_element_factory_make("glimagesink", "gli_sink");
    if (!data.sinkElement) {
        printf("Failed to make element gli sink\n");
        return 1;
    }

    data.rtp_depay_vp8 = gst_element_factory_make("rtpvp8depay", "rtp_depayloader_vp8");
    if (!data.rtp_depay_vp8) {
        printf("Failed to make element rtp depayloader\n");
        return 1;
    }

    data.vp8_decoder = gst_element_factory_make("vp8dec", "vp8_decoder");
    if (!data.vp8_decoder) {
        printf("Failed to make element vp8 decoder\n");
        return 1;
    }

    data.pipeline = gst_pipeline_new("test-pipeline");
    if (!data.pipeline) {
        printf("Failed to make element pipeline\n");
        return 1;
    }

    // Add elements
    if (!gst_bin_add(GST_BIN(data.pipeline), data.webrtc_source)) {
        printf("Failed to add element source\n");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(data.pipeline), data.rtp_depay_vp8)) {
        printf("Failed to add element rtp depayloader\n");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(data.pipeline), data.vp8_decoder)) {
        printf("Failed to add element decoder\n");
        return 1;
    }

    if (!gst_bin_add(GST_BIN(data.pipeline), data.sinkElement)) {
        printf("Failed to add element sink\n");
        return 1;
    }

    // Signals
    g_signal_connect(data.webrtc_source, "pad-added", G_CALLBACK(pad_added_handler), &data);
    g_signal_connect(data.webrtc_source, "on-negotiation-needed", G_CALLBACK(onNegotiationNeededCallback), &data);

    // Start playing
    printf("Start playing...\n");
    if (gst_element_set_state(data.pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        printf("Unable to set the pipeline to the playing state.\n");
        return 1;
    }

    printf("main loop...\n");
    mainLoop = g_main_loop_new(nullptr, FALSE);

    // Will loop forever
    printf("Looping...\n");
    g_main_loop_run(mainLoop);

    // Free resources
    g_main_loop_unref(mainLoop);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);
    gst_deinit();
    printf("Returning...\n");
    return 0;
}

void pad_added_handler(GstElement* src, GstPad* new_pad, CustomData* data)
{

    printf("Received new pad '%s' from '%s'\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

    if (!gst_element_link_many(src, data->rtp_depay_vp8, data->vp8_decoder, data->sinkElement, nullptr)) {
        printf("Failed to link source to sink\n");
    }
}

void onNegotiationNeededCallback(gpointer userData)
{

    handleSDPs();
}

void onRemoteDescSetCallback(GstPromise* promise, gpointer userData)
{

    g_assert_cmphex(gst_promise_wait(promise), ==, GST_PROMISE_RESULT_REPLIED);
    gst_promise_unref(promise);

    GstPromise* promiseAnswer = gst_promise_new_with_change_func(onAnswerCreatedCallback, nullptr, nullptr);
    g_signal_emit_by_name(data.webrtc_source, "create-answer", nullptr, promiseAnswer);
}

void onAnswerCreatedCallback(GstPromise* promise, gpointer userData)
{

    GstWebRTCSessionDescription* answerPointer = nullptr;

    g_assert_cmphex(gst_promise_wait(promise), ==, GST_PROMISE_RESULT_REPLIED);
    const GstStructure* reply = gst_promise_get_reply(promise);

    gst_promise_unref(promise);

    gst_structure_get(reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answerPointer, nullptr);
    if (!answerPointer->sdp) {
        printf("ERROR: No answer sdp!\n");
    }

    g_signal_emit_by_name(data.webrtc_source, "set-local-description", answerPointer, nullptr);
    data.sdpAnswer = gst_sdp_message_as_text(answerPointer->sdp);
    putAnswer();
}
