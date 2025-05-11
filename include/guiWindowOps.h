#ifndef _GUIWINDOWOPS_H
#define _GUIWINDOWOPS_H

#include "Backend.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include "../libs/implot/implot.h"
class guiWindowOps
{
private:
    std::unique_ptr<Backend> backendops;
    std::unordered_map<std::string, bool> financials_window_booleans;
    std::unordered_map<std::string, bool> selectable_booleans;
    std::unordered_map<std::string, bool> chart_booleans;
    std::vector<StockInfo> *watchlist_vec;
    std::vector<StockFinancials> *financial_vec;
    std::vector<Metrics> *metrics_vec;
    std::vector<ChartInfo> *chart_info_vec;
    char input_bucket[100];

private:
    void generate_menubar();
    void generate_equity_dropbox(const StockInfo &stock);
    void generate_etf_dropbox(const StockInfo &stock);
    void generate_crypto_dropbox(const StockInfo &stock);
    void openWebsite(const char *url);
    void reset_arr();
    void set_window_parameters(GLFWwindow *window, ImFont *font_change);
    void delete_operations(const std::string &ticker);
    int get_class_pos(const std::string &ticker);
    // void run_chart_vec_manager();

    void generate_table_label(const std::string &label, ImFont *font_change, ImVec4 &color);
    void generate_table(StockFinancials::BalanceSheetItems *ptr, const std::string &table_type);
    void generate_table(StockFinancials::CashflowItems *ptr, const std::string &table_type);
    void generate_eps_table(StockFinancials::EarningsItems *ptr);
    void generate_yearly_earnings_table(StockFinancials::EarningsItems *ptr);
    void generate_profit_margin_chart(StockFinancials::EarningsItems *ptr);
    void generate_metrics_table(const std::vector<std::unordered_map<std::string, std::string>> *metrics_bucket_ptr, const std::string &ticker);
    void generate_metrics_table_special(const std::vector<std::unordered_map<std::string, std::string>> *metrics_bucket_ptr, const std::string &ticker);

private:
    struct
    {
        ImGuiWindowFlags no_error_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGuiWindowFlags with_error_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMouseInputs;

    } flag_settings;

public:
    std::string temp_message;
    struct
    {
        bool api_key_empty;
        bool watchlist_empty;
    } file_status;

    struct
    {
        bool open_api_window = false;
        bool open_change_api_key_popup = false;
        bool making_api_call_window = false;
        bool open_add_to_watchlist_window = false;
        bool open_dynamic_window = false;
        bool open_error_window = false;

    } popup_booleans;

    struct
    {
        bool need_make_api = false;
        bool single_api_call = false;
        // single calls
        bool watchlist_call = false;
        bool api_key_entry_call = false;
        bool summary_call = false;
        bool chart_call = false;
        // Multi calls
        bool multi_watchlist_call = false;
        bool multi_financial_call = false;
        bool try_again = false;
        std::string ticker = "none";
    } api_workflow;

    struct
    {
        bool program_startup = true;
        bool refresh_watchlist = false;
        bool trigger_error = false;
        bool adding_api = false;
        bool changing_api = false;
        bool dropbox_financial_clicked = false;
        bool dropbox_chart_clicked = false;
    } program_state;

public:
    guiWindowOps();
    ~guiWindowOps();
    // ACTIONS
    void make_api_call(bool single_call, bool watchlistCall, bool apiKeyCall, bool multi_financial_call, bool multi_watchlist_call, bool summary_call, bool chart_call, const char *ticker);
    void delete_from_boolean_map(std::unordered_map<std::string, bool> &map, const std::string &s);
    void setup_window_boolean_maps();
    void reset_necessary_guiops_booleans();
    void run_chart_vec_manager();
    // SETTERS AND GETTERS
    std::vector<StockInfo> *get_watchlist_vec_ptr();
    const std::vector<StockFinancials> *get_fin_vec_ptr() const;
    const std::unordered_map<std::string, bool> &get_immutable_fin_bool_map_ref() const;
    std::unordered_map<std::string, bool> &get_financial_window_booleans();
    std::unordered_map<std::string, bool> &get_selectable_booleans();
    std::unordered_map<std::string, bool> &get_chart_booleans();
    const std::unordered_map<std::string, bool> &get_immutable_chart_booleans_map();

    // WINDOWS
    void generate_watchlist(GLFWwindow *window, ImFont *large_font);
    bool generate_stockfinancials_window(GLFWwindow *window, const std::string &ticker, ImFont *font_change);
    void apiKeyFileEmptyWindow(GLFWwindow *window, ImFont *defualt_f, ImFont *title_f, ImFont *midsize_f, ImFont *defualt_italic);
    void making_api_call_window(GLFWwindow *window, ImFont *midsize_f, bool make_api_call);
    void Dynamic_apikey_operation_window(GLFWwindow *window, const std::string &text, const std::string &button_label, ImFont *font_change, bool &open);
    bool display_chart_window(const std::string &ticker);
    void add_to_watchlist_window(GLFWwindow *window, ImFont *font_change, bool &open);
    void error_window(GLFWwindow *window, ImFont *font_change, bool &open);
};

#endif