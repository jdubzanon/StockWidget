#ifndef __FILEOPS__
#define __FILEOPS__

#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <crypto++/cryptlib.h>
#include <unordered_map>

namespace fs = std::filesystem;

class FileOps
{
private:
    // FILE OPEN TYPES
    const std::ios::openmode binary_in = std::ios::binary | std::ios::in;
    const std::ios::openmode binary_out = std::ios::binary | std::ios::out;
    const std::ios::openmode normal_in = std::ios::in;
    const std::ios::openmode normal_out = std::ios::out | std::ios::app;

    const char *home = std::getenv("HOME");
    // DIRECTORIES
    const std::string main_directory_path = std::string(home) + "/.local/share/stock_widget/";
    const std::string api_main_dir = main_directory_path + "api_info/";

    std::fstream watchlistfile;
    std::fstream enc_api_file;
    std::fstream api_key_iv_file;

    std::vector<std::string> watchlist_tracker;
    // FILE NAMES
    const std::string watchlist_file_name = "watchlist.dat";
    const std::string encrypted_apikey_file = "api_key.enc";
    const std::string seed_file = "key_iv.bin";

    // FILES
    const std::string watchlist_abs_path = main_directory_path + watchlist_file_name;
    const std::string key_iv_path = api_main_dir + seed_file;
    const std::string encrypted_api_key_path = api_main_dir + encrypted_apikey_file;

    const std::vector<std::string> directories_needed{main_directory_path, api_main_dir};
    const std::vector<std::string> files_needed{watchlist_abs_path, key_iv_path, encrypted_api_key_path};
    const std::unordered_map<int, std::filesystem::perms> perms_definitions = {
        {600, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write},
        {640, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::group_read},
        {400, std::filesystem::perms::owner_read}};

private:
    bool create_file(const std::string &path);
    int openfile_operation(std::fstream &file, const std::string &path, const std::ios::openmode &type);
    void close_file(std::fstream &file, int octal_reset, const std::string &path);
    bool set_permissions(const std::string &path, const fs::perms &p);
    int get_octal_permissions(fs::perms permissions);
    // FOR ADDING/DELETING OPERATIONS CHECK
    bool ticker_exists(const std::string &ticker);
    void truncate_file(std::fstream &file, const std::string &path);
    bool key_iv_file_empty();
    int get_preferred_octals(const std::string &path);
    bool create_directories();
    bool create_files();

public:
    void system_init();
    bool watchlistfile_empty();
    std::vector<std::string> create_api_endpoints_from_watchlist();
    std::vector<std::string> get_endpoints() const;
    bool set_all_file_permissions();
    bool apikeyfile_empty();

    // ADDING/DELETEING TICKER OPERATIONS
    void watchlist_tracker_init();
    std::vector<std::string> *get_watchlist_tracker();

    void add_to_watchlist(std::string ticker);
    void delete_watchlist_item(const std::string &ticker);

    // WRITING FILES AFTER API ENCRYPTION
    bool add_key_iv_info_to_file(const std::unordered_map<std::string, std::vector<uint8_t>> *const map);
    bool add_encrypted_api_key_to_file(const std::unordered_map<std::string, std::vector<uint8_t>> *const map);
    // READING FILES FOR API KEY DECYRPTION
    std::vector<std::vector<uint8_t>> read_key_iv_file(const size_t &aes_size, const size_t &iv_size);
    std::vector<uint8_t> read_api_enc_file();
    bool confirm_ticker_added_to_file(const std::string &ticker);
    bool confirm_ticker_deletion(const std::string &ticker);
};

#endif