#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <memory>
#include "../include/guiWindowOps.h"
#include "../include/GuiErrorWindow.h"

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main()
{
    std::unique_ptr<guiWindowOps> g;
    try
    {
        g = std::make_unique<guiWindowOps>();
    }
    catch (std::filesystem::filesystem_error &e)
    {
        GuiErrorWindow ge;
        ge.open_fatal_error_window();
    }

    const std::unordered_map<std::string, bool> &financial_booleans_map = g->get_immutable_fin_bool_map_ref();
    const std::unordered_map<std::string, bool> &chart_booleans_map = g->get_immutable_chart_booleans_map();

    // IMGUI CODE BEGINS
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return -1; // Exit if GLFW fails to initialize
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(1200, 600, "Stock Widget", NULL, NULL);
    if (window == nullptr)
    {
        glfwTerminate();
        return -1; // Exit if the window creation fails
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
#ifdef DEV_MODE
    std::string font_base_path = "../fonts/static/";
#else
    std::string font_base_path = "/usr/local/share/stockwidget/fonts/static/";

#endif
    std::string default_font_path = font_base_path + "Roboto-Medium.ttf";
    std::string default_font_italic_path = font_base_path + "Roboto-MediumItalic.ttf";
    std::string midsize_font_path = font_base_path + "Roboto-Medium.ttf";
    std::string title_font_path = font_base_path + "Roboto-Bold.ttf";
    ImFont *default_font = io.Fonts->AddFontFromFileTTF(default_font_path.c_str(), 16.0f);
    ImFont *default_font_italic = io.Fonts->AddFontFromFileTTF(default_font_italic_path.c_str(), 16.0f);
    ImFont *midsize_font = io.Fonts->AddFontFromFileTTF(midsize_font_path.c_str(), 24.0f);
    ImFont *title_font = io.Fonts->AddFontFromFileTTF(title_font_path.c_str(), 32.0f);

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // PROGRAM STARTUP
        if (g->program_state.program_startup && !g->program_state.trigger_error && !g->file_status.watchlist_empty)
        {
            g->api_workflow.multi_watchlist_call = true;
            g->making_api_call_window(window, midsize_font, g->api_workflow.need_make_api = true);
        }
        else
        {
            g->program_state.program_startup = false;
        }

        if (g->file_status.api_key_empty)
        {
            g->apiKeyFileEmptyWindow(window, default_font, title_font, midsize_font, default_font_italic);
        }

        // WATCHLIST
        if (!g->program_state.program_startup && !g->file_status.api_key_empty)
        {
            g->generate_watchlist(window, title_font);
        }

        // OPEN ERROR WINDOW
        if (g->program_state.trigger_error)
        {
            g->error_window(window, midsize_font, g->popup_booleans.open_error_window = true);
        }

        // API KEY CHANGE OR SET WINDOW
        if (g->popup_booleans.open_dynamic_window && !g->program_state.trigger_error)
        {
            if (g->program_state.adding_api)
            {
                g->Dynamic_apikey_operation_window(
                    window,
                    "Enter Your Api Key",
                    "Add Key",
                    midsize_font,
                    g->popup_booleans.open_dynamic_window

                );
            }
            else if (g->program_state.changing_api)
            {
                g->Dynamic_apikey_operation_window(
                    window,
                    "Enter Your Api Key",
                    "Change Key",
                    midsize_font,
                    g->popup_booleans.open_dynamic_window

                );
            }
        }

        // OPENS ADD STOCK WINDOW
        if (g->popup_booleans.open_add_to_watchlist_window)
        {

            g->add_to_watchlist_window(window, midsize_font, g->popup_booleans.open_add_to_watchlist_window);
        }

        // GENERATE THE WINDOW IF AN API CALL IS NEEDED
        if (!g->program_state.program_startup && g->api_workflow.need_make_api)
        {
            g->popup_booleans.making_api_call_window = true;
            g->making_api_call_window(window, midsize_font, g->api_workflow.need_make_api);
        }

        // // OPEN ERROR WINDOW
        // if (g->program_state.trigger_error)
        // {
        //     g->error_window(window, midsize_font, g->popup_booleans.open_error_window = true);
        // }

        // OPENING STOCK FINANCIALS WINDOW
        for (const auto &pair : financial_booleans_map)
        {
            if (pair.second)
            {
                if (g->generate_stockfinancials_window(window, pair.first, midsize_font))
                {
                    // NO STATEMENT NEEDED HERE
                }
            }
        }

        // OPENING CHARTS
        for (const auto &pair : chart_booleans_map)
        {
            if (pair.second)
            {
                if (g->display_chart_window(pair.first))
                {
                    // NO STATEMENT NEEDED
                }
            }
        }

        g->run_chart_vec_manager();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        // API SEQUENCE AND ERROR HANDLING
        if (g->api_workflow.need_make_api && !g->program_state.trigger_error)
        {

            // MULTI API CALL SECTION
            if (g->program_state.program_startup || g->program_state.refresh_watchlist || g->program_state.dropbox_financial_clicked)
            {

                g->api_workflow.try_again = true;

                if (g->program_state.program_startup || g->program_state.refresh_watchlist)
                {
                    g->api_workflow.ticker = "none";
                }
                try
                {
                    g->make_api_call(g->api_workflow.single_api_call = false,
                                     g->api_workflow.watchlist_call = false,
                                     g->api_workflow.api_key_entry_call = false,
                                     g->api_workflow.multi_financial_call,
                                     g->api_workflow.multi_watchlist_call,
                                     g->api_workflow.summary_call = false,
                                     g->api_workflow.chart_call = false,
                                     g->api_workflow.ticker.c_str());
                }
                catch (BackendException &e)
                {
                    g->program_state.trigger_error = true;
                    g->popup_booleans.open_error_window = true;
                    g->temp_message = e.what();

                    if (e.getType() == BackendException::ErrorType::FINANCIALS_TICKER_UNSUPPORTED ||
                        e.getType() == BackendException::ErrorType::API_KEY_FILE_EMPTY ||
                        e.getType() == BackendException::ErrorType::API_INFO_CURRENTLY_UNAVAILABLE)
                    {
                        g->api_workflow.try_again = false;
                    }
                    else if (e.getType() == BackendException::ErrorType::FINANICALS_JSON_EMPTY ||
                             e.getType() == BackendException::ErrorType::JSON_PARSE_FAILURE ||
                             e.getType() == BackendException::ErrorType::API_CALL_FAILED ||
                             e.getType() == BackendException::ErrorType::MULTI_API_CALL_TOTAL_FAILURE ||
                             e.getType() == BackendException::ErrorType::MULTI_PARSING_FAILURE)
                    {
                        g->api_workflow.try_again = true;
                    }

                    continue;
                }
            }
            // SINGLE API CALL SECTION
            else
            {

                try
                {
                    g->make_api_call(g->api_workflow.single_api_call = true,
                                     g->api_workflow.watchlist_call,
                                     g->api_workflow.api_key_entry_call,
                                     g->api_workflow.multi_financial_call = false,
                                     g->api_workflow.multi_watchlist_call = false,
                                     g->api_workflow.summary_call,
                                     g->api_workflow.chart_call,
                                     g->api_workflow.ticker.c_str());
                }
                catch (BackendException &e)
                {
                    g->api_workflow.need_make_api = false;
                    g->popup_booleans.making_api_call_window = false;
                    g->program_state.trigger_error = true;
                    g->popup_booleans.open_error_window = true;
                    g->api_workflow.try_again = true;
                    g->temp_message = e.what();
                    if (e.getType() == BackendException::ErrorType::API_CONFIRMATION_FAILED ||
                        e.getType() == BackendException::ErrorType::JSON_PARSE_FAILURE ||
                        e.getType() == BackendException::ErrorType::FILE_WRITE_COMMON ||
                        e.getType() == BackendException::ErrorType::API_CALL_FAILED)

                    {
                        if (e.getType() == BackendException::ErrorType::FILE_WRITE_COMMON)
                        {
                            std::vector<StockInfo> *watchlist_vec_ptr = g->get_watchlist_vec_ptr();
                            std::string target = make_uppercase(g->api_workflow.ticker);
                            auto it = std::find_if(
                                watchlist_vec_ptr->begin(), watchlist_vec_ptr->end(),
                                [&target](const StockInfo &stock)
                                { return stock.get_ticker() == target; });

                            if (it != watchlist_vec_ptr->end())
                            {
                                watchlist_vec_ptr->erase(it);
                            }
                            std::unordered_map<std::string, bool> &fin_bool_map = g->get_financial_window_booleans();
                            std::unordered_map<std::string, bool> &sel_bool_map = g->get_selectable_booleans();
                            std::unordered_map<std::string, bool> &chart_bool_map = g->get_chart_booleans();
                            g->delete_from_boolean_map(fin_bool_map, target);
                            g->delete_from_boolean_map(sel_bool_map, target);
                            g->delete_from_boolean_map(chart_bool_map, target);
                        }
                        else if (e.getType() == BackendException::ErrorType::API_CONFIRMATION_FAILED)
                        {
                            g->api_workflow.try_again = false;
                        }
                        else
                        {
                            g->popup_booleans.open_dynamic_window = true;
                            g->api_workflow.try_again = true;
                        }
                    }
                    else if (e.getType() == BackendException::ErrorType::API_INFO_CURRENTLY_UNAVAILABLE || e.getType() == BackendException::ErrorType::NO_EQUITY_TYPE)
                    {
                        g->popup_booleans.open_dynamic_window = true;
                        g->api_workflow.try_again = false; // redudancy
                    }
                    else if (e.getType() == BackendException::ErrorType::INVALID_TICKER)
                    {
                        g->popup_booleans.open_add_to_watchlist_window = true;
                        // WHEN I CLICK ADD STOCK BUTTON IM ADDING TICKERTO THESE MAPS, REGARDLESS IF ITS A GOOD OR BAD TICKER
                        // IF ITS AN INVALID TICKER I NEED TO REMOVE FROM THE MAPS SO I DONT HAVE HANGING BAD TICKERS IN THE MAP
                        std::string copy = make_uppercase(g->api_workflow.ticker);
                        std::unordered_map<std::string, bool> &fin_bool_map = g->get_financial_window_booleans();
                        std::unordered_map<std::string, bool> &sel_bool_map = g->get_selectable_booleans();
                        std::unordered_map<std::string, bool> &chart_bool_map = g->get_chart_booleans();
                        g->delete_from_boolean_map(fin_bool_map, copy);
                        g->delete_from_boolean_map(sel_bool_map, copy);
                        g->delete_from_boolean_map(chart_bool_map, copy);
                    }
                    continue;
                }
            }
            if (g->program_state.program_startup)
                g->setup_window_boolean_maps();
            g->reset_necessary_guiops_booleans();
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
