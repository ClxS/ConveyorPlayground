#pragma once
#include <Eigen/Dense>

namespace atlas::maths_helpers
{
    constexpr float c_pi = 3.14159265358979323846f;

    class Angle
    {
    public:
        enum class WrapMode
        {
            Clamp,
            Wrap,
            None
        };

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

        static Angle FromRadians(const float value, const WrapMode wrapMode)
        {
            Angle angle;
            angle.SetRadians(value, 0.0f, c_pi * 2.0f, wrapMode);
            return angle;
        }

        static Angle FromRadians(const float value, const float min, const float max, const WrapMode wrapMode)
        {
            Angle angle;
            angle.SetRadians(value, min, max, wrapMode);
            return angle;
        }

        static Angle FromDegrees(const float value)
        {
            Angle angle;
            angle.SetDegrees(value);
            return angle;
        }

        static Angle FromDegrees(const float value, const WrapMode wrapMode)
        {
            Angle angle;
            angle.SetDegrees(value, 0.0f, 360.0f, wrapMode);
            return angle;
        }

        static Angle FromDegrees(const float value, const float min, const float max, const WrapMode wrapMode)
        {
            Angle angle;
            angle.SetDegrees(value, min, max, wrapMode);
            return angle;
        }

        Angle& operator+=(const Angle& other)
        {
            SetRadians(m_Value + other.m_Value);
            return *this;
        }

        Angle& operator-=(const Angle& other)
        {
            SetRadians(m_Value - other.m_Value, m_ClampMin, m_ClampMax, m_WrapMode);
            return *this;
        }

        Angle operator-(const Angle& other) const
        {
            return FromRadians(m_Value - other.m_Value, m_ClampMin, m_ClampMax, m_WrapMode);
        }

        Angle& operator=(const Angle& other)
        {
            SetRadians(other.m_Value);
            return *this;
        }

        bool operator>(const Angle& other) const { return m_Value > other.m_Value; }
        bool operator>=(const Angle& other) const { return m_Value >= other.m_Value; }
        bool operator<=(const Angle& other) const { return m_Value < other.m_Value; }
        bool operator<(const Angle& other) const { return m_Value >= other.m_Value; }

        void SetRadians(const float value, const float min, const float max, const WrapMode wrapMode)
        {
            m_ClampMin = min;
            m_ClampMax = max;
            m_WrapMode = wrapMode;
            SetRadians(value);
        }

        void SetRadians(const float value, const WrapMode wrapMode)
        {
            m_WrapMode = wrapMode;
            SetRadians(value);
        }

        void SetRadians(const float value);

        void SetDegrees(const float value, const float min, const float max, const WrapMode wrapMode)
        {
            SetRadians(
                value  * (c_pi / 180.0f),
                min  * (c_pi / 180.0f),
                max  * (c_pi / 180.0f),
                wrapMode);
        }

        void SetDegrees(const float value, const WrapMode wrapMode)
        {
            SetRadians(
                value  * (c_pi / 180.0f),
                m_ClampMin,
                m_ClampMax,
                wrapMode);
        }

        void SetDegrees(const float value)
        {
            SetRadians(value  * (c_pi / 180.0f));
        }

    private:
        float m_Value{};

        float m_ClampMin{0.0f};
        float m_ClampMax{c_pi * 2.0f};
        WrapMode m_WrapMode{WrapMode::Wrap};
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

    Eigen::Matrix4f getMatrixForSphericalCoordinate(Angle pitch, Angle yaw, float distance);

    Eigen::Vector3f sphericalCoordinateToCartesian(Angle pitch, Angle yaw, float distance);
}

inline atlas::maths_helpers::Angle operator ""_radians(long double value)
{
    return atlas::maths_helpers::Angle::FromRadians(static_cast<float>(value));
}

inline atlas::maths_helpers::Angle operator ""_degrees(long double value)
{
    return atlas::maths_helpers::Angle::FromDegrees(static_cast<float>(value));
}
