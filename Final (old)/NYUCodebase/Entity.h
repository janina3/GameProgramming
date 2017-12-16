//
//  Entity.h
//  Final
//
//  Created by Janina Soriano on 11/30/17.
//  Copyright (c) 2017 Janina Soriano. All rights reserved.
//

#ifndef Entity_h
#define Entity_h

#include "ShaderProgram.h"
#include "Sheetsprite.h"
#include "Game.cpp"

using namespace std;

class Vector3
{
public:
    Vector3 (float x, float y, float z);
    Vector3(){};
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
};


class Entity
{
public:
    Entity(){};
    Vector3 position;
    Vector3 originalPosition;
    float rotation;
    GLuint textureID;
    string type;
    float width;
    float height;
    Vector3 velocity;
    Vector3 acceleration;
    SheetSprite sprite;
    float top;
    float bottom;
    float left;
    float right;
    Vector3 topRight;
    Vector3 topLeft;
    Vector3 bottomRight;
    Vector3 bottomLeft;
    void getTopBottomLeftRight()
    {
        height = sprite.size;
        width = sprite.size * sprite.aspect;
        top = position.y + height/2;
        bottom = position.y - height/2;
        left = position.x - width/2;
        right = position.x + width/2;
        topRight.x = right;
        topRight.y = top;
        topLeft.x = left;
        topLeft.y = top;
        bottomRight.x = right;
        bottomRight.y = bottom;
        bottomLeft.x = left;
        bottomLeft.y = bottom;
    }
    
    EnemyState enemyState;
    
    
    bool checkCollisions();
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    bool collidedTopRight;
    bool collidedTopLeft;
    bool collidedBottomRight;
    bool collidedBottomLeft;
    bool jump;
};


#endif