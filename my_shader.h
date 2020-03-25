/*
 * @Don't panic: Allons-y!
 * @Author: forty-twoo
 * @LastEditTime: 2020-03-25 22:43:52
 * @Description: shader函数 
 * @Source: ME
 */
#include "model.h"
#include "my_gl.h"

extern Model *model;
extern Vector3f light_to,light_p;
extern Vector3f eyep;
const TGAColor white = TGAColor(255, 255, 255, 255);
extern float *shadowbuffer;
extern float *curdepth;

class DepthShader: public IShader{
public:
    Vector3f pv[3];
    Matrix4f uniform_M;
    Matrix3f scaleM;
    virtual Vector3f vertex(int iface,int nvert){
        Vector3f vtx=model->vert(iface,nvert);
        vtx=scaleM*vtx;
        Vector3f pts=(uniform_M*vtx.homogeneous()).hnormalized();
        
        pv[nvert]=(ViewMatrix*vtx.homogeneous()).hnormalized();
        return pts;
    }
    virtual bool fragment(Vector3f bcoor,TGAColor &color,TGAImage&image,Vector3f curp){
        float zv;
        color=white*((6.0+zv)/6.f);
        return false;
    }
};
class DiffuseShader : public IShader{
public:
    Vector3f world_c[3];
    Vector3f varying_nms[3];
    Matrix4f uniform_MShadow;
    Matrix3f scaleM;
    virtual Vector3f vertex(int iface,int nvert){
        Vector3f vtx=model->vert(iface,nvert);
        vtx=scaleM*vtx;
        world_c[nvert]=vtx;
        varying_nms[nvert]=model->normal(iface,nvert);
        Vector3f pts=(ViewportMatrix*ProjMatrix*ViewMatrix*vtx.homogeneous()).hnormalized();
        return pts;
    }
    virtual bool fragment(Vector3f bcoor,TGAColor& color,TGAImage&image,Vector3f curp){
        Vector3f curn;
        for(int i=0;i<3;i++){
            curn[i]=bcoor[0]*varying_nms[0][i]+bcoor[1]*varying_nms[1][i]+bcoor[2]*varying_nms[2][i];
        }
        color=white;
        Vector3f light=(light_p-light_to).normalized();
        float intensity=0;
        //shadow
        curn.normalized();
        Vector3f bpts=(uniform_MShadow*curp.homogeneous()).hnormalized(); 
        float bias=0.005;
        int x,y,id;
        for(int i=-1;i<=1;i++){
            for(int j=-1;j<=1;j++){
                x=bpts[0]+i,y=bpts[1]+j;
                id=x*image.get_width()+y;
                intensity+=((bpts[2])>shadowbuffer[id]-bias)?1.f:0.f;
            }
        }
        intensity/=9.f;
        color=color*intensity*max(0.f,light.dot(curn));
        return false;
    }
};


class CompleteShader : public IShader{
public:
    Vector3f world_c[3];
    Vector2f varying_uv[3];
    Vector3f varying_nms[3];
    Matrix4f OriM;
    Matrix4f uniform_M;
    Matrix4f uniform_MIT;
    Matrix4f uniform_MShadow;
    
    CompleteShader(Matrix4f M, Matrix4f MIT, Matrix4f MS):uniform_M(M),uniform_MIT(MIT),uniform_MShadow(MS){}


    virtual Vector3f vertex(int iface,int nvert){
        Vector3f vtx=model->vert(iface,nvert);
        world_c[nvert]=vtx;
        varying_uv[nvert]=model->uv(iface,nvert);
        varying_nms[nvert]=model->normal(iface,nvert);
        Vector3f pts=(ViewportMatrix*ProjMatrix*ViewMatrix*vtx.homogeneous()).hnormalized();
        return pts;
    }
    virtual bool fragment(Vector3f bcoor,TGAColor& color,TGAImage&image,Vector3f curp){
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
        Vector3f l=-(light_to-light_p);
        l.normalize();
        l=TBN_*l;
        n.normalize();
        l.normalize();
        float intensity=1;

        //shadow
        //Vector3f ori=(OriM*curp.homogeneous()).hnormalized();
        Vector3f bpts=(uniform_MShadow*curp.homogeneous()).hnormalized(); 
        int id=(bpts[0])*image.get_width()+bpts[1];
        /*
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                cout<<world_c[i][j]<<" ";
            }
            cout<<endl;
        }
        cout<<endl;
        for(int j=0;j<3;j++){
            cout<<ori[j]<<" ";
        }
        cout<<endl;
        cout<<"--------------------"<<endl;
        */
        
        cout<<curp[0]<<" "<<curp[1]<<" "<<curp[2]<<endl;
        cout<<bpts[0]<<" "<<bpts[1]<<" "<<bpts[2]<<endl;
        cout<<endl;
        //cout<<shadowbuffer[id]<<" "<<bpts[2]<<endl;
        if(shadowbuffer[id]>bpts[2]){
            //intensity=max(0.f,(l.dot(n)-0.5f));
            intensity=0;
        }else{
            intensity=max(0.f,(l.dot(n)));
        }
        color=color*intensity;
        return false;
    }
};
