// This code is based off the Minkowski Portal Refinement algorithm by Gary Snethen in XenoCollide & Game Programming Gems 7.

namespace mpr
{
    struct CubePlanes
    {
        const clipplanes &p;
    
        CubePlanes(const clipplanes &p) : p(p) {}
    
        vec center() const { return p.o; }
    
        vec supportpoint(const vec &n) const
        {
            int besti = 7;
            float bestd = n.dot(p.v[7]);
            loopi(7)
            {
                float d = n.dot(p.v[i]);
                if(d > bestd) { besti = i; bestd = d; }
            }
            return p.v[besti];
        }
    };
    
    struct SolidCube
    {
        vec o;
        int size;
    
        SolidCube(float x, float y, float z, int size) : o(x, y, z), size(size) {}
        SolidCube(const vec &o, int size) : o(o), size(size) {}
        SolidCube(const ivec &o, int size) : o(o.tovec()), size(size) {}
 
        vec center() const { return vec(o).add(size/2); }
    
        vec supportpoint(const vec &n) const
        {
            vec p(o);
            if(n.x > 0) p.x += size;
            if(n.y > 0) p.y += size;
            if(n.z > 0) p.z += size; 
            return p;
        }
    };
    
    struct EntAABB
    {
        physent *ent;

        EntAABB(physent *ent) : ent(ent) {}

        vec center() const { vec o(ent->o); o.z += (ent->aboveeye - ent->eyeheight)/2; return o; }

        vec contactface(const vec &n, const vec &dir) const
        {
            vec an(n.x*dir.x < 0 ? fabs(n.x)/ent->xradius : 0, n.y*dir.y < 0 ? fabs(n.y)/ent->yradius : 0, n.z*dir.z < 0 ? fabs(n.z)*2/(ent->aboveeye + ent->eyeheight) : 0),
                fn(0, 0, 0);
            if(an.x > an.y)
            {
                if(an.x > an.z) fn.x = n.x > 0 ? 1 : -1;
                else if(an.z > 0) fn.z = n.z > 0 ? 1 : -1;
            }
            else if(an.y > an.z) fn.y = n.y > 0 ? 1 : -1;
            else if(an.z > 0) fn.z = n.z > 0 ? 1 : -1;
            return fn;
        }

        vec supportpoint(const vec &n) const
        {
            vec p(ent->o);
            if(n.x > 0) p.x += ent->xradius;
            else p.x -= ent->xradius;
            if(n.y > 0) p.y += ent->yradius;
            else p.y -= ent->yradius;
            if(n.z > 0) p.z += ent->aboveeye;
            else p.z -= ent->eyeheight;
            return p;
        }
    };

    struct EntOBB
    {
        physent *ent;
        quat orient;
        float zmargin;

        EntOBB(physent *ent, float zmargin = 0) : ent(ent), orient(vec(0, 0, 1), ent->yaw*RAD), zmargin(zmargin) {}

        vec center() const { vec o(ent->o); o.z += (ent->aboveeye - ent->eyeheight - zmargin)/2; return o; }

        vec contactface(const vec &wn, const vec &wdir) const
        {
            vec n = orient.invertedrotate(wn).div(vec(ent->xradius, ent->yradius, (ent->aboveeye + ent->eyeheight + zmargin)/2)), 
                dir = orient.invertedrotate(wdir), 
                an(fabs(n.x), fabs(n.y), dir.z ? fabs(n.z) : 0),
                fn(0, 0, 0);
            if(an.x > an.y)
            {
                if(an.x > an.z) fn.x = n.x*dir.x < 0 ? (n.x > 0 ? 1 : -1) : 0;
                else if(an.z > 0) fn.z = n.z*dir.z < 0 ? (n.z > 0 ? 1 : -1) : 0;
            }
            else if(an.y > an.z) fn.y = n.y*dir.y < 0 ? (n.y > 0 ? 1 : -1) : 0;
            else if(an.z > 0) fn.z = n.z*dir.z < 0 ? (n.z > 0 ? 1 : -1) : 0;
            return orient.rotate(fn);
        }

