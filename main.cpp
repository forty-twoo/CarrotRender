#include<vector>
#include<cmath>
#include<cstdlib>
#include<limits>
#include<iostream>
#include<algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "my_gl.h"

using namespace std;
const int width=800;
const int height=800;

Model *model = NULL;
float *zbuffer=new float[width*height];
Vec3f light_dir(0,0,-1);
Vec3f eyep(1,1,1),lookatp(0,0,0),eyegaze,up(0,1,0);

Matrix VecToMatrix(Vec3f v){
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}
Vec3f MatrixToVec(Matrix m){
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}
class GouraudShader : public IShader{
public:
    virtual Vec3f vertex(int iface,int nvert){
        Vec3f vtx=model->vert(iface,nvert);
        Vec3f pts;pts=MatrixToVec(ViewportMatrix*ProjMatrix*ViewMatrix*VecToMatrix(vtx));
        return pts;
    }
    virtual bool fragment(Vec3f*nms, Vec3f bcoor,TGAColor& color){
        Vec3f curn,tmp;
        for(int i=0;i<3;i++)tmp[i]=-light_dir[i];

        curn.x=bcoor.x*nms[0].x+bcoor.y*nms[1].x+bcoor.z*nms[2].x;
        curn.y=bcoor.x*nms[0].y+bcoor.y*nms[1].y+bcoor.z*nms[2].y;
        curn.z=bcoor.x*nms[0].z+bcoor.y*nms[1].z+bcoor.z*nms[2].z;
        curn.normalize();tmp.normalize();
        float intensity=std::max(curn*tmp,0.f);
        color=TGAColor(255*intensity,255*intensity,255*intensity,255);
        return false;
    }
};

int main(int argc,char ** argv){
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    lookat(eyep,lookatp,up);
    viewport(0,0,width,height);
    projection();

    light_dir.normalize();
    for(int i=0;i<width*height;i++){
        zbuffer[i]=-numeric_limits<float>::max();
    }
    GouraudShader myshader;
    for (int i=0; i<model->nfaces(); i++) {
        vector<int> face = model->face(i);
        Vec3f screen_c[3];
        Vec3f nms[3];
        for (int j=0; j<3; j++) {
            screen_c[j]=myshader.vertex(i,j); 
            nms[j]=model->normal(i,j);
        }
        triangle(screen_c,nms,myshader,image,zbuffer);
    }
    image.flip_vertically(); 
    image.write_tga_file("my.tga");
    delete model;
    return 0;
}
