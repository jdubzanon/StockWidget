#ifndef _METRICS_H
#define _METRICS_H
#include <string>
#include <unordered_map>
#include <vector>

class Metrics
{
private:
    std::string ticker;
    std::string quote_type;
    std::vector<std::unordered_map<std::string, std::string>> metrics_bucket;

public:
    Metrics(const std::string &t);
    const std::string &get_ticker() const;
    std::string get_quote_type();
    void set_quote_type(const std::string &type);
    const std::vector<std::unordered_map<std::string, std::string>> *get_metrics_bucket_ptr();
    std::vector<std::unordered_map<std::string, std::string>> *get_metrics_bucket_ptr_non_const();
};

#endif