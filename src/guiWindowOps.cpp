#include "guiWindowOps.h"

guiWindowOps::guiWindowOps()
    : watchlist_vec(nullptr)

{

    backendops = std::make_unique<Backend>();
    watchlist_vec = backendops->pass_stockinfo_ptr();
    financial_vec = backendops->pass_stock_financial_vec_ptr_non_const();
    metrics_vec = backendops->pass_metrics_ptr();
    chart_info_vec = backendops->pass_chart_info_vec_ptr();
    etf_holdings_vec = backendops->pass_etf_holdings_vec_ptr();
    file_status.api_key_empty = backendops->get_api_file_status();
    file_status.watchlist_empty = backendops->get_watchlist_file_status();
}

guiWindowOps::~guiWindowOps()
{
    watchlist_vec = nullptr;
    financial_vec = nullptr;
}

// PRIVATE
void guiWindowOps::generate_menubar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (!backendops->get_api_file_status())
            {
                if (ImGui::MenuItem("Add Stock"))
                {
                    popup_booleans.open_add_to_watchlist_window = true;
                }
            }
            if (backendops->get_api_file_status())
            {
                if (ImGui::MenuItem("Add Api Key"))
                {
                    popup_booleans.open_dynamic_window = true;
                    // these two toggle whats displayed on the dynamic window
                    program_state.changing_api = false; // redundancy
                    program_state.adding_api = true;
                    // api_workflow.watchlist_call = false;
                }
            }
            if (!backendops->get_api_file_status())
            {
                if (ImGui::MenuItem("Change Api Key"))
                {
                    reset_necessary_guiops_booleans();
                    // these two toggle what is being displayed on the window
                    program_state.adding_api = false; // redundancy
                    program_state.changing_api = true;

                    popup_booleans.open_dynamic_window = true;
                }
            }

            if (!backendops->get_watchlist_file_status() && !backendops->get_api_file_status())
            {
                if (ImGui::MenuItem("Refresh"))
                {
                    api_workflow.need_make_api = true;
                    program_state.refresh_watchlist = true;
                    api_workflow.multi_financial_call = false; // redundancy
                    api_workflow.multi_watchlist_call = true;
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
};

// PRIVATE
void guiWindowOps::generate_equity_dropbox(const StockInfo &stock)
{
    // DROPBOX CODE BELOW
    const char *action_items[] = {"Chart", "Financials", "Delete"};
    static int selected_action = -1;
    std::string dropbox_label = stock.get_ticker() + "\n" + "Actions";
    const char *preview_value = (selected_action == -1) ? "Select Action" : action_items[selected_action];
    if (ImGui::BeginCombo(dropbox_label.c_str(), preview_value))
    {
        for (int n = 0; n < IM_ARRAYSIZE(action_items); n++)
        {
            bool is_selected = (selected_action == n);
            if (ImGui::Selectable(action_items[n], is_selected))
            {
                selected_action = n;
            }
        }
        ImGui::EndCombo();
    }
    if (selected_action == 0)
    {
        if (!backendops->chart_already_generated(stock.get_ticker()))
        {
            chart_booleans.at(stock.get_ticker()) = true;
            program_state.dropbox_chart_clicked = true;
            api_workflow.need_make_api = true;
            api_workflow.single_api_call = true;
            api_workflow.chart_call = true;
            api_workflow.ticker = stock.get_ticker();
        }
        else
        {
            chart_booleans.at(stock.get_ticker()) = true;
        }
    }
    else if (selected_action == 1)
    {
        // ONLY EQUITYS HAVE FINANCIAL REPORTS
        if (stock.get_quote_type() == "EQUITY") // GUARD AGAINST DEFAULT OPTION, IF stock.quote_type() is other than known definitions
        {
            if (!backendops->financial_report_already_generated(stock.get_ticker()))
            {
                program_state.dropbox_financial_clicked = true;
                api_workflow.need_make_api = true;
                api_workflow.multi_financial_call = true;
                api_workflow.multi_watchlist_call = false;
                api_workflow.ticker = stock.get_ticker();
                financials_window_booleans.at(stock.get_ticker()) = true;
            }
            else
            {

                financials_window_booleans.at(stock.get_ticker()) = true;
            }
        }
        else // NO FINANCIAL REPORTS FOR ETF AND CRYPTO EXPOSE ERROR WINDOW
        {
            std::ostringstream oss;
            oss << "No Financial reports for " << stock.get_quote_type() << " type";
            std::string message = oss.str();
            program_state.trigger_error = true;
            temp_message = message;
            api_workflow.try_again = false;
        }
    }
    else if (selected_action == 2)
    {
        try
        {
            delete_operations(stock.get_ticker());
        }
        catch (BackendException &e)
        {
            temp_message = e.what();
            program_state.trigger_error = true;
            api_workflow.try_again = false;
        }
    }

    selected_action = -1;
}

void guiWindowOps::generate_etf_dropbox(const StockInfo &stock)
{
    // DROPBOX CODE BELOW
    const char *action_items[] = {"Chart", "Holdings", "Delete"};
    static int selected_action = -1;
    std::string dropbox_label = stock.get_ticker() + "\n" + "Actions";
    const char *preview_value = (selected_action == -1) ? "Select Action" : action_items[selected_action];
    if (ImGui::BeginCombo(dropbox_label.c_str(), preview_value))
    {
        for (int n = 0; n < IM_ARRAYSIZE(action_items); n++)
        {
            bool is_selected = (selected_action == n);
            if (ImGui::Selectable(action_items[n], is_selected))
            {
                selected_action = n;
            }
        }
        ImGui::EndCombo();
    }
    if (selected_action == 0)
    {
        if (!backendops->chart_already_generated(stock.get_ticker()))
        {
            chart_booleans.at(stock.get_ticker()) = true;
            program_state.dropbox_chart_clicked = true;
            api_workflow.need_make_api = true;
            api_workflow.single_api_call = true;
            api_workflow.chart_call = true;
            api_workflow.ticker = stock.get_ticker();
        }
        else
        {
            chart_booleans.at(stock.get_ticker()) = true;
        }
    }

    else if (selected_action == 1)
    {
        if (!backendops->etf_holdings_already_generated(stock.get_ticker()))
        {
            api_workflow.need_make_api = true;
            program_state.dropbox_etf_holdings_clicked = true;
            api_workflow.multi_etf_holdings_call = true;
            api_workflow.ticker = stock.get_ticker();
            etf_holdings_booleans.at(stock.get_ticker()) = true;
        }
        else
            etf_holdings_booleans.at(stock.get_ticker()) = true;
    }
    else if (selected_action == 2)
    {
        try
        {
            delete_operations(stock.get_ticker());
        }
        catch (BackendException &e)
        {
            temp_message = e.what();
            program_state.trigger_error = true;
            api_workflow.try_again = false;
        }
    }

    selected_action = -1;
}

void guiWindowOps::generate_crypto_dropbox(const StockInfo &stock)
{
    // DROPBOX CODE BELOW
    const char *action_items[] = {"Chart", "Delete"};
    static int selected_action = -1;
    std::string dropbox_label = stock.get_ticker() + "\n" + "Actions";
    const char *preview_value = (selected_action == -1) ? "Select Action" : action_items[selected_action];
    if (ImGui::BeginCombo(dropbox_label.c_str(), preview_value))
    {
        for (int n = 0; n < IM_ARRAYSIZE(action_items); n++)
        {
            bool is_selected = (selected_action == n);
            if (ImGui::Selectable(action_items[n], is_selected))
            {
                selected_action = n;
            }
        }
        ImGui::EndCombo();
    }
    if (selected_action == 0)
    {
        if (!backendops->chart_already_generated(stock.get_ticker()))
        {
            chart_booleans.at(stock.get_ticker()) = true;
            program_state.dropbox_chart_clicked = true;
            api_workflow.need_make_api = true;
            api_workflow.single_api_call = true;
            api_workflow.chart_call = true;
            api_workflow.ticker = stock.get_ticker();
        }
        else
        {
            chart_booleans.at(stock.get_ticker()) = true;
        }
    }
    else if (selected_action == 1)
    {
        try
        {
            delete_operations(stock.get_ticker());
        }
        catch (BackendException &e)
        {
            temp_message = e.what();
            program_state.trigger_error = true;
            api_workflow.try_again = false;
        }
    }

    selected_action = -1;
}

// PRIVATE
void guiWindowOps::openWebsite(const char *url)

{
    // For Linux: Use xdg-open to open a URL in the default browser
    std::string command = "xdg-open " + std::string(url);
    std::system(command.c_str());
}

void guiWindowOps::reset_arr()
{
    if (input_bucket[0] != '\n')
    {
        std::memset(input_bucket, 0, sizeof(input_bucket));
    }
}

// PRIVATE
void guiWindowOps::set_window_parameters(GLFWwindow *window, ImFont *font_change)
{
    int glfwidth, glfwheight;
    glfwGetWindowSize(window, &glfwidth, &glfwheight);
    ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(glfwidth / 2 - 200, glfwheight / 2 - 100), ImGuiCond_Once);
    ImGui::PushFont(font_change);
    ImGui::GetStyle().WindowRounding = 10.0f;
    ImGui::GetStyle().FrameRounding = 10.0f; // This rounds all buttons globally (set a value > 0)
}

