# První projekt

Pro vytvoření prvního projektu je dobré pochopit jak funguje cyklus enginu, který zprostředkuje snímek na obrazovce. (**Update + Render cyklus**) 

Pod **Update cyklem** myslím update entity, zpracováním vstupu - obecně zpracování změn ve světě. A **Render cyklem** myslím **Render Pipeline** (*Vykreslovací řetězec*), který po všech změnách vytvoří snímek.

Záměrem bylo udělat tento cyklus co nejjednodušší na spuštění, ale zároveň přizpůsobitelný.

## Update Cyklus

Základní cyklus ve kterém poběží celá hra se dělá pomocí **while** loop a metody Engine.is_running().

Do něj je důležité vložit ukončovací metodu celého **Update + Render Cyklu** Engine.send_to_window().

```cpp
#include <iostream>
#include "graphicengine.hpp"

Engine ge{"update cyklus", 500, 500};

int main(){
    while(ge.is_running()){
        std::cout << "Spouštím se každý snímek" << std::endl;
        ge.send_to_window();
    }
    return 0;
}
```

Do **while** loopu můžete vkládat kód, který průběžně svět mění. Například, postupná rotace světla.

Pro **Update cyklus** jsou důležitý 2 metody:
- Engine.update();
- Engine.pool_inputs();

```cpp
#include <iostream>
#include "graphicengine.hpp"

Engine ge{"update cyklus", 500, 500};

int main(){
    while(ge.is_running()){
        std::cout << "Spouštím se každý snímek" << std::endl;
        ge.update();
        ge.send_to_window();
    }
    return 0;
}
```

Pomocí Engine.update() spustíte update() metodu na všech entitách, které nejsou Thing.paused. A zpracují se geRef.queue_free() příkazy.

Tuto funkci spouštíte ve všech cyklech, nicméně spouštíte ji ručně, abyste mohli mít kód cyklu u sebe mohli jste si ho upravit dle libosti.

---

Engine.pool_inputs(); zpracuje všechny příchozí uživatelské vstupy, a aby Input fungoval je nutné tuto metodu spustit na začátku každého snímku. Engine.update(), ji volá automaticky (pokud již spuštěna nebyla), takže nemusíte nic řešit.

@note
Pokud chcete v update cyklu měnit něco **před updatem entit**, je potřeba spustit Engine.pool_inputs() ručně.
```cpp
#include <iostream>
#include "graphicengine.hpp"

Engine ge{"update cyklus", 500, 500};

enum Actions{
    PRIDANI_ENTITY
}

int main(){
    ge.input.set_action_list(std::vector{
        GLFW_KEY_SPACE
    });
    while(ge.is_running()){
        // Bez ge.pool_inputs(); to nebude fungovat ❌
        ge.pool_inputs(); // Teď to bude fungovat ✅

        if (ge.input.just_pressed(PRIDANI_ENTITY)){
            std::cout << "Chci něco dělat ještě před updatem entit" << std::endl;
            ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL));
        }

        std::cout << "Spouštím se každý snímek" << std::endl;
        ge.update();
        ge.send_to_window();
    }
    return 0;
}
```
Po Engine.upadate() byl Engine.pool_inputs() určitě zavolán, takže nic řešit nemusíte.
```cpp
/.../
int main(){
    /.../
    while(ge.is_running()){
        std::cout << "Spouštím se každý snímek" << std::endl;
        ge.update();

        // Toto bude fungovat, i bez pool inputs ✅
        if (ge.input.just_pressed(PRIDANI_ENTITY)){
            std::cout << "Chci něco dělat ještě před updatem entit" << std::endl;
            /.../ 
        }
        ge.send_to_window();
    }
    return 0;
}
```

## Render Cyklus

Render cyklus je trochu víc verbose (*česky mnohomluvný*), ale zato je přizpůsobitelnější.

V tomto Enginu vykreslují tzv. RenderPass(y). Vytváří se podobně jako entity. Obsahují vykreslovací funkce, ty musí člověk spouští ručně a uživatel má tedy větší přehled o tom co se vlastně ve **Vykreslovacím řetězci** děje a může řetězec přizpůsobit svým účelům, místo nějakého velkého univerzálního řetězce, který počítá plno věcí, které se ale stejně nevyužijí.

@note
Momentálně existují jen dva RenderPassy, ale jdou rozšířit, a tak přidat efekty jako SSAO, Shadow Mapy, Color-Correction atd.


### Jak vykreslovat na obrazovku?

To je hodně povídání, ale jak tedy spouští vykreslování?

Nejprv člověk musí vytvořit kameru. Pomocí ge.add<Camera>(fov, near_plane, far_plane).

Dále vytvoříme ForwardOpaque3DPass a jemu dodáme kameru.

Nakonec stačí jen zavolat ForwardOpaque3DPass.render(), aby bylo vykresleny entity.

```cpp
#include "graphicengine.hpp"

Engine ge{"render + update cyklus", 500, 500};

int main(){
    // Vytvoříme kameru
    auto camera = ge.add<Camera>(60.0f, 0.1f, 1000.0f); 
    // Vytvoříme ForwardOpaque3DPass
    auto forward_pass = ge.add_render_pass<ForwardOpaque3DPass>(camera);

    while(ge.is_running()){
        ge.update();
        // Vykreslíme
        forwad_pass->render();
        ge.send_to_window();
    }
    return 0;
}
```

To je vše. Teď stačí jen přidat entity, nastavit kameru, aby byly vidět.

### Ostatní RenderPassy

Zatím existuje jen jeden další RenderPass: ColorPass, který jen obrazovku zalije jednou barvou.

```cpp
#include "graphicengine.hpp"

Engine ge{"color pass", 500, 500};

int main(){
    // Vytvoříme kameru
    auto camera = ge.add<Camera>(60.0f, 0.1f, 1000.0f); 
    // Vytvoříme ColorPass
    auto color_pass = ge.add<ColorPass>(Color::GREEN)
    // Vytvoříme ForwardOpaque3DPass
    auto forward_pass = ge.add_render_pass<ForwardOpaque3DPass>(camera);

    while(ge.is_running()){
        ge.update();
        // Vykreslíme
        color_pass->render();
        forwad_pass->render();
        ge.send_to_window();
    }
    return 0;
}
```