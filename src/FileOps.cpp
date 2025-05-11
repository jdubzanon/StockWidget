#include "FileOps.h"

void FileOps::system_init()
{
    create_directories() && create_files();
}

bool FileOps::create_file(const std::string &path)
{
    std::ofstream outfile(path);
    outfile.close();

    return fs::exists(path);
}

// THIS WHOLE FUNCTION IS CHATGPT...WAY ABOVE MY HEAD LOL
int FileOps::get_octal_permissions(fs::perms permissions)
{
    int owner = 0, group = 0, others = 0;

    // Owner permissions

    if ((permissions & fs::perms::owner_read) != fs::perms::none)
        owner += 4;
    if ((permissions & fs::perms::owner_write) != fs::perms::none)
        owner += 2;
    if ((permissions & fs::perms::owner_exec) != fs::perms::none)
        owner += 1;

    // Group permissions
    if ((permissions & fs::perms::group_read) != fs::perms::none)
        group += 4;
    if ((permissions & fs::perms::group_write) != fs::perms::none)
        group += 2;
    if ((permissions & fs::perms::group_exec) != fs::perms::none)
        group += 1;

    // Others permissions
    if ((permissions & fs::perms::others_read) != fs::perms::none)
        others += 4;
    if ((permissions & fs::perms::others_write) != fs::perms::none)
        others += 2;
    if ((permissions & fs::perms::others_exec) != fs::perms::none)
        others += 1;

    // Convert to octal number
    return owner * 100 + group * 10 + others;
}

bool FileOps::set_permissions(const std::string &path, const fs::perms &p)
{
    try
    {
        fs::permissions(path, p);
    }
    catch (std::exception &e)
    {
        // failure -> hard code a permission
        const fs::perms &permission_def = perms_definitions.at(640);
        fs::permissions(path, permission_def);
        return false;
    };
    return true;
}

// SYSTEM CHECK ON STARTUP
bool FileOps::create_directories()
{
    for (const std::string &path : directories_needed)
    {
        if (!fs::exists(path))
        {
            if (!fs::create_directory(path))
            {
                return false;
            }
        }
    }
    return fs::exists(main_directory_path) && fs::exists(api_main_dir);
}

// SYSTEM CHECK ON STARTUP
bool FileOps::create_files()
{
    for (const std::string &path : files_needed)
    {
        if (!fs::exists(path))
        {
            create_file(path);
        }
    }

    return fs::exists(watchlist_abs_path) && fs::exists(key_iv_path) && fs::exists(encrypted_api_key_path);
}

// SYSTEM CHECK ON STARTUP
bool FileOps::set_all_file_permissions()
{
    bool watchlist_perms_changed, key_iv_perms_changed, api_key_perms_changed;

    const fs::perms watchlist_desired_perms = perms_definitions.at(600);
    watchlist_perms_changed = set_permissions(watchlist_abs_path, watchlist_desired_perms);

    const fs::perms desired_key_iv_permissions = perms_definitions.at(400);
    key_iv_perms_changed = set_permissions(key_iv_path, desired_key_iv_permissions);

    const fs::perms desired_api_key_permissions = perms_definitions.at(400);
    api_key_perms_changed = set_permissions(encrypted_api_key_path, desired_api_key_permissions);

    return api_key_perms_changed && key_iv_perms_changed && watchlist_perms_changed;
}

// // retuning the old octals so when i close the file i can set permissions back to what they were
int FileOps::openfile_operation(std::fstream &file, const std::string &path, const std::ios::openmode &type)
{
    if (file.is_open())
    {
        file.close();
    }

    // this is the files octals that i want at rest
    int files_preferred_octals = get_preferred_octals(path);

    // this is the file octals that i want it to be when reading/writing
    const fs::perms &desired_perms = perms_definitions.at(600);

    if (files_preferred_octals != get_octal_permissions(desired_perms))
    {
        set_permissions(path, desired_perms);
    }

    file.open(path, type);

    return files_preferred_octals;
}

