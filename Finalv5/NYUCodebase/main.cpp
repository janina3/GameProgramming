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
#include <vector>
#include <SDL_mixer.h>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#define RESOURCE "/Users/shayaansaiyed/Documents/School/Game/CS3113Homework/Platformer/NYUCodebase/"
#endif

#define FIXED_TIMESTAMP 0.0166666f
#define mapHeight 32
#define mapWidth 128
#define TILE_SIZE 0.16
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8

SDL_Window* displayWindow;

//ShaderProgram program;
unsigned int levelData[mapHeight][mapWidth];
int tileCount;

GLuint spriteSheetTexture;
GLuint georgeSpritesheet;
GLuint fontTexture;
GLuint spaceshipTexture;
GLuint georgeTexture;
//GLuint coinTexture;
//GLuint blobTexture;

Matrix projectionMatrix;
Matrix modelviewMatrix;

using namespace std;

//Read Header
//bool readHeader(std::ifstream &stream) {
//    string line;
//    mapWidth = -1;
//    mapHeight = -1;
//    while(getline(stream, line)) {
//        if(line == "") { break; }
//        istringstream sStream(line);
//        string key,value;
//        getline(sStream, key, '=');
//        getline(sStream, value);
//        if(key == "width") {
//            mapWidth = atoi(value.c_str());
//        } else if(key == "height"){
//            mapHeight = atoi(value.c_str());
//        } }
//    if(mapWidth == -1 || mapHeight == -1) {
//        return false;
//    } else { // allocate our map data
//        levelData = new int*[mapHeight];
//        for(int i = 0; i < mapHeight; ++i) {
//            levelData[i] = new int[mapWidth];
//        }
//        return true;
//    }
//}

//Load Texture
GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return retTexture;
}

//Read Layer Data
bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    int val = atoi(tile.c_str());
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                }
            } }
    }
    return true;
}

int tiles(unsigned int levelData[32][128])
{
    int counter = 0;
    for(int i = 0; i < mapHeight; i++)
    {
        for(int j = 0; j < mapWidth; i++)
        {
            if(levelData[i][j] != 0)
                counter++;
        }
    }
    
    return counter;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / (TILE_SIZE));
    *gridY = (int)( worldY / (TILE_SIZE));
}

//Solid Objects
vector<int> solidObjects = {1, 2, 3, 4, 16, 17, 18, 19, 32, 33, 34, 35};
bool isSolid(int pos)
{
    for(int i = 0; i < solidObjects.size(); i++)
    {
        if (pos == solidObjects[i])
            return true;
    }
    
    return false;
}

void DrawText(ShaderProgram program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
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
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, (int)text.length() * 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void drawTileMap(ShaderProgram* program){
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    float tileSize = 0.25;
    for(int y=0; y < mapHeight; y++) {
        for(int x=0; x < mapWidth; x++) {
            if (levelData[y][x] != 0){
                float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
                float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
                float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
                vertexData.insert(vertexData.end(), {
                    tileSize * x, -tileSize * y,
                    tileSize * x, (-tileSize * y)-tileSize,
                    (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                    tileSize * x, -tileSize * y,
                    (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                    (tileSize * x)+tileSize, -tileSize * y
                });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v+(spriteHeight),
                    u+spriteWidth, v+spriteHeight,
                    
                    u, v,
                    u+spriteWidth, v+spriteHeight,
                    u+spriteWidth, v
                });
            }
            
        }
    }
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


class Spritesheet {
public:
    Spritesheet(){};
    Spritesheet(unsigned int textureID, float u, float v, float width, float height, float size):textureID(textureID), u(u), v(v), width(width), height(height), size(size) {
        aspect = width/height;
    };
    Spritesheet(unsigned int textureID, int index, float size, int spriteCountX, int spriteCountY){
        
    };
    
    void Draw(ShaderProgram program){
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, -0.5f * size
        };
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);    }
    
    float aspect;
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};


//Lerp
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

//Vector3
class Vector3 {
public:
    Vector3 (float x, float y, float z);
    Vector3(){};
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
};

