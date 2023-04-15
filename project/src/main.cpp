#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <cmath>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

unsigned int loadTexture(char const *path);

void renderPlane(unsigned int planeVAO, unsigned int planeVBO);

void renderPath(unsigned int pathVAO, unsigned int pathVBO);

unsigned int loadCubemap(vector<std::string> faces);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void DrawImGui();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


struct PointLight {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight{
    glm:: vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    Camera camera = Camera();
    bool CameraMouseMovementUpdateEnabled = true;
    PointLight pointLight;
    DirectionalLight dirLight;
    bool day = true;
    bool ImGuiEnabled = false;
};

ProgramState *programState;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    stbi_set_flip_vertically_on_load(false);

    programState = new ProgramState;
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");



    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    // build and compile shaders
    // -------------------------
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader planeShader("resources/shaders/plane.vs", "resources/shaders/plane.fs");
    Shader houseShader("resources/shaders/house.vs", "resources/shaders/house.fs");
    Shader decorationShader("resources/shaders/decoration.vs", "resources/shaders/decoration.fs");
    Shader pathShader("resources/shaders/plane.vs", "resources/shaders/plane.fs");

    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_BaseColor.jpg").c_str());
    unsigned int normalMap  = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_Normal.jpg").c_str());
    unsigned int heightMap  = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_Height.png").c_str());
    unsigned int specMap  = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_AmbientOcclusion.jpg").c_str());

    planeShader.use();
    planeShader.setInt("diffuseMap", 0);
    planeShader.setInt("normalMap", 1);
    planeShader.setInt("depthMap", 2);
    planeShader.setInt("specMap", 3);


    unsigned int diffuseMap1 = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_basecolor.jpg").c_str());
    unsigned int normalMap1  = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_normal.jpg").c_str());
    unsigned int heightMap1  = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_height.png").c_str());
    unsigned int specMap1  = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_ambientOcclusion.jpg").c_str());

