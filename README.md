# StockWidget

StockWidget is a graphical desktop application FOR LINUX OS ONLY that displays real-time stock market data using OpenGL, ImGui, and GLFW.

## Features

* Live stock quote retrieval via Yahoo Finance API
* Interactive user interface built with ImGui

## Prerequisites to build from source

Make sure you have the following installed on your system to compile and test:


* C++17 compatible compiler
* CMake (version 3.22 or higher)
* Git
* Dependencies:

  * `libcrypto++-dev`
  * `libjsoncpp-dev`
  * `libcurl4-openssl-dev`
  * `libglfw3`
  * `libgl1`
  * `libx11-6`
  * `libxrandr2`
  * `libxi6`
  * `libxcursor1`
  * `libwayland-dev`
  * `libxkbcommon-dev`
  * `xorg-dev`

# Build from Source
git clone https://github.com/YourUserName/StockWidget.git
cd StockWidget
mkdir build && cd build
cmake ..
cmake --build .

# Download and Install
Run:
wget https://github.com/jdubzanon/StockWidget/releases/download/v1.0/stockwidget_1.0-1_amd64.deb


Once downloaded go to file where .deb file is and run:
sudo apt install ./stockwidget_1.0-1_amd64.deb