// PRIVATE
void guiWindowOps::delete_operations(const std::string &ticker)
{
    backendops->run_delete_from_watchlist_operations(ticker);
    backendops->run_delete_from_financials_operations(ticker);
    backendops->run_delete_from_charts_operations(ticker);
    backendops->run_delete_from_metrics_operations(ticker);
    backendops->run_delete_from_etf_holdings_operations(ticker);
    delete_from_boolean_map(financials_window_booleans, ticker);
    delete_from_boolean_map(selectable_booleans, ticker);
    delete_from_boolean_map(chart_booleans, ticker);
    delete_from_boolean_map(etf_holdings_booleans, ticker);
    file_status.watchlist_empty = backendops->get_watchlist_file_status();
}

// PRIVATE
int guiWindowOps::get_class_pos(const std::string &ticker)
{
    for (int i = 0; i < static_cast<int>(financial_vec->size()); ++i)
    {
        if (financial_vec->at(i).get_ticker() == ticker)
        {
            return i;
        }
    }
    return -1;
}

// PRIVATE
void guiWindowOps::generate_table_label(const std::string &label, ImFont *font_change, ImVec4 &color)
{
    ImGui::PushFont(font_change);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("%s", label.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

// PRIVATE BALANCE SHEET ITEMS
void guiWindowOps::generate_table(StockFinancials::BalanceSheetItems *ptr, const std::string &table_type)
{
    ImGui::BeginTable(table_type.c_str(), static_cast<int>(ptr->dates_tracker.size() + 1), ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit);

    for (int i = 0; i < static_cast<int>(ptr->dates_tracker.size()) + 1; i++)
    {
        (i == 0) ? ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 0.0f) : ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    }

    ImGui::TableHeadersRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Account ID");
    for (int i = 1; i < static_cast<int>(ptr->dates_tracker.size()) + 1; i++)
    {
        ImGui::TableSetColumnIndex(i);
        ImGui::Text("%s", ptr->dates_tracker.at(i).c_str());
    }
    // ORGAINIZING BY COLUMN
    auto it = ptr->accounting_items.begin();
    int column;
    do
    {
        ImGui::TableNextRow();

        // GRABBING THE ACCOUNTING ITEM AND ITS DICTIONARY ATTACHED TO IT
        const std::unordered_map<std::string, std::string> &acct_obj = ptr->reporting_map[*it];
        // ACCT OBJ IS A DICTIONARY ATTACHED TO EACH ACCOUTING ITEM, LOOPING THROUGH THAT DICTIONARY
        for (const auto &pair : acct_obj)
        {
            ImGui::TableSetColumnIndex(0);
            std::string acct_entry_id(*it);
            ImGui::Text("%s", acct_entry_id.c_str());
            // FINDING THE COLUMN
            column = backendops->find_column(pair.first, ptr);
            if (column != -1)
            {
                ptr->column_tracker[column] = true;
                ImGui::TableSetColumnIndex(column);
                ImGui::Text("%s", pair.second.c_str());
            }
        }

        // FINDING COLUMNS AND INSERTING N/A IF NO INFO AVAIL
        for (const auto &p : ptr->column_tracker)
        {
            if (!p.second)
            {
                column = p.first;
                ImGui::TableSetColumnIndex(column);
                ImGui::Text("N/A");
            }
        }

        // RESETING THE COLUMN TRACKER
        for (auto &p : ptr->column_tracker)
        {
            p.second = false;
        }

        it++;
    } while (it != ptr->accounting_items.end());
    ImGui::EndTable();
}

// PRIVE FOR CASHFLOW ITEMS
void guiWindowOps::generate_table(StockFinancials::CashflowItems *ptr, const std::string &table_type)
{
    ImGui::BeginTable(table_type.c_str(), static_cast<int>(ptr->dates_tracker.size() + 1), ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit);

    for (int i = 0; i < static_cast<int>(ptr->dates_tracker.size()) + 1; i++)
    {
        (i == 0) ? ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 0.0f) : ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    }

    ImGui::TableHeadersRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Account ID");
    for (int i = 1; i < static_cast<int>(ptr->dates_tracker.size()) + 1; i++)
    {
        ImGui::TableSetColumnIndex(i);
        ImGui::Text("%s", ptr->dates_tracker.at(i).c_str());
    }
    // ORGAINIZING BY COLUMN
    auto it = ptr->accounting_items.begin();
    int column;
    do
    {
        ImGui::TableNextRow();

        // GRABBING THE ACCOUNTING ITEM AND ITS DICTIONARY ATTACHED TO IT
        const std::unordered_map<std::string, std::string> &acct_obj = ptr->reporting_map[*it];
        // ACCT OBJ IS A DICTIONARY ATTACHED TO EACH ACCOUTING ITEM, LOOPING THROUGH THAT DICTIONARY
        for (const auto &pair : acct_obj)
        {
            ImGui::TableSetColumnIndex(0);
            std::string acct_entry_id(*it);
            ImGui::Text("%s", acct_entry_id.c_str());
            // FINDING THE COLUMN
            column = backendops->find_column(pair.first, ptr);
            if (column != -1)
            {
                ptr->column_tracker[column] = true;
                ImGui::TableSetColumnIndex(column);
                ImGui::Text("%s", pair.second.c_str());
            }
        }

        // FINDING COLUMNS AND INSERTING N/A IF NO INFO AVAIL
        for (const auto &p : ptr->column_tracker)
        {
            if (!p.second)
            {
                column = p.first;
                ImGui::TableSetColumnIndex(column);
                ImGui::Text("N/A");
            }
        }

        // RESETING THE COLUMN TRACKER
        for (auto &p : ptr->column_tracker)
        {
            p.second = false;
        }

        it++;
    } while (it != ptr->accounting_items.end());
    ImGui::EndTable();
}

