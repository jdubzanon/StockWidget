#include "RequestOps.h"

RequestOps::RequestOps(std::size_t endpoints_size)
    : headers(NULL), multi_handle(nullptr)
{
    header_apikey = "";
    // size comes from the amount of tickers in watchlist.dat
    watchlist_map.reserve(endpoints_size);
    curl_global_init(CURL_GLOBAL_ALL);
}
RequestOps::~RequestOps()
{
    curl_global_cleanup();
    curl_slist_free_all(headers);
}
size_t RequestOps::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    std::string *response = static_cast<std::string *>(userp);
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

size_t RequestOps::chartWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    std::string *response = static_cast<std::string *>(userp);
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

// MUST CALL SET HEADER FIRST
void RequestOps::add_headers(const std::string &key)
{
    std::string header_concat = headerkey_id + key;
    headers = curl_slist_append(headers, header_concat.c_str()); // Replace with your actual API key
    headers = curl_slist_append(headers, headerhost.c_str());
    headers = curl_slist_append(headers, headers_application.c_str());
}

struct curl_slist *RequestOps::get_headers()
{
    return headers;
}
// PRIVATE
void RequestOps::set_up_multi_handle_watchlist(const std::vector<std::string> &urls, const std::string &key)
{
    if (!multi_handle)
    {
        multi_handle = curl_multi_init();
    }

    if (headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    if (!headers)
        add_headers(key);

    for (const std::string &url : urls)
    {

        CURL *easy_handle = curl_easy_init();
        if (!easy_handle)
        {
            continue;
        }

        // ADD EXTRACTED TICKER TO CURLHANDLE TICKERS MAP
        std::string extacted_ticker = get_ticker_from_url(url);

        curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
        // add headers
        curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);
        // trial
        handle_to_ticker[easy_handle] = extacted_ticker;
        curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, handle_to_ticker[easy_handle].c_str());
        // Set a timeout to avoid hanging.
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, 3L);
        curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_LIMIT, 200L);
        curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_TIME, 10L);

        // init response map string
        watchlist_map[extacted_ticker] = "";

        // setup write callback and init pointer for userp* in callback function
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &watchlist_map[extacted_ticker]);

        curl_multi_add_handle(multi_handle, easy_handle);
        easy_handles.push_back(easy_handle);

    } // end for loops of urls
}

// PRIVATE
bool RequestOps::setup_multirequest_financials(const std::string &ticker, const std::string &key)
{

    if (!financial_map.empty())
    {
        financial_map.clear();
    }

    // BALANCE SHEET ENDPOINT
    std::string balance_sheet_url = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-balance-sheet?lang=en-US&region=US&symbol=" + ticker;

    // CASHFLOW ENDPOINT
    std::string cashflow_url = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-cashflow?region=US&lang=en-US&symbol=" + ticker;

    // EARNINGS ENDPOINT
    std::string earnings_head = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-earnings?symbol=";
    std::string earnings_tail = "&lang=en-US&region=US";
    std::string earnings_concat = earnings_head + ticker + earnings_tail;

    std::vector<std::string> urls{balance_sheet_url, cashflow_url, earnings_concat};

    if (!multi_handle)
    {
        multi_handle = curl_multi_init();
    }
    if (headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    if (!headers)
        add_headers(key);

    for (const std::string &url : urls)
    {

        CURL *easy_handle = curl_easy_init();
        if (!easy_handle)
        {
            continue;
        }

        // ADD EXTRACTED TICKER TO CURLHANDLE TICKERS MAP
        std::string extacted_ticker = get_ticker_from_url(url);

        curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
        // add headers
        curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);
        // Set a timeout to avoid hanging.
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 10L);

        // init response map string
        std::string response_type = get_response_type_from_url(url);
        if (!response_type.empty())
        {
            financial_map[response_type] = "";
            curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &financial_map[response_type]);
            curl_multi_add_handle(multi_handle, easy_handle);
            easy_handles.push_back(easy_handle);
        }
        else
        {
            return false;
        }

    } // end for loops of urls
    return true;
}

