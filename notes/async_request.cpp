#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <cmath>
#include "StockInfo.h"

// API KEY DO NOT ERASE!!!!!!!
// d9e656a793mshfb457422b030792p1728dcjsn066cc8928220

// Callback function to capture response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::vector<char> *buffer)
{
    // LIBCURL INHERENTLY RECIEVES DATA IN CHUNKS OF INDIVIDUAL CHARACTERS !!!
    size_t totalSize = size * nmemb;
    buffer->insert(buffer->end(), (char *)contents, (char *)contents + totalSize);
    return totalSize;
}

// this call back is the best option convert response to string then add to vector FOR EACH CHUNK A CALL BACK IS CALLED WILL NOT WORK WITH MULTI
//  size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<std::string>* responseList) {
//      size_t totalSize = size * nmemb;

//     // Convert to string
//     std::string responseChunk((char*)contents, totalSize);

//     // Append full response to vector
//     responseList->push_back(responseChunk);

//     return totalSize;
// }

int main()
{
    // writing to a file
    std::vector<std::string> stocks{};
    stocks.push_back("AAPL");
    stocks.push_back("MSFT");
    stocks.push_back("GLD");
    stocks.push_back("TSLA");

        // MAKING API CALL
    CURL *hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=AAPL&lang=en-US&region=US");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "x-rapidapi-key: d9e656a793mshfb457422b030792p1728dcjsn066cc8928220");
    headers = curl_slist_append(headers, "x-rapidapi-host: yahoo-finance-real-time1.p.rapidapi.com");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    std::vector<char> responseList;
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &responseList);

    CURLcode ret = curl_easy_perform(hnd);
    if (ret == CURLE_OK)
    {
        // Convert vector to string to display response
        std::string response(responseList.begin(), responseList.end());
        std::cout << "Response:\n"
                  << "should be response variable here" << std::endl;
    }
    else
    {
        std::cerr << "Request failed: " << curl_easy_strerror(ret) << std::endl;
    }
    curl_easy_cleanup(hnd);
    curl_slist_free_all(headers);

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::string Jresponse(responseList.begin(), responseList.end());
    std::istringstream stream(Jresponse);

    // jsonData is being modified in this function now i access root
    // using jsonData variable
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
        return 1;
    }

    // returning a reference to quoteSummary
    const Json::Value &root = jsonData["quoteSummary"];
    const Json::Value &result = root["result"][0];
    const Json::Value &price = result["price"];
    std::cout << "Price:" << std::endl;

    // organize values
    std::vector<std::string> tickerBucket, companyNameBucket;
    std::vector<double> priceBucket, changeBucket;
    changeBucket.push_back(
        std::round(
            price["regularMarketChange"].asDouble() * 100) /
        100);

    priceBucket.push_back(std::round(price["regularMarketPrice"].asDouble() * 100) / 100);

    tickerBucket.push_back(price["symbol"].asString());
    companyNameBucket.push_back(price["shortName"].asString());

    for (int i = 0; i < companyNameBucket.size(); i++)
    {
        std::cout << "company name: " << companyNameBucket[i] << std::endl;
        std::cout << "ticker symbol: " << tickerBucket[i] << std::endl;
        std::cout << "currnet change: " << changeBucket[i] << std::endl;
        std::cout << "company name: " << priceBucket[i] << std::endl;
    }

    return 0;
}