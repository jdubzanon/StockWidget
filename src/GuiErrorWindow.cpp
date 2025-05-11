#include "GuiErrorWindow.h"

void GuiErrorWindow::glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int GuiErrorWindow::open_fatal_error_window()
{
    // IMGUI CODE BEGINS
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        std::cout << "GLFW Initialization failed!" << std::endl;
        return -1; // Exit if GLFW fails to initialize
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(1200, 600, "Stock Widget", NULL, NULL);
    if (window == nullptr)
    {
        std::cout << "Window creation failed!" << std::endl;
        glfwTerminate();
        return -1; // Exit if the window creation fails
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImFont *default_font = io.Fonts->AddFontFromFileTTF("../fonts/static/Roboto-Medium.ttf", 16.0f);
    ImFont *midsize_font = io.Fonts->AddFontFromFileTTF("../fonts/static/Roboto-Medium.ttf", 24.0f);
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

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
        int height, width;
        glfwGetWindowSize(window, &width, &height);
        ImGui::SetNextWindowSize(ImVec2(width * 0.8f, height * 0.8f));
        ImGui::PushFont(midsize_font);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Begin("Fatal Error", nullptr, flags);
        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::PushFont(default_font);
        ImVec2 windowSize = (ImGui::GetWindowSize());
        ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, windowSize.y * 0.1f));
        ImGui::Text("%s", error_msg);
        ImGui::PopFont();
        ImGui::End();
        //  Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}