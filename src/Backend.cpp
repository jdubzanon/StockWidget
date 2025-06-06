#include "Backend.h"

Backend::Backend()
    : fileops(std::make_unique<FileOps>()),
      encryptops(std::make_unique<EncryptOps>()),
      requestops(nullptr),
      jsonparseops(std::make_unique<JsonParseOps>()),
      watchlist_tracker(nullptr)

{
    fileops->system_init();
    fileops->set_all_file_permissions();
    apifile_is_empty = fileops->apikeyfile_empty();
    fileops->watchlist_tracker_init();
    watchlist_tracker = fileops->get_watchlist_tracker();
    std::size_t watchlist_tracker_size = watchlist_tracker->size();
    requestops = std::make_unique<RequestOps>(watchlist_tracker_size);
}

Backend::~Backend()
{
    watchlist_tracker = nullptr;
}

bool Backend::get_api_file_status()
{
    apifile_is_empty = fileops->apikeyfile_empty();
    return apifile_is_empty;
}

bool Backend::get_watchlist_file_status()
{
    watchlist_is_empty = fileops->watchlistfile_empty();
    return watchlist_is_empty;
}

const std::vector<StockInfo> &Backend::get_stockinfo_vec()
{
    const std::vector<StockInfo> &info_vec = jsonparseops->get_immutable_stockinfo_vec();
    return info_vec;
}

std::vector<StockInfo> *Backend::pass_stockinfo_ptr()
{
    return jsonparseops->get_stockinfo_vec_ptr();
}

// this has to be run in the gui class init
std::vector<std::string> *Backend::get_backend_watchlist_tracker()
{
    return fileops->get_watchlist_tracker();
}

// this will have to wrapped in the GUI function  when a button is pressed try and catch api call operation
// i can try and catch apikeyfile empty as well
bool Backend::run_add_to_watchlist_operations(const std::string &ticker)
{
    bool unavail = false;

    std::string ticker_copy = ticker;
    std::string ticker_to_uppercase = make_uppercase(ticker_copy);
    if (fileops->apikeyfile_empty())
    {
        throw BackendException(BackendException::ErrorType::API_KEY_FILE_EMPTY, "Need to add an api key first..");
    }
    // deleting maps before making call
    std::unordered_map<std::string, std::string> &mutable_response_map = requestops->get_mutable_watchlist_response_map();
    mutable_response_map.clear();

    run_single_api_call_operations(ticker_to_uppercase);

    // INTERNET CHECK
    if (mutable_response_map.empty())
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "bro..\nare you even connected to the internt?\nif so api may be down");

    std::string &returned_json = mutable_response_map.at(ticker_to_uppercase);

    // API CONFIRMATION
    confirm_apikey(returned_json, mutable_response_map);

    jsonparseops->clean_resonse(returned_json);

    JsonParseOps::JSON_CODES jsonops_code = run_internal_parsing_operations(returned_json, ticker_to_uppercase);

    if (jsonops_code == JsonParseOps::JSON_CODES::INFORMATION_UNAVAILABLE)
    {
        unavail = true;
    }
    else if (jsonops_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED || jsonops_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED)
    {
        mutable_response_map.clear();
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Json failed to parse, try again?");
    }
    else if (jsonops_code == JsonParseOps::JSON_CODES::PUSHED_TO_INFO_VEC || unavail)
    {

        fileops->add_to_watchlist(ticker_to_uppercase);
    }
    else if (jsonops_code == JsonParseOps::JSON_CODES::TICKER_UNCONFIMED)
    {
        mutable_response_map.clear();
        throw BackendException(BackendException::ErrorType::INVALID_TICKER, "API doesn't support " + ticker_to_uppercase);
    }

    mutable_response_map.clear();

    if (jsonops_code == JsonParseOps::JSON_CODES::PUSHED_TO_INFO_VEC || unavail)
    {
        if (!fileops->confirm_ticker_added_to_file(ticker_to_uppercase))
        {
            mutable_response_map.clear();
            throw BackendException(BackendException::ErrorType::FILE_WRITE_COMMON, "There was a problem writing to ticker to file\nplease try again..");
        }
        if (unavail)
        {
            throw BackendException(BackendException::ErrorType::API_INFO_CURRENTLY_UNAVAILABLE, "Information is currently unavailableT\nicker added to watchlist");
        }
    }

    // FIND THE OBJECT THAT I JUST PUT IN THERE, CHECK THE QUOTE TYPE AND THROW ERROR/WARNING UNSUPPORTED TYPE

    const std::vector<StockInfo> &vec = jsonparseops->get_immutable_stockinfo_vec();
    auto it = std::find_if(vec.begin(), vec.end(),
                           [&ticker_to_uppercase](const StockInfo &node)
                           { return node.get_ticker() == ticker_to_uppercase; });
    if (it != vec.end())
    {
        int pos = std::distance(vec.begin(), it);
        const StockInfo &node = vec.at(pos);
        // had to ask chatgpt for this logic. i couldnt figure it out
        // if it doesnt equal any of these then throw error
        if (node.get_quote_type() != "EQUITY" && node.get_quote_type() != "CRYPTOCURRENCY" && node.get_quote_type() != "ETF")
        {
            std::ostringstream oss;
            oss << "Unsupported quote type\n"
                << node.get_quote_type() << "\ninformation may be unavailable";
            throw BackendException(BackendException::ErrorType::UNSUPPORTED_QUOTE_TYPE, oss.str());
        }
    }
    return true;
}

