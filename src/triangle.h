#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "plane.h"

class Triangle : public Plane{
public:
   double thirdX;
   // Exact bounds captured from the three input vertices at construction.
   // They cannot be recovered from the stored (center, right, up, vect) frame:
   // the constructor keeps only the in-plane coordinates of the third vertex,
   // so any out-of-plane residual would be lost and the box could clip the
   // triangle, making the BVH miss real intersections.
   Vector bmin, bmax;
   Triangle(Vector c, Vector b, Vector a, Texture* t);
   double getIntersection(Ray ray);
   bool getLightIntersection(Ray ray, double* fill);
   bool getBounds(Vector& lo, Vector& hi);
};

#endif
