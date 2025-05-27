#include "JsonParseOps.h"

bool JsonParseOps::in_stock_information_vec(const std::string &ticker)

{
    auto it = std::find_if(stock_information.begin(), stock_information.end(), [&ticker](StockInfo &node)
                           { return node.get_ticker() == ticker; });

    return (it != stock_information.end());
}

int JsonParseOps::find_column(const std::string &date, StockFinancials::BalanceSheetItems *ptr)
{

    for (const auto &p : ptr->dates_tracker)
    {
        if (date == p.second)
        {
            return p.first;
        }
    }
    return -1;
}

int JsonParseOps::find_column(const std::string &date, StockFinancials::CashflowItems *ptr)
{

    for (const auto &p : ptr->dates_tracker)
    {
        if (date == p.second)
        {
            return p.first;
        }
    }
    return -1;
}

JsonParseOps::JSON_CODES JsonParseOps::parse_watchlist_item(const std::string &returned_json)
{

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;
    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;
    // returning a reference to quoteSummary
    if (jsonData.empty() || jsonData.isNull())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    const Json::Value &root = jsonData["quoteSummary"];
    if (root.empty() || root.isNull())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    const Json::Value &result_array = root["result"];
    if (result_array.empty() || result_array.isNull() || !(result_array.isArray()))
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    const Json::Value &array_item = result_array[0];
    if (array_item.empty() || array_item.isNull())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    const Json::Value &price = array_item["price"];
    if (price.empty() || price.isNull() || !(array_item.isObject()))
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    // organize values
    std::string tickerBucket, companyNameBucket, quoteTypeBucket;
    double marketPriceBucket, marketChangeBucket;

    const Json::Value &company_n = price["shortName"];
    const Json::Value &ticker_b = price["symbol"];
    const Json::Value &market_p = price["regularMarketPrice"];
    const Json::Value &market_c = price["regularMarketChange"];
    const Json::Value &quote_t = price["quoteType"];
    (company_n.isNull()) ? companyNameBucket = "N/A" : companyNameBucket = price["shortName"].asString();
    (ticker_b.isNull()) ? tickerBucket = "N/A" : tickerBucket = price["symbol"].asString();
    (market_p.isNull()) ? marketPriceBucket = 0.00 : marketPriceBucket = std::round(price["regularMarketPrice"].asDouble() * 100) / 100;
    (market_c.isNull()) ? marketChangeBucket = 0.00 : marketChangeBucket = std::round(price["regularMarketChange"].asDouble() * 100) / 100;
    (quote_t.isNull()) ? quoteTypeBucket = "N/A" : quoteTypeBucket = price["quoteType"].asString();

    // if its already in there just return true
    if (in_stock_information_vec(tickerBucket))
        return JsonParseOps::JSON_CODES::ALREADY_IN_INFO_VEC;

    // if not in there add it sort it then return true
    stock_information.push_back(StockInfo(companyNameBucket, tickerBucket, marketPriceBucket, marketChangeBucket, quoteTypeBucket));
    std::sort(stock_information.begin(), stock_information.end(), customSort);

    return JsonParseOps::JSON_CODES::PUSHED_TO_INFO_VEC;
}

std::vector<StockInfo> &JsonParseOps::get_mutable_stockinfo_vec()
{
    return stock_information;
}

const std::vector<StockInfo> &JsonParseOps::get_immutable_stockinfo_vec() const
{
    return stock_information;
}

const std::vector<StockFinancials> &JsonParseOps::get_immutable_stock_financials_vec()
{
    return stock_financials_bucket;
}

std::vector<StockFinancials> &JsonParseOps::get_mutable_stock_financials_vec()
{
    return stock_financials_bucket;
}

std::vector<StockInfo> *JsonParseOps::get_stockinfo_vec_ptr()
{
    return &stock_information;
}

bool JsonParseOps::confirm_deletion(const std::string &ticker)
{

    auto it = std::find_if(stock_information.begin(), stock_information.end(), [&ticker](StockInfo &node)
                           { return node.get_ticker() == ticker; });
    return (it == stock_information.end());
}

