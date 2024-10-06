#version 330 core

layout (location = 0) in float type;
layout (location = 1) in vec3 position;
layout (location = 2) in vec3 velocity;
layout (location = 3) in float age;

out VertexData{
    float type;
    vec3 position;
    vec3 velocity;
    float age;
	vec2 textCord;
} v_out;

void main() {
	gl_Position = vec4(position, 1);
	v_out.type = type;
	v_out.position = position;
	v_out.velocity = velocity;
	v_out.age = age;
	v_out.textCord = vec2(0);
}