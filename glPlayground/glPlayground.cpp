// glPlayground.cpp : Defines the entry point for the console application.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "stdafx.h"

#include<iostream>
#include <string>
#include <thread>

using std::string;

void initWindow(GLFWwindow *&window) {
	glfwInit();
	//request a core opengl context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "glPlayground", nullptr, nullptr);
	glfwMakeContextCurrent(window);
}

int main() {
	// init glfw/glew
	GLFWwindow *window;
	initWindow(window);
	//init glew with modern opengl
	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(1.0, 0.0, 0.0, 1.0);

	// create vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLchar *vertexShaderSource =
		"#version 330 core\n\
    \n\
    layout (location = 0) in vec3 position;\n\
    \n\
    void main() {\n\
      gl_Position = vec4(position.x, position.y, position.z, 1.0);\n\
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
		"#version 330 core\n\
    \n\
    out vec4 color;\n\
    \n\
    void main() {\n\
      color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n\
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
		-0.5, -0.5, 0,
		0, 0.5, 0,
		0.5, -0.5, 0
	};

	GLuint indices[] = {
		0, 1, 2
	};

	// create vertex array object to store all gl state for drawing an object
	GLuint vao;
	//glCreateVertexArrays(1, &vao);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	//glCreateBuffers(1, &vbo);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint ebo;
	glCreateBuffers(1, &ebo);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, sizeof(vertices) / sizeof(GLfloat) / 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glfwSwapBuffers(window);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glfwTerminate();
	return 0;
}