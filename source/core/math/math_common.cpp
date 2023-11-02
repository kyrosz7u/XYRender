#include "core/math/math.h"

#include <iostream>

namespace Math
{
    // 静态成员变量初始化
    const Matrix4x4 Matrix4x4::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    const Matrix4x4 Matrix4x4::ZEROAFFINE(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    const Matrix4x4 Matrix4x4::IDENTITY(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

    const Matrix3x3 Matrix3x3::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);
    const Matrix3x3 Matrix3x3::IDENTITY(1, 0, 0, 0, 1, 0, 0, 0, 1);

    const Vector4 Vector4::ZERO(0, 0, 0, 0);
    const Vector4 Vector4::UNIT_SCALE(1.0f, 1.0f, 1.0f, 1.0f);

    const Vector3 Vector3::ZERO(0, 0, 0);
    const Vector3 Vector3::UNIT_X(1, 0, 0);
    const Vector3 Vector3::UNIT_Y(0, 1, 0);
    const Vector3 Vector3::UNIT_Z(0, 0, 1);
    const Vector3 Vector3::NEGATIVE_UNIT_X(-1, 0, 0);
    const Vector3 Vector3::NEGATIVE_UNIT_Y(0, -1, 0);
    const Vector3 Vector3::NEGATIVE_UNIT_Z(0, 0, -1);
    const Vector3 Vector3::UNIT_SCALE(1, 1, 1);

    const Vector2 Vector2::ZERO(0, 0);
    const Vector2 Vector2::UNIT_X(1, 0);
    const Vector2 Vector2::UNIT_Y(0, 1);
    const Vector2 Vector2::NEGATIVE_UNIT_X(-1, 0);
    const Vector2 Vector2::NEGATIVE_UNIT_Y(0, -1);
    const Vector2 Vector2::UNIT_SCALE(1, 1);

    // 一些公用方法
    bool realEqual(float a, float b, float tolerance /* = std::numeric_limits<float>::epsilon() */)
    {
        return std::fabs(b - a) <= tolerance;
    }

    // 旋转顺序：ZXY  坐标系：左手坐标系  与unity相同
    Matrix4x4 getRotationMatrix(const EulerAngle &eulerAngle)
    {
        float cx = cos(eulerAngle.x * Math_PI / 180.0f);
        float sx = sin(eulerAngle.x * Math_PI / 180.0f);
        float cy = cos(eulerAngle.y * Math_PI / 180.0f);
        float sy = sin(eulerAngle.y * Math_PI / 180.0f);
        float cz = cos(eulerAngle.z * Math_PI / 180.0f);
        float sz = sin(eulerAngle.z * Math_PI / 180.0f);

        Matrix3x3 rx(1, 0, 0,
                     0, cx, -sx,
                     0, sx, cx);

        Matrix3x3 ry(cy, 0, sy,
                     0, 1, 0,
                     -sy, 0, cy);

        Matrix3x3 rz(cz, -sz, 0,
                     sz, cz, 0,
                     0, 0, 1);

        Matrix3x3 r = rz * rx * ry;

        Matrix4x4 result;
        result.setMatrix3x3(r);

        return result;
    }

    /** Clamps X to be between Min and Max, inclusive */
    template< class T >
    static inline T Clamp( const T X, const T Min, const T Max )
    {
        return X<Min ? Min : X<Max ? X : Max;
    }

    // from ue4 source UnrealMathUtility.h
    float FMod(float X, float Y)
    {
        const float AbsY = fabsf(Y);
        if (AbsY <= 1.e-8f)
        {
            return 0.0f;
        }
        const float Div = (X / Y);
        // All floats where abs(f) >= 2^23 (8388608) are whole numbers so do not need truncation, and avoid overflow in TruncToFloat as they get even larger.
        const float Quotient = fabsf(Div) < FLOAT_NON_FRACTIONAL ? truncf(Div) : Div;
        float IntPortion = Y * Quotient;

        // Rounding and imprecision could cause IntPortion to exceed X and cause the result to be outside the expected range.
        // For example Fmod(55.8, 9.3) would result in a very small negative value!
        if (fabsf(IntPortion) > fabsf(X))
        {
            IntPortion = X;
        }

        const float Result = X - IntPortion;
        // Clamp to [-AbsY, AbsY] because of possible failures for very large numbers (>1e10) due to precision loss.
        // We could instead fall back to stock fmodf() for large values, however this would diverge from the SIMD VectorMod() which has no similar fallback with reasonable performance.
        return Clamp(Result, -AbsY, AbsY);
    }

    inline std::ostream &operator<<(std::ostream &os, const Vector3 &v)
    {
        char buf[128];
        sprintf(buf, "[ %f, %f, %f ]", v.x, v.y, v.z);
        os << buf;
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const Vector4 &v)
    {
        char buf[128];
        sprintf(buf, "[ %f, %f, %f %f]", v.x, v.y, v.z, v.w);
        os << buf;
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m)
    {
        os << "[";
        for (int i = 0; i < 3; ++i)
        {
            os << m[i] << std::endl;
        }
        os << m[3] << "]";

        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const Matrix3x3 &m)
    {
        os << "[";
        for (int i = 0; i < 2; ++i)
        {
            os << m[i] << std::endl;
        }
        os << m[2] << "]";

        return os;
    }
}
