#include <cstdio>
#include <cstdlib>
#include <string>
#include <curl/curl.h>

using std::string;

struct MemoryStruct {
  char *memory;
  size_t size;
};

template <typename D>
int invoke_webhook(const char*, const D*);

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  auto *p = (MemoryStruct *)userp;

  /* expand buffer */
  p->memory = static_cast<char *>(realloc(p->memory, p->size + realsize + 1));

  /* check buffer */
  if(p->memory == nullptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  /* copy contents to buffer */
  memcpy(&(p->memory[p->size]), contents, realsize);

  /* set new buffer size */
  p->size += realsize;

  /* ensure null termination */
  p->memory[p->size] = 0;

  /* return size */
  return realsize;
}

int main() {
  invoke_webhook("https://localhost:8887/api/hook", R"anydelim( {"msg" : "there"} )anydelim");
  invoke_webhook("https://localhost:8887/api/hook", R"anydelim( {"msg" : "there"} )anydelim");
  invoke_webhook("https://localhost:8887/api/hook", R"anydelim( {"msg" : "there"} )anydelim");
  invoke_webhook("https://localhost:8887/api/hook", R"anydelim( {"msg" : "there"} )anydelim");

  return 0;
}

template <typename D>
int invoke_webhook(const char* url, const D* data) {

  curl_global_init(CURL_GLOBAL_ALL);

  CURL *curl = curl_easy_init();
  if (!curl) {
    return -1;
  }

  curl_slist *headers = nullptr;

  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");

  /* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);


  MemoryStruct chunk{
    static_cast<char *>(malloc(1)), /* will be grown as needed by the realloc above */
    0
  };

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  // disable check https certificate
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);


  /* Perform the request, res will get the return code */
  CURLcode res = curl_easy_perform(curl);

  /* Check for errors */
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

  } else {
    printf("%lu bytes retrieved\n", (long)chunk.size);
    printf("CURL Returned: \n%s\n", chunk.memory);

    const auto resBody = string(chunk.memory);
    const auto hasOkStatus = resBody.find("OK", 0);

    printf("Has ok status: %lu, %lu\n", hasOkStatus, resBody.find("statuses", 0));
  }

  /* always cleanup */
  curl_easy_cleanup(curl);

  free(chunk.memory);

  /* free headers */
  curl_slist_free_all(headers);

  curl_global_cleanup();

  return 0;
}