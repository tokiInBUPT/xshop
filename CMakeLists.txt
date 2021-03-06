cmake_minimum_required(VERSION 3.5)

project(xshop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Sql Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Sql Network REQUIRED)

set(PROJECT_SOURCES
        main.cpp

        UI/CLI/print.cpp
        UI/CLI/utils.cpp
        UI/CLI/user.cpp
        UI/CLI/cli.cpp
        UI/CLI/cli.h

        UI/Server/Server.cpp UI/Server/Server.h
        UI/Server/Handler.cpp UI/Server/Handler.h

        Common/user.h Common/user.cpp
        Common/userconsumer.h Common/userconsumer.cpp
        Common/usermerchant.h Common/usermerchant.cpp

        Common/product.h Common/product.cpp
        Common/Product/productbook.h Common/Product/productbook.cpp
        Common/Product/productfood.h Common/Product/productfood.cpp
        Common/Product/productcloth.h Common/Product/productcloth.cpp

        Common/Protocol/http.cpp Common/Protocol/http.h

        Data/DataManager.h
        
        Data/Common/usermanager.h
        Data/Common/productmanager.h
        Data/Common/ordermanager.h

        Data/Backend/usermanager.h Data/Backend/usermanager.cpp
        Data/Backend/productmanager.h Data/Backend/productmanager.cpp
        Data/Backend/ordermanager.h Data/Backend/ordermanager.cpp

        Data/Client/httpclient.h Data/Client/httpclient.cpp
        Data/Client/usermanager.h Data/Client/usermanager.cpp
        Data/Client/productmanager.h Data/Client/productmanager.cpp
        Data/Client/ordermanager.h Data/Client/ordermanager.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(xshop
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(xshop SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(xshop
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(xshop PRIVATE ws2_32 Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(xshop PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(xshop)
endif()