        vec supportpoint(const vec &n) const
        {
            vec ln = orient.invertedrotate(n), p(0, 0, 0);
            if(ln.x > 0) p.x += ent->xradius;
            else p.x -= ent->xradius;
            if(ln.y > 0) p.y += ent->yradius;
            else p.y -= ent->yradius;
            if(ln.z > 0) p.z += ent->aboveeye;
            else p.z -= ent->eyeheight + zmargin;
            return orient.rotate(p).add(ent->o);
        }
    };

    struct EntCylinder
    {
        physent *ent;
        float zmargin;
 
        EntCylinder(physent *ent, float zmargin = 0) : ent(ent), zmargin(zmargin) {}
    
        vec center() const { vec o(ent->o); o.z += (ent->aboveeye - ent->eyeheight - zmargin)/2; return o; }
   
        vec contactface(const vec &n, const vec &dir) const
        {
            float dxy = n.dot2(n)/(ent->radius*ent->radius), dz = n.z*n.z*4/(ent->aboveeye + ent->eyeheight + zmargin);
            vec fn(0, 0, 0);
            if(dz > dxy && dir.z) fn.z = n.z*dir.z < 0 ? (n.z > 0 ? 1 : -1) : 0;
            else if(n.dot2(dir) < 0) 
            {
                fn.x = n.x;
                fn.y = n.y;
                fn.normalize();
            }
            return fn;
        }
 
        vec supportpoint(const vec &n) const
        {
            vec p(ent->o);
            if(n.z > 0) p.z += ent->aboveeye;
            else p.z -= ent->eyeheight + zmargin;
            if(n.x || n.y)
            {
                float r = ent->radius / n.magnitude2();
                p.x += n.x*r;
                p.y += n.y*r;
            }
            return p;
        }
    };

    struct EntCapsule
    {
        physent *ent;

        EntCapsule(physent *ent) : ent(ent) {}

        vec center() const { vec o(ent->o); o.z += (ent->aboveeye - ent->eyeheight)/2; return o; }

        vec supportpoint(const vec &n) const
        {
            vec p(ent->o);
            if(n.z > 0) p.z += ent->aboveeye - ent->radius;
            else p.z -= ent->eyeheight - ent->radius;
            p.add(vec(n).mul(ent->radius / n.magnitude()));
            return p;
        }
    };

    struct EntEllipsoid
    {
        physent *ent;

        EntEllipsoid(physent *ent) : ent(ent) {}

        vec center() const { vec o(ent->o); o.z += (ent->aboveeye - ent->eyeheight)/2; return o; }

        vec supportpoint(const vec &dir) const
        {
            vec p(ent->o), n = vec(dir).normalize();
            p.x += ent->radius*n.x;
            p.y += ent->radius*n.y; 
            p.z += (ent->aboveeye + ent->eyeheight)/2*(1 + n.z) - ent->eyeheight;
            return p;
        }
    };

    struct ModelOBB
    {
        vec o, radius;
        quat orient;

        ModelOBB(const vec &ent, const vec &center, const vec &radius, float yaw) : o(ent), radius(radius), orient(vec(0, 0, 1), yaw*RAD) 
        {
            o.add(orient.rotate(center));
        }

        vec center() const { return o; }

        vec contactface(const vec &wn, const vec &wdir) const
        {
            vec n = orient.invertedrotate(wn).div(radius), dir = orient.invertedrotate(wdir),
                an(fabs(n.x), fabs(n.y), dir.z ? fabs(n.z) : 0),
                fn(0, 0, 0);
            if(an.x > an.y)
            {
                if(an.x > an.z) fn.x = n.x*dir.x < 0 ? (n.x > 0 ? 1 : -1) : 0;
                else if(an.z > 0) fn.z = n.z*dir.z < 0 ? (n.z > 0 ? 1 : -1) : 0;
            }
            else if(an.y > an.z) fn.y = n.y*dir.y < 0 ? (n.y > 0 ? 1 : -1) : 0;
            else if(an.z > 0) fn.z = n.z*dir.z < 0 ? (n.z > 0 ? 1 : -1) : 0;
            return orient.rotate(fn);
        }

