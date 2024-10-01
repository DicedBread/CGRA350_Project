
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj,
	double deformation, double cov_density, double time) {
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
	glUniform1f(glGetUniformLocation(shader, "uCovDensity"), cov_density);

	// draw
	drawSphere();
}


Application::Application(GLFWwindow *window) : m_window(window) {
	
	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(1, 1, 1);

	setup();
}

void Application::setup() {
	m_is_rotate = false;
	m_cur_time = 0;
	m_rotate_radius = 25;
	m_rotate_speed = 0.5;
	m_distance = 10;
	m_yaw = 0;
	m_deformation = 0;
	m_veg_cov_density = 0;
}

void Application::render() {

	// setup current time
	m_cur_time += 0.1;

	if (m_is_rotate) {
		m_yaw = m_cur_time * m_rotate_speed * pi<float>() / 180.0;
	}

	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view;
	if (m_is_rotate) {
		view = translate(mat4(1), vec3(0, 0, -m_distance)) *
			rotate(mat4(1), m_pitch, vec3(1, 0, 0)) *
			rotate(mat4(1), m_yaw, vec3(0, 1, 0)) *
			translate(mat4(1), vec3(0, 0, m_rotate_radius));
	}
	else {
		view = translate(mat4(1), vec3(0, 0, -m_distance)) *
			rotate(mat4(1), m_pitch, vec3(1, 0, 0)) *
			rotate(mat4(1), m_yaw, vec3(0, 1, 0));
	}


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw the model
	m_model.draw(view, proj, m_deformation, m_veg_cov_density, m_cur_time);
}

void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 280), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// deformation level
	ImGui::SliderFloat("Deformation", &m_deformation, 0.0, 0.01, "%.3lf");
	ImGui::SliderFloat("Rotate Speed", &m_rotate_speed, 0.1, 1, "%.1lf");
	ImGui::SliderFloat("Rotate Radius", &m_rotate_radius, 10, 50, "%.0lf");
	ImGui::SliderFloat("Distance", &m_distance, 1, 20, "%.1lf");
	ImGui::SliderFloat("Veg-Cov Density", &m_veg_cov_density, 0.0, 1.0, "%.1lf");

	// pitch yaw
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>()/2, pi<float>()/2, "%.2lf");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2lf");

	// rotate
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::SameLine();
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::Checkbox("Rotate", &m_is_rotate);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	
	// finish creating window
	ImGui::End();
}

void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