// PRIVATE FOR QUARTERLY EPS
void guiWindowOps::generate_eps_table(StockFinancials::EarningsItems *ptr)
{

    ImGui::BeginTable("EPS Quarterly", 4);
    const char *table_labels[4]{"Date", "Estimate", "Actual", "Beat/\nMiss"};
    for (int i = 0; i < static_cast<int>(sizeof(table_labels)) / static_cast<int>(sizeof(table_labels[0])); i++)
    {
        ImGui::TableSetupColumn(table_labels[i], ImGuiTableColumnFlags_WidthFixed, 75.0f);
    }
    ImGui::TableHeadersRow();
    ImGui::TableNextRow();
    // ######################LOOPING THROUGH QUARTERLY EPS#############

    auto it = ptr->quartely_dates_inorder.begin();
    while (it != ptr->quartely_dates_inorder.end())
    {
        std::string outcome = "";
        std::unordered_map<std::string, float> &obj = ptr->quarterly_reporting_map.at(*it);
        std::string date = *it;
        float &estimate = obj.at("estimate");
        float &actual = obj.at("actual");
        if (actual > estimate)
        {
            outcome = "Beat";
        }
        else if (actual < estimate)
        {
            outcome = "Missed";
        }
        else
        {
            outcome = "Met";
        }
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", date.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.2f", estimate);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.2f", actual);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%s", outcome.c_str());
        ImGui::TableNextRow();
        it++;
    }
    ImGui::EndTable();
};

// PRIVATE FOR YEARLY INCOME REVENUE
void guiWindowOps::generate_yearly_earnings_table(StockFinancials::EarningsItems *ptr)
{
    ImGui::BeginTable("Yearly Earnings", 4);
    const char *table_labels[4]{"Date", "Revenue", "Earnings", "Profit Margin"};
    for (int i = 0; i < static_cast<int>(sizeof(table_labels)) / static_cast<int>(sizeof(table_labels[0])); i++)
    {
        ImGui::TableSetupColumn(table_labels[i], ImGuiTableColumnFlags_WidthFixed, 125.0f);
    }
    ImGui::TableHeadersRow();
    ImGui::TableNextRow();
    auto it = ptr->annual_dates_inorder.begin();
    while (it != ptr->annual_dates_inorder.end())
    {
        const std::unordered_map<std::string, long long int> &obj = ptr->annual_reporting_map.at(*it);
        long long int earnings = obj.at("earnings");
        long long int revenue = obj.at("revenue");
        double profit_margin_raw = (static_cast<double>(earnings) / revenue) * 100;
        // saving this for later need to generate chart
        double converted_profit_margin = std::round(profit_margin_raw * 100.0) / 100.0;
        ptr->percent_vec.push_back(converted_profit_margin);
        std::string shorted_rev = shorten_number(revenue);
        std::string shorted_earnings = shorten_number(earnings);
        std::string date(*it);
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", date.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", shorted_rev.c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", shorted_earnings.c_str());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.2f", converted_profit_margin);
        ImGui::TableNextRow();
        it++;
    }
    ImGui::EndTable();
}

