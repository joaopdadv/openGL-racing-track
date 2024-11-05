#include <glad/glad.h>
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
void processInput(GLFWwindow *window, float *x, float *y, float *z);

const char *vertexShaderSource = "#version 330 core\n"
								 "layout(location = 0) in vec3 position;\n"
								 "uniform mat4 model;\n"
								 "uniform mat4 view;\n"
								 "uniform mat4 projection;\n"
								 "void main() {\n"
								 "   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
								 "}\n";

const char *fragmentShaderSource4 = "#version 330 core\n"
									"out vec4 color;\n"
									"void main() {\n"
									"   color = vec4(1.0, 1.0, 0.0, 0.0); // Amarelo\n"
									"}\n";

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float groundVertices[] = {
	-5.0f, -0.6f, 5.0f,
	5.0f, -0.6f, -5.0f,
	5.0f, -0.6f, 5.0f,
	-5.0f, -0.6f, 5.0f,
	-5.0f, -0.6f, -5.0f,
	5.0f, -0.6f, -5.0f};

int main()
{
	float x = 0.0f;
	float y = 0.0f;
	float z = -5.0f;

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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	GLuint shaderProgram4 = compileShader(vertexShaderSource, fragmentShaderSource4);

	GLuint VBO1, VAO1;
	glGenVertexArrays(1, &VAO1);
	glGenBuffers(1, &VBO1);

	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		// Atender os eventos
		processInput(window, &x, &y, &z);

		// Limpar a tela
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(x, y, z));
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		model = glm::mat4(1.0f);
		glUseProgram(shaderProgram4);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram4, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram4, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram4, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO1);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Trocar os buffers da janela
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO1);
	glDeleteBuffers(1, &VBO1);
	glDeleteProgram(shaderProgram4);

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

void processInput(GLFWwindow *window, float *x, float *y, float *z)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		*x += 0.1f;
		printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		*x -= 0.1f;
		printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		*y += 0.1f;
		printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		*y -= 0.1f;
		printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		*z += 0.1f;
		printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		*z -= 0.1f;
		printf("(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		*x = 0.0f;
		*y = 0.0f;
		*z = -5.0f;
		printf("RESET\n(%.3f,%.3f,%.3f\n", *x, *y, *z);
	}
}