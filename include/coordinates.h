#ifndef COORDINATES_H
#define COORDINATES_H
#include "glm/glm.hpp"

class Vector3 {
public:
    float x;
    float y;
    float z;
    Vector3(float _x, float _y, float _z);
    explicit Vector3(glm::vec3 position);

    Vector3& operator= (const glm::vec3& position);
    Vector3& operator= (const Vector3& position);
    Vector3& operator+ (const Vector3& position);
    Vector3& operator+ (const glm::vec3& position);
    Vector3& operator- (const Vector3& position);
    Vector3& operator- (const glm::vec3& position);
};

class Position : public Vector3 {
    glm::mat4 pos_matrix = glm::mat4(1.0f);
public:
    glm::mat4& get_transformation_matrix();
    using Vector3::Vector3;
    using Vector3::operator=;
};

class Scale : public Vector3 {
    glm::mat4 scale_matrix = glm::mat4(1.0f);
public:
    using Vector3::Vector3;
    glm::mat4& get_transformation_matrix();
};

class Rotation {
    glm::mat4 rot_matrix = glm::mat4(1.0f);
public:
    void euler_2_quat();
    void quat_2_euler();

    float i;
    float j;
    float k;
    float r;

    float x;
    float y;
    float z;

    glm::mat4& get_transformation_matrix();

    Rotation(float _r, float _i, float _j, float _k);
    Rotation(float _x, float _y, float _z);
    Rotation();

    Rotation operator* (const Rotation& rot) const;
    Rotation& operator= (const glm::vec3& rotation);
    Rotation& operator*= (const Rotation& rot);
    [[nodiscard]] Rotation conjugate() const;

    static void rotate_point(float _x, float _y, float _z, glm::vec3& point);
};

class Transform {
public:
    Position position{0, 0, 0};
    Rotation rotation{1, 0, 0, 0};
    Scale scale{1, 1, 1};
};


#endif //COORDINATES_H
