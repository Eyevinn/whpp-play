#define GST_USE_UNSTABLE_API 1 //Removes compile warning

#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>

/*
void getPostOffer(){

const char url[1024] = "https://broadcaster.lab.sto.eyevinn.technology:8443/broadcaster/channel/sthlm"; //OLD

//https://wrtc-edge.lab.sto.eyevinn.technology:8443/whpp/channel/sthlm

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


//gst-launch-1.0 webrtcbin ! rtph264depay ! fakesink
//GST_DEBUG=5 gst-launch-1.0 webrtcbin name=webrtcbin stun-server=stun:stun.l.google.com:19302 ! queue ! fakesink &> dump.txt


//FLOW
//Create graph pipeline
//gst_element_factory_make()
//gst_element_link_many()
//Set / get properties
//g_signal_connect -- perhaps here onnegotiationneeded callback

//webrtc -> fakesink

static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
  "src_%u",
  GST_PAD_SRC,
  GST_PAD_SOMETIMES,
  GST_STATIC_CAPS ("ANY")
);

typedef struct _CustomData {
    GstElement* source;
    GstElement* pipeline;
    GstElement* fakeSinkElement;
    GstPad *source_pad;
} CustomData;


//static void on_negotiation_needed_handler (GstElement *src, CustomData *data);
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);

int32_t main(int32_t argc, char **argv) {
    
    gst_init(NULL, NULL);
    
    if (!gst_debug_is_active()) {
        gst_debug_set_active(TRUE);
        GstDebugLevel dbglevel = 5;
        if (dbglevel < GST_LEVEL_ERROR) {
            dbglevel = GST_LEVEL_ERROR;
            gst_debug_set_default_threshold(dbglevel);
        }
    }


    GMainLoop* mainLoop;
    CustomData data;
    GstWebRTCSessionDescription* sdp_dummy;
    GstSDPMessage *sdp_msg;
    
    gst_sdp_message_new (&sdp_msg);
    sdp_dummy = gst_webrtc_session_description_new (GST_WEBRTC_SDP_TYPE_ANSWER, sdp_msg);
    if (!sdp_dummy) {
        g_print("Failed to create sdp dummy");
        return 1;
    }
    g_print("%s", gst_sdp_message_as_text(sdp_dummy->sdp));
    
   
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
    
    g_print ("Connecting... ");
    g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);
    g_signal_emit_by_name(data.source, "set-remote-description", sdp_dummy, NULL);
    
    //Create pads
    data.source_pad = gst_pad_new_from_static_template (&src_factory, "source_pad_2");
    //data.source_pad = gst_pad_new("source_pad", GST_PAD_SRC);
    
    //gst_element_add_pad emits pad_added signal
    if (!gst_element_add_pad (data.source, data.source_pad)) {
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
  ret = gst_element_link_pads(data->source, GST_PAD_NAME (new_pad), data->fakeSinkElement, GST_PAD_NAME (sink_pad));
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print ("Pad link succeeded (type '%s').\n", new_pad_type);
  }
    
    //Link elements should not be needed if pad links are successful ?
    /*FROM DOC: Elements can be linked through their pads. If the link is straightforward, use the gst_element_link convenience function to link two elements, or gst_element_link_many for more elements in a row. Use gst_element_link_filtered to link two elements constrained by a specified set of GstCaps. For finer control, use gst_element_link_pads and gst_element_link_pads_filtered to specify the pads to link on each element by name.*/
   
    //Try to link elements
    /*
    if (!gst_element_link_many(data->source, data->fakeSinkElement, NULL)) {
            printf("Failed to link elements\n");
            //return 1;
        }
     */
}



