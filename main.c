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

    SoupSession *session = soup_session_new ();
    SoupMessageHeaders *response_headers;
    const char *content_type;
    const char *location;
    goffset content_length;
    char content[15360];
    char htype;

    int readBytes;
    SoupMessage *msg = soup_message_new (SOUP_METHOD_POST, url);
    GError *error = NULL;
      GInputStream *in_stream = soup_session_send (
        session,
        msg,
        NULL,
        &error);

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

    //g_print(content);
    g_print(location);
    //g_print("%s", content_type);
    //g_print("%i", content_length);

    g_object_unref (in_stream);
    g_object_unref (msg);
    g_object_unref (session);


    printf("Returning now...");
    return 0;

}



