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
#include <vector>

GLuint compileShader(const char *vertexSource, const char *fragmentSource);
void checkShaderCompileStatus(GLuint shader);
void checkProgramLinkStatus(GLuint program);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void resetCamera();
int init_gl_context();
unsigned int load_texture(const char* path);

const char *vertexShaderTrack = "#version 330 core\n"
								"layout(location = 0) in vec3 position;"
								"layout (location = 1) in vec2 aTexCoord;"
								"out vec2 TexCoord;"
								"uniform mat4 model;"
								"uniform mat4 view;"
								"uniform mat4 projection;"
								"void main() {"
								"   gl_Position = projection * view * model * vec4(position, 1.0f);"
								"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);"
								"}";

const char *fragmentShaderTrack = "#version 330 core\n"
								  "out vec4 color;"
								  "in vec2 TexCoord;"
								  "uniform sampler2D texture1;"
								  "void main() {"
								  "   color = vec4(texture(texture1, TexCoord).rgb, 1.0);"
								  "}";

const char *vertexShaderCar2 = "#version 330 core\n"
							   "layout(location = 0) in vec3 position;"
							   "layout (location = 1) in vec2 aTexCoord;"
							   "out vec2 TexCoord;"
							   "uniform mat4 model;"
							   "uniform mat4 view;"
							   "uniform mat4 projection;"
							   "void main() {"
							   "   gl_Position = projection * view * model * vec4(position, 1.0f);"
							   "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);"
							   "}";

const char *vertexShaderCar = "#version 330 core\n"
							  "layout(location = 0) in vec3 position;"
							  "layout (location = 1) in vec3 aColor;"
							  "uniform mat4 model;"
							  "uniform mat4 view;"
							  "uniform mat4 projection;"
							  "out vec3 cor;"
							  "void main() {"
							  "   gl_Position = projection * view * model * vec4(position, 1.0f);"
							  "   cor = aColor;"
							  "}";

const char *fragmentShaderCar = "#version 330 core\n"
								"in vec3 cor;"
								"out vec4 color;"
								"void main() {"
								"   color = vec4(cor, 1.0);"
								"}";

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// cores
glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 pink = glm::vec3(0.867f, 0.667f, 0.933f);
glm::vec3 green = glm::vec3(0.408f,0.722f,0.004f);
glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 yellow = glm::vec3(1.0f, 0.5f, 0.0f);

float sensitivity = 1.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float zoom = 45.0f;

GLFWwindow* window;

typedef struct {
	glm::vec3 position;
	std::vector<float> vertices;
	GLuint vao;
	bool texture; // se tem textura entao nao tem cor
} Object;

float groundVertices[] = {
	-5.0f, -0.6f, 5.0f, 0.0f, 0.0f,
	5.0f, -0.6f, -5.0f, 1.0f, 1.0f,
	5.0f, -0.6f, 5.0f, 1.0f, 0.0f,
	-5.0f, -0.6f, 5.0f, 0.0f, 0.0f,
	-5.0f, -0.6f, -5.0f, 0.0f, 1.0f,
	5.0f, -0.6f, -5.0f, 1.0f, 1.0f};

std::vector<float> createPlaneWithTexture(float width, float height, glm::vec3 center)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	std::vector<float> vertices = {
		center.x - halfWidth, center.y - halfHeight, center.z, 0.0f, 0.0f,
		center.x + halfWidth, center.y - halfHeight, center.z, 1.0f, 0.0f,
		center.x + halfWidth, center.y + halfHeight, center.z, 1.0f, 1.0f,
		center.x + halfWidth, center.y + halfHeight, center.z, 1.0f, 1.0f,
		center.x - halfWidth, center.y + halfHeight, center.z, 0.0f, 1.0f,
		center.x - halfWidth, center.y - halfHeight, center.z, 0.0f, 0.0f
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
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,

		// Back face
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,

		// Left face
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,

		// Right face
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,

		// Top face
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z,

		// Bottom face
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z,
	};

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

