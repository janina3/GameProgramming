//
//  main.cpp
//  Final
//
//  Created by Janina Soriano on 11/30/17.
//  Copyright (c) 2017 Janina Soriano. All rights reserved.
//


#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "Matrix.h"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cassert>
#include <vector>
#include <SDL_mixer.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define FIXED_TIMESTEP 0.02f
//#define mapHeight 64
//#define mapWidth 32
#define TILE_SIZE 0.25f



#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

using namespace std;

//Globals
enum GameState {STATE_START_SCREEN, LEVEL_1_STARTSCREEN, LEVEL_2_STARTSCREEN, LEVEL_3_STARTSCREEN, STATE_LEVEL_1, STATE_LEVEL_2, STATE_LEVEL_3, STATE_LOST, STATE_WON};
enum EntityType {PLAYER, ENEMY, BULLET};
enum EnemyState {IDLE, ATTACKING, RETURNING};

//State Manager
int state;

//Setup Matrix
Matrix projectionMatrix;
Matrix modelviewMatrix;

//Physics
float friction = 0.2;
float gravity = -0.2;


//Sprite Shet
int spriteSheetTexture;
int fontTexture;
//int spaceshipTexture;

//Gameplay
float health = 100;
int coinsCollected = 0;
bool drowning = 0;
bool GreenLock = true;

//Music
Mix_Music *arcade;
Mix_Chunk *collectcoin;


//Classes
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

//Map
int mapHeight;
int mapWidth;

int SPRITE_COUNT_Y = 30;
int SPRITE_COUNT_X = 30;
GLuint LoadTexture(const char *filePath);
int tileSheetTexture;
int** levelData;

bool readHeader(ifstream &stream)
{
    string line;
    mapWidth = -1;
    mapHeight = -1;
    while(getline(stream, line))
    {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width")
        {
            mapWidth = atoi(value.c_str());
        }
        else if(key == "height")
        {
            mapHeight = atoi(value.c_str());
        }
    }
    if(mapWidth == -1 || mapHeight == -1)
    {
        return false;
    }
    else
    {
        //allocate our map data
        levelData = new int*[mapHeight];
        for(int i = 0; i < mapHeight; ++i)
        {
            levelData[i] = new int[mapWidth];
        }
        return true;
    }
}

#define MAX_ENTITIES 1000
Entity Entities[MAX_ENTITIES];
int numOfEntities = 0;
int EntityIndex = 0;

//BlobAnimation
#define BLOB_FRAMES 2
#define CYCLE 20
int currentCycle = 0;
int currentBlobFram = 0;
SheetSprite blob1, blob2;




