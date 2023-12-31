#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

// Ball object struct
struct BallPosition {
    float x, y;
};

// variable initialization
std::vector<BallPosition> ballTrail;
const int trailLength = 20; // Adjust as needed

const float PI = 3.14159265359;
const int num_segments = 100;

float rotationAngleX = 80.0f;
float rotationAngleY = 0.0f;

float basketX = 0.0f;
float basketSpeed = 0.02f;

float ballX = 0.0f;
float ballY = -0.9f;
float ballSpeed = 0.05f;
float ballArc = 1.5f;
bool ballShot = false;

float netX = 0.0f;
float netY = 0.8f;
float netWidth = 0.2f;
float netHeight = 0.2f;
bool ballInNet = false;

int score = 0;
int highScore = 0;

// window size
const int windowWidth = 1900;
const int windowHeight = 1080;

// clock start
std::chrono::time_point<std::chrono::steady_clock> startTime;

// shoot ball with space bar
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !ballShot) {
        ballShot = true;
    }
}

// function to handle game logic
void updateGameLogic() {

    // flip basket if at edge
    basketX += basketSpeed;
    if (basketX >= 2.0f || basketX <= -2.0f) {
        basketSpeed = -basketSpeed;
    }

    // ball logic handling
    if (ballShot) {
        ballX += ballSpeed * cos(ballArc);
        ballY += ballSpeed * sin(ballArc);

        // if scored, increment score counter and reset ball
        if (fabs(ballX - basketX) < 0.1f && fabs(ballY - 1.0f) < 0.1f) {
            score++;
            ballShot = false;
            ballX = 0.0f;
            ballY = -0.9f;
        }

        // if not scored, reset ball
        if (ballX > 2.0f || ballX < -2.0f || ballY > 2.0f || ballY < -2.0f) {
            ballShot = false;
            ballX = 0.0f;
            ballY = -0.9f;
        }

        // Update ball trail
        ballTrail.insert(ballTrail.begin(), {ballX, ballY});
        if (ballTrail.size() > trailLength) {
            ballTrail.pop_back();
        }
    } 
    
    else {
        ballTrail.clear();
    }

    if (ballInNet) {
        netY -= 0.01f;
        if (netY < -1.0f) {
            netY = 0.8f;
            ballInNet = false;
        }
    }

    // Implement the time tracking logic
    static bool gameStarted = false;
    if (!gameStarted && ballShot) {
        startTime = std::chrono::steady_clock::now();
        gameStarted = true;
    }

    // start game timer
    if (gameStarted) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedTime >= 60) {
            glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
        }
    }

    // register score
    if (ballShot) {
        if (ballX >= netX - netWidth / 2 && ballX <= netX + netWidth / 2 &&
            ballY <= netY && ballY >= netY - netHeight) {
            ballInNet = true;
        }
    }
}

// draw rim
void draw3DCircle(float radius, float lineWidth) {
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < num_segments; ++i) {
        float theta = 2.0f * PI * float(i) / float(num_segments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        float z = 0.28f;
        glVertex3f(x, y, z);
    }
    glEnd();
}

// draw the ball
void drawBall(float x, float y, float radius) {
    const int num_segments = 100;
    const float PI = 3.14159265359;

    // outline
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.0f, 0.0f, 0.0f); // Black outline
    glVertex2f(x, y); // Center of the circle
    for (int i = 0; i <= num_segments; ++i) {
        float theta = 2.0f * PI * float(i) / float(num_segments);
        float dx = radius * cosf(theta);
        float dy = radius * sinf(theta);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();

    // inside
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 0.647f, 0.0f); // Orange color for the ball
    glVertex2f(x, y);
    for (int i = 0; i <= num_segments; ++i) {
        float theta = 2.0f * PI * float(i) / float(num_segments);
        float dx = (radius - 0.02f) * cosf(theta);
        float dy = (radius - 0.02f) * sinf(theta);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

// draw trail by keeping track of previous ball potitions
void drawBallTrail() {
    float trailRadius = 0.05f;
    for (size_t i = 0; i < ballTrail.size(); ++i) {
        float alpha = 1.0f - std::pow((float)i / ballTrail.size(), 5); 
        glColor4f(1.0f, 0.647f, 0.0f, alpha); 
        drawBall(ballTrail[i].x, ballTrail[i].y, trailRadius * (1.0f - (float)i / ballTrail.size()));
    }
}

// save high score to file
void saveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    } else {
        std::cerr << "Unable to save high score to file!" << std::endl;
    }
}

// load high score from file
void loadHighScore() {
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    } else {
        std::cerr << "Unable to load high score from file! Initializing high score to zero." << std::endl;
        highScore = 0;
    }
}

// update high score
void updateHighScore(int currentScore) {
    if (currentScore > highScore) {
        highScore = currentScore;
        saveHighScore();
    }
}


int main() {

    // initialize glfw, gl, glu
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Basketball Flash Game", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);


    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    loadHighScore();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw environment
    while (!glfwWindowShouldClose(window)) {
        updateGameLogic();

        // Set the background color to light beige
        glClearColor(0.937f, 0.882f, 0.788f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
	
        // Draw the border for the backboard
        glBegin(GL_QUADS);
        glColor3f(0.3f, 0.3f, 0.3f); // Dark grey color for the border
        glVertex2f(basketX - 0.525f, 1.425f); // Slightly larger than the backboard, but smaller border
        glVertex2f(basketX + 0.525f, 1.425f);
        glVertex2f(basketX + 0.525f, 0.775f);
        glVertex2f(basketX - 0.525f, 0.775f);
        glEnd();

        // Draw the backboard with a muted blue color
        glBegin(GL_QUADS);
        glColor3f(0.1f, 0.2f, 0.8f); // Sharper blue color
        glVertex2f(basketX - 0.5f, 1.4f); // Original size of the backboard
        glVertex2f(basketX + 0.5f, 1.4f);
        glVertex2f(basketX + 0.5f, 0.8f);
        glVertex2f(basketX - 0.5f, 0.8f);
        glEnd();

        glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f); // Red color for the outline
        glVertex2f(basketX - 0.12f, 1.0f); // Top-left vertex
        glVertex2f(basketX + 0.12f, 1.0f); // Top-right vertex
        glVertex2f(basketX + 0.12f, 0.8f); // Bottom-right vertex
        glVertex2f(basketX - 0.12f, 0.8f); // Bottom-left vertex
        glEnd();

        glBegin(GL_QUADS);
        glColor3f(0.1f, 0.2f, 0.8f); // Sharper blue color
        glVertex2f(basketX - 0.1f, 0.98f); // Top-left vertex
        glVertex2f(basketX + 0.1f, 0.98f); // Top-right vertex
        glVertex2f(basketX + 0.1f, 0.82f); // Bottom-right vertex
        glVertex2f(basketX - 0.1f, 0.82f); // Bottom-left vertex
        glEnd();
        
        glColor3f(1.0f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(basketX, 1.0f, 0.0f);
        glRotatef(rotationAngleX, 1.0f, 0.0f, 0.0f);
        draw3DCircle(0.2f, 2.0f);
        glPopMatrix();

	    drawBallTrail(); // Draw the trail
        drawBall(ballX, ballY, 0.05f);

        // drawNet();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    updateHighScore(score);

    // Display score and high score after the window closes
    std::cout << "Your score: " << score << std::endl;
    std::cout << "High score: " << highScore << std::endl;

    glfwTerminate();
    return 0;
}
