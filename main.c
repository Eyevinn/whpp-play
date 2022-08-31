#define GST_USE_UNSTABLE_API 1 //Removes compile warning

#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>

//FLOW
//Create graph pipeline
//gst_element_factory_make()
//gst_element_link_many()
//Set / get properties
//g_signal_connect -- perhaps here onnegotiationneeded callback

//webrtc -> fakesink



/*
void getPostOffer(){

 const char url[1024] = "https://broadcaster.lab.sto.eyevinn.technology:8443/broadcaster/channel/sthlm";

    SoupSession *session = soup_session_new ();
    SoupMessageHeaders *response_headers;
    const char *content_type;
    const char *location;
    goffset content_length;
    const char content[15360];

    int readBytes;
    SoupMessage *msg = soup_message_new (SOUP_METHOD_POST, url);
    GError *error = NULL;
    GInputStream *in_stream = soup_session_send (session, msg, NULL, &error);

if (error) {
        g_printerr ("Failed to download: %s\n", error->message);
        g_error_free (error);
        g_object_unref (msg);
        g_object_unref (session);
    }

    response_headers = soup_message_get_response_headers (msg);
    content_type = soup_message_headers_get_content_type (response_headers, NULL);
    content_length = soup_message_headers_get_content_length (response_headers);
    g_input_stream_read_all (in_stream, content, 14336, readBytes, NULL, NULL);
    location = soup_message_headers_get_one (response_headers, "location");

    //Get offer value
    gchar *textoffer;
    textoffer = strtok(content, ":");
    textoffer = strtok(NULL, ",");
    textoffer++;
    textoffer[strlen(textoffer)-1] = 0;
    g_strchomp(textoffer);
    g_print("%s", textoffer);
    //g_print("%s", content);
    //g_print("%s", location);
    //g_print("%s", content_type);
    //g_print("%i", content_length);

    //Cleanup
    g_object_unref (in_stream);
    g_object_unref (msg);
    g_object_unref (session);

}
*/


typedef struct _CustomData {
    GstElement* source;
    GstElement* pipeline;
    GstElement* fakeSinkElement;
} CustomData;

static void on_negotiation_needed_handler (GstElement *src, GstPad *pad, CustomData *data);

int32_t main(int32_t argc, char **argv) {

    gst_init(NULL, NULL);

    GMainLoop* mainLoop;
    CustomData data;
    
    data.source = gst_element_factory_make ("webrtcbin", "source");
    if (!data.source) {
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

    if (!gst_bin_add(GST_BIN(data.pipeline), data.source)){
        g_print("Failed to add element source");
        return 1;
    }
    
    if (!gst_bin_add(GST_BIN(data.pipeline), data.fakeSinkElement)) {
        g_print("Failed to add element sink");
        return 1;
    }
    
    /* Connect to the pad-added signal */
    g_print ("Connecting... ");
    g_signal_connect (data.source, "on_negotiation_needed", G_CALLBACK (on_negotiation_needed_handler), &data);

 
    /* Start playing */
    g_print ("Start playing... ");
    gst_element_set_state (data.pipeline, GST_STATE_PLAYING);

    g_print ("main loop... ");
    mainLoop = g_main_loop_new(NULL, FALSE);

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

/* This function will be called by the pad-added signal */
static void on_negotiation_needed_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
  GstPad *sink_pad = gst_element_get_static_pad (data->fakeSinkElement, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;
    
    g_print ("Callback!");
    
g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));
    
    /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps (new_pad);
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);
    
    ret = gst_pad_link (new_pad, sink_pad);
    
    if (GST_PAD_LINK_FAILED (ret)) {
       g_print ("Type is '%s' but link failed.\n", new_pad_type);
     } else {
       g_print ("Link succeeded (type '%s').\n", new_pad_type);
     }
    
    gst_caps_unref (new_pad_caps);
    gst_object_unref (sink_pad);
    
}