std::vector<float> createCarVertices()
{
	std::vector<float> bottom = createQuadVertices(1.0f, 0.5f, 2.0f,
												   glm::vec3(0.0f, -0.20f, 0.0f),
												   pink);

	std::vector<float> top = {
		-0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,
		0.3f, 0.0f, -0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,
		-0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.0f, 0.5f,   0.0f,0.451f,1.0f,
		0.3f, 0.3f, 0.5f,   0.0f,0.451f,1.0f,
		0.3f, 0.3f, 0.5f,   0.0f,0.451f,1.0f,
		-0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,

		// top
		-0.3f, 0.3f, 0.5f,  0.867f, 0.667f, 0.933f,
		0.3f, 0.3f, 0.5f,   0.867f, 0.667f, 0.933f,
		0.3f, 0.3f, -0.5f,  0.867f, 0.667f, 0.933f,
		0.3f, 0.3f, -0.5f,  0.867f, 0.667f, 0.933f,
		-0.3f, 0.3f, -0.5f, 0.867f, 0.667f, 0.933f,
		-0.3f, 0.3f, 0.5f,  0.867f, 0.667f, 0.933f,

		// front
		-0.5f, 0.0f, 0.5f, 0.0f,0.451f,1.0f,
		0.5f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,
		-0.3f, 0.3f, 0.5f, 0.0f,0.451f,1.0f,
		-0.5f, 0.0f, 0.5f, 0.0f,0.451f,1.0f,

		// back
		-0.5f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,
		0.5f, 0.0f, -0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,
		-0.5f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,

		// Left window
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,
		-0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,
		-0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,

		// Right window
		0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,
		0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,
		0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,
		0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,
		0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f};

	std::vector<float> lights;

	std::vector<float> headlights = createQuadVertices(0.2f, 0.2f, 0.1f,     glm::vec3(-0.2f, -0.1f, 1.0f), white);
	std::vector<float> rightHeadlight = createQuadVertices(0.2f, 0.2f, 0.1f, glm::vec3(0.2f,  -0.1f, 1.0f), white);

	std::vector<float> taillights = createQuadVertices(0.25f, 0.1f, 0.1f,     glm::vec3(-0.2f, -0.1f, -1.0f), red);
	std::vector<float> rightTaillight = createQuadVertices(0.25f, 0.1f, 0.1f, glm::vec3(0.2f,  -0.1f, -1.0f), red);

	std::vector<float> leftBlinker = createQuadVertices(0.1f, 0.1f, 0.1f,     glm::vec3(-0.37f, -0.1f, -1.0f), yellow);
	std::vector<float> rightBlinker = createQuadVertices(0.1f, 0.1f, 0.1f,    glm::vec3(0.37f,  -0.1f, -1.0f), yellow);

	mergeVec(lights, headlights, rightHeadlight);
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

std::vector<float> createTreeVertices(glm::vec3 position)
{
	std::vector<float> trunk = createQuadVertices(0.5f, 1.0f, 0.1f,
												   position,
												   pink);
	return trunk;
}

void render_tree(Object& tree, GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	model = glm::mat4(1.0f);
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glBindVertexArray(tree.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	model = glm::rotate(model, glm::radians(90.0f), tree.position);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glBindVertexArray(tree.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_object(Object& obj, int triangles, GLuint shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glBindVertexArray(obj.vao);
	glDrawArrays(GL_TRIANGLES, 0, triangles);
}

void glBind_object(Object& obj)
{
	int components = obj.texture ? 5 : 6;
	
	glBindVertexArray(obj.vao);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vao);
	glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(float), obj.vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	// se tem textura entao nao tem cor
	if (obj.texture) {
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	} else {
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
}

// TODOs
/*
	- adicionar textura na arvore do centro
	- placa
*/

int main()
{
	if (init_gl_context() < 0) return 1;
	resetCamera();

	GLuint VBO_Track, VAO_Track, VBO_Car, VAO_Car, VBO_Tree1, VAO_Tree1;
	GLuint VBO_Bg, VAO_Bg;
	glGenVertexArrays(1, &VAO_Track);
	glGenBuffers(1, &VBO_Track);

	glGenVertexArrays(1, &VAO_Car);
	glGenBuffers(1, &VBO_Car);

	glGenVertexArrays(1, &VAO_Tree1);
	glGenBuffers(1, &VBO_Tree1);

	glGenVertexArrays(1, &VAO_Bg);
	glGenBuffers(1, &VBO_Bg);

	Object tree1, car, background;

	tree1.position = glm::vec3(0.0f, -0.20f, 0.0f);
	tree1.vertices = createTreeVertices(tree1.position);
	tree1.vao = VAO_Tree1;
	tree1.texture = false;

	car.position = glm::vec3(0.0f);
	car.vertices = createCarVertices();
	car.vao = VAO_Car;
	car.texture = false;


	background.position = glm::vec3(0.0f, 4.0f, -7.0f);
	background.vertices = createPlaneWithTexture(35.0f, 10.0f, background.position);
	background.vao = VAO_Bg;
	background.texture = true;

	GLuint shaderTrack = compileShader(vertexShaderTrack, fragmentShaderTrack);
	GLuint shaderCar = compileShader(vertexShaderCar, fragmentShaderCar);

	// TRACK
	glBindVertexArray(VAO_Track);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Track);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBind_object(car);
	glBind_object(tree1);
	glBind_object(background);

	// LOAD TRACK TEXTURE
	unsigned int track_texture = load_texture("./images/track2.jpg");
	unsigned int test_texture = load_texture("./images/crowd-1.jpg");

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		// Atender os eventos
		processInput(window);

		// Limpar a tela
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(green.x, green.y, green.z, 1.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(zoom), 800.0f / 600.0f, 0.1f, 100.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, track_texture);

		model = glm::mat4(1.0f);
		glUseProgram(shaderTrack);
		glUniformMatrix4fv(glGetUniformLocation(shaderTrack, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderTrack, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderTrack, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO_Track);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// bg
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, test_texture);
		model = glm::mat4(1.0f);
		render_object(background, 6, shaderTrack, model, view, projection);

		// tree
		render_tree(tree1, shaderCar, model, view, projection);

		float radius = 3.0f; // Raio do círculo
		float speed = 1.0f;	 // Velocidade de rotação

		float x = radius * cos(speed * glfwGetTime());
		float z = radius * sin(speed * glfwGetTime());

		// Calcula o angulo da rotação com base na tangente do círculo
		float angle = atan2(-z, x);

		model = glm::translate(model, glm::vec3(x, 0.0f, z));			 // Translação para a posição circular
		model = glm::rotate(model, angle, glm::vec3(0.0f, speed, 0.0f)); // Rotação para apontar para onde está indo
		render_object(car, 36*1000, shaderCar, model, view, projection);

		// Trocar os buffers da janela
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO_Track);
	glDeleteBuffers(1, &VBO_Track);
	glDeleteVertexArrays(1, &VAO_Car);
	glDeleteBuffers(1, &VBO_Car);
	glDeleteVertexArrays(1, &VAO_Bg);
	glDeleteBuffers(1, &VBO_Bg);
	glDeleteVertexArrays(1, &VAO_Tree1);
	glDeleteBuffers(1, &VBO_Tree1);
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
	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 100.0f)
		zoom = 100.0f;
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
	cameraPos = glm::vec3(-4.7f, 3.0f, 10.0f);
	cameraFront = glm::vec3(0.4f, -0.3f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

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
		viraCamera(0.0f, 1.0f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		viraCamera(0.0f, -1.0f);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		viraCamera(-1.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		viraCamera(1.0f, 0.0f);

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		zoomControl(1.0f);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		zoomControl(-1.0f);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		resetCamera();
}

int init_gl_context()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Race Track", NULL, NULL);
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
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Ocorreu um erro inciando o GLEW!" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "GLEW inicializado com sucesso!" << std::endl;
		std::cout << glGetString(GL_VERSION) << std::endl;
	}
#endif

	return 1;
}

unsigned int load_texture(const char* path)
{
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
		std::cout << "Failed to load texture aaaaa" << std::endl;
	}
	stbi_image_free(data);

	return texture1;
}