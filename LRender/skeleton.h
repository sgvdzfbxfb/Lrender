#ifndef SKELETON_H
#define SKELETON_H

#include "dataType.h"

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
    float* translation_times;
    Vector3D* translation_values;
    /* rotations */
    int num_rotations;
    float* rotation_times;
    Vector4D* rotation_values;
    /* scales */
    int num_scales;
    float* scale_times;
    Vector3D* scale_values;
    /* interpolated */
    glm::mat4 transform;
} joint_t;

struct skeleton_t {
    float min_time;
    float max_time;
    int num_joints;
    joint_t* joints;
    /* cached result */
    glm::mat4* joint_matrices;
    glm::mat3* normal_matrices;
    float last_time;
};

class Skeleton {
  public:
    float min_time;
    float max_time;
    int num_joints;
    joint_t* joints;
    /* cached result */
    glm::mat4* joint_matrices;
    glm::mat3* normal_matrices;
    float last_time;
    /* skeleton loading/releasing */
    skeleton_t* skeleton_load(const char* filename);
    void skeleton_release(skeleton_t* skeleton);

    /* joint updating/retrieving */
    void skeleton_update_joints(skeleton_t* skeleton, float frame_time);
    glm::mat4* skeleton_get_joint_matrices(skeleton_t* skeleton);
    glm::mat3* skeleton_get_normal_matrices(skeleton_t* skeleton);
};
#endif
