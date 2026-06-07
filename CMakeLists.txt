cmake_minimum_required(VERSION 3.16)

project(HyprDisplay
    VERSION     0.1.0
    DESCRIPTION "Hyprland display settings manager"
    LANGUAGES   CXX
)

# Delegate everything else to src/CMakeLists.txt
add_subdirectory(src)
