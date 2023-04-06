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



void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

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
}pointLight;

Camera camera = Camera();

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
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
//    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);




    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader planeShader("resources/shaders/plane.vs", "resources/shaders/plane.fs");
    Shader houseShader("resources/shaders/house.vs", "resources/shaders/house.fs");
    Shader treeShader("resources/shaders/tree.vs", "resources/shaders/tree.fs");
    Shader decorationShader("resources/shaders/decoration.vs", "resources/shaders/decoration.fs");
    Shader pathShader("resources/shaders/plane.vs", "resources/shaders/plane.fs");

    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_BaseColor.jpg").c_str());
    unsigned int normalMap  = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_Normal.jpg").c_str());
    unsigned int heightMap  = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_Height.png").c_str());
    unsigned int specMap  = loadTexture(FileSystem::getPath("resources/textures/plane/Grass_005_Roughness.jpg").c_str());

    planeShader.use();
    planeShader.setInt("diffuseMap", 0);
    planeShader.setInt("normalMap", 1);
    planeShader.setInt("depthMap", 2);
    planeShader.setInt("specMap", 3);


    unsigned int diffuseMap1 = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_basecolor.jpg").c_str());
    unsigned int normalMap1  = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_normal.jpg").c_str());
    unsigned int heightMap1  = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_height.png").c_str());
    unsigned int specMap1  = loadTexture(FileSystem::getPath("resources/textures/stone floor/Stylized_Stone_Floor_005_roughness.jpg").c_str());

    pathShader.use();
    pathShader.setInt("diffuseMap", 4);
    pathShader.setInt("normalMap", 5);
    pathShader.setInt("depthMap", 6);
    pathShader.setInt("specMap", 7);
    // load models
    // -----------

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


    pointLight.position = glm::vec3(10.0f, 10.0, 10.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 0.5f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    camera.Position = glm::vec3(1.0, 1.0, 1.0);
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




    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(0.3,0.3,0.3, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        
        //house
        houseShader.use();
        houseShader.setVec3("lightPos", pointLight.position);
        houseShader.setVec3("pointLight.ambient", pointLight.ambient);
        houseShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        houseShader.setVec3("pointLight.specular", pointLight.specular);
        houseShader.setFloat("pointLight.constant", pointLight.constant);
        houseShader.setFloat("pointLight.linear", pointLight.linear);
        houseShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        houseShader.setVec3("viewPos", camera.Position);
        houseShader.setFloat("material.shininess", 8.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        houseShader.setMat4("projection", projection);
        houseShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f));    // it's a bit too big for our scene, so scale it down
        houseShader.setMat4("model", model);
        house.Draw(houseShader);

        //phormium1
        for(int i = 0; i < phormium1_pos.size(); i++) {
            decorationShader.use();
            decorationShader.setVec3("lightPos", pointLight.position);
            decorationShader.setVec3("pointLight.ambient", pointLight.ambient);
            decorationShader.setVec3("pointLight.diffuse", pointLight.diffuse);
            decorationShader.setVec3("pointLight.specular", pointLight.specular);
            decorationShader.setFloat("pointLight.constant", pointLight.constant);
            decorationShader.setFloat("pointLight.linear", pointLight.linear);
            decorationShader.setFloat("pointLight.quadratic", pointLight.quadratic);
            decorationShader.setVec3("viewPos", camera.Position);
            decorationShader.setFloat("material.shininess", 32.0f);
            // view/projection transformations
            projection = glm::perspective(glm::radians(camera.Zoom),
                                          (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
            view = camera.GetViewMatrix();
            decorationShader.setMat4("projection", projection);
            decorationShader.setMat4("view", view);

            // render the loaded model
            model = glm::mat4(1.0f);
            model = glm::translate(model, phormium1_pos[i]);
            model = glm::scale(model, glm::vec3(0.01f));    // it's a bit too big for our scene, so scale it down
            decorationShader.setMat4("model", model);
            phormium1.Draw(decorationShader);
        }

        //phormium2
        for(int i = 0; i < phormium2_pos.size(); i++) {
            decorationShader.use();
            decorationShader.setVec3("lightPos", pointLight.position);
            decorationShader.setVec3("pointLight.ambient", pointLight.ambient);
            decorationShader.setVec3("pointLight.diffuse", pointLight.diffuse);
            decorationShader.setVec3("pointLight.specular", pointLight.specular);
            decorationShader.setFloat("pointLight.constant", pointLight.constant);
            decorationShader.setFloat("pointLight.linear", pointLight.linear);
            decorationShader.setFloat("pointLight.quadratic", pointLight.quadratic);
            decorationShader.setVec3("viewPos", camera.Position);
            decorationShader.setFloat("material.shininess", 32.0f);
            // view/projection transformations
            projection = glm::perspective(glm::radians(camera.Zoom),
                                          (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
            view = camera.GetViewMatrix();
            decorationShader.setMat4("projection", projection);
            decorationShader.setMat4("view", view);

            // render the loaded model
            model = glm::mat4(1.0f);
            model = glm::translate(model, phormium2_pos[i]);
            model = glm::scale(model, glm::vec3(0.01f));    // it's a bit too big for our scene, so scale it down
            decorationShader.setMat4("model", model);
            phormium2.Draw(decorationShader);
        }
        //tree2

        for(int i = 0; i < tree1_positions.size(); i++) {
            treeShader.use();
            treeShader.setVec3("pointLight.position", pointLight.position);
            treeShader.setVec3("pointLight.ambient", pointLight.ambient);
            treeShader.setVec3("pointLight.diffuse", pointLight.diffuse);
            treeShader.setVec3("pointLight.specular", pointLight.specular);
            treeShader.setFloat("pointLight.constant", pointLight.constant);
            treeShader.setFloat("pointLight.linear", pointLight.linear);
            treeShader.setFloat("pointLight.quadratic", pointLight.quadratic);
            treeShader.setVec3("viewPosition", camera.Position);
            treeShader.setFloat("material.shininess", 32.0f);
            // view/projection transformations
            projection = glm::perspective(glm::radians(camera.Zoom),
                                          (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
            view = camera.GetViewMatrix();
            treeShader.setMat4("projection", projection);
            treeShader.setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, tree1_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));    // it's a bit too big for our scene, so scale it down
            treeShader.setMat4("model", model);
            tree_1.Draw(treeShader);
        }

        decorationShader.use();
        decorationShader.setVec3("lightPos", pointLight.position);
        decorationShader.setVec3("pointLight.ambient", pointLight.ambient);
        decorationShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        decorationShader.setVec3("pointLight.specular", pointLight.specular);
        decorationShader.setFloat("pointLight.constant", pointLight.constant);
        decorationShader.setFloat("pointLight.linear", pointLight.linear);
        decorationShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        decorationShader.setVec3("viewPos", camera.Position);
        decorationShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.3, 0.2, 0.5));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(0.02f));    // it's a bit too big for our scene, so scale it down
        decorationShader.setMat4("model", model);
        lightPole.Draw(decorationShader);

        decorationShader.use();
        decorationShader.setVec3("lightPos", pointLight.position);
        decorationShader.setVec3("pointLight.ambient", pointLight.ambient);
        decorationShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        decorationShader.setVec3("pointLight.specular", pointLight.specular);
        decorationShader.setFloat("pointLight.constant", pointLight.constant);
        decorationShader.setFloat("pointLight.linear", pointLight.linear);
        decorationShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        decorationShader.setVec3("viewPos", camera.Position);
        decorationShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        decorationShader.setMat4("projection", projection);
        decorationShader.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.3, 0.2, 0.5));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(0.02f));    // it's a bit too big for our scene, so scale it down
        decorationShader.setMat4("model", model);
        lightPole.Draw(decorationShader);

        
        //plane

        planeShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        planeShader.setMat4("projection", projection);
        planeShader.setMat4("view", view);
        planeShader.setMat4("model", model);
        planeShader.setVec3("viewPos", camera.Position);
        planeShader.setVec3("lightPos", pointLight.position);
        planeShader.setFloat("heightScale", heightScale);
        planeShader.setFloat("shininess", 64.0f);

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
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        pathShader.setMat4("projection", projection);
        pathShader.setMat4("view", view);
        pathShader.setMat4("model", model);
        pathShader.setVec3("viewPos", camera.Position);
        pathShader.setVec3("lightPos", pointLight.position);
        pathShader.setFloat("heightScale", heightScale);
        pathShader.setFloat("shininess", 256.0f);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, diffuseMap1);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, normalMap1);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, heightMap1);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, specMap1);

        renderPath(pathVAO, pathVBO);




        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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


    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
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
        // configure plane VAO
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
void renderPath(unsigned int planeVAO, unsigned int planeVBO)
{
    if (planeVAO == 0)
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
        // configure plane VAO
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