cmake_minimum_required(VERSION 3.14)

project(NetSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SEKCJA 1: Pobieranie Google Test ---
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git # Używamy GIT zamiast URL zip
  GIT_TAG v1.14.0  # Konkretna wersja naprawiająca błędy kompilacji
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

# --- SEKCJA 2: Aplikacja Główna ---
add_executable(Aplikacja src/main.cpp ${PLIKI_ZRODLOWE})

target_link_libraries(Aplikacja PRIVATE gtest gtest_main)


# --- SEKCJA 3: Konfiguracja Testów ---
enable_testing()

add_executable(Testy test/tests.cpp ${PLIKI_ZRODLOWE})

# Linkowanie dla pliku z testami
target_link_libraries(Testy PRIVATE gtest gtest_main)

# Integracja z VS Code / CTest
include(GoogleTest)
gtest_discover_tests(Testy)