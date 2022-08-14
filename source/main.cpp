#include <vector>
#include <iostream>
#include <mutex>
#include <thread>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

std::mutex mtx;
float *shadowbuffer = nullptr;
Model *model = nullptr;
constexpr int width = 1024;
constexpr int height = 1024;

Vec3f light_dir(1, 2, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct ZShader : public IShader
{
    mat<3, 3, float> Varying_pos;
    Vec4f vertex(int iface, int nthvert) override
    {
        Vec4f gl_vertex = Viewport * Projection * ModelView * embed<4>(model->vert(iface, nthvert));
        Varying_pos.set_col(nthvert, proj<3>(gl_vertex / gl_vertex[3]));
        return gl_vertex;
    }

    bool fragment(Vec3f bar, TGAColor &color) override
    {
        Vec3f p = Varying_pos * bar;
        color = TGAColor(255, 255, 255) * (p.z / 255.f);
        return false;
    }
};

float max_elevation_angle(float *zbuffer, Vec2f p, Vec2f dir)
{
    float maxangle = 0;
    for (float t = 0; t < 1000; t += 1.)
    {
        Vec2f cur = p + dir * t;
        if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0)
            return maxangle;
        float centerZbuffer = zbuffer[(int)p.x + (int)p.y * width];
        float distance = (p - cur).norm();
        if (distance < 1.f)
            continue;

        float elevation = zbuffer[int(cur.x) + int(cur.y) * width] - centerZbuffer;
        maxangle = std::max(maxangle, atanf(elevation / distance));
    }
    return maxangle;
}

void func1(float *zbuffer, int n, TGAImage &frame)
{
    // std::lock_guard<std::mutex> locker(mtx);
    // std::cout << "OK" << std::endl;
    int step = width / 4;         // 1024 / 4 = 256
    int x_start = (n % 4) * step; // n=4, x_start = 256
    int y_start = (n / 4) * step; // n=0, y_start = 0, y_end = 256;
    int x_end = std::min(width, ((n % 4) + 1) * step);
    int y_end = std::min(height, ((n / 4) + 1) * step);

    for (int x = x_start; x < x_end; x++)
    {
        for (int y = y_start; y < y_end; y++)
        {
            if (zbuffer[x + y * width] < -1e5)
                continue;
            float total = 0;
            for (float a = 0; a < 3.1415926 * 2 - 1e-4; a += 3.1415926 / 4)
            {
                total += 3.1415926 / 2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
            }
            total /= (3.1415926 / 2) * 8;
            total = pow(total, 5);
            frame.set(x, y, TGAColor(total * 255, total * 255, total * 255));
        }
    }
}

int main(int argc, char **argv)
{
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("../obj/african_head/african_head.obj");
    }

    float *zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++)
    {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    TGAImage frame(width, height, TGAImage::RGB);
    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye - center).norm());

    ZShader zshader;
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vec4f screen_pos[3];
        for (int j = 0; j < 3; j++)
        {
            screen_pos[j] = zshader.vertex(i, j);
        }
        triangle(screen_pos, zshader, frame, zbuffer);
    }
    frame.flip_vertically();
    frame.write_tga_file("depth.tga");
    frame.clear();

    std::thread ths[16];
    for (int i = 0; i < 16; i++)
        ths[i] = std::thread(func1, zbuffer, i, std::ref(frame));

    for (int i = 0; i < 16; i++)
        ths[i].join();

    frame.flip_vertically();
    frame.write_tga_file("frame.tga");

    delete model;
    delete[] zbuffer;
    delete[] shadowbuffer;

    return 0;
}