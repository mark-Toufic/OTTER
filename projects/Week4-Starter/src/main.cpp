#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "Camera.h"

#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
	case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
#endif
	default: break;
	}
}

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(800, 800);
// The title of our GLFW window
std::string windowTitle = "INFR-1350U";

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

int main() {
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	static const GLfloat points[] = {
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f
	};

	static const GLfloat colors[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	//VBO - Vertex buffer object
	VertexBuffer::Sptr posVbo = VertexBuffer::Create();
	posVbo->LoadData(points, 9);

	VertexBuffer::Sptr color_vbo = VertexBuffer::Create();
	color_vbo->LoadData(colors, 9);

	VertexArrayObject::Sptr vao = VertexArrayObject::Create();
	vao->AddVertexBuffer(posVbo, {
		BufferAttribute(0, 3, AttributeType::Float, 0, NULL)
		});
	vao->AddVertexBuffer(color_vbo, {
		{ 1, 3, AttributeType::Float, 0, NULL }
		});









	static const float interleaved[] = {
		// X      Y    Z       R     G     B
		 0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,   0.3f, 0.2f, 0.5f,
		-0.5f,  0.5f, 0.5f,   1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,   1.0f, 1.0f, 1.0f
	};
	VertexBuffer::Sptr interleaved_vbo = VertexBuffer::Create();
	interleaved_vbo->LoadData(interleaved, 6 * 4);

	static const uint16_t indices[] = {
		3, 0, 1,
		3, 1, 2
	};
	IndexBuffer::Sptr interleaved_ibo = IndexBuffer::Create();
	interleaved_ibo->LoadData(indices, 3 * 2);

	size_t stride = sizeof(float) * 6;
	VertexArrayObject::Sptr vao2 = VertexArrayObject::Create();
	vao2->AddVertexBuffer(interleaved_vbo, {
		BufferAttribute(0, 3, AttributeType::Float, stride, 0),
		BufferAttribute(1, 3, AttributeType::Float, stride, sizeof(float) * 3),
		});
	vao2->SetIndexBuffer(interleaved_ibo);



	static const float interleaved1[] = {
			// X      Y    Z       R     G     B
			 0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.5f,   0.3f, 5.2f, 0.5f,
			-0.5f,  0.5f, 0.5f,   5.0f, 6.0f, 3.0f,
			-0.5f, -0.5f, 0.5f,   4.0f, 1.0f, 1.0f
		};
		VertexBuffer::Sptr interleaved_vbo1 = VertexBuffer::Create();
		interleaved_vbo1->LoadData(interleaved1, 6 * 4);

		static const uint16_t indices1[] = {
			3, 0, 1,
			3, 1, 2
		};
		IndexBuffer::Sptr interleaved_ibo1 = IndexBuffer::Create();
		interleaved_ibo1->LoadData(indices1, 3 * 2);

		size_t stride1 = sizeof(float) * 6;
		VertexArrayObject::Sptr vao3 = VertexArrayObject::Create();
		vao3->AddVertexBuffer(interleaved_vbo1, {
			BufferAttribute(0, 3, AttributeType::Float, stride1, 0),
			BufferAttribute(1, 3, AttributeType::Float, stride1, sizeof(float) * 3),
			});
		vao3->SetIndexBuffer(interleaved_ibo1);





	// Load our shaders
	Shader::Sptr shader = Shader::Create();
	shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", ShaderPartType::Vertex);
	shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", ShaderPartType::Fragment);
	shader->Link();

	// GL states
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Get uniform location for the model view projection
		// Create a mat4 to store our mvp (for now)
	glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 transform2 = glm::mat4(1.0f);
	glm::mat4 transform3 = glm::mat4(1.0f);


	Camera::Sptr camera = Camera::Create();
	camera->SetPosition(glm::vec3(0, 1, 1));
	camera->LookAt(glm::vec3(0.0f));

	// Our high-precision timer
	double lastFrame = glfwGetTime();




	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// Rotate our models around the z axis
		transform = glm::rotate(glm::mat4(1.f), static_cast<float>(thisFrame), glm::vec3(0, 0, 1));
		transform2 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.0f, glm::sin(static_cast<float>(thisFrame))));
		transform3 = glm::translate(glm::mat4(1.0f), glm::vec3(glm::sin(static_cast<float>(thisFrame)), 0.0f, 0.0f));

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind our shader and upload the uniform
		
		shader->Bind();
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform);

		vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		vao->Unbind();
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* transform2);
		
		vao2->Bind();
		glDrawElements(GL_TRIANGLES, interleaved_ibo->GetElementCount(), (GLenum)interleaved_ibo->GetElementType(), nullptr);
		vao2->Unbind();

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform3);
		vao3->Bind();
		glDrawElements(GL_TRIANGLES, interleaved_ibo1->GetElementCount(), (GLenum)interleaved_ibo1->GetElementType(), nullptr);

		VertexArrayObject::Unbind();

		glfwSwapBuffers(window);
	}

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}