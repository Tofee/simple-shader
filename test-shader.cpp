#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
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

  void dumpConfig(EGLDisplay display, EGLConfig *config) {
     if(!config) return;
     
    EGLint value;
     
     eglGetConfigAttrib(display,*config,EGL_BUFFER_SIZE,&value);
     std::cout<<"Buffer Size "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_RED_SIZE,&value);
     std::cout<<"Red Size "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_GREEN_SIZE,&value);
     std::cout<<"Green Size "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_BLUE_SIZE,&value);
     std::cout<<"Blue Size "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_ALPHA_SIZE,&value);
     std::cout<<"Alpha Size "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_CONFIG_CAVEAT,&value);
     switch(value)
     {
         case  EGL_NONE : std::cout<<"EGL_CONFIG_CAVEAT EGL_NONE\n"; break;
         case  EGL_SLOW_CONFIG : std::cout<<"EGL_CONFIG_CAVEAT EGL_SLOW_CONFIG\n"; break;
     }
     eglGetConfigAttrib(display,*config,EGL_CONFIG_ID,&value);
     std::cout<<"Config ID "<<value<<"\n";

     // EGL_RENDERABLE_TYPE is a bit field
     eglGetConfigAttrib(display,*config,EGL_RENDERABLE_TYPE,&value);
     std::cout<<"Renderable Type: ";
     if (( value & EGL_OPENGL_ES2_BIT ) == EGL_OPENGL_ES2_BIT ) std::cout<<"EGL_OPENGL_ES2_BIT ";
     if (( value & EGL_OPENGL_ES3_BIT_KHR ) == EGL_OPENGL_ES3_BIT_KHR ) std::cout<<"EGL_OPENGL_ES3_BIT_KHR ";
     std::cout<<"\n";
     
     eglGetConfigAttrib(display,*config,EGL_DEPTH_SIZE,&value);
     std::cout<<"Depth size "<<value<<"\n";

     eglGetConfigAttrib(display,*config,EGL_MAX_PBUFFER_WIDTH,&value);
     std::cout<<"Max pbuffer width "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_MAX_PBUFFER_HEIGHT,&value);
     std::cout<<"Max pbuffer height "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_MAX_PBUFFER_PIXELS,&value);
     std::cout<<"Max pbuffer pixels "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_NATIVE_RENDERABLE,&value);
     std::cout<<"Native renderable "<<std::string(value ? "true" : "false")<<"\n";
     eglGetConfigAttrib(display,*config,EGL_NATIVE_VISUAL_ID,&value);
     std::cout<<"Native visual ID "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_NATIVE_VISUAL_TYPE,&value);
     std::cout<<"Native visual type "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_SAMPLE_BUFFERS,&value);
     std::cout<<"Sample Buffers "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_SAMPLES,&value);
     std::cout<<"Samples "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_SURFACE_TYPE,&value);
     std::cout<<"Surface type "<<value<<"\n";
     eglGetConfigAttrib(display,*config,EGL_TRANSPARENT_TYPE,&value);
     std::cout<<"--------------------------------------------------------------------------\n";
  }

  bool init() {
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

    if (getenv("SHOW_CONFIGS"))
    {
        EGLBoolean result;
        EGLint numConfigs;
        
        // first we call getConfigs with a NULL to see how many configs we have
        result=eglGetConfigs(display,NULL,0,&numConfigs);
        std::cout<< "number of configs found "<<numConfigs<<"\n";
        // now we create a buffer to store all our configs
        EGLConfig *configs = new EGLConfig[numConfigs];
        // and copy them into our buffer (don't forget to delete once done)
        result=eglGetConfigs(display,configs,numConfigs,&numConfigs);

        std::cout<<"--------------------------------------------------------------------------\n";
        for(int i=0; i<numConfigs; ++i)
        {
            std::cout<<"Config #"<<i<<"\n";
            dumpConfig(display, &configs[i]);
        }
        // now clear up the configs now we have done
        delete [] configs;
    }
    
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT_KHR | EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES_BIT,
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
    else {
        dumpConfig(display, &config);
    }

    const EGLint surfaceAttributes[] = {
            EGL_WIDTH, 16,
            EGL_HEIGHT, 16,
            EGL_NONE
    };

    // This is just a dummy surface that it needed to make an OpenGL context current (bind it to this thread)
    EGLSurface dummySurfaceForPlugin = eglCreatePbufferSurface(display, config, surfaceAttributes);
    if (dummySurfaceForPlugin == EGL_NO_SURFACE) {
      LOGE("Unable to create pbuffer surface !");
    }

    const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3,
                                      EGL_NONE, EGL_NONE,
                                      EGL_NONE};

    context_ = eglCreateContext(display, config, nullptr, context_attribs);

    if (eglMakeCurrent(display, dummySurfaceForPlugin, dummySurfaceForPlugin, context_) ==
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
