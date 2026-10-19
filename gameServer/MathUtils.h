#pragma once

#include <cmath>
#include <cstdlib>

struct Vector3
{
        float x = 0, y = 0, z = 0;

        Vector3 operator+(const Vector3 &other) const
        {
            return {x + other.x, y + other.y, z + other.z};
        }

        Vector3 operator-(const Vector3 &other) const
        {
            return {x - other.x, y - other.y, z - other.z};
        }

        Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }

        bool operator==(const Vector3 &other) const
        {
            const float epsilon = 0.0001f;
            return std::abs(x - other.x) < epsilon &&
                   std::abs(y - other.y) < epsilon &&
                   std::abs(z - other.z) < epsilon;
        }

        bool operator!=(const Vector3 &other) const
        {
            return !(*this == other);
        }

        float SqrMagnitude() const { return x * x + y * y + z * z; }
        float Magnitude() const { return std::sqrt(SqrMagnitude()); }
        Vector3 Normalized() const
        {
            float magnitude = Magnitude();
            if (magnitude < 0.0001f)
                return {0, 0, 0};

            return {x / magnitude, y / magnitude, z / magnitude};
        }
};