//Entity
void placeEntity(string type, float placeX, float placeY)
{
    if (EntityIndex >= MAX_ENTITIES)
    {
        EntityIndex %= MAX_ENTITIES;
    }
    
    Entities[EntityIndex].position.x = -3.70 + placeX;
    Entities[EntityIndex].position.y = 2.0 + placeY;
    Entities[EntityIndex].originalPosition.x = -3.70 + placeX;
    Entities[EntityIndex].originalPosition.y = 2.0 + placeY;
    Entities[EntityIndex].type = type;
    
    if (type == "Creature")
    {
        Entities[EntityIndex].sprite = SheetSprite(tileSheetTexture, 26.0/SPRITE_COUNT_X, 14.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
    }
    else if (type == "LevelTwo")
    {
        Entities[EntityIndex].sprite = SheetSprite(tileSheetTexture, 29.0/SPRITE_COUNT_X, 29.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 1.0);
    }
    else if (type == "LevelThree")
    {
        Entities[EntityIndex].sprite = SheetSprite(tileSheetTexture, 29.0/SPRITE_COUNT_X, 29.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 1.0);
    }
    else if (type == "Coin")
    {
        Entities[EntityIndex].sprite = SheetSprite(tileSheetTexture, 2.0/SPRITE_COUNT_X, 2.0/SPRITE_COUNT_Y, 2.0/30.0, 2.0/30.0, 0.25);
    }
    else if (type == "Blob")
    {
        blob1 = SheetSprite(tileSheetTexture, 21.0/SPRITE_COUNT_X, 15.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        blob2 = SheetSprite(tileSheetTexture, 22.0/SPRITE_COUNT_X, 15.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        Entities[EntityIndex].velocity.x = 1.0;
        Entities[EntityIndex].sprite.size = 0.25;
        Entities[EntityIndex].height = 1.0/30.0;
        Entities[EntityIndex].width = 1.0/30.0;
        Entities[EntityIndex].sprite.aspect = 1.0;
    }

    
    EntityIndex++;
    numOfEntities++;
}

bool readEntityData(ifstream &stream)
{
    string line;
    string type;
    
    while(getline(stream, line))
    {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type")
        {
            type = value;
        }
        else if(key == "location")
        {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str())*TILE_SIZE;
            float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
            placeEntity(type, placeX, placeY);
        }
    }
    return true;
}

bool readLayerData(std::ifstream &stream)
{
    string line;
    while(getline(stream, line))
    {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data")
        {
            for(int y=0; y < mapHeight; y++)
            {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++)
                {
                    getline(lineStream, tile, ',');
                    int val = atoi(tile.c_str());
                    if(val > 0)
                    {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    }
                    else
                    {
                        levelData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}

void createMap()
{
    ifstream infile(RESOURCE_FOLDER"level1.txt");
    string line;
    while (getline(infile, line))
    {
        if(line == "[header]")
        {
            if(!readHeader(infile))
            {
                return;
            }
        }
        else if(line == "[layer]")
        {
            readLayerData(infile);
        }
        else if(line == "[Object Layer 1]")
        {
            readEntityData(infile);
        }
    }
}



void renderMap(ShaderProgram program)
{
    vector<float> vertexData;
    vector<float> texCoordData;
    for(int y=0; y < mapHeight; y++)
    {
        for(int x=0; x < mapWidth; x++)
        {
            
            float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
            float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
            
            float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
            float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
            
            vertexData.insert(vertexData.end(),
            {
                TILE_SIZE * x, -TILE_SIZE * y,
                TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                TILE_SIZE * x, -TILE_SIZE * y,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
            });
            texCoordData.insert(texCoordData.end(),
            {
                u, v,
                u, v+(spriteHeight),
                u+spriteWidth, v+(spriteHeight),
                
                u, v,
                u+spriteWidth, v+(spriteHeight),
                u+spriteWidth, v
            });
            
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, tileSheetTexture);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, mapWidth*mapHeight*6);
}

//Functions
GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL)
    {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    stbi_image_free(image);
    return retTexture;
}

void DrawText(ShaderProgram& program, GLint fontTexture, string text, float size, float spacing)
{
    float texture_size = 1.0/16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    for(int i=0; i < text.size(); i++)
    {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(),
                            {
                                texture_x, texture_y,
                                texture_x, texture_y + texture_size,
                                texture_x + texture_size, texture_y,
                                texture_x + texture_size, texture_y + texture_size,
                                texture_x + texture_size, texture_y,
                                texture_x, texture_y + texture_size,
                            });
    }
    
    //glUseProgram(program.programID);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6 * (int)text.size());
}

//Lerp
float lerp(float v0, float v1, float t)
{
    return (1.0-t)*v0 + t*v1;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
    *gridX = (int)(worldX / (TILE_SIZE));
    *gridY = (int)(worldY / (TILE_SIZE));
}

bool collidesWithMap(Entity &ent)
{
    int xbottom;
    int ybottom;
    int xtop;
    int ytop;
    int xright;
    int yright;
    int xleft;
    int yleft;
    
    ent.getTopBottomLeftRight();
    
    //Bottom Collision
    worldToTileCoordinates(ent.position.x, ent.position.y - ent.height/2, &xbottom, &ybottom);
    if(levelData[ybottom][xbottom] != 13)
    {
        ent.collidedBottom = true;
        cout << levelData[ybottom][xbottom] << endl;
    }
    
    //Top Collision
    worldToTileCoordinates(ent.position.x, ent.position.y + ent.height/2, &xtop, &ytop);
    if(levelData[ytop][xtop] != 13)
    {
        ent.collidedTop = true;
        
    }
    
    //Left Collision
    worldToTileCoordinates(ent.position.x - ent.width/2, ent.position.y, &xleft, &yleft);
    if(levelData[yleft][xleft] != 13 )
    {
        ent.collidedLeft = true;
    }
    
    //Right Collision
    worldToTileCoordinates(ent.position.x + ent.width/2, ent.position.y, &xright, &yright);
    if(levelData[yright][xright] != 13 )
    {
        ent.collidedRight = true;
    }
    return false;
}

void handleCollisionWithMap(Entity &ent)
{
    int x_tile, y_tile;
    if(ent.collidedTop)
    {
        worldToTileCoordinates(ent.position.x, ent.top, &x_tile, &y_tile);
        ent.position.y = ((float)(2.0 - y_tile * TILE_SIZE) - ent.height/2) - TILE_SIZE;
        ent.velocity.y *= -1;
    }
    
    if(ent.collidedBottom)
    {
        worldToTileCoordinates(ent.position.x, ent.bottom, &x_tile, &y_tile);
        ent.position.y = (float)(2.0 - y_tile * TILE_SIZE) + ent.height/2;
    }
    
    if(ent.collidedLeft)
    {
        worldToTileCoordinates(ent.left, ent.position.y, &x_tile, &y_tile);
        ent.position.x = ((float)(x_tile * TILE_SIZE - 3.70) + ent.width/2) + TILE_SIZE;
    }
    
    if(ent.collidedRight)
    {
        worldToTileCoordinates(ent.right, ent.position.y, &x_tile, &y_tile);
        ent.position.x = ((float)(x_tile * TILE_SIZE - 3.70) - ent.width/2);
    }
    
    
}

//Particle
class Particle
{
public:
    Vector3 position;
    Vector3 velocity;
    float lifetime;
};


//Setup
//Entity SpaceshipPicture;
Entity player;

void SetupPlayer()
{
    player.sprite = SheetSprite(tileSheetTexture, 7.0/SPRITE_COUNT_X, 7.0/SPRITE_COUNT_Y, 1.0/SPRITE_COUNT_X, 1.0/SPRITE_COUNT_Y, 0.25);
    player.position.x = -1.0;
    player.originalPosition.x = -1.0;
    player.type = PLAYER;
}

//Update
void checkCollisionWithCreature(Entity &creature)
{
    creature.getTopBottomLeftRight();
    if (!((player.bottom >= creature.top)
          || (player.top <= creature.bottom)
          || (player.left >= creature.right)
          || (player.right <= creature.left)))
    {
        health -= 0.1;
        cout << health << endl;
    }
}

void checkLevelTwo(Entity &ent)
{
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        player.position.y = -4.0;
        player.position.x = player.originalPosition.x;
    }
}

void checkLevelThree(Entity &ent)
{
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        player.position.y = -9.0;
    }
}

void checkCollisionWithCoin(Entity &ent)
{
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        ent.position.x = 300;
        coinsCollected++;
        Mix_PlayChannel(-1, collectcoin, 0);
    }
}

void checkCollisionWithBlob(Entity &ent)
{
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        health -= 0.1;
    }
}

