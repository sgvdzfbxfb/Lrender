#ifndef TEXTURE_H
#define TEXTURE_H
#include <QString>
#include <QImage>
#include <qDebug>
#include "corecrt_math_defines.h"
#include "lrenderBasicCore.h"

enum TextureType { DIFFUSE_T, MICROFACET_T, MIRROR_T };

class Texture
{
private:
    int imgWidth = 0;
    int imgHeight = 0;
    QImage texture;

    // Compute reflection direction
    Vector3D reflect(const Vector3D& I, const Vector3D& N) const;

    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    Vector3D refract(const Vector3D& I, const Vector3D& N, const float& ior) const;

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3D& I, const Vector3D& N, const float& ior, float& kr) const;

    Vector3D toWorld(const Vector3D& a, const Vector3D& N);

public:
    QString path;
    Texture(TextureType t = DIFFUSE_T, Vector3D e = Vector3D(0, 0, 0));
    bool getTexture(QString path);
    Color getColorFromUv(Coord2D coord);

    TextureType m_type;
    //Vector3D m_color;
    Vector3D m_emission;
    float ior = 0.0;
    Vector3D Kd = Vector3D(0.725f, 0.71f, 0.68f);
    Vector3D Ks = Vector3D(0.f, 0.f, 0.f);
    float specularExponent = 0.0;
    //Texture tex;

    inline TextureType getType();
    //inline Vector3D getColor();
    inline Vector3D getColorAt(double u, double v);
    Vector3D getEmission();
    bool hasEmission();

    // sample a ray by Texture properties
    Vector3D sample(const Vector3D& wi, const Vector3D& N);
    // given a ray, calculate the PdF of this ray
    float pdf(const Vector3D& wi, const Vector3D& wo, const Vector3D& N);
    // given a ray, calculate the contribution of this ray
    Vector3D eval(const Vector3D& wi, const Vector3D& wo, const Vector3D& N);
};

#endif // TEXTURE_H
