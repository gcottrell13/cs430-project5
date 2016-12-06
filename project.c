#define GLFW_DLL 1
#define GLFW_INCLUDE_ES2 1
#define GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include "deps/linmath.h"

#include "imageread.c"

#define pi 3.141592653589793238462643383279502
#define ANIMATION_LENGTH 1

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
	{{1, -1}, {0.9999, 0.9999}},
	{{1, 1},  {0.9999, 0}},
	{{-1, 1}, {0, 0}},
	{{-1,-1}, {0,0.9999}},
	{{1, -1}, {0.9999, 0.9999}},
	{{-1, 1}, {0, 0}}
};

float rotation = 0;
float animate_rotation_to = 0;
float animate_rotation_start_val = 0;
float animate_rotation_start_time = 0;
float animate_rotation_end_time = 0;

float scale = 1;
float animate_scale_to = 1;
float animate_scale_start_val = 0;
float animate_scale_start_time = 0;
float animate_scale_end_time = 0;

float x_pos = 0;
float animate_x_pos_to = 0;
float animate_x_pos_start_val = 0;
float animate_x_pos_start_time = 0;
float animate_x_pos_end_time = 0;

float y_pos = 0;
float animate_y_pos_to = 0;
float animate_y_pos_start_val = 0;
float animate_y_pos_start_time = 0;
float animate_y_pos_end_time = 0;

float shear = 0;
float animate_shear_to = 0;
float animate_shear_start_val = 0;
float animate_shear_start_time = 0;
float animate_shear_end_time = 0;

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute lowp vec2 TexCoordIn;\n"
"attribute lowp vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying lowp vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, 1);
		
		float now = (float) glfwGetTime();
		if(key == GLFW_KEY_A && now > animate_x_pos_end_time)
		{
			animate_x_pos_to -= 0.5;
			animate_x_pos_start_val = x_pos;
			animate_x_pos_start_time = now;
			animate_x_pos_end_time = now + ANIMATION_LENGTH;
		}
		if(key == GLFW_KEY_D && now > animate_x_pos_end_time)
		{
			animate_x_pos_to += 0.5;
			animate_x_pos_start_val = x_pos;
			animate_x_pos_start_time = now;
			animate_x_pos_end_time = now + ANIMATION_LENGTH;
		}
		if(key == GLFW_KEY_S && now > animate_y_pos_end_time)
		{
			animate_y_pos_to -= 0.5;
			animate_y_pos_start_val = y_pos;
			animate_y_pos_start_time = now;
			animate_y_pos_end_time = now + ANIMATION_LENGTH;
		}
		if(key == GLFW_KEY_W && now > animate_y_pos_end_time)
		{
			animate_y_pos_to += 0.5;
			animate_y_pos_start_val = y_pos;
			animate_y_pos_start_time = now;
			animate_y_pos_end_time = now + ANIMATION_LENGTH;
		}
		
		if(key == GLFW_KEY_Q && now > animate_rotation_end_time)
		{
			animate_rotation_to += pi / 3;
			animate_rotation_start_val = rotation;
			animate_rotation_start_time = now;
			animate_rotation_end_time = now + ANIMATION_LENGTH;
		}
		if(key == GLFW_KEY_E && now > animate_rotation_end_time)
		{
			animate_rotation_to -= pi / 3;
			animate_rotation_start_val = rotation;
			animate_rotation_start_time = now;
			animate_rotation_end_time = now + ANIMATION_LENGTH;
		}
		
		if(key == GLFW_KEY_R && now > animate_scale_end_time)
		{
			animate_scale_to *= 2;
			animate_scale_start_val = scale;
			animate_scale_start_time = now;
			animate_scale_end_time = now + ANIMATION_LENGTH;
		}
		if(key == GLFW_KEY_F && now > animate_scale_end_time)
		{
			animate_scale_to *= 0.5;
			animate_scale_start_val = scale;
			animate_scale_start_time = now;
			animate_scale_end_time = now + ANIMATION_LENGTH;
		}
		
		if(key == GLFW_KEY_X && now > animate_shear_end_time)
		{
			animate_shear_to += 1;
			animate_shear_start_val = shear;
			animate_shear_start_time = now;
			animate_shear_end_time = now + ANIMATION_LENGTH;
		}
		if(key == GLFW_KEY_Z && now > animate_shear_end_time)
		{
			animate_shear_to -= 1;
			animate_shear_start_val = shear;
			animate_shear_start_time = now;
			animate_shear_end_time = now + ANIMATION_LENGTH;
		}
	}
}

float calculate_animation(float start_val, float end_val, float start_time, float end_time)
{
	float now = (float) glfwGetTime();
	if(now >= end_time) return end_val;
	return (end_val - start_val) * (now - start_time) / 
			(end_time - start_time) + start_val;
}

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader,
    GL_COMPILE_STATUS,
    &compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader,
      GL_INFO_LOG_LENGTH,
      &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}


int main(int argc, char** argv)
{
  if(argc < 2)
  {
    fprintf(stderr, "Usage: input.ppm\n");
    exit(EXIT_FAILURE);
  }

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    // more error checking! glLinkProgramOrDie!

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*) (sizeof(float) * 2));
    
    PPMmeta image_info = LoadPPM(argv[1]);

    int image_width = image_info.width;
    int image_height = image_info.height;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, 
     GL_UNSIGNED_BYTE, image_info.data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
		
		float now = (float) glfwGetTime();
		if(now < animate_x_pos_end_time)
			x_pos = calculate_animation(animate_x_pos_start_val, animate_x_pos_to, animate_x_pos_start_time, animate_x_pos_end_time);
		if(now < animate_y_pos_end_time)
			y_pos = calculate_animation(animate_y_pos_start_val, animate_y_pos_to, animate_y_pos_start_time, animate_y_pos_end_time);
		if(now < animate_rotation_end_time)
			rotation = calculate_animation(animate_rotation_start_val, animate_rotation_to, animate_rotation_start_time, animate_rotation_end_time);
		if(now < animate_scale_end_time)
			scale = calculate_animation(animate_scale_start_val, animate_scale_to, animate_scale_start_time, animate_scale_end_time);
		if(now < animate_shear_end_time)
			shear = calculate_animation(animate_shear_start_val, animate_shear_to, animate_shear_start_time, animate_shear_end_time);
		
        mat4x4_identity(m);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_translate_in_place(m, x_pos, y_pos, 0);
		mat4x4_linear_scale(m, scale, scale, 1);
		mat4x4_linear_shear_xy(m, 0, shear);
        mat4x4_rotate_Z(m, m, rotation);
        mat4x4_mul(mvp, p, m);
		
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawArrays(GL_TRIANGLES, 3, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]