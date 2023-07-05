#ifndef SKELETON_H
#define SKELETON_H

#include <qDebug>
#include "dataType.h"
#include "tools.h"

#define LINE_SIZE 256
#define UNUSED_VAR(x) ((void)(x))
#define EPSILON 1e-5f

/* skeleton loading/releasing */
typedef struct {
    int joint_index;
    int parent_index;
    glm::mat4 inverse_bind;
    /* translations */
    int num_translations;
    std::vector<float> translation_times;
    std::vector<Vector3D> translation_values;
    /* rotations */
    int num_rotations;
    std::vector<float> rotation_times;
    std::vector<Vector4D> rotation_values;
    /* scales */
    int num_scales;
    std::vector<float> scale_times;
    std::vector<Vector3D> scale_values;
    /* interpolated */
    glm::mat4 transform;
} joint_t;

struct skeleton_t {
    float min_time = 0.0;
    float max_time = 0.0;
    int num_joints = 0;
    std::vector<joint_t*> joints;
    /* cached result */
    std::vector<glm::mat4> joint_matrices;
    std::vector<glm::mat3> normal_matrices;
    float last_time = 0.0;
};

class Skeleton {
  public:
    skeleton_t ske;
    /* skeleton loading/releasing */
    bool skeleton_load(std::string filename);

    /* joint updating/retrieving */
    void skeleton_update_joints(skeleton_t* skeleton, float frame_time);
    std::vector<glm::mat4> skeleton_get_joint_matrices(skeleton_t* skeleton);
    std::vector<glm::mat3> skeleton_get_normal_matrices(skeleton_t* skeleton);
};
#endif
