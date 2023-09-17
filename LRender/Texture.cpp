#include "texture.h"
#include <QDebug>

Texture::Texture() {}

bool Texture::getTexture(QString path)
{
    this->path = path;
    if(texture.load(path))
    {
        texture = texture.mirrored();
        imgWidth = texture.width();
        imgHeight = texture.height();
        return true;
    }
    return false;
}

Color Texture::getColorFromUv(Coord2D coord)
{
    int x = static_cast<int>(coord.x * imgWidth - 0.5f) % imgWidth;
    int y = static_cast<int>(coord.y * imgHeight - 0.5f) % imgHeight;
    x = x < 0 ? imgWidth + x : x;
    y = y < 0 ? imgHeight + y : y;
    return Color(texture.pixelColor(x, y).red() / 255.f, texture.pixelColor(x, y).green() / 255.f, texture.pixelColor(x, y).blue() / 255.f);
}

Vector3D Texture::reflect(const Vector3D& I, const Vector3D& N) const
{
    return I - 2 * glm::dot(I, N) * N;
}

Vector3D Texture::refract(const Vector3D& I, const Vector3D& N, const float& ior) const
{
    float cosi = clamp(-1, 1, glm::dot(I, N));
    float etai = 1, etat = ior;
    Vector3D n = N;
    if (cosi < 0) { cosi = -cosi; }
    else { std::swap(etai, etat); n = -N; }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? Vector3D(0.f) : Vector3D(eta * I + (eta * cosi - sqrtf(k)) * n);
}

void Texture::fresnel(const Vector3D& I, const Vector3D& N, const float& ior, float& kr) const
{
    float cosi = clamp(-1, 1, glm::dot(I, N));
    float etai = 1, etat = ior;
    if (cosi > 0) { std::swap(etai, etat); }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}

Vector3D Texture::toWorld(const Vector3D& a, const Vector3D& N) {
    Vector3D B, C;
    if (std::fabs(N.x) > std::fabs(N.y)) {
        float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
        C = Vector3D(N.z * invLen, 0.0f, -N.x * invLen);
    }
    else {
        float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
        C = Vector3D(0.0f, N.z * invLen, -N.y * invLen);
    }
    B = glm::cross(C, N);
    return a.x * B + a.y * C + a.z * N;
}

Texture::Texture(TextureType t, Vector3D e) {
    m_type = t;
    //m_color = c;
    m_emission = e;
}

TextureType Texture::getType() { return m_type; }
///Vector3D Texture::getColor() { return m_color; }
Vector3D Texture::getEmission() { return m_emission; }
bool Texture::hasEmission() {
    if (glm::length(m_emission) > EPSILON) return true;
    else return false;
}

Vector3D Texture::getColorAt(double u, double v) {
    return Vector3D();
}

Vector3D Texture::sample(const Vector3D& wi, const Vector3D& N) {
    switch (m_type) {
    case DIFFUSE_T:
    {
        // uniform sample on the hemisphere
        float x_1 = get_random_float(), x_2 = get_random_float();
        float z = std::fabs(1.0f - 2.0f * x_1);
        float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
        Vector3D localRay(r * std::cos(phi), r * std::sin(phi), z);
        return toWorld(localRay, N);

        break;
    }
    case MIRROR_T:
    {
        Vector3D localRay = reflect(wi, N);
        return localRay;
        break;
    }
    case MICROFACET_T:
    {
        // uniform sample on the hemisphere
        float x_1 = get_random_float(), x_2 = get_random_float();
        float z = std::fabs(1.0f - 2.0f * x_1);
        float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
        Vector3D localRay(r * std::cos(phi), r * std::sin(phi), z);
        return toWorld(localRay, N);

        break;
    }
    }
}

float Texture::pdf(const Vector3D& wi, const Vector3D& wo, const Vector3D& N) {
    switch (m_type) {
    case DIFFUSE_T:
    {
        // uniform sample probability 1 / (2 * PI)
        if (glm::dot(wo, N) > 0.0f)
            return 0.5f / M_PI;
        else
            return 0.0f;
        break;
    }
    case MIRROR_T:
    {
        if (glm::dot(wo, N) > 0.0f)
            return 1.0f;
        else
            return 0.0f;
        break;
    }
    case MICROFACET_T:
    {
        // uniform sample probability 1 / (2 * PI)
        if (glm::dot(wo, N) > 0.0f)
            return 0.5f / M_PI;
        else
            return 0.0f;
        break;
    }
    }
}

Vector3D Texture::eval(const Vector3D& wi, const Vector3D& wo, const Vector3D& N) {
    switch (m_type) {
    case DIFFUSE_T:
    {
        // calculate the contribution of diffuse   model
        float cosalpha = glm::dot(N, wo);
        if (cosalpha > 0.0f) {
            Vector3D diffuse(Kd.x / M_PI, Kd.y / M_PI, Kd.z / M_PI);
            return diffuse;
        }
        else
            return Vector3D(0.0f);
        break;
    }
    case MIRROR_T:
    {
        float cosalpha = glm::dot(N, wo);
        if (cosalpha > 0.0f)
        {
            float divisor = cosalpha;
            if (divisor < 0.001) return Vector3D(0.0f);
            Vector3D mirror(1.0 / divisor);
            float F;
            fresnel(wi, N, ior, F);
            return F * mirror;
        }
        else
            return Vector3D(0.0f);
        break;
    }
    case MICROFACET_T:
    {
        float cosalpha = glm::dot(N, wo);
        if (cosalpha > 0.0f) {
            float F, G, D;

            fresnel(wi, N, ior, F);

            float Roughness = 1.f;
            auto G_func = [&](const float& Roughness, const Vector3D& wi, const Vector3D& wo, const Vector3D& N) {
                float A_wi, A_wo;
                A_wi = (-1 + sqrt(1 + Roughness * Roughness * pow(tan(acos(glm::dot(wi, N))), 2))) / 2;
                A_wo = (-1 + sqrt(1 + Roughness * Roughness * pow(tan(acos(glm::dot(wo, N))), 2))) / 2;
                float divisor = (1 + A_wi + A_wo);
                if (divisor < 0.001) return 1.f;
                else return 1.f / divisor;
            };
            G = G_func(Roughness, -wi, wo, N);

            auto D_func = [&](const float& Roughness, const Vector3D& h, const Vector3D& N) {
                float cos_sita = glm::dot(h, N);
                float divisor = (M_PI * pow(1.0 + cos_sita * (Roughness * Roughness - 1), 2));
                if (divisor < 0.001) return 1.f;
                else return (Roughness * Roughness) / divisor;
            };
            Vector3D h = normalize(-wi + wo);
            D = D_func(Roughness, h, N);

            Vector3D diffuse = (Vector3D(1.f) - F) * Vector3D(Kd.x / M_PI, Kd.y / M_PI, Kd.z / M_PI);
            Vector3D specular;
            float divisor = ((4 * (glm::dot(N, -wi)) * (glm::dot(N, wo))));
            if (divisor < 0.001) specular = Vector3D(1);
            else specular = Vector3D(F * G * D / divisor);
            return diffuse + specular;
        }
        else return Vector3D(0.0f);
        break;
    }
    }
}
