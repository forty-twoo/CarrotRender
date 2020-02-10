#include<cmath>
#include<iostream>
#include<limits>
#include<cstdlib>
#include "my_gl.h"

Matrix4f ViewportMatrix,PerspProjMatrix,ViewMatrix,OrthMatrix,ProjMatrix;



//模型默认在世界坐标系的原点处，相机坐标系原点在eyep处，相机坐标系的z轴为视线的反方向，定义物体在z轴负向
void viewport(int x,int y,int w,int h){
    ViewportMatrix=Matrix4f::Identity();
    ViewportMatrix(0,0)=(w)/2.0;
    ViewportMatrix(0,3)=(x+w)/2.0;
    ViewportMatrix(1,1)=(1.f*h)/2.0;
    ViewportMatrix(1,3)=(y+1.f*h)/2.0;
    ViewportMatrix(2,2)=(255.f)/2.0;
    ViewportMatrix(2,3)=(255.f)/2.0;
}

void lookat(Vector3f eyep,Vector3f lookatp, Vector3f up){
    Vector3f u,v,w,eyegaze;
    eyegaze=lookatp-eyep;
    for(int i=0;i<3;i++)w[i]=-eyegaze[i];
    w.normalize();
    u=up.cross(w);
    u.normalize();
    v=w.cross(u);v.normalize();
    Matrix4f ts=Matrix4f::Identity();
    Matrix4f mv=Matrix4f::Identity();
    for(int i=0;i<3;i++){
        ts(i,3)=-eyep[i];
    }
    for(int i=0;i<3;i++){
        mv(0,i)=u(i);
        mv(1,i)=v(i);
        mv(2,i)=w(i);
    }
    ViewMatrix=mv*ts;
    std::cout<<"ViewMatrix: "<<std::endl;
    std::cout<<ViewMatrix<<std::endl;
}
void projection(){
    float volume_l,volume_r,volume_b,volume_t,volume_n,volume_f;
    float cf=1.5;
    volume_f=-100000.0,volume_n=-cf;
    volume_t=cf,volume_b=-cf;
    volume_r=cf,volume_l=-cf;
    //投影幕为volume_n
    PerspProjMatrix(0,0)=volume_n;
    PerspProjMatrix(1,1)=volume_n;
    PerspProjMatrix(2,2)=volume_n+volume_f;
    PerspProjMatrix(2,3)=(-1.0)*volume_n*volume_f;
    PerspProjMatrix(3,2)=1;

    OrthMatrix=Matrix4f::Identity();
    OrthMatrix(0,0)=2.0/(volume_r-volume_l);
    OrthMatrix(1,1)=2.0/(volume_t-volume_b);
    OrthMatrix(2,2)=2.0/(volume_n-volume_f);
    OrthMatrix(0,3)=(-1.0)*(volume_r+volume_l)/(volume_r-volume_l);
    OrthMatrix(1,3)=(-1.0)*(volume_t+volume_b)/(volume_t-volume_b);
    OrthMatrix(2,3)=(-1.0)*(volume_n+volume_f/(volume_n-volume_f));

    ProjMatrix=OrthMatrix*PerspProjMatrix;
    std::cout<<"ProjMatrix: "<<std::endl;
    std::cout<<ProjMatrix<<std::endl;
}
Vector3f barycentric(Vector3f A,Vector3f B,Vector3f C,Vector3f p){
    Vector3f _A=A,_B=B,_C=C;
    _A[2]=_B[2]=_C[2]=0;
    Vector3f n=(_B-_A).cross((_C-_A));
    Vector3f n1=(_C-_B).cross((p-_B));
    Vector3f n2=(_A-_C).cross((p-_C));
    Vector3f n3=(_B-_A).cross((p-_A));
    float a,b,c;
    a=(float)(n.dot(n1))/(float)(n.dot(n));
    b=(float)(n.dot(n2))/(float)(n.dot(n));
    c=(float)(n.dot(n3))/(float)(n.dot(n));
    return Vector3f(a,b,c);
}
void triangle(Vector3f *pts,Vector3f *nms,Vector2f *uv,IShader &shader,TGAImage &image, float *zbuffer){
    Vector2f bbox[2];
    bbox[0][0]=std::min(pts[2][0],std::min(pts[0][0],pts[1][0]));
    bbox[0][1]=std::min(pts[2][1],std::min(pts[0][1],pts[1][1]));
    bbox[1][0]=std::max(pts[2][0],std::max(pts[0][0],pts[1][0]));
    bbox[1][1]=std::max(pts[2][1],std::max(pts[0][1],pts[1][1]));

    TGAColor color;
    for(int x=(int)bbox[0][0];x<=(int)bbox[1][0];x++){
        for(int y=(int)bbox[0][1];y<=(int)bbox[1][1];y++){
            Vector3f cur={(float)x,(float)y,0.0};
            Vector3f bcoor=barycentric(pts[0],pts[1],pts[2],cur);
            if(bcoor[0]<0 || bcoor[1]<0 || bcoor[2]<0)continue;
            //注意这个012的顺序
            cur[2]=bcoor[0]*pts[0][2]+bcoor[1]*pts[1][2]+bcoor[2]*pts[2][2];
            int id=cur[0]+cur[1]*image.get_width();
            if(id<0 || id>=image.get_width()*image.get_height()){
                std::cout<<"("<<x<<" , "<<y<<")"<<std::endl;
                std::cerr<<id<<std::endl;
                return;
            }
            if(zbuffer[id]<cur[2]){
                bool discard=shader.fragment(nms,uv,bcoor,color);
                if(!discard){
                    image.set(x,y,color);
                }
                zbuffer[id]=cur[2];
            }
        }
    }
}
