#include "StockFinancials.h"

StockFinancials::StockFinancials(const std::string &t)
    : ticker(t)
{
}

const std::string &StockFinancials::get_ticker() const
{
    return ticker;
}

StockFinancials::BalanceSheetItems *StockFinancials::get_balancesheet_ptr()
{
    return &balance_sheet_struct;
}

StockFinancials::CashflowItems *StockFinancials::get_cashflow_ptr()
{
    return &cashflow_struct;
}

const StockFinancials::CashflowItems *StockFinancials::get_cashflow_ptr_const() const
{
    return &cashflow_struct;
}

const StockFinancials::BalanceSheetItems *StockFinancials::get_balancesheet_ptr_const() const
{
    return &balance_sheet_struct;
}
StockFinancials::EarningsItems *StockFinancials::get_earnings_ptr()
{
    return &earnings_struct;
}