//Entity Class
class Entity{
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
    Spritesheet sprite;
    float top;
    float bottom;
    float left;
    float right;
    Vector3 topRight;
    Vector3 topLeft;
    Vector3 bottomRight;
    Vector3 bottomLeft;
    void getTopBottomLeftRight(){
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

class GameState
{
public:
    GameState(){}
    Entity player;
    Entity coin[10];
    Entity blob[5];
};

GameState state;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL1, STATE_GAME_LEVEL2, STATE_GAME_LEVEL3, STATE_GAME_OVER};
GameMode mode = STATE_MAIN_MENU;


//Player Actions
Matrix playerMM;
Matrix playerMVM;
Matrix coinMM[3];
Matrix coinMVM[3];
Matrix enemyMM[3];
Matrix enemyMVM[3];
int enemyIndex = 0;
int coinIndex = 0;
void placeEntity(const string& type, float placeX, float placeY){
    if (type == "player"){
        //initialize player
        Spritesheet playerSprite = Spritesheet(georgeTexture, 0, georgeSpritesheet);
        player.sprite = playerSprite;
        playerMM.Scale(0.5, 0.5, 0);
        player.size = Vector3(0.5, 0.5, 0);
        player.position = Vector3(placeX, placeY, 0);
        player.gravity = Vector3(0, -1, 0);
        player.velocity = Vector3(0, 0, 0);
        player.acceleration = Vector3(0, 0, 0);
        player.friction = Vector3(0.5, 0.5, 0);
        player.moving = false;
        state.player = player;
        
        playerMM.SetPosition(state.player.position.x, state.player.position.y, 0);
        playerMVM = viewMatrix * playerMM;
        viewMatrix.SetPosition(-state.player.position.x, -state.player.position.y, 0);
        
    }
    else if (type == "enemy"){
        Sprite enemySprite = Sprite(enemyTexture, 80, arneSpritesSpriteSheet);
        enemies[enemyIndex].sprite = enemySprite;
        enemyMM[enemyIndex].Scale(0.25, 0.25, 0);
        enemies[enemyIndex].size = Vector3(0.25, 0.25, 0);
        enemies[enemyIndex].position = Vector3(placeX, placeY, 0);
        enemies[enemyIndex].gravity = Vector3(0, -1, 0);
        enemies[enemyIndex].velocity = Vector3(0, 0, 0);
        enemies[enemyIndex].acceleration = Vector3(-1, 0, 0);
        enemies[enemyIndex].friction = Vector3(0.5, 0.5, 0);
        enemies[enemyIndex].collidedTop = false;
        enemies[enemyIndex].alive = true;
        state.enemies[enemyIndex] = enemies[enemyIndex];
        
        enemyMM[enemyIndex].SetPosition(state.enemies[enemyIndex].position.x, state.enemies[enemyIndex].position.y, 0);
        enemyMVM[enemyIndex] = enemyMM[enemyIndex] * viewMatrix;
        enemyIndex++;
    }
    else if (type == "collectible"){
        Sprite coinSprite = Sprite(coinTexture, 86, arneSpritesSpriteSheet);
        coins[coinIndex].sprite = coinSprite;
        coinMM[coinIndex].Scale(0.4, 0.4, 0);
        coins[coinIndex].size = Vector3(0.4, 0.4, 0);
        coins[coinIndex].position = Vector3(placeX, placeY, 0);
        coins[coinIndex].collected = false;
        state.coins[coinIndex] = coins[coinIndex];
        
        coinMM[coinIndex].SetPosition(state.coins[coinIndex].position.x, state.coins[coinIndex].position.y, 0);
        coinMVM[coinIndex] = coinMM[coinIndex] * viewMatrix;
        coinIndex++;
    }
    
    
}



//-------------VARIABLES-------------

//      STATE MANAGER
int state;

//      SETUP MATRIX


//      PHYSICS
float friction = 2.0;
float gravity = -2.0;

//      TIME
float ticks;
float lastFrameTicks = 0.0f;




//      GAMEPLAY
float health = 100;
int grammysCollected = 0;
bool drowning = 0;
bool GreenLock = true;


//  MUSIC
Mix_Music *arcade;
Mix_Chunk *collectcoin;




#define MAX_ENTITIES 1000
Entity Entities[MAX_ENTITIES];
int numOfEntities = 0;
int EntityIndex = 0;

//BlobAnimation
#define BLOB_FRAMES 2
#define CYCLE 20
int currentCycle = 0;
int currentBlobFram = 0;
Spritesheet blob1, blob2;
Spritesheet GSwitchLsprite, GSwitchULsprite;

void placeEntity(string type, float placeX, float placeY){
    if (EntityIndex >= MAX_ENTITIES){
        EntityIndex %= MAX_ENTITIES;
    }
    
    Entities[EntityIndex].position.x = -3.70 + placeX;
    Entities[EntityIndex].position.y = 2.0 + placeY;
    Entities[EntityIndex].originalPosition.x = -3.70 + placeX;
    Entities[EntityIndex].originalPosition.y = 2.0 + placeY;
    Entities[EntityIndex].type = type;
    
    if (type == "Ghost"){
        Entities[EntityIndex].sprite = Spritesheet(spriteSheetTexture, 26.0/SPRITE_COUNT_X, 14.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
    } else if (type == "LevelTwo"){
        Entities[EntityIndex].sprite = Spritesheet(spriteSheetTexture, 29.0/SPRITE_COUNT_X, 29.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 1.0);
    } else if (type == "LevelThree"){
        Entities[EntityIndex].sprite = Spritesheet(spriteSheetTexture, 29.0/SPRITE_COUNT_X, 29.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 1.0);
    } else if (type == "Grammy"){
        Entities[EntityIndex].sprite = Spritesheet(spriteSheetTexture, 2.0/SPRITE_COUNT_X, 2.0/SPRITE_COUNT_Y, 2.0/30.0, 2.0/30.0, 0.25);
    } else if (type == "Blob"){
        blob1 = Spritesheet(spriteSheetTexture, 21.0/SPRITE_COUNT_X, 15.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        blob2 = Spritesheet(spriteSheetTexture, 22.0/SPRITE_COUNT_X, 15.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        Entities[EntityIndex].velocity.x = 1.0;
        Entities[EntityIndex].sprite.size = 0.25;
        Entities[EntityIndex].height = 1.0/30.0;
        Entities[EntityIndex].width = 1.0/30.0;
        Entities[EntityIndex].sprite.aspect = 1.0;
    } else if (type == "GSwitch"){
        GSwitchLsprite = Spritesheet(spriteSheetTexture, 16.0/SPRITE_COUNT_X, 28.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        GSwitchULsprite = Spritesheet(spriteSheetTexture, 17.0/SPRITE_COUNT_X, 28.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        Entities[EntityIndex].sprite.size = 0.25;
        Entities[EntityIndex].sprite.aspect = 1.0;
    } else if (type == "GLock"){
        Entities[EntityIndex].sprite = Spritesheet(spriteSheetTexture, 15.0/SPRITE_COUNT_X, 6.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
    }
        
    EntityIndex++;
    numOfEntities++;
}

bool readEntityData(std::ifstream &stream) {
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
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



void createMap(){
    ifstream infile(RESOURCE_FOLDER"Level1.txt");
    string line;
    while (getline(infile, line)) {
        if(line == "[header]") {
//            if(!readHeader(infile)) {
//                return; }
        } else if(line == "[layer]") {
            readLayerData(infile);
        } else if(line == "[Object Layer 1]") {
            readEntityData(infile);
        }
    }
}



void renderMap(ShaderProgram program){
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int y=0; y < mapHeight; y++) {
        for(int x=0; x < mapWidth; x++) {
            
            float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
            float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
            float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
            float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
            vertexData.insert(vertexData.end(), {
                static_cast<float>(TILE_SIZE * x), static_cast<float>(-TILE_SIZE * y),
                static_cast<float>(TILE_SIZE * x), static_cast<float>((-TILE_SIZE * y)-TILE_SIZE),
                static_cast<float>((TILE_SIZE * x)+TILE_SIZE), static_cast<float>((-TILE_SIZE * y)-TILE_SIZE),
                static_cast<float>(TILE_SIZE * x), static_cast<float>(-TILE_SIZE * y),
                static_cast<float>((TILE_SIZE * x)+TILE_SIZE), static_cast<float>((-TILE_SIZE * y)-TILE_SIZE),
                static_cast<float>((TILE_SIZE * x)+TILE_SIZE), static_cast<float>(-TILE_SIZE * y)
            });
            texCoordData.insert(texCoordData.end(), {
                u, v,
                u, v+(spriteHeight),
                u+spriteWidth, v+(spriteHeight),
                u, v,
                u+spriteWidth, v+(spriteHeight),
                u+spriteWidth, v
            });
            
        }
    }
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, mapWidth*mapHeight*6);
}

//_____________FUNCTIONS____________________________
//      LOAD TEXTURES










bool collidesWithMap(Entity &ent) {
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
    if(levelData[ybottom][xbottom] != 108){
        ent.collidedBottom = true;
        cout << levelData[ybottom][xbottom] << endl;
    }
    
    //Top Collision
    worldToTileCoordinates(ent.position.x, ent.position.y + ent.height/2, &xtop, &ytop);
    if(levelData[ytop][xtop] != 108){
        ent.collidedTop = true;

    }
    
    //Left Collision
    worldToTileCoordinates(ent.position.x - ent.width/2, ent.position.y, &xleft, &yleft);
    if(levelData[yleft][xleft] != 108){
        ent.collidedLeft = true;
//        ent.position.x = ((float)(xleft * TILE_SIZE - 3.70) + ent.width/2) + TILE_SIZE;
    }
    
    //Right Collision
    worldToTileCoordinates(ent.position.x + ent.width/2, ent.position.y, &xright, &yright);
    if(levelData[yright][xright] != 108){
        ent.collidedRight = true;
//        ent.position.x = ((float)(xright * TILE_SIZE - 3.70) - ent.width/2);
    }
    return false;
}

void handleCollisionWithMap(Entity &ent){
    int x_tile, y_tile;
    if(ent.collidedTop){
        worldToTileCoordinates(ent.position.x, ent.top, &x_tile, &y_tile);
        ent.position.y = ((float)(2.0 - y_tile * TILE_SIZE) - ent.height/2) - TILE_SIZE;
        ent.velocity.y *= -1;
    }
    
    if(ent.collidedBottom){
        worldToTileCoordinates(ent.position.x, ent.bottom, &x_tile, &y_tile);
        ent.position.y = (float)(2.0 - y_tile * TILE_SIZE) + ent.height/2;
    }
    
    if(ent.collidedLeft){
        worldToTileCoordinates(ent.left, ent.position.y, &x_tile, &y_tile);
        ent.position.x = ((float)(x_tile * TILE_SIZE - 3.70) + ent.width/2) + TILE_SIZE;
    }
    
    if(ent.collidedRight){
        worldToTileCoordinates(ent.right, ent.position.y, &x_tile, &y_tile);
        ent.position.x = ((float)(x_tile * TILE_SIZE - 3.70) - ent.width/2);
    }
    
    
}


//_____________SETUP______________________
Entity SpaceshipPic;
Entity player;
float titleTextPosition;

//      Setup Player
void SetupPlayer(){
    player.sprite = Spritesheet(spriteSheetTexture, 7.0/SPRITE_COUNT_X, 7.0/SPRITE_COUNT_Y, 1.0/SPRITE_COUNT_X, 1.0/SPRITE_COUNT_Y, 0.25);
    player.position.x = -1.0;
    player.originalPosition.x = -1.0;
    player.type = player;
}


//      MAIN SETUP FUNCTION
ShaderProgram Setup(){
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glViewport(0, 0, 640, 480);
    
    glUseProgram(program.programID);
    
    projectionMatrix.SetOrthoProjection(-3.70, 3.70, -2.0f, 2.0f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetModelviewMatrix(modelviewMatrix);
    
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    arcade = Mix_LoadMUS(RESOURCE_FOLDER"arcade.wav");
    Mix_PlayMusic(arcade, -1);
    collectcoin = Mix_LoadWAV(RESOURCE_FOLDER"collectcoin.wav");
    
    fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    titleTextPosition = 10.0;
    
    spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
    spaceshipTexture = LoadTexture(RESOURCE_FOLDER"spaceship.png");
    SpaceshipPic.sprite = Spritesheet(spaceshipTexture, 0.0, 0.0, 1.2, 1.2, 1.0);
    SpaceshipPic.position.x = -10.0;
    

    createMap();
    SetupPlayer();
    return program;
}


//_____________PROCESS INPUT______________________
void ProcessGameInput(bool& done){
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_RIGHT]){
 //       if (player.collidedBottom){
            player.velocity.x += 0.1;
  //      }
    }

    if (keys[SDL_SCANCODE_LEFT]){
//        if (player.collidedBottom){
            player.velocity.x += -0.1;
//        }
    }
    
    if (keys[SDL_SCANCODE_UP]){
        if (player.collidedBottom){
            player.jump = true;
        }
    }
    if (keys[SDL_SCANCODE_DOWN]){
        if (!player.collidedBottom){
            player.velocity.y += -0.1;
        }
    }
    if (keys[SDL_SCANCODE_1]){
        player.position.y = player.originalPosition.y;
        player.position.x = player.originalPosition.x;
    }
    
    if (keys[SDL_SCANCODE_2]){
        player.position.y = -4.0;
        player.position.x = player.originalPosition.x;
    }
    if (keys[SDL_SCANCODE_3]){
        player.position.y = -9.0;
        player.position.x = player.originalPosition.x;
    }

}

void ProcessStartScreenInput(bool& done){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if(event.type == SDL_KEYDOWN) {
            
        }
    }
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_RETURN]){
        state = STATE_LEVEL_1;
    }
}

void ProcessInput(bool& done){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if(event.type == SDL_KEYDOWN) {
            
        }
    }
    if (state == STATE_LEVEL_1){
        ProcessGameInput(done);
    } else if (state == STATE_START_SCREEN){
        ProcessStartScreenInput(done);
    }
}


//_____________UPDATE_________________
void checkCollisionWithGhost(Entity &ghost){
    ghost.getTopBottomLeftRight();
    if (!((player.bottom >= ghost.top)
          || (player.top <= ghost.bottom)
          || (player.left >= ghost.right)
          || (player.right <= ghost.left)))
    {
        health -= 0.1;
        cout << health << endl;
    }
}

void checkLevelTwo(Entity &ent){
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

void checkLevelThree(Entity &ent){
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        cout << "got here";
        player.position.y = -9.0;
        //player.position.x = player.originalPosition.x;
    }
}

void checkCollisionWithGrammy(Entity &ent){
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        ent.position.x = 300;
        grammysCollected++;
        Mix_PlayChannel(-1, collectcoin, 0);
    }
}

