#include<cmath>
#include<iostream>
#include<limits>
#include<cstdlib>
#include "my_gl.h"

Matrix ViewportMatrix,PerspProjMatrix,ViewMatrix,OrthMatrix,ProjMatrix;



//模型默认在世界坐标系的原点处，相机坐标系原点在eyep处，相机坐标系的z轴为视线的反方向，定义物体在z轴负向
void viewport(int x,int y,int w,int h){
    ViewportMatrix=Matrix::identity(4);
    ViewportMatrix[0][0]=(w)/2.0;
    ViewportMatrix[0][3]=(x+w-1.0)/2.0;
    ViewportMatrix[1][1]=(1.f*h)/2.0;
    ViewportMatrix[1][3]=(y+1.f*h-1.0)/2.0;
    ViewportMatrix[2][2]=(255.f)/2.0;
    ViewportMatrix[2][3]=(255.f-1.0)/2.0;
}

void lookat(Vec3f eyep,Vec3f lookatp, Vec3f up){
    Vec3f u,v,w,eyegaze;
    eyegaze=lookatp-eyep;
    for(int i=0;i<3;i++)w[i]=-eyegaze[i];
    w.normalize();
    u=up^w;
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
    std::cout<<"ViewMatrix: "<<std::endl;
    std::cout<<ViewMatrix<<std::endl;
}
void projection(){
    float volume_l,volume_r,volume_b,volume_t,volume_n,volume_f;
    volume_f=-10000.0,volume_n=-1.0;
    volume_t=2.0,volume_b=-2.0;
    volume_r=2.0,volume_l=-2.0;
    //投影幕为volume_n
    PerspProjMatrix[0][0]=volume_n;
    PerspProjMatrix[1][1]=volume_n;
    PerspProjMatrix[2][2]=volume_n+volume_f;
    PerspProjMatrix[2][3]=(-1.0)*volume_n*volume_f;
    PerspProjMatrix[3][2]=1;

    OrthMatrix=Matrix::identity(4);
    OrthMatrix[0][0]=2.0/(volume_r-volume_l);
    OrthMatrix[1][1]=2.0/(volume_t-volume_b);
    OrthMatrix[2][2]=2.0/(volume_n-volume_f);
    OrthMatrix[0][3]=(-1.0)*(volume_r+volume_l)/(volume_r-volume_l);
    OrthMatrix[1][3]=(-1.0)*(volume_t+volume_b)/(volume_t-volume_b);
    OrthMatrix[2][3]=(-1.0)*(volume_n+volume_f/(volume_n-volume_f));

    ProjMatrix=OrthMatrix*PerspProjMatrix;
    std::cout<<"ProjMatrix: "<<std::endl;
    std::cout<<ProjMatrix<<std::endl;
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
void triangle(Vec3f *pts,Vec3f *nms,IShader &shader,TGAImage &image, float *zbuffer){
    Vec2f bbox[2];
    bbox[0].x=std::min(pts[2].x,std::min(pts[0].x,pts[1].x));
    bbox[0].y=std::min(pts[2].y,std::min(pts[0].y,pts[1].y));
    bbox[1].x=std::max(pts[2].x,std::max(pts[0].x,pts[1].x));
    bbox[1].y=std::max(pts[2].y,std::max(pts[0].y,pts[1].y));

    TGAColor color;
    for(int x=(int)bbox[0].x;x<=(int)bbox[1].x;x++){
        for(int y=(int)bbox[0].y;y<=(int)bbox[1].y;y++){
            Vec3f cur={(float)x,(float)y,0.0};
            Vec3f bcoor=barycentric(pts[0],pts[1],pts[2],cur);
            if(bcoor.x<0 || bcoor.y<0 || bcoor.z<0)continue;
            //注意这个012的顺序
            cur.z=bcoor.x*pts[0].z+bcoor.y*pts[1].z+bcoor.z*pts[2].z;
            int id=cur.x+cur.y*image.get_width();
            if(zbuffer[id]<cur.z){
                bool discard=shader.fragment(nms,bcoor,color);
                if(!discard){
                    image.set(x,y,color);
                }
                zbuffer[id]=cur.z;
            }
        }
    }
}