// Public
bool Backend::run_delete_from_watchlist_operations(const std::string &ticker)
{
    std::string ticker_copy = ticker;
    std::string ticker_to_uppercase = make_uppercase(ticker_copy);

    // remove StockInfo node from stock_information vector vector
    {
        std::vector<StockInfo> &mutable_vector = jsonparseops->get_mutable_stockinfo_vec();

        auto it = std::find_if(mutable_vector.begin(), mutable_vector.end(),
                               [&ticker_to_uppercase](const StockInfo &node)
                               {
                                   return node.get_ticker() == ticker_to_uppercase;
                               });

        if (it != mutable_vector.end())
            mutable_vector.erase(it);
    }

    //  confirm StockInfo node was deleted from stock_information
    if (!jsonparseops->confirm_deletion(ticker_to_uppercase))
    {
        throw BackendException(BackendException::ErrorType::DELETION_INCOMPLETE, "Something went wrong with deletion...");
    }

    // delete from watchlist tracker and from file
    fileops->delete_watchlist_item(ticker_to_uppercase);

    if (!fileops->confirm_ticker_deletion(ticker_to_uppercase))
    {
        throw BackendException(BackendException::ErrorType::DELETION_INCOMPLETE, "something went wrong with deleting from file..");
    }

    return true;
}

// PUBLIC
void Backend::run_delete_from_financials_operations(const std::string &ticker)
{
    std::string ticker_copy = ticker;
    std::string ticker_to_uppercase = make_uppercase(ticker_copy);
    {
        std::vector<StockFinancials> &mutable_vector = jsonparseops->get_mutable_stock_financials_vec();

        auto it = std::find_if(mutable_vector.begin(), mutable_vector.end(),
                               [&ticker_to_uppercase](const StockFinancials &node)
                               {
                                   return node.get_ticker() == ticker_to_uppercase;
                               });

        if (it != mutable_vector.end())
            mutable_vector.erase(it);
    }
}

// PUBLIC
void Backend::run_delete_from_charts_operations(const std::string &ticker)
{
    std::string ticker_copy = ticker;
    std::string ticker_to_uppercase = make_uppercase(ticker_copy);
    {
        std::vector<ChartInfo> &mutable_vector = jsonparseops->get_mutable_chart_info_vec();

        auto it = std::find_if(mutable_vector.begin(), mutable_vector.end(),
                               [&ticker_to_uppercase](const ChartInfo &node)
                               {
                                   return node.get_ticker() == ticker_to_uppercase;
                               });

        if (it != mutable_vector.end())
            mutable_vector.erase(it);
    }
}

// PUBLIC
void Backend::run_delete_from_metrics_operations(const std::string &ticker)
{
    std::string ticker_copy = ticker;
    std::string ticker_to_uppercase = make_uppercase(ticker_copy);
    {
        std::vector<Metrics> &mutable_vector = jsonparseops->get_mutable_metrics_vec();

        auto it = std::find_if(mutable_vector.begin(), mutable_vector.end(),
                               [&ticker_to_uppercase](const Metrics &node)
                               {
                                   return node.get_ticker() == ticker_to_uppercase;
                               });

        if (it != mutable_vector.end())
            mutable_vector.erase(it);
    }
}

// PUBLIC
void Backend::run_delete_from_etf_holdings_operations(const std::string &ticker)
{
    std::string ticker_copy = ticker;
    std::string ticker_to_uppercase = make_uppercase(ticker_copy);
    {
        std::vector<ETF_Holdings> &mutable_vector = jsonparseops->get_etf_holdings_vec();

        auto it = std::find_if(mutable_vector.begin(), mutable_vector.end(),
                               [&ticker_to_uppercase](const ETF_Holdings &node)
                               {
                                   return node.get_ticker() == ticker_to_uppercase;
                               });

        if (it != mutable_vector.end())
            mutable_vector.erase(it);
    }
}

