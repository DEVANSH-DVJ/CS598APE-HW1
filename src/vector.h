#ifndef __VECTOR_H_INCLUDED__
#define __VECTOR_H_INCLUDED__
#define _USE_MATH_DEFINES
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<cmath>
#include<limits>
#define inf std::numeric_limits<double>::infinity()

class Vector{
public:
  double x, y, z;
  Vector() {}
  Vector(double a, double b, double c) : x(a), y(b), z(c) {}

  // All of these are defined inline: they are the innermost operations of the
  // raytracer and were previously out-of-line in vector.cpp, so no call in any
  // other translation unit could be inlined or vectorised.
  inline void operator += (const Vector& r) { x+=r.x; y+=r.y; z+=r.z; }
  inline void operator -= (const Vector& r) { x-=r.x; y-=r.y; z-=r.z; }
  inline void operator *= (const double r)  { x*=r; y*=r; z*=r; }
  inline void operator *= (const float r)   { x*=r; y*=r; z*=r; }
  inline void operator *= (const int r)     { x*=r; y*=r; z*=r; }
  inline void operator /= (const double r)  { x/=r; y/=r; z/=r; }
  inline void operator /= (const float r)   { x/=r; y/=r; z/=r; }
  inline void operator /= (const int r)     { x/=r; y/=r; z/=r; }

  inline Vector operator + (const Vector& r) const { return Vector(x+r.x, y+r.y, z+r.z); }
  inline Vector operator - (const Vector& r) const { return Vector(x-r.x, y-r.y, z-r.z); }
  inline Vector operator * (const double r) const { return Vector(x*r, y*r, z*r); }
  inline Vector operator * (const float r)  const { return Vector(x*r, y*r, z*r); }
  inline Vector operator * (const int r)    const { return Vector(x*r, y*r, z*r); }
  inline Vector operator / (const double r) const { return Vector(x/r, y/r, z/r); }
  inline Vector operator / (const float r)  const { return Vector(x/r, y/r, z/r); }
  inline Vector operator / (const int r)    const { return Vector(x/r, y/r, z/r); }

  inline Vector cross(const Vector& a) const {
     return Vector(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
  }
  inline double mag2() const { return x*x+y*y+z*z; }
  inline double mag()  const { return sqrt(x*x+y*y+z*z); }
  inline double dot(const Vector& a) const { return x*a.x+y*a.y+z*a.z; }
  inline Vector normalize() const {
     const double m = sqrt(x*x+y*y+z*z);
     return Vector(x/m, y/m, z/m);
  }
} ;

class Ray{
public:
  Vector point, vector;
  Ray() {}
  Ray(const Vector& po, const Vector& ve) : point(po), vector(ve) {}
};

  inline Vector operator-(const Vector& b){
   return Vector(-b.x,-b.y,-b.z);
  }
  
  inline Vector operator+(const Vector& b){
   return b;
  }
  
  inline Vector operator*(const int a, const Vector& b){
   return Vector(a*b.x,a*b.y,a*b.z);
  }

  inline Vector operator*(const double a, const Vector& b){
   return Vector(a*b.x,a*b.y,a*b.z);
  }

  inline Vector operator*(const float a, const Vector& b){
   return Vector(a*b.x,a*b.y,a*b.z);
  }

  inline Vector operator/(const int a, const Vector& b){
   return Vector(a/b.x,a/b.y,a/b.z);
  }

  inline Vector operator/(const double a, const Vector& b){
   return Vector(a/b.x,a/b.y,a/b.z);
  }

  inline Vector operator/(const float a, const Vector& b){
   return Vector(a/b.x,a/b.y,a/b.z);
  }
  
  Vector solveScalers(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& solve);

int print_vector(FILE *stream, const struct printf_info *info, const void 
*const *args);

int print_vector_arginfo (const struct printf_info *info, size_t n, int 
*argtypes);

#endif
