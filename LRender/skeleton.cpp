#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skeleton.h"

/*
 * for skeletal animation, see
 * https://people.rennes.inria.fr/Ludovic.Hoyet/teaching/IMO/05_IMO2016_Skinning.pdf
 */

const char* private_get_extension(const char* filename) {
    const char* dot_pos = strrchr(filename, '.');
    return dot_pos == NULL ? "" : dot_pos + 1;
}

float float_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

Vector3D vec3_new(float x, float y, float z) {
    Vector3D v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

Vector4D quat_new(float x, float y, float z, float w) {
    Vector4D q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

Vector4D quat_slerp(Vector4D a, Vector4D b, float t) {
    float cos_angle = glm::dot(a, b);
    if (cos_angle < 0) {
        b = quat_new(-b.x, -b.y, -b.z, -b.w);
        cos_angle = -cos_angle;
    }
    if (cos_angle > 1 - EPSILON) {
        float x = float_lerp(a.x, b.x, t);
        float y = float_lerp(a.y, b.y, t);
        float z = float_lerp(a.z, b.z, t);
        float w = float_lerp(a.w, b.w, t);
        return quat_new(x, y, z, w);
    }
    else {
        float angle = (float)acos(cos_angle);
        float sin_angle = (float)sin(angle);
        float angle_a = (1 - t) * angle;
        float angle_b = t * angle;
        float factor_a = (float)sin(angle_a) / sin_angle;
        float factor_b = (float)sin(angle_b) / sin_angle;
        float x = factor_a * a.x + factor_b * b.x;
        float y = factor_a * a.y + factor_b * b.y;
        float z = factor_a * a.z + factor_b * b.z;
        float w = factor_a * a.w + factor_b * b.w;
        return quat_new(x, y, z, w);
    }
}

Vector3D vec3_lerp(Vector3D a, Vector3D b, float t) {
    float x = float_lerp(a.x, b.x, t);
    float y = float_lerp(a.y, b.y, t);
    float z = float_lerp(a.z, b.z, t);
    return vec3_new(x, y, z);
}

glm::mat4 mat4_identity(void) {
    glm::mat4 m = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    };
    return m;
}

glm::mat4 mat4_translate(float tx, float ty, float tz) {
    glm::mat4 m = mat4_identity();
    m[0][3] = tx;
    m[1][3] = ty;
    m[2][3] = tz;
    return m;
}

glm::mat4 mat4_from_quat(Vector4D q) {
    glm::mat4 m = mat4_identity();
    float xx = q.x * q.x;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;
    float yy = q.y * q.y;
    float yz = q.y * q.z;
    float yw = q.y * q.w;
    float zz = q.z * q.z;
    float zw = q.z * q.w;

    m[0][0] = 1 - 2 * (yy + zz);
    m[0][1] = 2 * (xy - zw);
    m[0][2] = 2 * (xz + yw);

    m[1][0] = 2 * (xy + zw);
    m[1][1] = 1 - 2 * (xx + zz);
    m[1][2] = 2 * (yz - xw);

    m[2][0] = 2 * (xz - yw);
    m[2][1] = 2 * (yz + xw);
    m[2][2] = 1 - 2 * (xx + yy);

    return m;
}

glm::mat4 mat4_scale(float sx, float sy, float sz) {
    glm::mat4 m = mat4_identity();
    assert(sx != 0 && sy != 0 && sz != 0);
    m[0][0] = sx;
    m[1][1] = sy;
    m[2][2] = sz;
    return m;
}

glm::mat4 mat4_from_trs(Vector3D t, Vector4D r, Vector3D s) {
    glm::mat4 translation = mat4_translate(t.x, t.y, t.z);
    glm::mat4 rotation = mat4_from_quat(r);
    glm::mat4 scale = mat4_scale(s.x, s.y, s.z);
    return translation * rotation * scale;
}

static float mat3_determinant(glm::mat3 m) {
    float a = +m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
    float b = -m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
    float c = +m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    return a + b + c;
}

static glm::mat3 mat3_adjoint(glm::mat3 m) {
    glm::mat3 adjoint;
    adjoint[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]);
    adjoint[0][1] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]);
    adjoint[0][2] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]);
    adjoint[1][0] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]);
    adjoint[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]);
    adjoint[1][2] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]);
    adjoint[2][0] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]);
    adjoint[2][1] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]);
    adjoint[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]);
    return adjoint;
}

