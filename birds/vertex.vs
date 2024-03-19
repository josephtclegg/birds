#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 normal;
out vec2 TexCoord;
out vec2 pos;
out vec3 FragPos;

uniform float st;
uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	float s = abs(sin(st/170));
	float c = abs(cos(st/170));
	//gl_Position = vec4(vec3(aPos.x, aPos.y+0.2, aPos.z), 1.0);
	gl_Position = transform * vec4(aPos, 1.0f);
	//gl_Position = projection * view * model * vec4(aPos, 1.0);
	//normal = aNormal;
	//pos = gl_Position.xy;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	//TexCoord = (TexCoord*model).xy;

	//FragPos = vec3(transform * vec4(aPos, 1.0));
	normal = mat3(transpose(inverse(transform))) * aNormal;
	normal = vec3(normal.x, normal.y, normal.z);

	//gl_Position = projection * view * vec4(FragPos, 1.0);
	//pos = FragPos.xy;
	pos = gl_Position.xy;
}