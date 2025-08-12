cmake_minimum_required(VERSION 3.16)

project(Helianthus VERSION 0.0.12 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts SerialPort WebEngineWidgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts SerialPort WebEngineWidgets Network)

set(PROJECT_SOURCES
        src/global/g_define.h
        src/components/MyLog/mylog.h
        src/components/MyLog/mylog.cpp
        src/components/MySetting/mysetting.h
        src/components/MySetting/mysetting.cpp
        src/components/MyChartView/mychartview.h
        src/components/MyChartView/mychartview.cpp
        src/components/MyProcess/myprocess.h
        src/components/MyProcess/myprocess.cpp
        src/components/MyHttp/myhttp.h
        src/components/MyHttp/myhttp.cpp
        src/main/main.cpp
        src/main/mainwindow.cpp
        src/main/mainwindow.h
        src/main/mainwindow.ui
        src/main/FlowPanel/flowpanel.cpp
        src/main/FlowPanel/flowpanel.h
        src/main/FlowPanel/flowpanel.ui
        src/FormSerial/formserial.h
        src/FormSerial/formserial.cpp
        src/FormSerial/formserial.ui
        src/FormResult/formresult.h
        src/FormResult/formresult.cpp
        src/FormResult/formresult.ui
        src/FormPlot/formplot.h
        src/FormPlot/formplot.cpp
        src/FormPlot/formplot.ui
        src/FormHistory/formhistory.h
        src/FormHistory/formhistory.cpp
        src/FormHistory/formhistory.ui
        src/FormSetting/formsetting.h
        src/FormSetting/formsetting.cpp
        src/FormSetting/formsetting.ui
        src/FormSetting/AutoUpdate/autoupdate.h
        src/FormSetting/AutoUpdate/autoupdate.cpp
        src/FormSetting/AutoUpdate/autoupdate.ui
        src/FormSetting/SerialFrame/serialframe.h
        src/FormSetting/SerialFrame/serialframe.cpp
        src/FormSetting/SerialFrame/serialframe.ui
        src/FormPlot/PlotData/plotdata.h
        src/FormPlot/PlotData/plotdata.cpp
        src/FormPlot/PlotFilter/plotfilter.h
        src/FormPlot/PlotFilter/plotfilter.cpp
        src/FormPlot/PlotFilter/plotfilter.ui
        src/main/ThreadWorker/threadworker.h
        src/main/ThreadWorker/threadworker.cpp
        src/FormPlot/PlotData/plotdata.h
        src/FormPlot/PlotData/plotdata.cpp
        src/FormPlot/PlotData/plotdata.ui
        src/FormPlot/PlotHistory/plothistory.h
        src/FormPlot/PlotHistory/plothistory.cpp
        src/FormPlot/PlotHistory/plothistory.ui
        src/FormSerial/SerialData/serialdata.h
        src/FormSerial/SerialData/serialdata.cpp
        src/FormSerial/SerialData/serialdata.ui
        src/FormOTO/formoto.h
        src/FormOTO/formoto.cpp
        src/FormOTO/formoto.ui
        src/tools/Integration/integration.h
        src/tools/Integration/integration.cpp
        src/tools/Integration/integration.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Helianthus
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        res.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Helianthus APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Helianthus SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Helianthus
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src/global"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/components/MyChartView"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/components/MyLog"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/components/MySetting"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/components/MyProcess"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/components/MyHttp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/Integration"
)

target_link_libraries(Helianthus PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Charts
  Qt${QT_VERSION_MAJOR}::SerialPort
  Qt${QT_VERSION_MAJOR}::WebEngineWidgets
  Qt${QT_VERSION_MAJOR}::Network
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Helianthus)
endif()
set_target_properties(Helianthus PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Helianthus
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Helianthus)
endif()

add_subdirectory(vendor/spdlog-1.15.0/)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
    set(APP_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/Helianthus.ico")

    set(APP_ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    file(WRITE ${APP_ICON_RC} "IDI_ICON1 ICON \"${APP_ICON_PATH}\"\n")

    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON_RC})
endif()
