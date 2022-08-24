#include <glib.h>

GMainLoop* mainLoop = NULL;

void intSignalHandler(int32_t code)
{
    g_main_loop_quit(mainLoop);
}

int32_t main(int32_t argc, char **argv) {
    {
        struct sigaction sigactionData = {};
        sigactionData.sa_handler = intSignalHandler;
        sigactionData.sa_flags = 0;
        sigemptyset(&sigactionData.sa_mask);
        sigaction(SIGINT, &sigactionData, NULL);
    }

    mainLoop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainLoop);
}