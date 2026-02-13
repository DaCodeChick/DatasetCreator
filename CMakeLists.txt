cmake_minimum_required(VERSION 3.16)
project(DatasetCreator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    Svg
)

# Optional: Apache Arrow for Parquet support
find_package(Arrow QUIET)
if(Arrow_FOUND)
    message(STATUS "Apache Arrow found - Parquet support enabled")
    add_compile_definitions(ENABLE_PARQUET)
else()
    message(STATUS "Apache Arrow not found - Parquet support disabled")
endif()

# Optional: HighFive for HDF5 support
find_package(HighFive QUIET)
if(HighFive_FOUND)
    message(STATUS "HighFive found - HDF5 support enabled")
    add_compile_definitions(ENABLE_HDF5)
else()
    message(STATUS "HighFive not found - HDF5 support disabled")
endif()

# Source files
set(CORE_SOURCES
    src/core/Dataset.cpp
    src/core/DatasetSample.cpp
    src/core/Metadata.cpp
)

set(PLUGIN_SOURCES
    src/plugins/PluginManager.cpp
    src/plugins/readers/TextReader.cpp
    src/plugins/readers/ImageReader.cpp
    src/plugins/readers/AudioReader.cpp
    src/plugins/readers/CSVReader.cpp
    src/plugins/writers/JSONWriter.cpp
    src/plugins/writers/JSONLWriter.cpp
    src/plugins/writers/CSVWriter.cpp
)

set(MANAGER_SOURCES
    src/managers/ImportManager.cpp
    src/managers/ExportManager.cpp
    src/managers/MetadataManager.cpp
    src/managers/ProjectManager.cpp
)

set(GUI_SOURCES
    src/gui/MainWindow.cpp
    src/gui/FileImportDialog.cpp
    src/gui/DatasetView.cpp
    src/gui/DatasetTreeModel.cpp
    src/gui/SamplePreview.cpp
    src/gui/MetadataEditor.cpp
    src/gui/ExportDialog.cpp
    src/gui/SubsetDialog.cpp
    src/gui/SubsetStatsWidget.cpp
    src/gui/AutoSplitDialog.cpp
    src/gui/KFoldDialog.cpp
)

set(UTIL_SOURCES
    src/utils/FileUtils.cpp
)

# Conditionally add Parquet writer
if(Arrow_FOUND)
    list(APPEND PLUGIN_SOURCES src/plugins/writers/ParquetWriter.cpp)
endif()

# Conditionally add HDF5 writer
if(HighFive_FOUND)
    list(APPEND PLUGIN_SOURCES src/plugins/writers/HDF5Writer.cpp)
endif()

# Main application
qt_add_executable(DatasetCreator
    src/main.cpp
    ${CORE_SOURCES}
    ${PLUGIN_SOURCES}
    ${MANAGER_SOURCES}
    ${GUI_SOURCES}
    ${UTIL_SOURCES}
)

target_include_directories(DatasetCreator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(DatasetCreator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Svg
)

if(Arrow_FOUND)
    target_link_libraries(DatasetCreator PRIVATE arrow_shared parquet_shared)
endif()

if(HighFive_FOUND)
    target_link_libraries(DatasetCreator PRIVATE HighFive)
endif()

# Plugin installation directory
set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/plugins")
target_compile_definitions(DatasetCreator PRIVATE 
    PLUGIN_DIR="${PLUGIN_INSTALL_DIR}"
)

# Installation
install(TARGETS DatasetCreator
    RUNTIME DESTINATION bin
)

# Test CLI executable (for command-line testing)
qt_add_executable(test_cli
    test_cli.cpp
    ${CORE_SOURCES}
    ${PLUGIN_SOURCES}
    ${MANAGER_SOURCES}
    ${UTIL_SOURCES}
)

target_include_directories(test_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(test_cli PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Svg
)

if(Arrow_FOUND)
    target_link_libraries(test_cli PRIVATE arrow_shared parquet_shared)
endif()

if(HighFive_FOUND)
    target_link_libraries(test_cli PRIVATE HighFive)
endif()

# Test metadata executable (for testing metadata editing)
qt_add_executable(test_metadata
    test_metadata.cpp
    ${CORE_SOURCES}
    ${PLUGIN_SOURCES}
    ${MANAGER_SOURCES}
    ${UTIL_SOURCES}
)

target_include_directories(test_metadata PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(test_metadata PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Svg
)

if(Arrow_FOUND)
    target_link_libraries(test_metadata PRIVATE arrow_shared parquet_shared)
endif()

if(HighFive_FOUND)
    target_link_libraries(test_metadata PRIVATE HighFive)
endif()

# Test subsets executable (for testing subset management)
qt_add_executable(test_subsets
    test_subsets.cpp
    ${CORE_SOURCES}
    ${PLUGIN_SOURCES}
    ${MANAGER_SOURCES}
    ${UTIL_SOURCES}
)

target_include_directories(test_subsets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(test_subsets PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Svg
)

if(Arrow_FOUND)
    target_link_libraries(test_subsets PRIVATE arrow_shared parquet_shared)
endif()

if(HighFive_FOUND)
    target_link_libraries(test_subsets PRIVATE HighFive)
endif()

# Enable testing
enable_testing()
add_subdirectory(tests)

