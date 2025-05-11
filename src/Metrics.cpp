#include "Metrics.h"

Metrics::Metrics(const std::string &t)
    : ticker(t), quote_type("")
{
}

const std::string &Metrics::get_ticker() const
{
    return ticker;
}

std::string Metrics::get_quote_type()
{
    return quote_type;
}

void Metrics::set_quote_type(const std::string &type)
{
    quote_type = type;
}

const std::vector<std::unordered_map<std::string, std::string>> *Metrics::get_metrics_bucket_ptr()
{
    return &metrics_bucket;
}

std::vector<std::unordered_map<std::string, std::string>> *Metrics::get_metrics_bucket_ptr_non_const()
{
    return &metrics_bucket;
}
