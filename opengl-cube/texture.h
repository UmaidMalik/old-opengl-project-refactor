#pragma once

#include "gl_api.h"

class Texture {
public:
    explicit Texture(const char* path);
    ~Texture();
    Texture(Texture&& other) noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    GLuint getTextureID() const { return id; }

private:
    GLuint id = 0;
};
