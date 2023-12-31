// John Haviland, Owen Kroeger, Anthony O'Neal
// CST-310
// Project 6: Specular Lighting, Objects, Illumination and Shaders
// October 29, 2023

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct Mesh {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
};

float lightIntensity = 1.0f; // Initial light intensity

void renderMesh(const Mesh& mesh, unsigned int VAO, unsigned int VBO, unsigned int EBO, float intensity) {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::vec3), &mesh.vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

const char* vertexShaderSource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aNormal;

	out vec3 FragPos;
	out vec3 Normal;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main()
	{
    	gl_Position = projection * view * model * vec4(aPos, 1.0);
    	FragPos = vec3(model * vec4(aPos, 1.0));
    	Normal = mat3(transpose(inverse(model))) * aNormal;
	}
)";

const char* fragmentShaderSource = R"(
	#version 330 core
	in vec3 FragPos;
	in vec3 Normal;

	out vec4 FragColor;

	uniform vec3 lightPos;
	uniform vec3 viewPos;
	uniform vec3 objectColor;
	uniform vec3 lightColor;
	uniform float lightIntensity; // Added uniform for light intensity

	void main()
	{
	    vec3 ambient = 0.2 * lightColor * 0.5 * lightIntensity; // Apply light intensity here

	    vec3 norm = normalize(Normal);
	    vec3 lightDir = normalize(lightPos - FragPos); // Compute the light direction here
	    float diff = max(dot(norm, lightDir), 0.0);
	    vec3 diffuse = diff * lightColor * 0.5 * lightIntensity; // Apply light intensity here

	    vec3 viewDir = normalize(viewPos - FragPos);
	    vec3 reflectDir = reflect(-lightDir, norm);
	    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	    vec3 specular = spec * lightColor;

	    vec3 result = (ambient + diffuse + specular) * objectColor;

	    FragColor = vec4(result, 1.0);
	}
)";

// void function that allows the user to change the light intensity with keys 1-8
// '1' is the lowest light intensity, '8' is the highest
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
    	switch (key) {
        	case GLFW_KEY_1:
            	lightIntensity = 1.0f;
            	break;
        	case GLFW_KEY_2:
            	lightIntensity = 1.25f;
            	break;
        	case GLFW_KEY_3:
            	lightIntensity = 1.5f;
            	break;
        	case GLFW_KEY_4:
            	lightIntensity = 1.75f;
            	break;
        	case GLFW_KEY_5:
            	lightIntensity = 2.0f;
            	break;
        	case GLFW_KEY_6:
            	lightIntensity = 2.25f;
            	break;
        	case GLFW_KEY_7:
            	lightIntensity = 2.5f;
            	break;
        	case GLFW_KEY_8:
            	lightIntensity = 3.0f;
            	break;
    	}
	}
}

int main() {
	// Initialize GLFW and GLEW
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Create a window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Project 6: Specular Lighting, Objects, Illumination and Shaders", NULL, NULL);
	if (!window) {
    	glfwTerminate();
    	return -1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) return -1;

	glEnable(GL_DEPTH_TEST);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Set up cube vertices and indices
	float vertices[] = {
    	0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    	0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    	-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = {
    	0, 1, 3,
    	1, 2, 3
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//glm::vec3 lightPos(2.0f, 1.0f, 1.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	glm::vec3 viewPos(0.0f, 0.0f, 3.0f);  // Adjusted to view the front face directly
	glm::vec3 lightPos(0.0f, 0.0f, 2.0f); // Positioned closer to the front face

	int modelLoc, viewLoc, projectionLoc, lightPosLoc, viewPosLoc, objectColorLoc, lightColorLoc, lightIntensityLoc;

	glfwSetKeyCallback(window, key_callback); // Register key callback

	Mesh myMesh;

	// Define vertices and normals for a cube
	myMesh.vertices = {
	    // Front face
	    glm::vec3(-0.5f, -0.5f, 0.5f),
	    glm::vec3(0.5f, -0.5f, 0.5f),
	    glm::vec3(0.5f, 0.5f, 0.5f),
	    glm::vec3(-0.5f, 0.5f, 0.5f),

	    // Back face
	    glm::vec3(-0.5f, -0.5f, -0.5f),
	    glm::vec3(0.5f, -0.5f, -0.5f),
	    glm::vec3(0.5f, 0.5f, -0.5f),
	    glm::vec3(-0.5f, 0.5f, -0.5f),
	};

	myMesh.normals = {
	    // Front face
	    glm::vec3(0.0f, 0.0f, 1.0f),
	    glm::vec3(0.0f, 0.0f, 1.0f),
	    glm::vec3(0.0f, 0.0f, 1.0f),
	    glm::vec3(0.0f, 0.0f, 1.0f),

	    // Back face
	    glm::vec3(0.0f, 0.0f, -1.0f),
	    glm::vec3(0.0f, 0.0f, -1.0f),
	    glm::vec3(0.0f, 0.0f, -1.0f),
	    glm::vec3(0.0f, 0.0f, -1.0f),
	};

	// Define indices for the cube
	myMesh.indices = {
	    // Front face
	    0, 1, 2,
	    2, 3, 0,

	    // Right face
	    1, 5, 6,
	    6, 2, 1,

	    // Back face
	    5, 4, 7,
	    7, 6, 5,

	    // Left face
	    4, 0, 3,
	    3, 7, 4,

	    // Top face
	    3, 2, 6,
	    6, 7, 3,

	    // Bottom face
	    4, 5, 1,
	    1, 0, 4,
	};


	while (!glfwWindowShouldClose(window)) {
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	    glUseProgram(shaderProgram);

	    glm::mat4 model = glm::mat4(1.0f);
	    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate 30 degrees around the X axis
	    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate 30 degrees around the Y axis

	    glm::vec4 frontFaceOriginLocal = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
	    glm::vec3 frontFaceOriginWorld = glm::vec3(model * frontFaceOriginLocal);

	    glm::mat4 view = glm::lookAt(viewPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	    modelLoc = glGetUniformLocation(shaderProgram, "model");
	    viewLoc = glGetUniformLocation(shaderProgram, "view");
	    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	    lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	    viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
	    objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
	    lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	    lightIntensityLoc = glGetUniformLocation(shaderProgram, "lightIntensity");

	    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
	    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));
	    glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));
	    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	    glUniform1f(lightIntensityLoc, lightIntensity);

	    renderMesh(myMesh, VAO, VBO, EBO, lightIntensity);

	    glfwSwapBuffers(window);
	    glfwPollEvents();
}

	glfwTerminate();

	return 0;
}
