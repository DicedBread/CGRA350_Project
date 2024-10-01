#version 330 core

// mesh data
layout(location = 0) in vec3 aPosition;

void main() {
	gl_Position = vec4(aPosition, 1);
}