    pathShader.use();
    pathShader.setInt("diffuseMap", 4);
    pathShader.setInt("normalMap", 5);
    pathShader.setInt("depthMap", 6);
    pathShader.setInt("specMap", 7);

    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/sh_ft.png"),
                    FileSystem::getPath("resources/textures/skybox/sh_bk.png"),
                    FileSystem::getPath("resources/textures/skybox/sh_up.png"),
                    FileSystem::getPath("resources/textures/skybox/sh_dn.png"),
                    FileSystem::getPath("resources/textures/skybox/sh_rt.png"),
                    FileSystem::getPath("resources/textures/skybox/sh_lf.png")
            };

    vector<std::string> faces1
            {
                    FileSystem::getPath("resources/textures/kurt/space_ft.png"),
                    FileSystem::getPath("resources/textures/kurt/space_bk.png"),
                    FileSystem::getPath("resources/textures/kurt/space_up.png"),
                    FileSystem::getPath("resources/textures/kurt/space_dn.png"),
                    FileSystem::getPath("resources/textures/kurt/space_rt.png"),
                    FileSystem::getPath("resources/textures/kurt/space_lf.png")
            };



    unsigned int cubemapTexture = loadCubemap(faces);
    unsigned int cubemapTexture1 = loadCubemap(faces1);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // load models

    Model house("resources/objects/Big_Old_House/Big_Old_House.obj");
    house.SetShaderTextureNamePrefix("material.");

    Model tree_1("resources/objects/Tree 02/Tree.obj");
    tree_1.SetShaderTextureNamePrefix("material.");

    Model phormium1("resources/objects/Phormium_OBJ/Phormium_1.obj");
    phormium1.SetShaderTextureNamePrefix("material.");

    Model phormium2("resources/objects/Phormium_OBJ/Phormium_3.obj");
    phormium2.SetShaderTextureNamePrefix("material.");


    Model lightPole("resources/objects/Light Pole/Light Pole.obj");
    lightPole.SetShaderTextureNamePrefix("material.");

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    programState->dirLight.direction = glm::vec3(-1.0, -1.0, -1.0);

    programState->dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    programState->dirLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    programState->dirLight.specular = glm::vec3(0.3f, 0.3f, 0.3f);


    glm::vec3 pointLightPositions[] = { glm::vec3(-0.15, 0.19, 0.5),
                                      glm::vec3(0.15, 0.19, 0.5)};

    programState->pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    programState->pointLight.diffuse = glm::vec3(0.5, 0.5, 0.5);
    programState->pointLight.specular = glm::vec3(0.2, 0.2, 0.2);

    programState->pointLight.constant = 1.0f;
    programState->pointLight.linear = 0.08f;
    programState->pointLight.quadratic = 0.032f;

    programState->camera.Position = glm::vec3(1.0, 1.0, 1.0);
    float heightScale = 0.01;
    vector<glm::vec3> tree1_positions;
    vector<glm::vec3> tree2_positions;

    float poz = -4.5;
    float p = 0.7;

    for(float i = 0; poz + i < 4.5; i+=p){
        for(float j = 0; poz + j < 4.5; j+=p){

            if (abs(poz + i) < 0.8) {
                if (poz + j < -0.8) {
                        tree1_positions.push_back(glm::vec3(poz + i, 0, poz + j));
                    }
            } else {
                if ( abs(poz + i) > 0.8 || abs(poz + j) > 0.8) {
                    tree1_positions.push_back(glm::vec3(poz + i, 0, poz + j));
                }
            }

        }
    }

    vector<glm::vec3> phormium1_pos;

    vector<glm::vec3> phormium2_pos;
    float ph_pos = 0.3;
    for(float i = 0; ph_pos + i < 4.5; i+=0.2){
        phormium1_pos.push_back(glm::vec3(0.2, 0.0, ph_pos+i));
        phormium2_pos.push_back(glm::vec3(-0.2, 0.0, ph_pos+i));
    }

    unsigned int planeVAO = 0;
    unsigned int planeVBO = 0;

    unsigned int pathVAO = 0;
    unsigned int pathVBO = 0;

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.3,0.3,0.3, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //house
        houseShader.use();
        houseShader.setBool("dan", programState->day);
        houseShader.setVec3("dirLight.direction", programState->dirLight.direction);
        houseShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        houseShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        houseShader.setVec3("dirLight.specular", programState->dirLight.specular);
        houseShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        houseShader.setVec3("pointLights[0].ambient", programState->pointLight.ambient);
        houseShader.setVec3("pointLights[0].diffuse", programState->pointLight.diffuse);
        houseShader.setVec3("pointLights[0].specular", programState->pointLight.specular);
        houseShader.setFloat("pointLights[0].constant", programState->pointLight.constant);
        houseShader.setFloat("pointLights[0].linear", programState->pointLight.linear);
        houseShader.setFloat("pointLights[0].quadratic", programState->pointLight.quadratic);
        houseShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        houseShader.setVec3("pointLights[1].ambient", programState->pointLight.ambient);
        houseShader.setVec3("pointLights[1].diffuse", programState->pointLight.diffuse);
        houseShader.setVec3("pointLights[1].specular", programState->pointLight.specular);
        houseShader.setFloat("pointLights[1].constant", programState->pointLight.constant);
        houseShader.setFloat("pointLights[1].linear", programState->pointLight.linear);
        houseShader.setFloat("pointLights[1].quadratic", programState->pointLight.quadratic);

        houseShader.setVec3("viewPos", programState->camera.Position);
        houseShader.setFloat("material.shininess", 32.0f);
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        houseShader.setMat4("projection", projection);
        houseShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f));    // it's a bit too big for our scene, so scale it down
        houseShader.setMat4("model", model);
        house.Draw(houseShader);




        decorationShader.use();
        decorationShader.setBool("dan", programState->day);
        decorationShader.setVec3("dirLight.direction", programState->dirLight.direction);
        decorationShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        decorationShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        decorationShader.setVec3("dirLight.specular", programState->dirLight.specular);
        decorationShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        decorationShader.setVec3("pointLights[0].ambient", programState->pointLight.ambient);
        decorationShader.setVec3("pointLights[0].diffuse", programState->pointLight.diffuse);
        decorationShader.setVec3("pointLights[0].specular", programState->pointLight.specular);
        decorationShader.setFloat("pointLights[0].constant", programState->pointLight.constant);
        decorationShader.setFloat("pointLights[0].linear", programState->pointLight.linear);
        decorationShader.setFloat("pointLights[0].quadratic", programState->pointLight.quadratic);
        decorationShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        decorationShader.setVec3("pointLights[1].ambient", programState->pointLight.ambient);
        decorationShader.setVec3("pointLights[1].diffuse", programState->pointLight.diffuse);
        decorationShader.setVec3("pointLights[1].specular", programState->pointLight.specular);
        decorationShader.setFloat("pointLights[1].constant", programState->pointLight.constant);
        decorationShader.setFloat("pointLights[1].linear", programState->pointLight.linear);
        decorationShader.setFloat("pointLights[1].quadratic", programState->pointLight.quadratic);
        decorationShader.setVec3("viewPosition", programState->camera.Position);
        decorationShader.setFloat("material.shininess", 32.0f);

        //phormium1
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);
        for(int i = 0; i < phormium1_pos.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, phormium1_pos[i]);
            model = glm::scale(model, glm::vec3(0.01f));    // it's a bit too big for our scene, so scale it down
            decorationShader.setMat4("model", model);
            phormium1.Draw(decorationShader);
        }

        //phormium2
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);

        for(int i = 0; i < phormium2_pos.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, phormium2_pos[i]);
            model = glm::scale(model, glm::vec3(0.01f));    // it's a bit too big for our scene, so scale it down
            decorationShader.setMat4("model", model);
            phormium2.Draw(decorationShader);
        }

        //tree2
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);
        for(int i = 0; i < tree1_positions.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, tree1_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));    // it's a bit too big for our scene, so scale it down
            decorationShader.setMat4("model", model);
            tree_1.Draw(decorationShader);
        }


        //Light Pole
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.3, 0.2, 0.5));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(0.02f));    // it's a bit too big for our scene, so scale it down
        decorationShader.setMat4("model", model);
        lightPole.Draw(decorationShader);

        //light pole2
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.3, 0.2, 0.5));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(0.02f));    // it's a bit too big for our scene, so scale it down
        decorationShader.setMat4("model", model);
        lightPole.Draw(decorationShader);
        



        //plane

        planeShader.use();
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        planeShader.setMat4("projection", projection);
        planeShader.setMat4("view", view);
        planeShader.setMat4("model", model);
        planeShader.setVec3("viewPos", programState->camera.Position);

        //
        planeShader.setBool("dan", programState->day);
        planeShader.setVec3("dirLight.direction", programState->dirLight.direction);
        planeShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        planeShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        planeShader.setVec3("dirLight.specular", programState->dirLight.specular);
        planeShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        planeShader.setVec3("pointLights[0].ambient", programState->pointLight.ambient);
        planeShader.setVec3("pointLights[0].diffuse", programState->pointLight.diffuse);
        planeShader.setVec3("pointLights[0].specular", programState->pointLight.specular);
        planeShader.setFloat("pointLights[0].constant", programState->pointLight.constant);
        planeShader.setFloat("pointLights[0].linear", programState->pointLight.linear);
        planeShader.setFloat("pointLights[0].quadratic", programState->pointLight.quadratic);
        planeShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        planeShader.setVec3("pointLights[1].ambient", programState->pointLight.ambient);
        planeShader.setVec3("pointLights[1].diffuse", programState->pointLight.diffuse);
        planeShader.setVec3("pointLights[1].specular", programState->pointLight.specular);
        planeShader.setFloat("pointLights[1].constant", programState->pointLight.constant);
        planeShader.setFloat("pointLights[1].linear", programState->pointLight.linear);
        planeShader.setFloat("pointLights[1].quadratic", programState->pointLight.quadratic);




        //planeShader.setVec3("lightPos", pointLightPositions[1]);
        planeShader.setFloat("heightScale", heightScale);
        planeShader.setFloat("shininess", 32.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, heightMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, specMap);

        renderPlane(planeVAO, planeVBO);

        //path
        pathShader.use();


        pathShader.setBool("dan", programState->day);
        pathShader.setVec3("dirLight.direction", programState->dirLight.direction);
        pathShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        pathShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        pathShader.setVec3("dirLight.specular", programState->dirLight.specular);
        pathShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        pathShader.setVec3("pointLights[0].ambient", programState->pointLight.ambient);
        pathShader.setVec3("pointLights[0].diffuse", programState->pointLight.diffuse);
        pathShader.setVec3("pointLights[0].specular", programState->pointLight.specular);
        pathShader.setFloat("pointLights[0].constant", programState->pointLight.constant);
        pathShader.setFloat("pointLights[0].linear", programState->pointLight.linear);
        pathShader.setFloat("pointLights[0].quadratic", programState->pointLight.quadratic);
        pathShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        pathShader.setVec3("pointLights[1].ambient", programState->pointLight.ambient);
        pathShader.setVec3("pointLights[1].diffuse", programState->pointLight.diffuse);
        pathShader.setVec3("pointLights[1].specular", programState->pointLight.specular);
        pathShader.setFloat("pointLights[1].constant", programState->pointLight.constant);
        pathShader.setFloat("pointLights[1].linear", programState->pointLight.linear);
        pathShader.setFloat("pointLights[1].quadratic", programState->pointLight.quadratic);
        pathShader.setFloat("heightScale", heightScale);
        pathShader.setFloat("shininess", 256.0f);

        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        pathShader.setMat4("projection", projection);
        pathShader.setMat4("view", view);
        pathShader.setMat4("model", model);
        pathShader.setVec3("viewPos", programState->camera.Position);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, diffuseMap1);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, normalMap1);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, heightMap1);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, specMap1);

        renderPath(pathVAO, pathVBO);


        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);

        if(programState->day){
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        }else{
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture1);
        }
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        if (programState->ImGuiEnabled)
            DrawImGui();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(programState->CameraMouseMovementUpdateEnabled) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            programState->CameraMouseMovementUpdateEnabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;


    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if(programState->CameraMouseMovementUpdateEnabled) {
        programState->camera.ProcessMouseScroll(yoffset);
    }
}

