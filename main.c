#include <glib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>

/*
GMainLoop* mainLoop = NULL;


void intSignalHandler(int32_t code)
{
    g_main_loop_quit(mainLoop);
}
*/
int32_t main(int32_t argc, char **argv) {
   /* {
        struct sigaction sigactionData = {};
        sigactionData.sa_handler = intSignalHandler;
        sigactionData.sa_flags = 0;
        sigemptyset(&sigactionData.sa_mask);
        sigaction(SIGINT, &sigactionData, NULL);
    }*/

    //gstreamer init
    //mainLoop = g_main_loop_new(NULL, FALSE);
    //g_main_loop_run(mainLoop);




    const char url[1024] = "https://broadcaster.lab.sto.eyevinn.technology:8443/broadcaster/channel/sthlm";
    //const char req_content_type = "application/json"

    SoupSession *session = soup_session_new ();
    SoupMessageHeaders *response_headers;
    const char *content_type;
    goffset content_length;
    char content[4096];
    SoupMessage *msg = soup_message_new (SOUP_METHOD_POST, url);
    GError *error = NULL;
      GInputStream *in_stream = soup_session_send (
        session,
        msg,
        NULL,
        &error);

     /*GBytes *bytes = soup_session_send_and_read (
        session,
        msg,
        NULL, // Pass a GCancellable here if you want to cancel a download
        &error);*/

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
    g_input_stream_read(in_stream, content, 1024, NULL, NULL);
    g_print(content);

    printf("Receiving:");
    g_print("%s", content_type);
    g_print("%i", content_length);
    //g_print(msg);
    
    //g_bytes_unref (bytes);
    g_object_unref (in_stream);
    g_object_unref (msg);
    g_object_unref (session);

    return 0;

}



