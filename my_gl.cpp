#include<cmath>
#include<iostream>
#include<limits>
#include<cstdlib>
#include "my_gl.h"
#include "my_shader.h"

Matrix4f ViewportMatrix,PerspProjMatrix,ViewMatrix,OrthMatrix,ProjMatrix,OrthMatrix_s;

//模型默认在世界坐标系的原点处，相机坐标系原点在eyep处，相机坐标系的z轴为视线的反方向，定义物体在z轴负向
void viewport(int x,int y){
    ViewportMatrix=Matrix4f::Identity();
    ViewportMatrix(0,0)=(x)/2.0;
    ViewportMatrix(0,3)=(x)/2.0;
    ViewportMatrix(1,1)=(y)/2.0;
    ViewportMatrix(1,3)=(y)/2.0;
    ViewportMatrix(2,2)=2.f/2.0;
    ViewportMatrix(2,3)=(2.f)/2.0;

}

void lookat(Vector3f cam_p,Vector3f cam_to, Vector3f up){
    Vector3f u,v,w,cam_gaze;
    cam_gaze=cam_to-cam_p;
    for(int i=0;i<3;i++)w[i]=-cam_gaze[i];
    w.normalize();
    u=up.cross(w);
    u.normalize();
    v=w.cross(u);v.normalize();
    Matrix4f ts=Matrix4f::Identity();
    Matrix4f mv=Matrix4f::Identity();
    for(int i=0;i<3;i++){
        ts(i,3)=-cam_p[i];
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
void projection(float coeff){
    float volume_l,volume_r,volume_b,volume_t,volume_n,volume_f;
    volume_f=-1000.0,volume_n=-1.0;
    volume_t=6,volume_b=-6;
    volume_r=6,volume_l=-6;
    //投影幕为volume_n
    /*
    PerspProjMatrix(0,0)=volume_n;
    PerspProjMatrix(1,1)=volume_n;
    PerspProjMatrix(2,2)=volume_n+volume_f;
    PerspProjMatrix(2,3)=(-1.0)*volume_n*volume_f;
    PerspProjMatrix(3,2)=1;
    */

    OrthMatrix=Matrix4f::Identity();
    OrthMatrix(0,0)=2.0/(volume_r-volume_l);
    OrthMatrix(1,1)=2.0/(volume_t-volume_b);
    OrthMatrix(2,2)=2.0/(volume_n-volume_f);
    OrthMatrix(0,3)=(-1.0)*(volume_r+volume_l)/(volume_r-volume_l);
    OrthMatrix(1,3)=(-1.0)*(volume_t+volume_b)/(volume_t-volume_b);
    OrthMatrix(2,3)=(-1.0)*(volume_n+volume_f/(volume_n-volume_f));

    ProjMatrix=OrthMatrix;
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

void triangle(Vector3f *pts,IShader &shader,TGAImage &image, float *buffer){
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
            //cout<<pts[0][2]<<" "<<pts[0][2]<<" "<<pts[2][2]<<endl;
            //注意这个012的顺序
            int id=cur[1]+cur[0]*image.get_width();
            if(id<0 || id>=image.get_width()*image.get_height()){
                std::cout<<"("<<x<<" , "<<y<<")"<<std::endl;
                std::cerr<<id<<std::endl;
                return;
            }
            cur[2]=bcoor[0]*pts[0][2]+bcoor[1]*pts[1][2]+bcoor[2]*pts[2][2];
            if(buffer[id]<cur[2]){
                buffer[id]=cur[2];
                Vector3f curp=cur;
                bool discard=shader.fragment(bcoor,color,image,curp);
                if(!discard){
                    image.set(x,y,color);
                }
            }
        }
    }
}