JsonParseOps::JSON_CODES JsonParseOps::apikey_confimed(const std::string &response)
{
    std::string match = "You are not subscribed to this API.";
    std::string match_two = "Too many requests";
    std::string match_three = "Invalid API key.";
    std::string extracted_message = "";
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &root, &errs))
    {
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;
    }

    if (root.isNull() || root.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    Json::Value &message = root["message"];

    if (message.isString())
    {
        extracted_message = message.asString();
        std::size_t pos = extracted_message.find(match);
        if (pos != std::string::npos)
            return JsonParseOps::JSON_CODES::API_KEY_FAILED;

        std::size_t pos_two = extracted_message.find(match_two);
        if (pos_two != std::string::npos)
        {
            return JsonParseOps::JSON_CODES::API_KEY_FAILED;
        }
        std::size_t pos_three = extracted_message.find(match_three);
        if (pos_three != std::string::npos)
            return JsonParseOps::JSON_CODES::API_KEY_FAILED;
    }

    return JsonParseOps::JSON_CODES::API_KEY_CONFIRMED;
}

JsonParseOps::JSON_CODES JsonParseOps::ticker_confirmed(const std::string &response)
{
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream stream(response);

    if (!Json::parseFromStream(reader, stream, &root, &errs))
    {
        return JSON_CODES::JSON_STREAM_FAILED;
    }

    if (root.isNull() || root.empty())
    {
        return JSON_CODES::JSON_PARSE_FAILED;
    }

    const Json::Value &quote_summary = root["quoteSummary"];
    if (quote_summary.isNull() || quote_summary.empty())
    {
        return JSON_CODES::JSON_PARSE_FAILED;
    }
    const Json::Value &results = quote_summary["result"];
    if (results.isNull() || !(results.isArray()))
    {
        return JSON_CODES::JSON_PARSE_FAILED;
    }
    const Json::Value &array_item = results[0];
    const Json::Value &errormsg = array_item["message"];

    if (!errormsg.isNull())
    {
        return JSON_CODES::TICKER_UNCONFIMED;
    }

    return JSON_CODES::TICKER_CONFIMED;
}

