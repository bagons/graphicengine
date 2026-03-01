# Spuštění enginu

## Základní prázdný projekt

Vytvořte si složku pro svůj projekt a do ní engnine *oclonujte*.


```bash
mkdir projekt
cd ./projekt
git clone https://github.com/bagons/graphicengine.git
```

Máme knihovnu, super! Nyní je ale potřeba knihovnu využít.

Vtvoříme například: **main.cpp** naší složce projektu.

Jako první přidáme header enginu. Tento jediný vám bude stačit a to je veškerý *including*, který bude potřeba udělat.

```cpp
#include "graphicengine.hpp"
```

Pak stačí vytvořit Engine objekt. Ten má 2 hlavní požadavky. Vytvořte ho pod jménem **ge** a jako **globalní staticky alokovanou** proměnou. Konstruktor bere *název*, *šířku* a *výšku* okna. Čtvrtý volitelný paramter je nastavení enginu, ale o tom později.

```cpp
#include "graphicengine.hpp"

Engine ge{"název okna", 500, 500};
```

Po přidání **main()** funkce lze program zkompilovat.

```cpp
#include "graphicengine.hpp"

Engine ge{"název okna", 500, 500};

int main(){
    return 0;
}

```

Aby se nám program zkompiloval i s enginem lze využít [CMake](https://cmake.org/).
Stačí mít CMake a ve složce projektu vytvořit **CMakeLists.txt**:

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

Tento CMakeLists.txt není nijak zvlaštní, nastaví standardy, připojí se složka **graphicengine** (*která má svůj vlastní CMakeLists.txt*), přidá se **main.cpp** ke kompilci do spustitelného souboru a napojí se knihovna (název přesně *graphicengine*) se napojí na projekt.

Co je ale lehce nestandartní je funkce graphicengine_setup(). Je to funkce nadefinovaná v CMakeLists.txt ve složce graphicsengine a dělá to že **POST_BUILD** daného targetu (${PROJECT_NAME}) to ve výsledné složce překopíruje zdroje enginu. Je to složka *engine* ve které jsou například shadery enginu.

@note
Jedna verze CMaku mi odmítala program zkompilovat, jelikož *${PROJEKT_NAME} target* byl out of scope při předání targetu v parametru funkce **graphicengine_setup(...)**, nicméně pak to prostě přestalo, aniž bych něco změnil. Stalo se mi jen jednou a jindy jsem s tím problém neměl, ale kdyby něco, stačí zkopírovat složku graphicengine/res do (*export složka spustitelného souboru*)/engine/res.
Nebo můžete přidat vlastní příkaz do CMakeLists.txt projektu. Například, který zkopíruje složku za vás.
```
add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/graphicengine/res"
        "${CMAKE_CURRENT_BINARY_DIR}/engine/res"
)
```

Následně stačí program zkompilovat pomocí **CMake**(u), v příkazové řádce spustě.

```bash
cmake .
cmake --build .
```

A je to. Teď stačí jen napsat **main.cpp**, co něco dělá.

```cpp
#include "graphicengine.hpp"
#include <iostream>

Engine ge{"window name", 500, 500};

int main(){
    std::cout << "Program začal" << std::endl;
    
    while (ge.is_running()){
        ge.update();
        ge.send_it_to_window();
    }

    std::cout << "Program skončil" << std::endl;

    return 0;
}
```

*(program který vytvoří prázné okno, které tam i zůstane, dokud není vypnuto)*

Ale jak udělat program, co něco dělá, bude až v návodu vykreslování.

## Pokročilé spouštění

Engine má i nějaké nastavení, kterými můžete upůsovit engine vašim potřebám.

Jednotlivé parametry jsou popsané na stránce EngineSettings, ale vezměme si že chceme například vypnou [Gamma Correction](https://en.wikipedia.org/wiki/Gamma_correction).

Postup je následovný.

Člověk si vytvoří vlastní instanci EngineSettings **struct** u a pomocí volitelných paramterů dokáže upravit právě jenom **gamma_correction** nastavení, aniž by měnil ostatní.

Pak jen stačí přidat váš struct nastavení jako 4. parametr

```cpp
#include "graphicengine.hpp"
#include <iostream>

EngineSettings settings = {.gamma_correction = false};
Engine ge{"window name", 500, 500, settings};

int main(){
    ...
}
```

Samozřejmě parametrů jde nastavic i víc.

```cpp
EngineSettings settings = {.MAX_NR_POINT_LIGHTS = 64, .gamma_correction = false};
Engine ge{"window name", 500, 500, settings};

```