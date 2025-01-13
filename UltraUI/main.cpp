#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <functional>

using namespace std;

// Константи вікна
const int WINDOW_WIDTH{ 900 };
const int WINDOW_HEIGHT{ 600 };
const char* WINDOW_TITLE{ "UltraUI Window" };

// Вершинний шейдер
const char* VertexShaderSource{ R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)" };

// Фрагментний шейдер
const char* FragmentShaderSource{ R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.4, 0.6, 1.0, 1.0);
    }
)" };

// 🔧 Функція для компіляції шейдера
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader{ glCreateShader(type) };
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }

    return shader;
}

// 🔧 Функція для створення шейдерної програми
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// 🎨 UI Елемент
class UIElement {
public:
    float x, y, width, height;
    function<void()> onClick;

    UIElement(float x, float y, float width, float height, function<void()> onClick)
        : x(x), y(y), width(width), height(height), onClick(onClick) {
    }

    virtual void draw() = 0;
    virtual void handleMouseClick(float mouseX, float mouseY) {
        if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height) {
            if (onClick) {
                onClick();
            }
        }
    }
};

// 🎨 Кнопка
class Button : public UIElement {
public:
    Button(float x, float y, float width, float height, function<void()> onClick)
        : UIElement(x, y, width, height, onClick) {
    }

    void draw() override {
        // Вершини прямокутника
        float vertices[] = {
            x, y,
            x + width, y,
            x + width, y + height,
            x, y + height
        };

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_QUADS, 0, 4);

        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

// 🖱️ Обробка кліків миші
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        mouseX = (mouseX / WINDOW_WIDTH) * 2.0f - 1.0f;
        mouseY = -((mouseY / WINDOW_HEIGHT) * 2.0f - 1.0f);

        // Обробити натискання мишею на UI-елементах
        // Для прикладу додамо кнопку
        static Button button1(-0.5f, -0.5f, 0.3f, 0.2f, []() {
            cout << "Button Clicked!" << endl;
            });
        button1.handleMouseClick(mouseX, mouseY);
    }
}

int main() {
    // Ініціалізація GLFW
    if (!glfwInit()) {
        cout << "Error: #1\n";
        return -1;
    }

    // Створення вікна
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        cout << "Error to create Window!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cout << "Error GLEW initialization\n";
        return -1;
    }

    // Реєстрація callback-функції для обробки кліків мишею
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Створення шейдерної програми
    unsigned int shaderProgram = createShaderProgram(VertexShaderSource, FragmentShaderSource);

    // Головний цикл
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Малюємо UI-елементи
        static Button button1(-0.5f, -0.5f, 0.3f, 0.2f, []() {
            cout << "Button Clicked!" << endl;
            });
        button1.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