JsonParseOps::JSON_CODES JsonParseOps::information_unavailable_check(const std::string &response)
{

    std::string match = "source currently unavailable";
    std::string extracted = "";
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream stream(response);

    if (!Json::parseFromStream(reader, stream, &root, &errs))
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;

    if (root.isNull() || root.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    Json::Value quote_summary = root["quoteSummary"];
    if (quote_summary.isNull() || quote_summary.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    Json::Value results = quote_summary["result"];
    if (results.isNull() || !(results.isArray()) || results.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    if (!results.isNull() && results.isArray() && !results.empty())
    {
        const Json::Value &array_item = results[0];
        const Json::Value &errmsg = array_item["message"];
        if (!errmsg.isNull())
        {
            extracted = errmsg.asString();
        }
    }

    std::size_t pos = extracted.find(match);
    // if it doesnt equal npos that means it found it so that means its unavail so unavail is true
    return (pos != std::string::npos) ? JsonParseOps::JSON_CODES::INFORMATION_UNAVAILABLE : JsonParseOps::JSON_CODES::INFORMATION_CONFIRMED;
}

bool JsonParseOps::check_financials_availability(const std::string &returned_json)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return false;
    }

    // IF NOT AVAILABLE RESPONSE WOULD LOOK LIKE: "Quote not found for symbol: [TICKER]"
    return jsonData.isString();
}

void JsonParseOps::add_default_stockinfo_node_to_stockinformation(const std::string &ticker)
{
    stock_information.push_back(StockInfo("unavail", ticker, 0.00, 0.00, "N/A"));
}

void JsonParseOps::clean_resonse(std::string &response)
{
    if (response[0] != '{')
    {
        response = "{" + response;
    }

    bool v = response[response.length() - 2] == '}';
    bool w = response[response.length() - 1] == '}';
    if (!(v && w))
    {
        response = response + "}";
    }
}

bool JsonParseOps::parse_balancesheet_statement(const std::string &returned_json, const std::string &ticker, StockFinancials &stock_fin)
{

    StockFinancials::BalanceSheetItems *ptr = stock_fin.get_balancesheet_ptr();
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;
    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return false;
    }
    if (jsonData.isNull() || jsonData.empty())
        return false;
    //  GETTING THE DATES FOR BALANCE SHEET
    const Json::Value &date_root = jsonData["balanceSheetHistory"];
    if (date_root.isNull() || date_root.empty())
        return false;

    const Json::Value &date_array = date_root["balanceSheetStatements"];
    if (date_array.isNull() || date_array.empty() || !date_array.isArray())
        return false;

    if (date_array.isArray())
    {
        int balance_sheet_date_entry = 1;
        for (const auto &obj : date_array)
        {
            const Json::Value &end_date = obj["endDate"];
            if (end_date.isObject() && !end_date.empty())
            {
                ptr->dates_tracker.insert_or_assign(balance_sheet_date_entry, end_date["fmt"].asString());
                ptr->column_tracker.insert_or_assign(balance_sheet_date_entry, false);
                balance_sheet_date_entry++;
            }
        }
    }

    // GETTING BALANCE SHEET ITEMS

    Json::Value &timeseries = jsonData["timeseries"];

    for (Json::Value::const_iterator itr = timeseries.begin(); itr != timeseries.end(); ++itr)
    {
        const Json::Value &accounting_entry = itr.name();
        const Json::Value &entry_array = *itr;
        if (entry_array.isArray())
        {

            // CREATING NESTED DICTIONARY

            ptr->reporting_map[accounting_entry.asString()];
            // CREATING THE KEY //this holds the keys in order since unordered map is well.. unordered
            ptr->accounting_items.push_back(accounting_entry.asString());

            for (const Json::Value &mini_obj : entry_array)
            {
                const Json::Value &reported_value_obj = mini_obj["reportedValue"];
                // ADDING DATE AND REPORTED VALUE TO THE RECENTLY ADDED KEY
                if (!mini_obj.isNull() && !mini_obj.empty())
                {
                    const Json::Value &fmt = reported_value_obj["fmt"];
                    if (!fmt.isNull())
                        ptr->reporting_map[accounting_entry.asString()].insert_or_assign(mini_obj["asOfDate"].asString(), reported_value_obj["fmt"].asString());
                    else
                        ptr->reporting_map[accounting_entry.asString()].insert_or_assign(mini_obj["asOfDate"].asString(), "N/A");
                }
            }
        }
    }

    return true;
}

