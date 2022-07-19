#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include "tgaimage.h"
#include "geometry.h"

class Model
{
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int>> faces_;
	std::vector<std::vector<int>> texs_;
	std::vector<Vec2f> tex_coord{};
	TGAImage diffusemap{};

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f uv(int i) ;
	std::vector<int> face(int idx);
	std::vector<int> tex(int itex);
};

#endif //__MODEL_H__