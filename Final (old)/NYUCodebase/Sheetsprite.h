//
//  Sheetsprite.h
//  Final
//
//  Created by Janina Soriano on 11/30/17.
//  Copyright (c) 2017 Janina Soriano. All rights reserved.
//

#ifndef Sheetsprite_h
#define Sheetsprite_h

#include "ShaderProgram.h"



class SheetSprite
{
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size):textureID(textureID), u(u), v(v), width(width), height(height), size(size)
    {
        aspect = width/height;
    };
    SheetSprite(unsigned int textureID, int index, float size, int spriteCountX, int spriteCountY)
    {
        
    };
    
    void Draw(ShaderProgram program)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] =
        {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        
        float vertices[] =
        {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size
        };
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    float aspect;
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};


#endif
