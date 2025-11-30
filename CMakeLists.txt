cmake_minimum_required(VERSION 3.16)

project(cutrepad VERSION 0.0.1 LANGUAGES CXX)
add_compile_definitions(APP_VERSION="${PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES cutrepad_es_ES.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        languages/cpp.ini
        languages/gemix.ini
        translations/${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # Crea el ejecutable
    qt_add_executable(cutrepad
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}        
        appconfig.h
        appconfig.cpp
        configmanager.h
        configmanager.cpp
    )

    # Copia los archivos de lenguaje (solo cuando se ejecuta CMake)
    file(COPY ${CMAKE_SOURCE_DIR}/languages DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    # Crea los archivos de traducciones .ts y .qm
    qt_add_translations(cutrepad
        SOURCE_TARGETS cutrepad
        TS_FILES translations/${TS_FILES}
    )

    # Muy importante añadir el target update_translations que genera implicitamente qt_add_translations o no actualiza el .ts
    add_dependencies(cutrepad update_translations)
else()
    if(ANDROID)
        add_library(cutrepad SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(cutrepad
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(cutrepad PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.cutrepad)
endif()
set_target_properties(cutrepad PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS cutrepad
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cutrepad)
endif()


