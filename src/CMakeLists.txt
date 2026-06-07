# ── Qt toolchain settings ──────────────────────────────────────────────────
# AUTOMOC  : automatically runs moc on headers that contain Q_OBJECT
# AUTOUIC  : automatically runs uic on .ui files (we have none, but harmless)
# AUTORCC  : automatically runs rcc on .qrc resource files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generates build/compile_commands.json — used by VS Code's C++ extension
# (and clangd) to know exactly which flags and include paths each file uses.
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Find Qt ───────────────────────────────────────────────────────────────
# First line: probe whether Qt6 or Qt5 is installed, store the major version.
# Second line: actually import the specific components we need.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

# ── Source files ──────────────────────────────────────────────────────────
# Paths are relative to this file's directory (src/).
set(PROJECT_SOURCES
    main.cpp

    models/Monitor.h

    services/HyprlandService.h
    services/HyprlandService.cpp
    services/LogService.h
    services/LogService.cpp
    services/FileLogService.h
    services/FileLogService.cpp

    viewmodels/DisplayViewModel.h
    viewmodels/DisplayViewModel.cpp

    views/DisplayArrangeWidget.h
    views/DisplayArrangeWidget.cpp
    views/MainWindow.h
    views/MainWindow.cpp
)

# ── Executable target ─────────────────────────────────────────────────────
# qt_add_executable is the Qt6-aware wrapper around add_executable.
# MANUAL_FINALIZATION defers some Qt-specific bookkeeping to qt_finalize_executable
# below — required when other target_* calls must happen in between.
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HyprDisplay MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(HyprDisplay ${PROJECT_SOURCES})
endif()

# ── Include directories ───────────────────────────────────────────────────
# Adding src/ itself as an include root means any header under src/ can be
# included with its path relative to src/, e.g. #include "models/Monitor.h".
target_include_directories(HyprDisplay PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ── Link libraries ────────────────────────────────────────────────────────
# PRIVATE: these libs are only needed to build this target, not by downstream consumers.
# Qt::Core provides QString, QProcess, QJsonDocument, etc.
# Qt::Widgets provides QApplication, QMainWindow, QWidget, etc.
target_link_libraries(HyprDisplay PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

# ── Install rules ─────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS HyprDisplay
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ── Qt6 finalisation ──────────────────────────────────────────────────────
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HyprDisplay)
endif()
