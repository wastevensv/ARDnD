#include <GL/glew.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include <cstring>
#include <fstream>
#include <streambuf>

#include "glhelper.hpp"
#include "cvhelper.hpp"
#include "verts.hpp"
#include "shaders.h"

#define STATIC_IMAGES

float backdrop_vert[] = {
//  Position             Texture
     1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // Bottom Right
     1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // Top Right
     0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // Top Left
     
     1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // Bottom Right
     0.0f,  0.0f,  0.0f, 0.0f, 0.0f, // Bottom Left
     0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // Top Left
};

float axis_vertices[] = {
// Position              Color
     0.0f,  0.0f,  0.0f, 1.0f, 1.0f,  1.0f, // Origin
     1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, // X Line
    
     0.0f,  0.0f,  0.0f, 1.0f, 1.0f,  1.0f, // Origin
     0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, // Y Line
    
     0.0f,  0.0f,  0.0f, 1.0f, 1.0f,  1.0f, // Origin
     0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  1.0f, // Z Line
};

cv::Mat cameraFrame;
cv::VideoCapture stream1;

#ifdef STATIC_IMAGES
bool updated = true;
#endif

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS ){
      if(key == GLFW_KEY_W) { 
        char filename[] = "out.bmp";
        SOIL_save_screenshot
	(
		filename,
		SOIL_SAVE_TYPE_BMP,
		0, 0, 800, 600
	);
      }
    #ifdef STATIC_IMAGES
    } else if(key == GLFW_KEY_N) {
        // Capture Image
        stream1 >> cameraFrame;
        if( cameraFrame.empty() ) {
         exit(0);
        }
        updated = true;
    #endif
    }
}

