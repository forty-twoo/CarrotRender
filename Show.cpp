/*
 * @Don't panic: Allons-y!
 * @Author: forty-twoo
 * @LastEditTime: 2020-03-11 19:55:59
 * @Description: 利用SDL输出图片到窗口
 * @Source: ME
 */
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include <stdio.h>
#include<string>
#include "shelp.h"

const int SCREEN_WIDTH=800;
const int SCREEN_HEIGHT=800;

bool init();
bool loadMedia();
void close();
SDL_Surface *loadSurface(std::string path);
SDL_RWops *rwop;
SDL_Window* gWindow=NULL;
SDL_Surface* gScreenSurface=NULL;
SDL_Surface* gPNGSurface=NULL;
bool init(){
    bool success=true;
    if(SDL_Init(SDL_INIT_VIDEO)<0){
        printf("SDL could not initialize! SDL Error: %s\n",SDL_GetError());
        success=false;
    }else{
        gWindow=SDL_CreateWindow("SDL Test",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
        if(gWindow==NULL){
            printf("Window could not be created! SDL Error: %s\n",SDL_GetError());
            success=false;
        }else{
            gScreenSurface=SDL_GetWindowSurface(gWindow);
        }
    }
    return success;
}
bool loadMedia(std::string path){
    bool success=true;
    gPNGSurface=loadSurface(path);
    if(gPNGSurface==NULL){
        printf("Failed to load TGA image!\n");
        success=false;
    }
    return success;
}
void close(){
    SDL_FreeSurface(gPNGSurface);
    gPNGSurface=NULL;
    SDL_DestroyWindow(gWindow);
    gWindow=NULL;
    IMG_Quit();
    SDL_Quit();
}
SDL_Surface* loadSurface(std::string path){
    SDL_Surface* optimizedSurface=NULL;
    rwop=SDL_RWFromFile(path.c_str(),"rb");
    SDL_Surface *loadSurface=IMG_LoadTGA_RW(rwop);
    if(loadSurface==NULL){
        printf("Unable to load image %s! SDL_image Error: %s\n",path.c_str(),IMG_GetError());
    }else{
        optimizedSurface=SDL_ConvertSurface(loadSurface,gScreenSurface->format,0);
        if(optimizedSurface==NULL){
            printf("Unable to optimize image %s! SDL Error: %s\n",path.c_str(),SDL_GetError());
        }
        SDL_FreeSurface(loadSurface);
    }
    return optimizedSurface;
}
void show_pic(std::string path){
    if(!init()){
        printf("Failed to initialize!\n");
    }else{
        if(!loadMedia(path)){
            printf("Failed to load media!\n");
        }else{
            bool quit=false;
            SDL_Event e;
            while(!quit){
                while(SDL_PollEvent(&e)!=0){
                    if(e.type==SDL_QUIT){
                        quit=true;
                    }
                }
                SDL_BlitSurface(gPNGSurface,NULL,gScreenSurface,NULL);
                SDL_UpdateWindowSurface(gWindow);
            }
        }
    }
    close();
}
