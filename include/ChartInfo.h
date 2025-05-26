#ifndef _CHARTINFO_H
#define _CHARTINFO_H
#include <string>
#include <unordered_map>
#include <iostream>
#include <jsoncpp/json/json.h>

class ChartInfo
{
private:
    std::string ticker;
    bool is_diplaying;
    bool api_error;
    std::string error_def;
    std::unordered_map<std::string, std::string> response_map;
    std::unordered_map<std::string, std::vector<double>> price_data_vectors{
        {"1yr", std::vector<double>{}},
        {"3yr", std::vector<double>{}},
        {"5yr", std::vector<double>{}}

    };
    std::unordered_map<std::string, std::vector<double>> timestamp_vectors{
        {"1yr", std::vector<double>{}},
        {"3yr", std::vector<double>{}},
        {"5yr", std::vector<double>{}}

    };
    std::unordered_map<std::string, std::vector<double>> below_avg_vectors{
        {"1yr", std::vector<double>{}},
        {"3yr", std::vector<double>{}},
        {"5yr", std::vector<double>{}}

    };

    std::unordered_map<std::string, double> avg_prices{
        {"1y", double{}},
        {"3y", double{}},
        {"5y", double{}}};

public:
    ChartInfo(const std::string &t);
    const std::string &get_ticker() const;
    std::unordered_map<std::string, std::string> &get_mutable_chart_respone_map();
    const std::unordered_map<std::string, std::string> &get_immutable_chart_respone_map() const;
    void set_api_chart_error(bool set);
    const bool get_api_chart_error() const;
    const std::string &get_error_def() const;
    void set_api_def(const char *def);
    const bool &get_is_displaying() const;
    void set_is_displaying(bool set);
    std::vector<double> &get_price_data_ref(const std::string &yr);
    std::vector<double> &get_timestamp_ref(const std::string &yr);
    std::vector<double> &get_below_avg_vec(const std::string &yr);
    void set_avg_price(const double &p, const std::string &yr);
    const double &get_avg_price(const std::string &yr) const;
    const std::vector<double> &get_below_avg_vec_const(const std::string &yr) const;
};

#endif