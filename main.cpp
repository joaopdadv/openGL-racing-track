#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader_m.h"
#include "camera.h"

#include <iostream>

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

// lighting
glm::vec3 lightPos(0.0f, 1.0f, 0.0f);

float trackVertices[] = {
//   x       y      z     xn    yn    zn    u     v
    -5.0f,  -0.6f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     5.0f,  -0.6f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     5.0f,  -0.6f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  -0.6f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    -5.0f,  -0.6f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  -0.6f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

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
    } else {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("./shaders/materials.vs", "./shaders/materials.fs");
    Shader lightCubeShader("./shaders/light_cube.vs", "./shaders/light_cube.fs");
    Shader lightingTextureShader("./shaders/materials_texture.vs", "./shaders/materials_texture.fs");

	GLuint track_texture = load_texture("./images/track2.jpg");
    GLuint background_texture = load_texture("./images/crowd-1.jpg");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
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

    // glBind_object(background);
    /* BACKGROUND END */

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

        { /* DRAW CUBE (removed)*/
            lightingShader.use();
            lightingShader.setVec3("light.position", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);
            lightingShader.setVec3("light.ambient", ambientColor);
            lightingShader.setVec3("light.diffuse", diffuseColor);

            lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
            lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
            lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
            lightingShader.setFloat("material.shininess", 32.0f);   
            lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            model = glm::mat4(1.0f);
            lightingShader.setMat4("model", model);
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);


            // render the cube
            // glBindVertexArray(cubeVAO);
            // glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        { /* DRAW TRACK, BACKGROUND */
            lightingTextureShader.use();

            lightingTextureShader.setVec3("light.position", lightPos);
            lightingTextureShader.setVec3("viewPos", camera.Position);
            lightingTextureShader.setVec3("light.ambient", ambientColor);
            lightingTextureShader.setVec3("light.diffuse", diffuseColor);

            lightingTextureShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
            lightingTextureShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
            lightingTextureShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
            lightingTextureShader.setFloat("material.shininess", 32.0f);   
            lightingTextureShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

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

        { /* DRAW LAMP */
            lightCubeShader.use();
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
            lightCubeShader.setMat4("model", model);

            glBindVertexArray(lightCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
       
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

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

	// // se tem textura entao nao tem cor
	// if (obj.texture) {
	// 	// texture attribute
    //     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)(6 * sizeof(float)));
    //     glEnableVertexAttribArray(2);
	// } else {
    //     // TODO veriticar se esta correto
	// 	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)(3 * sizeof(float)));
	// 	glEnableVertexAttribArray(1);
	// }
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