// PUBLIC
void Backend::run_add_api_key_operations(const std::string &api_key)
{

    // ENCRYPTION
    encryptops->clear_map();
    encryptops->generate_key_iv();
    encryptops->encrypt_api_key(api_key);

    // CHECK IF API KEY IS VALID
    bool success = requestops->perform_single_request_watchlist("AAPL", api_key);
    std::unordered_map<std::string, std::string> &watchlist_map = requestops->get_mutable_watchlist_response_map();

    if (!success)
    {
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "API call failed. try again..");
    }

    const std::string &response = watchlist_map.at("AAPL");
    JsonParseOps::JSON_CODES apikey_conf_code = jsonparseops->apikey_confimed(response);
    if (apikey_conf_code == JsonParseOps::JSON_CODES::API_KEY_FAILED)
    {
        watchlist_map.clear();
        throw BackendException(BackendException::ErrorType::API_CONFIRMATION_FAILED, "API key failed, please check api key...\nKey was not saved");
    }
    else if (apikey_conf_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED)
    {
        watchlist_map.clear();
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Json Parse Failed!\nTry Again?");
    }

    if (apikey_conf_code == JsonParseOps::JSON_CODES::API_KEY_CONFIRMED)
    {
        watchlist_map.clear();
        // WRITE ENCRYPTION IF THE API KEY IS CONFIRMED
        const std::unordered_map<std::string, std::vector<uint8_t>> *immut_map = encryptops->get_immutable_map();
        bool failed_to_mod_iv_file = fileops->add_key_iv_info_to_file(immut_map);
        bool failed_to_mod_api_file = fileops->add_encrypted_api_key_to_file(immut_map);
        // IF ISSUE WRITING TO FILE THROW ERROR
        if (failed_to_mod_api_file || failed_to_mod_iv_file)
        {
            throw BackendException(BackendException::ErrorType::API_FILE_WRITING_FAILED, "something went wrong with writing to files..");
        }
        immut_map = nullptr;
        encryptops->clear_map();
        apifile_is_empty = fileops->apikeyfile_empty();
    }
    return;
}

// INTERNAL NOT USED OUTSIDE CLASS
void Backend::build_map_from_files()
{
    // read_key_iv_file needs to be written
    std::unordered_map<std::string, std::vector<uint8_t>> *mutable_map = encryptops->get_mutable_map();
    if (!mutable_map->empty())
    {
        mutable_map->clear();
    }
    const size_t aes_size = encryptops->get_aes_size();
    const size_t iv_size = encryptops->get_iv_size();
    std::vector<std::vector<uint8_t>> key_iv_info = fileops->read_key_iv_file(aes_size, iv_size);
    mutable_map->insert_or_assign("key", key_iv_info[0]);
    mutable_map->insert_or_assign("iv", key_iv_info[1]);

    std::vector<uint8_t> api_data = fileops->read_api_enc_file();
    mutable_map->insert_or_assign("encrypted_apikey", api_data);
}

// THIS IS INTERNAL/PRIVATE
std::string Backend::run_decrypt_apikey_operations()
{

    // this builds the map in EncryptOps so now i can access inside encryptops no need to pass pointer
    build_map_from_files();
    std::string decyrpted_str = encryptops->decrypt_api_key();
    return decyrpted_str;
}

// PRIVATE FOR run_multi_watchlist_api_calls_operations()
void Backend::confirm_apikey(const std::string &response, bool &conf_ref, std::unordered_map<std::string, std::string> &map_ref)
{
    JsonParseOps::JSON_CODES conf_code = jsonparseops->apikey_confimed(response);
    if (conf_code == JsonParseOps::JSON_CODES::API_KEY_CONFIRMED)
    {
        conf_ref = true;
        return;
    }

    else if (conf_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED || conf_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED)
    {
        map_ref.clear();
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Json parse failed in multi watchlist\n--API Confirmation\ntry again?");
    }

    else if (conf_code == JsonParseOps::JSON_CODES::API_KEY_FAILED)
    {
        map_ref.clear();
        throw BackendException(BackendException::ErrorType::API_CONFIRMATION_FAILED, "API Confirmation failed multi watchlist");
    }

    return;
}

// PRIVATE GENERIC APIKEY CONFIRMATION
void Backend::confirm_apikey(const std::string &response, std::unordered_map<std::string, std::string> &map_ref)
{
    JsonParseOps::JSON_CODES conf_code = jsonparseops->apikey_confimed(response);
    if (conf_code == JsonParseOps::JSON_CODES::API_KEY_CONFIRMED)
        return;
    else if (conf_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED || conf_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED)
    {
        map_ref.clear();
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Json parse failed in multi watchlist\n--API Confirmation\ntry again?");
    }

    else if (conf_code == JsonParseOps::JSON_CODES::API_KEY_FAILED)
    {
        map_ref.clear();
        throw BackendException(BackendException::ErrorType::API_CONFIRMATION_FAILED, "API Confirmation failed multi watchlist");
    }
}

// MAKING API CALL USING API KEYS
// PRIVATE NEED FOR JSONCPP FUNCTION
// WRAPPED IN add_watchlist_operations()
void Backend::run_single_api_call_operations(const std::string &ticker)
{
    encryptops->clear_map();
    std::string api_key = "";
    api_key = run_decrypt_apikey_operations();
    encryptops->clear_map();

    bool success = requestops->perform_single_request_watchlist(ticker, api_key);
    if (!success)
    {
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "something went wrong with api call. please try again...");
    }
    return;
}

