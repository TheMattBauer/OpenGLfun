#pragma once
#include <string>
#include <gl/glew.h>

class Texture
{
public:
    Texture(const std::string& fileName);
    void Bind(unsigned int unit);

    virtual ~Texture(void);
private:
    Texture(const Texture& other) {}
    Texture& operator=(const Texture& other) {}

    GLuint m_texture;
};

