cmake_minimum_required(VERSION 3.14) # FetchContent wymaga min 3.14

project(MojProjekt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SEKCJA 1: Pobieranie Google Test ---
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# To zapobiega nadpisywaniu ustawień kompilatora przez gtest (szczególnie na Windows)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
# ----------------------------------------

include_directories(include)

# Lista plików z Twoją logiką
set(PLIKI_ZRODLOWE
    src/helpers.cpp
    src/nodes.cpp
    src/package.cpp
    src/storage_types.cpp
)

# Aplikacja Główna
add_executable(Aplikacja src/main.cpp ${PLIKI_ZRODLOWE})

# --- SEKCJA 2: Konfiguracja Testów ---
enable_testing()

# Tworzymy plik wykonywalny testów
add_executable(Testy tests.cpp ${PLIKI_ZRODLOWE})

# Łączymy go z bibliotekami gtest
# gtest_main -> dostarcza gotową funkcję main(), więc nie musisz jej pisać w tests.cpp!
# gtest -> sama biblioteka asercji
target_link_libraries(Testy PRIVATE gtest_main gtest)

# Dzięki temu VS Code wykryje testy w zakładce "Testing"
include(GoogleTest)
gtest_discover_tests(Testy)