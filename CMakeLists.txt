cmake_minimum_required(VERSION 3.22.1)
project(StockWidget)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall)


#GLFW
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)


# ---- Find Third-Party Libraries ----
find_library(CRYPTOPP_LIB cryptopp)
if(NOT CRYPTOPP_LIB)
    message(FATAL_ERROR "Could NOT find Crypto++ library")
endif()

find_library(JSONCPP_LIB jsoncpp)
if(NOT JSONCPP_LIB)
    message(FATAL_ERROR "Could NOT find JSONCPP library")
endif()

find_library(CURL_LIB curl)
if(NOT CURL_LIB)
    message(FATAL_ERROR "Could NOT find CURL library")
endif()


add_subdirectory(libs/glfw)

# === Create the library ===
# This will compile all the class cpp files into one static library
file(GLOB LIB_SOURCES "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")


# === IMGUI ===
file(GLOB ROOT_H libs/imgui/*.h)
file(GLOB ROOT_CPP libs/imgui/*.cpp)
set(IMGUI_ROOT_SOURCES ${ROOT_H} ${ROOT_CPP})

set(IMGUI_ADDITIONAL_SOURCES
	libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp

)

#ADDING IMPLOT
file(GLOB IMPLOT_H libs/implot/*.h)
file(GLOB IMPLOT_CPP libs/implot/*.cpp)
set(IMPLOT_ADDON ${IMPLOT_H} ${IMPLOT_CPP})


set(ALL_IMGUI_SOURCES ${IMGUI_ROOT_SOURCES} ${IMGUI_ADDITIONAL_SOURCES} ${IMPLOT_ADDON})



add_library(stocklib ${LIB_SOURCES} ${ALL_IMGUI_SOURCES})

# Specify where the library looks for headers
target_include_directories(stocklib PUBLIC 
		include
		libs/imgui
		libs/imgui/backends
	 	libs/glfw/include
	 	libs/implot
	 	/usr/include/crypto++
)



# === Create the executable ===

target_link_libraries(stocklib
    PRIVATE
    glfw
    OpenGL::GL
    ${CMAKE_THREAD_LIBS_INIT}
    ${CRYPTOPP_LIB}
    ${JSONCPP_LIB}
    ${CURL_LIB}
)

add_executable(StockWidget src/main.cpp)
target_link_libraries(StockWidget PRIVATE stocklib)

#compile conditions, commented out .deb build, TO RUN FROM COMPILE YOU MUST UNCOMMENT CODE UNDER HERE!!
#target_compile_definitions(StockWidget PRIVATE DEV_MODE)


# Install binary
install(TARGETS StockWidget DESTINATION bin)

# Install desktop entry
install(FILES widget.desktop DESTINATION share/applications)

# Install icon
install(FILES icons/stockwidget.png DESTINATION share/icons/hicolor/256x256/apps)

#Install fonts
install(DIRECTORY fonts/ DESTINATION share/stockwidget/fonts)