void renderPlane(unsigned int planeVAO, unsigned int planeVBO)
{
    if (planeVAO == 0)
    {

        glm::vec3 pos1(-5.0f,  5.0f, 0.0f);
        glm::vec3 pos2(-5.0f, -5.0f, 0.0f);
        glm::vec3 pos3( 5.0f, -5.0f, 0.0f);
        glm::vec3 pos4( 5.0f,  5.0f, 0.0f);

        //        texture coordinates
        glm::vec2 uv1(0.0f, 50.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(50.0f, 0.0f);
        glm::vec2 uv4(50.0f, 50.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 10.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 10.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
void renderPath(unsigned int pathVAO, unsigned int pathVBO)
{
    if (pathVAO == 0)
    {

        glm::vec3 pos1(-0.1f,  0.1f, 0.001f);
        glm::vec3 pos2(-0.1f, -5.0f, 0.001f);
        glm::vec3 pos3( 0.1f, -5.0f, 0.001f);
        glm::vec3 pos4( 0.1f,  0.1f, 0.001f);

        //        texture coordinates
        glm::vec2 uv1(0.0f, 40.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(2.0f, 0.0f);
        glm::vec2 uv4(2.0f, 40.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 10.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 10.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        glGenVertexArrays(1, &pathVAO);
        glGenBuffers(1, &pathVBO);
        glBindVertexArray(pathVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pathVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(pathVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;


        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void DrawImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Lights window");
        ImGui::Checkbox("Day", &programState->day);
        if(programState->day){
            ImGui::Text("Directional light");
            ImGui::DragFloat3("Direction", (float *)&programState->dirLight.direction, 0.05, -1.0, 0.0 );
            ImGui::ColorEdit3("ambient", (float *)&programState->dirLight.ambient);
            ImGui::ColorEdit3("diffuse", (float *)&programState->dirLight.diffuse);
            ImGui::ColorEdit3("specular", (float *)&programState->dirLight.specular);

            ImGui::End();

        }else {
            ImGui::Text("Point lights");
            ImGui::ColorEdit3("ambient", (float *)&programState->pointLight.ambient);
            ImGui::ColorEdit3("diffuse", (float *)&programState->pointLight.diffuse);
            ImGui::ColorEdit3("specular", (float *)&programState->pointLight.specular);
            ImGui::DragFloat("constant", &programState->pointLight.constant, 0.05, 0.0, 10.0);
            ImGui::DragFloat("linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
            ImGui::DragFloat("quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);

            ImGui::End();
        }
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