        vec supportpoint(const vec &n) const
        {
            vec ln = orient.invertedrotate(n), p(0, 0, 0);
            if(ln.x > 0) p.x += radius.x;
            else p.x -= radius.x;
            if(ln.y > 0) p.y += radius.y;
            else p.y -= radius.y;
            if(ln.z > 0) p.z += radius.z;
            else p.z -= radius.z;
            return orient.rotate(p).add(o);
        }
    };

    struct ModelEllipse
    {
        vec o, radius;
        quat orient;

        ModelEllipse(const vec &ent, const vec &center, const vec &radius, float yaw) : o(ent), radius(radius), orient(vec(0, 0, 1), yaw*RAD) 
        {
            o.add(orient.rotate(center));
        }

        vec center() const { return o; }

        vec contactface(const vec &wn, const vec &wdir) const
        {
            vec n = orient.invertedrotate(wn).div(radius), dir = orient.invertedrotate(wdir);
            float dxy = n.dot2(n), dz = n.z*n.z;
            vec fn(0, 0, 0);
            if(dz > dxy && dir.z) fn.z = n.z*dir.z < 0 ? (n.z > 0 ? 1 : -1) : 0;
            else if(n.dot2(dir) < 0) 
            {
                fn.x = n.x*radius.x;
                fn.y = n.y*radius.y;
                fn.normalize();
            }
            return orient.rotate(fn);
        }

        vec supportpoint(const vec &n) const
        {
            vec ln = orient.invertedrotate(n), p(0, 0, 0);
            if(ln.z > 0) p.z += radius.z;
            else p.z -= radius.z;
            if(ln.x || ln.y)
            {
                float r = n.magnitude2();
                p.x += ln.x*radius.x/r;
                p.y += ln.y*radius.y/r;
            }
            return orient.rotate(p).add(o);
        }
    };
 
    const float boundarytolerance = 1e-3f;
        
    template<class T, class U>
    bool collide(const T &p1, const U &p2)
    {
        // v0 = center of Minkowski difference
        vec v0 = p2.center().sub(p1.center());
        if(v0.iszero()) return true;  // v0 and origin overlap ==> hit
    
        // v1 = support in direction of origin
        vec n = vec(v0).neg();
        vec v1 = p2.supportpoint(n).sub(p1.supportpoint(vec(n).neg()));
        if(v1.dot(n) <= 0) return false;  // origin outside v1 support plane ==> miss
    
        // v2 = support perpendicular to plane containing origin, v0 and v1
        n.cross(v1, v0);
        if(n.iszero()) return true;   // v0, v1 and origin colinear (and origin inside v1 support plane) == > hit
        vec v2 = p2.supportpoint(n).sub(p1.supportpoint(vec(n).neg()));
        if(v2.dot(n) <= 0) return false;  // origin outside v2 support plane ==> miss
    
        // v3 = support perpendicular to plane containing v0, v1 and v2
        n.cross(v0, v1, v2);
    
        // If the origin is on the - side of the plane, reverse the direction of the plane
        if(n.dot(v0) > 0)
        {
            swap(v1, v2);
            n.neg();
        }
    
        ///
        // Phase One: Find a valid portal
    
        loopi(100)
        {
            // Obtain the next support point
            vec v3 = p2.supportpoint(n).sub(p1.supportpoint(vec(n).neg()));
            if(v3.dot(n) <= 0) return false;  // origin outside v3 support plane ==> miss
    
            // If origin is outside (v1,v0,v3), then portal is invalid -- eliminate v2 and find new support outside face
            vec v3xv0;
            v3xv0.cross(v3, v0);
            if(v1.dot(v3xv0) < 0)
            {
                v2 = v3;
                n.cross(v0, v1, v3);
                continue;
            }
    
            // If origin is outside (v3,v0,v2), then portal is invalid -- eliminate v1 and find new support outside face
            if(v2.dot(v3xv0) > 0)
            {
                v1 = v3;
                n.cross(v0, v3, v2);
                continue;
            }
    
            ///
            // Phase Two: Refine the portal
    
            for(int j = 0;; j++)
            {
                // Compute outward facing normal of the portal
                n.cross(v1, v2, v3);
    
                // If the origin is inside the portal, we have a hit
                if(n.dot(v1) >= 0) return true;
    
                n.normalize();

                // Find the support point in the direction of the portal's normal
                vec v4 = p2.supportpoint(n).sub(p1.supportpoint(vec(n).neg()));
    
                // If the origin is outside the support plane or the boundary is thin enough, we have a miss
                if(v4.dot(n) <= 0 || vec(v4).sub(v3).dot(n) <= boundarytolerance || j > 100) return false;

                // Test origin against the three planes that separate the new portal candidates: (v1,v4,v0) (v2,v4,v0) (v3,v4,v0)
                // Note:  We're taking advantage of the triple product identities here as an optimization
                //        (v1 % v4) * v0 == v1 * (v4 % v0)    > 0 if origin inside (v1, v4, v0)
                //        (v2 % v4) * v0 == v2 * (v4 % v0)    > 0 if origin inside (v2, v4, v0)
                //        (v3 % v4) * v0 == v3 * (v4 % v0)    > 0 if origin inside (v3, v4, v0)
                vec v4xv0;
                v4xv0.cross(v4, v0);
                if(v1.dot(v4xv0) > 0)
                {
                    if(v2.dot(v4xv0) > 0) v1 = v4;    // Inside v1 & inside v2 ==> eliminate v1
                    else v3 = v4;                   // Inside v1 & outside v2 ==> eliminate v3
                }
                else
                {
                    if(v3.dot(v4xv0) > 0) v2 = v4;    // Outside v1 & inside v3 ==> eliminate v2
                    else v1 = v4;                   // Outside v1 & outside v3 ==> eliminate v1
                }
            }
        }
        return false;
    }

