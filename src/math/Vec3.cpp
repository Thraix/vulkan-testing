#include "Vec3.h"

#include <math/MathFunc.h>
#include <math/Maths.h>
#include <cassert>

namespace Greet{

  Vec3::Vec3(float x, float y, float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vec3::Vec3(const Vec4& vec4)
    : x(vec4.x), y(vec4.y), z(vec4.z)
  {

  }

  float Vec3::Length() const
  {
    return sqrt(x*x + y*y + z*z);
  }

  float Vec3::Dot(const Vec3& vec) const
  {
    return x * vec.x + y * vec.y + z * vec.z;
  }

  Vec3 Vec3::Cross(const Vec3& vec) const
  {
    float x_ = y * vec.z - z * vec.y;
    float y_ = z * vec.x - x * vec.z;
    float z_ = x * vec.y - y * vec.x;
    return Vec3(x_, y_, z_);
  }

  Vec3& Vec3::Normalize()
  {
    float len = Length();
    x /= len;
    y /= len;
    z /= len;
    return *this;
  }

  Vec3& Vec3::Rotate(const float& angle, const Vec3& axis)
  {
    float sh = (float)sin(Math::ToRadians(angle / 2.0));
    float ch = (float)cos(Math::ToRadians(angle / 2.0));

    float rX = axis.x * sh;
    float rY = axis.y * sh;
    float rZ = axis.z * sh;
    float rW = ch;

    Quaternion rotation(rX,rY,rZ,rW);
    Quaternion conjugate = rotation.Conjugate();

    Quaternion w = rotation * (*this) * conjugate;

    x = w.x;
    y = w.y;
    z = w.z;

    return *this;
  }

  Vec3& Vec3::Add(const Vec3& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vec3& Vec3::Subtract(const Vec3& other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  Vec3& Vec3::Multiply(const Vec3& other)
  {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
  }

  Vec3& Vec3::Divide(const Vec3& other)
  {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
  }

  Vec3& Vec3::Add(const float c)
  {
    x += c;
    y += c;
    z += c;
    return *this;
  }

  Vec3& Vec3::Subtract(const float c)
  {
    x -= c;
    y -= c;
    z -= c;
    return *this;
  }

  Vec3& Vec3::Multiply(const float c)
  {
    x *= c;
    y *= c;
    z *= c;
    return *this;
  }

  Vec3& Vec3::Divide(const float c)
  {
    x /= c;
    y /= c;
    z /= c;
    return *this;
  }

  bool Vec3::Compare(const Vec3& other)
  {
    return x == other.x && y == other.y && z == other.z;
  }

  Vec3 operator+(const Vec3& first, const Vec3 &second)
  {
    return Vec3(first.x, first.y,first.z).Add(second);
  }

  Vec3 operator-(const Vec3& first, const Vec3 &second)
  {
    return Vec3(first.x, first.y, first.z).Subtract(second);
  }

  Vec3 operator*(const Vec3& first, const Vec3 &second)
  {
    return Vec3(first.x, first.y, first.z).Multiply(second);
  }

  Vec3 operator/(const Vec3& first, const Vec3 &second)
  {
    return Vec3(first.x, first.y, first.z).Divide(second);
  }
  Vec3 operator+(const Vec3& first, const float c)
  {
    return Vec3(first.x, first.y,first.z).Add(c);
  }

  Vec3 operator-(const Vec3& first, const float c)
  {
    return Vec3(first.x, first.y,first.z).Subtract(c);
  }

  Vec3 operator*(const Vec3&  first, const float c)
  {
    return Vec3(first.x, first.y,first.z).Multiply(c);
  }

  Vec3 operator/(const Vec3& first, const float c)
  {
    return Vec3(first.x, first.y, first.z).Divide(c);
  }

  float Vec3::operator[](uint i)
  {
    assert(i < 3);
    return *((&x)+i);
  }

  Vec3& Vec3::operator+=(const Vec3 &other)
  {
    return Add(other);
  }

  Vec3& Vec3::operator-=(const Vec3 &other)
  {
    return Subtract(other);
  }

  Vec3& Vec3::operator*=(const Vec3 &other)
  {
    return Multiply(other);
  }

  Vec3& Vec3::operator/=(const Vec3 &other)
  {
    return Divide(other);
  }

  Vec3& Vec3::operator+=(const float other)
  {
    x += other;
    y += other;
    z += other;
    return *this;
  }

  Vec3& Vec3::operator-=(const float other)
  {
    x -= other;
    y -= other;
    z -= other;
    return *this;
  }

  Vec3& Vec3::operator*=(const float other)
  {
    x *= other;
    y *= other;
    z *= other;
    return *this;
  }

  Vec3& Vec3::operator/=(const float other)
  {
    x /= other;
    y /= other;
    z /= other;
    return *this;
  }

  bool Vec3::operator==(const Vec3 &other)
  {
    return Compare(other);
  }

  bool Vec3::operator!=(const Vec3 &other)
  {
    return !Compare(other);
  }

  std::ostream& operator<<(std::ostream& stream, const Vec3& vec)
  {
    return stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
  }
}
