#define GLEW_STATIC
#define _USE_MATH_DEFINES

#include <cmath>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "entities/models/CubeModel.h"
#include "rendering/Window.h"
#include "rendering/Camera.h"
#include "glm/ext.hpp"
#include "rendering/shaders/SkyboxShaderProgram.h"
#include "utils/Texture.h"
#include "entities/models/PlaneModel.h"
#include "entities/models/Pyramid.h"
#include "entities/models/RegularPyramid.h"
#include "entities/models/CircularFrustum.h"
#include "entities/models/Cylinder.h"
#include "entities/models/Cone.h"
#include "entities/models/Tetrahedron.h"
#include "entities/models/RegularTetrahedron.h"
#include "entities/models/Sphere.h"
#include "entities/composites/Ufo.h"

#define STB_IMAGE_IMPLEMENTATION

Camera *cameraPtr; //in order to change camera view we need access

bool drop_fan = 0;
float rotationSpeed = 20.0f;
float cameraSpeed = 0.1f;
DirectionalLight *sunlight;
std::vector<PointLight *> groundLights;
std::vector<PointLight *> windmillLights;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) //move camera forward
    {
        cameraPtr->setPosition(glm::vec3(cameraPtr->getPosition() + cameraPtr->getViewDirection() * cameraSpeed));
    }

    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) //move camera backward
    {
        cameraPtr->setPosition(glm::vec3(cameraPtr->getPosition() - cameraPtr->getViewDirection() * cameraSpeed));
    }

    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) //move camera to the right
    {
        cameraPtr->setPosition(cameraPtr->getPosition() + glm::normalize(glm::cross(cameraPtr->getViewDirection(), cameraPtr->getUP())) * cameraSpeed);
    }

    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) //move camera to the left
    {
        cameraPtr->setPosition(cameraPtr->getPosition() - glm::normalize(glm::cross(cameraPtr->getViewDirection(), cameraPtr->getUP())) * cameraSpeed);
    }

    if (key == GLFW_KEY_X && (action == GLFW_REPEAT || action == GLFW_PRESS)) //move camera up
    {
        cameraPtr->setPosition(cameraPtr->getPosition() + cameraPtr->getUP() * cameraSpeed);
    }

    if (key == GLFW_KEY_Z && (action == GLFW_REPEAT || action == GLFW_PRESS)) //move camera down
    {
        cameraPtr->setPosition(cameraPtr->getPosition() - cameraPtr->getUP() * cameraSpeed);
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) //close window
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) //drop fan
        drop_fan = 1;

    if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS)) //speed up fan to the left
        rotationSpeed -= 1;

    if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS)) //speed up fan to the right
        rotationSpeed += 1;

    float lightChangeSpeed = 0.1f;

    if (key == GLFW_KEY_1 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        sunlight->setIntensity(sunlight->getAttributes().globalIntensity + lightChangeSpeed);

    if (key == GLFW_KEY_2 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        sunlight->setIntensity(sunlight->getAttributes().globalIntensity - lightChangeSpeed);

    if (key == GLFW_KEY_3 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        for (auto groundLight : groundLights)
            groundLight->setIntensity(groundLight->getAttributes().globalIntensity + lightChangeSpeed);

    if (key == GLFW_KEY_4 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        for (auto groundLight : groundLights)
            groundLight->setIntensity(groundLight->getAttributes().globalIntensity - lightChangeSpeed);

    if (key == GLFW_KEY_5 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        for (auto windmillLight : windmillLights)
            windmillLight->setIntensity(windmillLight->getAttributes().globalIntensity + lightChangeSpeed);

    if (key == GLFW_KEY_6 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        for (auto windmillLight : windmillLights)
            windmillLight->setIntensity(windmillLight->getAttributes().globalIntensity - lightChangeSpeed);
}

void cursorCallback(GLFWwindow *window, double xPosition, double yPosition)
{
    glm::vec2 newMousePosition = glm::vec2(xPosition, yPosition);
    cameraPtr->updateMouse(newMousePosition);
}

int main()
{
    /*  params  */

    const unsigned int SCR_WIDTH = 600;
    const unsigned int SCR_HEIGHT = 600;
    const std::string TITLE = "Wiatrak";
    const ColorInt BG_COLOR = {15, 15, 15};

    const std::string absorberVertexShaderPath = "res/shaders/absorber.vs";
    const std::string absorberFragmentShaderPath = "res/shaders/absorber.fs";

    const std::string lightVertexShaderPath = "res/shaders/light.vs";
    const std::string lightFragmentShaderPath = "res/shaders/light.fs";

    const std::string depthVertexShaderPath = "res/shaders/depth.vs";
    const std::string depthFragmentShaderPath = "res/shaders/depth.fs";

    const std::string skyboxVertexShaderPath = "res/shaders/skybox.vs";
    const std::string skyboxFragmentShaderPath = "res/shaders/skybox.fs";

    /*  rendering  */

    Window w(SCR_WIDTH, SCR_HEIGHT, TITLE);
    w.makeContextCurrent();
    w.setKeyCallback(keyCallback);
    w.setCursorCallback(cursorCallback);

    Renderer r(BG_COLOR);
    AbsorberShaderProgram asp(absorberVertexShaderPath, absorberFragmentShaderPath);
    LightShaderProgram lsp(lightVertexShaderPath, lightFragmentShaderPath);
    DepthShaderProgram dsp(depthVertexShaderPath, depthFragmentShaderPath);
    SkyboxShaderProgram sbsp(skyboxVertexShaderPath, skyboxFragmentShaderPath);

    /*  textures  */
    std::string skyboxTexturePathPrefix = "res/textures/skybox/hw_nightsky/nightsky";
    Texture skyboxTexture({skyboxTexturePathPrefix + "_ft.tga",
                           skyboxTexturePathPrefix + "_bk.tga",
                           skyboxTexturePathPrefix + "_up.tga",
                           skyboxTexturePathPrefix + "_dn.tga",
                           skyboxTexturePathPrefix + "_rt.tga",
                           skyboxTexturePathPrefix + "_lf.tga"});
					 
    Texture woodTexture("res/textures/wood.bmp");
    Texture groundTexture("res/textures/sand.bmp");

    /*  stuff  */
    Scene s;
    Camera c(SCR_WIDTH, SCR_HEIGHT);
    cameraPtr = &c;

    /*  models  */
    CubeModel smallCube(0.05f, 0, 1, 2);
    CubeModel bigCube(30.0f, 0, 1, 2);
    PlaneModel planeModel(100.0f, 100.0f, 0, 1, 2);

    //materials
    Material groundMaterial(ColorInt(241, 140, 142), ColorFloat(0.5f, 0.5f, 0.5f), 1024.0f);
    Material woodMaterial(ColorInt(120, 120, 120), ColorFloat(0.5f, 0.5f, 0.5f), 32.0f);
    Material skyboxMaterial(ColorInt(255, 255, 255), ColorFloat(0.5f, 0.5f, 0.5f), 0.0f);

    /*  skybox  */
    Skybox skybox(bigCube, skyboxMaterial, skyboxTexture);
    skybox.setPosition({0.0f, 1.0f, 1.0f});

    /*  light attributes  */
    PointLightAttributes groundLightAttributes(ColorInt(255, 0, 0), 0.2f, 0.4f, 0.4f, 1.0f, 0.22f, 0.2f);
    PointLightAttributes windmillLightAttributes(ColorInt(255, 255, 255), 0.2f, 0.4f, 0.4f, 1.0f, 0.22f, 0.2f);
    DirectionalLightAttributes dla({-1.0f, -1.0f, -1.0f}, ColorInt(50, 50, 50), 0.2f, 0.4f, 0.5f);

    /* light */

    DirectionalLight directionalLight(dla);

    sunlight = &directionalLight;

    /* absorbers */

    Absorber plane(planeModel, groundMaterial, groundTexture);
    plane.setPosition({0.0f, 0.0f, 0.0f});

    /* adding to scene */

    s.addAbsorber(plane);

    s.addSkybox(skybox);

    s.setDirectionalLight(directionalLight);

    s.turnOnShadows();

    // FOUNDATION VVV
    //height of the windmill
    float root_h = 0.8f;

    //creating root
    Absorber foundation_root(smallCube, woodMaterial, woodTexture);
    foundation_root.setPosition({0.0f, root_h, 0.0f});
    s.addAbsorber(foundation_root);

    //angle between height and legs (approx)
    double angle_degrees = 6;
    double angle = angle_degrees * 3.141592 / 180;

    //legs info
    float plank_length = (root_h) / cos(angle); //needs to scale with height
    float plank_width = 0.015f;
    float plank_depth = 0.015f;

    //settings for legs
    glm::vec3 leg_rotations[] = {{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}};
    glm::vec3 leg_positions[] = {{-0.7071 * (plank_length)*sin(angle) / 2, root_h / 2, 0.7071 * (plank_length)*sin(angle) / 2},
                                 {-0.7071 * (plank_length)*sin(angle) / 2, root_h / 2, -0.7071 * (plank_length)*sin(angle) / 2},
                                 {0.7071 * (plank_length)*sin(angle) / 2, root_h / 2, -0.7071 * (plank_length)*sin(angle) / 2},
                                 {0.7071 * (plank_length)*sin(angle) / 2, root_h / 2, 0.7071 * (plank_length)*sin(angle) / 2}};

    //creating legs
    CuboidModel leg(plank_width, plank_length, plank_depth, 0, 1, 2);
    Absorber legs(leg, woodMaterial, woodTexture);
    std::vector<Absorber *> legs_v;
    Absorber *obj2[4];
    for (int i = 0; i < 4; i++)
    {
        obj2[i] = new Absorber(leg, woodMaterial, woodTexture);
        legs_v.push_back(obj2[i]);
        obj2[i]->rotate(angle, leg_rotations[i]);
        obj2[i]->rotate(0.785, glm::vec3(0.0f, 1.0f, 0.0f));
        s.addAbsorber(*obj2[i]);
        obj2[i]->setPosition({leg_positions[i]});
        foundation_root.addChild(obj2[i]);
    }
    // FOUNDATION ^^^

    // PADDLES VVV
    int no_paddles = 15;

    //how far from the center the middle point of a paddle should be
    float radius_of_paddles = 0.15f;

    //paddle info
    float pad_length = 0.15f;
    float pad_width = 0.03f;
    float pad_depth = 0.01f;

    //connector between the center and a paddle
    float con_length = radius_of_paddles;
    float con_width = pad_width / 2;
    float con_depth = pad_depth / 2;

    CuboidModel child(pad_width, pad_length, pad_depth, 0, 1, 2);     //paddle
    CuboidModel connector(con_width, con_length, con_depth, 0, 1, 2); //connector
    Cylinder cylinder(0.025f, 0.025f, 100, 0, 1, 20);                  //the center

    //creating the center
    Absorber parent(cylinder, woodMaterial, woodTexture);
    parent.rotate(3.14 / 2, glm::vec3(1.0f, 0.0f, 0.0f));
    parent.setPosition({0.0f, 0.0f, -2.0f});
    s.addAbsorber(parent);

    //creating paddles and connectors in a loop
    std::vector<Absorber *> vec_paddles;
    std::vector<Absorber *> vec_connectors;
    Absorber *obj[100];
    Absorber *con[100];
    for (int i = 0; i < no_paddles; i++)
    {
        obj[i] = new Absorber(child, woodMaterial, woodTexture);
        con[i] = new Absorber(connector, woodMaterial, woodTexture);
        vec_paddles.push_back(obj[i]);
        vec_connectors.push_back(con[i]);
        parent.addChild(obj[i]);
        parent.addChild(con[i]);
    }

    //setting and rotating paddles and connectors
    double i = 0;
    for (auto el : vec_paddles)
    {
        el->setPosition({radius_of_paddles * sin(2 * 3.1416 * (i) / (no_paddles)), radius_of_paddles * cos(2 * 3.1416 * (i) / no_paddles), -2.0f});
        el->rotate(2 * 3.1416 * i / double(no_paddles) + 3.14, {0.0f, 0.0f, -1.0f});
        s.addAbsorber(*el);
        i++;
    }
    for (auto el : vec_connectors)
    {
        el->setPosition({radius_of_paddles * 0.5 * sin(2 * 3.1416 * (i) / (no_paddles)), radius_of_paddles * 0.5 * cos(2 * 3.1416 * (i) / no_paddles), -2.0f});
        el->rotate(2 * 3.1416 * i / double(no_paddles) + 3.14, {0.0f, 0.0f, -1.0f});
        s.addAbsorber(*el);
        i++;
    }

    //the connector between root(where legs join) and the center of the fan
    float base_pad_connector_length = 0.1f;
    CuboidModel base_pad_con(0.01f, 0.01f, base_pad_connector_length, 0, 1, 2);
    Absorber base_pad_con_abs(base_pad_con, woodMaterial, woodTexture);
    s.addAbsorber(base_pad_con_abs);

    //joining connector to root
    base_pad_con_abs.setPosition(foundation_root.getPosition());
    base_pad_con_abs.setPosition(foundation_root.getPosition() + (glm::vec3{0.0f, 0.0f, base_pad_connector_length / 2}));
    foundation_root.addChild(&base_pad_con_abs);

    //joining the center to root
    parent.setPosition(foundation_root.getPosition());
    parent.setPosition(parent.getPosition() + (glm::vec3{0.0f, 0.0f, base_pad_connector_length}));
    foundation_root.addChild(&parent);

    //light in the middle od the fan


    //CubeModel small_light(0.01f, 0, 1, 2);
    Cylinder small_light(0.01f, 0.01f, 30, 0, 1, 2);
    PointLight light4(small_light, windmillLightAttributes);
    light4.rotate(M_PI / 2.0f, {1.0f, 0.0, 0.0f});
    s.addLight(light4);
    light4.setPosition(parent.getPosition());
    light4.translate({0.0f, 0.0f, 0.025 * 0.5f + 0.001f});
    parent.addChild(&light4);

    windmillLights.push_back(&light4);

    // PADDLES ^^^^

    // TAIL VVV
    //the wide tail
    float cm_tail_length = 0.17f;
    CuboidModel cm_tail(cm_tail_length, 0.10f, 0.005f, 0, 1, 2);
    Absorber tail(cm_tail, woodMaterial, woodTexture);
    s.addAbsorber(tail);

    //the thin connector appended to tail
    float con_tail_length = 0.2f;
    CuboidModel cm_tb_con(con_tail_length, 0.01f, 0.005f, 0, 1, 2);
    Absorber tail_base_connector(cm_tb_con, woodMaterial, woodTexture);
    s.addAbsorber(tail_base_connector);

    //the other end of the connector
    CuboidModel cm_tail_end(0.01f, 0.01f, 0.01f, 0, 1, 2);
    Absorber tail_end(cm_tail_end, woodMaterial, woodTexture);
    s.addAbsorber(tail_end);

    //making relations between the objects
    tail_end.translate({-(con_tail_length) / 2, 0.0f, 0.0f});
    tail_end.addChild(&tail_base_connector);
    tail_end.setPosition(tail_end.getPosition() + glm::vec3(-(cm_tail_length + con_tail_length) / 2, 0.0f, 0.0f));
    tail_base_connector.addChild(&tail);
    tail_end.setPosition(foundation_root.getPosition() - glm::vec3(0.0f, 0.0f, 0.0f));

    foundation_root.addChild(&tail_end);
    // TAIL ^^^

    // BARRELS VVV
    float barrel_h = 0.105;
    float barrel_r = 0.025;
    Cylinder barrel_c(barrel_r, barrel_h, 100, 0, 1, 20);

    Absorber barrel1(barrel_c, woodMaterial);
    s.addAbsorber(barrel1);
    barrel1.setPosition({-((plank_length)*sin(angle) * 1.4 + barrel_r * 2), barrel_h / 2, 0.0f});
    foundation_root.addChild(&barrel1);

    Absorber barrel2(barrel_c, woodMaterial);
    s.addAbsorber(barrel2);
    barrel2.setPosition({(plank_length)*sin(angle) * 1.4 + barrel_r * 2, barrel_h / 2, (plank_length)*sin(angle) * 1.6 + barrel_r * 2});
    foundation_root.addChild(&barrel2);

    Absorber barrel3(barrel_c, woodMaterial);
    s.addAbsorber(barrel3);
    barrel3.rotate(3.14 / 2, glm::vec3(1.0f, 0.0f, 0.0f));
    barrel3.setPosition({-((plank_length)*sin(angle) * 1.3 + barrel_r * 2), barrel_h / 2, (plank_length)*sin(angle) * 2 + barrel_r * 2});
    barrel3.translate({0.0f, -barrel_h / 2 + barrel_r, 0.0f});
    foundation_root.addChild(&barrel3);
    // BARRELS ^^^

    //initial positions of objects
    tail_end.rotateRelative(foundation_root.getPosition(), 0.2, glm::vec3(0.0f, 0.0f, 1.0f));
    tail_end.rotateRelative(foundation_root.getPosition(), 3.14, glm::vec3(0.0f, 1.0f, 0.0f));
    foundation_root.setPosition({1.0f, root_h, 0.0f});

    //moving tail paramteres
    int tail_direction = 1;
    float time_until_switch = 1.2;
    float time_counter = time_until_switch;

    int lightsNum = 6;
    float pathRadius = 13.0f;
    float radiusDiff = 2.0f;
    glm::vec3 pathCenter = {foundation_root.getPosition().x, 0.0f, foundation_root.getPosition().z - pathRadius};

    for(int i = 0; i < lightsNum; i++)
    {
        auto* lightLow = new PointLight(smallCube, groundLightAttributes);
        auto* lightHigh = new PointLight(smallCube, groundLightAttributes);
        lightLow->setPosition({pathCenter.x + (pathRadius - radiusDiff * 0.5f) * std::sin((float(i + 1) / (lightsNum + 1)) * M_PI / 2.0f), 0.0f, pathCenter.z + (pathRadius - radiusDiff * 0.5f) * std::cos((float(i + 1) / (lightsNum + 1)) * M_PI / 2.0f)});
        lightHigh->setPosition({pathCenter.x + (pathRadius + radiusDiff * 0.5f) * std::sin((float(i + 1) / (lightsNum + 1)) * M_PI / 2.0f), 0.0f, pathCenter.z + (pathRadius + radiusDiff * 0.5f) * std::cos((float(i + 1) / (lightsNum + 1)) * M_PI / 2.0f)});
        s.addLight(*lightLow);
        s.addLight(*lightHigh);
        groundLights.push_back(lightLow);
        groundLights.push_back(lightHigh);
    }

    Ufo ufo(1.0f, 0, 1, 2);
    s.addCompositeEntity(ufo);

    float ufoRadius = 6.0f;
    float ufoSpeed = 0.5f;
    float ufoHeight = 5.0f;

    //loop
    float deltaTime;
    float lastFrame = 0.0f;
	float fall_speed = 0.0;
    while (!w.shouldClose())
    {
        // time calculations
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float paddles_direction = -1; // controls the directon of moving paddles and of the fallen fan

        //parameters for realistic fall physics
        float fall_acceleration = 0.02;

        if (drop_fan)
        {
            fall_speed += deltaTime * fall_acceleration; //acceleration during fall

            //fall animation
            if (parent.getPosition().y >= radius_of_paddles + pad_length / 2)
			{
				parent.translate({ 0.0f, -fall_speed, 0.0f });
				if (parent.getPosition().y < radius_of_paddles + pad_length / 2)
                {
					parent.setPosition({ parent.getPosition().x, radius_of_paddles + pad_length / 2, parent.getPosition().z });
                    fall_speed *= -0.7;
                }
			}

            //movement sideways during and after fall
            parent.translate(foundation_root.getRotation() * glm::vec3(-0.1 * (radius_of_paddles + pad_length / 2) * rotationSpeed * deltaTime * paddles_direction, 0.0f, 0.0f));
        }

        //rotating tail
        time_counter -= deltaTime;
        if (time_counter < 0)
        {
            time_counter = time_until_switch;
            tail_direction *= -1;
        }
        tail_end.rotateRelative(foundation_root.getPosition(), tail_direction * 20 * deltaTime * 0.03, glm::vec3(0.0f, 1.0f, 0.0f));
        //rotating fans
        parent.rotateLocal(rotationSpeed * deltaTime * 0.1 * paddles_direction, {0.0f, 1.0f, 0.0f});

        ufo.setPosition({foundation_root.getPosition().x + ufoRadius * std::cos(ufoSpeed * currentFrame), ufoHeight, foundation_root.getPosition().z + ufoRadius * std::sin(ufoSpeed * currentFrame)});

        w.draw(r, s, dsp, asp, lsp, sbsp, c);
    }

    for(auto* light : groundLights)
        delete light;

    return 0;
}