// PUBLIC
bool Backend::run_multi_watchlist_api_calls_operations()
{
    std::vector<std::string> unvail_stocks;
    std::vector<std::string> parsing_failed_stocks;
    bool unavail_on = false;
    bool parsing_failed_on = false;
    if (fileops->apikeyfile_empty())
    {
        throw BackendException(BackendException::ErrorType::API_KEY_FILE_EMPTY, "NEED TO ENTER API KEY");
    }
    // CLEAR STOCK INFO VECTOR IF NEEDED

    {
        const std::vector<StockInfo> &immutable_stockinfo_vec = jsonparseops->get_immutable_stockinfo_vec();
        if (!immutable_stockinfo_vec.empty())
        {
            std::vector<StockInfo> &mutable_stockinfo_vec = jsonparseops->get_mutable_stockinfo_vec();
            mutable_stockinfo_vec.clear();
        }
    }

    // CLEAR HANDLE TICKER BEFORE USING
    std::unordered_map<CURL *, std::string> handle_to_ticker_map = requestops->get_handle_to_ticker();
    handle_to_ticker_map.clear();

    // CLEAR RESPONSE MAP IF NEEDED BEFORE MAKING CALL
    std::unordered_map<std::string, std::string> &watchlist_mutable_map = requestops->get_mutable_watchlist_response_map();
    watchlist_mutable_map.clear();

    encryptops->clear_map();

    // BUILD DECRYPTION MAP AND DECRYPT API KEY
    std::string api_key = "";
    api_key = run_decrypt_apikey_operations();
    encryptops->clear_map();
    // BUILDING RESPONSE MAP
    std::vector<std::string> endpoints = fileops->create_api_endpoints_from_watchlist();
    std::vector<std::string> temp_endpoints;
    int start = 0;
    int end = 9;
    bool end_endpoints = false;

    while (true)
    {
        if (static_cast<unsigned long>(end) < endpoints.size())
        {
            temp_endpoints.insert(temp_endpoints.end(), endpoints.begin() + start, endpoints.begin() + end);
        }
        else
        {
            temp_endpoints.insert(temp_endpoints.end(), endpoints.begin() + start, endpoints.end());
            end_endpoints = true;
        }

        if (!requestops->perform_multirequest_watchlist(temp_endpoints, api_key))
        {
            throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "Multi api call failed. try again...");
        }
        if (end_endpoints)
        {
            break;
        }

        start += 10;
        end += 10;
        temp_endpoints.clear();
        // need to wait 1.25seconds
        // API CAN ONLY DO 10 request per second;
        std::this_thread::sleep_for(std::chrono::milliseconds(1250));
    } // end of while loop

    // RequestOps::manually_delete_responses() deletes all bad responses, if all responses are bad then thats a possible bad internet connection or api failure

    if (watchlist_mutable_map.empty())
    {
        throw BackendException(BackendException::ErrorType::MULTI_API_CALL_TOTAL_FAILURE, "Bro.. \nAre you even connected to the internet?\nif you are api may be down right now");
    }

    bool apikey_confimed = false;

    for (auto &e : endpoints)
    {
        // handles: "quoteSummary": { "result" : [ {"error" : true, "message" : "source currently unavailable"} ] }
        std::string local_ticker = requestops->get_ticker_from_url(e);
        auto it = watchlist_mutable_map.find(local_ticker);
        if (it != watchlist_mutable_map.end())
        {
            std::string &response = watchlist_mutable_map.at(it->first);

            // ONLY NEED THIS TO OCCUR ONCE
            if (!apikey_confimed)
            {
                bool &apikey_conf_ref = apikey_confimed;
                confirm_apikey(response, apikey_conf_ref, watchlist_mutable_map);
            }

            jsonparseops->clean_resonse(response);

            JsonParseOps::JSON_CODES info_avail = jsonparseops->information_unavailable_check(response);
            if (info_avail == JsonParseOps::JSON_CODES::INFORMATION_UNAVAILABLE)
            {
                unvail_stocks.push_back(local_ticker + "\n");
                jsonparseops->add_default_stockinfo_node_to_stockinformation(local_ticker);
                unavail_on = true;
                continue;
            }

            JsonParseOps::JSON_CODES parse_code = jsonparseops->parse_watchlist_item(response);
            if (parse_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED || parse_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED)
            {
                parsing_failed_on = true;
                std::string push_local_t = "-" + local_ticker + "\n";
                parsing_failed_stocks.push_back(push_local_t);
                continue;
            }
        }
    }

    // CLEAR MAPS AFTER USING
    handle_to_ticker_map.clear();
    watchlist_mutable_map.clear();
    requestops->multicurl_cleanup();

    if (unavail_on && parsing_failed_on)
    {
        std::string header = "Multiple Issues Getting Info for:\n";
        std::string bad_stocks;
        std::ostringstream oss;
        oss << header;
        for (const auto &s : unvail_stocks)
        {
            oss << "-" << s << "\n";
        }
        for (const auto &t : parsing_failed_stocks)
        {
            oss << t;
        }
        throw BackendException(BackendException::ErrorType::MULTI_PARSING_FAILURE, oss.str());
    }
    else if (parsing_failed_on && !unavail_on)
    {
        std::string header = "Issues Parsing\n";
        std::ostringstream oss;
        oss << header;
        for (const auto &t : parsing_failed_stocks)
        {
            oss << t;
        }
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, oss.str());
    }
    else if (unavail_on && !parsing_failed_on)
    {
        std::string header = "There was some unavailable information for: \n";
        std::string missing_stocks;
        std::ostringstream oss;
        oss << header;
        for (const auto &s : unvail_stocks)
        {
            oss << "-" << s;
        }

        throw BackendException(BackendException::ErrorType::API_INFO_CURRENTLY_UNAVAILABLE, oss.str());
    }
    return true;
}

