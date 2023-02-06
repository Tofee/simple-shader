#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

#define LOGI(...)                                                              \
  ((void)printf(__VA_ARGS__))
#define LOGW(...)                                                              \
  ((void)printf(__VA_ARGS__))
#define LOGE(...)                                                              \
  ((void)printf(__VA_ARGS__))

// utilitaire : charger un fichier texte et renvoyer une chaine de caracteres.
std::string read( const char *filename )
{
    std::stringbuf source;
    std::ifstream in(filename);
    // verifie que le fichier existe
    if(in.good() == false)
        printf("[error] loading program '%s'...\n", filename);
    else
        printf("loading program '%s'...\n", filename);
    
    // lire le fichier, le caractere '\0' ne peut pas se trouver dans le source de shader
    in.get(source, 0);
    // renvoyer la chaine de caracteres
    return source.str();
}

class GLContext {
private:
  EGLContext context_ = EGL_NO_CONTEXT;
  EGLConfig config_;

  GLContext(GLContext const &);
  void operator=(GLContext const &);

public:
  GLContext() {}

  ~GLContext() {
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display, context_);
    }
    eglTerminate(display);
  }

  bool init() {
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_ALPHA_SIZE,
                              8,
                              EGL_NONE};

    EGLint num_configs;
    EGLConfig config;
    eglChooseConfig(display, attribs, &config, 1, &num_configs);

    if (!num_configs) {
      LOGE("Unable to retrieve EGL config");
      return false;
    }

    const EGLint context_attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3,
                                      EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE};

    context_ = eglCreateContext(display, config, nullptr, context_attribs);

    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context_) ==
        EGL_FALSE) {
      LOGE("Unable to eglMakeCurrent");
      return false;
    }

    return true;
  }
};

static bool compileShader(GLuint *shader, const GLenum type,
                          const GLchar *source, const int32_t iSize) {
  if (source == NULL || iSize <= 0)
    return false;

  *shader = glCreateShader(type);
  glShaderSource(*shader, 1, &source, &iSize);

  glCompileShader(*shader);

  GLint logLength;
  glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    GLchar *log = (GLchar *)malloc(logLength);
    glGetShaderInfoLog(*shader, logLength, &logLength, log);
    LOGI("Shader compile log:\n%s", log);
    free(log);
  }

  GLint status;
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
  if (status == 0) {
    glDeleteShader(*shader);
    LOGE("Failed to compile shader");
    return false;
  }

  return true;
}

static bool linkProgram(const GLuint prog) {
  GLint status;

  glLinkProgram(prog);

  GLint logLength;
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    GLchar *log = (GLchar *)malloc(logLength);
    glGetProgramInfoLog(prog, logLength, &logLength, log);
    LOGE("Program link log:\n%s", log);
    free(log);
  }

  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (status == 0) {
    LOGE("Program link failed\n");
    return false;
  }

  return true;
}

int main( int argc, char **argv ) 
{
   if(argc < 3) {
       LOGE("[error]: usage is: simple-shader <vertex.glsl> <fragment.glsl\n");
       return 1;
   }

  GLContext context;
  if (!context.init()) {
    return false;
  }

  // charger le source du vertex shader
  std::string vertexCode= read(argv[1]);
  std::string fragCode= read(argv[2]);

  LOGI("Compiling vertex shader...\n");
  GLuint vertextShader;
  if (!compileShader(&vertextShader, GL_VERTEX_SHADER, vertexCode.c_str(), vertexCode.size())) {
    return false;
  }

  LOGI("Compiling fragment shader...\n");
  GLuint fragShader;
  if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragCode.c_str(), fragCode.size())) {
    return false;
  }

  // Create shader program
  GLuint program = glCreateProgram();

  glAttachShader(program, vertextShader);
  glAttachShader(program, fragShader);

  bool ret = linkProgram(program);

  glDeleteShader(vertextShader);
  glDeleteShader(fragShader);
  glDeleteProgram(program);

  return ret;
}
