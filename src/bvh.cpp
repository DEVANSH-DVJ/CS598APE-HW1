#include "bvh.h"
#include "shape.h"

#include <algorithm>

namespace {

const int   NUM_BINS      = 12;
const int   MAX_LEAF_SIZE = 4;
const double TRAVERSAL_COST = 1.0;   // relative to one shape intersection

// Slab test. Returns whether the ray's [0, tMax] segment meets the box, and if
// so sets tEnter to the entry distance (clamped at 0 for an origin inside).
inline bool slabHit(const AABB& b, const Vector& o, const Vector& invD,
                    double tMax, double& tEnter) {
   double t0 = (b.lo.x - o.x) * invD.x;
   double t1 = (b.hi.x - o.x) * invD.x;
   if (t0 > t1) { const double s = t0; t0 = t1; t1 = s; }
   double tmin = t0, tmax = t1;

   t0 = (b.lo.y - o.y) * invD.y;
   t1 = (b.hi.y - o.y) * invD.y;
   if (t0 > t1) { const double s = t0; t0 = t1; t1 = s; }
   if (t0 > tmin) tmin = t0;
   if (t1 < tmax) tmax = t1;

   t0 = (b.lo.z - o.z) * invD.z;
   t1 = (b.hi.z - o.z) * invD.z;
   if (t0 > t1) { const double s = t0; t0 = t1; t1 = s; }
   if (t0 > tmin) tmin = t0;
   if (t1 < tmax) tmax = t1;

   if (tmin > tmax || tmax < 0.0 || tmin > tMax) return false;
   tEnter = (tmin < 0.0) ? 0.0 : tmin;
   return true;
}

} // namespace

// Recursively partition idx[start,end) with a binned surface-area heuristic.
int BVH::buildRange(std::vector<AABB>& boxes, std::vector<Vector>& cents,
                    std::vector<int>& idx, int start, int end,
                    const std::vector<Shape*>& all) {
   const int nodeIdx = (int)nodes.size();
   nodes.push_back(Node());

   AABB bounds, centBounds;
   for (int i = start; i < end; i++) {
      bounds.expand(boxes[idx[i]]);
      centBounds.expand(cents[idx[i]]);
   }

   const int count = end - start;
   int bestAxis = -1, bestSplit = -1;
   double bestCost = (double)count;   // cost of making this a leaf

   if (count > MAX_LEAF_SIZE) {
      // Bin along each axis and pick the cheapest split plane.
      for (int axis = 0; axis < 3; axis++) {
         const double clo = (axis == 0) ? centBounds.lo.x : (axis == 1) ? centBounds.lo.y : centBounds.lo.z;
         const double chi = (axis == 0) ? centBounds.hi.x : (axis == 1) ? centBounds.hi.y : centBounds.hi.z;
         const double extent = chi - clo;
         if (extent <= 0.0) continue;
         const double scale = NUM_BINS / extent;

         AABB binBox[NUM_BINS];
         int   binCount[NUM_BINS] = {0};
         for (int i = start; i < end; i++) {
            const Vector& c = cents[idx[i]];
            const double cv = (axis == 0) ? c.x : (axis == 1) ? c.y : c.z;
            int b = (int)((cv - clo) * scale);
            if (b < 0) b = 0;
            if (b >= NUM_BINS) b = NUM_BINS - 1;
            binCount[b]++;
            binBox[b].expand(boxes[idx[i]]);
         }

         // Sweep to get left/right areas and counts for each of the 11 planes.
         AABB leftAcc;
         double leftArea[NUM_BINS];
         int    leftCount[NUM_BINS];
         int running = 0;
         for (int b = 0; b < NUM_BINS; b++) {
            leftAcc.expand(binBox[b]);
            running += binCount[b];
            leftArea[b]  = leftAcc.area();
            leftCount[b] = running;
         }
         AABB rightAcc;
         double rightArea[NUM_BINS];
         int    rightCount[NUM_BINS];
         running = 0;
         for (int b = NUM_BINS - 1; b >= 0; b--) {
            rightAcc.expand(binBox[b]);
            running += binCount[b];
            rightArea[b]  = rightAcc.area();
            rightCount[b] = running;
         }

         const double invArea = (bounds.area() > 0.0) ? 1.0 / bounds.area() : 0.0;
         for (int b = 0; b < NUM_BINS - 1; b++) {
            if (leftCount[b] == 0 || rightCount[b + 1] == 0) continue;
            const double cost = TRAVERSAL_COST +
               invArea * (leftArea[b] * leftCount[b] + rightArea[b + 1] * rightCount[b + 1]);
            if (cost < bestCost) {
               bestCost  = cost;
               bestAxis  = axis;
               bestSplit = b;
            }
         }
      }
   }

   if (bestAxis < 0) {
      nodes[nodeIdx].box   = bounds;
      nodes[nodeIdx].left  = -1;
      nodes[nodeIdx].start = (int)order.size();
      nodes[nodeIdx].count = count;
      for (int i = start; i < end; i++) order.push_back(all[idx[i]]);
      return nodeIdx;
   }

   const int axis = bestAxis;
   const double clo = (axis == 0) ? centBounds.lo.x : (axis == 1) ? centBounds.lo.y : centBounds.lo.z;
   const double chi = (axis == 0) ? centBounds.hi.x : (axis == 1) ? centBounds.hi.y : centBounds.hi.z;
   const double scale = NUM_BINS / (chi - clo);
   const int split = bestSplit;

   std::vector<int>::iterator midIt = std::partition(
      idx.begin() + start, idx.begin() + end,
      [&](int s) {
         const Vector& c = cents[s];
         const double cv = (axis == 0) ? c.x : (axis == 1) ? c.y : c.z;
         int b = (int)((cv - clo) * scale);
         if (b < 0) b = 0;
         if (b >= NUM_BINS) b = NUM_BINS - 1;
         return b <= split;
      });
   int mid = (int)(midIt - idx.begin());
   if (mid == start || mid == end) mid = start + count / 2;  // degenerate guard

   const int l = buildRange(boxes, cents, idx, start, mid, all);
   const int r = buildRange(boxes, cents, idx, mid, end, all);
   (void)r;   // right child is always l's subtree successor; recorded via left+1 chain
   nodes[nodeIdx].box   = bounds;
   nodes[nodeIdx].left  = l;
   nodes[nodeIdx].start = r;
   nodes[nodeIdx].count = 0;
   return nodeIdx;
}

