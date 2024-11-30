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

#include <shader.h>

#ifdef __linux__
#include <irrKlang.h>
using namespace irrklang;
#endif

GLuint compileShader(const char *vertexSource, const char *fragmentSource);
void checkShaderCompileStatus(GLuint shader);
void checkProgramLinkStatus(GLuint program);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void resetCamera();
int init_gl_context();
void processCarInput(GLFWwindow *window, glm::vec3 &carPosition, float &carRotationAngle, float deltaTime);
bool isCarWithinRadius(const glm::vec3 &carPosition, float minRadius, float maxRadius);
unsigned int load_texture(const char* path);
int init_sound_engine();
void play_bonk();

const char *vertexShaderTest = "#version 330 core\n"
							  "layout(location = 0) in vec3 aPos;"
							  "layout (location = 1) in vec3 aColor;"
							  "layout (location = 2) in vec3 aNormal;"
							  "layout (location = 3) in vec3 aTextCoord;"
							  "uniform mat4 model;"
							  "uniform mat4 view;"
							  "uniform mat4 projection;"
							  "out vec3 Color;"
							  "out vec3 FragPos;"
							  "out vec3 Normal;"
							  "out vec2 TextCoord;"
							  "void main() {"
							  "   FragPos = vec3(model * vec4(aPos, 1.0));"
							  "   Normal = mat3(transpose(inverse(model))) * aNormal;"
							  "   gl_Position = projection * view * model * vec4(FragPos, 1.0f);"
							  "   Color = aColor;"
							  "   TextCoord = vec2(aTextCoord.x, aTextCoord.y);"
							  "}";

const char *fragmentShaderTest = "#version 330 core\n"
							  "in vec3 Color;"
							  "in vec3 FragPos;"
							  "in vec3 Normal;"
							  "in vec2 TextCoord;"
								"uniform sampler2D texture1;"

								"out vec4 FragColor;"

								"struct Material {"
								"	vec3 ambient;"
								"	vec3 diffuse;"
								"	vec3 specular;"    
								"	float shininess;"
								"};" 

								"struct Light {"
								"	vec3 position;"
								"	vec3 ambient;"
								"	vec3 diffuse;"
								"	vec3 specular;"
								"};"

								"uniform vec3 viewPos;"
								"uniform Material material;"
								"uniform Light light;"

								"void main() {"
									"/* ambient */"
									"vec3 ambient = light.ambient * material.ambient;"

									"/* diffuse */" 
									"vec3 norm = normalize(Normal);"
									"vec3 lightDir = normalize(light.position - FragPos);"
									"float diff = max(dot(norm, lightDir), 0.0);"
									"vec3 diffuse = light.diffuse * (diff * material.diffuse);"

									"/* specular */"
									"vec3 viewDir = normalize(viewPos - FragPos);"
									"vec3 reflectDir = reflect(-lightDir, norm);"
									"float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
									"vec3 specular = light.specular * (spec * material.specular);"

									"vec3 result = ambient + diffuse + specular;"
								  "FragColor = vec4(texture(texture1, TextCoord).rgb, 1.0);"
									// "FragColor = vec4(result, 1.0);" // TODO ver como aplicar textura!
								"}";

const char *vertexShaderTexture = "#version 330 core\n"
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

const char *fragmentShaderTexture = "#version 330 core\n"
								  "out vec4 color;"
								  "in vec2 TexCoord;"
								  "uniform sampler2D texture1;"
								  "void main() {"
								  "   color = vec4(texture(texture1, TexCoord).rgb, 1.0);"
								  "}";

const char *vertexShaderColor = "#version 330 core\n"
							  "layout(location = 0) in vec3 position;"
							  "layout (location = 1) in vec4 aColor;"
							  "uniform mat4 model;"
							  "uniform mat4 view;"
							  "uniform mat4 projection;"
							  "out vec4 cor;"
							  "void main() {"
							  "   gl_Position = projection * view * model * vec4(position, 1.0f);"
							  "   cor = aColor;"
							  "}";

const char *fragmentShaderColor = "#version 330 core\n"
								"in vec4 cor;"
								"out vec4 color;"
								"void main() {"
								"   color = cor;"
								"}";

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// cores
glm::vec4 black = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
glm::vec4 pink = glm::vec4(0.867f, 0.667f, 0.933f, 1.0f);
glm::vec4 green = glm::vec4(0.408f,0.722f,0.004f, 1.0f);
glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
glm::vec4 white = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
glm::vec4 yellow = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);

float sensitivity = 1.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float zoom = 45.0f;

GLFWwindow* window;
ISoundEngine* soundEngine;

const int COMPONENTS = 11; // x, y, z, r, g, b, xn, yn, zn, u, v

typedef struct {
	glm::vec3 position;
	std::vector<float> vertices;
	GLuint vao;
	bool texture; // se tem textura entao nao tem cor
} Object;

