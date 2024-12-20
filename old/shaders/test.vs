#version 330 core

layout(location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTextCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Color;
out vec3 FragPos;
out vec3 Normal;
out vec2 TextCoord;

void main() {
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;
  gl_Position = projection * view * model * vec4(FragPos, 1.0f);
  Color = aColor;
  TextCoord = vec2(aTextCoord.x, aTextCoord.y);
};