void checkCollisionWithBlob(Entity &ent){
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        health -= 0.1;
    }
}

void checkCollisionWithGSwitch(Entity &ent){
    ent.getTopBottomLeftRight();
    if (!((player.bottom >= ent.top)
          || (player.top <= ent.bottom)
          || (player.left >= ent.right)
          || (player.right <= ent.left)))
    {
        GreenLock = false;
        Mix_PlayChannel(-1, collectcoin, 0);
    }
}



void checkPlayerCollisions(){
    for (int i = 0; i < numOfEntities; i++){
        if (Entities[i].type == "Ghost"){
            checkCollisionWithGhost(Entities[i]);
        } else if (Entities[i].type == "LevelTwo"){
            checkLevelTwo(Entities[i]);
        } else if (Entities[i].type == "LevelThree"){
            checkLevelThree(Entities[i]);
        } else if (Entities[i].type == "Grammy"){
            checkCollisionWithGrammy(Entities[i]);
        } else if (Entities[i].type == "Blob"){
            checkCollisionWithBlob(Entities[i]);
        } else if (Entities[i].type == "GSwitch"){
            checkCollisionWithGSwitch(Entities[i]);
        }
    }
}

void UpdatePlayer(float elapsed){
    
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
    
    player.velocity.y += gravity * elapsed;
    player.velocity.x = lerp(player.velocity.x, 0.0f, friction*elapsed);
    
    collidesWithMap(player);
    handleCollisionWithMap(player);

    checkPlayerCollisions();
    
    if(player.collidedBottom){
        player.velocity.y = 0.0;
    }
    
    if(player.jump){
        player.velocity.y = 2.0;
        player.jump = false;
    }
    
    player.position.x += player.velocity.x * elapsed;
    player.position.y += player.velocity.y * elapsed;
    if (health <= 0){
        state = STATE_LOST;
    }
    
    if (player.position.x > 10.0 && player.position.y < -8.0 && !GreenLock){
        state = STATE_WON;
    }
    
}