bool JsonParseOps::parse_cashflow_statement(const std::string &returned_json, const std::string &ticker, StockFinancials &stock_fin)
{

    StockFinancials::CashflowItems *ptr = stock_fin.get_cashflow_ptr();

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return false;
    }
    if (jsonData.isNull() || jsonData.empty())
        return false;

    //  GETTING THE DATES FOR BALANCE SHEET
    const Json::Value &date_root = jsonData["cashflowStatementHistory"];
    if (date_root.isNull() || date_root.empty())
        return false;
    const Json::Value &date_array = date_root["cashflowStatements"];
    if (date_array.isNull() || date_array.empty() || !date_array.isArray())
        return false;
    if (date_array.isArray())
    {
        int balance_sheet_date_entry = 1;
        for (const auto &obj : date_array)
        {
            const Json::Value &end_date = obj["endDate"];
            if (end_date.isObject() && !end_date.empty())
            {
                ptr->dates_tracker.insert_or_assign(balance_sheet_date_entry, end_date["fmt"].asString());
                ptr->column_tracker.insert_or_assign(balance_sheet_date_entry, false);
                balance_sheet_date_entry++;
            }
        }
    }

    // GETTING INDIVIDUAL ENTRIES SHEET ITEMS

    Json::Value &timeseries = jsonData["timeseries"];
    if (timeseries.isNull() || timeseries.empty())
        return false;
    for (Json::Value::const_iterator itr = timeseries.begin(); itr != timeseries.end(); ++itr)
    {
        const Json::Value &accounting_entry = itr.name();
        const Json::Value &entry_array = *itr;
        if (entry_array.isArray())
        {

            // CREATING NESTED DICTIONARY ADDING TO ACCOUNTING ITEMS
            std::string acct_entry = accounting_entry.asString();
            // THE REGEX IS SKIPPING OVER TRAILING ACCOUTING ENTRIES IN JSON
            std::regex pattern("trailing");
            if (!std::regex_search(acct_entry, pattern))
            {
                ptr->reporting_map[accounting_entry.asString()];
                ptr->accounting_items.push_back(accounting_entry.asString());
                for (const Json::Value &mini_obj : entry_array)
                {
                    if (!mini_obj.isNull() && !mini_obj.empty())
                    {
                        const Json::Value &reported_value_obj = mini_obj["reportedValue"];
                        if (!reported_value_obj.isNull() && !reported_value_obj.empty())
                        {
                            const Json::Value &fmt = reported_value_obj["fmt"];
                            if (!fmt.isNull())
                                ptr->reporting_map[accounting_entry.asString()].insert_or_assign(mini_obj["asOfDate"].asString(), reported_value_obj["fmt"].asString());
                            else
                                ptr->reporting_map[accounting_entry.asString()].insert_or_assign(mini_obj["asOfDate"].asString(), "N/A");
                        }
                    }
                }
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool JsonParseOps::parse_earnings_statement(const std::string &returned_json, const std::string &ticker, StockFinancials &stock_fin)
{
    StockFinancials::EarningsItems *ptr = stock_fin.get_earnings_ptr();

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return false;
    }

    if (jsonData.isNull() || jsonData.empty())
        return false;

    const Json::Value &root = jsonData["earnings"];
    if (root.isNull() || root.empty())
        return false;

    const Json::Value &qt_eps_root = root["earningsChart"];
    if (qt_eps_root.isNull() || qt_eps_root.empty())
        return false;

    const Json::Value &qt_eps = qt_eps_root["quarterly"];
    if (qt_eps.isNull() || qt_eps.empty())
        return false;

    if (qt_eps.isArray())
    {
        for (const auto &dict : qt_eps)
        {
            const Json::Value &date = dict["date"];
            const Json::Value &acutual = dict["actual"];
            const Json::Value &estimate = dict["estimate"];
            std::unordered_map<std::string, float> estimate_actual_map;
            estimate_actual_map.insert_or_assign("actual", acutual.asFloat());
            estimate_actual_map.insert_or_assign("estimate", estimate.asFloat());
            ptr->quarterly_reporting_map[date.asString()] = estimate_actual_map;
            ptr->quartely_dates_inorder.push_back(date.asString());
        }
    }

    // YEARLY EARNINGS REPORT AND PERCENTAGE ACCUMULATION
    const Json::Value &fin_charts = root["financialsChart"];
    const Json::Value &yearly = fin_charts["yearly"]; // <- Array of dicts (python speak)
    if (yearly.isArray())
    {
        for (const auto &dict : yearly)
        {
            const Json::Value &date = dict["date"];
            const Json::Value &earnings = dict["earnings"];
            const Json::Value &revenue = dict["revenue"];
            ptr->annual_dates_inorder.push_back(date.asString());
            std::unordered_map<std::string, long long int> revenue_income_map;
            revenue_income_map.insert_or_assign("earnings", earnings.asLargestInt());
            revenue_income_map.insert_or_assign("revenue", revenue.asLargestInt());
            ptr->annual_reporting_map.insert_or_assign(date.asString(), revenue_income_map);
            double percent = (earnings.asDouble() / revenue.asDouble()) * 100;
            double converted_percent = std::round(percent * 100.0) / 100.0;
            ptr->percent_vec.push_back(converted_percent);
        }
    }
    return true;
}

bool JsonParseOps::set_metric_quote_type(const std::string &returned_json, Metrics &m)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;
    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return false;
    }
    if (jsonData.isNull() || jsonData.empty())
        return false;
    const Json::Value &price = jsonData["price"];
    if (price.isNull() || price.empty() || !(price.isObject()))
        return false;
    const Json::Value &quote_type_alt = jsonData["quoteType"];

    if (price.isObject())
    {
        const Json::Value &quote_type = price["quoteType"];
        if (!quote_type.isNull())
        {
            m.set_quote_type(quote_type.asString());
        }
        else
        {
            if (quote_type_alt.isObject()) // redundancy
            {
                const Json::Value &q = quote_type_alt["quoteType"];
                if (!q.isNull())
                {
                    m.set_quote_type(q.asString());
                }
            }
        }
    }
    return true;
}