// CHATGPT GENERATED  NEED HELP BIG TIME WITH THIS ONE
// PRIVATE FOR CHART TRENDS
void guiWindowOps::generate_profit_margin_chart(StockFinancials::EarningsItems *ptr)
{
    static float percent_arr[4];
    static float xs[] = {0, 1, 2, 3};

    // Store strings to keep memory valid
    static std::string label_strs[4];
    static const char *x_vals[4];

    int max_points = std::min(4, static_cast<int>(ptr->annual_dates_inorder.size()));
    for (int i = 0; i < max_points; i++)
    {
        label_strs[i] = ptr->annual_dates_inorder.at(i);
        x_vals[i] = label_strs[i].c_str(); // safe: string lives throughout function
    }

    auto it = ptr->percent_vec.begin();
    for (int i = 0; i < max_points && it != ptr->percent_vec.end(); ++i, ++it)
    {
        percent_arr[i] = *it;
    }

    if (ImPlot::BeginPlot("Profit Margin Over Time"))
    {
        ImPlot::SetupAxes(nullptr, "Profit %");
        ImPlot::SetupAxisTicks(ImAxis_X1, 0, 3, max_points, x_vals);
        ImPlot::PlotLine("Profit %", xs, percent_arr, max_points);
        ImPlot::EndPlot();
    }
}
// PRIVATE
void guiWindowOps::generate_metrics_table(const std::vector<std::unordered_map<std::string, std::string>> *metrics_bucket_ptr, const std::string &ticker)
{

    int col_tracker = 0;
    int max_column = 2;
    ImGui::BeginTable(ticker.c_str(), 3, ImGuiTableFlags_BordersOuter);
    ImGui::TableNextRow();
    for (const auto &map : *metrics_bucket_ptr)
    {
        const std::unordered_map<std::string, std::string> &generic_map = map;
        ImGui::TableSetColumnIndex(col_tracker);
        for (const auto &e : generic_map)
        {
            ImGui::Text("%s", e.first.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", e.second.c_str());
        }

        col_tracker++;
        if (col_tracker > max_column)
        {
            col_tracker = 0;
            ImGui::TableNextRow();
        }
    }
    ImGui::EndTable();

    ImGui::PushID(ticker.c_str());
    if (ImGui::Button("Close"))
    {
        selectable_booleans.at(ticker) = false;
    }
    ImGui::PopID();
}

// PRIVATE
void guiWindowOps::generate_metrics_table_special(const std::vector<std::unordered_map<std::string, std::string>> *metrics_bucket_ptr, const std::string &ticker)
{
    // total size is 8
    size_t lastitem = metrics_bucket_ptr->size() - 1; // should be 7

    int column_tracker = 0;
    int max_column = 2;
    ImGui::BeginTable("##CryptoTable", 3, ImGuiTableFlags_BordersOuter);
    ImGui::TableNextRow();
    for (int i = 0; i < static_cast<int>(lastitem); i++)
    {
        const std::unordered_map<std::string, std::string> &object = metrics_bucket_ptr->at(i);
        ImGui::TableSetColumnIndex(column_tracker);
        for (const auto &v : object)
        {
            ImGui::Text("%s", v.first.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", v.second.c_str());
        }
        column_tracker++;
        if (column_tracker > max_column)
        {
            column_tracker = 0;
            ImGui::TableNextRow();
        }
    }
    ImGui::EndTable();

    const std::unordered_map<std::string, std::string> &last_object = metrics_bucket_ptr->at(lastitem);
    float wrap_width = ImGui::GetContentRegionAvail().x;

    for (const auto &v : last_object)
    {
        ImGui::SeparatorText(v.first.c_str());
        ImGui::PushTextWrapPos(wrap_width);
        ImGui::Text("%s", v.second.c_str());
    }
    ImGui::PopTextWrapPos();
    ImGui::PushID(ticker.c_str());
    if (ImGui::Button("close"))
    {
        selectable_booleans.at(ticker) = false;
    }
    ImGui::PopID();
}

// ##############################################PUBLIC#######################

std::vector<StockInfo> *guiWindowOps::get_watchlist_vec_ptr()
{
    return watchlist_vec;
}

// PUBLIC
const std::vector<StockFinancials> *guiWindowOps::get_fin_vec_ptr() const
{
    return financial_vec;
}

// PUBLIC
const std::unordered_map<std::string, bool> &guiWindowOps::get_immutable_fin_bool_map_ref() const
{
    return financials_window_booleans;
}

// PUBLIC
std::unordered_map<std::string, bool> &guiWindowOps::get_financial_window_booleans()
{
    return financials_window_booleans;
};

// PUBLIC

std::unordered_map<std::string, bool> &guiWindowOps::get_chart_booleans()
{
    return chart_booleans;
}

// PUBLIC
const std::unordered_map<std::string, bool> &guiWindowOps::get_immutable_chart_booleans_map()
{
    return chart_booleans;
}

// PUBLIC
std::unordered_map<std::string, bool> &guiWindowOps::get_selectable_booleans()
{
    return selectable_booleans;
};

// PUBLIC
std::unordered_map<std::string, bool> &guiWindowOps::get_etf_holdings_booleans()
{
    return etf_holdings_booleans;
}

// #################ACTIONS######################3########

void guiWindowOps::delete_from_boolean_map(std::unordered_map<std::string, bool> &map, const std::string &s)
{
    if (auto it = map.find(s); it != map.end())
    {
        map.erase(it);
    }
}

// PUBLIC
void guiWindowOps::make_api_call(bool single_call, bool watchlistCall, bool apiKeyCall, bool mulit_financial_call, bool multi_watchlist_call, bool multi_etf_holdings_call, bool summary_call, bool chart_call, const char *ticker)
{

    if (single_call)
    {
        if (watchlistCall) //
        {
            file_status.watchlist_empty = backendops->get_watchlist_file_status();
            backendops->run_add_to_watchlist_operations(std::string(ticker));
        }

        if (summary_call)
        {
            backendops->run_generate_summary_operations(std::string(ticker));
        }

        if (chart_call)
        {
            backendops->run_charting_operations(std::string(ticker));
        }

        if (apiKeyCall) //
        {
            std::string y(input_bucket);
            backendops->run_add_api_key_operations(y);
            file_status.api_key_empty = backendops->get_api_file_status();
            reset_arr();
        }
    }
    else
    {
        if (multi_watchlist_call)
        {
            backendops->run_multi_watchlist_api_calls_operations();
        }
        else if (mulit_financial_call)
        {
            backendops->run_financials_operations(ticker);
        }
        else if (multi_etf_holdings_call)
        {
            backendops->run_etf_holdings_operations(ticker);
        }
    }
}

// PUBLIC
void guiWindowOps::run_chart_vec_manager()
{
    /* LOOK AT THE BOOLEANS MAP FOR CHARTS TO SEE IF THEY ARE SET TO FALSE, IF THEY ARE SET TO FALSE,
    THEN LOOK AT THE CHART OBJECTS IN THE CHARTS VECTOR, IM FINDING THE OBJECT IN CHARTS VECTOR BY USING BOOLEANS MAP KEY WHICH ARE TICKERS.
    IF THE OBJECTS ATTR, IS_DISPLAYING = TRUE, THEN DELETE THE OBJECT FROM CHARTS VECTOR.
    THIS MEANS THAT THE CHART WAS ONCE OPEN BUT THEN IT WAS CLOSED AND ITS NOT BEING USED.
    THE MOST IM WILLING TO STORE IN THE CHARTS VECTOR IS FIVES CHART OBJECTS FOR SPACE MANAGEMENT.
    CHART OBJECTS HOLD A LOT OF DATA*/

    const int max = 5;
    if (static_cast<int>(chart_info_vec->size()) > max)
    {
        auto chart_map_it = chart_booleans.begin();
        while (chart_map_it != chart_booleans.end())
        {
            if (!(chart_map_it->second))
            {
                std::string target = chart_map_it->first;
                // SINCE NEW ITEMS IN CHART VEC PUSHED TO THE BACK, LOOK AT OLDEST CREATED ONES AND DELETE THEM IF NOT OPEN
                auto chart_vec_it = std::find_if(chart_info_vec->begin(), chart_info_vec->end(), [&target](const ChartInfo &obj)
                                                 { return obj.get_ticker() == target; });
                if (chart_vec_it != chart_info_vec->end())
                {
                    if (chart_vec_it->get_is_displaying())
                    {
                        chart_info_vec->erase(chart_vec_it);
                    }
                }
            }

            chart_map_it++;
        }
    }
}

// ######################WINDOWS#########################
//  PUBLIC
void guiWindowOps::generate_watchlist(GLFWwindow *window, ImFont *large_font)
{

    // WINDOW STYLES
    int glfw_width, glfw_height;
    glfwGetWindowSize(window, &glfw_width, &glfw_height);
    ImVec2 bigWindowSize = ImVec2(glfw_width, glfw_height);
    ImGui::SetNextWindowSize(ImVec2(bigWindowSize.x * 0.75, bigWindowSize.y * 0.75), ImGuiCond_FirstUseEver);

    ImVec2 watchlist_pos = ImVec2(10.0f, 10.0f);
    ImGui::SetNextWindowPos(watchlist_pos, ImGuiCond_Once);
    ImGui::GetStyle().WindowRounding = 10.0f;
    ImGui::GetStyle().FrameRounding = 10.0f; // This rounds all buttons globally (set a value > 0)

    ImGuiWindowFlags flags;

    if (program_state.trigger_error)
    {
        flags = flag_settings.with_error_flags;
    }
    else
    {
        flags = flag_settings.no_error_flags;
    }

    ImGui::Begin("Watchlist", NULL, flags);

    generate_menubar();
    if (!backendops->get_watchlist_file_status())
    {
        float window_width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosX(window_width - 75);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.25f, 0.8f));
        if (ImGui::Button("Refresh"))
        {
            api_workflow.need_make_api = true;
            program_state.refresh_watchlist = true;
            api_workflow.multi_financial_call = false; // redundancy
            api_workflow.multi_watchlist_call = true;
        }
        ImGui::PopStyleColor();
    }
    if (watchlist_vec->empty())
    {
        ImGui::PushFont(large_font);
        ImVec2 windosize = ImGui::GetWindowSize();
        ImVec2 textsize = ImGui::CalcTextSize("-- Watchlist is empty! --");
        ImGui::SetCursorPos(
            ImVec2(
                (windosize.x / 2) - textsize.x * 0.5f,
                windosize.y / 2));
        ImGui::Text("-- Watchlist is empty! --");
        ImGui::PopFont();
    }

    else
    {
        if (ImGui::BeginTable("Your Watchlist", 5, ImGuiTableFlags_BordersOuter))
        {

            std::string header_text[5]{"Ticker", "Company\nName", "Price\nChange", "Current\nPrice", "Actions"};

            for (int i = 0; i < 5; i++)
            {
                if (i == 1)
                {
                    ImGui::TableSetupColumn(header_text[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
                }
                else if (i == 4)
                {
                    ImGui::TableSetupColumn(header_text[i].c_str(), ImGuiTableColumnFlags_WidthFixed, 200.0f);
                }
                else
                {
                    ImGui::TableSetupColumn(header_text[i].c_str(), ImGuiTableColumnFlags_WidthFixed, 75.0f);
                }
            }

            ImGui::TableHeadersRow();

            float next_pos = ImGui::GetCursorPosY();
            for (const auto &stock : *(watchlist_vec))
            {

                ImGui::TableNextRow();

                ImGui::SetCursorPosY(next_pos);

                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.647f, 0.0f, 1.0f));
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(stock.get_ticker().c_str(), &selectable_booleans[stock.get_ticker()]))
                {
                    selectable_booleans[stock.get_ticker()] = true;
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
                ImGui::PopStyleColor(2);
                // COLUMN 2

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", stock.get_company_name().c_str());
                // COLUMN 3
                ImGui::TableSetColumnIndex(2);
                if (stock.get_price_change() > 0.00)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 0.7f));
                    ImGui::Text("%.2f", stock.get_price_change());
                    ImGui::PopStyleColor();
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.0f, 0.0f, 0.7f));
                    ImGui::Text("%.2f", stock.get_price_change());
                    ImGui::PopStyleColor();
                }

                // COLUMN 4
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.2f", stock.get_market_price());

                // COLUMN 5
                // Set the style for the button (background and text colors)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Greenish button background
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Button color when hovered
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.8f, 0.4f, 1.0f));  // Button color when active (clicked)

                // Change the button text color
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                ImGui::TableSetColumnIndex(4);

                // DROPBOX COMBO BOX GENERATION FUNCTION
                if (stock.get_quote_type() == "EQUITY")
                    generate_equity_dropbox(stock);

                else if (stock.get_quote_type() == "ETF")
                    generate_etf_dropbox(stock);

                else if (stock.get_quote_type() == "CRYPTOCURRENCY")
                    generate_crypto_dropbox(stock);

                else
                    generate_equity_dropbox(stock);

                ImGui::PopStyleColor(4);

                if (!selectable_booleans.empty())
                {

                    auto it = selectable_booleans.find(stock.get_ticker());
                    if (it != selectable_booleans.end())
                    {

                        if (selectable_booleans.at(stock.get_ticker()))
                        {
                            if (!backendops->summary_already_generated(stock.get_ticker()))
                            {
                                api_workflow.need_make_api = true;
                                api_workflow.ticker = stock.get_ticker();
                                api_workflow.summary_call = true;
                                popup_booleans.making_api_call_window = true;
                            }

                            else
                            {
                                const size_t pos = backendops->get_metrics_position_in_vec(stock.get_ticker());
                                if (static_cast<int>(pos) != -1)
                                {

                                    Metrics &object = metrics_vec->at(pos);
                                    const std::vector<std::unordered_map<std::string, std::string>> *metrics_bucket_ptr = object.get_metrics_bucket_ptr();
                                    ImGui::EndTable();
                                    if (!(object.get_quote_type() == "CRYPTOCURRENCY"))
                                    {
                                        generate_metrics_table(metrics_bucket_ptr, stock.get_ticker());
                                    }
                                    else
                                    {
                                        generate_metrics_table_special(metrics_bucket_ptr, stock.get_ticker());
                                    }
                                    float current_pos = ImGui::GetCursorPosY();
                                    next_pos = current_pos;
                                    ImGui::BeginTable("Your Watchlist", 5, ImGuiTableFlags_BordersOuter);
                                }
                            }
                        } //
                    }
                }
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