bool RequestOps::setup_multi_request_holdings(const std::string &ticker, const std::string &key)
{
    if (!holdings_map.empty())
    {
        holdings_map.clear();
    }

    std::string holdings_url_head = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-top-holdings?symbol=";
    std::string holdings_url_foot = "&region=US&lang=en-US";
    std::string holdings_url_concat = holdings_url_head + ticker + holdings_url_foot;

    std::string profile_url = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-profile?region=US&lang=en-US&symbol=" + ticker;

    std::string urls[2] = {holdings_url_concat, profile_url};

    if (!multi_handle)
    {
        multi_handle = curl_multi_init();
    }
    if (!headers)
    {
        add_headers(key);
    }

    for (const std::string &url : urls)
    {
        CURL *easy_handle = curl_easy_init();
        if (!easy_handle)
        {
            continue;
        }

        curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 10L);

        std::string response_type = get_response_type_from_url(url);

        if (!response_type.empty())
        {
            holdings_map[response_type] = "";
            curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &holdings_map[response_type]);
            curl_multi_add_handle(multi_handle, easy_handle);
        }
        else
        {
            std::cout << "get response type failed" << std::endl;
            return false;
        }

    } // end for loops of urls
    return true;
}

bool RequestOps::perform_multirequest_watchlist(const std::vector<std::string> &urls, std::string &key)
{

    set_up_multi_handle_watchlist(urls, key);

    int still_running = 0;
    do
    {
        CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK)
        {
            return false;
        }

        int numfds = 0;
        mc = curl_multi_wait(multi_handle, nullptr, 0, 500, &numfds);
        if (mc != CURLM_OK)
        {
            return false;
        }

    } while (still_running);
    manually_delete_responses(&watchlist_map);

    return true;
}

bool RequestOps::perform_multirequest_financials(const std::string &ticker, const std::string &key)
{
    if (!setup_multirequest_financials(ticker, key))
    {
        return false;
    }

    int still_running = 0;
    do
    {
        CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK)
        {
            return false;
        }

        int numfds = 0;
        mc = curl_multi_wait(multi_handle, nullptr, 0, 1000, &numfds);
        if (mc != CURLM_OK)
        {
            return false;
        }

    } while (still_running);
    manually_delete_responses(&financial_map);
    return true;
}

bool RequestOps::perform_multi_request_holdings(const std::string &ticker, const std::string &key)
{
    if (!setup_multi_request_holdings(ticker, key))
        return false;

    int still_running = 0;
    do
    {
        CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK)
        {
            return false;
        }

        int numfds = 0;
        mc = curl_multi_wait(multi_handle, nullptr, 0, 1000, &numfds);
        if (mc != CURLM_OK)
        {
            return false;
        }

    } while (still_running);
    manually_delete_responses(&holdings_map);

    return true;
}

void RequestOps::multicurl_cleanup()
{

    for (CURL *handle : easy_handles)
    {
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
    }

    easy_handles.clear();

    if (multi_handle)
    {
        curl_multi_cleanup(multi_handle);
        multi_handle = nullptr;
    }
}

const std::unordered_map<std::string, std::string> &RequestOps::get_watchlist_responses() const
{
    return watchlist_map;
}

std::unordered_map<std::string, std::string> &RequestOps::get_mutable_watchlist_response_map()
{
    return watchlist_map;
}

bool RequestOps::perform_single_request_watchlist(const std::string &ticker, const std::string &key)
{
    std::string url_head;
    std::string url_foot;
    std::string url_concat;
    CURL *handle = curl_easy_init();
    if (!handle)
    {
        return false;
    }

    if (headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    if (!headers)
        add_headers(key);

    url_head = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=";
    url_foot = "&lang=en-US&region=US";
    url_concat = (url_head + ticker + url_foot);

    if (headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    if (!headers)
    {
        add_headers(key);
    }

    curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(handle, CURLOPT_URL, url_concat.c_str());

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);

    watchlist_map[ticker] = "";
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &watchlist_map[ticker]);

    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10L);
    CURLcode ret = curl_easy_perform(handle);

    curl_easy_cleanup(handle);
    manually_delete_responses(&watchlist_map);
    return (ret == CURLE_OK);
}