glm::mat3 mat3_inverse_transpose(glm::mat3 m) {
    glm::mat3 adjoint, inverse_transpose;
    float determinant, inv_determinant;
    int i, j;

    adjoint = mat3_adjoint(m);
    determinant = mat3_determinant(m);
    inv_determinant = 1 / determinant;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            inverse_transpose[i][j] = adjoint[i][j] * inv_determinant;
        }
    }
    return inverse_transpose;
}

glm::mat3 mat3_from_mat4(glm::mat4 m) {
    glm::mat3 n;
    n[0][0] = m[0][0];
    n[0][1] = m[0][1];
    n[0][2] = m[0][2];
    n[1][0] = m[1][0];
    n[1][1] = m[1][1];
    n[1][2] = m[1][2];
    n[2][0] = m[2][0];
    n[2][1] = m[2][1];
    n[2][2] = m[2][2];
    return n;
}

static void read_inverse_bind(FILE *file, joint_t *joint) {
    char line[LINE_SIZE];
    int items;
    int i;
    items = fscanf(file, " %s", line);
    assert(items == 1 && strcmp(line, "inverse-bind:") == 0);
    for (i = 0; i < 4; i++) {
        items = fscanf(file, " %f %f %f %f",
                       &joint->inverse_bind[i][0],
                       &joint->inverse_bind[i][1],
                       &joint->inverse_bind[i][2],
                       &joint->inverse_bind[i][3]);
        assert(items == 4);
    }
    UNUSED_VAR(items);
}

static void read_translations(FILE *file, joint_t *joint) {
    int items;
    int i;
    items = fscanf(file, " translations %d:", &joint->num_translations);
    assert(items == 1 && joint->num_translations >= 0);
    if (joint->num_translations > 0) {
        int time_size = sizeof(float) * joint->num_translations;
        int value_size = sizeof(Vector3D) * joint->num_translations;
        joint->translation_times = (float*)malloc(time_size);
        joint->translation_values = (Vector3D*)malloc(value_size);
        for (i = 0; i < joint->num_translations; i++) {
            items = fscanf(file, " time: %f, value: [%f, %f, %f]",
                           &joint->translation_times[i],
                           &joint->translation_values[i].x,
                           &joint->translation_values[i].y,
                           &joint->translation_values[i].z);
            assert(items == 4);
        }
    } else {
        joint->translation_times = NULL;
        joint->translation_values = NULL;
    }
    UNUSED_VAR(items);
}

static void read_rotations(FILE *file, joint_t *joint) {
    int items;
    int i;
    items = fscanf(file, " rotations %d:", &joint->num_rotations);
    assert(items == 1 && joint->num_rotations >= 0);
    if (joint->num_rotations > 0) {
        int time_size = sizeof(float) * joint->num_rotations;
        int value_size = sizeof(Vector4D) * joint->num_rotations;
        joint->rotation_times = (float*)malloc(time_size);
        joint->rotation_values = (Vector4D*)malloc(value_size);
        for (i = 0; i < joint->num_rotations; i++) {
            items = fscanf(file, " time: %f, value: [%f, %f, %f, %f]",
                           &joint->rotation_times[i],
                           &joint->rotation_values[i].x,
                           &joint->rotation_values[i].y,
                           &joint->rotation_values[i].z,
                           &joint->rotation_values[i].w);
            assert(items == 5);
        }
    } else {
        joint->rotation_times = NULL;
        joint->rotation_values = NULL;
    }
    UNUSED_VAR(items);
}

