# Grafický Engine

Jednoduchý Grafický Engine ve formě C++ knihovny na základě OpenGL a GLFW.

Maturitní projekt GJK - školní rok 2025/26

[teoretická práce](https://github.com/bagons/graphicengine)

****

## Instalace

Oklonujte si knihovnu a dejte ji do složky projektu

```bash
mkdir projekt
cd ./projekt
git clone https://github.com/bagons/graphicengine.git
```
Vytvořte CMakeLists.txt a main.cpp

```bash
touch CMakeLists.txt
touch main.cpp
```

Základní program, který otevře okno, dokud není vypnuto

```c++
#include "graphicengine.hpp"
#include <iostream>

// Nastavení enginu
Engine ge{"window name", 500, 500};

int main(){
std::cout << "Project started" << std::endl;

    while (ge.is_running()){
        ge.update();
        ge.send_it_to_window();
    }

    std::cout << "Project ended" << std::endl;

    return 0;
}
```

Základní CMake na kompilaci

```cmake
# CMake Nastavení
cmake_minimum_required(VERSION 3.31)
set(CMAKE_CXX_STANDARD 20)
# Název vašeho projektu
project(demo)

# Připojení knihovny
add_subdirectory(graphicengine)
include_directories(graphicengine/include)
        
# Soubor(y) vašeho projektu
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} graphicengine)

# Překopírování složek zdrojů k sputitelnému souboru
graphicengine_setup(${PROJECT_NAME})
```

```bash
cmake .
cmake --build .
```

Rozsáhlejší návod na instalaci je na [úvodní stránce](https://bagons.github.io/graphicengine/) technické dokumentace, nebo v PDF teoretické práce.

## Zdroje

**Manuál + Technická dokumentace**

https://bagons.github.io/graphicengine/

**Dema najdete zde:**

https://github.com/bagons/ge-demos