void BVH::build(const std::vector<Shape*>& all, std::vector<Shape*>& unboundedOut) {
   nodes.clear();
   order.clear();
   unboundedOut.clear();
   root = -1;

   std::vector<Shape*> bounded;
   std::vector<AABB>   boxes;
   std::vector<Vector> cents;
   bounded.reserve(all.size());
   boxes.reserve(all.size());
   cents.reserve(all.size());

   for (size_t i = 0; i < all.size(); i++) {
      Vector lo(0,0,0), hi(0,0,0);
      if (all[i]->getBounds(lo, hi)) {
         AABB b;
         b.expand(lo);
         b.expand(hi);
         bounded.push_back(all[i]);
         boxes.push_back(b);
         cents.push_back(b.centroid());
      } else {
         unboundedOut.push_back(all[i]);
      }
   }
   if (bounded.empty()) return;

   std::vector<int> idx(bounded.size());
   for (size_t i = 0; i < idx.size(); i++) idx[i] = (int)i;

   nodes.reserve(2 * bounded.size());
   order.reserve(bounded.size());
   root = buildRange(boxes, cents, idx, 0, (int)bounded.size(), bounded);
}

Shape* BVH::closestHit(const Ray& ray, double& tBest) const {
   if (root < 0) return NULL;

   const Vector& o = ray.point;
   const Vector invD(1.0/ray.vector.x, 1.0/ray.vector.y, 1.0/ray.vector.z);

   Shape* best = NULL;
   int stack[64];
   double stackT[64];
   int sp = 0;
   double tEnter;
   if (!slabHit(nodes[root].box, o, invD, tBest, tEnter)) return NULL;
   stack[sp] = root; stackT[sp] = tEnter; sp++;

   while (sp > 0) {
      sp--;
      const int ni = stack[sp];
      if (stackT[sp] >= tBest) continue;   // a closer hit was found meanwhile
      const Node& nd = nodes[ni];

      if (nd.left < 0) {
         Shape* const* leaf = &order[nd.start];
         for (int i = 0; i < nd.count; i++) {
            const double t = leaf[i]->getIntersection(ray);
            if (t < tBest) { tBest = t; best = leaf[i]; }
         }
         continue;
      }

      const int cl = nd.left, cr = nd.start;
      double tl, tr;
      const bool hl = slabHit(nodes[cl].box, o, invD, tBest, tl);
      const bool hr = slabHit(nodes[cr].box, o, invD, tBest, tr);
      // Push the farther child first so the nearer one is visited next.
      if (hl && hr) {
         if (tl <= tr) {
            stack[sp] = cr; stackT[sp] = tr; sp++;
            stack[sp] = cl; stackT[sp] = tl; sp++;
         } else {
            stack[sp] = cl; stackT[sp] = tl; sp++;
            stack[sp] = cr; stackT[sp] = tr; sp++;
         }
      } else if (hl) {
         stack[sp] = cl; stackT[sp] = tl; sp++;
      } else if (hr) {
         stack[sp] = cr; stackT[sp] = tr; sp++;
      }
   }
   return best;
}

bool BVH::anyLightHit(const Ray& ray, double* fill, double tMax) const {
   if (root < 0) return false;

   const Vector& o = ray.point;
   const Vector invD(1.0/ray.vector.x, 1.0/ray.vector.y, 1.0/ray.vector.z);

   int stack[64];
   int sp = 0;
   double tEnter;
   if (!slabHit(nodes[root].box, o, invD, tMax, tEnter)) return false;
   stack[sp++] = root;

   while (sp > 0) {
      const Node& nd = nodes[stack[--sp]];
      if (nd.left < 0) {
         Shape* const* leaf = &order[nd.start];
         for (int i = 0; i < nd.count; i++) {
            if (leaf[i]->getLightIntersection(ray, fill)) return true;
         }
         continue;
      }
      double tl, tr;
      if (slabHit(nodes[nd.left].box,  o, invD, tMax, tl)) stack[sp++] = nd.left;
      if (slabHit(nodes[nd.start].box, o, invD, tMax, tr)) stack[sp++] = nd.start;
   }
   return false;
}