void checkPlayerCollisions()
{
    for (int i = 0; i < numOfEntities; i++)
    {
        if (Entities[i].type == "Creature")
        {
            checkCollisionWithCreature(Entities[i]);
        }
        else if (Entities[i].type == "LevelTwo")
        {
            checkLevelTwo(Entities[i]);
        }
        else if (Entities[i].type == "LevelThree")
        {
            checkLevelThree(Entities[i]);
        }
        else if (Entities[i].type == "Coin")
        {
            checkCollisionWithCoin(Entities[i]);
        }
        else if (Entities[i].type == "Blob")
        {
            checkCollisionWithBlob(Entities[i]);
        }
    }
}


float getDistance(Vector3 v1, Vector3 v2)
{
    float x = v1.x - v2.x;
    float y = v1.y - v2.y;
    return (sqrtf(x*x + y*y));
}


void checkStatus(Entity &creature)
{
    float dist = getDistance(creature.position, player.position);
    if (dist <= 0.75)
    {
        creature.enemyState = ATTACKING;
    }
    else if (dist >= 1.5 && creature.enemyState == ATTACKING)
    {
        creature.enemyState = RETURNING;
    }
    else if (creature.position.x == creature.originalPosition.x && creature.position.y == creature.originalPosition.y
             && creature.enemyState == RETURNING)
    {
        creature.enemyState = IDLE;
    }
}

