#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <iostream>
#include <math/Vec4.h>

namespace Greet{

  struct Vec3
  {
    union
    {
      float vals[3];
      struct {
        float x, y, z;
      };
      struct {
        float r, g, b;
      };
      struct {
        float h, s, v;
      };
    };
    Vec3() = default;
    Vec3(float x, float y, float z);
    Vec3(const Vec4& vec4);

    float Length() const;
    float Dot(const Vec3& vec) const;
    Vec3 Cross(const Vec3& vec) const;

    // All These modifies the current vec3
    Vec3& Normalize();
    Vec3& Rotate(const float& angle, const Vec3& axis);
    Vec3& Add(const Vec3& other);
    Vec3& Subtract(const Vec3& other);
    Vec3& Multiply(const Vec3& other);
    Vec3& Divide(const Vec3& other);

    Vec3& Add(const float other);
    Vec3& Subtract(const float other);
    Vec3& Multiply(const float other);
    Vec3& Divide(const float other);

    bool Compare(const Vec3& other);

    friend Vec3 operator+(const Vec3& first, const Vec3 &second);
    friend Vec3 operator-(const Vec3& first, const Vec3 &second);
    friend Vec3 operator*(const Vec3& first, const Vec3 &second);
    friend Vec3 operator/(const Vec3& first, const Vec3 &second);

    friend Vec3 operator+(const Vec3& first, const float c);
    friend Vec3 operator-(const Vec3& first, const float c);
    friend Vec3 operator*(const Vec3& first, const float c);
    friend Vec3 operator/(const Vec3& first, const float c);

    float operator[](uint i);

    Vec3& operator+=(const Vec3 &other);
    Vec3& operator-=(const Vec3 &other);
    Vec3& operator*=(const Vec3 &other);
    Vec3& operator/=(const Vec3 &other);


    Vec3& operator+=(const float c);
    Vec3& operator-=(const float c);
    Vec3& operator*=(const float c);
    Vec3& operator/=(const float c);

    bool operator!=(const Vec3 &second);
    bool operator==(const Vec3 &second);

    friend std::ostream& operator<<(std::ostream& stream, const Vec3& vec);
  };
}
