#ifndef _ETF_HOLDINGS_H_
#define _ETF_HOLDINGS_H_

#include <string>
#include <unordered_map>
#include <vector>

class ETF_Holdings
{
private:
    std::string ticker;
    std::unordered_map<std::string, float> holdings_percent;
    std::unordered_map<std::string, std::string> holdings_company_name;
    std::vector<std::string> holdings_keys;

public:
    ETF_Holdings(const std::string &t);
    const std::string &get_ticker();
    std::unordered_map<std::string, float> &get_holdings_float();
    std::unordered_map<std::string, std::string> &get_holdings_company_name();
    std::vector<std::string> &get_holidings_keys();
};

#endif