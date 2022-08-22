#include <curl/curl.h>
#include <stdio.h>

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
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        response = curl_easy_perform(curl);
        
        if(response != CURLE_OK)
              fprintf(stderr, "curl_easy_perform() failed: %s\n",
                      curl_easy_strerror(response));
        
        
        curl_easy_cleanup(curl);
    };
    
    curl_global_cleanup();
        
    
  return 0;
}
