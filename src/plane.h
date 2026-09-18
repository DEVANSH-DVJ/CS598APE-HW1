#ifndef __PLANE_H__
#define __PLANE_H__

#include "shape.h"

class Plane : public Shape{
public:
  Vector vect, right, up;
  double d;
  // Cached inverse of the basis [right up vect]. solveScalers() re-derives this
  // 3x3 inverse on every call, but the basis only changes when the shape's
  // angles do -- which is at most once per frame, not once per ray.
  Vector inv0, inv1, inv2;
  double invDenom;
  void updateInverse();
  inline Vector solveLocal(const Vector& c) const {
     return Vector((inv0.x*c.x + inv0.y*c.y + inv0.z*c.z) * invDenom,
                   (inv1.x*c.x + inv1.y*c.y + inv1.z*c.z) * invDenom,
                   (inv2.x*c.x + inv2.y*c.y + inv2.z*c.z) * invDenom);
  }
  Plane(const Vector &c, Texture* t, double ya, double pi, double ro, double tx, double ty);
  double getIntersection(Ray ray);
  bool getLightIntersection(Ray ray, double* toFill);
  void move();
  void getColor(unsigned char* toFill, double* am, double* op, double* ref, Autonoma* r, Ray ray, unsigned int depth);
  Vector getNormal(Vector point);
  unsigned char reversible();
  void setAngles(double yaw, double pitch, double roll);
  void setYaw(double d);
  void setPitch(double d);
  void setRoll(double d);
};

#endif
