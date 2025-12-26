#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <map>
#include <functional>
#include "shader.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "keysettings.h"
#include "camera.h"
#include <sstream>
#include <string>
#include "model.h"
#include <filesystem>

// ---------------------------------------------------------------------------------------------- Window
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// ---------------------------------------------------------------------------------------------- Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraFOV = 45.0f;

float cameraSpeed = 2.5f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// ---------------------------------------------------------------------------------------------- Keymapping
std::set<int> keysPressed = {};
std::map<int, KeySettings> keymap = {};
void handleKey(GLFWwindow* window, KeySettings& key);
void setupKeyMap(GLFWwindow* window);

// ---------------------------------------------------------------------------------------------- Mouse
float mouseLastX = 0.0f;
float mouseLastY = 0.0f;
float mouseSensitivity = 0.15f;

bool isFirstMouseMovement = true;

// ---------------------------------------------------------------------------------------------- Time
float currentTime = 0.0f;
float deltaTime = 0.0f;

// ---------------------------------------------------------------------------------------------- Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// ---------------------------------------------------------------------------------------------- Utility
unsigned int loadTexture(const char* imagePath, const bool isPng = false);

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window." << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD." << std::endl;
		return -1;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	stbi_set_flip_vertically_on_load(true);

	mouseLastX = static_cast<float>(SCREEN_WIDTH) / 2;
	mouseLastY = static_cast<float>(SCREEN_HEIGHT) / 2;
	
	// ---------------------------------------------------------------------------------------------- CAMERA PROJECTION
	glm::mat4 view = glm::mat4(1.0f);
	view = camera.GetViewMatrix();

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

	glEnable(GL_DEPTH_TEST);

	// ---------------------------------------------------------------------------------------------- SHADER
	Shader shader("model_loading.vert", "model_loading.frag");
	Model model("backpack/backpack.obj");

	// ---------------------------------------------------------------------------------------------- KEYS
	setupKeyMap(window);

	// ---------------------------------------------------------------------------------------------- RENDER LOOP
	while (!glfwWindowShouldClose(window)) {
		deltaTime = glfwGetTime() - currentTime;
		currentTime = glfwGetTime();

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ------------------------------------------| Projection & Transformations
		shader.use();

		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		shader.setMat("projection", projection);
		shader.setMat("view", view);

		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.0f));
		modelMat = glm::scale(modelMat, glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setMat("model", modelMat);
		model.Draw(shader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void handleKey(GLFWwindow* window, KeySettings& key) {
	bool isPressed = glfwGetKey(window, key.key) == GLFW_PRESS;

	if (isPressed) {
		if (keysPressed.count(key.key) == 0) {
			keysPressed.insert(key.key);
			key.func();
		}
		else if (key.shouldRepeat) {
			if (currentTime - key.lastTriggerTime >= key.cooldown) {
				key.lastTriggerTime = currentTime;
				key.func();
			}
		}
	}
	else {
		keysPressed.erase(key.key);
	}
}

void setupKeyMap(GLFWwindow* window)
{
	keymap[GLFW_KEY_W] = KeySettings{
		GLFW_KEY_W,
		[&] {
			camera.ProcessKeyboard(FORWARD, deltaTime);
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_S] = KeySettings{
		GLFW_KEY_S,
		[&] {
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_A] = KeySettings{
		GLFW_KEY_A,
		[&] {
			camera.ProcessKeyboard(LEFT, deltaTime);
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_D] = KeySettings{
		GLFW_KEY_D,
		[&] {
			camera.ProcessKeyboard(RIGHT, deltaTime);
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_Q] = KeySettings{
		GLFW_KEY_Q,
		[&] {
			camera.Position.y += camera.MovementSpeed * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_E] = KeySettings{
		GLFW_KEY_E,
		[&] {
			camera.Position.y -= camera.MovementSpeed * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_T] = KeySettings{
		GLFW_KEY_T,
		[&] {
			GLint modes[2];
			glGetIntegerv(GL_POLYGON_MODE, modes);
			glPolygonMode(GL_FRONT_AND_BACK, modes[0] == GL_LINE ? GL_FILL : GL_LINE);
		},
		false
	};

	keymap[GLFW_KEY_ESCAPE] = KeySettings{
		GLFW_KEY_ESCAPE,
		[&] {
			glfwSetWindowShouldClose(window, true);
		},
		false
	};
}

void processInput(GLFWwindow* window) {
	for (auto& map : keymap) {
		handleKey(window, map.second);
	}
}

unsigned int loadTexture(const char* imagePath, const bool isPng) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, (isPng ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture." << std::endl;
	}
	stbi_image_free(data);

	return texture;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (isFirstMouseMovement) {
		mouseLastX = xpos;
		mouseLastY = ypos;
		isFirstMouseMovement = false;
	}

	float xoffset = xpos - mouseLastX;
	float yoffset = mouseLastY - ypos;
	mouseLastX = xpos;
	mouseLastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
