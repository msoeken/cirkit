cmake_minimum_required(VERSION 2.8.11)
project(revkit_gui)

add_definitions(-std=c++11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex filesystem graph program_options system)

include_directories( ../src )
link_directories( ../build/src )

set(revkit_viewer_SRCS
    src/circuit_view.cpp
    src/truth_table_widget.cpp
    programs/viewer/viewer_window.cpp
    programs/viewer/main.cpp)

add_executable(revkit_viewer ${revkit_viewer_SRCS})
target_link_libraries(revkit_viewer Qt5::Gui Qt5::Widgets cirkit_reversible_static cirkit_classical_static cirkit_core_static ${Boost_LIBRARIES})

set(revkit_gui_FORMS programs/gui/main_widget.ui)
qt5_wrap_ui(revkit_gui_FORMS_HEADERS ${revkit_gui_FORMS})

set(revkit_gui_SRCS
    programs/gui/core/button_bar.cpp
    programs/gui/core/flow_container.cpp
    programs/gui/core/flow_item_tree_widget.cpp
    programs/gui/core/flow_scene.cpp
    programs/gui/core/flow_widget.cpp
    programs/gui/core/main_widget.cpp
    programs/gui/core/messages_widget.cpp
    programs/gui/core/port_item.cpp
    programs/gui/main_window.cpp
    programs/gui/main.cpp)

add_executable(revkit_gui ${revkit_gui_SRCS} ${revkit_gui_FORMS_HEADERS})
target_link_libraries(revkit_gui Qt5::Gui Qt5::Widgets cirkit_reversible_static cirkit_classical_static cirkit_core_static ${Boost_LIBRARIES})