float getDistance(Vector3 v1, Vector3 v2){
    float x = v1.x - v2.x;
    float y = v1.y - v2.y;
    return (sqrtf(x*x + y*y));
}

void checkStatus(Entity &ghost){
    float dist = getDistance(ghost.position, player.position);
    if (dist <= 0.75){
        ghost.enemyState = ATTACKING;
    } else if (dist >= 1.5 && ghost.enemyState == ATTACKING){
        ghost.enemyState = RETURNING;
    } else if (ghost.position.x == ghost.originalPosition.x
               && ghost.position.y == ghost.originalPosition.y
               && ghost.enemyState == RETURNING){
        ghost.enemyState = IDLE;
    }
}

void UpdateGhosts_Idle(float elapsed, Entity &ghost){
    ghost.velocity.x = 0;
    ghost.velocity.y = 0;
}

void UpdateGhosts_Attacking(float elapsed, Entity &ghost){
    float x = player.position.x - ghost.position.x;
    float y = player.position.y - ghost.position.y;
    float dist = getDistance(player.position, ghost.position);
    if (dist != 0.0){
        x /= dist;
    }
    if (dist != 0.0){
        y /= dist;
    }
    ghost.velocity.x = x * 0.5;
    ghost.velocity.y = y * 0.5;
}