// PUBLIC
bool guiWindowOps::generate_stockfinancials_window(GLFWwindow *window, const std::string &ticker, ImFont *font_change)
{
    int class_pos;
    class_pos = get_class_pos(ticker);
    if (class_pos == -1)
    {
        return false;
    }

    static float windowYpos = 10.0;

    int glfw_width, glfw_height;
    glfwGetWindowSize(window, &glfw_width, &glfw_height);
    ImVec2 bigWindowSize = ImVec2(glfw_width, glfw_height);
    ImGui::SetNextWindowSize(ImVec2(bigWindowSize.x * 0.85, bigWindowSize.y * 0.85), ImGuiCond_FirstUseEver);

    ImVec2 fin_win_pos = ImVec2(bigWindowSize.x * 0.75f, windowYpos);
    ImGui::SetNextWindowPos(fin_win_pos, ImGuiCond_Once);
    ImGui::GetStyle().WindowRounding = 10.0f;
    ImGui::GetStyle().FrameRounding = 10.0f;
    windowYpos += 1.0;
    if (windowYpos > bigWindowSize.y * 0.90)
    {
        windowYpos = 10.0;
    }

    StockFinancials &class_ref = financial_vec->at(class_pos);

    StockFinancials::BalanceSheetItems *bs_ptr = class_ref.get_balancesheet_ptr();
    StockFinancials::CashflowItems *cf_ptr = class_ref.get_cashflow_ptr();
    StockFinancials::EarningsItems *earnings_ptr = class_ref.get_earnings_ptr();
    std::string text = "Financials for " + ticker;
    auto &ref = financials_window_booleans[ticker];

    ImVec4 orange = ImVec4(1.0f, 0.647f, 0.0f, 1.0f); // RGB: (255, 165, 0), A: 1.0 (full opacity)
    ImVec4 blue = ImVec4(0.8f, 0.6f, 1.0f, 1.0f);     // RGB: (255, 165, 0), A: 1.0 (full opacity)
    ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);    // RGB: (0, 255, 0), A: 1.0 (fully opaque)

    ImGui::PushFont(font_change);

    ImGui::Begin(text.c_str(), &ref, ImGuiWindowFlags_NoCollapse);
    ImGui::PopFont();
    generate_table_label("Earnings Section", font_change, green);
    ImGui::SeparatorText("Quarterly EPS");
    generate_eps_table(earnings_ptr);
    ImGui::SeparatorText("Yearly Profit Margins");
    generate_yearly_earnings_table(earnings_ptr);
    ImGui::SeparatorText("Profit Margin Trends");
    generate_profit_margin_chart(earnings_ptr);

    generate_table_label("Balance Sheet", font_change, orange);
    generate_table(bs_ptr, "Balance Sheet");
    generate_table_label("Cashflow", font_change, blue);
    generate_table(cf_ptr, "Cashflow");

    ImGui::End();

    return true;
}

