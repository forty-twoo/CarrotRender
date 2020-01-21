#include<vector>
#include<cmath>
#include<cstdlib>
#include<limits>
#include<iostream>
#include<algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
using namespace std;
const TGAColor white = TGAColor(255,255,255,255);
const TGAColor red = TGAColor(255,0,0,255);
const TGAColor green = TGAColor(0,255,0,255);
const TGAColor blue = TGAColor(0,0,255,255);
Model *model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;
Vec3f light_dir(0,0,-1);
Vec3f eyep(-1,-1,1),eyegaze;
Vec3f lookatp(0,0,0);
Matrix ViewportMatrix,PerspProjMatrix,ViewMatrix;

//模型默认在世界坐标系的原点处，相机坐标系原点在eyep处，相机坐标系的z轴为视线的反方向
Matrix GetViewport(int x,int y,int w,int h){
    Matrix m=Matrix::identity(4);
    m[0][0]=(width)/2.0;
    m[0][3]=(x+width-1.0)/2.0;
    m[1][1]=(height)/2.0;
    m[1][3]=(y+height-1.0)/2.0;
    m[2][3]=depth/2;
    m[2][2]=depth/2;
    return m;
}
void InitTransform(){
    Matrix ModelMatrix=Matrix::identity(4);
    eyegaze=lookatp-eyep;
    Vec3f u,v,w,t;
    t=Vec3f(0,1,0);
    for(int i=0;i<3;i++)w[i]=-eyegaze[i];
    w.normalize();
    u=t^w;
    u.normalize();
    v=w^u;v.normalize();
    Matrix ts=Matrix::identity(4);
    Matrix mv=Matrix::identity(4);
    for(int i=0;i<3;i++)ts[i][3]=-eyep[i];
    for(int i=0;i<3;i++){
        mv[0][i]=u[i];
        mv[1][i]=v[i];
        mv[2][i]=w[i];
    }
    ViewMatrix=mv*ts;

    //投影幕中心在lootatp的位置
    PerspProjMatrix[0][0]=-1.0*(lookatp-eyep).norm();
    PerspProjMatrix[1][1]=-1.0*(lookatp-eyep).norm();
    PerspProjMatrix[2][2]=-1.0*(lookatp-eyep).norm()+(-100);
    PerspProjMatrix[2][3]=(lookatp-eyep).norm()*(-100);
    PerspProjMatrix[3][2]=1;
    PerspProjMatrix=Matrix::identity(4);
    PerspProjMatrix[3][2] = -1.f/(eyep-lookatp).norm();


    cout<<PerspProjMatrix<<endl;
    ViewportMatrix=GetViewport(0,0,width*3/4,height*3/4);
}

void DrawLine(Vec2i p0,Vec2i p1, TGAImage &image, TGAColor color){
    int x0,x1,y0,y1;
    x0=p0.x,y0=p0.y,x1=p1.x,y1=p1.y;
    bool flg=false;
    if(abs(x1-x0)<abs(y1-y0)){
        swap(x0,y0);swap(x1,y1);
        flg=true;
    }
    if(x1<x0){
        swap(x1,x0);swap(y1,y0);
    }
    int x=x0,y=y0;float d;
    if(y1>=y0)d=(x+1.0)*(y0-y1)+(y*1.0+0.5)*(x1-x0)+(x0*y1-x1*y0);
    else d=(x+1.0)*(y0-y1)+(y*1.0-0.5)*(x1-x0)+(x0*y1-x1*y0);
    for(;x<=x1;x++){
        if(flg)image.set(y,x,color);
        else image.set(x,y,color);
        if(y1<y0){
            if(d<0)d+=(y0-y1);
            else y-=1,d+=(y0-y1-x1+x0);
        }else{
            if(d<0)y+=1,d+=(y0-y1+x1-x0);
            else d+=(y0-y1);
        }
    }
}
Vec3f barycentric(Vec3f A,Vec3f B,Vec3f C,Vec3f p){
    Vec3f _A,_B,_C;
    _A=A;_A.z=0;
    _B=B;_B.z=0;
    _C=C;_C.z=0;
    Vec3f n=(_B-_A)^(_C-_A);
    Vec3f n1=(_C-_B)^(p-_B);
    Vec3f n2=(_A-_C)^(p-_C);
    Vec3f n3=(_B-_A)^(p-_A);
    float a,b,c;
    a=(float)(n*n1)/(float)(n*n);
    b=(float)(n*n2)/(float)(n*n);
    c=(float)(n*n3)/(float)(n*n);
    return Vec3f(a,b,c);
}
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


void DrawTriangle(Vec3f *pts,float *zbuffer,TGAImage &image,TGAColor color){ Vec2i bbox[2];
    bbox[0].x=min(pts[2].x,min(pts[0].x,pts[1].x));
    bbox[0].y=min(pts[2].y,min(pts[0].y,pts[1].y));
    bbox[1].x=max(pts[2].x,max(pts[0].x,pts[1].x));
    bbox[1].y=max(pts[2].y,max(pts[0].y,pts[1].y));

    for(int x=bbox[0].x;x<=bbox[1].x;x++){
        for(int y=bbox[0].y;y<=bbox[1].y;y++){
            for(int i=0;i<3;i++){
                Vec3f cur={(float)x,(float)y,0.0};
                Vec3f bcoor=barycentric(pts[0],pts[1],pts[2],cur);
                if(bcoor.x<0 || bcoor.y<0 || bcoor.z<0)continue;
                //cout<<bcoor.x<<" "<<bcoor.y<<" "<<bcoor.z<<endl;
                cur.z=bcoor.x*pts[2].z+bcoor.y*pts[1].z+bcoor.z*pts[0].z;
                int id=cur.x+cur.y*width;
                if(zbuffer[id]<cur.z){
                    zbuffer[id]=cur.z;
                    image.set(x,y,color);
                }
            }

        }
    }
}
int main(int argc,char ** argv){
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    float *zbuffer=new float[width*height];
    for(int i=0;i<width*height;i++){
        zbuffer[i]=-numeric_limits<float>::max();
    }
    InitTransform();
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f world_coor[3];
        Vec3f pts[3];
        for (int j=0; j<3; j++) {
            Vec3f v=model->vert(face[j]);
            world_coor[j]=v;
            pts[j]=MatrixToVec(ViewportMatrix*PerspProjMatrix*ViewMatrix*VecToMatrix(world_coor[j]));
            Vec3f tmp;
            tmp=MatrixToVec(ViewMatrix*VecToMatrix(v));
            cout<<tmp<<endl;
        }
        Vec3f n=(world_coor[2]-world_coor[0])^(world_coor[1]-world_coor[0]);
        n.normalize();
        float intensity=n*light_dir;
        if(intensity>0){
            DrawTriangle(pts,zbuffer,image,TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("my.tga");
    delete model;
    return 0;
}