static void read_scales(FILE *file, joint_t *joint) {
    int items;
    int i;
    items = fscanf(file, " scales %d:", &joint->num_scales);
    assert(items == 1 && joint->num_scales >= 0);
    if (joint->num_scales > 0) {
        int time_size = sizeof(float) * joint->num_scales;
        int value_size = sizeof(Vector3D) * joint->num_scales;
        joint->scale_times = (float*)malloc(time_size);
        joint->scale_values = (Vector3D*)malloc(value_size);
        for (i = 0; i < joint->num_scales; i++) {
            items = fscanf(file, " time: %f, value: [%f, %f, %f]",
                           &joint->scale_times[i],
                           &joint->scale_values[i].x,
                           &joint->scale_values[i].y,
                           &joint->scale_values[i].z);
            assert(items == 4);
        }
    } else {
        joint->scale_times = NULL;
        joint->scale_values = NULL;
    }
    UNUSED_VAR(items);
}

static joint_t load_joint(FILE *file) {
    joint_t joint;
    int items;

    items = fscanf(file, " joint %d:", &joint.joint_index);
    assert(items == 1);
    items = fscanf(file, " parent-index: %d", &joint.parent_index);
    assert(items == 1);

    read_inverse_bind(file, &joint);
    read_translations(file, &joint);
    read_rotations(file, &joint);
    read_scales(file, &joint);

    UNUSED_VAR(items);
    return joint;
}

static void initialize_cache(skeleton_t *skeleton) {
    int joint_matrix_size = sizeof(glm::mat4) * skeleton->num_joints;
    int normal_matrix_size = sizeof(glm::mat3) * skeleton->num_joints;
    skeleton->joint_matrices = (glm::mat4*)malloc(joint_matrix_size);
    skeleton->normal_matrices = (glm::mat3*)malloc(normal_matrix_size);
    memset(skeleton->joint_matrices, 0, joint_matrix_size);
    memset(skeleton->normal_matrices, 0, normal_matrix_size);
    skeleton->last_time = -1;
}

static skeleton_t* load_ani(const char *filename) {
    skeleton_t *skeleton;
    FILE *file;
    int items;
    int i;

    skeleton = (skeleton_t*)malloc(sizeof(skeleton_t));

    file = fopen(filename, "rb");
    assert(file != NULL);

    items = fscanf(file, " joint-size: %d", &skeleton->num_joints);
    assert(items == 1 && skeleton->num_joints > 0);
    items = fscanf(file, " time-range: [%f, %f]",
                   &skeleton->min_time, &skeleton->max_time);
    assert(items == 2 && skeleton->min_time < skeleton->max_time);

    skeleton->joints = (joint_t*)malloc(sizeof(joint_t) * skeleton->num_joints);
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t joint = load_joint(file);
        assert(joint.joint_index == i);
        skeleton->joints[i] = joint;
    }

    fclose(file);

    initialize_cache(skeleton);

    UNUSED_VAR(items);
    return skeleton;
}

static Vector3D get_translation(joint_t *joint, float frame_time) {
    int num_translations = joint->num_translations;
    float *translation_times = joint->translation_times;
    Vector3D *translation_values = joint->translation_values;

    if (num_translations == 0) {
        return vec3_new(0, 0, 0);
    } else if (frame_time <= translation_times[0]) {
        return translation_values[0];
    } else if (frame_time >= translation_times[num_translations - 1]) {
        return translation_values[num_translations - 1];
    } else {
        int i;
        for (i = 0; i < num_translations - 1; i++) {
            float curr_time = translation_times[i];
            float next_time = translation_times[i + 1];
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                Vector3D curr_translation = translation_values[i];
                Vector3D next_translation = translation_values[i + 1];
                return vec3_lerp(curr_translation, next_translation, t);
            }
        }
        assert(0);
        return vec3_new(0, 0, 0);
    }
}

