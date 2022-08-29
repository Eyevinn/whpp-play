#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>


//GMainLoop* mainLoop = NULL;

/*
void intSignalHandler(int32_t code)
{
    g_main_loop_quit(mainLoop);
}
*/


int32_t main(int32_t argc, char **argv) {
    /*{
        struct sigaction sigactionData = {};
        sigactionData.sa_handler = intSignalHandler;
        sigactionData.sa_flags = 0;
        sigemptyset(&sigactionData.sa_mask);
        sigaction(SIGINT, &sigactionData, NULL);
    }
*/
    //gstreamer init
    //mainLoop = g_main_loop_new(NULL, FALSE);
    //g_main_loop_run(mainLoop);


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
        return 1;
    }

    response_headers = soup_message_get_response_headers (msg);
    content_type = soup_message_headers_get_content_type (response_headers, NULL);
    content_length = soup_message_headers_get_content_length (response_headers);
    g_input_stream_read_all (in_stream, content, 14336, readBytes, NULL, NULL);
    location = soup_message_headers_get_one (response_headers, "location");

    //Get offer value
    char *textoffer;
    textoffer = strtok(content, ":");
    textoffer = strtok(NULL, ",");
    g_strchomp(textoffer);
    //g_print("%s", textoffer);
    //g_print("%s", content);
    //g_print("%s", location);
    //g_print("%s", content_type);
    //g_print("%i", content_length);

    //Cleanup
    g_object_unref (in_stream);
    g_object_unref (msg);
    g_object_unref (session);


    //on_offer_received

    //sdp generation should be after setting remote and local and remote description
    /*FIRST DRAFT OF SDP GENRATION WITH SUCCESS ASSERTION
    int ret;
    GstSDPMessage *sdp;
    ret = gst_sdp_message_new (&sdp);
    g_assert_cmphex (ret, ==, GST_SDP_OK);
    ret = gst_sdp_message_parse_buffer ((guint8 *) textoffer, strlen (textoffer), sdp);
    g_assert_cmphex (ret, ==, GST_SDP_OK);
    g_print("%s", sdp);

    GstWebRTCSessionDescription *offer = NULL;
    offer = gst_webrtc_session_description_new (GST_WEBRTC_SDP_TYPE_OFFER, sdp);
    g_assert_nonnull (offer);
    */

    printf("Returning now...");
    return 0;

}



