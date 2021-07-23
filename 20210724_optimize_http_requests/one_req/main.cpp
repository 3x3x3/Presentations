#include <curl/curl.h>
#include <string>
#include <cstdio>
#include <chrono>
#include <unistd.h>


size_t curl_cb(void *content, size_t size, size_t nmemb, std::string *buffer) {
	buffer->append((char*)content, size*nmemb);
	return size*nmemb;
}

double request() {
    const std::string sURL = "https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT";
    //const std::string sURL = "https://api.binance.com/api/v3/exchangeInfo";
    std::string sRespJson;

    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL *pCurl = curl_easy_init();

    curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, curl_cb);
    curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(pCurl, CURLOPT_URL, sURL.c_str());
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &sRespJson);
    //curl_easy_setopt(pCurl, CURLOPT_ENCODING, "gzip");
    //curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(pCurl);

    if ( res != CURLE_OK ) {
        printf("%s", curl_easy_strerror(res));
        return 0L;
    }

    curl_easy_cleanup(pCurl);
    curl_global_cleanup();

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;

    //printf("Rcv Json: %s\n", sRespJson.c_str());
    printf("Time: %f sec\n", sec.count());

    return sec.count();
}

int main() {
    const int TRY_CNT = 3;
    double dTotTime = 0.0;

    for ( int i=0 ; i<TRY_CNT ; i++ ) {
        dTotTime += request();
        sleep(0.2);
    }

    printf("Average Time: %f sec\n", dTotTime / TRY_CNT);

    return 0;
}