// PUBLIC
void guiWindowOps::apiKeyFileEmptyWindow(GLFWwindow *window, ImFont *default_f, ImFont *title_f, ImFont *midsize_f, ImFont *default_italic)
{
    int glfw_width, glfw_height;
    glfwGetWindowSize(window, &glfw_width, &glfw_height);
    ImVec2 bigWindowSize = ImVec2(glfw_width, glfw_height);
    ImGui::SetNextWindowSize(ImVec2(bigWindowSize.x * 0.75f, bigWindowSize.y * 0.75f), ImGuiCond_FirstUseEver);
    ImVec2 watchlist_pos = ImVec2(10.0f, 10.0f);
    ImGui::SetNextWindowPos(watchlist_pos, ImGuiCond_Once);
    ImGui::GetStyle().WindowRounding = 10.0f;
    ImGui::GetStyle().FrameRounding = 10.0f;

    // Start the window
    ImGui::Begin("Stock Widget", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    generate_menubar();
    ImGui::PushFont(title_f);
    ImVec2 window_size = ImGui::GetWindowSize();
    ImGui::SetCursorPosX(
        (window_size.x - ImGui::CalcTextSize("Welcome to Stock Widget").x) * 0.5f);
    ImGui::Text("Welcome to Stock Widget");
    ImGui::PopFont();
    ImGui::SetCursorPosY(window_size.y * 0.25f);
    ImGui::SetCursorPosX(25.0f);
    ImGui::PushFont(midsize_f);
    ImGui::Text("First You need to add your API key");
    ImGui::PopFont();
    ImGui::SetCursorPosX(50.0f);
    ImGui::Text("-If you dont have an api key you must sign up at rapidapi.com");
    ImGui::SetCursorPosX(50.0f);
    ImGui::Text("-After sigining up at rapidapi.com:");
    ImGui::SetCursorPosX(50.0f);
    ImGui::Text("-Go here: ");
    ImGui::SameLine();
    ImGui::PushFont(default_italic);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImGui::Selectable(" https://rapidapi.com/3b-data-3b-data-default/api/yahoo-finance-real-time1"))
    {
        openWebsite("https://rapidapi.com/3b-data-3b-data-default/api/yahoo-finance-real-time1");
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(2);
    ImGui::SetCursorPosX(50.0f);
    ImGui::Text("-sign up for this api. there is a free tier but i do recommend the $10 per month tier");
    ImGui::SetCursorPosX(50.0f);
    ImGui::Text("-After signing up for the API: Go to Menu->Add API key");
    ImGui::SetCursorPosX(50.0f);
    ImGui::PushFont(default_italic);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
    ImGui::Text("-disclosure: Im not making any money off this API. This is just the API I decided to build on.");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::PushFont(midsize_f);
    ImGui::Text("To change API key");
    ImGui::PopFont();
    ImGui::SetCursorPosX(50);
    ImGui::Text("-Go to Menu->Change API key");
    ImGui::PushFont(midsize_f);
    ImGui::Text("How to add to your to watchlist");
    ImGui::PopFont();
    ImGui::SetCursorPosX(50);
    ImGui::Text("-Go to Menu->Add Stock");

    ImGui::End();
}

// PUBLIC
void guiWindowOps::making_api_call_window(GLFWwindow *window, ImFont *midsize_f, bool make_api_call)
{
    if (make_api_call)
    {
        set_window_parameters(window, midsize_f);
        ImGui::Begin("Making Api Call", &popup_booleans.open_api_window);
        ImGui::Text("Getting info from API");
        ImGui::PopFont();
        ImGui::Text("Please Wait...");
        ImGui::End();
    }
}

// PUBLIC
// THIS IS FOR CHANGE OR ADD API KEY
void guiWindowOps::Dynamic_apikey_operation_window(GLFWwindow *window, const std::string &text, const std::string &button_label, ImFont *font_change, bool &open)
{
    set_window_parameters(window, font_change);
    ImGui::Begin("Add Api Key", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", text.c_str());
    ImGui::PopFont();
    ImGui::InputText("Enter Key", input_bucket, IM_ARRAYSIZE(input_bucket));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);
    ImGui::Text("use Ctrl-v to paste");
    ImGui::SameLine();
    float text_size = ImGui::CalcTextSize(button_label.c_str()).x + 15;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x) - text_size);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));
    if (ImGui::Button(button_label.c_str()))
    {

        // TURN OUTPUT TO STRING THEN PASS TO ADD API OPERATIONS BACKEND
        // ARRAY HAS TO BE OUTSIDE OF LOOP --SEE TOP OF MAIN FUNCTIONS FOR ARRAY DECLARATION
        api_workflow.need_make_api = true;
        api_workflow.ticker = "none";
        api_workflow.api_key_entry_call = true;
        api_workflow.watchlist_call = false; // redundancy
        api_workflow.try_again = true;
        open = false;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::PopStyleColor();
    ImGui::End();
}

