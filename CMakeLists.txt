cmake_minimum_required(VERSION 3.16)

project(Adem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(AdemPreview
    main.cpp
    DBConnection.cpp
    DBConnection.h
    User.cpp
    User.h
    LoginWindow.cpp
    LoginWindow.h
    LoginWindow.ui
    MainDashboard.cpp
    MainDashboard.h
    MainDashboard.ui
    GameStatus.cpp
    GameStatus.h
    GameStatus.ui
)

target_link_libraries(AdemPreview PRIVATE Qt6::Widgets Qt6::Sql)