void UpdateGhosts_Returning(float elapsed, Entity &ghost){
    float x = ghost.originalPosition.x - ghost.position.x;
    float y = ghost.originalPosition.y - ghost.position.y;
    float dist = getDistance(ghost.originalPosition, ghost.position);
    if (dist != 0.0){
        x /= dist;
    }
    if (dist != 0.0){
        y /= dist;
    }
    ghost.velocity.x = x * 0.5;
    ghost.velocity.y = y * 0.5;
}

void UpdateGhosts( float elapsed){
    for(int i = 0; i < MAX_ENTITIES; i++){
        if (Entities[i].type == "Ghost"){
            checkStatus(Entities[i]);
            if (Entities[i].enemyState == IDLE){
                UpdateGhosts_Idle(elapsed, Entities[i]);
            } else if (Entities[i].enemyState == ATTACKING){
                UpdateGhosts_Attacking(elapsed, Entities[i]);
            } else if (Entities[i].enemyState == RETURNING){
                UpdateGhosts_Returning(elapsed, Entities[i]);
            }
            Entities[i].position.x += Entities[i].velocity.x * elapsed;
            Entities[i].position.y += Entities[i].velocity.y * elapsed;
        }
    }
}

Vector3 blobLeftFloorScanner;
Vector3 blobRightFloorScanner;
Vector3 blobLeftWallScanner;
Vector3 blobRightFLoorScanner;

