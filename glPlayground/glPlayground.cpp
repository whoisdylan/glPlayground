// glPlayground.cpp : Defines the entry point for the console application.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "stdafx.h"

#include<iostream>
#include <string>
#include <thread>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using std::string;

void initWindow(GLFWwindow *&window, int width, int height) {
	glfwInit();
	//request a core opengl context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(width, height, "glPlayground", nullptr, nullptr);
	glfwMakeContextCurrent(window);
}

int main() {
	// init glfw/glew
	const int windowWidth = 800;
	const int windowHeight = 600;
	GLFWwindow *window;
	initWindow(window, windowWidth, windowHeight);
	//init glew with modern opengl
	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(0.75, 0.75, 0.75, 1.0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	// create vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLchar *vertexShaderSource =
		"#version 450 core\n\
    \n\
    layout (location = 0) in vec3 position;\n\
    layout (location = 1) in vec3 color;\n\
	\n\
	uniform mat4 tModelToWorld;\n\
	uniform mat4 tWorldToView;\n\
	uniform mat4 tViewToClip;\n\
	\n\
	out vec3 vertexColor;\n\
    \n\
    void main() {\n\
		gl_Position = tViewToClip * tWorldToView * tModelToWorld * vec4(position, 1.0);\n\
		vertexColor = color;\n\
    }";
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cout << "vertex shader compile failed: " << std::endl;
		std::cout << infoLog << std::endl;
	}

	// create fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar *fragmentShaderSource =
		"#version 450 core\n\
    \n\
		in vec3 vertexColor;\n\
    out vec4 color;\n\
    \n\
    void main() {\n\
      color = vec4(vertexColor, 1.0);\n\
    }";
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cout << "fragment shader compile failed: " << std::endl;
		std::cout << infoLog << std::endl;
	}

	// create shader program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "program linking failed: " << std::endl;
		std::cout << infoLog << std::endl;
	}

	GLfloat vertices[] = {
		-0.5, -0.5, 0.375,
		0, 0.5, 0.0,
		0.5, -0.5, 0.375,
		0, -0.5, -0.625
	};

	GLfloat colors[] = {
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 0.0, 1.0
	};

	GLuint indices[] = {
		0, 1, 2,
		2, 1, 3,
		3, 1, 0
	};

	// create vertex array object to store all gl state for drawing
	GLuint vao;
	//glCreateVertexArrays(1, &vao);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create vbo
	GLuint vboVertices;
	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);
	glEnableVertexAttribArray(0);

	// create vbo for triangle colors
	GLuint vboColors;
	glGenBuffers(1, &vboColors);
	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);
	glEnableVertexAttribArray(1);

	// create index ebo for the mesh
	GLuint ebo;
	//glCreateBuffers(1, &ebo);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0); //unbind vao
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind vboColors
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbind ebo

	// create view and project matrices
	glUseProgram(program); // bind shader program

	const GLint tModelToWorldULoc = glGetUniformLocation(program, "tModelToWorld");
	const GLint tWorldToViewULoc = glGetUniformLocation(program, "tWorldToView");
	const GLint tViewToClipULoc = glGetUniformLocation(program, "tViewToClip");

	// create camera view matrix
	glm::mat4 tWorldToView;
	tWorldToView = glm::translate(tWorldToView, glm::vec3(0.0, 0.0, -3.0));
	glUniformMatrix4fv(tWorldToViewULoc, 1, GL_FALSE, glm::value_ptr(tWorldToView));

	// create view frustum (projection) matrix
	const glm::mat4 tViewToClip = glm::perspective(glm::radians(45.0f), (float) windowWidth / (float) windowHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(tViewToClipULoc, 1, GL_FALSE, glm::value_ptr(tViewToClip));

	glUseProgram(0); // unbind shader program

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		glm::mat4 tModelToWorld;
		const double now = glfwGetTime();
		const float rotAngle = sin(now);
		tModelToWorld = glm::translate(tModelToWorld, glm::vec3(sin(now), 0.0, 0.0));
		tModelToWorld = glm::rotate(tModelToWorld, rotAngle, glm::vec3(0.0, 1.0, 0.0));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program);
		glBindVertexArray(vao);
		glUniformMatrix4fv(tModelToWorldULoc, 1, GL_FALSE, glm::value_ptr(tModelToWorld));
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glfwSwapBuffers(window);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vboVertices);
	glDeleteBuffers(1, &vboColors);
	glDeleteVertexArrays(1, &vao);
	glfwTerminate();
	return 0;
}