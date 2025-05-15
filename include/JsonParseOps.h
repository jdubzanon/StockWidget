#ifndef _JSONPARSEOPS_H
#define _JSONPARSEOPS_H

#include "StockInfo.h"
#include "StockFinancials.h"
#include "Metrics.h"
#include "ChartInfo.h"
#include "utils.h"
#include <vector>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <curl/curl.h>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <regex>

class JsonParseOps
{
private:
    std::vector<StockInfo> stock_information;
    std::vector<StockFinancials> stock_financials_bucket;
    std::vector<Metrics> stock_metrics_vec;
    std::vector<ChartInfo> chart_info_vec;

private:
    bool in_stock_information_vec(const std::string &ticker);

    struct sortMethod
    {
        bool operator()(StockInfo a, StockInfo b) { return a.get_ticker() < b.get_ticker(); }
    } customSort;

public:
    enum class JSON_CODES
    {
        JSON_STREAM_FAILED,
        JSON_PARSE_FAILED,
        TICKER_UNCONFIMED,
        TICKER_CONFIMED,
        INFORMATION_UNAVAILABLE,
        INFORMATION_CONFIRMED,
        ALREADY_IN_INFO_VEC,
        PUSHED_TO_INFO_VEC,
        API_KEY_FAILED,
        API_KEY_CONFIRMED

    };

public:
    std::vector<StockInfo> &get_mutable_stockinfo_vec();
    const std::vector<StockInfo> &get_immutable_stockinfo_vec() const;

    std::vector<StockInfo> *get_stockinfo_vec_ptr();
    JSON_CODES parse_watchlist_item(const std::string &returned_json);
    JSON_CODES apikey_confimed(const std::string &response);
    JSON_CODES ticker_confirmed(const std::string &response);
    JSON_CODES information_unavailable_check(const std::string &response);
    bool check_financials_availability(const std::string &returned_json);
    void add_default_stockinfo_node_to_stockinformation(const std::string &ticker);
    bool confirm_deletion(const std::string &ticker);
    void clean_resonse(std::string &response);
    // FINANCIALS GENERATION CODE
    int find_column(const std::string &date, StockFinancials::BalanceSheetItems *ptr);
    int find_column(const std::string &date, StockFinancials::CashflowItems *ptr);
    bool parse_balancesheet_statement(const std::string &returned_json, const std::string &ticker, StockFinancials &stock_fin);
    bool parse_cashflow_statement(const std::string &returned_json, const std::string &ticker, StockFinancials &stock_fin);
    bool parse_earnings_statement(const std::string &returned_json, const std::string &ticker, StockFinancials &stock_fin);
    std::vector<StockFinancials> *get_financial_vec_ptr_non_const();
    const std::vector<StockFinancials> &get_immutable_stock_financials_vec();
    std::vector<StockFinancials> &get_mutable_stock_financials_vec();

    // METRICS
    const std::vector<Metrics> &get_immutable_metrics_vec();
    std::vector<Metrics> &get_mutable_metrics_vec();
    bool set_metric_quote_type(const std::string &returned_json, Metrics &m);
    bool parse_type_equity_summary(const std::string &returned_json, Metrics &m);
    bool parse_type_etf_summary(const std::string &returned_json, Metrics &m);
    bool parse_summary_crypto(const std::string &returned_json, Metrics &m);
    std::vector<Metrics> *get_metrics_vec_ptr();
    // CHART STUFF
    bool parse_chart_response(const std::string &ticker, ChartInfo &c);
    std::vector<ChartInfo> &get_chart_info_vec();
    const std::vector<ChartInfo> &get_immutable_chart_info_vec() const;
    std::vector<ChartInfo> &get_mutable_chart_info_vec();
    std::vector<ChartInfo> *get_chart_info_vec_ptr();
};

#endif
