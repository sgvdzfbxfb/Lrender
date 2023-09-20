#include "triangle.h"

inline Bounds3 Union(const Bounds3& b, const Vector3D& p)
{
    Bounds3 ret;
    ret.pMin = Vector3D(std::min(b.pMin.x, p.x), std::min(b.pMin.y, p.y), std::min(b.pMin.z, p.z));
    ret.pMax = Vector3D(std::max(b.pMax.x, p.x), std::max(b.pMax.y, p.y), std::max(b.pMax.z, p.z));
    return ret;
}

Vector3D getBarycentric(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D loc) {
    float alpha = glm::length(glm::cross(loc - v0, loc - v1)) * 0.5;
    float beta  = glm::length(glm::cross(loc - v1, loc - v2)) * 0.5;
    float gmma  = glm::length(glm::cross(loc - v2, loc - v0)) * 0.5;
    return glm::normalize(Vector3D(alpha, beta, gmma));
}

Triangle::Triangle(Vertex _v0, Vertex _v1, Vertex _v2, Texture* _m) : v0(_v0), v1(_v1), v2(_v2), m(_m) {
    updateTrangle();
}
void Triangle::updateTrangle() {
    e1 = v1.worldPos - v0.worldPos;
    e2 = v2.worldPos - v0.worldPos;
    normal = normalize(glm::cross(e1, e2));
    area = glm::length(glm::cross(e1, e2)) * 0.5f;
}
bool Triangle::intersect(const Ray& ray) { return true; }
bool Triangle::intersect(const Ray& ray, float& tnear, uint32_t& index) const {
    return false;
}
Intersection Triangle::getIntersection(Ray ray) {
    Intersection inter;

    if (glm::dot(ray.direction, normal) > 0) return inter;
    double u, v, t_tmp = 0;
    Vector3D pvec = glm::cross(ray.direction, e2);
    double det = glm::dot(e1, pvec);
    if (fabs(det) < EPSILON) return inter;

    double det_inv = 1. / det;
    Vector3D tvec = ray.origin - v0.worldPos;
    u = glm::dot(tvec, pvec) * det_inv;
    if (u < 0 || u > 1) return inter;
    Vector3D qvec = glm::cross(tvec, e1);
    v = glm::dot(ray.direction, qvec) * det_inv;
    if (v < 0 || u + v > 1) return inter;
    t_tmp = glm::dot(e2, qvec) * det_inv;

    if (t_tmp < 0) return inter;

    inter.happened = true;
    inter.coords = ray(t_tmp);
    if (0) {
        Vector3D barPos = getBarycentric(this->v0.worldPos, this->v1.worldPos, this->v2.worldPos, inter.coords);
        Coord2D texUv(0.0, 0.0);
        /*Vector3D bc_corrected = { 0, 0, 0 };
        bc_corrected[0] = barPos[0] / this->v0.clipPos.w;
        bc_corrected[1] = barPos[1] / this->v1.clipPos.w;
        bc_corrected[2] = barPos[2] / this->v2.clipPos.w;
        float Z_n = 1. / (bc_corrected[0] + bc_corrected[1] + bc_corrected[2]);
        for (int i = 0; i < 3; i++) bc_corrected[i] *= Z_n;*/
        texUv += this->v0.texUv * barPos[0]; texUv += this->v1.texUv * barPos[1]; texUv += this->v2.texUv * barPos[2];
        inter.interPointColor = this->m->getColorFromUv(texUv);
    }
    inter.normal = normal;
    inter.distance = t_tmp;
    inter.obj = this;
    inter.m = this->m;
    return inter;
}
void Triangle::getSurfaceProperties(const Vector3D& P, const Vector3D& I, const uint32_t& index, const Vector2D& uv, Vector3D& N, Vector2D& st) const {
    N = normal;
    //        throw std::runtime_error("triangle::getSurfaceProperties not
    //        implemented.");
}
Vector3D Triangle::evalDiffuseColor(const Vector2D&) const {
    return Vector3D(0.5, 0.5, 0.5);
}
Bounds3 Triangle::getBounds() { return Union(Bounds3(v0.worldPos, v1.worldPos), v2.worldPos); }
void Triangle::Sample(Intersection& pos, float& pdf) {
    float x = std::sqrt(get_random_float()), y = get_random_float();
    pos.coords = v0.worldPos * (1.0f - x) + v1.worldPos * (x * (1.0f - y)) + v2.worldPos * (x * y);
    pos.normal = this->normal;
    pdf = 1.0f / area;
}
float Triangle::getArea() {
    return area;
}
bool Triangle::hasEmit() {
    if (glm::length(m->m_emission) > EPSILON) return true;
    else return false;
}

