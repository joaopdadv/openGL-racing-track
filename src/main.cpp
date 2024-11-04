#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Funções para compilar shaders
GLuint compileShader(const char* vertexSource, const char* fragmentSource);
void checkShaderCompileStatus(GLuint shader);
void checkProgramLinkStatus(GLuint program);
// Processar eventos de teclado
void processInput(GLFWwindow *window, float *x, float *y, float *z);

// Shaders (cada objeto terá um shader próprio)
const char* vertexShaderSource = "#version 330 core\n"
    "layout(location = 0) in vec3 position;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
    "}\n";

// const char* fragmentShaderSource1 = "#version 330 core\n"
//     "out vec4 color;\n"
//     "void main() {\n"
//     "   color = vec4(1.0, 0.0, 0.0, 0.0); // Vermelho\n"
//     "}\n";

// const char* fragmentShaderSource2 = "#version 330 core\n"
//     "out vec4 color;\n"
//     "void main() {\n"
//     "   color = vec4(0.0, 1.0, 0.0, 1.0); // Verde\n"
//     "}\n";

// const char* fragmentShaderSource3 = "#version 330 core\n"
//     "out vec4 color;\n"
//     "void main() {\n"
//     "   color = vec4(0.0, 0.0, 1.0, 1.0); // Azul\n"
//     "}\n";

const char* fragmentShaderSource4 = "#version 330 core
	out vec4 FragColor;
	in vec2 TexCoord;
	// texture samplers
	uniform sampler2D texture1;
	void main()
	{
		FragColor = vec4(texture(texture1, TexCoord).rgb, 1.0);
	}
";


// Shader ourShader("./shaders/vertex.glsl", "./shaders/fragment.glsl");

// Dados dos objetos (cubo, pirâmide, tetraedro)
float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  // fundo
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,  // frente
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,  // lateral esquerda
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,  // lateral direita
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,  // fundo
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,  // topo
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };


float pyramidVertices[] = {
    0.0f,  0.5f,  0.0f,  // Topo
   -0.5f, -0.5f, -0.5f,  // Base
    0.5f, -0.5f, -0.5f,  // Base
    0.5f, -0.5f,  0.5f,  // Base
   -0.5f, -0.5f,  0.5f   // Base
};

float tetrahedronVertices[] = {
    0.0f,  0.5f,  0.0f,
   -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.0f, -0.5f, -0.5f
};

float groundVertices[] = {
   -5.0f, -0.6f,  5.0f, 0.0f, 0.0f,
    5.0f, -0.6f, -5.0f, 1.0f, 1.0f,
    5.0f, -0.6f,  5.0f, 1.0f, 0.0f,
   -5.0f, -0.6f,  5.0f, 0.0f, 0.0f,
   -5.0f, -0.6f, -5.0f, 0.0f, 1.0f,
    5.0f, -0.6f, -5.0f, 1.0f, 1.0f,
};

int main() {

   float x = 0.0f;
   float y = 0.0f;
   float z = -5.0f;

    // Inicializar GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Falha ao inicializar GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Três Objetos com Shaders Diferentes (e o chão)", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Falha ao criar a janela GLFW\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Inicializar GLEW
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		// std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    // Compilar os shaders para cada objeto
    // GLuint shaderProgram1 = compileShader(vertexShaderSource, fragmentShaderSource1);
    // GLuint shaderProgram2 = compileShader(vertexShaderSource, fragmentShaderSource2);
    // GLuint shaderProgram3 = compileShader(vertexShaderSource, fragmentShaderSource3);
    // GLuint shaderProgram4 = compileShader(vertexShaderSource, fragmentShaderSource4);

    // Configurar buffers de vértices para cada objeto
    GLuint VBOs[4], VAOs[4];
    glGenVertexArrays(4, VAOs);
    glGenBuffers(4, VBOs);

    // // Cubo
    // glBindVertexArray(VAOs[0]);
    // glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // // Pirâmide
    // glBindVertexArray(VAOs[1]);
    // glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // // Tetraedro
    // glBindVertexArray(VAOs[2]);
    // glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedronVertices), tetrahedronVertices, GL_STATIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // Solo
    glBindVertexArray(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

	// load and create a texture
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
	unsigned char *data = stbi_load("./images/track2.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture aaaaa" << std::endl;
	}
	stbi_image_free(data);

    // Loop de renderização
    while (!glfwWindowShouldClose(window)) {

        // Atender os eventos
        processInput(window, &x, &y, &z);

        // Limpar a tela
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Renderizar cada objeto com seu shader

        glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        view  = glm::translate(view, glm::vec3(x, y, z));
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Solo
        model = glm::mat4(1.0f);
        glUseProgram(shaderProgram4);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAOs[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // // Cubo
        // model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
        // glUseProgram(shaderProgram1);
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // glBindVertexArray(VAOs[0]);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        // // Pirâmide
        // model = glm::mat4(1.0f);
        // glUseProgram(shaderProgram2);
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // glBindVertexArray(VAOs[1]);
        // glDrawArrays(GL_TRIANGLE_FAN, 0, 5);

        // // Tetraedro
        // model = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f));
        // glUseProgram(shaderProgram3);
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // glBindVertexArray(VAOs[2]);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);



        // Trocar os buffers da janela
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpar recursos
    glDeleteVertexArrays(3, VAOs);
    glDeleteBuffers(3, VBOs);
    glDeleteProgram(shaderProgram1);
    glDeleteProgram(shaderProgram2);
    glDeleteProgram(shaderProgram3);

    glfwTerminate();
    return 0;
}

// Funções auxiliares
GLuint compileShader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileStatus(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileStatus(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLinkStatus(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void checkShaderCompileStatus(GLuint shader) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Erro de compilação de shader: %s\n", infoLog);
    }
}

void checkProgramLinkStatus(GLuint program) {
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Erro ao linkar programa: %s\n", infoLog);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, float *x, float *y, float *z)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        *x+=0.1f;
        printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        *x-=0.1f;
        printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }


    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        *y+=0.1f;
        printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        *y-=0.1f;
        printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        *z+=0.1f;
        printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }


    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        *z-=0.1f;
        printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        *x = 0.0f;
        *y = 0.0f;
        *z = -5.0f;
        printf("RESET\n(%.3f,%.3f,%.3f\n", *x, *y, *z);
    }
}
