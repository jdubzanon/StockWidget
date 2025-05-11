#include "utils.h"

std::string make_uppercase(const std::string &ticker)
{
    std::string ticker_copy = ticker;
    std::transform(ticker_copy.begin(), ticker_copy.end(), ticker_copy.begin(), ::toupper);
    return ticker_copy;
}

std::string shorten_number(long long int num)
{
    std::ostringstream oss;
    if (num >= 1e9)
    {
        oss << std::fixed << std::setprecision(1) << num / 1e9 << "B"; // Billions
    }
    else if (num >= 1e6)
    {
        oss << std::fixed << std::setprecision(1) << num / 1e6 << "M"; // Millions
    }
    else if (num >= 1e3)
    {
        oss << std::fixed << std::setprecision(1) << num / 1e3 << "K"; // Thousands
    }
    else
    {
        oss << num; // No shortening
    }
    return oss.str();
}