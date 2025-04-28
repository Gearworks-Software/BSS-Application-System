cmake_minimum_required(VERSION 3.19)
project(frontend-desktop LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network MultimediaWidgets)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(frontend-desktop
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    rsrc.qrc
    NetworkManager.cpp
    NetworkManager.h
)

# qt_add_resources(frontend-desktop "resources"
#     PREFIX "/"
#     FILES resources/style/app.qss
# )

qt_add_resources(frontend-desktop "resources"
    PREFIX "/"
    FILES rsrc.qrc
)

target_link_libraries(frontend-desktop
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::MultimediaWidgets
)

include(GNUInstallDirs)

install(TARGETS frontend-desktop
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET frontend-desktop
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