float groundVertices[] = {
// x       y      z     r     g     b     xn    yn    zn    u     v
	-5.0f,  -0.6f,  5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 5.0f,  -0.6f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	 5.0f,  -0.6f,  5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	-5.0f,  -0.6f,  5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	-5.0f,  -0.6f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	 5.0f,  -0.6f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};


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

std::vector<float> createQuadVertices(float width, float height, float depth, glm::vec3 center, glm::vec4 color)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	float halfDepth = depth / 2.0f;

	std::vector<float> vertices = {
		// Front face
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,

		// Back face
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,

		// Left face
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,

		// Right face
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,

		// Top face
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y + halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y + halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,

		// Bottom face
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x + halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z + halfDepth, color.x, color.y, color.z, color.w,
		center.x - halfWidth, center.y - halfHeight, center.z - halfDepth, color.x, color.y, color.z, color.w,
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
		-0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.0f, -0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.0f, 0.5f,   0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, 0.5f,   0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, 0.5f,   0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f,

		// top
		-0.3f, 0.3f, 0.5f,  0.867f, 0.667f, 0.933f,1.0f,
		0.3f, 0.3f, 0.5f,   0.867f, 0.667f, 0.933f,1.0f,
		0.3f, 0.3f, -0.5f,  0.867f, 0.667f, 0.933f,1.0f,
		0.3f, 0.3f, -0.5f,  0.867f, 0.667f, 0.933f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.867f, 0.667f, 0.933f,1.0f,
		-0.3f, 0.3f, 0.5f,  0.867f, 0.667f, 0.933f,1.0f,

		// front
		-0.5f, 0.0f, 0.5f, 0.0f,0.451f,1.0f,1.0f,
		0.5f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, 0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.5f, 0.0f, 0.5f, 0.0f,0.451f,1.0f,1.0f,

		// back
		-0.5f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		0.5f, 0.0f, -0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, -0.5f,  0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.5f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,1.0f,

		// Left window
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		-0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f,

		// Right window
		0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, 0.5f,  0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.3f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.0f, -0.5f, 0.0f,0.451f,1.0f,1.0f,
		0.3f, 0.0f, 0.5f,  0.0f,0.451f,1.0f,1.0f};

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
	auto color = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
	return createPlaneWithTexture(1.0f, 1.8f, position);
}

std::vector<float> createSignBaseVertices(glm::vec3 position)
{
	float height = 1.0f;
	float width = 0.05f;
	auto base =  createQuadVertices(width, height, width, position, black);
	return base;
}

std::vector<float> createSignVertices(glm::vec3 position)
{
	float height = 1.0f;

	glm::vec3 signPos = position;
	signPos.y += height - 0.25f;
	auto sign = createPlaneWithTexture(0.5f, 0.5f, signPos);
	return sign;
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
	int components = obj.texture ? 5 : 7;
	
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
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, components * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
}

