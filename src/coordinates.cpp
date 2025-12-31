#include "coordinates.h"

#include <iostream>

Vector3::Vector3(const float _x, const float _y, const float _z) {
    x = _x;
    y = _y;
    z = _z;
}

Vector3::Vector3(const glm::vec3 position) {
    x = position.x;
    y = position.y;
    z = position.z;
}

Vector3& Vector3::operator=(const glm::vec3& position) {
    x = position.x;
    y = position.y;
    z = position.z;
    return *this;
}

Vector3& Vector3::operator=(const Vector3& position) = default;

Vector3& Vector3::operator+ (const Vector3& position) {
    x += position.x;
    y += position.y;
    z += position.z;
    return *this;
}


Vector3& Vector3::operator+ (const glm::vec3& position) {
    x += position.x;
    y += position.y;
    z += position.z;
    return *this;
}


Vector3& Vector3::operator- (const Vector3& position) {
    x -= position.x;
    y -= position.y;
    z -= position.z;
    return *this;
}

Vector3& Vector3::operator- (const glm::vec3& position) {
    x -= position.x;
    y -= position.y;
    z -= position.z;
    return *this;
}

glm::mat4& Position::get_transformation_matrix() {
    pos_matrix[3][0] = x;
    pos_matrix[3][1] = y;
    pos_matrix[3][2] = z;
    return pos_matrix;
}

glm::mat4& Scale::get_transformation_matrix() {
    scale_matrix[0][0] = x;
    scale_matrix[1][1] = y;
    scale_matrix[2][2] = z;
    return scale_matrix;
}

glm::mat4& Rotation::get_transformation_matrix() {
    euler_2_quat();

    rot_matrix[0][0] = 1 - 2 * (j * j + k * k);
    rot_matrix[1][0] = 2 * (i * j - k * r);
    rot_matrix[2][0] = 2 * (i * k + j * r);

    rot_matrix[0][1] = 2 * (i * j + r * k);
    rot_matrix[1][1] = 1 - 2 * (i * i + k * k);
    rot_matrix[2][1] = 2 * (k * j - r * i);

    rot_matrix[0][2] = 2 * (i * k - r * j);
    rot_matrix[1][2] = 2 * (j * k + r * i);
    rot_matrix[2][2] = 1 - 2 * (i * i + j * j);

    return rot_matrix;
}

Rotation::Rotation(float _r, float _i, float _j, float _k) {
    r = _r;
    i = _i;
    j = _j;
    k = _k;

    x = 0.0f;
    y = 0.0f;
    z = 0.0f;

    quat_2_euler();
}

Rotation::Rotation() {
    i = 0.0f;
    j = 0.0f;
    k = 0.0f;
    r = 1.0f;

    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

Rotation::Rotation(float _x, float _y, float _z) {
    i = 0.0f;
    j = 0.0f;
    k = 0.0f;
    r = 1.0f;

    x = _x;
    y = _y;
    z = _z;
    euler_2_quat();
}

Rotation Rotation::operator* (const Rotation& rot) const {
    Rotation multiplied_rot{};
    multiplied_rot.r = r * rot.r - r * rot.i - j * rot.j - k * rot.k;
    multiplied_rot.i = r * rot.i + rot.r * i - j * rot.k - k * rot.j;
    multiplied_rot.j = r * rot.j + rot.r * j + k * rot.i - i * rot.k;
    multiplied_rot.k = r * rot.k + rot.r * k + i * rot.j - j * rot.i;

    return multiplied_rot;
}

void Rotation::euler_2_quat() {
    r = 1.0f;
    i = 0;
    j = 0;
    k = 0;

    *this *= Rotation(cos(x / 2), sin(x / 2), 0, 0);;
    *this *= Rotation(cos(y / 2), 0, sin(y / 2), 0);
    *this *= Rotation(cos(z / 2), 0, 0, sin(z / 2));
}

void Rotation::quat_2_euler() {
    x = atan2(2 * (r * i + j * k), 1 - 2 * (i * i + j * j));
    y = asin(2 * (r * j - i * k));
    z = atan2(2 * (r * k + i * j), 1 - 2 * (k * k + j * j));
}

Rotation& Rotation::operator*= (const Rotation& rot) {
    const float _w = r * rot.r - i * rot.i - j * rot.j - k * rot.k;
    const float _x = r * rot.i + rot.r * i + j * rot.k - k * rot.j;
    const float _y = r * rot.j - rot.k * i + j * rot.r + k * rot.i;
    const float _z = r * rot.k + rot.j * i - j * rot.i + k * rot.r;

    r = _w;
    i = _x;
    j = _y;
    k = _z;

    return *this;
}

Rotation Rotation::conjugate() const {
    return {r, -i, -j, -k};
}

void Rotation::rotate_point(float _x, float _y, float _z, glm::vec3& point) {
    Rotation rot{_x, _y, _z};
    const Rotation rot_c = rot.conjugate();

    rot *= Rotation{0, point.x, point.y, point.z};
    rot *= rot_c;

    point.x = rot.i;
    point.y = rot.j;
    point.z = rot.k;
}
