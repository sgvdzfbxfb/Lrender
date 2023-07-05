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

float float_lerp(float a, float b, float t) {
    return a + (b - a) * t;
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
    return Vector3D(x, y, z);
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
    //return rotation * translation * scale;
    return scale * rotation * translation;
}

static void read_inverse_bind(FILE* file, joint_t* joint) {
    char line[LINE_SIZE];
    int items;
    int i;
    items = fscanf(file, " %s", line);
    assert(items == 1 && strcmp(line, "inverse-bind:") == 0);
    for (i = 0; i < 4; i++) {
        items = fscanf(file, " %f %f %f %f\n",
                       &joint->inverse_bind[i][0],
                       &joint->inverse_bind[i][1],
                       &joint->inverse_bind[i][2],
                       &joint->inverse_bind[i][3]);
        assert(items == 4);
    }
    UNUSED_VAR(items);
}

static void read_translations(FILE* file, joint_t* joint) {
    int items;
    int i;

    items = fscanf(file, " translations %d:\n", &joint->num_translations);
    assert(items == 1 && joint->num_translations >= 0);
    if (joint->num_translations > 0) {
        int time_size = sizeof(float) * joint->num_translations;
        int value_size = sizeof(Vector3D) * joint->num_translations;
        joint->translation_times.resize(time_size, 0.0);
        joint->translation_values.resize(value_size, Vector3D(0.0, 0.0, 0.0));
        for (i = 0; i < joint->num_translations; i++) {
            items = fscanf(file, " time: %f, value: [%f, %f, %f]\n",
                           &joint->translation_times.at(i),
                           &joint->translation_values.at(i).x,
                           &joint->translation_values.at(i).y,
                           &joint->translation_values.at(i).z);
            assert(items == 4);
        }
    }
    UNUSED_VAR(items);
}

static void read_rotations(FILE* file, joint_t* joint) {
    int items;
    int i;
    items = fscanf(file, " rotations %d:\n", &joint->num_rotations);
    assert(items == 1 && joint->num_rotations >= 0);
    if (joint->num_rotations > 0) {
        int time_size = sizeof(float) * joint->num_rotations;
        int value_size = sizeof(Vector4D) * joint->num_rotations;
        joint->rotation_times.resize(time_size, 0.0);
        joint->rotation_values.resize(value_size, Vector4D(0.0, 0.0, 0.0, 0.0));
        for (i = 0; i < joint->num_rotations; i++) {
            items = fscanf(file, " time: %f, value: [%f, %f, %f, %f]\n",
                           &joint->rotation_times.at(i),
                           &joint->rotation_values.at(i).x,
                           &joint->rotation_values.at(i).y,
                           &joint->rotation_values.at(i).z,
                           &joint->rotation_values.at(i).w);
            assert(items == 5);
        }
    }
    UNUSED_VAR(items);
}

static void read_scales(FILE* file, joint_t* joint) {
    int items;
    int i;
    items = fscanf(file, " scales %d:\n", &joint->num_scales);
    assert(items == 1 && joint->num_scales >= 0);
    if (joint->num_scales > 0) {
        int time_size = sizeof(float) * joint->num_scales;
        int value_size = sizeof(Vector3D) * joint->num_scales;
        joint->scale_times.resize(time_size, 0.0);
        joint->scale_values.resize(value_size, Vector3D(0.0, 0.0, 0.0));
        for (i = 0; i < joint->num_scales; i++) {
            items = fscanf(file, " time: %f, value: [%f, %f, %f]\n",
                           &joint->scale_times.at(i),
                           &joint->scale_values.at(i).x,
                           &joint->scale_values.at(i).y,
                           &joint->scale_values.at(i).z);
            assert(items == 4);
        }
    }
    UNUSED_VAR(items);
}

static joint_t* load_joint(FILE* file) {
    joint_t* joint = new joint_t;
    int items;

    items = fscanf(file, " joint %d:\n", &joint->joint_index);
    assert(items == 1);
    items = fscanf(file, " parent-index: %d\n", &joint->parent_index);
    assert(items == 1);

    read_inverse_bind(file, joint);
    read_translations(file, joint);
    read_rotations(file, joint);
    read_scales(file, joint);

    UNUSED_VAR(items);
    return joint;
}

static void initialize_cache(skeleton_t skeleton) {
    skeleton.last_time = -1;
}

static skeleton_t load_ani(std::string filename) {
    skeleton_t* skeleton;
    FILE* file;
    int items;
    int i;

    skeleton = new skeleton_t;
    char ch[100]; strcpy(ch, filename.c_str());
    file = fopen(ch, "rb");
    assert(file != NULL);

    items = fscanf(file, "joint-size: %d\n", &(skeleton->num_joints));
    items = fscanf(file, "time-range: [%f, %f]\n", &(skeleton->min_time), &(skeleton->max_time));
    for (i = 0; i < skeleton->num_joints; i++) {
        joint_t* joint = load_joint(file);
        assert(joint->joint_index == i);
        skeleton->joints.push_back(joint);
    }

    fclose(file);
    initialize_cache(*(skeleton));
    UNUSED_VAR(items);
    return *(skeleton);
}