bool JsonParseOps::parse_type_equity_summary(const std::string &returned_json, Metrics &m)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::unordered_map<std::string, std::string> equity_key_converter{
        {"open", "open"},
        {"bid", "bid"},
        {"ask", "ask"},
        {"dayHigh", "day high"},
        {"dayLow", "day low"},
        {"volume", "volume"},
        {"marketCap", "market cap"},
        {"trailingPE", "trailing P/E"},
        {"forwardPE", "forward P/E"},
        {"fiftyTwoWeekHigh", "52 wk high"},
        {"fiftyTwoWeekLow", "52 wk low"},
        {"fiftyDayAverage", "52 day avg"},

    };

    std::string equity_keys[12]{
        "open",
        "bid",
        "ask",
        "dayHigh",
        "dayLow",
        "volume",
        "marketCap",
        "trailingPE",
        "forwardPE",
        "fiftyTwoWeekHigh",
        "fiftyTwoWeekLow",
        "fiftyDayAverage",
    };

    // vector holds multiple single key : pair objects
    std::vector<std::unordered_map<std::string, std::string>> *metrics_bucket = m.get_metrics_bucket_ptr_non_const();

    std::istringstream stream(returned_json);

    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
        return false;

    if (jsonData.isNull() || jsonData.empty())
        return false;

    const Json::Value &summary = jsonData["summaryDetail"];

    if (summary.isNull() || summary.empty())
        return false;

    for (int i = 0; i < 12; i++)
    {
        std::unordered_map<std::string, std::string> generic_map;

        if (!summary[equity_keys[i]].isNull())
        {
            if (summary[equity_keys[i]].asLargestInt() > 1e3)
            {

                generic_map.insert_or_assign(
                    equity_key_converter[equity_keys[i]], shorten_number(summary[equity_keys[i]].asLargestInt()));
                metrics_bucket->push_back(std::move(generic_map));
            }
            else
            {
                double value = summary[equity_keys[i]].asDouble();
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << value;
                generic_map.insert_or_assign(
                    equity_key_converter[equity_keys[i]], oss.str());
                metrics_bucket->push_back(std::move(generic_map));
            }
        }
        else
        {
            generic_map.insert_or_assign(equity_keys[i], "N/A");
            metrics_bucket->push_back(std::move(generic_map));
        }
    }
    return true;
}

bool JsonParseOps::parse_type_etf_summary(const std::string &returned_json, Metrics &m)
{

    int key_count = 12;
    std::vector<std::unordered_map<std::string, std::string>> *metric_ptr = m.get_metrics_bucket_ptr_non_const();
    std::unordered_map<std::string, std::string> key_converter{
        {"open", "open"},
        {"dayHigh", "day high"},
        {"dayLow", "day low"},
        {"trailingPE", "trailing P/E"},
        {"fiftyTwoWeekHigh", "52 wk high"},
        {"fiftyTwoWeekLow", "52 wk low"},
        {"trailingAnnualDividendRate", "Dividend Rate"},
        {"yield", "yield"},
        {"fiveYearAverageReturn", "5yr avg retrun"},
        {"category", "category"},
        {"totalAssets", "AUM"},
        {"fundFamily", "fund family"}

    };

    std::string keys[key_count]{
        "open",
        "dayHigh",
        "dayLow",
        "fiftyTwoWeekHigh",
        "fiftyTwoWeekLow",
        "trailingPE",
        "trailingAnnualDividendRate",
        "yield",
        "fiveYearAverageReturn",
        "totalAssets",
        "category",
        "fundFamily"

    };

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return false;
    }
    if (jsonData.isNull() || jsonData.empty() || !(jsonData.isObject()))
        return false;

    const Json::Value &summary_detail = jsonData["summaryDetail"];
    if (summary_detail.isNull() || summary_detail.empty())
        return false;
    const Json::Value &key_stats = jsonData["defaultKeyStatistics"];
    if (key_stats.isNull() || key_stats.empty())
        return false;

    for (int i = 0; i < key_count; i++)
    {
        std::unordered_map<std::string, std::string> generic_map;
        if (i < 7)
        {
            if (!summary_detail[keys[i]].isNull())
            {
                std::ostringstream oss;
                double value = summary_detail[keys[i]].asDouble();
                oss << std::fixed << std::setprecision(2) << value;
                std::string entry = oss.str();
                generic_map.insert_or_assign(key_converter[keys[i]], entry);
            }
            else
            {
                generic_map.insert_or_assign(key_converter[keys[i]], "N/A");
            }
        }
        else if (i == 7 || i == 8)
        {
            if (!key_stats[keys[i]].isNull())
            {
                std::ostringstream oss;
                double value = key_stats[keys[i]].asDouble() * 100;
                oss << std::fixed << std::setprecision(2) << value;
                std::string converted = oss.str() + "%";
                generic_map.insert_or_assign(key_converter[keys[i]], converted);
            }
            else
            {
                generic_map.insert_or_assign(key_converter[keys[i]], "N/A");
            }
        }
        else if (i == 9)
        {
            if (!key_stats[keys[i]].isNull())
            {
                long long int big_value = key_stats[keys[i]].asLargestInt();
                std::string conversion = shorten_number(big_value);
                generic_map.insert_or_assign(key_converter[keys[i]], conversion);
            }
            else
            {
                generic_map.insert_or_assign(key_converter[keys[i]], "N/A");
            }
        }
        else
        {
            (key_stats[keys[i]].isNull()) ? generic_map.insert_or_assign(key_converter[keys[i]], "N/A") : generic_map.insert_or_assign(key_converter[keys[i]], key_stats[keys[i]].asString());
        }
        metric_ptr->push_back(std::move(generic_map));
    }

    return true;
}

