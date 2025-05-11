#ifndef _GUIERRORWINDOW_H
#define _GUIERRORWINDOW_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
class GuiErrorWindow
{
public:
    const char *error_msg = R"(
    There was a fatal error trying to create paths.
    Not sure why this happened. You may need sudo perms to create paths
    
    step 1: open terminal (Ctrl + Alt + T) or sometimes (windows key + T) or go to programs find terminal and click it
    
    step 2: run command ->   sudo   mkdir ~/.local/share/stock_widget (do not forget to . in .local !!)
    
    step 3: run command ->   touch   ~/.local/share/stock_widget/watchlist.dat
    
    step 4: run command ->   mkdir   ~/.local/share/stock_widget/api_info
    
    step 5: run command ->   touch   ~/.local/share/stock_widget/api_info/api_key.enc
    
    step 6: run command ->   touch   ~/.local/share/stock_widget/api_info/key_iv.bin
    
    step 7: say a quick prayer for it to work (optional) LOL
    
    step 8: close the program and try to reopen again
    
    if you have any issues you can email me at thorntonbill343@gmail.com
    )";

private:
    static void
    glfw_error_callback(int error, const char *description);

public:
    int open_fatal_error_window();
};

#endif