// // ACTION ON FILE
// READING OPERATION
std::vector<std::string> FileOps::create_api_endpoints_from_watchlist()
{
    std::vector<std::string> endpoints;
    int preferred_octals = openfile_operation(watchlistfile, watchlist_abs_path, normal_in);
    std::string api_head = "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=";
    std::string api_tail = "&lang=en-US&region=US";
    std::string ticker;
    while (std::getline(watchlistfile, ticker))
    {
        std::string endpoint = api_head + std::string(ticker) + api_tail;
        endpoints.push_back(endpoint);
        ticker = "";
        endpoint = "";
    }
    close_file(watchlistfile, preferred_octals, watchlist_abs_path);
    return endpoints;
}

void FileOps::close_file(std::fstream &file, int octal_reset, const std::string &path)
{
    file.close();
    set_permissions(path, perms_definitions.at(octal_reset));
}

// // ACTION ON FILE
// WRITING OPERATION
void FileOps::add_to_watchlist(std::string ticker)
{
    if (!ticker_exists(ticker))
    {
        int preferred_file_octals = openfile_operation(watchlistfile, watchlist_abs_path, normal_out);
        std::string modified_ticker = ticker + '\n';
        watchlistfile << modified_ticker;
        close_file(watchlistfile, preferred_file_octals, watchlist_abs_path);
        watchlist_tracker.push_back(ticker);
    }
}

// set api key window test SYSTEM CHECK
bool FileOps::apikeyfile_empty()
{
    return fs::is_empty(encrypted_api_key_path);
}

// add to watchlist empty screen test SYSTEM CHECK
bool FileOps::watchlistfile_empty()
{
    return fs::file_size(watchlist_abs_path) == 0;
}

// for truncate file check when writing key_iv
bool FileOps::key_iv_file_empty()
{
    return fs::file_size(key_iv_path) == 0;
}

// THIS WILL TRACK WATCHLIST FOR DELETING OPERATIONS
// READING OPERATION
void FileOps::watchlist_tracker_init()
{
    int preferred_file_octals = openfile_operation(watchlistfile, watchlist_abs_path, normal_in);
    std::string ticker = "";
    if (!watchlistfile_empty())
    {
        while (std::getline(watchlistfile, ticker))
        {
            watchlist_tracker.push_back(ticker);
        }
        close_file(watchlistfile, preferred_file_octals, watchlist_abs_path);
    }
}

std::vector<std::string> *FileOps::get_watchlist_tracker()
{
    return &watchlist_tracker;
}

bool FileOps::ticker_exists(const std::string &ticker)
{
    auto it = std::find(watchlist_tracker.begin(), watchlist_tracker.end(), ticker);
    return (it != watchlist_tracker.end());
}

// WRITING OPERATION
void FileOps::delete_watchlist_item(const std::string &ticker)
{

    // delete from watchlist tracker
    auto it = std::find(watchlist_tracker.begin(), watchlist_tracker.end(), ticker);
    if (it != watchlist_tracker.end())
    {

        watchlist_tracker.erase(it);
    }

    // clear file and write whats left in watchlist tracker to file
    truncate_file(watchlistfile, watchlist_abs_path);
    int preferred_octals = openfile_operation(watchlistfile, watchlist_abs_path, normal_out);

    // CANT USE add_to_watchlist() BECAUSE OF TICKER EXISTS CONDITION HAVE TO MANUALLY WRITE IT TO FILE
    if (!watchlist_tracker.empty())
    {
        for (const auto &t : watchlist_tracker)
        {
            watchlistfile << t << "\n";
        }
        close_file(watchlistfile, preferred_octals, watchlist_abs_path);
    }
}

void FileOps::truncate_file(std::fstream &file, const std::string &path)
{

    if (file.is_open())
    {
        file.close();
    }
    int preferred_octals = get_preferred_octals(path);

    fs::perms desired_perms = perms_definitions.at(600);

    if (preferred_octals != get_octal_permissions(desired_perms))
    {
        set_permissions(path, desired_perms);
    }

    file.open(path, std::ios::trunc | std::ios::out);
    close_file(file, preferred_octals, path);
}

