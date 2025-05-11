#ifndef _STOCKINFO__
#define __STOCKINFO__

#include <string>

using std::string;

class StockInfo
{
private:
    string company_name;
    string ticker;
    double market_price;
    double price_change;
    string quote_type;

public:
    StockInfo(const string &name, const string &e_ticker, const double &m_price, const double &e_pchange, const string q_type);
    string get_company_name() const;
    string get_ticker() const;
    string get_quote_type() const;
    string get_quote_type_non_const();
    double get_market_price() const;
    double get_price_change() const;
};

#endif
