# Světla

## Vytváření světel

V Enginu jsou k momentálně dispozici 3 druhy světel:

- PointLight
- DirectionalLight
- SpotLight

Každý z nich se přidává stejně jako jakákoliv entita, pomocí ge.add()

## Struktura data v UBO

Data o rozpoložení světel Engine předává ve formě uniform buffer objectu.

Není třeba vědět, co to přesně je, ale prakticky to znamená toto:

Pokud do vašeho **Fragment shaderu** zkopírujete tento kód, tak po kompilaci, Engine automaticky napojí **UBO** na uniform **LIGHTS** a vám dodá data do shaderu.

```glsl
/* <GRAPHIC ENGINE TEMPLATE CODE> */
struct PointLight{
    vec4 light_data;
    vec3 position;
};

struct DirectionalLight{
    vec4 light_data;
    vec3 direction;
};

struct SpotLight{
    vec4 light_data;
    float cut_off;
    vec3 position;
    vec3 direction;
};

layout (std140) uniform LIGHTS
{
    vec3 BASE_AMBINET_LIGHT;
    PointLight point_lights[NR_POINT_LIGHTS];
    DirectionalLight directional_lights[NR_DIRECTIONAL_LIGHTS];
    SpotLight spot_lights[NR_SPOT_LIGHTS];
};
/* </GRAPHIC ENGINE TEMPLATE CODE> */
```

Pak stačí jen využívat proměné **BASE_AMBINET_LIGHT**, **point_lights**, **directional_lights**, **spot_lights**, pro tvorbu vlastních světelných efektů.

To může vypadat následovně:

```glsl
vec3 lighting(){
    vec3 norm = normalize(NORMAL);
    vec3 out_light = vec3(0.0f);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        vec4 light_data = point_lights[i].light_data;
        vec3 dir_to_light = normalize(point_lights[i].position - FRAG_GLOBAL_POS);
        /.../
        out_light += ...
    }

    for(int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        /.../
    }

    for (int i = 0; i < NR_SPOT_LIGHTS; i++){
        /.../
    }

    return out_light;
}
```

Projedou se všechny světla v cyklu. Jak již bylo [zdokumentováno](https://github.com/bagons/graphicengine/blob/main/docs/dokumentace.pdf), do shaderu se automaticky definují **NR_POINT_LIGHTS**, **NR_DIRECTIONAL_LIGHTS**, **NR_SPOT_LIGHTS**, který určují maximální počet jednotlivých světel, aby kompilátor shader věděl jak dlouhý bude cyklus, aby ho optimalizoval.

Znamená to ale, že se projíždí, přes neexistující světla, které ale mají data vynulované.

@note
Engine zaručuje, že v datech jsou nejprv existující světla, poté ta neexistující, vynulovaná.

@note
Můžete nahlédnou to shader graphicengine/res/shaders/obj_phong.glsl