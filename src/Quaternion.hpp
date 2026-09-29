
#ifndef _QUATERION_HPP_
#define _QUATERNION_HPP_

#include <cassert>

#include "typedefs.hpp"
#include "Matrix3x3.hpp"
#include "Vector3.hpp"

template<typename T> class Matrix3x3;
template<typename T> class Vector3;

template<typename T>
class Quaternion {
public:
  enum { D = 4 };

  typedef T ValueT;

  union {
    struct { T s; T x; T y; T z; };
    struct { T a; T b; T c; T d; };
    T v[D];
  };

  explicit Quaternion(const T &value=0) : s(value), x(value), y(value), z(value) {}
  Quaternion(const T &a, const T &b, const T &c=0, const T &d=0): s(a), x(b), y(c), z(d) {}

  Quaternion(const Matrix3x3<T> &m){
    // Old way to convert rotation matrix to quaternion:

    // s = 0.5*sqrt(1 + m(0,0) + m(1,1) + m(2,2));
    // x = (m(2,1) - m(1,2)) / (4*s);
    // y = (m(0,2) - m(2,0)) / (4*s);
    // z = (m(1,0) - m(0,1)) / (4*s);

    // New way to convert rotation matrix to quaternion:

    T trace = m(0,0) + m(1,1) + m(2,2);
    if(trace > 0) {
      T s4 = std::sqrt(trace + 1.0f) * 2; // S=4*s
      s = 0.25f * s4;
      x = (m(2,1) - m(1,2)) / s4;
      y = (m(0,2) - m(2,0)) / s4;
      z = (m(1,0) - m(0,1)) / s4;
    } else if((m(0,0) > m(1,1)) && (m(0,0) > m(2,2))) {
      T s4 = std::sqrt(1.0f + m(0,0) - m(1,1) - m(2,2)) * 2; // S=4*x
      s = (m(2,1) - m(1,2)) / s4;
      x = 0.25f * s4;
      y = (m(0,1) + m(1,0)) / s4;
      z = (m(0,2) + m(2,0)) / s4;
    } else if(m(1,1) > m(2,2)) {
      T s4 = std::sqrt(1.0f + m(1,1) - m(0,0) - m(2,2)) * 2; // S=4*y
      s = (m(0,2) - m(2,0)) / s4;
      x = (m(0,1) + m(1,0)) / s4;
      y = 0.25f * s4;
      z = (m(1,2) + m(2,1)) / s4;
    } else {
      T s4 = std::sqrt(1.0f + m(2,2) - m(0,0) - m(1,1)) * 2; // S=4*z
      s = (m(1,0) - m(0,1)) / s4;
      x = (m(0,2) + m(2,0)) / s4;
      y = (m(1,2) + m(2,1)) / s4;
      z = 0.25f * s4;
    }
  }

  // assignment operators
  Quaternion& operator+=(const Quaternion &r) { s+=r.s; x+=r.x; y+=r.y; z+=r.z; return *this; }
  Quaternion& operator-=(const Quaternion &r) { s-=r.s; x-=r.x; y-=r.y; z-=r.z; return *this; }
  Quaternion& operator*=(const Quaternion &r) { s*=r.s; x*=r.x; y*=r.y; z*=r.z; return *this; }
  Quaternion& operator/=(const Quaternion &r) { s/=r.s; x/=r.x; y/=r.y; z/=r.z; return *this; }

  Quaternion& operator+=(const T *r) { s+=r[0]; x+=r[1]; y+=r[2]; z+=r[3]; return *this; }
  Quaternion& operator-=(const T *r) { s-=r[0]; x-=r[1]; y-=r[2]; z-=r[3]; return *this; }
  Quaternion& operator*=(const T *r) { s*=r[0]; x*=r[1]; y*=r[2]; z*=r[3]; return *this; }
  Quaternion& operator/=(const T *r) { s/=r[0]; x/=r[1]; y/=r[2]; z/=r[3]; return *this; }

