#include "CenterBody.hpp"

#include <string>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_shader.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

CenterBody::CenterBody()
{
	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + 
		std::string("//res//shaders//color_vert_central.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + 
		std::string("//res//shaders//color_frag_central.glsl"));
	shader = sb.build();
}

CenterBody::~CenterBody()
{

}

void CenterBody::draw(const glm::mat4 &view, const glm::mat4 proj,
	double deltaTime, double deformation, double covDensity) {

	rotateAngle += deltaTime * rotateSpeed;
	rotateAngle = fmod(rotateAngle, 2 * pi<float>());
	modelTransform = translate(mat4(1), vec3(0, 0, 6)) *
		rotate(mat4(1), rotateAngle, vec3(0, 1, 0));

	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	// deformation or not
	int is_deformation = fabs(deformation) > 1E-3 ? 1 : 0;
	glUniform1i(glGetUniformLocation(shader, "uIsDeformation"), is_deformation);
	glUniform1f(glGetUniformLocation(shader, "uDeformation"), deformation);

	// cov-density
	glUniform1f(glGetUniformLocation(shader, "uCovDensity"), covDensity);

	// draw
	drawSphere();
}
