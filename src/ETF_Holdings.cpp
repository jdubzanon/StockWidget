#include "ETF_Holdings.h"

ETF_Holdings::ETF_Holdings(const std::string &t)
    : ticker(t)
{
}

const std::string &ETF_Holdings::get_ticker() const
{
    return ticker;
}

std::unordered_map<std::string, float> &ETF_Holdings::get_holdings_float()
{
    return holdings_percent;
}

std::unordered_map<std::string, std::string> &ETF_Holdings::get_holdings_company_name()
{
    return holdings_company_name;
}

std::vector<std::string> &ETF_Holdings::get_holidings_keys()
{
    return holdings_keys;
}

std::unordered_map<std::string, std::string> &ETF_Holdings::get_profile_map()
{
    return profile_map;
}