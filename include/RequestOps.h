#ifndef __REQUEST_OPS__
#define __REQUEST_OPS__
#include "ChartInfo.h"
#include <iostream>
#include <jsoncpp/json/json.h>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

class RequestOps
{
private:
    // HEADER BUILDING
    struct curl_slist *headers;
    const std::string headerkey_id = "x-rapidapi-key: ";
    std::string header_apikey;
    const std::string headerhost = "x-rapidapi-host: yahoo-finance-real-time1.p.rapidapi.com";
    const std::string headers_application = "Accept: application/json";

    // INIT
    CURLM *multi_handle;

    // TRACKING EASY HANDLES FOR DELETION
    std::vector<CURL *> easy_handles;

    std::unordered_map<CURL *, std::string> handle_to_ticker;

    // MAP FOR CALLBACK
    std::unordered_map<std::string, std::string> watchlist_map;
    std::unordered_map<std::string, std::string> financial_map;
    std::unordered_map<std::string, std::string> summary_map;

private:
    // must be ran for every MULTI curl call BEGIN
    void set_up_multi_handle_watchlist(const std::vector<std::string> &urls, const std::string &key);
    std::string get_response_type_from_url(const std::string &url);
    bool setup_multirequest_financials(const std::string &ticker, const std::string &key);
    // callbacks for request functions
    static size_t chartWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    void add_headers(const std::string &key);

public:
    RequestOps(std::size_t endpoint_size);
    ~RequestOps();
    struct curl_slist *get_headers();

    bool perform_multirequest_watchlist(const std::vector<std::string> &urls, std::string &key);
    bool perform_multirequest_financials(const std::string &ticker, const std::string &key);
    void multicurl_cleanup();

    // call this for jsoncpp processing GETTERS
    const std::unordered_map<std::string, std::string> &get_watchlist_responses() const;
    std::unordered_map<std::string, std::string> &get_mutable_watchlist_response_map();
    const std::unordered_map<std::string, std::string> &get_immutable_financial_map_ref();
    std::unordered_map<std::string, std::string> &get_mutable_financial_map_ref();
    const std::unordered_map<std::string, std::string> &get_immutable_summary_map_ref();
    std::unordered_map<std::string, std::string> &get_mutable_summary_map_ref();

    std::string get_ticker_from_url(const std::string &url);
    std::unordered_map<CURL *, std::string> &get_handle_to_ticker();

    void manually_delete_responses(std::unordered_map<std::string, std::string> *map);
    // when user adds another ticker to watchlist
    bool perform_single_request_watchlist(const std::string &ticker, const std::string &key);
    bool perform_single_request_summary(const std::string &ticker, const std::string &key);
    void perform_single_request_charts(const std::string &ticker, const std::string &key, ChartInfo &c);

    // TESTING
};

#endif
