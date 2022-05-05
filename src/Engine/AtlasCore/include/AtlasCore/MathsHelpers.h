#pragma once
#include <Eigen/Core>

namespace atlas::maths_helpers
{
    constexpr float c_pi = 3.14159265358979323846f;

    class Angle
    {
    public:
        [[nodiscard]] float AsRadians() const { return m_Value; }
        [[nodiscard]] float AsDegrees() const { return m_Value * (180.0f / c_pi); }

        Angle operator-() const
        {
            return FromRadians(-m_Value);
        }

        static Angle FromRadians(const float value)
        {
            Angle angle;
            angle.SetRadians(value);
            return angle;
        }

        static Angle FromDegrees(const float value)
        {
            Angle angle;
            angle.SetDegrees(value);
            return angle;
        }

        Angle& operator+=(const Angle& other)
        {
            m_Value += other.m_Value;
            return *this;
        }

        void SetRadians(const float value) { m_Value = value; }
        void SetDegrees(const float value) { m_Value = value  * (c_pi / 180.0f); }

    private:
        float m_Value{};
    };

    struct Rectangle
    {
        Rectangle(const float top,
            const float bottom,
            const float left,
            const float right)
            : m_Values{top, bottom, left, right }
        {
        }

        [[nodiscard]] float GetTop() const { return m_Values[0]; }
        [[nodiscard]] float GetBottom() const { return m_Values[1]; }
        [[nodiscard]] float GetLeft() const { return m_Values[2]; }
        [[nodiscard]] float GetRight() const { return m_Values[3]; }

        Eigen::Vector4f m_Values;
    };

    Eigen::Matrix4f createLookAtMatrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& target, const Eigen::Vector3f& inUp, bool leftHanded = true);
    Eigen::Matrix4f createProjectionMatrix(
        Angle fovY,
        float aspectRatio,
        float near,
        float far,
        bool homogenousDepth,
        bool leftHanded = true);

    Eigen::Matrix4f createOrthographicMatrix(
        const Rectangle& viewRectangle,
        float near,
        float far,
        float offset,
        bool homogenousDepth,
        bool leftHanded = true);

}

inline atlas::maths_helpers::Angle operator ""_radians(long double value)
{
    return atlas::maths_helpers::Angle::FromRadians(static_cast<float>(value));
}

inline atlas::maths_helpers::Angle operator ""_degrees(long double value)
{
    return atlas::maths_helpers::Angle::FromDegrees(static_cast<float>(value));
}