// PUBLIC
bool Backend::run_financials_operations(const std::string &ticker)
{
    // CLEAR FINANCIAL MAP BEFORE EACH CALL
    {
        std::unordered_map<std::string, std::string> &fin_map_ref = requestops->get_mutable_financial_map_ref();
        if (!fin_map_ref.empty())
        {
            fin_map_ref.clear();
        }
    }

    encryptops->clear_map();
    std::string api_key = "";
    api_key = run_decrypt_apikey_operations();
    encryptops->clear_map();
    requestops->perform_multirequest_financials(ticker, api_key);
    const std::unordered_map<std::string, std::string> &fin_map_ref = requestops->get_immutable_financial_map_ref(); // setup ref to financialmap json
    std::unordered_map<std::string, std::string> &mutable_fin_map_ref = requestops->get_mutable_financial_map_ref();

    if (fin_map_ref.empty())
        throw BackendException(BackendException::ErrorType::MULTI_API_CALL_TOTAL_FAILURE, "Bro..\nare you even connected to the internet\nIf you are api may be down..");

    const std::string *bs_json = nullptr;
    const std::string *cf_json = nullptr;
    const std::string *earnings_json = nullptr;

    const auto bs_it = fin_map_ref.find("balance-sheet");
    if (bs_it != fin_map_ref.end())
    {
        bs_json = &mutable_fin_map_ref.at("balance-sheet");
    }
    else
    {
        mutable_fin_map_ref["balance-sheet"] = "";
        bs_json = &mutable_fin_map_ref.at("balance-sheet");
    }

    const auto cf_it = fin_map_ref.find("cashflow");
    if (cf_it != fin_map_ref.end())
    {
        cf_json = &mutable_fin_map_ref.at("cashflow");
    }
    else
    {
        mutable_fin_map_ref["cashflow"] = "";
        cf_json = &mutable_fin_map_ref.at("cashflow");
    }

    const auto earning_it = fin_map_ref.find("earnings");
    if (earning_it != fin_map_ref.end())
    {
        earnings_json = &mutable_fin_map_ref.at("earnings");
    }
    else
    {
        mutable_fin_map_ref["earnings"] = "";
        earnings_json = &mutable_fin_map_ref.at("earnings");
    }

    // GRAB ONE OF THE RESPONSES AND CHECK FOR APIKEY CONFIMATION
    if (!bs_json->empty())
        confirm_apikey(mutable_fin_map_ref.at("balance-sheet"), mutable_fin_map_ref);

    // TICKER NOT SUPPORTED
    for (const auto &pair : fin_map_ref)
    {
        const std::string &ref = pair.second;
        if (jsonparseops->check_financials_availability(ref))
        {
            std::ostringstream oss;
            oss << "Ticker " << ticker << " is unsupported, sorry";
            std::string message = oss.str();
            throw BackendException(BackendException::ErrorType::FINANCIALS_TICKER_UNSUPPORTED, message);
        }
    }

    // EMPTY JSONS
    if (bs_json->empty() || cf_json->empty() || earnings_json->empty())
    {
        std::string bs_string;
        std::string cf_string;
        std::string earning_string;
        (bs_json->empty()) ? bs_string = "- balance sheet empty\n" : bs_string = "";
        (cf_json->empty()) ? cf_string = "- cashflow statement empty\n" : cf_string = "";
        (earnings_json->empty()) ? earning_string = "- earnings empty\n" : earning_string = "";
        std::ostringstream oss;
        oss << "Something went wrong\n " << bs_string << cf_string << earning_string;
        std::string message = oss.str();
        throw BackendException(BackendException::ErrorType::FINANICALS_JSON_EMPTY, message);
    }

    // INSTANTIATE StockFinancial CLASS AND POPULATE IT USING JSONPARSEOPS CLASS THEN MOVE IT TO JSONPARSE VECTOR
    StockFinancials stock_fin(ticker);

    if (!jsonparseops->parse_balancesheet_statement((*bs_json), ticker, stock_fin))
    {
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Parsing Failed for balance sheet, try again");
    }

    if (!jsonparseops->parse_cashflow_statement((*cf_json), ticker, stock_fin))
    {
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Parsing Failed for cashflow, try again");
    }
    if (!jsonparseops->parse_earnings_statement((*earnings_json), ticker, stock_fin))
    {
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Parsing Failed for earnings statement, try again");
    }
    {
        // MOVE stock_fin to a vector that JsonParseOps holds
        std::vector<StockFinancials> &mutable_stock_fin_vec = jsonparseops->get_mutable_stock_financials_vec();
        mutable_stock_fin_vec.push_back(std::move(stock_fin));
    }
    requestops->multicurl_cleanup();
    return true;
}