void UpdateCreatures_Idle(float elapsed, Entity &creature)
{
    creature.velocity.x = 0;
    creature.velocity.y = 0;
}

void UpdateCreatures_Attacking(float elapsed, Entity &creature)
{
    float x = player.position.x - creature.position.x;
    float y = player.position.y - creature.position.y;
    float dist = getDistance(player.position, creature.position);
    if (dist != 0.0){
        x /= dist;
    }
    if (dist != 0.0){
        y /= dist;
    }
    creature.velocity.x = x * 0.5;
    creature.velocity.y = y * 0.5;
}

void UpdateCreatures_Returning(float elapsed, Entity &creature)
{
    float x = creature.originalPosition.x - creature.position.x;
    float y = creature.originalPosition.y - creature.position.y;
    float dist = getDistance(creature.originalPosition, creature.position);
    if (dist != 0.0){
        x /= dist;
    }
    if (dist != 0.0){
        y /= dist;
    }
    creature.velocity.x = x * 0.5;
    creature.velocity.y = y * 0.5;
}


int main(int argc, char *argv[])
{
    //START SETUP
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Spaceship", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glViewport(0, 0, 640, 360);
    
    glUseProgram(program.programID);
    
    projectionMatrix.SetOrthoProjection(-4.0, 4.0, -2.25f, 2.25f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetModelviewMatrix(modelviewMatrix);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    arcade = Mix_LoadMUS(RESOURCE_FOLDER"arcade.wav");
    Mix_PlayMusic(arcade, 1);
    collectcoin = Mix_LoadWAV(RESOURCE_FOLDER"collectcoin.wav");
    
    fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    
    tileSheetTexture = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
    //spaceshipTexture = LoadTexture(RESOURCE_FOLDER"spaceship.jpg");
    //SpaceshipPicture.sprite = SheetSprite(spaceshipTexture, 0.0, 0.0, 1.0, 1.0, 1.0);
    //SpaceshipPicture.position.x = -10.0;
    
    
    state = STATE_START_SCREEN;
    createMap();
    SetupPlayer();
    
    //END SETUP
    
    
    //Time Variables
    float lastFrameTicks = 0;
    float time = 0.0f;
    
    bool done = false;
    while (!done)
    {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += time;
        if(elapsed < FIXED_TIMESTEP)
        {
            time = elapsed;
        }
        
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
            else if(event.type == SDL_KEYDOWN)
            {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE)
                {
                    //do something when space is pressed
                }
            }
        }
        
        if (state == STATE_LEVEL_1)
        {
            
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_LEFT])
            {
               //go left@
                player.velocity.x += -0.1;
                
            }
            
            if (keys[SDL_SCANCODE_RIGHT])
            {
                //go right!
                player.velocity.x += 0.1;
            }

            
            if (keys[SDL_SCANCODE_UP])
            {
                if (player.collidedBottom)
                {
                    player.jump = true;
                }
            }
            if (keys[SDL_SCANCODE_DOWN])
            {
                if (!player.collidedBottom)
                {
                    player.velocity.y += -0.1;
                }
            }
            if (keys[SDL_SCANCODE_1])
            {
                player.position.y = player.originalPosition.y;
                player.position.x = player.originalPosition.x;
            }
            
            if (keys[SDL_SCANCODE_2])
            {
                player.position.y = -4.0;
                player.position.x = player.originalPosition.x;
            }
            if (keys[SDL_SCANCODE_3])
            {
                player.position.y = -9.0;
                player.position.x = player.originalPosition.x;
            }

        }
        
        else if (state == STATE_START_SCREEN)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
                {
                    done = true;
                }
                else if(event.type == SDL_KEYDOWN)
                {
                    done = false;
                }
            }
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_RETURN])
            {
                state = STATE_LEVEL_1;
            }
        }
        
        if (state == STATE_LEVEL_1)
        {
            
            //Player
            player.collidedBottom = false;
            player.collidedTop = false;
            player.collidedLeft = false;
            player.collidedRight = false;
            player.collidedTopLeft = false;
            player.collidedTopRight = false;
            player.collidedBottomLeft = false;
            player.collidedBottomRight = false;
            
            player.velocity.x += player.acceleration.x * elapsed;
            player.velocity.y += player.acceleration.y * elapsed;
            
            player.velocity.y += -gravity * elapsed;
            player.velocity.x = lerp(player.velocity.x, 0.0f, friction*elapsed);
            
            //collidesWithMap(player);
            //handleCollisionWithMap(player);
            
            checkPlayerCollisions();
            
            if(player.collidedBottom)
            {
                player.velocity.y = 0.0;
            }
            
            if(player.jump)
            {
                player.velocity.y = 2.0;
                player.jump = false;
            }
            
            player.position.x += player.velocity.x * elapsed;
            player.position.y += player.velocity.y * elapsed;
            if (health <= 0)
            {
                state = STATE_LOST;
            }
            
            if (player.position.x > 10.0 && player.position.y < -8.0 && !GreenLock)
            {
                state = STATE_WON;
            }

            //Creatures
            for(int i = 0; i < MAX_ENTITIES; i++)
            {
                if (Entities[i].type == "Creature")
                {
                    checkStatus(Entities[i]);
                    if (Entities[i].enemyState == IDLE){
                        UpdateCreatures_Idle(elapsed, Entities[i]);
                    }
                    else if (Entities[i].enemyState == ATTACKING)
                    {
                        UpdateCreatures_Attacking(elapsed, Entities[i]);
                    }
                    else if (Entities[i].enemyState == RETURNING)
                    {
                        UpdateCreatures_Returning(elapsed, Entities[i]);
                    }
                    Entities[i].position.x += Entities[i].velocity.x * elapsed;
                    Entities[i].position.y += Entities[i].velocity.y * elapsed;
                }
            }
            
            
            Vector3 blobLeftFloorScanner;
            Vector3 blobRightFloorScanner;
            Vector3 blobLeftWallScanner;
            Vector3 blobRightFLoorScanner;
            
            
            //UpdateBlob
            for (int i = 0; i < MAX_ENTITIES; i++)
            {
                if (Entities[i].type == "Blob")
                {
                    Entities[i].collidedLeft = false;
                    Entities[i].collidedRight = false;
                    Entities[i].collidedTop = false;
                    Entities[i].collidedBottom = false;
                    
                    currentCycle++;
                    currentCycle %= CYCLE;
                    if (currentCycle == 19)
                    {
                        currentBlobFram++;
                        currentBlobFram %= BLOB_FRAMES;
                    }
                    
                    Entities[i].velocity.y += gravity * elapsed;
                    Entities[i].position.x += Entities[i].velocity.x * elapsed;
                    Entities[i].position.y += Entities[i].velocity.y * elapsed;
                    
                    collidesWithMap(Entities[i]);
                    handleCollisionWithMap(Entities[i]);
                    
                    if (Entities[i].collidedLeft || Entities[i].collidedRight)
                    {
                        Entities[i].velocity.x *= -1.0;
                    }
                    if (Entities[i].collidedBottom)
                    {
                        Entities[i].velocity.y = 0.0;
                    }
                }
            }

        }
        
        else if (state == STATE_START_SCREEN)
        {
            //SpaceshipPicture.position.x = lerp(SpaceshipPicture.position.x, -2.0, 0.3);
            //SpaceshipPicture.position.y = lerp(SpaceshipPicture.position.y, -0.5, 0.3);
        }
        
        
        //Render
        //Start
        if (state == STATE_START_SCREEN)
        {
            //modelviewMatrix.Identity();
            //modelviewMatrix.Translate(SpaceshipPicture.position.x, SpaceshipPicture.position.y, 0.0);
            //modelviewMatrix.Scale(3.0, 4.5, 0.0);
            //program.SetModelviewMatrix(modelviewMatrix);
            
            modelviewMatrix.Identity();
            modelviewMatrix.Translate(0.0, 1.00, 0.0);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(program, fontTexture, "SPACE COINS", 0.75f, -0.3f);

        }
        
        //Game
        else if (state == STATE_LEVEL_1)
        {
            modelviewMatrix.Identity();
            modelviewMatrix.Translate(-3.7, 2.0, 0.0);
            program.SetModelviewMatrix(modelviewMatrix);
            renderMap(program);
            
            modelviewMatrix.Identity();
            modelviewMatrix.Translate(player.position.x, player.position.y, 0.0);
            program.SetModelviewMatrix(modelviewMatrix);
            player.sprite.Draw(program);
            
            for (int i = 0; i < MAX_ENTITIES; i++)
            {
                modelviewMatrix.Identity();
                modelviewMatrix.Translate(Entities[i].position.x, Entities[i].position.y, 0.0 );
                program.SetModelviewMatrix(modelviewMatrix);
                if (Entities[i].type == "Creature")
                {
                    Entities[i].sprite.Draw(program);
                }
                if (Entities[i].type == "LevelTwo")
                {
                    modelviewMatrix.Scale(4.00, 2.00, 1.0);
                    Entities[i].sprite.Draw(program);
                }
                if (Entities[i].type == "Coin")
                {
                    Entities[i].sprite.Draw(program);
                }
                if (Entities[i].type == "Blob")
                {
                    if (currentBlobFram == 0)
                    {
                        blob1.Draw(program);
                    }
                    else
                    {
                        blob2.Draw(program);
                    }
                }
        
            }
            
            modelviewMatrix.Identity();
            modelviewMatrix.Translate(player.position.x + 1.0,player.position.y + 0.75, 0.0);
            program.SetModelviewMatrix(modelviewMatrix);
            int h = health;
            DrawText(program, fontTexture, to_string(h), 0.5, -0.3);
            
            modelviewMatrix.Identity();
            modelviewMatrix.Translate(player.position.x - 1.0,player.position.y + 0.75, 0.0);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(program, fontTexture, to_string(coinsCollected), 0.5, -0.3);

        }
        
        //Lost
        else if (state == STATE_LOST)
        {
            modelviewMatrix.Identity();
            program.SetModelviewMatrix(modelviewMatrix);
            modelviewMatrix.Identity();
            modelviewMatrix.Translate(-2.0, 0.0, 0.0);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(program, fontTexture, "YOU LOST", 0.75, -0.3);
        }
        
        //Won
        else if (state == STATE_WON)
        {
            modelviewMatrix.Identity();
            program.SetModelviewMatrix(modelviewMatrix);
            modelviewMatrix.Translate(-2.0, 0.0, 0.0);
            modelviewMatrix.Identity();
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(program, fontTexture, "YOU WIN", 0.75, -0.3);
        }

        
        SDL_GL_SwapWindow(displayWindow);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    SDL_Quit();
    return 0;
}