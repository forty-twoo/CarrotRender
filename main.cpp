/*
 * @Don't panic: Allons-y!
 * @Author: forty-twoo
 * @LastEditTime: 2020-03-23 13:51:03
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
Vector3f light_dir(1.2,1.5,1),light_p(0.2,1,1),light_to(0,0,0);
Vector3f eyep(0,0,1),lookatp(0,0,0),eyegaze,up(0,1,0);

int main(int argc,char ** argv){
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/diablo3_pose.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    viewport(0,0,width*3/4,height*3/4);
    for(int i=0;i<width*height;i++){
        zbuffer[i]=-numeric_limits<float>::max();
        shadowbuffer[i]=-numeric_limits<float>::max();
    }

    lookat(eyep,lookatp,up);
    projection();
    GouraudShader myshader;
    for (int i=0; i<model->nfaces(); i++) {
        vector<int> face = model->face(i);
        Vector3f screen_c[3];
        for (int j=0; j<3; j++) {
            screen_c[j]=myshader.vertex(i,j); 
        }
        triangle(screen_c,myshader,image,zbuffer);
    }
    image.flip_vertically(); 
    image.write_tga_file("my.tga");
    delete model;
    show_pic("my.tga");
    
    return 0;
}
