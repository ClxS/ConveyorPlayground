#include "AtlasCorePCH.h"
#include "MathsHelpers.h"

Eigen::Matrix4f atlas::maths_helpers::createLookAtMatrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& target, const Eigen::Vector3f& inUp, const bool leftHanded)
{
    const Eigen::Vector3f view = (leftHanded ? (target - eye) : (eye - target)).normalized();
    const Eigen::Vector3f right = inUp.cross(view).normalized();
    const Eigen::Vector3f up = view.cross(right);

    Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
    mat.row(0) = Eigen::Vector4f{right.x(), right.y(), right.z(), -right.dot(eye)};
    mat.row(1) = Eigen::Vector4f{up.x(), up.y(), up.z(), -up.dot(eye)};
    mat.row(2) = Eigen::Vector4f{view.x(), view.y(), view.z(), -view.dot(eye)};
    mat(3, 3) = 1.0f;
    return mat;
}

Eigen::Matrix4f atlas::maths_helpers::createProjectionMatrix(
        const Angle fovY,
        const float aspectRatio,
        const float near,
        const float far,
        const bool homogenousDepth,
        const bool leftHanded)
{
    const float height = 1.0f/tan(fovY.AsRadians() * 0.5f);
    const float width  = height * 1.0f / aspectRatio;

    const float diff = far - near;
    const float aa = homogenousDepth ? (      far + near)/diff : far/diff;
    const float bb = homogenousDepth ? (2.0f* far * near)/diff : near*aa;

    Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
    mat(0, 0) = width;
    mat(1, 1) = height;
    mat(2, 2) = leftHanded ? aa : -aa;
    mat(3, 2) = leftHanded ? 1.0f: -1.0f;
    mat(2, 3) = -bb;
    return mat;
}

Eigen::Matrix4f atlas::maths_helpers::createOrthographicMatrix(
        const Rectangle& viewRectangle,
        const float near,
        const float far,
        const float offset,
        const bool homogenousDepth,
        const bool leftHanded)
{
    const float right = viewRectangle.GetRight();
    const float left = viewRectangle.GetLeft();
    const float top = viewRectangle.GetTop();
    const float bottom = viewRectangle.GetBottom();

    const float aa = 2.0f / (right - left);
    const float bb = 2.0f / (top - bottom);
    const float cc = (homogenousDepth ? 2.0f : 1.0f) / (far - near);
    const float dd = (left + right )/(left   - right);
    const float ee = (top  + bottom)/(bottom - top  );

    const float viewDepth = (near - far);
    const float ff = (homogenousDepth ? (near + far) : near) / viewDepth;

    Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
    mat(0, 0) = aa;
    mat(1, 1) = bb;
    mat(2, 2) = leftHanded ? cc : -cc;
    mat(0, 3) = dd + offset;
    mat(1, 3) = ee;
    mat(2, 3) = ff;
    mat(3, 3) = 1.0f;
    return mat;
}