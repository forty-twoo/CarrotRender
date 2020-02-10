#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include<Eigen/Dense>
#include "tgaimage.h"
using namespace Eigen;

class Model {
private:
	std::vector<Vector3f> verts_;
	std::vector<std::vector<Vector3i> > faces_; // attention, this Vec3i means vertex/uv/normal
	std::vector<Vector3f> norms_;
	std::vector<Vector2f> uv_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vector3f normal(int iface,int nvert);
	Vector3f normalmap_coor(Vector2f uvf);
	Vector3f vert(int i);
	Vector3f vert(int iface,int nvert);
	Vector2f uv(int iface,int nvert);
	TGAColor diffuse(Vector2f uv);
	std::vector<int> face(int idx);
};

#endif //__MODEL_H__