#include "BackendException.h"

BackendException::BackendException(ErrorType type, const std::string &message)
    : type(type), message(message) {}

const char *BackendException::what() const noexcept
{
    return message.c_str();
}

BackendException::ErrorType BackendException::getType() const
{
    return type;
}
