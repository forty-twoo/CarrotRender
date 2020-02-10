#include<vector>
#include<cmath>
#include<cstdlib>
#include<limits>
#include<iostream>
#include<algorithm>

#include "tgaimage.h"
#include "my_gl.h"
#include "model.h"

using namespace std;
const int width=800;
const int height=800;

Model *model = NULL;
float *zbuffer=new float[2*width*height];
Vector3f light_dir(0.5,0,1);
Vector3f eyep(0.5,0.5,2),lookatp(0,0,0),eyegaze,up(0,1,0);

class GouraudShader : public IShader{
public:
    Vector3f world_c[3];
    virtual Vector3f vertex(int iface,int nvert){
        Vector3f vtx=model->vert(iface,nvert);
        world_c[nvert]=vtx;
        Vector3f pts=(ViewportMatrix*ProjMatrix*ViewMatrix*vtx.homogeneous()).hnormalized();
        return pts;
    }
    virtual bool fragment(Vector3f*nms, Vector2f *uv,Vector3f bcoor,TGAColor& color){
        Vector3f curn;
        Vector2f curuv;
        for(int i=0;i<2;i++){
            curuv[i]=bcoor[0]*uv[0][i]+bcoor[1]*uv[1][i]+bcoor[2]*uv[2][i];
        }
        for(int i=0;i<3;i++){
            curn[i]=bcoor[0]*nms[0][i]+bcoor[1]*nms[1][i]+bcoor[2]*nms[2][i];
        }
        color=model->diffuse(curuv);

        Matrix<float,2,3> TBN;
        Matrix<float,2,3> t_vtx;
        for(int i=0;i<3;i++){
            t_vtx(0,i)=world_c[2][i]-world_c[0][i];
            t_vtx(1,i)=world_c[1][i]-world_c[0][i];
        }
        Matrix<float,2,2> t_uv;
        for(int i=0;i<2;i++){
            t_uv(0,i)=uv[2][i]-uv[0][i];
            t_uv(1,i)=uv[1][i]-uv[0][i];
        }
        Matrix<float,2,2> t_uv_I;
        float c=(t_uv(0,0)*t_uv(1,1)-t_uv(0,1)*t_uv(1,0));
        t_uv_I(0,0)=t_uv(1,1)/c;
        t_uv_I(0,1)=-t_uv(0,1)/c;
        t_uv_I(1,0)=-t_uv(1,0)/c;
        t_uv_I(1,1)=t_uv(0,0)/c;
        TBN=t_uv_I*t_vtx;
        Vector3f T,B;
        for(int i=0;i<3;i++)T[i]=TBN(0,i);
        T.normalize();
        Vector3f N=curn;
        N.normalize();
        B=N.cross(T);
        B.normalize();
        Matrix<float,3,3>TBN_;
        for(int i=0;i<3;i++){
            TBN_(0,i)=T[i];
            TBN_(1,i)=B[i];
            TBN_(2,i)=N[i];
        }
        
        //cout<<TBN_<<endl<<endl;
        Vector3f n=model->normalmap_coor(curuv);
        //cout<<"("<<n[0]<<","<<n[1]<<","<<n[2]<<")\n";
        n=TBN_*n;
        Vector3f l=light_dir;
        n.normalize();
        l.normalize();
        float intensity=max(0.f,l.dot(n));
        
        color=color*intensity;
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
    viewport(0,0,width,height);
    lookat(eyep,lookatp,up);
    projection();

    for(int i=0;i<width*height;i++){
        zbuffer[i]=-numeric_limits<float>::max();
    }
    GouraudShader myshader;

    for (int i=0; i<model->nfaces(); i++) {
        vector<int> face = model->face(i);
        Vector3f screen_c[3];
        Vector3f nms[3];
        Vector2f uv[3];
        for (int j=0; j<3; j++) {
            screen_c[j]=myshader.vertex(i,j); 
            //nms[j]=MatrixToVec(myshader.uniform_MIT*VecToMatrix(model->normal(i,j)));
            nms[j]=model->normal(i,j);
            uv[j]=model->uv(i,j);
            
        }
        triangle(screen_c,nms,uv,myshader,image,zbuffer);
    }
    image.flip_vertically(); 
    image.write_tga_file("my.tga");
    delete model;
    return 0;
}