// PUBLIC
void guiWindowOps::add_to_watchlist_window(GLFWwindow *window, ImFont *font_change, bool &open)
{
    set_window_parameters(window, font_change);
    ImGui::Begin("Add To Watchlist", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    ImGui::Text("Add Ticker");
    ImGui::PopFont();
    ImGui::InputText("Enter Ticker", input_bucket, IM_ARRAYSIZE(input_bucket));
    ImVec2 window_size = ImGui::GetWindowSize();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);
    ImGui::Text("Crypto is suffixed -USD Example BTC-USD");
    ImGui::SameLine();
    ImGui::SetCursorPosX(window_size.x - 50);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));
    if (ImGui::Button("Add"))
    {
        popup_booleans.open_add_to_watchlist_window = false;
        api_workflow.ticker = std::string(input_bucket);
        api_workflow.need_make_api = true;
        api_workflow.watchlist_call = true;
        api_workflow.api_key_entry_call = false; // redundancy
        std::string copy = api_workflow.ticker;
        std::string uppercase_copy = make_uppercase(copy);
        financials_window_booleans.insert_or_assign(uppercase_copy, false);
        selectable_booleans.insert_or_assign(uppercase_copy, false);
        chart_booleans.insert_or_assign(uppercase_copy, false);
        etf_holdings_booleans.insert_or_assign(uppercase_copy, false);
        reset_arr();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::PopStyleColor();
    ImGui::End();
}

// PUBLIC
bool guiWindowOps::display_chart_window(const std::string &ticker)
{

    int pos = backendops->get_chart_obj_position_in_vec(ticker);

    if ((pos == -1))
    {
        return false;
    }

    ChartInfo &object_ref = chart_info_vec->at(pos);
    bool &open_ref = chart_booleans.at(ticker);

    std::ostringstream oss;
    oss << ticker << " Chart";
    std::string windowid = oss.str();

    ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_FirstUseEver);
    ImGui::Begin(windowid.c_str(), &open_ref);
    std::vector<double> &plots = object_ref.get_price_data_ref();
    std::vector<double> &tstamps = object_ref.get_timestamp_ref();
    const std::vector<double> &below_avg = object_ref.get_below_avg_vec_const();
    int size = static_cast<int>(tstamps.size());
    const double &avg = object_ref.get_avg_price();

    std::string stock_price_label = object_ref.get_ticker() + " Price";
    std::string axis_two_label = object_ref.get_ticker() + " Avg Price";
    std::string chart_label = object_ref.get_ticker() + " 1yr Chart";
    std::ostringstream avg_oss;
    avg_oss << "Below avg price of " << std::fixed << std::setprecision(2) << object_ref.get_avg_price();
    if (ImPlot::BeginPlot(chart_label.c_str()))
    {

        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
        // PLOT 1
        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0, 0, 0, 0));
        ImPlot::PlotLine(stock_price_label.c_str(), tstamps.data(), plots.data(), size, ImPlotFlags_None);
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1.0f, 0.2f, 0.2f, 0.25f));
        ImPlot::PlotShaded(avg_oss.str().c_str(), tstamps.data(), below_avg.data(), size, avg);
        ImPlot::PopStyleColor();
        ImPlot::EndPlot();
    }
    ImGui::End();
    return true;
}

