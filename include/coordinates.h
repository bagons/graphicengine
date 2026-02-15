#ifndef COORDINATES_H
#define COORDINATES_H
#include "glm/glm.hpp"

/// An in Engine 3D vector data type
/// @ingroup Coordinates
class Vector3 {
public:
    float x;
    float y;
    float z;

    /// Constructs a vector from raw coordinates
    Vector3(float _x, float _y, float _z);
    /// Constructs a vector from a GLM vector type
    explicit Vector3(glm::vec3 position);

    Vector3& operator= (const glm::vec3& position);
    Vector3& operator= (const Vector3& position);
    Vector3& operator+ (const Vector3& position);
    Vector3& operator+ (const glm::vec3& position);
    Vector3& operator- (const Vector3& position);
    Vector3& operator- (const glm::vec3& position);

    /// Calculates the Euclidean distance to another Vector3
    /// @param other The target vector
    /// @return The distance as a float
    [[nodiscard]] float distance_to(Vector3& other) const;

    /// Converts the internal data to a GLM compatible vec3
    [[nodiscard]] glm::vec3 glm_vector() const;
};

/// Position data type, which calculates it's transformation matrix form a 3D vector
/// @ingroup Coordinates
class Position : public Vector3 {
    /// Transformation matrix used for matrix multiplication
    glm::mat4 pos_matrix = glm::mat4(1.0f);
public:
    /// Returns the translation matrix based on current x, y, z values
    glm::mat4& get_transformation_matrix();

    using Vector3::Vector3;
    using Vector3::operator=;
};

/// Scale data type, which calculates it's transformation matrix from a 3D vector
/// @ingroup Coordinates
class Scale : public Vector3 {
    /// Transformation matrix used for matrix multiplication
    glm::mat4 scale_matrix = glm::mat4(1.0f);
public:
    using Vector3::Vector3;
    using Vector3::operator=;

    /// Returns the scaling matrix based on current x, y, z values
    glm::mat4& get_transformation_matrix();
};

/// Handles 3D rotation using both Quaternions and Euler angles (Euler in radians)
/// @ingroup Coordinates
class Rotation {
    /// Transformation matrix used for matrix multiplication
    glm::mat4 rot_matrix = glm::mat4(1.0f);
public:
    /// Updates Quaternion components (r, i, j, k) based on current Euler angles (x, y, z)
    void euler_2_quat();
    /// Updates Euler angles (x, y, z) based on current Quaternion components
    void quat_2_euler();

    // Quaternion components
    float i;
    float j;
    float k;
    float r;

    // Euler angle components in radians
    float x;
    float y;
    float z;

    /// Returns the rotation matrix derived from the quaternion
    glm::mat4& get_transformation_matrix();

    /// Constructs rotation from Quaternion components
    Rotation(float _r, float _i, float _j, float _k);
    /// Constructs rotation from Euler angles
    Rotation(float _x, float _y, float _z);
    /// Constructs an identity rotation
    Rotation();

    /// Multiplies two rotations (Quaternion multiplication)
    Rotation operator* (const Rotation& rot) const;
    Rotation& operator= (const glm::vec3& rotation);
    Rotation& operator*= (const Rotation& rot);

    /// Returns the inverse rotation
    [[nodiscard]] Rotation conjugate() const;

    /// Rotates a specific point in 3D space by given angles
    /// @param _x Euler X rotation
    /// @param _y Euler Y rotation
    /// @param _z Euler Z rotation
    /// @param point The point to be modified
    static void rotate_point(float _x, float _y, float _z, glm::vec3& point);
};

/// Container class combining Position, Rotation, and Scale, used by all SpatialThing(s)
/// @ingroup Coordinates
class Transform {
public:
    /// Global translation
    Position position{0, 0, 0};
    /// Global orientation
    Rotation rotation{1, 0, 0, 0};
    /// Global scale
    Scale scale{1, 1, 1};
};

#endif //COORDINATES_H