bool JsonParseOps::parse_summary_crypto(const std::string &returned_json, Metrics &m)
{
    int key_count = 8;
    std::vector<std::unordered_map<std::string, std::string>> *metric_bucket_ptr = m.get_metrics_bucket_ptr_non_const();
    std::unordered_map<std::string, std::string> key_converter{
        {"dayHigh", "day high"},
        {"dayLow", "day low"},
        {"fiftyTwoWeekHigh", "52 wk high"},
        {"fiftyTwoWeekLow", "52 wk low"},
        {"marketCap", "market cap"},
        {"averageDailyVolume10Day", "10day avg vol"},
        {"volume24Hr", "volume24Hr"},
        {"description", "description:\n"}

    };

    std::string keys[key_count]{
        "marketCap",
        "averageDailyVolume10Day",
        "volume24Hr",
        "dayHigh",
        "dayLow",
        "fiftyTwoWeekHigh",
        "fiftyTwoWeekLow",
        "description"

    };

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
        return false;
    if (jsonData.isNull() || jsonData.empty() || !(jsonData.isObject()))
        return false;
    const Json::Value &price = jsonData["price"];
    if (price.isNull() || price.empty())
        return false;

    const Json::Value &summary_detail = jsonData["summaryDetail"];
    if (summary_detail.isNull() || summary_detail.empty())
        return false;
    const Json::Value &summary_profile = jsonData["summaryProfile"];
    if (summary_profile.isNull() || summary_profile.empty())
        return false;
    for (int i = 0; i < key_count; i++)
    {
        std::unordered_map<std::string, std::string> generic_map;
        if (i < 3) // 0 to 2
        {

            if (!price[keys[i]].isNull())
            {
                std::string value = shorten_number(price[keys[i]].asLargestInt());
                generic_map.insert_or_assign(key_converter[keys[i]], value);
            }
            else
            {
                generic_map.insert_or_assign(key_converter[keys[i]], "N/A");
            }
        }
        else if (i > 2 && i < 7) // 3 to 6
        {
            if (!summary_detail[keys[i]].isNull())
            {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << summary_detail[keys[i]].asDouble();
                generic_map.insert_or_assign(key_converter[keys[i]], oss.str());
            }
            else
            {
                generic_map.insert_or_assign(key_converter[keys[i]], "N/A");
            }
        }
        else
        {
            if ((summary_profile.isObject()) && !(summary_profile.empty()) && !(summary_profile[keys[i]].isNull()))
            {
                generic_map.insert_or_assign(key_converter[keys[i]], summary_profile[keys[i]].asString());
            }
            else
            {
                generic_map.insert_or_assign(key_converter[keys[i]], "N/A");
            }
        }

        metric_bucket_ptr->push_back(std::move(generic_map));
    }
    return true;
}