// PUBLIC
bool Backend::run_generate_summary_operations(const std::string &ticker)
{
    // CLEAR MAP FIRST
    auto &summary_map_ref = requestops->get_mutable_summary_map_ref();
    if (!summary_map_ref.empty())
    {
        summary_map_ref.clear();
    }
    encryptops->clear_map();
    std::string api_key = "";
    api_key = run_decrypt_apikey_operations();
    encryptops->clear_map();

    if (!requestops->perform_single_request_summary(ticker, api_key))
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "summary api call failed");

    const std::unordered_map<std::string, std::string> &sum_map_ref = requestops->get_immutable_summary_map_ref();

    if (sum_map_ref.empty())
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "bro..\nare you connected to the internet\nif so api might be down");

    const std::string &returned_json = sum_map_ref.at("summary");

    if (!sum_map_ref.empty())
        confirm_apikey(returned_json, summary_map_ref);

    Metrics metric(ticker);
    Metrics &metric_ref = metric;

    std::string quote_type = "";

    if (!jsonparseops->set_metric_quote_type(returned_json, metric_ref)) // redundancy
    {
        quote_type = get_quote_type_from_stockinfo_vec(ticker, quote_type);
        metric.set_quote_type(quote_type);
    }
    else
    {
        quote_type = metric.get_quote_type();
    }

    // PARSING CATERED ACCORDING TO THE TYPE OF STOCK IM LOOKING AT EITHER ETF EQUITY OR CRYPTO
    if (quote_type == "EQUITY")
    {
        if (!jsonparseops->parse_type_equity_summary(returned_json, metric_ref))
        {
            throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE_SUMMARY, "something went wrong\nfailed to parse equity summary");
        }
    }
    else if (quote_type == "ETF")
    {
        // run etf parsing code here
        if (!jsonparseops->parse_type_etf_summary(returned_json, metric_ref))
        {
            throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE_SUMMARY, "something went wrong\nfailed to parse ETF summary");
        }
    }
    else if (quote_type == "CRYPTOCURRENCY")
    {
        if (!jsonparseops->parse_summary_crypto(returned_json, metric_ref))
        {
            throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE_SUMMARY, "something went wrong\nfailed to parse crypto summary");
        }
    }
    else // if i cant get a quote type ill just run this and get what i can get then throw an error explaining the issues
    {
        if (!jsonparseops->parse_type_equity_summary(returned_json, metric_ref))
        {
            std::ostringstream oss;
            oss << "Something went wrong\n"
                << quote_type << " unsupported";
            throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE_SUMMARY, oss.str());
        }
    }

    {
        std::vector<Metrics> &mutable_met_vec = jsonparseops->get_mutable_metrics_vec();
        mutable_met_vec.push_back(std::move(metric));
    }

    if (quote_type.empty() && quote_type != "CRYPTOCURRENCY" && quote_type != "EQUITY" && quote_type != "ETF")
    {
        throw BackendException(BackendException::ErrorType::NO_EQUITY_TYPE, "Couldn't derive a security type for stock\nran equity parsing as default\nsome information may be missing..sorry");
    }

    return true;
}

// PUBLIC
bool Backend::run_charting_operations(const std::string &ticker, const std::string &requested_yr)
{
    encryptops->clear_map();
    std::string api_key = "";
    api_key = run_decrypt_apikey_operations();
    encryptops->clear_map();

    int pos = get_chart_obj_position_in_vec(ticker);
    std::vector<ChartInfo> &chart_vec = jsonparseops->get_chart_info_vec();
    ChartInfo &c_info = chart_vec.at(pos);

    requestops->perform_single_request_charts(ticker, api_key, c_info, requested_yr);

    if (c_info.get_api_chart_error())
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, c_info.get_error_def());

    // FOR INERNET CONNECTION
    std::unordered_map<std::string, std::string> &mutable_chart_response_map = c_info.get_mutable_chart_respone_map();
    if (mutable_chart_response_map.empty())
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "bro..\nare you even connected to the internet?\nif so api might be down");

    // CONFIRM API KEY
    auto it = mutable_chart_response_map.find(ticker);
    if (it != mutable_chart_response_map.end())
    {
        const std::string &response = mutable_chart_response_map.at(ticker);
        confirm_apikey(response, mutable_chart_response_map);
    }

    if (!jsonparseops->parse_chart_response(ticker, c_info, requested_yr))
    {
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "parse failed in charting operations");
    }
    std::vector<double> &price_vec = c_info.get_price_data_ref(requested_yr);

    // avg price  and below avg vec is for custom shading of the chart
    double avg = (std::accumulate(price_vec.begin(), price_vec.end(), 0.0f)) / price_vec.size();
    std::vector<double> &below_avg_vec = c_info.get_below_avg_vec(requested_yr);

    c_info.set_avg_price(avg, requested_yr);

    for (auto it = price_vec.begin(); it != price_vec.end(); it++)
    {

        if (*it < avg)
        {
            below_avg_vec.push_back(*it);
        }
        else
        {
            below_avg_vec.push_back(avg);
        }
    }

    // IF IM PARSING ITS DISPLAYING ITS ALL PART OF THE PROCESS, USING THIS FOR MY GUIWINDOWOPS CHART MANAGAGER
    // SEE guiWindowOps::run_chart_vec_manager() FOR MORE INFO
    c_info.set_is_displaying(true);
    return true;
}

