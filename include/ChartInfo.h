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
    double avg_price;
    std::string error_def;
    std::unordered_map<std::string, std::string> response_map;
    std::vector<double> price_data_vec;
    std::vector<double> timestamps_vec;

    std::vector<double> below_avg_data_vec;

public:
    ChartInfo(const std::string &t);
    const std::string &get_ticker() const;
    std::unordered_map<std::string, std::string> &get_mutable_chart_respone_map();
    const std::unordered_map<std::string, std::string> &get_immutable_chart_respone_map() const;
    std::vector<double> &get_price_data_ref();
    std::vector<double> &get_timestamp_ref();
    void set_api_chart_error(bool set);
    const bool get_api_chart_error() const;
    const std::string &get_error_def() const;
    void set_api_def(const char *def);
    void set_avg_price(const double &p);
    const double &get_avg_price() const;
    std::vector<double> &get_below_avg_vec();
    const std::vector<double> &get_below_avg_vec_const() const;
    const bool &get_is_displaying() const;
    void set_is_displaying(bool set);
};

#endif