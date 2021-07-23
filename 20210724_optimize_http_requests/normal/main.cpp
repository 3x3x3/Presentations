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
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL *pCurl = curl_easy_init();
    CURLcode res;

    curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(pCurl, CURLOPT_ENCODING, "gzip");
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, curl_cb);
    curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    for ( size_t i=0 ; i<URL_CNT ; i++ ) {
        curl_easy_setopt(pCurl, CURLOPT_URL, URL[i].c_str());
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &sRespJson[i]);
        
		res = curl_easy_perform(pCurl);

		/* Check for errors */ 
		if ( res != CURLE_OK ) {
            printf("%s", curl_easy_strerror(res));
            return 999;
		}
    }

    curl_easy_cleanup(pCurl);
    curl_global_cleanup();

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