bool JsonParseOps::parse_chart_response(const std::string &ticker, ChartInfo &c, const std::string &requested_yr)
{

    const std::unordered_map<std::string, std::string> &response_map = c.get_immutable_chart_respone_map();

    const std::string &returned_json = response_map.at(ticker);

    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
        return false;

    if (jsonData.isNull() || jsonData.empty())
        return false;
    Json::Value *root = &jsonData["chart"];
    if (root->isNull() || root->empty())
        return false;
    std::string keys[4]{"result", "indicators", "adjclose", "adjclose"};
    for (int i = 0; i < 4; i++)
    {
        root = &((*root)[keys[i]]);
        if (root->isArray() && !(i == 3))
        {
            root = &((*root)[0]);
        }
        if (root->isNull() || root->empty())
        {
            return false;
        }
    }

    std::string ts_keys[2]{"result", "timestamp"};
    Json::Value *ts_root = &jsonData["chart"];
    if (ts_root->isNull() || ts_root->empty())
        return false;

    for (int i = 0; i < 2; i++)
    {
        ts_root = &((*ts_root)[ts_keys[i]]);
        if (ts_keys[i] != "timestamp" && ts_root->isArray())
        {
            ts_root = &((*ts_root)[0]);
        }
        if (ts_root->isNull() || ts_root->empty())
        {
            return false;
        }
    }

    std::vector<double> &chart_data = c.get_price_data_ref(requested_yr);
    for (auto &v : *root)
    {
        chart_data.push_back(v.asDouble());
    }

    std::vector<double> &ts_ref = c.get_timestamp_ref(requested_yr);
    for (auto &v : *ts_root)
    {
        ts_ref.push_back(v.asDouble());
    }

    {
        std::unordered_map<std::string, std::string> &response_map = c.get_mutable_chart_respone_map();
        response_map.clear();
    }

    return true;
}

JsonParseOps::JSON_CODES JsonParseOps::parse_etf_response_topholdings(const std::string &returned_json, ETF_Holdings &eh)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
    {
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;
    }

    if (jsonData.isNull() || jsonData.empty())
    {
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;
    }

    const Json::Value &topholdings = jsonData["topHoldings"];

    if (topholdings.isNull() || topholdings.empty())
    {
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;
    }

    const Json::Value &holdings = topholdings["holdings"];

    if (holdings.isNull() || !holdings.isArray())
    {
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;
    }

    if (holdings.isArray() && holdings.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_SUCCESS;

    if (holdings.isArray() && !holdings.empty())
    {

        std::unordered_map<std::string, float> &holdings_float_map = eh.get_holdings_float();
        std::unordered_map<std::string, std::string> &holdings_string_map = eh.get_holdings_company_name();
        std::vector<std::string> &holding_keys = eh.get_holidings_keys();

        for (Json::Value::const_iterator itr = holdings.begin(); itr != holdings.end(); ++itr)
        {

            std::string temp_symbol_var;
            float temp_holding_percent;
            std::string temp_name_val;

            Json::Value symbol = (*itr)["symbol"];
            if (symbol.isNull())
                continue;
            else
                temp_symbol_var = symbol.asString();

            Json::Value company_name = (*itr)["holdingName"];
            if (company_name.isNull())
                temp_name_val = "N/A";
            else
                temp_name_val = company_name.asString();

            Json::Value holding_percent = (*itr)["holdingPercent"];
            if (holding_percent.isNull())
                temp_holding_percent = 0.00;
            else
                temp_holding_percent = (holding_percent.asFloat() * 100);

            holdings_float_map[temp_symbol_var] = temp_holding_percent; // HOLDS PERCENT HOLDING
            holdings_string_map[temp_symbol_var] = temp_name_val;       // HOLDS COMPANY NAME
            holding_keys.push_back(std::move(temp_symbol_var));         // KEYS IN ORDER USED TO ACCESS BOTH MAPS
        }
        return JsonParseOps::JSON_CODES::JSON_PARSE_SUCCESS;
    }

    return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;
}

