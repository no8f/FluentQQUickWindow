cmake_minimum_required(VERSION 3.16)

project(FluentQQUickWindow VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#add this to your QT Creator Kit konfig
#DQT_QML_GENERATE_QMLLS_INI ON

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appFluentQQUickWindow
    main.cpp
)

qt_add_qml_module(appFluentQQUickWindow
    URI FluentQQUickWindow
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES customquickwindow.h customquickwindow.cpp
        RESOURCES rsc.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appFluentQQUickWindow PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appFluentQQUickWindow
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appFluentQQUickWindow
    PRIVATE
    Qt6::Quick
    Qt6::Widgets
    dwmapi
)

include(GNUInstallDirs)
install(TARGETS appFluentQQUickWindow
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
