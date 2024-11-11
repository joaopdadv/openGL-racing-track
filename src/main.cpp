#ifdef __APPLE__
#include <glad/glad.h>
#endif

#ifdef __linux__
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Shader.h>
#include <iostream>
#include <stdio.h>

GLuint compileShader(const char *vertexSource, const char *fragmentSource);
void checkShaderCompileStatus(GLuint shader);
void checkProgramLinkStatus(GLuint program);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void resetCamera();

const char *vertexShaderSource = "#version 330 core\n"
								 "layout(location = 0) in vec3 position;\n"
								 "layout (location = 1) in vec2 aTexCoord;\n"
								 "out vec2 TexCoord;\n"
								 "uniform mat4 model;\n"
								 "uniform mat4 view;\n"
								 "uniform mat4 projection;\n"
								 "void main() {\n"
								 "   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
								 "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
								 "}\n";

const char *fragmentShaderTrack = "#version 330 core\n"
								  "out vec4 color;\n"
								  "in vec2 TexCoord;\n"
								  "uniform sampler2D texture1;\n"
								  "void main() {\n"
								  "   color = vec4(texture(texture1, TexCoord).rgb, 1.0);\n"
								  "}\n";

const char *fragmentShaderCar = "#version 330 core\n"
								"out vec4 color;\n"
								"void main() {\n"
								"   color = vec4(0.0, 0.0, 1.0, 1.0); // Azul\n"
								"}\n";

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float sensitivity = 1.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float zoom = 45.0f;

float groundVertices[] = {
	-5.0f, -0.6f, 5.0f, 0.0f, 0.0f,
	5.0f, -0.6f, -5.0f, 1.0f, 1.0f,
	5.0f, -0.6f, 5.0f, 1.0f, 0.0f,
	-5.0f, -0.6f, 5.0f, 0.0f, 0.0f,
	-5.0f, -0.6f, -5.0f, 0.0f, 1.0f,
	5.0f, -0.6f, -5.0f, 1.0f, 1.0f};

// Ainda é um cubo
// TODO: modelar carro
// float carVertices[] = {