void UpdateBlob(float elapsed){
    for (int i = 0; i < MAX_ENTITIES; i++){
        if (Entities[i].type == "Blob"){
            Entities[i].collidedLeft = false;
            Entities[i].collidedRight = false;
            Entities[i].collidedTop = false;
            Entities[i].collidedBottom = false;
            
            currentCycle++;
            currentCycle %= CYCLE;
            if (currentCycle == 19){
                currentBlobFram++;
                currentBlobFram %= BLOB_FRAMES;
            }
            
            Entities[i].velocity.y += gravity * elapsed;
            Entities[i].position.x += Entities[i].velocity.x * elapsed;
            Entities[i].position.y += Entities[i].velocity.y * elapsed;
            
            collidesWithMap(Entities[i]);
            handleCollisionWithMap(Entities[i]);
            
            if (Entities[i].collidedLeft || Entities[i].collidedRight){
                Entities[i].velocity.x *= -1.0;
            } if (Entities[i].collidedBottom){
                Entities[i].velocity.y = 0.0;
            }
        }
    }
    
}

void Update(float elapsed){
    if (state == STATE_LEVEL_1){
        UpdatePlayer(elapsed);
        UpdateGhosts(elapsed);
        UpdateBlob(elapsed);
    } else if (state == STATE_START_SCREEN){
        SpaceshipPic.position.x = lerp(SpaceshipPic.position.x, -2.0, 0.3);
        SpaceshipPic.position.y = lerp(SpaceshipPic.position.y, -0.5, 0.3);
    }
    
}


