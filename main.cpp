#include "App.h"
#include "RenderSystem.h"
#include "MissileSystem.h"

#include <iostream>
#include <thread>
#include <sstream>

using namespace std;


static GLFWwindow* setupGLFW(int width, int height, char const* title, GLFWerrorfun error_callback) {
  glfwSetErrorCallback(error_callback);

  // Initialize GLFW and set an error callback
  if (!glfwInit()) {
    cout << "Unable to initialize GLFW" << endl;
    return nullptr;
  }

  // For the sake of OS X, ensure that we get a modern OpenGL context
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create a window with the desired dimensions and title
  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    cout << "Unable to create GLFW window" << endl;
    return nullptr;
  }

  return window;
}

static bool setupGLEW() {
  // Initialize GLEW, which automatically makes available any OpenGL
  // extensions supported on the system.
  //
  // glewInit must only be called after a GL context is made current.
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    cout << "GLEW could not be initialized." << endl;
    return false;
  }

  // Purge the GL_INVALID_ENUM which glewInit may cause.
  while (glGetError() != GL_NO_ERROR) {}

  return true;
}


// Keep a reference to the active app so that GLFW callbacks can access it.
static App* G_APP = nullptr;

// Processes ASCII keyboard input.
void keyboard_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods) {
  G_APP->OnKeyEvent(key, action, mods);
}

// A GLFW callback handling GLFW errors
void error_callback(int /*error*/, char const* description) {
  cerr << description << endl;
}

// Generates simulation window title text
std::string make_window_title(App const& app, int framerate) {
  std::stringstream builder;
  builder << "Warbird: " << app.state.entities.silos.at("ship").missiles
          << " | Unum: " << app.state.entities.silos.at("Unum").missiles
          << " | Secundus: " << app.state.entities.silos.at("Secundus").missiles
          << " | U/S: " << (1000.0 * app.GetTimeScaling()) / 40.0
          << " | F/S: " << framerate
          << " | " << CAMERAS[app.state.active_camera]
          << " | Gravity: " << (app.state.gravity_enabled ? "On" : "Off")
          << " | Thrust: " << (int)THRUSTS[app.state.active_thrust_factor]
          ;
  return builder.str();
}

// Entry point.
int main(int /*argc*/, char** /*argv*/) {
  // Initialize GLFW
  GLFWwindow* const window = setupGLFW(1024, 768, "Project Phase 1", &error_callback);
  if (!window) {
    cout << "Unable to initialize GLFW." << endl;
    return 1;
  }

  // Mark the OpenGL context as current. This is necessary for any gl* and glew* actions to apply to this window.
  glfwMakeContextCurrent(window);

  // Initialize GLEW.
  // Note that this has to happen AFTER a GL context is made current.
  if (!setupGLEW()) {
    cout << "GLEW could not be initialized." << endl;
    return 1;
  }

  // Set up our app object.
  // Note that this has to happen AFTER a GL context is made current.
  App app;

  RenderSystem renderSystem{
    window,

    // Transformation from camera space into clip space.
    // This gives a foreshortening effect, and maps all visible geometry into the volume of a unit cube.
    //
    // This camera can only see objects between 1 unit and 100,001 units away from it,
    // with a 75-degree field of view (along the Y axis). The 4/3 ratio determines the field of view
    // along the X axis, and serves to couple the viewing frustum to the (default) dimensions of the canvas.
    glm::perspective(glm::radians(75.0f), 4.0f / 3.0f, 1.0f, 100001.0f),
  };

  MissileSystem missileSystem{};

  {
    G_APP = &app;

    // Register for keyboard events on this window
    glfwSetKeyCallback(window, &keyboard_callback);

    // Notify the app object that a GL context has been acquired
    G_APP->OnAcquireContext(window);

    // Game Loop pattern
    // More information at http://gameprogrammingpatterns.com/game-loop.html
    // This particular game loop is modeled after one at http://gafferongames.com/game-physics/fix-your-timestep/
    {
      double const dt = 0.005;  // Fixed timestep for simulation evolution

      // Time elapsed (in seconds) since GLFW startup
      double currentTime = glfwGetTime();
      // Accumulates time as time passes. The simulator consumes this in discrete time quanta.
      double accumulator = 0.0;

      // Tracks our approximate FPS
      float prevFPS = 0;

      while (!glfwWindowShouldClose(window)) {
        double const newTime = glfwGetTime();
        double const delta = newTime - currentTime;
        currentTime = newTime;

        // Update simulation
        {
          // Accumulate the period of time which has passed since the last frame.
          // Apply a scalar factor to the difference to decouple the simulation's clock speed
          //   from the real world's clock speed.
          accumulator += G_APP->GetTimeScaling() * delta;

          // Run the simulation for as many time quanta as possible.
          while (accumulator >= dt) {
            accumulator -= dt;
            G_APP->OnTimeStep(dt);
            missileSystem.Update(G_APP->state, dt);
            // TODO: call siloSystem.Update();
          }
        }

        // Render simulation
        //
        // Note that some time may have been left unsimulated at this point.
        // We could do some fancy interpolation/extrapolation with the remainder,
        // but that's not terribly important here.
        renderSystem.Render(G_APP->state);

        // Interact with window
        {
          // Update the current FPS
          float const currentFPS = 1 / ((glfwGetTime() - newTime) + delta);
          prevFPS = 0.05*currentFPS + 0.95*prevFPS;

          // viewing window title update
          glfwSetWindowTitle(window, make_window_title(*G_APP, (int)prevFPS).c_str());
        }

        glfwPollEvents();

        // Relinquish the rest of our timeslice to other programs on this CPU.
        this_thread::yield();
      }
    }

    // Clean up after ourselves
    G_APP->OnReleaseContext();

    G_APP = nullptr;
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  return 0;
}
