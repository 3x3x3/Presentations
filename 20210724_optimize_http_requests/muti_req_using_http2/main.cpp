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
    const size_t URL_CNT = 3;
    const std::string URL[URL_CNT] = {
        "https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT",
        "https://api.binance.com/api/v3/ticker/price?symbol=ETHUSDT",
        "https://api.binance.com/api/v3/ticker/price?symbol=XRPUSDT"
    };

    std::string sRespJson[URL_CNT];

    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    CURLM *pCurlMulti = curl_multi_init();
    curl_multi_setopt(pCurlMulti, CURLMOPT_MAX_HOST_CONNECTIONS, (long) 1L);
    curl_multi_setopt(pCurlMulti, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

    CURL *pCurls[URL_CNT];

    for ( int i=0 ; i<URL_CNT ; i++ ) {
        pCurls[i] = curl_easy_init();

        curl_easy_setopt(pCurls[i], CURLOPT_URL, URL[i].c_str());
        curl_easy_setopt(pCurls[i], CURLOPT_WRITEDATA, &sRespJson[i]);
        curl_easy_setopt(pCurls[i], CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(pCurls[i], CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(pCurls[i], CURLOPT_ENCODING, "gzip");
        curl_easy_setopt(pCurls[i], CURLOPT_WRITEFUNCTION, curl_cb);
        curl_easy_setopt(pCurls[i], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2);

        curl_multi_add_handle(pCurlMulti, pCurls[i]);
    }

    //perform requests
    int still_running = 1;
    while ( still_running ) {
        curl_multi_perform(pCurlMulti, &still_running);
    }

    //cleanups
    for ( int i=0 ; i<URL_CNT ; i++ ) {
        curl_easy_cleanup(pCurls[i]);
    }

    curl_multi_cleanup(pCurlMulti);

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;

    printf("Rcv Json1: %s\n", sRespJson[0].c_str());
    printf("Rcv Json2: %s\n", sRespJson[1].c_str());
    printf("Rcv Json3: %s\n", sRespJson[2].c_str());
    printf("Time: %f sec\n", sec.count());

    return sec.count();
}

int main() {
    const int TRY_CNT = 10;
    double dTotTime = 0.0;

    for ( int i=0 ; i<TRY_CNT ; i++ ) {
        dTotTime += request();
        sleep(0.2);
    }

    printf("Average Time: %f sec\n", dTotTime / TRY_CNT);

    return 0;
}
