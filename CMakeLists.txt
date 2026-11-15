cmake_minimum_required(VERSION 3.20)
project(SenderApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ищем OpenCV, установленный через vcpkg
find_package(OpenCV REQUIRED)

# Создаем исполняемый файл
add_executable(sender.exe sender.cpp)

# Привязываем библиотеки OpenCV
target_link_libraries(sender.exe PRIVATE ${OpenCV_LIBS})

# Привязываем библиотеку сокетов Windows
target_link_libraries(sender.exe PRIVATE ws2_32)
