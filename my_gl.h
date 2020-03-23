/*
 * @Don't panic: Allons-y!
 * @Author: forty-twoo
 * @LastEditTime: 2020-03-23 13:44:58
 * @Description: shader基类，一些头文件
 * @Source: ME
 */
#ifndef _MY_GL_H_
#define _MY_GL_H_

#include "tgaimage.h"
#include<iostream>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace std;
using namespace Eigen;

extern Matrix4f ViewportMatrix,PerspProjMatrix,ViewMatrix,OrthMatrix,ProjMatrix;
extern Matrix4f PerspProjMatrix_s,ViewMatrix_s,OrthMatrix_s,ProjMatrix_s;

void viewport(int x,int y,int w,int h);
void lookat(Vector3f eyep,Vector3f lookatp, Vector3f up);
void projection();

Vector3f barycentric(Vector3f A,Vector3f B,Vector3f C,Vector3f p);

class IShader {
public:
    virtual ~IShader() {};
    virtual Vector3f vertex(int iface,int nthvert) = 0;  //计算screen_coordinate
    virtual bool fragment(Vector3f sz_z,Vector3f bcoor,TGAColor &color)=0; //计算某点的颜色
};

void triangle(Vector3f *pts,IShader &shader,TGAImage &image, float *zbuffer);

#endif 