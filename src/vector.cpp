#include<string.h>
#include<stdio.h>
#include<limits>
#include<math.h>
#include<stdlib.h>
//#include <printf.h>
#include <stddef.h>
#include "vector.h"

Vector solveScalers(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& C){
   double denom = v1.z*v2.y*v3.x-v1.y*v2.z*v3.x-v1.z*v2.x*v3.y+v1.x*v2.z*v3.y+v1.y*v2.x*v3.z-v1.x*v2.y*v3.z;
   double a = C.z*v2.y*v3.x-C.y*v2.z*v3.x-C.z*v2.x*v3.y+C.x*v2.z*v3.y+C.y*v2.x*v3.z-C.x*v2.y*v3.z;
   double b = -C.z*v1.y*v3.x+C.y*v1.z*v3.x+C.z*v1.x*v3.y-C.x*v1.z*v3.y-C.y*v1.x*v3.z+C.x*v1.y*v3.z;
   double c = C.z*v1.y*v2.x-C.y*v1.z*v2.x-C.z*v1.x*v2.y+C.x*v1.z*v2.y+C.y*v1.x*v2.z-C.x*v1.y*v2.z;
   return Vector(a/denom, b/denom, c/denom);
}