JsonParseOps::JSON_CODES JsonParseOps::parse_etf_response_profile(const std::string &returned_json, ETF_Holdings &eh)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;
    if (jsonData.isNull() || jsonData.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    const Json::Value &summary_profile = jsonData["summaryProfile"];
    if (summary_profile.isNull() || summary_profile.empty())
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;

    std::unordered_map<std::string, std::string> &profile_map = eh.get_profile_map();

    const Json::Value &business_summary = summary_profile["longBusinessSummary"];

    if (business_summary.isNull() || business_summary.empty())
        profile_map["profile"] = "Profile Unavailable";
    else
        profile_map["profile"] = clean_text(business_summary.asString()); // GETS RID OF -null- in text
    // MUST USE IMGUI::TEXTWRAPPED TO GET THIS TO WRAP PROPERLY
    return JsonParseOps::JSON_CODES::JSON_PARSE_SUCCESS;
}

JsonParseOps::JSON_CODES JsonParseOps::parse_etf_sector_weightings(const std::string &returned_json, ETF_Holdings &eh)
{
    Json::CharReaderBuilder reader;
    Json::Value jsonData;
    std::string errs;

    std::istringstream stream(returned_json);
    if (!Json::parseFromStream(reader, stream, &jsonData, &errs))
        return JsonParseOps::JSON_CODES::JSON_STREAM_FAILED;

    if (jsonData.isNull() || jsonData.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;
    const Json::Value &top_holdings = jsonData["topHoldings"];
    if (top_holdings.isNull() || top_holdings.empty())
        return JsonParseOps::JSON_CODES::JSON_PARSE_FAILED;

    const Json::Value &sector_holdings_section = top_holdings["sectorWeightings"];
    if (!sector_holdings_section.isNull() && sector_holdings_section.isArray() && !sector_holdings_section.empty())
    {
        std::vector<std::string> &sector_names = eh.get_sector_names_vec();
        std::vector<double> &sector_weights = eh.get_sector_weights_vec();
        for (Json::Value::const_iterator itr = sector_holdings_section.begin(); itr != sector_holdings_section.end(); itr++)
        {
            const Json::Value &sector_obj = *itr;
            const std::string sector_name = std::string(sector_obj.getMemberNames()[0]);
            sector_names.push_back(sector_name);
            sector_weights.push_back((sector_obj[sector_name].asDouble() * 100));
        }
    }

    return JsonParseOps::JSON_CODES::JSON_PARSE_SUCCESS;
}

std::string JsonParseOps::clean_text(const std::string &text)
{
    std::regex pattern(R"(\s*-null-\s*?.\s*)");
    std::string cleaned_string = std::regex_replace(text, pattern, " ");
    return cleaned_string;
}

std::vector<StockFinancials> *JsonParseOps::get_financial_vec_ptr_non_const()
{
    return &stock_financials_bucket;
}

const std::vector<Metrics> &JsonParseOps::get_immutable_metrics_vec()
{
    return stock_metrics_vec;
}

std::vector<Metrics> &JsonParseOps::get_mutable_metrics_vec()
{
    return stock_metrics_vec;
}

std::vector<Metrics> *JsonParseOps::get_metrics_vec_ptr()
{
    return &stock_metrics_vec;
}

std::vector<ChartInfo> &JsonParseOps::get_chart_info_vec()
{
    return chart_info_vec;
}
const std::vector<ChartInfo> &JsonParseOps::get_immutable_chart_info_vec() const
{
    return chart_info_vec;
}

std::vector<ChartInfo> &JsonParseOps::get_mutable_chart_info_vec()
{
    return chart_info_vec;
}

std::vector<ChartInfo> *JsonParseOps::get_chart_info_vec_ptr()
{
    return &chart_info_vec;
}

std::vector<ETF_Holdings> *JsonParseOps::get_etf_holdings_vec_ptr()
{
    return &etf_holdings_vec;
}

std::vector<ETF_Holdings> &JsonParseOps::get_etf_holdings_vec()
{
    return etf_holdings_vec;
}