// };
	float carVertices[] = {
		// Base of the car
		-0.5f, -0.5f, -1.0f,  0.5f, -0.5f, -1.0f,  0.5f, -0.5f, 1.0f,
		0.5f, -0.5f, 1.0f,  -0.5f, -0.5f, 1.0f,  -0.5f, -0.5f, -1.0f,

		// Top of the car
		-0.5f, 0.0f, -1.0f,  0.5f, 0.0f, -1.0f,  0.5f, 0.0f, 1.0f,
		0.5f, 0.0f, 1.0f,  -0.5f, 0.0f, 1.0f,  -0.5f, 0.0f, -1.0f,

		// Front face
		-0.5f, -0.5f, 1.0f,  0.5f, -0.5f, 1.0f,  0.5f, 0.0f, 1.0f,
		0.5f, 0.0f, 1.0f,  -0.5f, 0.0f, 1.0f,  -0.5f, -0.5f, 1.0f,

		// Back face
		-0.5f, -0.5f, -1.0f,  0.5f, -0.5f, -1.0f,  0.5f, 0.0f, -1.0f,
		0.5f, 0.0f, -1.0f,  -0.5f, 0.0f, -1.0f,  -0.5f, -0.5f, -1.0f,

		// Left face
		-0.5f, -0.5f, -1.0f,  -0.5f, -0.5f, 1.0f,  -0.5f, 0.0f, 1.0f,
		-0.5f, 0.0f, 1.0f,  -0.5f, 0.0f, -1.0f,  -0.5f, -0.5f, -1.0f,

		// Right face
		0.5f, -0.5f, -1.0f,  0.5f, -0.5f, 1.0f,  0.5f, 0.0f, 1.0f,
		0.5f, 0.0f, 1.0f,  0.5f, 0.0f, -1.0f,  0.5f, -0.5f, -1.0f,

		// Roof of the car
		-0.3f, 0.0f, -0.5f,  0.3f, 0.0f, -0.5f,  0.3f, 0.3f, -0.5f,
		0.3f, 0.3f, -0.5f,  -0.3f, 0.3f, -0.5f,  -0.3f, 0.0f, -0.5f,

		-0.3f, 0.0f, 0.5f,  0.3f, 0.0f, 0.5f,  0.3f, 0.3f, 0.5f,
		0.3f, 0.3f, 0.5f,  -0.3f, 0.3f, 0.5f,  -0.3f, 0.0f, 0.5f,

		// Front windshield
		-0.3f, 0.3f, 0.5f,  0.3f, 0.3f, 0.5f,  0.3f, 0.3f, -0.5f,
		0.3f, 0.3f, -0.5f,  -0.3f, 0.3f, -0.5f,  -0.3f, 0.3f, 0.5f,

		// Hood
		-0.5f, 0.0f, 0.5f,  0.5f, 0.0f, 0.5f,  0.3f, 0.3f, 0.5f,
		0.3f, 0.3f, 0.5f,  -0.3f, 0.3f, 0.5f,  -0.5f, 0.0f, 0.5f,

		// Trunk
		-0.5f, 0.0f, -0.5f,  0.5f, 0.0f, -0.5f,  0.3f, 0.3f, -0.5f,
		0.3f, 0.3f, -0.5f,  -0.3f, 0.3f, -0.5f,  -0.5f, 0.0f, -0.5f,

		// Front left wheel
		-0.6f, -0.5f, 0.7f,  -0.4f, -0.5f, 0.7f,  -0.4f, -0.3f, 0.7f,
		-0.4f, -0.3f, 0.7f,  -0.6f, -0.3f, 0.7f,  -0.6f, -0.5f, 0.7f,

		-0.6f, -0.5f, 0.9f,  -0.4f, -0.5f, 0.9f,  -0.4f, -0.3f, 0.9f,
		-0.4f, -0.3f, 0.9f,  -0.6f, -0.3f, 0.9f,  -0.6f, -0.5f, 0.9f,

		-0.6f, -0.5f, 0.7f,  -0.6f, -0.5f, 0.9f,  -0.6f, -0.3f, 0.9f,
		-0.6f, -0.3f, 0.9f,  -0.6f, -0.3f, 0.7f,  -0.6f, -0.5f, 0.7f,

		-0.4f, -0.5f, 0.7f,  -0.4f, -0.5f, 0.9f,  -0.4f, -0.3f, 0.9f,
		-0.4f, -0.3f, 0.9f,  -0.4f, -0.3f, 0.7f,  -0.4f, -0.5f, 0.7f,

		// Front right wheel
		0.4f, -0.5f, 0.7f,  0.6f, -0.5f, 0.7f,  0.6f, -0.3f, 0.7f,
		0.6f, -0.3f, 0.7f,  0.4f, -0.3f, 0.7f,  0.4f, -0.5f, 0.7f,

		0.4f, -0.5f, 0.9f,  0.6f, -0.5f, 0.9f,  0.6f, -0.3f, 0.9f,
		0.6f, -0.3f, 0.9f,  0.4f, -0.3f, 0.9f,  0.4f, -0.5f, 0.9f,

		0.4f, -0.5f, 0.7f,  0.4f, -0.5f, 0.9f,  0.4f, -0.3f, 0.9f,
		0.4f, -0.3f, 0.9f,  0.4f, -0.3f, 0.7f,  0.4f, -0.5f, 0.7f,

		0.6f, -0.5f, 0.7f,  0.6f, -0.5f, 0.9f,  0.6f, -0.3f, 0.9f,
		0.6f, -0.3f, 0.9f,  0.6f, -0.3f, 0.7f,  0.6f, -0.5f, 0.7f,

		// Back left wheel
		-0.6f, -0.5f, -0.9f,  -0.4f, -0.5f, -0.9f,  -0.4f, -0.3f, -0.9f,
		-0.4f, -0.3f, -0.9f,  -0.6f, -0.3f, -0.9f,  -0.6f, -0.5f, -0.9f,

		-0.6f, -0.5f, -0.7f,  -0.4f, -0.5f, -0.7f,  -0.4f, -0.3f, -0.7f,
		-0.4f, -0.3f, -0.7f,  -0.6f, -0.3f, -0.7f,  -0.6f, -0.5f, -0.7f,

		-0.6f, -0.5f, -0.9f,  -0.6f, -0.5f, -0.7f,  -0.6f, -0.3f, -0.7f,
		-0.6f, -0.3f, -0.7f,  -0.6f, -0.3f, -0.9f,  -0.6f, -0.5f, -0.9f,

		-0.4f, -0.5f, -0.9f,  -0.4f, -0.5f, -0.7f,  -0.4f, -0.3f, -0.7f,
		-0.4f, -0.3f, -0.7f,  -0.4f, -0.3f, -0.9f,  -0.4f, -0.5f, -0.9f,

		// Back right wheel
		0.4f, -0.5f, -0.9f,  0.6f, -0.5f, -0.9f,  0.6f, -0.3f, -0.9f,
		0.6f, -0.3f, -0.9f,  0.4f, -0.3f, -0.9f,  0.4f, -0.5f, -0.9f,

		0.4f, -0.5f, -0.7f,  0.6f, -0.5f, -0.7f,  0.6f, -0.3f, -0.7f,
		0.6f, -0.3f, -0.7f,  0.4f, -0.3f, -0.7f,  0.4f, -0.5f, -0.7f,

		0.4f, -0.5f, -0.9f,  0.4f, -0.5f, -0.7f,  0.4f, -0.3f, -0.7f,
		0.4f, -0.3f, -0.7f,  0.4f, -0.3f, -0.9f,  0.4f, -0.5f, -0.9f,

		0.6f, -0.5f, -0.9f,  0.6f, -0.5f, -0.7f,  0.6f, -0.3f, -0.7f,
		0.6f, -0.3f, -0.7f,  0.6f, -0.3f, -0.9f,  0.6f, -0.5f, -0.9f
	};

