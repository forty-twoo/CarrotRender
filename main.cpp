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
const int width = 1024;
const int height = 1024;
const int depth = 800;
Vec3f light_dir(0,0,-1);
float volume_l,volume_r,volume_b,volume_t,volume_n,volume_f;
Vec3f eyep(1,1,1),lookatp(0,0,0),eyegaze;
Matrix ViewportMatrix,PerspProjMatrix,ViewMatrix,OrthMatrix;

//模型默认在世界坐标系的原点处，相机坐标系原点在eyep处，相机坐标系的z轴为视线的反方向，定义物体在z轴负向
Matrix GetViewport(int x,int y,int w,int h){
    Matrix m=Matrix::identity(4);
    m[0][0]=(width)/2.0;
    m[0][3]=(x+width-1.0)/2.0;
    m[1][1]=(1.f*height)/2.0;
    m[1][3]=(y+1.f*height-1.0)/2.0;
    m[2][2]=(1.f*depth)/2.0;
    m[2][3]=(1.f*depth-1.0)/2.0;
    return m;
}
void InitTransform(){

    volume_f=-10000.0,volume_n=-1.0;
    volume_t=1.0,volume_b=-1.0;
    volume_r=1.0,volume_l=-1.0;

    Matrix ModelMatrix=Matrix::identity(4);
    Vec3f u,v,w,t;
    t=Vec3f(0,1,0);
    eyegaze=lookatp-eyep;
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

    ViewportMatrix=GetViewport(0,0,width,height);
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


void DrawTriangle(Vec3f *pts,Vec3f *nms,Vec2i* uv, float *zbuffer,TGAImage &image){
    Vec2i bbox[2];
    bbox[0].x=min(pts[2].x,min(pts[0].x,pts[1].x));
    bbox[0].y=min(pts[2].y,min(pts[0].y,pts[1].y));
    bbox[1].x=max(pts[2].x,max(pts[0].x,pts[1].x));
    bbox[1].y=max(pts[2].y,max(pts[0].y,pts[1].y));

    for(int x=bbox[0].x;x<=bbox[1].x;x++){
        for(int y=bbox[0].y;y<=bbox[1].y;y++){
            Vec3f cur={(float)x,(float)y,0.0};
            Vec3f curn;
            Vec3f bcoor=barycentric(pts[0],pts[1],pts[2],cur);
            if(bcoor.x<0 || bcoor.y<0 || bcoor.z<0)continue;
            //注意这个012的顺序
            cur.z=bcoor.x*pts[0].z+bcoor.y*pts[1].z+bcoor.z*pts[2].z;
            int id=cur.x+cur.y*width;
            if(id<0){
                cerr<<"id<0"<<endl;
            }
            if(zbuffer[id]<cur.z){
                zbuffer[id]=cur.z;
                Vec3f curn;//法向量
                Vec2i curuv;//纹理坐标
                //法向量插值
                for(int i=0;i<3;i++){
                    curn[i]=bcoor.x*nms[0][i]+bcoor.y*nms[1][i]+bcoor.z*nms[2][i];
                }
                //纹理坐标插值
                curuv.x=(int)(bcoor.x*uv[0].x+bcoor.y*uv[1].x+bcoor.z*uv[2].x);
                curuv.y=(int)(bcoor.x*uv[0].y+bcoor.y*uv[1].y+bcoor.z*uv[2].y);
                TGAColor color=model->getcolor(curuv);

                Vec3f tmp=light_dir;
                for(int i=0;i<3;i++)tmp[i]=-tmp[i];
                tmp.normalize();
                curn.normalize();
                float intensity=tmp*curn;
                if(intensity<0)intensity=0;
                image.set(x,y,TGAColor(color.r*intensity,color.g*intensity,color.b*intensity,255));
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
        vector<int> face = model->face(i);
        Vec3f world_coor[3];
        Vec3f pts[3];
        Vec3f norm[3];
        Vec2i uv[3];
        for (int j=0; j<3; j++) {
            Vec3f v=model->vert(face[j]);
            world_coor[j]=v;
            pts[j]=MatrixToVec(ViewportMatrix*OrthMatrix*PerspProjMatrix*ViewMatrix*VecToMatrix(v));
            norm[j]=model->norm(i,j);
            uv[j]=model->uv(i,j);
        }
        //norm:法向量插值不需要转化为相机坐标系，直接用世界坐标系即可
        DrawTriangle(pts,norm,uv,zbuffer,image);
    }
    image.flip_vertically(); 
    image.write_tga_file("my.tga");
    delete model;
    return 0;
}
