
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <list>

// project
#include "asteroid.hpp"
#include "cgra/cgra_mesh.hpp"

#include "ParticleEmitter.hpp"
#include "ParticleModifier.hpp"

#include <chrono>
#include "opengl.hpp"

struct AsteroidAndPartEmitter{
  Asteroid asteroid;
  ParticleEmitter particleEmitter;
};

// Main application class
//
class Application
{
private:
  // window
  glm::vec2 m_windowsize;
  GLFWwindow *m_window;

  // oribital camera
  float m_pitch = .86;
  float m_yaw = -.86;
  float m_distance = 20;

  // last input
  bool m_leftMouseDown = false;
  glm::vec2 m_mousePosition;

  // drawing flags
  bool m_show_axis = false;
  bool m_show_grid = false;
  bool m_showWireframe = false;

  int asteroidCount = 20;
  float resetYLevel = -30;
  float spawnHeight = 100;


  int m_frames_per_astreroid = 60;
  int m_frames_since_last_asteroid = 0;

  std::vector<AsteroidAndPartEmitter> m_asteroids;

  std::chrono::time_point<std::chrono::system_clock> m_previousFrameTime;

public:
  // setup
  Application(GLFWwindow *);

  // disable copy constructors (for safety)
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  // rendering callbacks (every frame)
  void render();
  void renderGUI();
  void spawnAsteroid();
  void cullAsteroids();

  void randomizeAsteroidParams(AsteroidAndPartEmitter& aAndPe);

  // input callbacks
  void cursorPosCallback(double xpos, double ypos);
  void mouseButtonCallback(int button, int action, int mods);
  void scrollCallback(double xoffset, double yoffset);
  void keyCallback(int key, int scancode, int action, int mods);
  void charCallback(unsigned int c);
};