//_____________RENDER_________________

void RenderGameState(ShaderProgram program){
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(-3.7, 2.0, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    renderMap(program);
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(player.position.x, player.position.y, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    player.sprite.Draw(program);
    
    for (int i = 0; i < MAX_ENTITIES; i++){
        modelviewMatrix.Identity();
        modelviewMatrix.Translate(Entities[i].position.x, Entities[i].position.y, 0.0 );
        program.SetModelviewMatrix(modelviewMatrix);
        if (Entities[i].type == "Ghost"){
            Entities[i].sprite.Draw(program);
        }
        if (Entities[i].type == "LevelTwo"){
            modelviewMatrix.Scale(4.00, 2.00, 1.0);
            Entities[i].sprite.Draw(program);
        }
        if (Entities[i].type == "Grammy"){
            Entities[i].sprite.Draw(program);
        }
        if (Entities[i].type == "Blob"){
            if (currentBlobFram == 0){
                blob1.Draw(program);
            } else {
                blob2.Draw(program);
            }
        }
        if (Entities[i].type == "GSwitch"){
            if (GreenLock){
                GSwitchLsprite.Draw(program);
            } else {
                GSwitchULsprite.Draw(program);
            }
        }
        if (Entities[i].type == "GLock"){
            if (GreenLock){
                Entities[i].sprite.Draw(program);
            }
        }
    }
    
    modelviewMatrix.Identity();
    modelviewMatrix.Scale(2.0, 2.0, 2.0);
    modelviewMatrix.Translate(-player.position.x, -player.position.y, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(player.position.x + 1.0,player.position.y + 0.75, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    int h = health;
    DrawText(program, fontTexture, to_string(h), 0.5, -0.3);
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(player.position.x - 1.0,player.position.y + 0.75, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    DrawText(program, fontTexture, to_string(grammysCollected), 0.5, -0.3);
    

}

void RenderStartScreen(ShaderProgram program){
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(SpaceshipPic.position.x, SpaceshipPic.position.y, 0.0);
    modelviewMatrix.Scale(3.0, 4.5, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    SpaceshipPic.sprite.Draw(program);
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(0.0, 1.5, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    DrawText(program, fontTexture, "COIN", 0.75, -0.3);
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(0.0, 1.00, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    DrawText(program, fontTexture, "COLLECTOR", 0.75, -0.3);
}

void RenderGameOver(ShaderProgram program){
    modelviewMatrix.Identity();
    program.SetModelviewMatrix(modelviewMatrix);
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(-2.0, 0.0, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    DrawText(program, fontTexture, "GAME OVER", 0.75, -0.3);
}

void RenderWIN(ShaderProgram program){
    modelviewMatrix.Identity();
    program.SetModelviewMatrix(modelviewMatrix);
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(-2.0, 0.0, 0.0);
    program.SetModelviewMatrix(modelviewMatrix);
    DrawText(program, fontTexture, "YOU WIN", 0.75, -0.3);
}

void Render(ShaderProgram program){
    if (state == STATE_START_SCREEN){
        RenderStartScreen(program);
    } else if (state == STATE_LEVEL_1){
        RenderGameState(program);
    } else if (state == STATE_LOST){
        RenderGameOver(program);
    } else if (state == STATE_WON){
        RenderWIN(program);
    }
    
}



//___________________________________


int main(int argc, char *argv[])
{
    ShaderProgram program = Setup();

    bool done = false;
    while (!done) {
        
        //      ELAPSED TIME
        ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        ProcessInput(done);
        Update(elapsed);
        Render(program);
        
        SDL_GL_SwapWindow(displayWindow);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    SDL_Quit();
    return 0;
}
