#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "Matrix.h"
#include "stb_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ShaderProgram.h"

#include "assert.h"


#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

ShaderProgram Setup(){
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glViewport(0, 0, 640, 360);
    
    return program;
}

class Entity{
public:
    void Draw(ShaderProgram& program, Matrix& modelviewMatrix){
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        program.SetModelviewMatrix(modelviewMatrix);
        
        float x_coord = width / 2.0;
        float y_coord = height / 2.0;
        float vertices[] = {-1 * x_coord, -1 * y_coord, x_coord, -1 * y_coord, x_coord, y_coord, -1 * x_coord, -1 * y_coord, x_coord, y_coord, -1 * x_coord, y_coord};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    };
    float top(){
        return (y_position + (height/2));
    };
    float bottom(){
        return (y_position - (height/2));
    };
    float right(){
        return (x_position + (width/2));
    };
    float left(){
        return (x_position - (width/2));
    };
    float x_position;
    float y_position;
    float rotation;
    GLuint textureID;
    float width;
    float height;
    float speed;
    float direction_x;
    float direction_y;
};

int main(int argc, char *argv[])
{
    
    ShaderProgram program = Setup();
    
    //Load all textures
    GLuint ball1Texture = LoadTexture("ball1.png");
    GLuint ball2Texture = LoadTexture("ball2.png");
    GLuint poppyWinsTexture = LoadTexture("poppywins.png");
    GLuint branchWinsTexture = LoadTexture("branchwins.png");
    
    Entity rpaddle;
    Entity lpaddle;
    Entity ball;
    Entity background;
    
    rpaddle.textureID = LoadTexture("poppy.png");
    lpaddle.textureID = LoadTexture("branch.png");
    
    Matrix projectionMatrix;
    Matrix modelviewMatrix;
    Matrix viewMatrix;
    
    projectionMatrix.SetOrthoProjection(-3.70, 3.70, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    float lastFrameTicks = 0.0f;
    float angle = 0.0f;
    
    rpaddle.y_position = 0.0;
    rpaddle.x_position = 3.40;
    
    lpaddle.x_position = -3.40;
    lpaddle.y_position = 0.0;
    
    ball.x_position = 0.0;
    ball.y_position = 0.0;
    
    ball.direction_x = 1.0;
    ball.direction_y = 1.0;
    ball.speed = 2.0;
    
    float acceleration = 1.0;
    bool rpaddleWins = 0;
    bool lpaddleWins = 0;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        //Increase the ball speed as the game progresses
        acceleration += 0.001;
        
        //Get keyboard input
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_UP]) {
            if(rpaddle.top() < 2.0){
                rpaddle.y_position += 0.10;
            }
        } else if(keys[SDL_SCANCODE_DOWN]) {
            if(rpaddle.bottom() > -2.0){
                rpaddle.y_position -= 0.10;
            }
        }
        if(keys[SDL_SCANCODE_W]) {
            if(lpaddle.top() < 2.0){
                lpaddle.y_position += 0.10;
            }
        } else if(keys[SDL_SCANCODE_S]) {
            if(lpaddle.bottom() > -2.0){
                lpaddle.y_position -= 0.10;
            }
        }
        
        //Get the time elapsed since the last iteration fo the loop
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        angle += elapsed;
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(modelviewMatrix);
        
        //___________Background___________________
        if (rpaddleWins || lpaddleWins){
            modelviewMatrix.Identity();
            
            if (rpaddleWins == 1 && lpaddleWins == 0){
                background.textureID = poppyWinsTexture;
            }
            if (rpaddleWins == 0 && lpaddleWins == 1){
                background.textureID = branchWinsTexture;
            }
            
            background.height = 4.0;
            background.width = 7.40;
            background.Draw(program, modelviewMatrix);
        }
        
        //___________Right Paddel____________
        
        modelviewMatrix.Identity();
        
        rpaddle.width = 0.5;
        rpaddle.height = 1.5;
        
        modelviewMatrix.Translate(rpaddle.x_position, rpaddle.y_position, 0.0);
        
        rpaddle.Draw(program, modelviewMatrix);
        
        //___________Left Paddle____________
        
        modelviewMatrix.Identity();
        
        lpaddle.width = 0.5;
        lpaddle.height = 1.5;
        
        modelviewMatrix.Translate(lpaddle.x_position, lpaddle.y_position, 0.0);
        
        lpaddle.Draw(program, modelviewMatrix);
        
        //___________Ball____________
        
        modelviewMatrix.Identity();
        
        ball.width = 0.5;
        ball.height = 0.5;
        
        //test collision with top
        if(ball.top() >= 2.0){
            ball.direction_y = -1;
        }
        //test collision with rpaddle
        if(ball.right() >= rpaddle.left() && !((ball.bottom() > rpaddle.top()) || (ball.top() < rpaddle.bottom()))){
            ball.direction_x = -1;
        }
        
        //test collision with lpaddle
        if(ball.left() <= lpaddle.right() && !((ball.bottom() > lpaddle.top()) || (ball.top() < lpaddle.bottom()))){
            ball.direction_x = 1;
        }
        
        //test collision with top of rpaddle
        if(ball.bottom() <= rpaddle.top() && !((ball.right() <= rpaddle.left()))){
            ball.direction_y = 1;
        }
        
        //test collision with bottom of rpaddle
        if(ball.top() >= rpaddle.bottom() && !(ball.right() <= rpaddle.left())){
            ball.direction_y = -1;
        }
        
        //test collision with top of rpaddle
        if(ball.bottom() <= lpaddle.top() && !(ball.left() >= lpaddle.right())){
            ball.direction_y = 1;
        }
        
        //test collision with bottom of rpaddle
        if(ball.top() >= lpaddle.bottom() && !(ball.left() >= lpaddle.right())){
            ball.direction_y = -1;
        }
        
        //test collision with bottom
        if (ball.bottom() <= -2.0){
            ball.direction_y = 1;
        }
        
        ball.y_position += ball.direction_y * elapsed * ball.speed * acceleration;
        ball.x_position += ball.direction_x * elapsed * ball.speed * acceleration;
        
        if (ball.right() < lpaddle.left()){
            rpaddleWins = 1;
            lpaddleWins = 0;
            ball.x_position = 0.0;
            ball.y_position = 0.0;
            acceleration = 1.0;
        }
        
        if (ball.left() > rpaddle.right()){
            rpaddleWins = 0;
            lpaddleWins = 1;
            ball.x_position = 0.0;
            ball.y_position = 0.0;
            acceleration = 1.0;
        }
        
        modelviewMatrix.Translate(ball.x_position, ball.y_position, 0.0);
        if (ball.direction_x > 0){
            ball.textureID = ball1Texture;
        } else {
            ball.textureID = ball2Texture;
        }
        ball.Draw(program, modelviewMatrix);
        
        
        SDL_GL_SwapWindow(displayWindow);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    SDL_Quit();
    return 0;
}