  Quaternion& operator+=(const T s) { this->s += s; x+=s; y+=s; z+=s; return *this; }
  Quaternion& operator-=(const T s) { this->s -= s;x-=s; y-=s; z-=s; return *this; }
  Quaternion& operator*=(const T s) { this->s *= s; x*=s; y*=s; z*=s; return *this; }
  Quaternion& operator/=(const T s) {
    const T d=static_cast<T>(1)/s; return operator*=(d);
  }

  // unary operators
  Quaternion operator+() const { return *this; }
  Quaternion operator-() const { return Quaternion(-s, -x, -y, -z); }

  // binary operators
  Quaternion operator+(const Quaternion &r) const { return Quaternion(*this)+=r; }
  Quaternion operator-(const Quaternion &r) const { return Quaternion(*this)-=r; }
  Quaternion operator*(const Quaternion &r) const { return Quaternion(*this)*=r; }
  Quaternion operator/(const Quaternion &r) const { return Quaternion(*this)/=r; }

  Quaternion operator+(const T *r) const { return Quaternion(*this)+=r; }
  Quaternion operator-(const T *r) const { return Quaternion(*this)-=r; }
  Quaternion operator*(const T *r) const { return Quaternion(*this)*=r; }
  Quaternion operator/(const T *r) const { return Quaternion(*this)/=r; }

  Quaternion operator+(const T s) const { return Quaternion(*this)+=s; }
  Quaternion operator-(const T s) const { return Quaternion(*this)-=s; }
  Quaternion operator*(const T s) const { return Quaternion(*this)*=s; }
  Quaternion operator/(const T s) const { return Quaternion(*this)/=s; }

  // comparison operators
  bool operator==(const Quaternion &r) const {
    return ((s==r.s) && (x==r.x) && (y==r.y) && (z==r.z));
  }
  bool operator!=(const Quaternion &r) const { return !(*this==r); }

  // cast operator
  template<typename T2>
  operator Quaternion<T2>() const {
    return
      Quaternion<T2>(static_cast<T2>(s), static_cast<T2>(x),static_cast<T2>(y),static_cast<T2>(z));
  }

  const T& operator[](const tIndex i) const { assert(i<D); return v[i]; }
  T& operator[](const tIndex i) {
    return const_cast<T &>(static_cast<const Quaternion &>(*this)[i]);
  }

  // special calculative functions

  Quaternion& normalize() { return (s==0&&x==0&&y==0&&z==0) ? (*this):(*this)/=length(); }
  Quaternion normalized() const { return Quaternion(*this).normalize(); }

  Quaternion quatProduct(const Quaternion &r) const {
    Vector3<T> v1(x, y, z);
    Vector3<T> v2(r.x, r.y, r.z);
    Vector3<T> vf = v2*s + v1*r.s + v1.crossProduct(v2);
    return Quaternion(s*r.s - v1.dotProduct(v2), vf.x, vf.y, vf.z);
  }

  T length() const { return std::sqrt(lengthSquare()); }
  T lengthSquare() const { return (s*s + x*x + y*y + z*z); }
  T distanceTo(const Quaternion &t) const { return (*this-t).length(); }
  T distanceSquareTo(const Quaternion &t) const { return (*this-t).lengthSquare(); }

  Matrix3x3<T> toRotationMatrix() const {
    return Matrix3x3<T>(
      1 - 2*(y*y + z*z), 2*(x*y - z*s),     2*(x*z + y*s),
      2*(y*x + z*s),     1 - 2*(x*x + z*z), 2*(y*z - x*s),
      2*(z*x - y*s),     2*(z*y + x*s),     1 - 2*(x*x + y*y));
  }


  friend std::istream& operator>>(std::istream &in, Quaternion &vec) {
    return (in >> vec.s >> vec.x >> vec.y >> vec.z);
  }
  friend std::ostream& operator<<(std::ostream &out, const Quaternion &vec) {
    return (out << vec.s << " " << vec.x << " " << vec.y << " " << vec.z);
  }
};

typedef Quaternion<tReal> Quatf;

inline const Quatf operator*(const tReal s, const Quatf &r) { return r*s; }

#endif  /* _QUATERNION_HPP_ */