    template<class T, class U>
    bool collide(const T &p1, const U &p2, vec *contactnormal, vec *contactpoint1, vec *contactpoint2)
    {
        // v0 = center of Minkowski sum
        vec v01 = p1.center();
        vec v02 = p2.center();
        vec v0 = vec(v02).sub(v01);
    
        // Avoid case where centers overlap -- any direction is fine in this case
        if(v0.iszero()) v0 = vec(0, 0, 1e-5f);
    
        // v1 = support in direction of origin
        vec n = vec(v0).neg();
        vec v11 = p1.supportpoint(vec(n).neg());
        vec v12 = p2.supportpoint(n);
        vec v1 = vec(v12).sub(v11);
        if(v1.dot(n) <= 0)
        {
            if(contactnormal) *contactnormal = n;
            return false;
        }
    
        // v2 - support perpendicular to v1,v0
        n.cross(v1, v0);
        if(n.iszero())
        {
            n = vec(v1).sub(v0);
            n.normalize();
            if(contactnormal) *contactnormal = n;
            if(contactpoint1) *contactpoint1 = v11;
            if(contactpoint2) *contactpoint2 = v12;
            return true;
        }
        vec v21 = p1.supportpoint(vec(n).neg());
        vec v22 = p2.supportpoint(n);
        vec v2 = vec(v22).sub(v21);
        if(v2.dot(n) <= 0)
        {
            if(contactnormal) *contactnormal = n;
            return false;
        }
    
        // Determine whether origin is on + or - side of plane (v1,v0,v2)
        n.cross(v0, v1, v2);
        ASSERT( !n.iszero() );
        // If the origin is on the - side of the plane, reverse the direction of the plane
        if(n.dot(v0) > 0)
        {
            swap(v1, v2);
            swap(v11, v21);
            swap(v12, v22);
            n.neg();
        }
    
        ///
        // Phase One: Identify a portal
    
        loopi(100)
        {
            // Obtain the support point in a direction perpendicular to the existing plane
            // Note: This point is guaranteed to lie off the plane
            vec v31 = p1.supportpoint(vec(n).neg());
            vec v32 = p2.supportpoint(n);
            vec v3 = vec(v32).sub(v31);
            if(v3.dot(n) <= 0)
            {
                if(contactnormal) *contactnormal = n;
                return false;
            }
    
            // If origin is outside (v1,v0,v3), then eliminate v2 and loop
            vec v3xv0;
            v3xv0.cross(v3, v0);
            if(v1.dot(v3xv0) < 0)
            {
                v2 = v3;
                v21 = v31;
                v22 = v32;
                n.cross(v0, v1, v3);
                continue;
            }
    
            // If origin is outside (v3,v0,v2), then eliminate v1 and loop
            if(v2.dot(v3xv0) > 0)
            {
                v1 = v3;
                v11 = v31;
                v12 = v32;
                n.cross(v0, v3, v2);
                continue;
            }
    
            bool hit = false;
    
            ///
            // Phase Two: Refine the portal
    
            // We are now inside of a wedge...
            for(int j = 0;; j++)
            {
                // Compute normal of the wedge face
                n.cross(v1, v2, v3);
    
                // Can this happen???  Can it be handled more cleanly?
                if(n.iszero())
                {
                    ASSERT(0);
                    return true;
                }
    
                n.normalize();
    
                // If the origin is inside the wedge, we have a hit
                if(n.dot(v1) >= 0 && !hit)
                {
                    if(contactnormal) *contactnormal = n;
    
                    // Compute the barycentric coordinates of the origin
                    if(contactpoint1 || contactpoint2)
                    {
                        float b0 = v3.scalartriple(v1, v2),
                              b1 = v0.scalartriple(v3, v2),
                              b2 = v3.scalartriple(v0, v1),
                              b3 = v0.scalartriple(v2, v1),
                              sum = b0 + b1 + b2 + b3;
                        if(sum <= 0)
                        {
                            b0 = 0;
                            b1 = n.scalartriple(v2, v3);
                            b2 = n.scalartriple(v3, v1);
                            b3 = n.scalartriple(v1, v2);
                            sum = b1 + b2 + b3;
                        }
                        if(contactpoint1)
                            *contactpoint1 = (vec(v01).mul(b0).add(vec(v11).mul(b1)).add(vec(v21).mul(b2)).add(vec(v31).mul(b3))).mul(1.0f/sum);
                        if(contactpoint2)
                            *contactpoint2 = (vec(v02).mul(b0).add(vec(v12).mul(b1)).add(vec(v22).mul(b2)).add(vec(v32).mul(b3))).mul(1.0f/sum);
                    }
    
                    // HIT!!!
                    hit = true;
                }
    
                // Find the support point in the direction of the wedge face
                vec v41 = p1.supportpoint(vec(n).neg());
                vec v42 = p2.supportpoint(n);
                vec v4 = vec(v42).sub(v41);
    
                // If the boundary is thin enough or the origin is outside the support plane for the newly discovered vertex, then we can terminate
                if(v4.dot(n) <= 0 || vec(v4).sub(v3).dot(n) <= boundarytolerance || j > 100)
                {
                    if(contactnormal) *contactnormal = n;
                    return hit;
                }
  
                // Test origin against the three planes that separate the new portal candidates: (v1,v4,v0) (v2,v4,v0) (v3,v4,v0)
                // Note:  We're taking advantage of the triple product identities here as an optimization
                //        (v1 % v4) * v0 == v1 * (v4 % v0)    > 0 if origin inside (v1, v4, v0)
                //        (v2 % v4) * v0 == v2 * (v4 % v0)    > 0 if origin inside (v2, v4, v0)
                //        (v3 % v4) * v0 == v3 * (v4 % v0)    > 0 if origin inside (v3, v4, v0)
                vec v4xv0;
                v4xv0.cross(v4, v0);
                if(v1.dot(v4xv0) > 0) // Compute the tetrahedron dividing face d1 = (v4,v0,v1)
                {
                    if(v2.dot(v4xv0) > 0) // Compute the tetrahedron dividing face d2 = (v4,v0,v2)
                    {
                        // Inside d1 & inside d2 ==> eliminate v1
                        v1 = v4;
                        v11 = v41;
                        v12 = v42;
                    }
                    else
                    {
                        // Inside d1 & outside d2 ==> eliminate v3
                        v3 = v4;
                        v31 = v41;
                        v32 = v42;
                    }
                }
                else
                {
                    if(v3.dot(v4xv0) > 0) // Compute the tetrahedron dividing face d3 = (v4,v0,v3)
                    {
                        // Outside d1 & inside d3 ==> eliminate v2
                        v2 = v4;
                        v21 = v41;
                        v22 = v42;
                    }
                    else
                    {
                        // Outside d1 & outside d3 ==> eliminate v1
                        v1 = v4;
                        v11 = v41;
                        v12 = v42;
                    }
                }
            }
        }
        return false;
    }
}

