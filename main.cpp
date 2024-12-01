#ifdef __APPLE__
#include <glad/glad.h>
#endif

#ifdef __linux__
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>

#ifdef __linux__
#include <irrKlang.h>
using namespace irrklang;
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader_m.h"
#include "camera.h"

typedef struct {
	glm::vec3 position;
	std::vector<float> vertices;
	GLuint vao;
	bool texture; // se tem textura entao nao tem cor
} Object;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int load_texture(const char* path);
std::vector<float> createPlaneWithTexture(float width, float height, glm::vec3 center);
void glBind_object(Object& obj);
std::vector<float> createQuadVertices(float width, float height, float depth, glm::vec3 center, glm::vec3 color);
std::vector<float> createQuadVertices(float width, float height, float depth, glm::vec3 center);
void mergeVec(std::vector<float> &dest, std::vector<float> v1, std::vector<float> v2);
void mergeVec(std::vector<float> &dest, std::vector<float> v1);
std::vector<float> createCarVertices();
void processCarInput(GLFWwindow *window, glm::vec3 &carPosition, float &carRotationAngle, float deltaTime);
int init_sound_engine();
void play_bonk();
bool isCarWithinRadius(const glm::vec3 &carPosition, float minRadius, float maxRadius);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(-4.97621f, 2.79689f, 7.93694f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(0.0f, -0.1f, 1.0f); // posicao do farol do carro!

// cores
glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 pink = glm::vec3(0.867f, 0.667f, 0.933f);
glm::vec3 green = glm::vec3(0.408f,0.722f,0.004f);
glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 yellow = glm::vec3(1.0f, 0.5f, 0.0f);

float trackVertices[] = {
//   x       y      z     xn    yn    zn    u     v
    -5.0f,  -0.6f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     5.0f,  -0.6f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     5.0f,  -0.6f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  -0.6f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    -5.0f,  -0.6f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  -0.6f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

ISoundEngine* soundEngine;

int main()
{
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Iluminação : Materiais", NULL, NULL);
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

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glew: load all OpenGL function pointers
    // ---------------------------------------
    if(glewInit()!=GLEW_OK) {
        std::cout << "Ocorreu um erro iniciando GLEW!" << std::endl;
        return 1;
    } else {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

	if (init_sound_engine() < 0) return 1;

    float lastFrame = 0.0f;
	glm::vec3 carPosition(2.7f, 0.0f, 1.0f);
	float carRotationAngle = 0.0f;

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("./shaders/materials.vs", "./shaders/materials.fs");
    Shader lightCubeShader("./shaders/light_cube.vs", "./shaders/light_cube.fs");
    Shader lightingTextureShader("./shaders/materials_texture.vs", "./shaders/materials_texture_directional.fs");
    Shader lightingColorShader("./shaders/materials_color.vs", "./shaders/materials_color.fs");

	GLuint track_texture = load_texture("./images/track2.jpg");
    GLuint background_texture = load_texture("./images/crowd-1.jpg");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // first, configure the cube's VAO (and VBO)
    

    /* TRACK START */
    unsigned int trackVBO, trackVAO;
    glGenVertexArrays(1, &trackVAO);
    glGenBuffers(1, &trackVBO);
    glBindBuffer(GL_ARRAY_BUFFER, trackVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(trackVertices), trackVertices, GL_STATIC_DRAW);
    glBindVertexArray(trackVAO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
    /* TRACK END */

    /* LIGHT CUBE START*/
    unsigned int lightCubeVBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &lightCubeVBO);

	std::vector<float> headlights = createQuadVertices(0.9f, 0.05f, 0.1f, lightPos);


    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, headlights.size() * sizeof(float), headlights.data(), GL_STATIC_DRAW);
    glBindVertexArray(lightCubeVAO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    /* LIGHT CUBE END*/

    /* BACKGROUND START */
    unsigned int backgroundVBO, backgroundVAO;
    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);

    Object background;
	background.position = glm::vec3(0.0f, 4.0f, -7.0f);
    background.vertices = createPlaneWithTexture(35.0f, 10.0f, background.position);
    background.vao = backgroundVAO;
    background.texture = true;

    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, background.vertices.size() * sizeof(float), background.vertices.data(), GL_STATIC_DRAW);
    glBindVertexArray(backgroundVAO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
    /* BACKGROUND END */

    /* CAR START*/
    unsigned int carVBO, carVAO;
    glGenVertexArrays(1, &carVAO);
    glGenBuffers(1, &carVBO);

    Object car;
	car.position = glm::vec3(0.0f);
    car.vertices =  createCarVertices();
    car.vao = carVAO;
    car.texture = false;

    glBindBuffer(GL_ARRAY_BUFFER, carVBO);
    glBufferData(GL_ARRAY_BUFFER, car.vertices.size() * sizeof(float), car.vertices.data(), GL_STATIC_DRAW);
    glBindVertexArray(carVAO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
    /* CAR END*/

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
		processCarInput(window, carPosition, carRotationAngle, deltaTime);

        if (!isCarWithinRadius(carPosition, 1.8f, 4.0f)) {
			play_bonk();
			carPosition = glm::vec3(2.7f, 0.0f, 1.0f);
			carRotationAngle = 0.0f;
		}

        // render
        // ------
		glClearColor(0.09f, 0.078f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // light properties
        glm::vec3 lightColor;
        // Luz Branca
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence

        { /* DRAW TRACK, BACKGROUND */
            lightingTextureShader.use();
            glm::vec3 carForward = glm::normalize(glm::vec3(-sin(glm::radians(carRotationAngle)), 0.0f, -cos(glm::radians(carRotationAngle))));

            lightingTextureShader.setVec3("light.direction", carForward);
            lightingTextureShader.setVec3("viewPos", camera.Position);
            lightingTextureShader.setVec3("light.ambient", ambientColor);
            lightingTextureShader.setVec3("light.diffuse", diffuseColor);

            lightingTextureShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
            lightingTextureShader.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
            lightingTextureShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
            lightingTextureShader.setFloat("material.shininess", 32.0f);   
            lightingTextureShader.setVec3("light.specular", 0.25f, 0.25f, 0.25f); // reduce specular light intensity

            model = glm::mat4(1.0f);
            lightingTextureShader.setMat4("model", model);
            lightingTextureShader.setMat4("projection", projection);
            lightingTextureShader.setMat4("view", view);

            // render track
            glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_2D, track_texture);
            glBindVertexArray(trackVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // render background
            glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_2D, background_texture);
            glBindVertexArray(backgroundVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        { /* DRAW CAR */
            lightingColorShader.use();

            lightingColorShader.setVec3("light.position", lightPos);
            lightingColorShader.setVec3("viewPos", camera.Position);
            lightingColorShader.setVec3("light.ambient", ambientColor);
            lightingColorShader.setVec3("light.diffuse", diffuseColor);

            lightingColorShader.setVec3("material.ambient", 0.25f, 0.20725f, 0.20725f);
            lightingColorShader.setVec3("material.diffuse", 1.0f, 0.829f, 0.829f);
            lightingColorShader.setVec3("material.specular", 0.296648f, 0.296648f, 0.296648f);
            lightingColorShader.setFloat("material.shininess", 0.088f * 128.0f);   
            lightingColorShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            glm::vec3 lightOffset = glm::vec3(0.0f, 0.1f, 1.0f);
            glm::vec3 rotatedLightOffset = glm::rotate(glm::mat4(1.0f), glm::radians(carRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightOffset, 0.0f);
            lightPos = carPosition + rotatedLightOffset; // Atualiza a posição da luz para a posição do farol do carro
            lightingColorShader.setVec3("light.position", lightPos);
            // Cria a matriz de modelo do carro
            model = glm::mat4(1.0f);
            model = glm::translate(model, carPosition); // Translação para a posição do carro
            model = glm::rotate(model, glm::radians(carRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotação do carro
            
            
            lightingColorShader.setMat4("model", model);
            lightingColorShader.setMat4("projection", projection);
            lightingColorShader.setMat4("view", view);

            // render car
            glBindVertexArray(carVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6*1024);

            // render light
            // usamos o mesmo model do carro para ficar na mesma posicao
            lightCubeShader.use();
            lightCubeShader.setMat4("model", model);
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);
            glBindVertexArray(lightCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6*6);
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
void processInput(GLFWwindow *window)
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int load_texture(const char* path)
{
    std::cout << "Loading texture: " << path << std::endl;
	// -------------------------
	unsigned int texture1;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
        std::cout << "Failed to load texture: " << path << std::endl;
		exit(1);
	}
	stbi_image_free(data);

	return texture1;
}

void glBind_object(Object& obj)
{
	int components = obj.texture ? 8 : -1;

    if (components == -1) {
        std::cout << "ERROR! ainda nao implementamos um Object com cor!" << std::endl;
        exit(1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, obj.vao);
    glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(float), obj.vertices.data(), GL_STATIC_DRAW);
    glBindVertexArray(obj.vao);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, components * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, components * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

std::vector<float> createPlaneWithTexture(float width, float height, glm::vec3 center)
{
    // NOTE: essa funcao cria um plano com valores normais fixos!
    // basicamente funciona somente para o nosso background
    
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	std::vector<float> vertices = {
    //  x                     y                      z         xn    yn    zn    u     v
		center.x - halfWidth, center.y - halfHeight, center.z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		center.x + halfWidth, center.y - halfHeight, center.z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		center.x + halfWidth, center.y + halfHeight, center.z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		center.x + halfWidth, center.y + halfHeight, center.z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		center.x - halfWidth, center.y + halfHeight, center.z, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		center.x - halfWidth, center.y - halfHeight, center.z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
	};

	return vertices;
}


std::vector<float> createQuadVertices(float width, float height, float depth, glm::vec3 center, glm::vec3 color)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	float halfDepth = depth / 2.0f;

    std::vector<float> vertices = {
        // Front face
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f, color.x, color.y, color.z,

        // Back face
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f, color.x, color.y, color.z,

        // Left face
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, -1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, -1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, -1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, -1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, -1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, -1.0f, 0.0f, 0.0f, color.x, color.y, color.z,

        // Right face
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 1.0f, 0.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 1.0f, 0.0f, 0.0f, color.x, color.y, color.z,

        // Top face
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 1.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 1.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 1.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 1.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 1.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 1.0f, 0.0f, color.x, color.y, color.z,

        // Bottom face
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, -1.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, -1.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, -1.0f, 0.0f, color.x, color.y, color.z,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, -1.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, -1.0f, 0.0f, color.x, color.y, color.z,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, -1.0f, 0.0f, color.x, color.y, color.z,
    };

	return vertices;
}

std::vector<float> createQuadVertices(float width, float height, float depth, glm::vec3 center)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	float halfDepth = depth / 2.0f;

    std::vector<float> vertices = {
        // Front face
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f,
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f,
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, 0.0f, 1.0f,

        // Back face
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f,
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, 0.0f, -1.0f,

        // Left face
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, -1.0f, 0.0f, 0.0f,
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, -1.0f, 0.0f, 0.0f,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, -1.0f, 0.0f, 0.0f,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, -1.0f, 0.0f, 0.0f,
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, -1.0f, 0.0f, 0.0f,
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, -1.0f, 0.0f, 0.0f,

        // Right face
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 1.0f, 0.0f, 0.0f,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 1.0f, 0.0f, 0.0f,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 1.0f, 0.0f, 0.0f,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 1.0f, 0.0f, 0.0f,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 1.0f, 0.0f, 0.0f,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 1.0f, 0.0f, 0.0f,

        // Top face
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 1.0f, 0.0f,
        center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 1.0f, 0.0f,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 1.0f, 0.0f,
        center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 1.0f, 0.0f,
        center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, 0.0f, 1.0f, 0.0f,
        center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, 0.0f, 1.0f, 0.0f,

        // Bottom face
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, -1.0f, 0.0f,
        center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, -1.0f, 0.0f,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, -1.0f, 0.0f,
        center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, -1.0f, 0.0f,
        center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, 0.0f, -1.0f, 0.0f,
        center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, 0.0f, -1.0f, 0.0f,
    };

	return vertices;
}

std::vector<float> createCarVertices()
{
	std::vector<float> bottom = createQuadVertices(1.0f, 0.5f, 2.0f,
												   glm::vec3(0.0f, -0.20f, 0.0f),
												   pink);
    std::vector<float> top = {
        // x     y     z      xn    yn    zn    r      g      b
        -0.3f, 0.0f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.0f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.0f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.0f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.0f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.0f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.451f, 1.0f,

        // top
        -0.3f, 0.3f,  0.5f,  0.0f,  1.0f,  0.0f, 0.867f, 0.667f, 0.933f,
         0.3f, 0.3f,  0.5f,  0.0f,  1.0f,  0.0f, 0.867f, 0.667f, 0.933f,
         0.3f, 0.3f, -0.5f,  0.0f,  1.0f,  0.0f, 0.867f, 0.667f, 0.933f,
         0.3f, 0.3f, -0.5f,  0.0f,  1.0f,  0.0f, 0.867f, 0.667f, 0.933f,
        -0.3f, 0.3f, -0.5f,  0.0f,  1.0f,  0.0f, 0.867f, 0.667f, 0.933f,
        -0.3f, 0.3f,  0.5f,  0.0f,  1.0f,  0.0f, 0.867f, 0.667f, 0.933f,

        // front
        -0.5f, 0.0f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.451f, 1.0f,
         0.5f, 0.0f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.451f, 1.0f,
        -0.5f, 0.0f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.451f, 1.0f,

        // back
        -0.5f, 0.0f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.451f, 1.0f,
         0.5f, 0.0f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.451f, 1.0f,
        -0.5f, 0.0f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.451f, 1.0f,

        // Left window
        -0.3f, 0.0f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.3f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.0f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
        -0.3f, 0.0f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,

        // Right window
         0.3f, 0.0f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.3f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.0f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f,
         0.3f, 0.0f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.451f, 1.0f
    };

	std::vector<float> lights;

	std::vector<float> taillights = createQuadVertices(0.25f, 0.1f, 0.1f,     glm::vec3(-0.2f, -0.1f, -1.0f), red);
	std::vector<float> rightTaillight = createQuadVertices(0.25f, 0.1f, 0.1f, glm::vec3(0.2f,  -0.1f, -1.0f), red);

	std::vector<float> leftBlinker = createQuadVertices(0.1f, 0.1f, 0.1f,     glm::vec3(-0.37f, -0.1f, -1.0f), yellow);
	std::vector<float> rightBlinker = createQuadVertices(0.1f, 0.1f, 0.1f,    glm::vec3(0.37f,  -0.1f, -1.0f), yellow);

	mergeVec(lights, taillights, rightTaillight);
	mergeVec(lights, leftBlinker, rightBlinker);

	std::vector<float> frontLeftWheel = createQuadVertices(0.2f, 0.2f, 0.2f, glm::vec3(-0.5f, -0.4f, 0.8f), black);
	std::vector<float> frontRightWheel = createQuadVertices(0.2f, 0.2f, 0.2f, glm::vec3(0.5f, -0.4f, 0.8f), black);
	std::vector<float> backLeftWheel = createQuadVertices(0.2f, 0.2f, 0.2f, glm::vec3(-0.5f, -0.4f, -0.8f), black);
	std::vector<float> backRightWheel = createQuadVertices(0.2f, 0.2f, 0.2f, glm::vec3(0.5f, -0.4f, -0.8f), black);

	std::vector<float> vertices;
	mergeVec(vertices, lights);
	mergeVec(vertices, bottom, top);
	mergeVec(vertices, frontRightWheel, frontLeftWheel);
	mergeVec(vertices, backRightWheel, backLeftWheel);

	return vertices;
}

void mergeVec(std::vector<float> &dest, std::vector<float> v1, std::vector<float> v2)
{
	dest.insert(dest.end(), v1.begin(), v1.end());
	dest.insert(dest.end(), v2.begin(), v2.end());
}
void mergeVec(std::vector<float> &dest, std::vector<float> v1)
{
	dest.insert(dest.end(), v1.begin(), v1.end());
}

void processCarInput(GLFWwindow *window, glm::vec3 &carPosition, float &carRotationAngle, float deltaTime)
{
    const float carSpeed = 2.0f;        // Velocidade de translação do carro
    const float rotationSpeed = 90.0f; // Velocidade de rotação do carro (em graus por segundo)

    // Calcular o vetor "forward" baseado no ângulo de rotação
    glm::vec3 forward = glm::vec3(sin(glm::radians(carRotationAngle)), 0.0f, cos(glm::radians(carRotationAngle)));

    // Rotação do carro
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        carRotationAngle += rotationSpeed * deltaTime; // Rotaciona para a esquerda
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        carRotationAngle -= rotationSpeed * deltaTime; // Rotaciona para a direita

    // Movimento do carro em relação à direção atual
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        carPosition += forward * carSpeed * deltaTime; // Move para frente
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        carPosition -= forward * carSpeed * deltaTime; // Move para trás

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        // imprima as coordenadas da camera e do carro
        camera.print_coordenates();
    }
}

bool isCarWithinRadius(const glm::vec3 &carPosition, float minRadius, float maxRadius) {
    float distance = glm::length(carPosition);
    return distance <= maxRadius && distance >= minRadius;
}


int init_sound_engine()
{
#ifdef __APPLE__
	return 1; // sem som no mac :/
#endif

	soundEngine = createIrrKlangDevice();
	if (!soundEngine) {
		printf("Erro ao inicializar a engine de SOM!!\n");
		return -1;
	}
	const char* musicFile = "sounds/topgear-soundtrack1.ogg";
	ISound* music = soundEngine->play2D(musicFile,
		true, false, true, ESM_AUTO_DETECT, true);

	if (!musicFile) return -1;
	return 1;
}

void play_bonk()
{
#ifdef __APPLE__
	return 1; // sem som no mac :/
#endif
	soundEngine->play2D("sounds/bonk.wav");
}