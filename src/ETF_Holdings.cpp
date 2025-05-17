#include "ETF_Holdings.h"

ETF_Holdings::ETF_Holdings(const std::string &t)
    : ticker(t), other_holdings(0.00), other_industry_weight(0.00)
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

std::unordered_map<std::string, float> &ETF_Holdings::get_sector_weightings()
{
    return sector_weightings;
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

void ETF_Holdings::set_other_holdings(float set)
{
    other_holdings = set;
}
void ETF_Holdings::set_other_industry_weight(float set)
{
    other_industry_weight = set;
}

const float &ETF_Holdings::get_other_holdings() const
{
    return other_holdings;
}
const float &ETF_Holdings::get_other_industry_weight() const
{
    return other_industry_weight;
}