int main()
{
	resetCamera();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	#ifdef __APPLE__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	#endif

	#ifdef __linux__ 
	if (glewInit() != GLEW_OK) {
		std::cout << "Ocorreu um erro inciando o GLEW!" << std::endl;
		return 1;
	} else {
		std::cout << "GLEW inicializado com sucesso!" << std::endl;
		std::cout << glGetString(GL_VERSION) << std::endl;
	}
	#endif


	GLuint shaderTrack = compileShader(vertexShaderSource, fragmentShaderTrack);
	GLuint shaderCar = compileShader(vertexShaderSource, fragmentShaderCar);

	GLuint VBO_Track, VAO_Track, VBO_Car, VAO_Car;
	glGenVertexArrays(1, &VAO_Track);
	glGenBuffers(1, &VBO_Track);

	glGenVertexArrays(1, &VAO_Car);
	glGenBuffers(1, &VBO_Car);

	// TRACK
	glBindVertexArray(VAO_Track);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Track);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// CAR
	glBindVertexArray(VAO_Car);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Car);
	glBufferData(GL_ARRAY_BUFFER, sizeof(carVertices), carVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// LOAD TRACK TEXTURE
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

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		// Atender os eventos
		processInput(window);

		// Limpar a tela
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(zoom), 800.0f / 600.0f, 0.1f, 100.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);

		model = glm::mat4(1.0f);
		glUseProgram(shaderTrack);
		glUniformMatrix4fv(glGetUniformLocation(shaderTrack, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderTrack, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderTrack, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO_Track);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
		glUseProgram(shaderCar);
		glUniformMatrix4fv(glGetUniformLocation(shaderCar, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderCar, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderCar, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO_Car);
		glDrawArrays(GL_TRIANGLES, 0, 36 * 1000);

		// Trocar os buffers da janela
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO_Track);
	glDeleteBuffers(1, &VBO_Track);
	glDeleteVertexArrays(1, &VAO_Car);
	glDeleteBuffers(1, &VBO_Car);
	glDeleteProgram(shaderTrack);
	glDeleteProgram(shaderCar);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Funções auxiliares
GLuint compileShader(const char *vertexSource, const char *fragmentSource)
{
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

void checkShaderCompileStatus(GLuint shader)
{
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		fprintf(stderr, "Erro de compilação de shader: %s\n", infoLog);
	}
}

void zoomControl(float z)
{
	zoom += z;
	if (zoom < 1.0f) zoom = 1.0f;
	if (zoom > 100.0f) zoom = 100.0f;
}

void viraCamera(float x, float y)
{
	yaw += x * sensitivity;
	pitch += y * sensitivity;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void checkProgramLinkStatus(GLuint program)
{
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		fprintf(stderr, "Erro ao linkar programa: %s\n", infoLog);
	}
}
void resetCamera()
{
	// printf("CameraPos: %f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z);
	// printf("CameraFront: %f, %f, %f\n", cameraFront.x, cameraFront.y, cameraFront.z);
	// printf("CameraUp: %f, %f, %f\n", cameraUp.x, cameraUp.y, cameraUp.z);
	// printf("Yaw: %f\n", yaw);
	// printf("Pitch: %f\n", pitch);
	// printf("Zoom: %f\n", zoom);

    cameraPos   = glm::vec3(-0.1f, 3.0f,  10.0f);
    cameraFront = glm::vec3(0.0f, -0.3f, -1.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    sensitivity = 1.0f;
    yaw = -90.0f;
    pitch = 0.0f;
    zoom = 45.0f;
}

void processInput(GLFWwindow *window)
{
    const float cameraSpeed = 0.05f; // adjust accordingly

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        cameraPos += glm::vec3(0.0f, -1.0f, 0.0f) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        viraCamera(0.0f,1.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        viraCamera(0.0f,-1.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        viraCamera(-1.0f,0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        viraCamera(1.0f,0.0f);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        zoomControl(1.0f);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        zoomControl(-1.0f);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        resetCamera();
}
