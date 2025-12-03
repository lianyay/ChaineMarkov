cmake_minimum_required(VERSION 3.24)
project(2526_TI301_PJT C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Erreur Compilation
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Fichier Source
set(SOURCES
    src/main.c
    src/utils.c
    src/hasse.c
    src/list.c
    src/tarjan.c
    src/matrix.c
)

# Creation Executable
add_executable(TI_301_PJT ${SOURCES})

# Destination Défini
target_include_directories(TI_301_PJT PRIVATE inc)
