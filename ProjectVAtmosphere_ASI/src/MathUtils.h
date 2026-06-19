#pragma once

#include <cmath>

struct float3
{
    float x;
    float y;
    float z;
};

struct float2
{
    float x;
    float y;
};

struct float4
{
    float x;
    float y;
    float z;
    float w;
};

struct float4x4
{
    float m[4][4];
};

namespace PVA
{
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kDegToRad = kPi / 180.0f;

    inline float Clamp01(const float value)
    {
        if (value <= 0.0f)
        {
            return 0.0f;
        }

        if (value >= 1.0f)
        {
            return 1.0f;
        }

        return value;
    }

    inline float Lerp(const float a, const float b, const float factor)
    {
        return a + ((b - a) * factor);
    }

    inline float Min(const float a, const float b)
    {
        return a < b ? a : b;
    }

    inline float Max(const float a, const float b)
    {
        return a > b ? a : b;
    }

    inline float Saturate(const float value)
    {
        return Clamp01(value);
    }

    inline float3 Add(const float3& a, const float3& b)
    {
        return float3{ a.x + b.x, a.y + b.y, a.z + b.z };
    }

    inline float3 Subtract(const float3& a, const float3& b)
    {
        return float3{ a.x - b.x, a.y - b.y, a.z - b.z };
    }

    inline float3 Scale(const float3& value, const float scale)
    {
        return float3{ value.x * scale, value.y * scale, value.z * scale };
    }

    inline float Dot(const float3& a, const float3& b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    inline float3 Cross(const float3& a, const float3& b)
    {
        return float3
        {
            (a.y * b.z) - (a.z * b.y),
            (a.z * b.x) - (a.x * b.z),
            (a.x * b.y) - (a.y * b.x)
        };
    }

    inline float LengthSq(const float3& value)
    {
        return Dot(value, value);
    }

    inline float Length(const float3& value)
    {
        return std::sqrt(LengthSq(value));
    }

    inline float3 Normalize(const float3& value)
    {
        const float len = Length(value);
        if (len <= 0.00001f)
        {
            return float3{ 0.0f, 0.0f, 0.0f };
        }

        return Scale(value, 1.0f / len);
    }

    inline float3 Lerp(const float3& a, const float3& b, const float factor)
    {
        return float3
        {
            Lerp(a.x, b.x, factor),
            Lerp(a.y, b.y, factor),
            Lerp(a.z, b.z, factor)
        };
    }

    inline float SmoothStep(const float edge0, const float edge1, const float value)
    {
        const float range = edge1 - edge0;
        if (range == 0.0f)
        {
            return value < edge0 ? 0.0f : 1.0f;
        }

        const float t = Saturate((value - edge0) / range);
        return t * t * (3.0f - (2.0f * t));
    }

    inline float4x4 Identity4x4()
    {
        return float4x4
        {
            {
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f, 1.0f }
            }
        };
    }

    inline float4x4 Multiply(const float4x4& a, const float4x4& b)
    {
        float4x4 result = {};

        for (int row = 0; row < 4; ++row)
        {
            for (int column = 0; column < 4; ++column)
            {
                result.m[row][column] =
                    (a.m[row][0] * b.m[0][column]) +
                    (a.m[row][1] * b.m[1][column]) +
                    (a.m[row][2] * b.m[2][column]) +
                    (a.m[row][3] * b.m[3][column]);
            }
        }

        return result;
    }

    inline float4x4 PerspectiveFovLH(const float fovYRadians, const float aspectRatio, const float nearPlane, const float farPlane)
    {
        const float safeAspect = aspectRatio > 0.01f ? aspectRatio : 1.7777778f;
        const float yScale = 1.0f / std::tan(fovYRadians * 0.5f);
        const float xScale = yScale / safeAspect;
        const float zScale = farPlane / (farPlane - nearPlane);

        float4x4 result = {};
        result.m[0][0] = xScale;
        result.m[1][1] = yScale;
        result.m[2][2] = zScale;
        result.m[2][3] = 1.0f;
        result.m[3][2] = -nearPlane * zScale;
        return result;
    }

    inline float4x4 LookToLH(const float3& eye, const float3& forward, const float3& up)
    {
        const float3 zAxis = Normalize(forward);
        const float3 xAxis = Normalize(Cross(up, zAxis));
        const float3 yAxis = Cross(zAxis, xAxis);

        float4x4 result = Identity4x4();
        result.m[0][0] = xAxis.x;
        result.m[1][0] = xAxis.y;
        result.m[2][0] = xAxis.z;
        result.m[3][0] = -Dot(xAxis, eye);

        result.m[0][1] = yAxis.x;
        result.m[1][1] = yAxis.y;
        result.m[2][1] = yAxis.z;
        result.m[3][1] = -Dot(yAxis, eye);

        result.m[0][2] = zAxis.x;
        result.m[1][2] = zAxis.y;
        result.m[2][2] = zAxis.z;
        result.m[3][2] = -Dot(zAxis, eye);

        return result;
    }

    inline float3 ForwardFromGtaRotation(const float3& rotationDegrees)
    {
        const float pitch = rotationDegrees.x * kDegToRad;
        const float yaw = rotationDegrees.z * kDegToRad;
        const float cosPitch = std::cos(pitch);

        return Normalize(float3
        {
            -std::sin(yaw) * cosPitch,
            std::cos(yaw) * cosPitch,
            std::sin(pitch)
        });
    }
}