// READING OPERATION
std::vector<std::vector<uint8_t>> FileOps::read_key_iv_file(const size_t &aes_size, const size_t &iv_size)
{
    // when writing it writes key first then it writes iv so i have to read key first then iv
    std::vector<std::vector<uint8_t>> key_iv_vector;

    std::vector<uint8_t> key(aes_size);
    std::vector<uint8_t> iv(iv_size);

    int preferred_octals = openfile_operation(api_key_iv_file, key_iv_path, binary_in);
    api_key_iv_file.read(reinterpret_cast<char *>(key.data()), key.size());
    api_key_iv_file.read(reinterpret_cast<char *>(iv.data()), iv.size());
    key_iv_vector.push_back(key);
    key_iv_vector.push_back(iv);
    close_file(api_key_iv_file, preferred_octals, key_iv_path);
    return key_iv_vector;
}

// READING OPERATION
std::vector<uint8_t> FileOps::read_api_enc_file()
{
    int preferred_octals = openfile_operation(enc_api_file, encrypted_api_key_path, binary_in);
    enc_api_file.seekg(0, std::ios::end);
    std::streamsize file_size = enc_api_file.tellg();
    enc_api_file.seekg(0, std::ios::beg);

    std::vector<uint8_t> api_data(file_size);
    enc_api_file.read(reinterpret_cast<char *>(api_data.data()), api_data.size());
    close_file(enc_api_file, preferred_octals, encrypted_api_key_path);
    return api_data;
}

// ENCRYTPION WRITING
// WRITING OPERATION
bool FileOps::add_key_iv_info_to_file(const std::unordered_map<std::string, std::vector<uint8_t>> *const immutable_map)
{

    if (!key_iv_file_empty())
    {
        truncate_file(api_key_iv_file, key_iv_path);
    }
    int preferred_octals = openfile_operation(api_key_iv_file, key_iv_path, binary_out);
    api_key_iv_file.write(
        reinterpret_cast<const char *>(immutable_map->at("key").data()),
        immutable_map->at("key").size());

    api_key_iv_file.write(
        reinterpret_cast<const char *>(immutable_map->at("iv").data()),
        immutable_map->at("iv").size());

    close_file(api_key_iv_file, preferred_octals, key_iv_path);
    return fs::file_size(key_iv_path) == 0;
}

// ENCRYPTION WRITING
// WRITING OPERATION BINARY
bool FileOps::add_encrypted_api_key_to_file(const std::unordered_map<std::string, std::vector<uint8_t>> *const immutable_map)
{
    if (!apikeyfile_empty())
    {
        truncate_file(enc_api_file, encrypted_api_key_path);
    }

    int perferred_octals = openfile_operation(enc_api_file, encrypted_api_key_path, binary_out);
    enc_api_file.write(
        reinterpret_cast<const char *>(immutable_map->at("encrypted_apikey").data()),
        immutable_map->at("encrypted_apikey").size());
    close_file(enc_api_file, perferred_octals, encrypted_api_key_path);
    return fs::file_size(encrypted_api_key_path) == 0;
}

int FileOps::get_preferred_octals(const std::string &path)
{
    fs::file_status file_status_now = fs::status(path);
    fs::perms perms_now = file_status_now.permissions();
    int current_octals = get_octal_permissions(perms_now);
    return current_octals;
}

bool FileOps::confirm_ticker_added_to_file(const std::string &ticker)
{
    int preferred_octals = openfile_operation(watchlistfile, watchlist_abs_path, normal_in);
    std::string line;
    while (getline(watchlistfile, line))
    {
        if (line == ticker)
        {
            return true;
        }
    }
    close_file(watchlistfile, preferred_octals, watchlist_abs_path);
    return false;
}

bool FileOps::confirm_ticker_deletion(const std::string &ticker)
{
    int preferred_octals = openfile_operation(watchlistfile, watchlist_abs_path, normal_in);
    std::string line;
    while (getline(watchlistfile, line))
    {
        if (line == ticker)
        {
            return false;
        }
    }
    close_file(watchlistfile, preferred_octals, watchlist_abs_path);
    return true;
}
