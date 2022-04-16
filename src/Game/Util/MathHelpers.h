#pragma once

namespace cpp_conv::math_helpers
{
    constexpr float c_pi = 3.14159265358979323846f;

    struct Angle
    {
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
}

inline cpp_conv::math_helpers::Angle operator ""_radians(long double value)
{
    return cpp_conv::math_helpers::Angle::FromRadians(static_cast<float>(value));
}

inline cpp_conv::math_helpers::Angle operator ""_degrees(long double value)
{
    return cpp_conv::math_helpers::Angle::FromDegrees(static_cast<float>(value));
}
