#ifndef WIATRAK_SCENE_H
#define WIATRAK_SCENE_H

#include <vector>
#include "../entities/Entity.h"
#include "../entities/absorbers/Absorber.h"

#include "../entities/lights/PointLight.h"
#include "../entities/lights/DirectionalLight.h"
#include "../entities/skybox/Skybox.h"
#include "../entities/CompositeEntity.h"

class Scene
{
    const int MAX_POINT_LIGHTS = 30; // same as in shader, needed for static array in shader

    //containing pointers, because we create and modify entities outside Scene
    std::vector<const Absorber *> absorbers;
    const DirectionalLight *directionalLight = nullptr;
    std::vector<const PointLight *> lights;
    std::vector<const Skybox *> skybox;

    bool shadows = false;

public:
    const std::vector<const Absorber *> &getAbsorbers() const;

    void addAbsorber(const Absorber &absorber);

    const std::vector<const PointLight *> &getLights() const;

    void addLight(const PointLight &light);

    const DirectionalLight *getDirectionalLight() const;

    void setDirectionalLight(const DirectionalLight &light);

    void addCompositeEntity(const CompositeEntity &compositeEntity);

    bool isShadowsTurnedOn() const;

    bool turnOnShadows();

    bool turnOffShadows();

    const std::vector<const Skybox *> &getSkybox() const;

    void addSkybox(const Skybox &skybox);
};

#endif //WIATRAK_SCENE_H
