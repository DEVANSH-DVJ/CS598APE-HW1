#ifndef __BVH_H__
#define __BVH_H__

#include <vector>
#include "vector.h"

class Shape;

// Axis-aligned bounding box.
struct AABB {
   Vector lo, hi;
   AABB() : lo(inf, inf, inf), hi(-inf, -inf, -inf) {}

   inline void expand(const Vector& p) {
      if (p.x < lo.x) lo.x = p.x;
      if (p.y < lo.y) lo.y = p.y;
      if (p.z < lo.z) lo.z = p.z;
      if (p.x > hi.x) hi.x = p.x;
      if (p.y > hi.y) hi.y = p.y;
      if (p.z > hi.z) hi.z = p.z;
   }
   // An empty box is lo=+inf, hi=-inf; merging one must be a no-op. Expanding
   // with its corners instead would drive this box to lo=-inf, hi=+inf and make
   // area() infinite, which silently defeats the SAH sweep over empty bins.
   inline bool isEmpty() const { return lo.x > hi.x; }
   inline void expand(const AABB& b) { if (b.isEmpty()) return; expand(b.lo); expand(b.hi); }

   inline Vector centroid() const {
      return Vector(0.5*(lo.x+hi.x), 0.5*(lo.y+hi.y), 0.5*(lo.z+hi.z));
   }
   // Surface area; 0 for an empty box. Used as the SAH cost metric.
   inline double area() const {
      const double dx = hi.x-lo.x, dy = hi.y-lo.y, dz = hi.z-lo.z;
      if (dx < 0 || dy < 0 || dz < 0) return 0.0;
      return 2.0*(dx*dy + dy*dz + dz*dx);
   }
};

// Bounding volume hierarchy over the bounded shapes of a scene.
//
// Shapes that have no finite bounding box (planes are infinite) cannot be put
// in the tree; build() reports them separately and the callers scan those
// linearly, which is fine because every scene here has a handful of them at
// most while meshes contribute O(10^5) triangles.
class BVH {
public:
   BVH() : root(-1) {}

   // (Re)build over `all`. Shapes whose getBounds() returns false are appended
   // to `unboundedOut` instead of being inserted into the tree.
   void build(const std::vector<Shape*>& all, std::vector<Shape*>& unboundedOut);

   // Nearest intersection with t in (0, tBest). Returns the hit shape or NULL,
   // and narrows tBest to the hit time.
   Shape* closestHit(const Ray& ray, double& tBest) const;

   // Walks every shape whose box the ray segment [0, tMax] crosses, in tree
   // order, invoking shape->getLightIntersection(ray, fill) until one returns
   // true. Returns whether that happened.
   bool anyLightHit(const Ray& ray, double* fill, double tMax) const;

   bool empty() const { return root < 0; }

private:
   struct Node {
      AABB box;
      int left;        // index of first child, or -1 for a leaf
      int start, count; // leaf range into `order`
   };

   std::vector<Node> nodes;
   std::vector<Shape*> order;   // shapes permuted into leaf order
   int root;

   int buildRange(std::vector<AABB>& boxes, std::vector<Vector>& cents,
                  std::vector<int>& idx, int start, int end,
                  const std::vector<Shape*>& all);
};

#endif
