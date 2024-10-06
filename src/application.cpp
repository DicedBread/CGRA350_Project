
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

Application::Application(GLFWwindow *window) : m_window(window) {
    m_previousFrameTime = std::chrono::system_clock::now();
	// pe.InitParticleSystem(vec3(0,0,0));
	
	setup();
	asteroidCount = 30;
    
	for(int i = 0; i < asteroidCount; i++){
        spawnAsteroid();
        cout << i << " out of " << asteroidCount << " loaded" << endl;
    }
}

void Application::setup() {
	m_distance = 10;
	m_yaw = 0;
	m_deformation = 0;
	m_veg_cov_density = 0;
}

void Application::render() {
	auto currentTime = std::chrono::system_clock::now();
    auto deltaTime =
        std::chrono::duration<double>(
			currentTime - m_previousFrameTime).count();
    m_previousFrameTime = currentTime;
	
	// auto deltaTime = m_delat_t;

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
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance)) *
		rotate(mat4(1), m_pitch, vec3(1, 0, 0)) *
		rotate(mat4(1), m_yaw, vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	for(int i = 0; i < m_asteroids.size(); i++){
        bool shouldReset = m_asteroids.at(i).asteroid.position.y < resetYLevel;
        if(shouldReset){
            randomizeAsteroidParams(m_asteroids.at(i));
        }
    }    

    // pe.updateParticles(deltaTime);
    for (auto &aAndPe : m_asteroids) {
        aAndPe.asteroid.update_model_transform(deltaTime);
        aAndPe.particleEmitter.updateParticles(deltaTime);
        aAndPe.asteroid.draw(view, proj, m_deformation, m_veg_cov_density);
    }

    for (auto &aAndPe : m_asteroids) {
        aAndPe.particleEmitter.render(view, proj);
    }
}

void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// deformation level
	ImGui::SliderFloat("Deformation", &m_deformation, 0.0, 0.1, "%.2lf");
	// ImGui::SliderFloat("Rotate Speed", &m_rotate_speed, 0.1, 1, "%.1lf");
	// ImGui::SliderFloat("Rotate Radius", &m_rotate_radius, 10, 50, "%.0lf");
	ImGui::SliderFloat("Distance", &m_distance, 1, 20, "%.1lf");
	ImGui::SliderFloat("Veg-Cov Density", &m_veg_cov_density, 0.0, 1.0, "%.1lf");

	// pitch yaw
	// ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>()/2, pi<float>()/2, "%.2lf");
	// ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2lf");

	// rotate
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::SameLine();
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::Checkbox("Rotate", &m_is_rotate);
	ImGui::Checkbox("Wireframe", &m_showWireframe);

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Scene edit")) {
		ImGui::SliderFloat("Spawn height", &spawnHeight, 0, 500);
		ImGui::SliderFloat("Reset height", &resetYLevel, -100, 500);
	}

	if (ImGui::CollapsingHeader("Particle emitters")) {
		for (int i = 0; i < m_asteroids.size(); i++) {
			ParticleModifier pm(m_asteroids.at(i).particleEmitter);
			pm.drawUi();
		}
	}

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

void Application::spawnAsteroid() {
    // Asteroid a = ;
    // ParticleEmitter pe = ;
    // AsteroidAndPartEmitter e(a, pe);
    AsteroidAndPartEmitter e = {Asteroid(std::chrono::system_clock::now().time_since_epoch().count()), ParticleEmitter()};
    e.particleEmitter.InitParticleSystem(vec3(0));
    randomizeAsteroidParams(e);
    m_asteroids.push_back(e);
}

void Application::cullAsteroids() {
    // m_asteroids.remove_if(
    //     [this](const Asteroid &asteroid) { return asteroid.position.y < -10; });
}

void Application::randomizeAsteroidParams(AsteroidAndPartEmitter& aAndPe){
    static std::random_device rd;
    static std::mt19937 rng(rd());
    static std::uniform_real_distribution<> spawn_position_dist(-200, 200);
    static std::uniform_real_distribution<> target_position_dist(-50, 50);
    static std::uniform_real_distribution<> velocityXZRange(-10, 10);

    static std::uniform_real_distribution<> speed_dist(10, 15);
    static std::normal_distribution<> rotation_axis_dist(0, 1);
    static std::uniform_int_distribution<> rotation_velocity_dist(1, 4);

    vec3 spawn_position =
        vec3(spawn_position_dist(rng), spawnHeight, spawn_position_dist(rng));

    vec3 target_position =
        vec3(spawn_position.x + target_position_dist(rng), 0, spawn_position.z + target_position_dist(rng));

    vec3 velocity =
        normalize(target_position - spawn_position) * (float)speed_dist(rng);

    vec3 rotation_axis = vec3(rotation_axis_dist(rng), rotation_axis_dist(rng),
                              rotation_axis_dist(rng));

    double rotation_velocity = rotation_velocity_dist(rng);

    aAndPe.asteroid.position = spawn_position;
    aAndPe.asteroid.velocity = velocity;
    aAndPe.asteroid.rotation_axis = rotation_axis;
    aAndPe.asteroid.rotation_velocity = rotation_velocity;

    // aAndPe.particleEmitter.destroy();
    // aAndPe.particleEmitter.InitParticleSystem(spawn_position);
    aAndPe.particleEmitter.updatePosition(spawn_position);
    aAndPe.particleEmitter.emitterVelocity = velocity;
    aAndPe.particleEmitter.emitterSpeed = length(velocity);
    aAndPe.particleEmitter.initVelocity = velocity;
    aAndPe.particleEmitter.speed = length(velocity) / 2;
    aAndPe.particleEmitter.initColor = vec3(1, 0.3, 0);
    aAndPe.particleEmitter.endColor = vec3(1, 0.8, 0);
    aAndPe.particleEmitter.lifeTime = 3;
    aAndPe.particleEmitter.spawnRadius = 2;
}
