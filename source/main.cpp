#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

float *shadowbuffer = NULL;
Model *model = NULL;
const int width = 1024;
const int height = 1024;

Vec3f light_dir(1, 2, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct PhongShader : public IShader
{
    mat<2, 3, float> uvs;
    // mat<4, 4, float> NormalMat;
    mat<4, 4, float> NormalMat_IT;
    mat<3, 3, float> Varying_normals;
    mat<3, 3, float> Varying_pos;
    mat<4, 4, float> Mshadow;
    PhongShader(Matrix MIT, Matrix MS) : uvs(), NormalMat_IT(MIT), Varying_normals(), Varying_pos(), Mshadow(MS) {}

    virtual Vec4f vertex(int iface, int nthvert)
    {
        uvs.set_col(nthvert, model->uv(iface, nthvert));
        Varying_normals.set_col(nthvert, proj<3>(NormalMat_IT * embed<4>(model->normal(iface, nthvert), 1.f)));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));   //这里的embed就是简单的Vec3f -> Vec4f
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
        Varying_pos.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        Vec2f uv = uvs * bar;
        Vec3f bn = (Varying_normals * bar).normalize();
        Vec4f sb_pos = Mshadow * embed<4>(Varying_pos * bar);
        sb_pos = sb_pos / sb_pos[3];

        int idx = int(sb_pos[0]) + int(sb_pos[1]) * width;
        mat<3, 3, float> A;
        A[0] = Varying_pos.col(1) - Varying_pos.col(0);
        A[1] = Varying_pos.col(2) - Varying_pos.col(0);
        A[2] = bn;

        mat<3, 3, float> A_I = A.invert_transpose().transpose();
        Vec3f i = A_I * Vec3f(uvs[0][1] - uvs[0][0], uvs[0][2] - uvs[0][0], 0);
        Vec3f j = A_I * Vec3f(uvs[1][1] - uvs[1][0], uvs[1][2] - uvs[1][0], 0);

        mat<3, 3, float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        Vec3f normal = (B * model->normal(uv)).normalize();

        float shadow = .3 + .7 * (shadowbuffer[idx] < sb_pos[2] + 20);
        Vec3f reflect = (normal * (normal * light_dir * 2.f) - light_dir).normalize();
        float diff = std::max(0.f, std::min(255.f, normal * light_dir));
        float spec = pow(std::max(0.f, reflect.z), model->specular(uv));
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i = 0; i < 3; i++)
            color[i] = std::min<float>(20 + c[i] * shadow * (1 * diff), 255);
        return false; // no, we do not discard this pixel
    }
};
struct DepthShader : public IShader
{
    mat<3, 3, float> Varying_pos;

    DepthShader() : Varying_pos() {}
    virtual Vec4f vertex(int iface, int nthvert)
    {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
        Varying_pos.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }
    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        Vec3f p = Varying_pos * bar;
        color = TGAColor(255, 255, 255) * (p.z / 255.f);
        return false;
    }
};
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

    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye - center).norm());
    light_dir.normalize();

    float *zbuffer = new float[width * height];
    shadowbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++)
    {
        zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
    }
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage depth(width, height, TGAImage::RGB);
    DepthShader ShadowMap;

    {
        TGAImage depth(width, height, TGAImage::RGB);
        lookat(light_dir, center, up);
        viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        projection(0);
        for (int i = 0; i < model->nfaces(); i++)
        {
            Vec4f screen_coords[3];
            for (int j = 0; j < 3; j++)
            {
                screen_coords[j] = ShadowMap.vertex(i, j);
            }
            triangle(screen_coords, ShadowMap, depth, shadowbuffer);
        }
        depth.flip_vertically(); // to place the origin in the bottom left corner of the image
        depth.write_tga_file("depth.tga");
    }

    Matrix M = Viewport * Projection * ModelView;

    {
        TGAImage frame(width, height, TGAImage::RGB);
        lookat(eye, center, up);
        viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        projection(-1.f / (eye - center).norm());
        PhongShader shader((Projection * ModelView).invert_transpose(), M * (Viewport * Projection * ModelView).invert_transpose().transpose());

        for (int i = 0; i < model->nfaces(); i++)
        {
            Vec4f screen_coords[3];
            for (int j = 0; j < 3; j++)
            {
                screen_coords[j] = shader.vertex(i, j);
            }
            triangle(screen_coords, shader, frame, zbuffer);
        }

        frame.flip_vertically(); // to place the origin in the bottom left corner of the image
        frame.write_tga_file("framebuffer.tga");
    }

    delete model;
    delete[] zbuffer;
    delete[] shadowbuffer;
    return 0;
}