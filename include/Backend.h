#ifndef __BACKEND__
#define __BACKEND__

#include "FileOps.h"
#include "EncryptOps.h"
#include "RequestOps.h"
#include "JsonParseOps.h"
#include "utils.h"
#include "BackendException.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <memory>
#include <numeric>

class Backend
{
private:
    std::unique_ptr<FileOps> fileops;
    std::unique_ptr<EncryptOps> encryptops;
    std::unique_ptr<RequestOps> requestops;
    std::unique_ptr<JsonParseOps> jsonparseops;
    bool apifile_is_empty;
    bool watchlist_is_empty;
    std::vector<std::string> *watchlist_tracker;

private:
    void build_map_from_files();
    std::string run_decrypt_apikey_operations();
    void run_single_api_call_operations(const std::string &ticker);
    JsonParseOps::JSON_CODES run_internal_parsing_operations(const std::string &response, const std::string &ticker_copy); // returns true or false based on if information was unavailable from api
    std::string get_quote_type_from_stockinfo_vec(const std::string &ticker, const std::string &quote_type);
    void confirm_apikey(const std::string &response, bool &conf_ref, std::unordered_map<std::string, std::string> &map_ref);
    void confirm_apikey(const std::string &response, std::unordered_map<std::string, std::string> &map_ref);

public:
    Backend();
    ~Backend();

    bool get_api_file_status();
    bool get_watchlist_file_status();
    std::vector<std::string> *get_backend_watchlist_tracker();
    bool run_add_to_watchlist_operations(const std::string &ticker);
    bool run_delete_from_watchlist_operations(const std::string &ticker);
    void run_delete_from_financials_operations(const std::string &ticker);
    void run_delete_from_charts_operations(const std::string &ticker);
    void run_delete_from_metrics_operations(const std::string &ticker);
    void run_add_api_key_operations(const std::string &api_key);
    bool run_multi_watchlist_api_calls_operations();
    bool run_financials_operations(const std::string &ticker);
    bool run_generate_summary_operations(const std::string &ticker);
    bool run_charting_operations(const std::string &ticker);
    void run_etf_holdings_operations(const std::string &ticker);
    bool financial_report_already_generated(const std::string &ticker);
    bool chart_already_generated(const std::string &ticker);
    bool summary_already_generated(const std::string &ticker);
    bool etf_holdings_already_generated(const std::string &ticker);
    int find_column(const std::string &date, StockFinancials::BalanceSheetItems *ptr);
    int find_column(const std::string &date, StockFinancials::CashflowItems *ptr);
    size_t get_metrics_position_in_vec(const std::string &ticker);
    int get_chart_obj_position_in_vec(const std::string &ticker);

    std::vector<StockFinancials> *pass_stock_financial_vec_ptr_non_const();
    std::vector<Metrics> *pass_metrics_ptr();
    std::vector<ChartInfo> *pass_chart_info_vec_ptr();
    std::vector<StockInfo> *pass_stockinfo_ptr();
    std::vector<ETF_Holdings> *pass_etf_holdings_vec_ptr();
    const std::vector<StockInfo> &get_stockinfo_vec();
    int get_holdings_position_in_vec(const std::string &ticker);
};
#endif