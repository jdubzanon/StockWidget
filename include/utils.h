#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

std::string make_uppercase(const std::string &ticker);
std::string shorten_number(long long int num);

#endif