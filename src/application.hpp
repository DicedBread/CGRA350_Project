
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"

#include "Asteroid.hpp"
#include "ParticleEmitter.hpp"
#include "ParticleModifier.hpp"
#include "CenterBody.hpp"

struct AsteroidAndPartEmitter {
    Asteroid asteroid;
    ParticleEmitter particleEmitter;
};

// Main application class
//
class Application {
  private:
    // window
    glm::vec2 m_windowsize;
    GLFWwindow *m_window;

    // oribital camera
    float m_pitch = .0;
    float m_yaw = .0;
    float m_distance = .0;

    static constexpr const char* scenesStrings[] = {"main", "particle", "asteroid"};
    enum Scenes{
      MAIN,
      PARTICLE,
      ASTEROID
    };

    int activeScene = MAIN;

    // deformation
    float m_deformation = .0;

    // vegetation coverage density
    float m_veg_cov_density = .0;

    // last input
    bool m_leftMouseDown = false;
    glm::vec2 m_mousePosition;

    // drawing flags
    bool m_show_axis = false;
    bool m_show_grid = false;
    bool m_showWireframe = false;

    int asteroidCount = 5;
    float resetYLevel = -30;
    float spawnHeight = 100;

    int m_frames_per_astreroid = 60;
    int m_frames_since_last_asteroid = 0;

    AsteroidMeshConfig asteroidMeshConfig;
    std::vector<AsteroidAndPartEmitter> m_asteroids;

	// central body
	CenterBody centerBody;

    std::chrono::time_point<std::chrono::system_clock> m_previousFrameTime;

  public:
    // setup
    Application(GLFWwindow *);

    // disable copy constructors (for safety)
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    // setup rotate parameters
    void setup();

    // rendering callbacks (every frame)
    void render();
    void renderGUI();
    void spawnAsteroid();
    void cullAsteroids();

    void randomizeAsteroidParams(AsteroidAndPartEmitter &aAndPe);

    // input callbacks
    void cursorPosCallback(double xpos, double ypos);
    void mouseButtonCallback(int button, int action, int mods);
    void scrollCallback(double xoffset, double yoffset);
    void keyCallback(int key, int scancode, int action, int mods);
    void charCallback(unsigned int c);
};