Bounds3::Bounds3()
{
    double minNum = -FLT_MAX;
    double maxNum = FLT_MAX;
    pMax = Vector3D(minNum, minNum, minNum);
    pMin = Vector3D(maxNum, maxNum, maxNum);
}
Bounds3::Bounds3(const Vector3D p) : pMin(p), pMax(p) {}
Bounds3::Bounds3(const Vector3D p1, const Vector3D p2)
{
    pMin = Vector3D(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
    pMax = Vector3D(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
}

Vector3D Bounds3::Diagonal() const { return pMax - pMin; }
int Bounds3::maxExtent() const
{
    Vector3D d = Diagonal();
    if (d.x > d.y && d.x > d.z)
        return 0;
    else if (d.y > d.z)
        return 1;
    else
        return 2;
}

double Bounds3::SurfaceArea() const
{
    Vector3D d = Diagonal();
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

Vector3D Bounds3::Centroid() { return Vector3D(0.5 * pMin.x + 0.5 * pMax.x, 0.5 * pMin.y + 0.5 * pMax.y, 0.5 * pMin.z + 0.5 * pMax.z); }
Bounds3 Bounds3::Intersect(const Bounds3& b)
{
    return Bounds3(Vector3D(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y),
        fmax(pMin.z, b.pMin.z)),
        Vector3D(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y),
            fmin(pMax.z, b.pMax.z)));
}

Vector3D Bounds3::Offset(const Vector3D& p) const
{
    Vector3D o = p - pMin;
    if (pMax.x > pMin.x)
        o.x /= pMax.x - pMin.x;
    if (pMax.y > pMin.y)
        o.y /= pMax.y - pMin.y;
    if (pMax.z > pMin.z)
        o.z /= pMax.z - pMin.z;
    return o;
}

bool Bounds3::Overlaps(const Bounds3& b1, const Bounds3& b2)
{
    bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
    bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
    bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
    return (x && y && z);
}

bool Bounds3::Inside(const Vector3D& p, const Bounds3& b)
{
    return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
        p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
}
inline const Vector3D& Bounds3::operator[](int i) const
{
    return (i == 0) ? pMin : pMax;
}

bool Bounds3::IntersectP(const Ray& ray, const Vector3D& invDir, const std::array<int, 3>& dirIsNeg) const
{
    float t1, t2, t_min_x, t_max_x, t_min_y, t_max_y, t_min_z, t_max_z, t_enter, t_exit;
    t1 = (pMin.x - ray.origin.x) * invDir.x; t2 = (pMax.x - ray.origin.x) * invDir.x;
    t_min_x = fminf(t1, t2); t_max_x = fmaxf(t1, t2);
    t1 = (pMin.y - ray.origin.y) * invDir.y; t2 = (pMax.y - ray.origin.y) * invDir.y;
    t_min_y = fminf(t1, t2); t_max_y = fmaxf(t1, t2);
    t1 = (pMin.z - ray.origin.z) * invDir.z; t2 = (pMax.z - ray.origin.z) * invDir.z;
    t_min_z = fminf(t1, t2); t_max_z = fmaxf(t1, t2);

    t_exit = fminf(fminf(t_max_x, t_max_y), t_max_z);
    if (t_exit < 0) return false;
    t_enter = fmaxf(fmaxf(t_min_x, t_min_y), t_min_z);
    if (t_enter > t_exit)return false;
    return true;
}