// TODO luz e reflexo
// TODO diminuir volume da musica de fundo
// TODO editar sound do bonk para ser mais instantaneo
/**
 * x, y, z, r, g, b, xn, yn, zn, u, v
 * 
 * vetores normais devem ser calculados (xn,yn,zn) a partir de x, y e z
 * auto v1 = glm::cross(vec3(x,y,z), vec3(x2,y2,z2));
 * compara-se a linha atual (x,y,z) com a linha seguinte (x2,y2,z2)
 * auto vetorNormal = glm::normalize(v1);
 * 11 componentes, fazer com que todo struct Object tenha essas componentes
 * para posicionar a luz tem exemplo nos slides de uma struct Light no shader
*/

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main()
{
	if (init_gl_context() < 0) return 1;
	if (init_sound_engine() < 0) return 1;
	
	resetCamera();

	float lastFrame = 0.0f;
	glm::vec3 carPosition(2.7f, 0.0f, 1.0f);
	float carRotationAngle = 0.0f;

	GLuint VBO_Track, VAO_Track, VBO_Car, VAO_Car, VBO_Tree1, VAO_Tree1;
	GLuint VBO_Bg, VAO_Bg, VBO_Sign, VAO_Sign, VBO_SignBase, VAO_SignBase;
	glGenVertexArrays(1, &VAO_Track);
	glGenBuffers(1, &VBO_Track);

	glGenVertexArrays(1, &VAO_Car);
	glGenBuffers(1, &VBO_Car);

	glGenVertexArrays(1, &VAO_Tree1);
	glGenBuffers(1, &VBO_Tree1);

	glGenVertexArrays(1, &VAO_Bg);
	glGenBuffers(1, &VBO_Bg);

	glGenVertexArrays(1, &VAO_Sign);
	glGenBuffers(1, &VBO_Sign);

	glGenVertexArrays(1, &VAO_SignBase);
	glGenBuffers(1, &VBO_SignBase);

	Object tree1, car, background, sign, signBase;

	car.position = glm::vec3(0.0f);
	car.vertices = createCarVertices();
	car.vao = VAO_Car;
	car.texture = false;

	sign.position = glm::vec3(0.0f, -0.2f, 0.0f);
	sign.vertices = createSignVertices(sign.position);
	sign.vao = VAO_Sign;
	sign.texture = true;

	signBase.position = sign.position;
	signBase.vertices = createSignBaseVertices(signBase.position);
	signBase.vao = VAO_SignBase;
	signBase.texture = false;

	background.position = glm::vec3(0.0f, 4.0f, -7.0f);
	background.vertices = createPlaneWithTexture(35.0f, 10.0f, background.position);
	background.vao = VAO_Bg;
	background.texture = true;

	GLuint shaderTexture = compileShader(vertexShaderTexture, fragmentShaderTexture);
	GLuint shaderColor = compileShader(vertexShaderColor, fragmentShaderColor);
	GLuint shaderTest = compileShader(vertexShaderTest, fragmentShaderTest);
	Shader lightingShader("./shaders/test.vs", "./shaders/test.fs");

	// TRACK
	glBindVertexArray(VAO_Track);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Track);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, COMPONENTS * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	// cor
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, COMPONENTS * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// normais
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, COMPONENTS * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// texture
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, COMPONENTS * sizeof(float), (void *)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBind_object(car);
	glBind_object(background);
	glBind_object(sign);
	glBind_object(signBase);

	// LOAD TRACK TEXTURE
	unsigned int track_texture = load_texture("./images/track2.jpg");
	unsigned int test_texture = load_texture("./images/crowd-1.jpg");
	// unsigned int tree_texture = load_texture("./images/xmas-tree.png");
	unsigned int sign_texture = load_texture("./images/rotatoria.jpg");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window))
	{
		// Atender os eventos
		// processInput(window);

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
		lightingShader.use();
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", cameraPos);

		glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // decrease the influence
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence

		lightingShader.setVec3("light.ambient", ambientColor);
		lightingShader.setVec3("light.diffuse", diffuseColor);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		lightingShader.setFloat("material.shininess", 32.0f);

		lightingShader.setMat4("model", model);
		lightingShader.setMat4("view", view);
		lightingShader.setMat4("projection", projection);
		
		glBindVertexArray(VAO_Track);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// bg
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, test_texture);
		model = glm::mat4(1.0f);
		render_object(background, 6, shaderTexture, model, view, projection);

		// sign
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sign_texture);
		render_object(sign, 6, shaderTexture, model, view, projection);

		render_object(signBase, 1024, shaderColor, model, view, projection);


		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame; // Calcula o tempo decorrido entre os frames
		lastFrame = currentFrame;
		// Atualiza a posição e rotação do carro
		processCarInput(window, carPosition, carRotationAngle, deltaTime);

		// Verifica se está dentro do raio
		if (!isCarWithinRadius(carPosition, 1.8f, 4.0f)) {
			play_bonk(); // TODO editar o audio para o bonk tocar instantaneamente
			carPosition = glm::vec3(2.7f, 0.0f, 1.0f);
			carRotationAngle = 0.0f;
			// std::cout << "O carro saiu da área permitida!" << std::endl;
		} else {
			// std::cout << "O carro está dentro da área permitida." << std::endl;
		}

		// Cria a matriz de modelo do carro
		// glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, carPosition); // Translação para a posição do carro
		model = glm::rotate(model, glm::radians(carRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotação do carro
		render_object(car, 36*1000, shaderColor, model, view, projection);

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
	glDeleteProgram(shaderTexture);
	glDeleteProgram(shaderColor);

	glfwTerminate();
	return 0;
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
}

bool isCarWithinRadius(const glm::vec3 &carPosition, float minRadius, float maxRadius) {
    float distance = glm::length(carPosition);
    return distance <= maxRadius && distance >= minRadius;
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

// void processInput(GLFWwindow *window)
// {
// 	const float cameraSpeed = 0.05f; // adjust accordingly

// 	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
// 		glfwSetWindowShouldClose(window, true);

// 	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
// 		cameraPos += cameraSpeed * cameraFront;
// 	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
// 		cameraPos -= cameraSpeed * cameraFront;
// 	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
// 		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
// 	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
// 		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
// 	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
// 		cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
// 	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
// 		cameraPos += glm::vec3(0.0f, -1.0f, 0.0f) * cameraSpeed;

// 	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
// 		viraCamera(0.0f, 1.0f);
// 	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
// 		viraCamera(0.0f, -1.0f);
// 	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
// 		viraCamera(-1.0f, 0.0f);
// 	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
// 		viraCamera(1.0f, 0.0f);

// 	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
// 		zoomControl(1.0f);
// 	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
// 		zoomControl(-1.0f);

// 	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
// 		resetCamera();
// }

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
		exit(1);
	}
	stbi_image_free(data);

	return texture1;
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
	soundEngine->play2D("sounds/bonk.ogg");
}
