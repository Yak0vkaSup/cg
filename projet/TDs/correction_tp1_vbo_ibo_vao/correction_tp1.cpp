#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "common/GLShader.h"

struct vec2 { float x, y; };
struct vec3 { float x, y, z; };
struct vec4 { float x, y, z, w; };

struct Vertex
{
    vec2 position;
    vec3 color;
};

struct Application
{
    GLShader m_basicProgram;
    uint32_t m_VBO = 0;
    uint32_t m_IBO = 0;
    uint32_t m_VAO = 0;

    void Initialize()
    {
        m_basicProgram.LoadVertexShader("basic.vs.glsl");
        m_basicProgram.LoadFragmentShader("basic.fs.glsl");
        m_basicProgram.Create();
        uint32_t program = m_basicProgram.GetProgram();

        const std::vector<Vertex> triangles =
        {
            {{0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}},    // sommet 0
            {{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  // sommet 1
            {{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},   // sommet 2
        };
        //GLuint == uint32_t
        //GLushort == uint16_t
        const uint16_t indices[] = { 0, 1, 2 };

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*triangles.size(), triangles.data(), GL_STATIC_DRAW);
        glGenBuffers(1, &m_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)*3, indices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &m_VAO);        
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        // {[x, y], [r, g, b]}
        const int POSITION = glGetAttribLocation(program, "a_Position");
        const int COLOR = glGetAttribLocation(program, "a_Color");
        glVertexAttribPointer(POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex)/*stride*/, (void*)offsetof(Vertex, position));
        glVertexAttribPointer(COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex)/*stride*/, (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(POSITION);        
        glEnableVertexAttribArray(COLOR);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void Terminate()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_IBO);

        m_basicProgram.Destroy();
    }

    void Render()
    { 
        glClear(GL_COLOR_BUFFER_BIT);

        uint32_t program = m_basicProgram.GetProgram();
        glUseProgram(program);

        double time = glfwGetTime();
        int timeLocation = glGetUniformLocation(program, "u_Time");
        glUniform1f(timeLocation, static_cast<float>(time));       

        glBindVertexArray(m_VAO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO); // certains pilotes ne stocke pas l'IBO dans le VAO
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};



int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // ICI !
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        std::cout << "Erreur d'initialisation de GLEW" << std::endl;
    }

    Application app;
    app.Initialize();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {        
        /* Render here */
        app.Render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    app.Terminate();

    glfwTerminate();
    return 0;
}