static Vector4D get_rotation(joint_t *joint, float frame_time) {
    int num_rotations = joint->num_rotations;
    float *rotation_times = joint->rotation_times;
    Vector4D *rotation_values = joint->rotation_values;

    if (num_rotations == 0) {
        return quat_new(0, 0, 0, 1);
    } else if (frame_time <= rotation_times[0]) {
        return rotation_values[0];
    } else if (frame_time >= rotation_times[num_rotations - 1]) {
        return rotation_values[num_rotations - 1];
    } else {
        int i;
        for (i = 0; i < num_rotations - 1; i++) {
            float curr_time = rotation_times[i];
            float next_time = rotation_times[i + 1];
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                Vector4D curr_rotation = rotation_values[i];
                Vector4D next_rotation = rotation_values[i + 1];
                return quat_slerp(curr_rotation, next_rotation, t);
            }
        }
        assert(0);
        return quat_new(0, 0, 0, 1);
    }
}

static Vector3D get_scale(joint_t *joint, float frame_time) {
    int num_scales = joint->num_scales;
    float *scale_times = joint->scale_times;
    Vector3D *scale_values = joint->scale_values;

    if (num_scales == 0) {
        return vec3_new(1, 1, 1);
    } else if (frame_time <= scale_times[0]) {
        return scale_values[0];
    } else if (frame_time >= scale_times[num_scales - 1]) {
        return scale_values[num_scales - 1];
    } else {
        int i;
        for (i = 0; i < num_scales - 1; i++) {
            float curr_time = scale_times[i];
            float next_time = scale_times[i + 1];
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                Vector3D curr_scale = scale_values[i];
                Vector3D next_scale = scale_values[i + 1];
                return vec3_lerp(curr_scale, next_scale, t);
            }
        }
        assert(0);
        return vec3_new(1, 1, 1);
    }
}


void Skeleton::skeleton_load(const char *filename) {
    const char *extension = private_get_extension(filename);
    if (strcmp(extension, "ani") == 0) {
        qDebug() << "1";
        ske = load_ani(filename);
        qDebug() << "2";
    }
}

void Skeleton::skeleton_release(skeleton_t *skeleton) {
    int i;
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t *joint = &skeleton->joints[i];
        free(joint->translation_times);
        free(joint->translation_values);
        free(joint->rotation_times);
        free(joint->rotation_values);
        free(joint->scale_times);
        free(joint->scale_values);
    }
    free(skeleton->joints);
    free(skeleton->joint_matrices);
    free(skeleton->normal_matrices);
    free(skeleton);
}

void Skeleton::skeleton_update_joints(skeleton_t *skeleton, float frame_time) {
    frame_time = (float)fmod(frame_time, skeleton->max_time);
    if (frame_time != skeleton->last_time) {
        int i;
        for (i = 0; i < skeleton->num_joints; i++) {
            joint_t *joint = &skeleton->joints[i];
            Vector3D translation = get_translation(joint, frame_time);
            Vector4D rotation = get_rotation(joint, frame_time);
            Vector3D scale = get_scale(joint, frame_time);
            glm::mat4 joint_matrix;
            glm::mat3 normal_matrix;

            joint->transform = mat4_from_trs(translation, rotation, scale);
            if (joint->parent_index >= 0) {
                joint_t *parent = &skeleton->joints[joint->parent_index];
                joint->transform = parent->transform * joint->transform;
            }

            joint_matrix = joint->transform * joint->inverse_bind;
            normal_matrix = mat3_inverse_transpose(mat3_from_mat4(joint_matrix));
            skeleton->joint_matrices[i] = joint_matrix;
            skeleton->normal_matrices[i] = normal_matrix;
        }
        skeleton->last_time = frame_time;
    }
}

glm::mat4* Skeleton::skeleton_get_joint_matrices(skeleton_t *skeleton) {
    return skeleton->joint_matrices;
}

glm::mat3* Skeleton::skeleton_get_normal_matrices(skeleton_t *skeleton) {
    return skeleton->normal_matrices;
}