bool RequestOps::perform_single_request_summary(const std::string &ticker, const std::string &key)
{
    CURL *handle = curl_easy_init();
    if (!handle)
    {
        return false;
    }
    if (headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    if (!headers)
        add_headers(key);

    std::string head = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-summary?lang=en-US&symbol=";
    std::string tail = "&region=US";
    std::string url_concat = head + ticker + tail;
    if (!headers)
    {
        add_headers(key);
    }

    curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(handle, CURLOPT_URL, url_concat.c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);

    summary_map["summary"] = "";
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &summary_map["summary"]);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10L);
    CURLcode ret = curl_easy_perform(handle);
    curl_easy_cleanup(handle);
    manually_delete_responses(&summary_map);
    return (ret == CURLE_OK);
}

void RequestOps::perform_single_request_charts(const std::string &ticker, const std::string &key, ChartInfo &c)
{
    std::string url_head = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-chart?symbol=";
    std::string url_tail = "&region=US&lang=en-US&useYfid=true&includeAdjustedClose=true&events=div%2Csplit%2Cearn&range=1y&interval=1d&includePrePost=false";
    std::string url_concat = url_head + ticker + url_tail;
    CURL *handle = curl_easy_init();
    if (!handle)
    {

        c.set_api_chart_error(true);
        c.set_api_def("charts request handle fault");
        return;
    }
    if (!headers)
    {
        add_headers(key);
    }
    curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(handle, CURLOPT_URL, url_concat.c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);

    std::unordered_map<std::string, std::string> &chart_map = c.get_mutable_chart_respone_map();
    chart_map[ticker] = "";
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, chartWriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &chart_map[ticker]);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10L);
    CURLcode ret = curl_easy_perform(handle);
    if (ret != CURLE_OK)
    {
        c.set_api_chart_error(true);
        c.set_api_def("return was not ok, api call failed");
        return;
    }
    curl_easy_cleanup(handle);
    handle = nullptr;
    manually_delete_responses(&chart_map);

    return;
}

std::string RequestOps::get_ticker_from_url(const std::string &url)
{
    std::regex pattern("symbol=(\\w+-?\\w+)&lang");
    std::smatch match;
    if (std::regex_search(url, match, pattern))
    {
        return match[1];
    }
    return "";
}

std::string RequestOps::get_response_type_from_url(const std::string &url)
{
    std::regex pattern("stock/get-(\\w+-?\\w+)?");
    std::smatch match;
    if (std::regex_search(url, match, pattern))
    {
        return match[1];
    }
    return "";
}

// GETTERS
std::unordered_map<CURL *, std::string> &RequestOps::get_handle_to_ticker()
{
    return handle_to_ticker;
}

const std::unordered_map<std::string, std::string> &RequestOps::get_immutable_financial_map_ref()
{
    return financial_map;
}

std::unordered_map<std::string, std::string> &RequestOps::get_mutable_financial_map_ref()
{
    return financial_map;
}

const std::unordered_map<std::string, std::string> &RequestOps::get_immutable_summary_map_ref()
{
    return summary_map;
}

std::unordered_map<std::string, std::string> &RequestOps::get_mutable_summary_map_ref()
{
    return summary_map;
}

std::unordered_map<std::string, std::string> &RequestOps::get_holdings_map()
{
    return holdings_map;
}

void RequestOps::manually_delete_responses(std::unordered_map<std::string, std::string> *map)
{
    for (auto it = map->begin(); it != map->end();)
    {
        if (it->second.empty())
        {
            it = map->erase(it); // erase returns next valid iterator
        }
        else
        {
            ++it;
        }
    }
}