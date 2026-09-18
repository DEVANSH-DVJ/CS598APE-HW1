#include "plane.h"

Plane::Plane(const Vector &c, Texture* t, double ya, double pi, double ro, double tx, double ty) : Shape(c, t, ya, pi, ro), vect(c), right(c), up(c){
   textureX = tx; textureY = ty;
   setAngles(yaw, pitch, roll);
   normalMap = NULL;
   mapX = textureX; mapY = textureY;
}

void Plane::setAngles(double a, double b, double c){
   yaw =a; pitch = b; roll = c;
   xcos = cos(yaw);
   xsin = sin(yaw);
   ycos = cos(pitch);
   ysin = sin(pitch);
   zcos = cos(roll);
   zsin = sin(roll);
   vect.x = xsin*ycos*zcos+ysin*zsin;
   vect.y = ysin*zcos-xsin*ycos*zsin;
   vect.z = xcos*ycos;
   up.x = -xsin*ysin*zcos+ycos*zsin;
   up.y = ycos*zcos+xsin*ysin*zsin;
   up.z = -xcos*ysin;
   right.x = xcos*zcos;
   right.y = -xcos*zsin;
   right.z = -xsin;
   d = -vect.dot(center);
   updateInverse();
}

void Plane::setYaw(double a){
   yaw =a;
   xcos = cos(yaw);
   xsin = sin(yaw);
   
   vect.x = xsin*ycos*zcos+ysin*zsin;
   vect.y = ysin*zcos-xsin*ycos*zsin;
   vect.z = xcos*ycos;
   up.x = -xsin*ysin*zcos+ycos*zsin;
   up.y = ycos*zcos+xsin*ysin*zsin;
   up.z = -xcos*ysin;   
   right.x = xcos*zcos;
   right.y = -xcos*zsin;
   right.z = -xsin;
   d = -vect.dot(center);
   updateInverse();
}

void Plane::setPitch(double b){
   pitch = b;
   ycos = cos(pitch);
   ysin = sin(pitch);
   vect.x = xsin*ycos*zcos+ysin*zsin;
   vect.y = ysin*zcos-xsin*ycos*zsin;
   vect.z = xcos*ycos;
   up.x = -xsin*ysin*zcos+ycos*zsin;
   up.y = ycos*zcos+xsin*ysin*zsin;
   up.z = -xcos*ysin;
   d = -vect.dot(center);
   updateInverse();
}

void Plane::setRoll(double c){
   roll = c;
   zcos = cos(roll);
   zsin = sin(roll);
   vect.x = xsin*ycos*zcos+ysin*zsin;
   vect.y = ysin*zcos-xsin*ycos*zsin;
//   vect.z = xcos*ycos;
   up.x = -xsin*ysin*zcos+ycos*zsin;
   up.y = ycos*zcos+xsin*ysin*zsin;
  // up.z = -xcos*ysin;
   right.x = xcos*zcos;
   right.y = -xcos*zsin;
   //right.z = -xsin;
   d = -vect.dot(center);
   updateInverse();
}

// Cofactor rows of [right up vect], matching the term grouping in solveScalers.
void Plane::updateInverse(){
   const Vector &v1 = right, &v2 = up, &v3 = vect;
   const double denom = v1.z*v2.y*v3.x-v1.y*v2.z*v3.x-v1.z*v2.x*v3.y
                       +v1.x*v2.z*v3.y+v1.y*v2.x*v3.z-v1.x*v2.y*v3.z;
   invDenom = 1.0/denom;
   inv0 = Vector(v2.z*v3.y-v2.y*v3.z, v2.x*v3.z-v2.z*v3.x, v2.y*v3.x-v2.x*v3.y);
   inv1 = Vector(v1.y*v3.z-v1.z*v3.y, v1.z*v3.x-v1.x*v3.z, v1.x*v3.y-v1.y*v3.x);
   inv2 = Vector(v1.z*v2.y-v1.y*v2.z, v1.x*v2.z-v1.z*v2.x, v1.y*v2.x-v1.x*v2.y);
}

double Plane::getIntersection(Ray ray){
   const double t = ray.vector.dot(vect);
   const double norm = vect.dot(ray.point)+d;
   const double r = -norm/t;
   return (r>0)?r:inf;
}

bool Plane::getLightIntersection(Ray ray, double* fill){
   const double t = ray.vector.dot(vect);
   const double norm = vect.dot(ray.point)+d;
   const double r = -norm/t;
   if(r<=0. || r>=1.) return false;

   if(texture->opacity>1-1E-6) return true;   
   Vector dist = solveLocal(ray.point-center);
   unsigned char temp[4];
   double amb, op, ref;
   texture->getColor(temp, &amb, &op, &ref,fix(dist.x/textureX-.5), fix(dist.y/textureY-.5));
   if(op>1-1E-6) return true;
   fill[0]*=temp[0]/255.;
   fill[1]*=temp[1]/255.;
   fill[2]*=temp[2]/255.;
   return false;
}

void Plane::move(){
   d = -vect.dot(center);
}
void Plane::getColor(unsigned char* toFill,double* am, double* op, double* ref, Autonoma* r, Ray ray, unsigned int depth){
   Vector dist = solveLocal(ray.point-center);
   texture->getColor(toFill, am, op, ref, fix(dist.x/textureX-.5), fix(dist.y/textureY-.5));
}
unsigned char Plane::reversible(){ 
   return 1; }

Vector Plane::getNormal(Vector point){
   if(normalMap==NULL)
      return vect;
   else{
      Vector dist = solveLocal(point-center);
      double am, ref, op;
      unsigned char norm[3];
      normalMap->getColor(norm, &am, &op, &ref, fix(dist.x/mapX-.5+mapOffX), fix(dist.y/mapY-.5+mapOffY));
      Vector ret = ((norm[0]-128)*right+(norm[1]-128)*up+norm[2]*vect).normalize();
      return ret;
   }
}