// PUBLIC
bool guiWindowOps::generate_etf_holdings_window(GLFWwindow *window, const std::string &ticker, ImFont *font_change)
{
    int pos = backendops->get_holdings_position_in_vec(ticker);
    if (pos == -1)
        return false;
    ETF_Holdings &etf_ref = etf_holdings_vec->at(pos);
    bool &open_ref = etf_holdings_booleans.at(ticker);

    std::ostringstream oss;
    oss << ticker << " Holdings";
    std::string windowid = oss.str();

    int glfw_width, glfw_height;
    glfwGetWindowSize(window, &glfw_width, &glfw_height);
    ImVec2 bigWindowSize = ImVec2(glfw_width, glfw_height);
    ImGui::SetNextWindowSize(ImVec2(bigWindowSize.x * 0.85, bigWindowSize.y * 0.85), ImGuiCond_FirstUseEver);

    ImGui::GetStyle().WindowRounding = 10.0f;
    ImGui::GetStyle().FrameRounding = 10.0f;

    const std::unordered_map<std::string, float> &percent_map = etf_ref.get_holdings_float();
    const std::unordered_map<std::string, std::string> &company_name_map = etf_ref.get_holdings_company_name();
    const std::vector<std::string> &keys = etf_ref.get_holidings_keys();

    ImGui::Begin(ticker.c_str(), &open_ref, ImGuiWindowFlags_NoCollapse);
    if (!keys.empty())
    {

        ImVec2 window_size = ImGui::GetWindowSize();
        ImGui::PushFont(font_change);
        ImGui::Text("Top Holdings");
        ImGui::SameLine();
        ImGui::SetCursorPosX(window_size.x * 0.6f);
        ImGui::Text("Sector Weights");
        ImGui::PopFont();

        ImGui::BeginTable(windowid.c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(window_size.x * 0.4f, window_size.y * 0.5f));
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Holding Percent", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        for (auto it = keys.begin(); it != keys.end(); it++)
        {
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", company_name_map.at(*it).c_str());
            ImGui::TableSetColumnIndex(1);
            ImVec2 cursor_pos = ImGui::GetCursorPos();
            ImGui::SetCursorPosX(cursor_pos.x + 35);
            ImGui::Text("%.2f", percent_map.at(*it));
            ImGui::TableNextRow();
        }
        if (etf_ref.get_other_holdings() > 0)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Other");
            ImGui::TableSetColumnIndex(1);
            ImVec2 cursor_pos = ImGui::GetCursorPos();
            ImGui::SetCursorPosX(cursor_pos.x + 35);
            ImGui::Text("%.2f", etf_ref.get_other_holdings());
        }
        ImGui::EndTable();
        ImGui::SameLine();

        const std::vector<std::string> &sector_names = etf_ref.get_sector_names_vec();
        const std::vector<double> &sector_weights = etf_ref.get_sector_weights_vec();

        std::vector<const char *> sector_names_cstr;
        for (const auto &name : sector_names)
            sector_names_cstr.push_back(name.c_str());

        int slices = static_cast<int>(sector_names.size());
        ImVec2 plot_size = ImVec2(window_size.x * 0.55f, window_size.y * 0.5f);

        if (ImPlot::BeginPlot(ticker.c_str(), plot_size, ImPlotFlags_NoMouseText | ImPlotFlags_Equal))
        {
            ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines);
            ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines);
            ImPlot::PlotPieChart(sector_names_cstr.data(), sector_weights.data(), slices, 0.5, 0.5, 0.4, "%.2f", 90, ImPlotPieChartFlags_Exploding);
            ImPlot::EndPlot();
        }
    }
    else
    {
        ImGui::PushFont(font_change);
        ImGui::Text("ETF %s has no top holdings or sector weights", ticker.c_str());
        ImGui::PopFont();
    }

    ImGui::SeparatorText("Profile Description");
    std::unordered_map<std::string, std::string> &profile_desc = etf_ref.get_profile_map();
    ImGui::TextWrapped("%s", profile_desc.at("profile").c_str());

    ImGui::End();
    return true;
}

// PUBLIC
void guiWindowOps::error_window(GLFWwindow *window, ImFont *font_change, bool &open)
{
    set_window_parameters(window, font_change);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    ImGui::Begin("Error Window", &open, ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("%s", temp_message.c_str());
    ImVec2 windowsize = ImGui::GetWindowSize();
    ImGui::SetCursorPos(ImVec2(windowsize.x - 100, windowsize.y - 50));
    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    if (api_workflow.try_again)
    {
        if (ImGui::Button("Try Again"))
        {
            program_state.trigger_error = false;
            reset_arr();
            open = false;
        }
    }

    ImGui::SetCursorPos(ImVec2(10.0f, windowsize.y - 50));
    if (ImGui::Button("Close"))
    {

        reset_arr();
        reset_necessary_guiops_booleans();
        open = false;
    }
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
}

// PUBLIC
void guiWindowOps::setup_window_boolean_maps()
{
    for (const auto &watchlist_item : *watchlist_vec)
    {
        etf_holdings_booleans.insert_or_assign(watchlist_item.get_ticker(), false);
        chart_booleans.insert_or_assign(watchlist_item.get_ticker(), false);
        selectable_booleans.insert_or_assign(watchlist_item.get_ticker(), false);
        financials_window_booleans.insert_or_assign(watchlist_item.get_ticker(), false);
    }
}

// PUBLIC
void guiWindowOps::reset_necessary_guiops_booleans()
{

    api_workflow.need_make_api = false;
    api_workflow.multi_financial_call = false;
    api_workflow.multi_watchlist_call = false;
    api_workflow.try_again = false;
    api_workflow.summary_call = false;
    api_workflow.chart_call = false;
    api_workflow.api_key_entry_call = false;
    api_workflow.multi_etf_holdings_call = false;

    program_state.adding_api = false;
    program_state.changing_api = false;
    program_state.program_startup = false;
    program_state.refresh_watchlist = false;
    program_state.dropbox_financial_clicked = false;
    program_state.dropbox_etf_holdings_clicked = false;
    program_state.dropbox_chart_clicked = false;
    program_state.trigger_error = false;

    popup_booleans.open_add_to_watchlist_window = false;
    popup_booleans.open_dynamic_window = false;
    popup_booleans.open_add_to_watchlist_window = false;
    popup_booleans.open_dynamic_window = false;
    popup_booleans.making_api_call_window = false;
}
