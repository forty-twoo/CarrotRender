/*
 * @Don't panic: Allons-y!
 * @Author: forty-twoo
 * @LastEditTime: 2020-03-23 13:49:03
 * @Description: shader函数 
 * @Source: ME
 */
#include "model.h"
#include "my_gl.h"

extern Model *model;
extern Vector3f light_dir;
extern Vector3f eyep;
class GouraudShader : public IShader{
public:
    Vector3f world_c[3];
    Vector2f varying_uv[3];
    Vector3f varying_nms[3];

    virtual Vector3f vertex(int iface,int nvert){
        Vector3f vtx=model->vert(iface,nvert);
        world_c[nvert]=vtx;
        varying_uv[nvert]=model->uv(iface,nvert);
        varying_nms[nvert]=model->normal(iface,nvert);
        Vector3f pts=(ViewportMatrix*ProjMatrix*ViewMatrix*vtx.homogeneous()).hnormalized();
        return pts;
    }
    virtual bool fragment(Vector3f sc_z,Vector3f bcoor,TGAColor& color){
        Vector3f curn;
        Vector2f curuv;
        for(int i=0;i<2;i++){
            curuv[i]=bcoor[0]*varying_uv[0][i]+bcoor[1]*varying_uv[1][i]+bcoor[2]*varying_uv[2][i];
        }
        for(int i=0;i<3;i++){
            curn[i]=bcoor[0]*varying_nms[0][i]+bcoor[1]*varying_nms[1][i]+bcoor[2]*varying_nms[2][i];
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
            t_uv(0,i)=varying_uv[2][i]-varying_uv[0][i];
            t_uv(1,i)=varying_uv[1][i]-varying_uv[0][i];
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
        Vector4f T1,B1,N1;
        for(int i=0;i<3;i++)T1[i]=T[i],B1[i]=B[i],N1[i]=N[i];

        T1=ViewMatrix*T1;
        B1=ViewMatrix*B1;
        N1=ViewMatrix*N1;

        Matrix<float,3,3>TBN_;
        for(int i=0;i<3;i++){
            TBN_(0,i)=T1[i];
            TBN_(1,i)=B1[i];
            TBN_(2,i)=N1[i];
        }
        TBN_.transpose();
        
        //cout<<TBN_<<endl<<endl;
        Vector3f n=model->normalmap_coor(curuv);
        //cout<<"("<<n[0]<<","<<n[1]<<","<<n[2]<<")\n";
        Vector3f l=light_dir;
        l=TBN_*l;
        n.normalize();
        l.normalize();
        float intensity=max(0.f,l.dot(n));
        color=color*intensity;
        return false;
    }
};
