#include "StockInfo.h"

StockInfo::StockInfo(const string &name, const string &e_ticker, const double &m_price, const double &e_pchange, const string q_type)
    : company_name(name), ticker(e_ticker), market_price(m_price), price_change(e_pchange), quote_type(q_type) {}

string StockInfo::get_company_name() const
{
    return company_name;
}
string StockInfo::get_ticker() const
{
    return ticker;
}

string StockInfo::get_quote_type_non_const()
{
    return quote_type;
}

string StockInfo::get_quote_type() const
{
    return quote_type;
}

double StockInfo::get_market_price() const
{
    return market_price;
}
double StockInfo::get_price_change() const
{
    return price_change;
}
