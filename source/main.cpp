#include <vector>
#include <iostream>
#include <vector>
#include <limits>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
Model *model = NULL;
TGAImage diffuse(1024, 1024, TGAImage::RGB);
const int width = 1280;
const int height = 1024;

Vec3f barycentric(Vec3f *pts, Vec3f P)
{
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1)
		return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec3f *pts, Vec2f *uv, float *z_buffer, TGAImage &image, TGAColor color)
{
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = std::max(0, std::min(bboxmin.x, (int)pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, (int)pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, (int)pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, (int)pts[i].y));
		uv[i].u *= 1024;
		uv[i].v *= 1024;
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
				continue;
			P.z = 0;
			Vec2f uv_pos;
			for (int i = 0; i < 3; i++)
			{
				P.z += pts[i].z * bc_screen.raw[i];
				uv_pos.u += uv[i].u * bc_screen.raw[i];
				uv_pos.v += uv[i].v * bc_screen.raw[i];
			}
			if (z_buffer[int(P.x + P.y * width)] < P.z)
			{
				z_buffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, diffuse.get(uv_pos.u, uv_pos.v) * color);
			}
		}
	}
}

int main(int argc, char **argv)
{
	model = new Model("../obj/african_head.obj");
	TGAImage frame(width, height, TGAImage::RGB);
	diffuse.read_tga_file("../image//african_head_diffuse.tga");
	// Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)};
	// triangle(pts, frame, TGAColor(255, 0, 0, 255));
	Vec3f light_Dir(0, 0, -1);
	float *z_buffer = new float[width * height];
	for (int i = 0; i < width * height; i++)
		z_buffer[i] = std::numeric_limits<float>::min();
	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec3f screen_pos[3];
		Vec3f world_Pos[3];
		Vec2f uv[3];
		std::vector<int> face = model->face(i);
		std::vector<int> tex = model->tex(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f pos = model->vert(face[j]);
			screen_pos[j] = Vec3f((pos.x + 1.) * width / 2. + .5, (pos.y + 1.) * height / 2. + .5, (pos.z + 1.) / 2);
			world_Pos[j] = pos;
			uv[j] = model->uv(tex[j]);
		}
		Vec3f world_normal = (world_Pos[2] - world_Pos[0]) ^ (world_Pos[1] - world_Pos[0]);
		world_normal.normalize();
		float intensity = world_normal * light_Dir;
		if (intensity > 0)
			triangle(screen_pos, uv, z_buffer, frame, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	}
	frame.flip_vertically(); // to place the origin in the bottom left corner of the image
	frame.write_tga_file("framebuffer.tga");
	delete model;
	delete[] z_buffer;
	return 0;
}