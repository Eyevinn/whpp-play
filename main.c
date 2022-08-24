#include <glib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
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
    handshake();
}
*/

char *Hbuffer[1024];
char *location[1024];
char *sdp[4096];
char *answer[5004];
char *rURL[1024];


void resOfferToSdpAnswer(char *res_str)
 {
    char *token = strtok(res_str, ":");
    token = strtok(NULL, ",");
    strcpy(sdp, token);
    strcpy(answer, "{\"answer\":");
    strcat(answer, sdp);
    strcat(answer, "}");
    
 }

struct memory {
   char *response;
   size_t size;
 };


static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
 {
   size_t realsize = size * nmemb;
   struct memory *mem = (struct memory *)userp;
 
   char *ptr = realloc(mem->response, mem->size + realsize + 1);
   if(ptr == NULL)
     return 0;  /* out of memory! */
 
   mem->response = ptr;
   memcpy(&(mem->response[mem->size]), data, realsize);
   mem->size += realsize;
   mem->response[mem->size] = 0;
   
    //printf(ptr);
 
   return realsize;
 }

struct memory chunk = {0};

static size_t header_callback(char *buffer, size_t size,
                              size_t nitems, void *userdata)
{
    /*This callback function must return the number of bytes actually taken care of. If that amount differs from the amount passed in to your function, it will signal an error to the library. This will cause the transfer to get aborted and the libcurl function in progress will return CURLE_WRITE_ERROR.*/
    
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    size_t numbytes = size * nitems;
    
    //Split buffer
    char *token = strtok(buffer, ":");
    
    //If location header, get location address
    if (strcmp(token,"location") == 0) {
        token = strtok(NULL, " ");
        //Save location address to buffer
        strcpy(Hbuffer, token);

    };
        return numbytes;
}

int
main (int argc, char *argv[])
{
    
    CURL *curl;
    CURLcode response;
    
    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Content-Type: application/json");
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    
    if (curl) {
 
        //Setup POST request
        curl_easy_setopt(curl, CURLOPT_URL,"https://broadcaster.lab.sto.eyevinn.technology:8443/broadcaster/channel/sthlm");
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
         /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        response = curl_easy_perform(curl);
        
        char *location = Hbuffer;
        g_strchomp(location);
        char *body = chunk.response;
        resOfferToSdpAnswer(body);
        printf(location);
        //printf(body);
        //printf("%s", answer);

        if(response != CURLE_OK){
              fprintf(stderr, "curl_easy_perform() failed: %s\n",
                      curl_easy_strerror(response));
        }

        curl_easy_reset(curl);

        //Setup PUT request
        curl_easy_setopt(curl, CURLOPT_URL, location);
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, answer);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        response = curl_easy_perform(curl);

        if(response != CURLE_OK){
              fprintf(stderr, "curl_easy_perform() failed: %s\n",
                      curl_easy_strerror(response));
        }
        
        curl_easy_cleanup(curl);
    };
    
    curl_global_cleanup();
    
  return 0;
}


