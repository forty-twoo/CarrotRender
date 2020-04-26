/*
 * @Don't panic: Allons-y!
 * @Author: forty-twoo
 * @LastEditTime: 2020-04-26 11:04:40
 * @Description: 主函数
 * @Source: ME
 */
#include<vector>
#include<cmath>
#include<cstdlib>
#include<limits>
#include<iostream>
#include<algorithm>

#include "shelp.h"
#include "tgaimage.h"
#include "my_gl.h"
#include "model.h"
#include "my_shader.h"

using namespace std;
const int width=800;
const int height=800;

Model *model = NULL;
float *zbuffer=new float[2*width*height];
float *shadowbuffer=new float[2*width*height];
Vector3f light_p(-5,8,7),light_to(0,-2,0);
Vector3f eyep(0,8,12),lookatp(0,-2,0),eyegaze,up(0,1,0);

int main(int argc,char ** argv){
    if (2>argc) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage depth_image(width, height, TGAImage::RGB);
    viewport(width,height);
    for(int i=0;i<width*height;i++){
        zbuffer[i]=-numeric_limits<float>::max();
        shadowbuffer[i]=-numeric_limits<float>::max();
    }
    
    Matrix4f M;
    DepthShader depthshader;
    lookat(light_p,light_to,up);
    projection();
    depthshader.scaleM=Matrix3f::Identity();
    for(int i=0;i<3;i++)depthshader.scaleM(i,i)=0.5;
    M=ViewportMatrix*ProjMatrix*ViewMatrix;
    depthshader.uniform_M=M;
    for(int m=1;m<argc;m++){
        model=new Model(argv[m]);
        for (int i=0; i<model->nfaces(); i++) {
            vector<int> face = model->face(i);
            Vector3f screen_c[3];
            for (int j=0; j<3; j++) {
                screen_c[j]=depthshader.vertex(i,j); 
            }
            triangle(screen_c,depthshader,depth_image,shadowbuffer);
        }
        delete model;
    }
    depth_image.flip_vertically(); 
    depth_image.write_tga_file("depth.tga");

    DiffuseShader myshader;
    lookat(eyep,lookatp,up);
    myshader.uniform_M=ViewportMatrix*ProjMatrix*ViewMatrix;
    myshader.uniform_MShadow=M*myshader.uniform_M.inverse();
    cout<<"M:\n";
    cout<<M<<endl;
    cout<<"Uniform_MS:\n";
    cout<<myshader.uniform_MShadow<<endl;

    myshader.scaleM=Matrix3f::Identity();
    for(int i=0;i<3;i++)myshader.scaleM(i,i)=0.5;
    for(int m=1;m<argc;m++){
        model=new Model(argv[m]);
        for (int i=0; i<model->nfaces(); i++) {
            vector<int> face = model->face(i);
            Vector3f screen_c[3];
            for (int j=0; j<3; j++) {
                screen_c[j]=myshader.vertex(i,j); 
            }
            triangle(screen_c,myshader,image,zbuffer);
        }
        delete model;
    }
    image.flip_vertically(); 
    image.write_tga_file("my.tga");

    show_pic("my.tga");
    return 0;
}