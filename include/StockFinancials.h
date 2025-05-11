#ifndef _STOCKFINANCIALS_H
#define _STOCKFINANCIALS_H
#include <string>
#include <unordered_map>
#include <vector>

class StockFinancials
{
private:
    std::string ticker;

public:
    struct BalanceSheetItems
    {
        std::vector<std::string> accounting_items;
        std::unordered_map<int, std::string> dates_tracker;
        std::unordered_map<int, bool> column_tracker;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> reporting_map;
    } balance_sheet_struct;

    struct CashflowItems
    {
        std::vector<std::string> accounting_items;
        std::unordered_map<int, std::string> dates_tracker;
        std::unordered_map<int, bool> column_tracker;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> reporting_map;
    } cashflow_struct;
    struct EarningsItems
    {
        std::vector<std::string> quartely_dates_inorder;
        std::vector<std::string> annual_dates_inorder;
        std::unordered_map<int, std::string> dates_tracker;
        std::unordered_map<int, bool> column_tracker;
        std::unordered_map<std::string, std::unordered_map<std::string, float>> quarterly_reporting_map;
        std::unordered_map<std::string, std::unordered_map<std::string, long long int>> annual_reporting_map;
        std::vector<double> percent_vec;
    } earnings_struct;

public:
    StockFinancials(const std::string &t);
    const std::string &get_ticker() const;
    StockFinancials::BalanceSheetItems *get_balancesheet_ptr();
    StockFinancials::CashflowItems *get_cashflow_ptr();
    StockFinancials::EarningsItems *get_earnings_ptr();
    const StockFinancials::BalanceSheetItems *get_balancesheet_ptr_const() const;
    const StockFinancials::CashflowItems *get_cashflow_ptr_const() const;
    
};

#endif