int main()
{
    
    // --- OpenCV Init ---
    //0 is the id of video device. 0 if you have only one camera.
    #ifdef STATIC_IMAGES
    stream1 = VideoCapture("samples/%02d.jpg");
    #else
    stream1 = VideoCapture(1);
    #endif

    if (!stream1.isOpened()) { //check if video device has been initialised
            std::cerr << "cannot open camera" << std::endl;
    }
         
    // --- GLFW Init ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // --- Create window ---
    GLFWwindow* window = glfwCreateWindow(800, 600,
                                      "OpenGL", nullptr, nullptr); // Windowed
    if(window == NULL) return 2;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    
    // --- GLEW/GL Init ---
    glewExperimental = GL_TRUE;
    glewInit();

    // --- Application Specific Setup ---
    
    
    // Create FG Vertex Array Object
    GLuint bd_vao;
    glGenVertexArrays(1, &bd_vao);
    glBindVertexArray(bd_vao);
    
    // Create FG Vertex Buffer Object
    GLuint bd_vbo;
    glGenBuffers(1, &bd_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bd_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backdrop_vert),
                                  backdrop_vert, GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    
    // Create BG Vertex Array Object
    GLuint bb_vao;
    glGenVertexArrays(1, &bb_vao);
    glBindVertexArray(bb_vao);

    // Create BG Vertex Buffer Object
    GLuint bb_vbo;
    glGenBuffers(1, &bb_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bb_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backdrop_vert),
                                  backdrop_vert, GL_STATIC_DRAW);
    
    glBindVertexArray(0);
   
    // Create Axis Vertex Array Object
    GLuint axis_vao;
    glGenVertexArrays(1, &axis_vao);
    glBindVertexArray(axis_vao);

    // Create Axis Vertex Buffer Object
    GLuint axis_vbo;
    glGenBuffers(1, &axis_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
    
    vector<Vert<float, 8>> objVertices;
    int obj_length;
    loadOBJ("object.obj", objVertices);
    glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(Vert<float, 8>),
                                  &objVertices[0], GL_STATIC_DRAW);
    obj_length = objVertices.size();

    // Make object shader program.
    GLuint colorShaderProgram;
    makeShader(SHADER_VERT3D, SHADER_FRAG_SIMPLE, colorShaderProgram);
    
    glBindVertexArray(axis_vao);
    glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);

    GLint cvpAttrib = glGetAttribLocation(colorShaderProgram, "position");
    glEnableVertexAttribArray(cvpAttrib);
    glVertexAttribPointer(cvpAttrib, 3, GL_FLOAT, GL_FALSE,
                 8*sizeof(float), 0);

    GLint cvcAttrib = glGetAttribLocation(colorShaderProgram, "texcoord");
    glEnableVertexAttribArray(cvcAttrib);
    glVertexAttribPointer(cvcAttrib, 2, GL_FLOAT, GL_FALSE,
                 8*sizeof(float), (void*)(3*sizeof(float)));

    GLint cvnAttrib = glGetAttribLocation(colorShaderProgram, "normal");
    glEnableVertexAttribArray(cvnAttrib);
    glVertexAttribPointer(cvnAttrib, 3, GL_FLOAT, GL_FALSE,
                 8*sizeof(float), (void*)(5*sizeof(float)));
    
    // Make 'pretty' backdrop shader program.
    GLuint prettyShaderProgram;
    makeShader(SHADER_VERT3D, SHADER_FRAG_PRETTY, prettyShaderProgram);
    
    glBindVertexArray(bd_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bd_vbo);

    GLint pvpAttrib = glGetAttribLocation(prettyShaderProgram, "position");
    glEnableVertexAttribArray(pvpAttrib);
    glVertexAttribPointer(pvpAttrib, 3, GL_FLOAT, GL_FALSE,
               5*sizeof(float), 0);

    GLint pvtAttrib = glGetAttribLocation(prettyShaderProgram, "texcoord");
    glEnableVertexAttribArray(pvtAttrib);
    glVertexAttribPointer(pvtAttrib, 2, GL_FLOAT, GL_FALSE,
               5*sizeof(float), (void*)(3 * sizeof(float)));

    // Make simple backdrop shader program.
    GLuint blankShaderProgram;
    makeShader(SHADER_VERT3D, SHADER_FRAG_SIMPLE, blankShaderProgram);

    glBindVertexArray(bb_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bb_vbo);

    GLint bvpAttrib = glGetAttribLocation(blankShaderProgram, "position");
    glEnableVertexAttribArray(bvpAttrib);
    glVertexAttribPointer(bvpAttrib, 3, GL_FLOAT, GL_FALSE,
               5*sizeof(float), 0);

    GLint bvtAttrib = glGetAttribLocation(blankShaderProgram, "texcoord");
    glEnableVertexAttribArray(bvtAttrib);
    glVertexAttribPointer(bvtAttrib, 2, GL_FLOAT, GL_FALSE,
               5*sizeof(float), (void*)(3 * sizeof(float)));

    // Setup view
    glm::mat4 view = glm::lookAt(
                   glm::vec3( 0.5f,  2.0f,  0.5f),
                   glm::vec3( 0.5f,  0.0f,  0.5f),
                   glm::vec3( 0.0f,  0.0f,  1.0f)
                   );
    glm::mat4 proj = 
          glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    glm::mat4 model = glm::mat4();

    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "view" ),
                                            1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "proj" ),
                                            1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "model"),
                                            1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "view" ),
                                            1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "proj" ),
                                            1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "model"),
                                            1, GL_FALSE, glm::value_ptr(model));
    
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "view" ),
                                            1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "proj" ),
                                            1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "model"),
                                            1, GL_FALSE, glm::value_ptr(model));

    // Create texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);


    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    int frame_num = 0;
    // Capture Image
    stream1 >> cameraFrame;
    if( cameraFrame.empty() ) exit(-1);

    // --- Main Loop ---
    while(!glfwWindowShouldClose(window))
    {
        // Check for Keypress
        glfwPollEvents();

        #ifndef STATIC_IMAGES
        // Capture Image
        stream1 >> cameraFrame;
        if( cameraFrame.empty() ) break;
        #else
        if(!updated) continue;
        updated = false;
        #endif

        // Reset
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        // Move camera
        //GLfloat radius = 2.0f;
        //GLfloat camX = sin(glfwGetTime()) * radius;
        //GLfloat camY = cos(glfwGetTime()) * radius;
        //view = glm::lookAt(glm::vec3(camX,camY,1.5f),
        //        glm::vec3(0.0f, 0.0f, 0.0f),
        //        glm::vec3(0.0f, 0.0f, 1.0f));

        // Clone Image
        cv::Mat processedFrame = cameraFrame.clone();
        vector<vector<float>> poses;

        // Find objects and estimate poses.
        findObjects(processedFrame, poses, 0);
        cerr << frame_num <<"--------" << endl;
        for(int i = 0; i < poses.size(); i++) {
          cerr << i << ":" << poses[i][0] << ", \t" << poses[i][1] << endl;
          cerr <<     "  " << poses[i][2] << ", \t" << poses[i][3] << endl;
        }

        // Draw Baseboard
        model = glm::mat4();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                      processedFrame.cols, processedFrame.rows, 0, GL_BGR,
                      GL_UNSIGNED_BYTE, processedFrame.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glUseProgram(blankShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "view"),
          1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "proj"),
          1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "model"),
          1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(bd_vao);
          glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Draw Backboard
        model = glm::rotate(
                  model,
                  glm::radians(90.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f)
                );
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                      cameraFrame.cols, cameraFrame.rows, 0, GL_BGR,
                      GL_UNSIGNED_BYTE, cameraFrame.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glUseProgram(blankShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "view"),
          1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "proj"),
          1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "model"),
          1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(bb_vao);
          glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Draw Objects
        for(int i = 0; i < poses.size(); i++) {
          model = glm::mat4();
          model = glm::translate(model, glm::vec3(poses[i][0], poses[i][1], 0.0f));
          model = glm::rotate(model, poses[i][2], glm::vec3(0.0f, 0.0f, 1.0f));
          model = glm::scale(model, glm::vec3(poses[i][3]));

          glUseProgram(colorShaderProgram);
          glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "view"),
                  1, GL_FALSE, glm::value_ptr(view));
          glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "proj"),
                  1, GL_FALSE, glm::value_ptr(proj));
          glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "model"),
                  1, GL_FALSE, glm::value_ptr(model));

          glBindVertexArray(axis_vao);
            glDrawArrays(GL_TRIANGLES, 0, obj_length);
          glBindVertexArray(0);
        }
        
        //Display
        glfwSwapBuffers(window);
        frame_num++;
    }

    // --- Cleanup/Shutdown ---
    glDeleteProgram(colorShaderProgram);
    glDeleteProgram(prettyShaderProgram);
    glDeleteProgram(blankShaderProgram);
    
    glDeleteBuffers(1, &axis_vbo);
    glDeleteBuffers(1, &bd_vbo);
    glDeleteBuffers(1, &bb_vbo);

    glDeleteVertexArrays(1, &axis_vao);
    glDeleteVertexArrays(1, &bd_vao);
    glDeleteVertexArrays(1, &bb_vao);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
