//
//  Entity.cpp
//  Final
//
//  Created by Janina Soriano on 11/30/17.
//  Copyright (c) 2017 Janina Soriano. All rights reserved.
//

#include "Entity.h"
#include "../Sheetsprite/Sheetsprite.h"
#include "../Game/Game.cpp"

using namespace std;

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
SheetSprite GSwitchLsprite, GSwitchULsprite;

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
    else if (type == "GSwitch")
    {
        GSwitchLsprite = SheetSprite(tileSheetTexture, 16.0/SPRITE_COUNT_X, 28.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        GSwitchULsprite = SheetSprite(tileSheetTexture, 17.0/SPRITE_COUNT_X, 28.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
        Entities[EntityIndex].sprite.size = 0.25;
        Entities[EntityIndex].sprite.aspect = 1.0;
    }
    else if (type == "GLock")
    {
        Entities[EntityIndex].sprite = SheetSprite(tileSheetTexture, 15.0/SPRITE_COUNT_X, 6.0/SPRITE_COUNT_Y, 1.0/30.0, 1.0/30.0, 0.25);
    }
    
    EntityIndex++;
    numOfEntities++;
}

bool readEntityData(std::ifstream &stream)
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

