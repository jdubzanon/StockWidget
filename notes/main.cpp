
// API KEY DO NOT ERASE!!!!!!!
// d9e656a793mshfb457422b030792p1728dcjsn066cc8928220

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include "StockInfo.h"
#include <memory>
#include <cmath>

// A simple write callback that appends data to a std::string.
// The pointer passed to WRITEDATA is the address of the std::string
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    std::string *response = static_cast<std::string *>(userp);
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

// name, ticker, price, change
void collect_stocks(std::vector<std::unique_ptr<StockInfo>> &my_stocks, const Json::Value &root_json)
{
    const Json::Value &result = root_json["result"][0];
    const Json::Value &price = result["price"];
    std::string company_name = price["shortName"].asString();
    std::string companyticker = price["ticker"].asString();
    double marketprice = std::round(price["regularMarketPrice"].asDouble() * 100) / 100;
    double pricechange = std::round(price["regularMarketChange"].asDouble() * 100) / 100;
    my_stocks.emplace_back(std::make_unique<StockInfo>(company_name, companyticker, marketprice, pricechange));
}

const Json::Value get_root(std::string &valuedpair)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(valuedpair);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
        return errs;
    }

    const Json::Value root = jsonData["quoteSummary"];
    return root;
}

int main()
{
    // Initialize libcurl globally.
    curl_global_init(CURL_GLOBAL_ALL);

    // Create a multi handle.
    CURLM *multi_handle = curl_multi_init();

    // List of URLs to fetch.
    std::vector<std::string> urls = {
        "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=AAPL&lang=en-US&region=US",
        "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=MSFT&lang=en-US&region=US"};

    // This vector will hold our easy handles.
    std::vector<CURL *> easy_handles;

    // Unordered map to store responses; keys are CURL* and values are the response strings.
    std::unordered_map<CURL *, std::string> responseMap;
    responseMap.reserve(urls.size()); // Avoid rehashing

    // Set up HTTP headers required by RapidAPI.
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "x-rapidapi-key: d9e656a793mshfb457422b030792p1728dcjsn066cc8928220"); // Replace with your actual API key
    headers = curl_slist_append(headers, "x-rapidapi-host: yahoo-finance-real-time1.p.rapidapi.com");
    headers = curl_slist_append(headers, "Accept: application/json");

    // Set up each easy handle.
    for (const std::string &url : urls)
    {
        CURL *easy_handle = curl_easy_init();
        if (!easy_handle)
        {
            std::cerr << "Failed to initialize easy handle." << std::endl;
            continue;
        }

        curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);

        // Set a timeout to avoid hanging.
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 10L);

        // Initialize the response string for this handle in the map.
        responseMap[easy_handle] = "";
        // Set the write callback.
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        // Pass a pointer to the response string as WRITEDATA.
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &responseMap[easy_handle]);
        // Optionally store the handle as private data.
        curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, easy_handle);

        // Add the easy handle to the multi handle.
        curl_multi_add_handle(multi_handle, easy_handle);
        easy_handles.push_back(easy_handle);

        // std::cout << "[main] Added request for: " << url << std::endl;
    }

    // Perform the multi request.
    // int still_running = 0;
    // CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
    // if (mc != CURLM_OK)
    // {
    //     std::cerr << "curl_multi_perform() failed: " << curl_multi_strerror(mc) << std::endl;
    // }

    // // Loop until all transfers are complete.
    // while (still_running)
    // {
    //     int numfds = 0;
    //     mc = curl_multi_wait(multi_handle, nullptr, 0, 1000, &numfds);
    //     if (mc != CURLM_OK)
    //     {
    //         std::cerr << "curl_multi_wait() failed: " << curl_multi_strerror(mc) << std::endl;
    //         break;
    //     }
    //     mc = curl_multi_perform(multi_handle, &still_running);
    // }

    int still_running = 0;
    do
    {
        CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK)
        {
            std::cerr << "curl_multi_perform() failed: " << curl_multi_strerror(mc) << std::endl;
            break; // If this fails, you might want to break the loop
        }

        int numfds = 0;
        mc = curl_multi_wait(multi_handle, nullptr, 0, 1000, &numfds);
        if (mc != CURLM_OK)
        {
            std::cerr << "curl_multi_wait() failed: " << curl_multi_strerror(mc) << std::endl;
            break; // If this fails, break out of the loop
        }

    } while (still_running);

    // START ORGANIZING CLASSES
    // std::vector<std::unique_ptr<StockInfo>> stocks;

    // for (const auto &pair : responseMap)
    // {
    //     Json::Value root = get_root(responseMap[pair.first]);
    //     collect_stocks(stocks, root);
    // }

    // for (const auto &stock : stocks)
    // {
    //     std::string c = stock->get_company_name();
    //     std::cout << c << std::endl;
    // }
    // Cleanup: Remove and clean up each easy handle.
    for (CURL *handle : easy_handles)
    {
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
    }

    for (const auto &pair : responseMap)
    {
        std::cout << pair.second << std::endl;
        std::cout << std::endl;
    }

    // Free the header list.
    curl_slist_free_all(headers);
    // Clean up the multi handle and global libcurl state.
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();

    return 0;
}
