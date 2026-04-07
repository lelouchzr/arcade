#include "OpenGl.hpp"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <algorithm>
#include <stdexcept>

namespace arcade::bonus {
OpenGl::~OpenGl() {
    close();
}

void OpenGl::open() {
    if (context_)
        return;
    try {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        host_.open("ARCADE / 400 - OpenGL", SDL_WINDOW_OPENGL);
        context_ = SDL_GL_CreateContext(host_.window());
        if (!context_)
            throw std::runtime_error(SDL_GetError());
        SDL_GL_MakeCurrent(host_.window(), context_);
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Frame::Width, Frame::Height, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, nullptr);
        if (glGetError() != GL_NO_ERROR)
            throw std::runtime_error("OpenGL texture allocation failed");
    } catch (...) {
        close();
        throw;
    }
}

void OpenGl::close() noexcept {
    if (context_) {
        SDL_GL_MakeCurrent(host_.window(), context_);
        if (texture_)
            glDeleteTextures(1, &texture_);
        SDL_GL_DeleteContext(context_);
    }
    texture_ = 0;
    context_ = nullptr;
    host_.close();
}

std::vector<Input> OpenGl::poll() {
    return host_.poll();
}

void OpenGl::draw(const Frame &frame) {
    if (!context_)
        throw std::runtime_error("OpenGL display is closed");
    SDL_GL_MakeCurrent(host_.window(), context_);
    canvas_.render(frame);
    int width = 0, height = 0;
    SDL_GL_GetDrawableSize(host_.window(), &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(10.f / 255, 15.f / 255, 23.f / 255, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    const double scale = std::min(static_cast<double>(width) / Frame::Width,
                                  static_cast<double>(height) / Frame::Height);
    const int w = static_cast<int>(Frame::Width * scale),
              h = static_cast<int>(Frame::Height * scale);
    glViewport((width - w) / 2, (height - h) / 2, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Frame::Width, Frame::Height, GL_BGRA, GL_UNSIGNED_BYTE,
                    canvas_.pixels().data());
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1, 1);
    glTexCoord2f(1, 0);
    glVertex2f(1, 1);
    glTexCoord2f(1, 1);
    glVertex2f(1, -1);
    glTexCoord2f(0, 1);
    glVertex2f(-1, -1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    if (glGetError() != GL_NO_ERROR)
        throw std::runtime_error("OpenGL frame upload failed");
    SDL_GL_SwapWindow(host_.window());
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::OpenGl, arcade::PluginKind::Graphics, "opengl",
              "OPENGL / TEXTURED CANVAS")