// PUBLIC
void Backend::run_etf_holdings_operations(const std::string &ticker)
{
    // REQUEST OPS MAP OPERATIONS
    std::unordered_map<std::string, std::string> &map = requestops->get_holdings_map();
    // CLEAR MAP BEFORE POPULATING
    if (!map.empty())
        map.clear();

    // PUT THIS IN A FUNCTION REFRACTOR CODE
    encryptops->clear_map();
    std::string api_key = "";
    api_key = run_decrypt_apikey_operations();
    encryptops->clear_map();

    // THIS POPULATES MAP AND MANUALLY DELETES THINGS IF RETURN WAS BAD
    if (!requestops->perform_multi_request_holdings(ticker, api_key))
    {
        requestops->multicurl_cleanup();
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "API call failed, Try again?");
    }

    requestops->multicurl_cleanup();

    // CHECK IF EMPTY FROM MANUAL DELETE FUNCTION IN REQUEST OPS. IF ITS EMPTY NO RESPONSES WERE RECIEVED
    if (map.empty())
        throw BackendException(BackendException::ErrorType::API_CALL_FAILED, "bro..\nare you even connected to the internet?\nif so API may be down");

    const std::string &top_holdings_json = map.at("top-holdings");
    confirm_apikey(top_holdings_json, map);

    // ETF_HOLDINGS OBJECT MAP OPERATIONS
    ETF_Holdings eh(ticker);
    ETF_Holdings &eh_ref = eh;

    JsonParseOps::JSON_CODES holdings_parse_code = jsonparseops->parse_etf_response_topholdings(top_holdings_json, eh_ref);
    if (holdings_parse_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED || holdings_parse_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED)
    {
        map.clear();
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Something went wrong with holdings\nTry again?");
    }

    JsonParseOps::JSON_CODES sector_json_code = jsonparseops->parse_etf_sector_weightings(top_holdings_json, eh_ref);
    if (sector_json_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED || sector_json_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED)
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "sector json parse failure\ntry again?");

    const std::string &profile_json = map.at("profile");
    JsonParseOps::JSON_CODES profile_parse_code = jsonparseops->parse_etf_response_profile(profile_json, eh_ref);
    if (profile_parse_code == JsonParseOps::JSON_CODES::JSON_STREAM_FAILED || holdings_parse_code == JsonParseOps::JSON_CODES::JSON_PARSE_FAILED)
    {
        map.clear();
        throw BackendException(BackendException::ErrorType::JSON_PARSE_FAILURE, "Something went wrong with parsing profile\nTry again?");
    }

    // // SETTING OTHER HOLDINGS VALUE AND OTHER INDUSTRY WEIGHT VALUE
    const std::unordered_map<std::string, float> &percent = eh.get_holdings_float();

    if (!percent.empty())
    {
        float total_holdings = 0.00;
        for (const auto &p : percent)
            total_holdings += p.second;
        // JUST IN CASE IF TOTAL HOLDINGS IS GREATER THAN 100 I DONT WANT A NEGATIVE OTHER HOLDINGS NUMBER
        if (total_holdings < 100)
            eh.set_other_holdings(
                (100 - total_holdings));
    }

    // SECTOR WEIGHTINGS
    const std::vector<double> &sector_weights = eh.get_sector_weights_vec();
    if (!sector_weights.empty())
    {
        float total_industry_weight = 0.00;
        for (const auto &w : sector_weights)
            total_industry_weight += w;
        if (total_industry_weight < 100)
            eh.set_other_industry_weight((100 - total_industry_weight));
    }
    std::vector<ETF_Holdings> *holdings_vec = jsonparseops->get_etf_holdings_vec_ptr();
    holdings_vec->push_back(std::move(eh));

    return;
}

