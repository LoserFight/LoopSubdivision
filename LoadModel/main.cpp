#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//pos and trans
float pos[3] = {0};
float rotation[3] = { 0 };

//loop subdivision
int loop = 0;
int lastLoop = 0;
bool Tosave = false;
int Toswitch = 0;
bool changed = true;

int main(int argc, const char** argv){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "J.D.", NULL, NULL);
    if (window == NULL)
    {
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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    //glm::vec3 pointLightPositions[] = {
    //     glm::vec3(0.7f,  0.2f,  2.0f),
    //    glm::vec3(2.3f, -3.3f, -10.0f)
    //};


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);




    // build and compile shaders
    // -------------------------
    Shader ourShader("1.model_loading.vs", "white.fs");

    // load models
    // -----------
    string pa = "naca/bunny.obj";

    if (argc == 2) {
        pa = std::string(argv[1]);
    }
        Model ourModel(pa);

        //Model ourModel("naca/naca.stl");
 

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();




        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.09f);
        ourShader.setFloat("light.quadratic", 0.032f);

        ourShader.setVec3("dirLight.direction", -0.2f, -0.2f, -1.0f);
        ourShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ourShader.setVec3("dirLight.specular", 0.8f, 0.8f, 0.8f);

        //ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        //ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        //ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        //ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        //ourShader.setFloat("pointLights[0].constant", 1.0f);
        //ourShader.setFloat("pointLights[0].linear", 0.09);
        //ourShader.setFloat("pointLights[0].quadratic", 0.032);

        //ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        //ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        //ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        //ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        //ourShader.setFloat("pointLights[1].constant", 1.0f);
        //ourShader.setFloat("pointLights[1].linear", 0.09);
        //ourShader.setFloat("pointLights[1].quadratic", 0.032);


        ourShader.setVec3("spotLight.position", camera.Position);
        ourShader.setVec3("spotLight.direction", camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("spotLight.constant", 1.0f);
        ourShader.setFloat("spotLight.linear", 0.09f);
        ourShader.setFloat("spotLight.quadratic", 0.032f);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        ourShader.setFloat("material.shininess", 64.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(pos[0], pos[1], pos[2])); // translate it down so it's at the center of the scene
        model = glm::rotate(model, rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        if (lastLoop != loop) {
            if (lastLoop < loop ) {
                if(loop == 1)
                ourModel.loopsubS(loop);
                else {
                    ourModel.loopsub();
                }
            }
            else {
                ourModel.loopsubS(loop);
            }
            if (loop == 0) {
                ourModel.meshes.clear();
                ourModel.meshes.push_back(Model::TriToMesh(ourModel.TMeshOri));

            }
            lastLoop = loop;
        }
        if(!changed){
            if (Toswitch==1) {
                if (loop == 0) {
                    ourModel.meshes.clear();
                    ourModel.meshes.push_back(Model::TriToLine(ourModel.TMeshOri));
                }
                else {
                    ourModel.meshes.clear();
                    ourModel.meshes.push_back(Model::TriToLine(ourModel.subDivisionMeshX));
                }
            }
            else if(Toswitch == 0) {
                if (loop == 0) {
                    ourModel.meshes.clear();
                    ourModel.meshes.push_back(Model::TriToMesh(ourModel.TMeshOri));
                }
                else {
                    ourModel.meshes.clear();
                    ourModel.meshes.push_back(Model::TriToMesh(ourModel.subDivisionMeshX));
                }
            }
            changed = true;
        }
        if (Toswitch==0) {
            ourModel.Draw(ourShader);
        }
        else if(Toswitch == 1) {
            ourModel.DrawLine(ourShader);
        }
        else if (Toswitch == 2) {
            ourModel.DrawPoint(ourShader);
        }
        if (Tosave) {
            Tosave = false;
            ourModel.subDivisionMeshX->save_obj("nacaSub.obj");
        }


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
void processInput(GLFWwindow* window)
{
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
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS|| glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS  || glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            rotation[0]+=0.01f* yoffset;
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            rotation[1] += 0.01f * yoffset;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            rotation[2] += 0.01f * yoffset;
    }
    else
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        loop++;
        if (loop > 5)
            loop = 5;

    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        loop--;
        if (loop < 0)
            loop = 0;

    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        Tosave = true;

    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        Toswitch = (Toswitch+1)%3;
        changed = false;

    }


    

}