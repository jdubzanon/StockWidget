#ifndef _ETF_HOLDINGS_H_
#define _ETF_HOLDINGS_H_

#include <string>
#include <unordered_map>
#include <vector>

class ETF_Holdings
{
private:
    std::string ticker;
    float other_holdings;
    float other_industry_weight;

    std::unordered_map<std::string, float> holdings_percent;
    std::unordered_map<std::string, std::string> holdings_company_name;
    std::vector<std::string> holdings_keys;
    std::unordered_map<std::string, float> sector_weightings;
    std::unordered_map<std::string, std::string> profile_map;

public:
    ETF_Holdings(const std::string &t);
    const std::string &get_ticker() const;
    std::unordered_map<std::string, float> &get_holdings_float();
    std::unordered_map<std::string, float> &get_sector_weightings();
    std::unordered_map<std::string, std::string> &get_holdings_company_name();
    std::vector<std::string> &get_holidings_keys();
    std::unordered_map<std::string, std::string> &get_profile_map();
    void set_other_holdings(float set);
    void set_other_industry_weight(float set);

    const float &get_other_holdings() const;
    const float &get_other_industry_weight() const;
};

#endif