#include <stdio.h>  // printf, fprintf, stderr, sprintf, FILE, fopen, fclose, fread
#include <math.h>   // sqrt

#include <glad/glad.h>
#include <glad/glad.c>

#include <GLFW/glfw3.h>


void error_callback(int error, const char* description);

// Helpers
void calculate_frame_timings();
char *read_entire_file(const char *filename);
int compile_shader(const char *file_path, GLuint shader_ID);
GLuint load_shaders(const char *vertex_file_path, const char *fragment_file_path);

int main()
{
    //
    glfwSetErrorCallback(error_callback);

    //
    if (!glfwInit()) {
        printf("Error initializing GLFW\n");
        return -1;
    }

    //
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //
    int resx = 640, resy = 480;
    GLFWwindow *window = glfwCreateWindow(resx, resy, "test", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -2;
    }

    //
    glfwMakeContextCurrent(window); 
    glfwSwapInterval(0);

    //
    if(!gladLoadGL()) {
        printf("Something went wrong while loading GLAD!\n");
        glfwTerminate();
        return -3;
    }
    
    //
    GLuint program = load_shaders("vertex_shader.vs", "fragment_shader.fs");
    if (program == 0) {
        fprintf(stderr, "Could not load shaders. Exiting\n");
        glfwTerminate();
        return -4;
    }

    //
    GLuint vertex_array_object;
    glGenVertexArrays(1, &vertex_array_object); // @NOTE: should destroy this at exit
    glBindVertexArray(vertex_array_object);

    //
    int texture_width = 4;
    int texture_height = 4;
    unsigned char texture_data[] = {
        255, 152,   0, // orange
        156,  39, 176, // purple
          3, 169, 244, // light blue
        139, 195,  74, // light green

        255,  87,  34, // deep orange
        103,  58, 183, // deep purple
          0, 188, 212, // cyan
        205, 220,  57, // lime

        244,  67,  54, // red
         63,  81, 181, // indigo
          0, 150, 137, // teal
        255, 235,  59, // yellow
        
        233,  30,  99, // pink
         33, 150, 243, // blue
         76, 175,  80, // green
        255, 193,   7, // amber
    };

    GLuint texture;
    glGenTextures(1, &texture); // @NOTE: should destroy this at exit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);

    //
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "texture_sampler"), 0);


    //
    int space_state_prev = 0;

    //
    glClearColor(3.0/255, 72/255.0, 133/255.0, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        //
        calculate_frame_timings(window);

        //
        glfwPollEvents();

        //
        if (glfwGetKey(window, GLFW_KEY_SPACE) && !space_state_prev) {
            glBindTexture(GL_TEXTURE_2D, texture);
            for (int i = 0; i < 3*texture_width*texture_width; i++) {
                texture_data[i] = 255 - texture_data[i];
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
            printf("Inverted texture colors\n");
        }
        space_state_prev = glfwGetKey(window, GLFW_KEY_SPACE);

        //
        glClear(GL_COLOR_BUFFER_BIT);
    
        //
        glUseProgram(program);
        glBindVertexArray(vertex_array_object);
        glBindTexture(GL_TEXTURE_2D, texture);

        //
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //
        glfwSwapBuffers(window);
    }

    //
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(program);
    glDeleteTextures(1, &texture);

    //
    glfwTerminate();
    
    return 0;
}



// Helpers
char *read_entire_file(const char *filename) {
    FILE *f = fopen(filename, "rb");

    if (f == NULL) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char*)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    string[fsize] = '\0';
    fclose(f);

    return string;
}

int compile_shader(const char * file_path, GLuint shader_ID) {
    char *shader_code = read_entire_file(file_path);
    if (shader_code == NULL) {
        fprintf(stderr, "Error: Could not read shader file: \"%s\"\n", file_path);
        return -1;
    }
    printf("Compiling shader : %s\n", file_path);
    glShaderSource(shader_ID, 1, (const char**)&shader_code , NULL);
    glCompileShader(shader_ID);

    GLint result;
    glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &result);

    if ( result == GL_FALSE ){
        GLint info_log_length;
        glGetShaderiv(shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);

        char shader_error_message[info_log_length+1];
        glGetShaderInfoLog(shader_ID, info_log_length, NULL, shader_error_message);
        fprintf(stderr, "Error while compiling shader \"%s\":\n%s", file_path, shader_error_message);

        free(shader_code);
        return -2;
    }

    free(shader_code);

    return 0;
}

GLuint load_shaders(const char * vertex_file_path,const char * fragment_file_path){
    GLuint vertex_shader_ID   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_ID = glCreateShader(GL_FRAGMENT_SHADER);

    int err1 = compile_shader(vertex_file_path, vertex_shader_ID);
    int err2 = compile_shader(fragment_file_path, fragment_shader_ID);

    if (err1 || err2) {
        glDeleteShader(vertex_shader_ID);
        glDeleteShader(fragment_shader_ID);
        return 0;
    }

    GLuint program_ID = glCreateProgram();
    glAttachShader(program_ID, vertex_shader_ID);
    glAttachShader(program_ID, fragment_shader_ID);
    glLinkProgram(program_ID);

    GLint result;
    glGetProgramiv(program_ID, GL_LINK_STATUS, &result);

    if ( result == GL_FALSE ){
        GLint info_log_length;
        glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar program_error_message[info_log_length+1];
        glGetProgramInfoLog(program_ID, info_log_length, NULL, program_error_message);
        printf("Error while linking program:\n%s\n", program_error_message);
        
        glDeleteShader(vertex_shader_ID);
        glDeleteShader(fragment_shader_ID);
        return 0;
    }

    glDeleteShader(vertex_shader_ID);
    glDeleteShader(fragment_shader_ID);

    return program_ID;
}

void calculate_frame_timings(GLFWwindow *window)
{
    static double t1 = 0.0;
    static double avg_dt = 0.0;
    static double avg_dt2 = 0.0;
    static int num_samples = 60;
    static int counter = 0;

    double t2 = glfwGetTime();
    double dt = t2-t1;
    t1 = t2;

    avg_dt += dt;
    avg_dt2 += dt*dt;
    counter++;

    if (counter == num_samples) {
        avg_dt  /= num_samples;
        avg_dt2 /= num_samples;
        double std_dt = sqrt(avg_dt2 - avg_dt*avg_dt);
        double ste_dt = std_dt/sqrt(num_samples);

        char window_title_string[128];
        sprintf(window_title_string, "dt: avg = %.3fms, std = %.3fms, ste = %.4fms. fps = %.1f", 1000.0*avg_dt, 1000.0*std_dt, 1000.0*ste_dt, 1.0/avg_dt);
        glfwSetWindowTitle(window, window_title_string);

        num_samples = 1.0/avg_dt;
        
        avg_dt = 0.0;
        avg_dt2 = 0.0;
        counter = 0;
    }
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s (%d)\n", description, error);
}