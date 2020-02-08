#ifndef _MY_GL_H_
#define _MY_GL_H_

#include "tgaimage.h"
#include "geometry.h"

extern Matrix ViewportMatrix,PerspProjMatrix,ViewMatrix,OrthMatrix,ProjMatrix;

void viewport(int x,int y,int w,int h);
void lookat(Vec3f eyep,Vec3f lookatp, Vec3f up);
void projection();
Vec3f barycentric(Vec3f A,Vec3f B,Vec3f C,Vec3f p);

class IShader {
public:
    virtual ~IShader() {};
    virtual Vec3f vertex(int iface,int nthvert) = 0;  //计算screen_coordinate
    virtual bool fragment(Vec3f *nms,Vec3f bcoor,TGAColor &color)=0; //计算某点的颜色
};

void triangle(Vec3f *pts,Vec3f *nms,IShader &shader,TGAImage &image, float *zbuffer);

#endif 