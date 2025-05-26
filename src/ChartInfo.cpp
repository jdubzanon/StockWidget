#include "ChartInfo.h"

ChartInfo::ChartInfo(const std::string &t)
    : ticker(t), is_diplaying(false), api_error(false)
{
}

const std::string &ChartInfo::get_ticker() const
{
    return ticker;
}

// TOTAL RESPONSE RAW
std::unordered_map<std::string, std::string> &ChartInfo::get_mutable_chart_respone_map()
{
    return response_map;
}
// TOTAL RESPONSE RAW
const std::unordered_map<std::string, std::string> &ChartInfo::get_immutable_chart_respone_map() const
{
    return response_map;
}

void ChartInfo::set_api_chart_error(bool set)
{
    api_error = set;
}

const bool ChartInfo::get_api_chart_error() const
{
    return api_error;
}

const std::string &ChartInfo::get_error_def() const
{
    return error_def;
}

void ChartInfo::set_api_def(const char *def)
{
    error_def = def;
}

std::vector<double> &ChartInfo::get_price_data_ref(const std::string &yr)
{
    return price_data_vectors.at(yr);
}

std::vector<double> &ChartInfo::get_timestamp_ref(const std::string &yr)
{
    return timestamp_vectors.at(yr);
}

void ChartInfo::set_avg_price(const double &p, const std::string &yr)
{
    avg_prices.at(yr) = p;
}

const double &ChartInfo::get_avg_price(const std::string &yr) const
{
    return avg_prices.at(yr);
}

std::vector<double> &ChartInfo::get_below_avg_vec(const std::string &yr)
{
    return below_avg_vectors.at(yr);
}

const std::vector<double> &ChartInfo::get_below_avg_vec_const(const std::string &yr) const
{
    return below_avg_vectors.at(yr);
}

const bool &ChartInfo::get_is_displaying() const
{
    return is_diplaying;
}

void ChartInfo::set_is_displaying(bool set)
{
    is_diplaying = set;
}
