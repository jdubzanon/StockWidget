#include <stdexcept>
#include <string>

#include <stdexcept>
#include <string>

class BackendException : public std::exception
{
public:
    enum class ErrorType
    {
        PATH_CREATION_FAILED,
        API_CALL_FAILED,
        MULTI_API_CALL_TOTAL_FAILURE,
        MULTI_PARSING_FAILURE,
        API_KEY_FILE_EMPTY,
        API_CONFIRMATION_FAILED,
        JSON_PARSE_FAILURE,
        STOCKNODE_MISSING,
        DELETION_INCOMPLETE,
        API_FILE_WRITING_FAILED,
        INVALID_TICKER,
        FILE_WRITE_COMMON,
        GET_TICKER_BY_HANDLE_ERROR,
        API_INFO_CURRENTLY_UNAVAILABLE,
        FINANCIALS_TICKER_UNSUPPORTED,
        FINANICALS_JSON_EMPTY,
        NO_EQUITY_TYPE,

    };

    BackendException(ErrorType type, const std::string &message);

    const char *what() const noexcept override;

    ErrorType getType() const;

private:
    ErrorType type;
    std::string message;
};
