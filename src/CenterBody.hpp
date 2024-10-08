#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl.hpp"

class CenterBody
{
public:
	CenterBody();
	~CenterBody();

	void draw(const glm::mat4& view, const glm::mat4 proj,
		double deltaTime, double defomation, double covDensity);
private:
	GLuint shader = 0;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	float rotateAngle = 0.0;
	float rotateSpeed = 0.5;
};
