# Entity

Jak vytvářet, používat a mazat entity.

## Vytváření entit

Entity se vytváří pomocí Engine.add metody. Je to template funkce a jako template parametr se bere classa entity, kterou chcete vytvořit.

Například:

```cpp
auto mesh_thing = ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material((Shaders::VERTEX_UV_NORMAL)));
```

Jak vidíte, funkci jsme dodali parametry. Tyto parametry se přesunou do konstruktoru MeshThing classy.

Je to podobný system jako chytré pointery v C++.

```cpp
auto objekt = std::make_shared<Object>(a, b, c);
```

Engine.add nám vrátí geRef<T> objekt, který slouží jako reference přes, kterou se entita ovládá.

Po vytvoření entity se automaticky přidá do update cyklu a render cyklu, pokud je rendrovatelná.

@note
Entita jde přidat aniž by jste si uložili referenci, ale berte ohled na to, že ji nebudete moc smazat, až na konci programu.

## Používání entit

Entity se používají pomocí operátoru **->** na geRef objektu.

Například:

```cpp
auto mesh_thing = ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material((Shaders::VERTEX_UV_NORMAL)));
mesh_thing->transform.scale = Scale{2.0f, 2.0f, 2.0f};
```

## Mazání entit

Entity se mažou pomocí geRef.free() nebo geRef.queue_free() metody.

Zatímco **free()** smaže entitu rovnou, **queue_free()** přidá entitu do fronty na smazaní a je smazána na konci ge.update() cyklu.

Kdybyste spustili **free()** v update() funkci nějaké entity, dostanete segfault, protože rozhodíte iterátor skrz kontejner na entity.

Update metodách entit použijte vždy **queue_free()** a všude jinde použijte **free()**.

Příklad správného využití Engine.queue_free():

```cpp
class CustomEntity : public Thing{
    geRef<MeshThing> mesh_thing;
    CustomEntity(geRef<MeshThing> mt){
        mesh_thing = mt;
    }

    void update() override {
        if (ge.input.just_pressed(SMAZ)) {
            // mesh_thing.free(); toto je špatně ❌
            mesh_thing.queue_free(); // takhle se maže správně✅
        }
    }
}
```

Tady je příklad správných využítí geRef.free();

```cpp
#include <iostream>
#include "graphicengine.hpp"

Engine ge{"mazani", 500, 500};

enum Actions{
    SMAZ
}

int main(){
    ge.input.set_action_list(std::vector{
        GLFW_KEY_SPACE
    });
    
    auto mesh_thing_1 = ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material((Shaders::VERTEX_UV_NORMAL)));
    auto mesh_thing_2 = ge.add<MeshThing>(ge.meshes.get_cube(), ge.shaders.get_base_material((Shaders::VERTEX_UV_NORMAL)));

    mesh_thing_1.free(); // toto je správně ✅

    while(ge.is_running()){
        ge.pool_inputs();
        if (ge.input.just_pressed(SMAZ)){
            mesh_thing_2.free(); // toto taky správně ✅ (mimo ge.update())
        }

        ge.update();
        ge.send_to_window();
    }
    return 0;
}
```

## Custom entity

Lze vytvářet i vlastní entity, takže nejste omezeni pouze na entity Enginu.

Vytváří se jednoduše.

Stačí si vybrat nějakou Engine entitu ze které chcete dědit a vytvoříte novou classu.

A pak jen přepíšete metody, které potřebujete, většinou Thing.update(), abyste přidali vlastní chování entity.

```cpp
class Enemy : public MeshThing{
    float rychlost_vzletu = 3.0f;
    Enemy(float rychlost){
        rychlost_vzletu = rychlost;

    }

    void update() override{
        transform.position.y += ge.frame_delta() * rychlost_vzletu;
    }
}

int main(){
    auto enemy = ge.add<Enemy>(12.0f);
}
```

Přepisovat Thing.render() lze, ovšem nedoporučuji, protože akorát ztratíte možnost vykreslovat a budete si ji muset napsat sami.