static Vector3D get_translation(joint_t* joint, float frame_time) {
    int num_translations = joint->num_translations;
    std::vector<float> translation_times = joint->translation_times;
    std::vector <Vector3D> translation_values = joint->translation_values;
    if (num_translations == 0) {
        return Vector3D(0.0, 0.0, 0.0);
    } else if (frame_time <= translation_times.at(0)) {
        return translation_values.at(0);
    } else if (frame_time >= translation_times.at(num_translations - 1)) {
        return translation_values.at(num_translations - 1);
    } else {
        int i;
        for (i = 0; i < num_translations - 1; i++) {
            float curr_time = translation_times.at(i);
            float next_time = translation_times.at(i + 1);
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                Vector3D curr_translation = translation_values.at(i);
                Vector3D next_translation = translation_values.at(i + 1);
                return vec3_lerp(curr_translation, next_translation, t);
            }
        }
        assert(0);
        return Vector3D(0.0, 0.0, 0.0);
    }
}

static Vector4D get_rotation(joint_t* joint, float frame_time) {
    int num_rotations = joint->num_rotations;
    std::vector<float> rotation_times = joint->rotation_times;
    std::vector <Vector4D> rotation_values = joint->rotation_values;

    if (num_rotations == 0) {
        return quat_new(0, 0, 0, 1);
    } else if (frame_time <= rotation_times.at(0)) {
        return rotation_values.at(0);
    } else if (frame_time >= rotation_times.at(num_rotations - 1)) {
        return rotation_values.at(num_rotations - 1);
    } else {
        int i;
        for (i = 0; i < num_rotations - 1; i++) {
            float curr_time = rotation_times.at(i);
            float next_time = rotation_times.at(i + 1);
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                Vector4D curr_rotation = rotation_values.at(i);
                Vector4D next_rotation = rotation_values.at(i + 1);
                return quat_slerp(curr_rotation, next_rotation, t);
            }
        }
        assert(0);
        return quat_new(0, 0, 0, 1);
    }
}

static Vector3D get_scale(joint_t* joint, float frame_time) {
    int num_scales = joint->num_scales;
    std::vector<float> scale_times = joint->scale_times;
    std::vector <Vector3D> scale_values = joint->scale_values;

    if (num_scales == 0) {
        return Vector3D(1.0, 1.0, 1.0);
    } else if (frame_time <= scale_times.at(0)) {
        return scale_values.at(0);
    } else if (frame_time >= scale_times.at(num_scales - 1)) {
        return scale_values.at(num_scales - 1);
    } else {
        int i;
        for (i = 0; i < num_scales - 1; i++) {
            float curr_time = scale_times.at(i);
            float next_time = scale_times.at(i + 1);
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                Vector3D curr_scale = scale_values.at(i);
                Vector3D next_scale = scale_values.at(i + 1);
                return vec3_lerp(curr_scale, next_scale, t);
            }
        }
        assert(0);
        return Vector3D(1.0, 1.0, 1.0);
    }
}


bool Skeleton::skeleton_load(std::string filename) {
    std::vector<std::string> aniFile;
    getAllTypeFiles(filename, aniFile, "ani");
    if (aniFile.size() > 0) {
        ske = load_ani(aniFile.at(0));
    }
}

void Skeleton::skeleton_update_joints(skeleton_t* skeleton, float frame_time) {
    frame_time = fmod(frame_time, skeleton->max_time);
    if (frame_time != skeleton->last_time) {
        for (int i = 0; i < skeleton->num_joints; i++) {
            joint_t* joint = skeleton->joints.at(i);
            Vector3D translation = get_translation(joint, frame_time);
            Vector4D rotation = get_rotation(joint, frame_time);
            Vector3D scale = get_scale(joint, frame_time);
            glm::mat4 joint_matrix;
            glm::mat3 normal_matrix;

            joint->transform = mat4_from_trs(translation, rotation, scale);
            if (joint->parent_index >= 0) {
                joint_t* parent = skeleton->joints.at(joint->parent_index);
                joint->transform = joint->transform * parent->transform;
            }
            joint_matrix = joint->inverse_bind * joint->transform;
            normal_matrix = glm::mat3(glm::transpose(glm::inverse(joint_matrix)));
            if (skeleton->joint_matrices.size() <= i) {
                skeleton->joint_matrices.push_back(joint_matrix);
                skeleton->normal_matrices.push_back(normal_matrix);
            }
            else {
                skeleton->joint_matrices.at(i) = joint_matrix;
                skeleton->normal_matrices.at(i) = normal_matrix;
            }
        }
        skeleton->last_time = frame_time;
    }
}
