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

void renderMesh(const Mesh& mesh, unsigned int VAO, unsigned int VBO, unsigned int EBO) {
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

    void main()
    {
        vec3 ambient = 0.2 * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * objectColor;

        FragColor = vec4(result, 1.0);
    }
)";

int main() {
    // Initialize GLFW and GLEW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create eight windows
    GLFWwindow* windows[8];
    unsigned int shaderPrograms[8];

    for (int i = 0; i < 8; i++) {
        windows[i] = glfwCreateWindow(800, 600, "Mesh Renderer", NULL, NULL);
        if (!windows[i]) {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(windows[i]);

        if (glewInit() != GLEW_OK) return -1;

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderPrograms[i] = glCreateProgram();
        glAttachShader(shaderPrograms[i], vertexShader);
        glAttachShader(shaderPrograms[i], fragmentShader);
        glLinkProgram(shaderPrograms[i]);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

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

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);

    int modelLoc, viewLoc, projectionLoc, lightPosLoc, viewPosLoc, objectColorLoc, lightColorLoc;
    Mesh myMesh;
    myMesh.vertices = {
        glm::vec3(0.5f, 0.5f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.0f)
    };
    myMesh.normals = {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    myMesh.indices = {
        0, 1, 3,
        1, 2, 3
    };

    while (!glfwWindowShouldClose(windows[0])) {
        for (int i = 0; i < 8; i++) {
            glfwMakeContextCurrent(windows[i]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(shaderPrograms[i]);

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = glm::lookAt(viewPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

            modelLoc = glGetUniformLocation(shaderPrograms[i], "model");
            viewLoc = glGetUniformLocation(shaderPrograms[i], "view");
            projectionLoc = glGetUniformLocation(shaderPrograms[i], "projection");
            lightPosLoc = glGetUniformLocation(shaderPrograms[i], "lightPos");
            viewPosLoc = glGetUniformLocation(shaderPrograms[i], "viewPos");
            objectColorLoc = glGetUniformLocation(shaderPrograms[i], "objectColor");
            lightColorLoc = glGetUniformLocation(shaderPrograms[i], "lightColor");

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

            glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));
            glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

            renderMesh(myMesh, VAO, VBO, EBO);

            glfwSwapBuffers(windows[i]);
            glfwPollEvents();
        }
    }

    for (int i = 0; i < 8; i++) {
        glfwDestroyWindow(windows[i]);
    }

    glfwTerminate();

    return 0;
}