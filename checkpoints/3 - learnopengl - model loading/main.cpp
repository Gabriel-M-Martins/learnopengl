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
unsigned int generateCube();
glm::vec3 lightPos;
glm::vec3 lightColor;

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
	Shader shader("resources/shaders/model_loading.vert", "resources/shaders/model_loading.frag");
	Model backpackModel("resources/models/backpack/backpack.obj");
	Model robotModel("resources/models/drone_obj/drone.obj");

	// ---------------------------------------------------------------------------------------------- KEYS
	setupKeyMap(window);

	unsigned int cubeVAO = generateCube();
	lightPos = glm::vec3(0.5f, 0.5f, 1.0f);
	lightColor = glm::vec3(1.0f);
	
	Shader lightShader("resources/shaders/lamp.vert", "resources/shaders/lamp.frag");

	// ---------------------------------------------------------------------------------------------- RENDER LOOP
	while (!glfwWindowShouldClose(window)) {
		deltaTime = glfwGetTime() - currentTime;
		currentTime = glfwGetTime();

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ------------------------------------------------------------------------------------| Projection & Transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		// ------------------------------------------| Lights
		glm::mat4 lightModel = glm::mat4(1.0f);
		float radius = 3.0;

		lightModel = glm::translate(lightModel, lightPos);
		lightModel = glm::scale(lightModel, glm::vec3(0.2f));

		lightShader.use();
		lightShader.setMat("model", lightModel);
		lightShader.setMat("view", view);
		lightShader.setMat("projection", projection);
		lightShader.setFloat("color", lightColor);

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ------------------------------------------| Objects
		// ----------------------| Backpack
		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, glm::vec3(-1.0f, 0.0f, 0.0f));
		modelMat = glm::scale(modelMat, glm::vec3(0.5f));
		
		shader.use();
		shader.setMat("projection",		projection);
		shader.setMat("view",			view);
		shader.setMat("model",			modelMat);
		
		shader.setBool("flipUV", false);

		shader.setFloat("pointLights[0].position",	lightPos);
		shader.setFloat("pointLights[0].constant",	1.0f);
		shader.setFloat("pointLights[0].linear",	0.09f);
		shader.setFloat("pointLights[0].linear",	0.032f);

		shader.setFloat("pointLights[0].diffuse",	lightColor);
		shader.setFloat("pointLights[0].specular",	lightColor);
		shader.setFloat("pointLights[0].ambient",	lightColor * 0.2f);

		backpackModel.Draw(shader);

		// ----------------------| Robot
		modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMat = glm::scale(modelMat, glm::vec3(1.0f));

		shader.setBool("flipUV", true);
		shader.setMat("model", modelMat);
		robotModel.Draw(shader);

		// ------------------------------------------| Clean Up
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
	keymap[GLFW_KEY_C] = KeySettings{
		GLFW_KEY_C,
		[&] {
			lightColor = glm::vec3(1.0f);
		},
	};

	keymap[GLFW_KEY_R] = KeySettings{
		GLFW_KEY_R,
		[&] {
			lightColor = glm::vec3(1.0f, 0.0f, 0.0f);
		},
	};

	keymap[GLFW_KEY_G] = KeySettings{
		GLFW_KEY_G,
		[&] {
			lightColor = glm::vec3(0.0f, 1.0f, 0.0f);
		},
	};

	keymap[GLFW_KEY_B] = KeySettings{
		GLFW_KEY_B,
		[&] {
			lightColor = glm::vec3(0.0f, 0.0f, 1.0f);
		},
	};

	keymap[GLFW_KEY_UP] = KeySettings{
		GLFW_KEY_UP,
		[&] {
			lightPos.y += SPEED * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_DOWN] = KeySettings{
		GLFW_KEY_DOWN,
		[&] {
			lightPos.y -= SPEED * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_RIGHT] = KeySettings{
		GLFW_KEY_RIGHT,
		[&] {
			lightPos.x += SPEED * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_LEFT] = KeySettings{
		GLFW_KEY_LEFT,
		[&] {
			lightPos.x -= SPEED * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_HOME] = KeySettings{
		GLFW_KEY_HOME,
		[&] {
			lightPos.z += SPEED * deltaTime;
		},
		true,
		0.01f
	};

	keymap[GLFW_KEY_END] = KeySettings{
		GLFW_KEY_END,
		[&] {
			lightPos.z -= SPEED * deltaTime;
		},
		true,
		0.01f
	};

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
		}
	};

	keymap[GLFW_KEY_ESCAPE] = KeySettings{
		GLFW_KEY_ESCAPE,
		[&] {
			glfwSetWindowShouldClose(window, true);
		}
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

unsigned int generateCube()
{
	float vertices[] = {
		// positions
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	};

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	return VAO;
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