// PUBLIC
bool Backend::run_api_confirmation_window_operations(GLuint *out_texture, int *out_width, int *out_height)
{
    bool image_prep = fileops->prep_image_file_data();

    if (!image_prep)
        return image_prep;

    const std::vector<unsigned char> *data_ptr = fileops->get_file_data();
    const std::size_t &fsize = fileops->get_file_size();
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load_from_memory(data_ptr->data(), (int)fsize, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;
    return true;
}

// PUBLIC
bool Backend::summary_already_generated(const std::string &ticker)
{
    const std::vector<Metrics> &immut_met_vec = jsonparseops->get_immutable_metrics_vec();
    for (const auto &s : immut_met_vec)
    {
        if (s.get_ticker() == ticker)
        {
            return true;
        }
    }
    return false;
}

// PUBLIC
bool Backend::etf_holdings_already_generated(const std::string &ticker)
{
    const std::vector<ETF_Holdings> *holding_vec = jsonparseops->get_etf_holdings_vec_ptr();
    for (const auto &obj : *holding_vec)
    {
        if (obj.get_ticker() == ticker)
            return true;
    }
    return false;
}

// PUBLIC
size_t
Backend::get_metrics_position_in_vec(const std::string &ticker)
{

    const std::vector<Metrics> &immut_met_vec = jsonparseops->get_immutable_metrics_vec();
    for (auto it = immut_met_vec.begin(); it != immut_met_vec.end(); it++)
    {
        if (it->get_ticker() == ticker)
        {
            return std::distance(immut_met_vec.begin(), it);
        }
    }
    return -1;
}

// PUBLIC
int Backend::get_chart_obj_position_in_vec(const std::string &ticker)
{
    const std::vector<ChartInfo> &vec = jsonparseops->get_immutable_chart_info_vec();
    for (auto it = vec.begin(); it != vec.end(); it++)
    {
        if (it->get_ticker() == ticker)
        {

            return std::distance(vec.begin(), it);
        }
    }

    return -1;
};

// PUBLIC
int Backend::get_holdings_position_in_vec(const std::string &ticker)
{
    const std::vector<ETF_Holdings> *holdings_vec = jsonparseops->get_etf_holdings_vec_ptr();
    auto it = std::find_if(holdings_vec->begin(), holdings_vec->end(), [ticker](const ETF_Holdings &e)
                           { return e.get_ticker() == ticker; });
    if (it == holdings_vec->end())
        return -1;
    return std::distance(holdings_vec->begin(), it);
}

// PRIVATE
JsonParseOps::JSON_CODES Backend::run_internal_parsing_operations(const std::string &response, const std::string &ticker_copy)
{
    JsonParseOps::JSON_CODES info_avail_code = jsonparseops->information_unavailable_check(response);
    if (info_avail_code == JsonParseOps::JSON_CODES::INFORMATION_UNAVAILABLE)
    {
        jsonparseops->add_default_stockinfo_node_to_stockinformation(ticker_copy);
        return info_avail_code;
    }
    // confirm ticker is valid
    JsonParseOps::JSON_CODES code = jsonparseops->ticker_confirmed(response);

    if (code == JsonParseOps::JSON_CODES::TICKER_CONFIMED)
    {
        JsonParseOps::JSON_CODES parse_code = jsonparseops->parse_watchlist_item(response);

        return parse_code;
    }

    return code;
}

// PUBLIC
bool Backend::financial_report_already_generated(const std::string &ticker)
{
    const std::vector<StockFinancials> &stock_fin_bucket = jsonparseops->get_immutable_stock_financials_vec();
    for (const auto &fin_report : stock_fin_bucket)
    {
        if (fin_report.get_ticker() == ticker)
        {
            return true;
        }
    }
    return false;
}

// PUBLIC
bool Backend::chart_already_generated(const std::string &ticker)
{
    const std::vector<ChartInfo> &chart_info_vec = jsonparseops->get_immutable_chart_info_vec();

    for (const auto &obj : chart_info_vec)
    {
        if (obj.get_ticker() == ticker)
        {
            return true;
        }
    }
    return false;
};

// PUBLIC
void Backend::add_chart_obj_to_vector(const std::string &ticker)
{
    ChartInfo chartinfo(ticker);
    std::vector<ChartInfo> &chart_vec = jsonparseops->get_chart_info_vec();
    chart_vec.push_back(std::move(chartinfo));
}

std::vector<StockFinancials> *Backend::pass_stock_financial_vec_ptr_non_const()
{
    return jsonparseops->get_financial_vec_ptr_non_const();
}

std::vector<Metrics> *Backend::pass_metrics_ptr()
{
    return jsonparseops->get_metrics_vec_ptr();
}

std::vector<ChartInfo> *Backend::pass_chart_info_vec_ptr()
{
    return jsonparseops->get_chart_info_vec_ptr();
}

std::vector<ETF_Holdings> *Backend::pass_etf_holdings_vec_ptr()
{
    return jsonparseops->get_etf_holdings_vec_ptr();
}

// std::vector<StockInfo> *Backend::pass_stockinfo_vec_ptr()
// {
//     return jsonparseops->;
// }

// PRIVATE
std::string Backend::get_quote_type_from_stockinfo_vec(const std::string &ticker, const std::string &quote_type)
{
    // run a loop through stockinfo vec to get quote type
    std::vector<StockInfo> &immut_vec = jsonparseops->get_mutable_stockinfo_vec();
    for (auto &s : immut_vec)
    {
        if (s.get_ticker() == ticker)
        {
            return s.get_quote_type_non_const();
        }
    }
    return "";
}

int Backend::find_column(const std::string &date, StockFinancials::BalanceSheetItems *ptr)
{
    return jsonparseops->find_column(date, ptr);
}

int Backend::find_column(const std::string &date, StockFinancials::CashflowItems *ptr)
{
    return jsonparseops->find_column(date, ptr);
}
