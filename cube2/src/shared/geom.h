struct vec4;
struct vec2;

struct vec
{
    union
    {
        struct { float x, y, z; };
        struct { float r, g, b; };
        float v[3];
    };

    vec() {}
    explicit vec(int a) : x(a), y(a), z(a) {} 
    explicit vec(float a) : x(a), y(a), z(a) {} 
    vec(float a, float b, float c) : x(a), y(b), z(c) {}
    explicit vec(int v[3]) : x(v[0]), y(v[1]), z(v[2]) {}
    explicit vec(float *v) : x(v[0]), y(v[1]), z(v[2]) {}
    explicit vec(const vec4 &v);
    explicit vec(const vec2 &v, float z = 0);

    vec(float yaw, float pitch) : x(-sinf(yaw)*cosf(pitch)), y(cosf(yaw)*cosf(pitch)), z(sinf(pitch)) {}

    float &operator[](int i)       { return v[i]; }
    float  operator[](int i) const { return v[i]; }
    
    vec &set(int i, float f) { v[i] = f; return *this; }

    bool operator==(const vec &o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const vec &o) const { return x != o.x || y != o.y || z != o.z; }

    bool iszero() const { return x==0 && y==0 && z==0; }
    float squaredlen() const { return x*x + y*y + z*z; }
    float dot2(const vec &o) const { return x*o.x + y*o.y; }
    float dot(const vec &o) const { return x*o.x + y*o.y + z*o.z; }
    vec &mul(const vec &o)   { x *= o.x; y *= o.y; z *= o.z; return *this; }
    vec &mul(float f)        { x *= f; y *= f; z *= f; return *this; }
    vec &div(const vec &o)   { x /= o.x; y /= o.y; z /= o.z; return *this; }
    vec &div(float f)        { x /= f; y /= f; z /= f; return *this; }
    vec &add(const vec &o)   { x += o.x; y += o.y; z += o.z; return *this; }
    vec &add(float f)        { x += f; y += f; z += f; return *this; }
    vec &sub(const vec &o)   { x -= o.x; y -= o.y; z -= o.z; return *this; }
    vec &sub(float f)        { x -= f; y -= f; z -= f; return *this; }
    vec &neg2()              { x = -x; y = -y; return *this; }
    vec &neg()               { x = -x; y = -y; z = -z; return *this; }
    vec &min(const vec &o)   { x = ::min(x, o.x); y = ::min(y, o.y); z = ::min(z, o.z); return *this; }
    vec &max(const vec &o)   { x = ::max(x, o.x); y = ::max(y, o.y); z = ::max(z, o.z); return *this; }
    vec &min(float f)        { x = ::min(x, f); y = ::min(y, f); z = ::min(z, f); return *this; }
    vec &max(float f)        { x = ::max(x, f); y = ::max(y, f); z = ::max(z, f); return *this; }
    vec &clamp(float f, float h) { x = ::clamp(x, f, h); y = ::clamp(y, f, h); z = ::clamp(z, f, h); return *this; }
    float magnitude2() const { return sqrtf(dot2(*this)); }
    float magnitude() const  { return sqrtf(squaredlen()); }
    vec &normalize()         { div(magnitude()); return *this; }
    bool isnormalized() const { float m = squaredlen(); return (m>0.99f && m<1.01f); }
    float squaredist(const vec &e) const { return vec(*this).sub(e).squaredlen(); }
    float dist(const vec &e) const { vec t; return dist(e, t); }
    float dist(const vec &e, vec &t) const { t = *this; t.sub(e); return t.magnitude(); }
    float dist2(const vec &o) const { float dx = x-o.x, dy = y-o.y; return sqrtf(dx*dx + dy*dy); }
    bool reject(const vec &o, float r) { return x>o.x+r || x<o.x-r || y>o.y+r || y<o.y-r; }
    template<class A, class B>
    vec &cross(const A &a, const B &b) { x = a.y*b.z-a.z*b.y; y = a.z*b.x-a.x*b.z; z = a.x*b.y-a.y*b.x; return *this; }
    vec &cross(const vec &o, const vec &a, const vec &b) { return cross(vec(a).sub(o), vec(b).sub(o)); }
    float scalartriple(const vec &a, const vec &b) const { return x*(a.y*b.z-a.z*b.y) + y*(a.z*b.x-a.x*b.z) + z*(a.x*b.y-a.y*b.x); }
    vec &reflectz(float rz) { z = 2*rz - z; return *this; }
    vec &reflect(const vec &n) { float k = 2*dot(n); x -= k*n.x; y -= k*n.y; z -= k*n.z; return *this; }
    vec &project(const vec &n) { float k = dot(n); x -= k*n.x; y -= k*n.y; z -= k*n.z; return *this; }
    vec &projectxydir(const vec &n) { if(n.z) z = -(x*n.x/n.z + y*n.y/n.z); return *this; }
    vec &projectxy(const vec &n)
    {
        float m = squaredlen(), k = dot(n);
        projectxydir(n);
        rescale(sqrtf(::max(m - k*k, 0.0f)));
        return *this;
    }
    vec &projectxy(const vec &n, float threshold)
    {
        float m = squaredlen(), k = ::min(dot(n), threshold);
        projectxydir(n);
        rescale(sqrtf(::max(m - k*k, 0.0f)));
        return *this;
    }
    vec &lerp(const vec &b, float t) { x += (b.x-x)*t; y += (b.y-y)*t; z += (b.z-z)*t; return *this; }
    vec &lerp(const vec &a, const vec &b, float t) { x = a.x + (b.x-a.x)*t; y = a.y + (b.y-a.y)*t; z = a.z + (b.z-a.z)*t; return *this; }

    vec &rescale(float k)
    {
        float mag = magnitude();
        if(mag > 1e-6f) mul(k / mag);
        return *this;
    }

    vec &rotate_around_z(float c, float s) { float rx = x, ry = y; x = c*rx-s*ry; y = c*ry+s*rx; return *this; }
    vec &rotate_around_x(float c, float s) { float ry = y, rz = z; y = c*ry-s*rz; z = c*rz+s*ry; return *this; }
    vec &rotate_around_y(float c, float s) { float rx = x, rz = z; x = c*rx-s*rz; z = c*rz+s*rx; return *this; }

    vec &rotate_around_z(float angle) { return rotate_around_z(cosf(angle), sinf(angle)); }
    vec &rotate_around_x(float angle) { return rotate_around_x(cosf(angle), sinf(angle)); }
    vec &rotate_around_y(float angle) { return rotate_around_y(cosf(angle), sinf(angle)); }

    vec &rotate(float angle, const vec &d)
    {
        float c = cosf(angle), s = sinf(angle);
        return rotate(c, s, d);
    }

    vec &rotate(float c, float s, const vec &d)
    {
        *this = vec(x*(d.x*d.x*(1-c)+c) + y*(d.x*d.y*(1-c)-d.z*s) + z*(d.x*d.z*(1-c)+d.y*s),
                    x*(d.y*d.x*(1-c)+d.z*s) + y*(d.y*d.y*(1-c)+c) + z*(d.y*d.z*(1-c)-d.x*s),
                    x*(d.x*d.z*(1-c)-d.y*s) + y*(d.y*d.z*(1-c)+d.x*s) + z*(d.z*d.z*(1-c)+c));
        return *this;
    }

    void orthogonal(const vec &d)
    {
        int i = fabs(d.x) > fabs(d.y) ? (fabs(d.x) > fabs(d.z) ? 0 : 2) : (fabs(d.y) > fabs(d.z) ? 1 : 2); 
        v[i] = d[(i+1)%3];
        v[(i+1)%3] = -d[i];
        v[(i+2)%3] = 0;
    }

    void orthonormalize(vec &s, vec &t) const
    {
        s.sub(vec(*this).mul(dot(s)));
        t.sub(vec(*this).mul(dot(t)))
         .sub(vec(s).mul(s.dot(t)));
    }

    template<class T> float dist_to_bb(const T &min, const T &max) const
    {
        float sqrdist = 0;
        loopi(3)
        {
            if     (v[i] < min[i]) { float delta = v[i]-min[i]; sqrdist += delta*delta; }
            else if(v[i] > max[i]) { float delta = max[i]-v[i]; sqrdist += delta*delta; }
        }
        return sqrtf(sqrdist);
    }

    template<class T, class S> float dist_to_bb(const T &o, S size) const
    {
        return dist_to_bb(o, T(o).add(size));
    }
};

static inline bool htcmp(const vec &x, const vec &y)
{
    return x == y;
}

static inline uint hthash(const vec &k)
{
    union { uint i; float f; } x, y, z;
    x.f = k.x; y.f = k.y; z.f = k.z;
    uint v = x.i^y.i^z.i;
    return v + (v>>12);
}

struct vec4
{
    union
    {
        struct { float x, y, z, w; };
        float v[4];
    };

    vec4() {}
    explicit vec4(const vec &p, float w = 0) : x(p.x), y(p.y), z(p.z), w(w) {}
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    float &operator[](int i)       { return v[i]; }
    float  operator[](int i) const { return v[i]; }

    float dot3(const vec4 &o) const { return x*o.x + y*o.y + z*o.z; }
    float dot3(const vec &o) const { return x*o.x + y*o.y + z*o.z; }
    float dot(const vec4 &o) const { return dot3(o) + w*o.w; }
    float dot(const vec &o) const  { return x*o.x + y*o.y + z*o.z + w; }
    float squaredlen() const { return dot(*this); }
    float magnitude() const  { return sqrtf(squaredlen()); }
    float magnitude3() const { return sqrtf(dot3(*this)); }
    vec4 &normalize() { mul(1/magnitude()); return *this; }

    vec4 &lerp(const vec4 &b, float t)
    {
        x += (b.x-x)*t;
        y += (b.y-y)*t;
        z += (b.z-z)*t;
        w += (b.w-w)*t;
        return *this;
    }
    vec4 &lerp(const vec4 &a, const vec4 &b, float t) 
    { 
        x = a.x+(b.x-a.x)*t; 
        y = a.y+(b.y-a.y)*t; 
        z = a.z+(b.z-a.z)*t;
        w = a.w+(b.w-a.w)*t;
        return *this;
    }

    vec4 &mul3(float f)      { x *= f; y *= f; z *= f; return *this; }
    vec4 &mul(float f)       { mul3(f); w *= f; return *this; }
    vec4 &mul(const vec4 &o) { x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
    vec4 &div3(float f)      { x /= f; y /= f; z /= f; return *this; }
    vec4 &div(float f)       { div3(f); w /= f; return *this; }
    vec4 &div(const vec4 &o) { x /= o.x; y /= o.y; z /= o.z; w /= o.w; return *this; }
    vec4 &add(const vec4 &o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    vec4 &addw(float f)      { w += f; return *this; }
    vec4 &sub(const vec4 &o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
    vec4 &subw(float f)      { w -= f; return *this; }
    vec4 &neg3()             { x = -x; y = -y; z = -z; return *this; }
    vec4 &neg()              { neg3(); w = -w; return *this; }

    void setxyz(const vec &v) { x = v.x; y = v.y; z = v.z; }
};

inline vec::vec(const vec4 &v) : x(v.x), y(v.y), z(v.z) {}

struct vec2
{
    union
    {
        struct { float x, y; };
        float v[2];
    };

    vec2() {}
    vec2(float x, float y) : x(x), y(y) {}
    explicit vec2(const vec &v) : x(v.x), y(v.y) {}
    explicit vec2(const vec4 &v) : x(v.x), y(v.y) {}

    float &operator[](int i)       { return v[i]; }
    float  operator[](int i) const { return v[i]; }

    bool operator==(const vec2 &o) const { return x == o.x && y == o.y; }
    bool operator!=(const vec2 &o) const { return x != o.x || y != o.y; }

    bool iszero() const { return x==0 && y==0; }
    float dot(const vec2 &o) const  { return x*o.x + y*o.y; }
    float squaredlen() const { return dot(*this); }
    float magnitude() const  { return sqrtf(squaredlen()); }
    vec2 &normalize() { mul(1/magnitude()); return *this; }
    float cross(const vec2 &o) const { return x*o.y - y*o.x; }

    vec2 &mul(float f)       { x *= f; y *= f; return *this; }
    vec2 &mul(const vec2 &o) { x *= o.x; y *= o.y; return *this; }
    vec2 &div(float f)       { x /= f; y /= f; return *this; }
    vec2 &div(const vec2 &o) { x /= o.x; y /= o.y; return *this; }
    vec2 &add(float f)       { x += f; y += f; return *this; }
    vec2 &add(const vec2 &o) { x += o.x; y += o.y; return *this; }
    vec2 &sub(float f)       { x -= f; y -= f; return *this; }
    vec2 &sub(const vec2 &o) { x -= o.x; y -= o.y; return *this; }
    vec2 &neg()              { x = -x; y = -y; return *this; }
};

inline vec::vec(const vec2 &v, float z) : x(v.x), y(v.y), z(z) {}

static inline bool htcmp(const vec2 &x, const vec2 &y)
{
    return x == y;
}

static inline uint hthash(const vec2 &k)
{
    union { uint i; float f; } x, y;
    x.f = k.x; y.f = k.y;
    uint v = x.i^y.i;
    return v + (v>>12);
}

struct matrix3x3;
struct matrix3x4;

struct quat : vec4
{
    quat() {}
    quat(float x, float y, float z, float w) : vec4(x, y, z, w) {}
    quat(const vec &axis, float angle)
    {
        w = cosf(angle/2);
        float s = sinf(angle/2);
        x = s*axis.x;
        y = s*axis.y;
        z = s*axis.z;
    }
    explicit quat(const vec &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        restorew();
    }
    explicit quat(const matrix3x3 &m) { convertmatrix(m); }
    explicit quat(const matrix3x4 &m) { convertmatrix(m); }

    void restorew() { w = 1.0f-x*x-y*y-z*z; w = w<0 ? 0 : -sqrtf(w); }
    
    quat &add(const vec4 &o) { vec4::add(o); return *this; }
    quat &sub(const vec4 &o) { vec4::sub(o); return *this; }
    quat &mul(float k) { vec4::mul(k); return *this; }

    quat &mul(const quat &p, const quat &o)
    {
        x = p.w*o.x + p.x*o.w + p.y*o.z - p.z*o.y;
        y = p.w*o.y - p.x*o.z + p.y*o.w + p.z*o.x;
        z = p.w*o.z + p.x*o.y - p.y*o.x + p.z*o.w;
        w = p.w*o.w - p.x*o.x - p.y*o.y - p.z*o.z;
        return *this;
    }
    quat &mul(const quat &o) { return mul(quat(*this), o); }

    quat &invert() { neg3(); return *this; }

    void calcangleaxis(float &angle, vec &axis)
    {
        float rr = dot3(*this);
        if(rr>0)
        {
            angle = 2*acosf(w);
            axis = vec(x, y, z).mul(1/rr); 
        }
        else { angle = 0; axis = vec(0, 0, 1); }
    }

    vec rotate(const vec &v) const
    {
        return vec().cross(*this, vec().cross(*this, v).add(vec(v).mul(w))).mul(2).add(v);
    }

    vec invertedrotate(const vec &v) const
    {
        return vec().cross(*this, vec().cross(*this, v).sub(vec(v).mul(w))).mul(2).add(v);
    }

    template<class M>
    void convertmatrix(const M &m)
    {
        float trace = m.a.x + m.b.y + m.c.z;
        if(trace>0)
        {
            float r = sqrtf(1 + trace), inv = 0.5f/r;
            w = 0.5f*r;
            x = (m.c.y - m.b.z)*inv;
            y = (m.a.z - m.c.x)*inv;
            z = (m.b.x - m.a.y)*inv;
        }
        else if(m.a.x > m.b.y && m.a.x > m.c.z)
        {
            float r = sqrtf(1 + m.a.x - m.b.y - m.c.z), inv = 0.5f/r;
            x = 0.5f*r;
            y = (m.b.x + m.a.y)*inv;
            z = (m.a.z + m.c.x)*inv;
            w = (m.c.y - m.b.z)*inv;
        }
        else if(m.b.y > m.c.z)
        {
            float r = sqrtf(1 + m.b.y - m.a.x - m.c.z), inv = 0.5f/r;
            x = (m.b.x + m.a.y)*inv;
            y = 0.5f*r;
            z = (m.c.y + m.b.z)*inv;
            w = (m.a.z - m.c.x)*inv;
        }
        else
        {
            float r = sqrtf(1 + m.c.z - m.a.x - m.b.y), inv = 0.5f/r;
            x = (m.a.z + m.c.x)*inv;
            y = (m.c.y + m.b.z)*inv;
            z = 0.5f*r;
            w = (m.b.x - m.a.y)*inv;
        }
    }
};

struct dualquat
{
    quat real, dual;

    dualquat() {}
    dualquat(const quat &q, const vec &p) 
        : real(q),
          dual(0.5f*( p.x*q.w + p.y*q.z - p.z*q.y),
               0.5f*(-p.x*q.z + p.y*q.w + p.z*q.x),
               0.5f*( p.x*q.y - p.y*q.x + p.z*q.w),
              -0.5f*( p.x*q.x + p.y*q.y + p.z*q.z))
    {
    }
    explicit dualquat(const quat &q) : real(q), dual(0, 0, 0, 0) {}
    explicit dualquat(const matrix3x4 &m);

    dualquat &mul(float k) { real.mul(k); dual.mul(k); return *this; }
    dualquat &add(const dualquat &d) { real.add(d.real); dual.add(d.dual); return *this; }

    dualquat &lerp(const dualquat &to, float t)
    {
        float k = real.dot(to.real) < 0 ? -t : t;
        real.mul(1-t).add(vec4(to.real).mul(k));
        dual.mul(1-t).add(vec4(to.dual).mul(k));
        return *this;
    }
    dualquat &lerp(const dualquat &from, const dualquat &to, float t)
    {
        float k = from.real.dot(to.real) < 0 ? -t : t;
        (real = from.real).mul(1-t).add(vec4(to.real).mul(k));
        (dual = from.dual).mul(1-t).add(vec4(to.dual).mul(k));
        return *this;
    }

    dualquat &invert()
    {
        real.invert();
        dual.invert();
        dual.sub(quat(real).mul(2*real.dot(dual)));
        return *this;
    }
    
    void mul(const dualquat &p, const dualquat &o)
    {
        real.mul(p.real, o.real);
        dual.mul(p.real, o.dual).add(quat().mul(p.dual, o.real));
    }       
    void mul(const dualquat &o) { mul(dualquat(*this), o); }    
  
    void mulorient(const quat &q)
    {
        real.mul(q, quat(real));
        dual.mul(quat(q).invert(), quat(dual));
    }

    void mulorient(const quat &q, const dualquat &base)
    {
        quat trans;
        trans.mul(base.dual, quat(base.real).invert());
        dual.mul(quat(q).invert(), quat(real).mul(trans).add(dual));

        real.mul(q, quat(real));
        dual.add(quat().mul(real, trans.invert())).sub(quat(real).mul(2*base.real.dot(base.dual)));
    }

    void normalize()
    {
        float invlen = 1/real.magnitude();
        real.mul(invlen);
        dual.mul(invlen);
    }

    void translate(const vec &p)
    {
        dual.x +=  0.5f*( p.x*real.w + p.y*real.z - p.z*real.y);
        dual.y +=  0.5f*(-p.x*real.z + p.y*real.w + p.z*real.x);
        dual.z +=  0.5f*( p.x*real.y - p.y*real.x + p.z*real.w);
        dual.w += -0.5f*( p.x*real.x + p.y*real.y + p.z*real.z);
    }

    void scale(float k)
    {
        dual.mul(k);
    }

    void fixantipodal(const dualquat &d)
    {
        if(real.dot(d.real) < 0)
        {
            real.neg();
            dual.neg();
        }
    }

    void accumulate(const dualquat &d, float k)
    {
        if(real.dot(d.real) < 0) k = -k;
        real.add(vec4(d.real).mul(k));
        dual.add(vec4(d.dual).mul(k));
    }

    vec transform(const vec &v) const
    {
        return vec().cross(real, vec().cross(real, v).add(vec(v).mul(real.w)).add(vec(dual))).add(vec(dual).mul(real.w)).sub(vec(real).mul(dual.w)).mul(2).add(v);
    }

    vec transposedtransform(const vec &v) const
    {
        return dualquat(*this).invert().transform(v);
    }

    vec transformnormal(const vec &v) const
    {
        return real.rotate(v);
    }

    vec transposedtransformnormal(const vec &v) const
    {
        return real.invertedrotate(v);
    }

    vec gettranslation() const
    {
        return vec().cross(real, dual).add(vec(dual).mul(real.w)).sub(vec(real).mul(dual.w)).mul(2);
    }
};

struct matrix3x3
{
    vec a, b, c;

    matrix3x3() {}
    matrix3x3(const vec &a, const vec &b, const vec &c) : a(a), b(b), c(c) {}
    explicit matrix3x3(float angle, const vec &axis) { rotate(angle, axis); }
    explicit matrix3x3(const quat &q)
    {
        float x = q.x, y = q.y, z = q.z, w = q.w,
              tx = 2*x, ty = 2*y, tz = 2*z,
              txx = tx*x, tyy = ty*y, tzz = tz*z,
              txy = tx*y, txz = tx*z, tyz = ty*z,
              twx = w*tx, twy = w*ty, twz = w*tz;
        a = vec(1 - (tyy + tzz), txy - twz, txz + twy);
        b = vec(txy + twz, 1 - (txx + tzz), tyz - twx);
        c = vec(txz - twy, tyz + twx, 1 - (txx + tyy));
    }

    void mul(const matrix3x3 &m, const matrix3x3 &n)
    {
        a = vec(n.a).mul(m.a.x).add(vec(n.b).mul(m.a.y)).add(vec(n.c).mul(m.a.z));
        b = vec(n.a).mul(m.b.x).add(vec(n.b).mul(m.b.y)).add(vec(n.c).mul(m.b.z));
        c = vec(n.a).mul(m.c.x).add(vec(n.b).mul(m.c.y)).add(vec(n.c).mul(m.c.z));
    }
    void mul(const matrix3x3 &n) { mul(*this, n); }

    void multranspose(const matrix3x3 &m, const matrix3x3 &n)
    {
        a = vec(m.a.dot(n.a), m.a.dot(n.b), m.a.dot(n.c));
        b = vec(m.b.dot(n.a), m.b.dot(n.b), m.b.dot(n.c));
        c = vec(m.c.dot(n.a), m.c.dot(n.b), m.c.dot(n.c));
    }

    void transposemul(const matrix3x3 &m, const matrix3x3 &n)
    {
        a = vec(n.a).mul(m.a.x).add(vec(n.b).mul(m.b.x)).add(vec(n.c).mul(m.c.x));
        b = vec(n.a).mul(m.a.y).add(vec(n.b).mul(m.b.y)).add(vec(n.c).mul(m.c.y));
        c = vec(n.a).mul(m.a.z).add(vec(n.b).mul(m.b.z)).add(vec(n.c).mul(m.c.z));
    }

    void transpose(const matrix3x3 &o)
    {
        a = vec(o.a.x, o.b.x, o.c.x);
        b = vec(o.a.y, o.b.y, o.c.y);
        c = vec(o.a.z, o.b.z, o.c.z);
    }

    void rotate(float angle, const vec &axis)
    {
        rotate(cosf(angle), sinf(angle), axis);
    }

    void rotate(float ck, float sk, const vec &axis)
    {
        a = vec(axis.x*axis.x*(1-ck)+ck, axis.x*axis.y*(1-ck)-axis.z*sk, axis.x*axis.z*(1-ck)+axis.y*sk);
        b = vec(axis.y*axis.x*(1-ck)+axis.z*sk, axis.y*axis.y*(1-ck)+ck, axis.y*axis.z*(1-ck)-axis.x*sk);
        c = vec(axis.x*axis.z*(1-ck)-axis.y*sk, axis.y*axis.z*(1-ck)+axis.x*sk, axis.z*axis.z*(1-ck)+ck);
    }

    bool calcangleaxis(float &angle, vec &axis, float threshold = 1e-9f)
    {
        angle = acosf(clamp(0.5f*(a.x + b.y + c.z - 1), -1.0f, 1.0f));

		if(angle <= 0) axis = vec(0, 0, 1);
        else if(angle < M_PI) 
        {
            axis = vec(c.y - b.z, a.z - c.x, b.x - a.y);
            float r = axis.magnitude();
            if(r <= threshold) return false;
            axis.mul(1/r);
        }
        else if(a.x >= b.y && a.x >= c.z)
        {
            float r = sqrtf(max(1 + a.x - b.y - c.z, 0.0f));
            if(r <= threshold) return false;
            axis.x = 0.5f*r;
            axis.y = a.y/r;
            axis.z = a.z/r;
        }
        else if(b.y >= c.z)
        {
            float r = sqrtf(max(1 + b.y - a.x - c.z, 0.0f));
            if(r <= threshold) return false;
            axis.y = 0.5f*r;
            axis.x = a.y/r;
            axis.z = b.z/r;
        }
        else
        {
            float r = sqrtf(max(1 + b.y - a.x - c.z, 0.0f));
            if(r <= threshold) return false;
            axis.z = 0.5f*r;
            axis.x = a.z/r;
            axis.y = b.z/r;
        }
        return true;
    }

    vec transform(const vec &o) const { return vec(a.dot(o), b.dot(o), c.dot(o)); }
    vec transposedtransform(const vec &o) const
    {
        return vec(a.x*o.x + b.x*o.y + c.x*o.z,
                   a.y*o.x + b.y*o.y + c.y*o.z,
                   a.z*o.x + b.z*o.y + c.z*o.z);
    }
};

struct matrix2x3
{
    vec a, b;

    matrix2x3() {}
    matrix2x3(const vec &a, const vec &b) : a(a), b(b) {}

    vec2 transform(const vec &o) const { return vec2(a.dot(o), b.dot(o)); }
    vec transposedtransform(const vec2 &o) const
    {
        return vec(a.x*o.x + b.x*o.y,
                   a.y*o.x + b.y*o.y,
                   a.z*o.x + b.z*o.y);
    }
};

struct matrix3x4
{
    vec4 a, b, c;
    
    matrix3x4() {}
    matrix3x4(const vec4 &x, const vec4 &y, const vec4 &z) : a(x), b(y), c(z) {}
    matrix3x4(const matrix3x3 &rot, const vec &trans)
     : a(rot.a, trans.x), b(rot.b, trans.y), c(rot.c, trans.z)
    {}
    matrix3x4(const dualquat &d)
    {
        vec4 r = vec4(d.real).mul(1/d.real.squaredlen()), rr = vec4(r).mul(d.real);
        r.mul(2);
        float xy = r.x*d.real.y, xz = r.x*d.real.z, yz = r.y*d.real.z,
              wx = r.w*d.real.x, wy = r.w*d.real.y, wz = r.w*d.real.z;
        a = vec4(rr.w + rr.x - rr.y - rr.z, xy - wz, xz + wy,
            -(d.dual.w*r.x - d.dual.x*r.w + d.dual.y*r.z - d.dual.z*r.y));
        b = vec4(xy + wz, rr.w + rr.y - rr.x - rr.z, yz - wx,
            -(d.dual.w*r.y - d.dual.x*r.z - d.dual.y*r.w + d.dual.z*r.x));
        c = vec4(xz - wy, yz + wx, rr.w + rr.z - rr.x - rr.y,
            -(d.dual.w*r.z + d.dual.x*r.y - d.dual.y*r.x - d.dual.z*r.w));
    }

    void mul(float k)
    {
        a.mul(k);
        b.mul(k);
        c.mul(k);
    }

    void scale(float k)
    {
        a.mul(k);
        b.mul(k);
        c.mul(k);
    }
    
    void translate(const vec &p)
    {
        a.w += p.x;
        b.w += p.y;
        c.w += p.z;
    }

    void transformedtranslate(const vec &p, float scale = 1)
    {
        a.w += a.dot3(p)*scale;
        b.w += b.dot3(p)*scale;
        c.w += c.dot3(p)*scale;
    }

    void accumulate(const matrix3x4 &m, float k)
    {
        a.add(vec4(m.a).mul(k));
        b.add(vec4(m.b).mul(k));
        c.add(vec4(m.c).mul(k));
    }

    void normalize()
    {
        a.mul3(1/a.magnitude3());
        b.mul3(1/b.magnitude3());
        c.mul3(1/c.magnitude3());
    }

    void lerp(const matrix3x4 &to, float t)
    {
        a.lerp(to.a, t);
        b.lerp(to.b, t);
        c.lerp(to.c, t);
    }
    void lerp(const matrix3x4 &from, const matrix3x4 &to, float t)
    {
        a.lerp(from.a, to.a, t);
        b.lerp(from.b, to.b, t);
        c.lerp(from.c, to.c, t);
    }

    void identity()
    {
        a = vec4(1, 0, 0, 0);
        b = vec4(0, 1, 0, 0);
        c = vec4(0, 0, 1, 0);
    }

    void mul(const matrix3x4 &m, const matrix3x4 &n)
    {
        a = vec4(n.a).mul(m.a.x).add(vec4(n.b).mul(m.a.y)).add(vec4(n.c).mul(m.a.z)).addw(m.a.w);
        b = vec4(n.a).mul(m.b.x).add(vec4(n.b).mul(m.b.y)).add(vec4(n.c).mul(m.b.z)).addw(m.b.w);
        c = vec4(n.a).mul(m.c.x).add(vec4(n.b).mul(m.c.y)).add(vec4(n.c).mul(m.c.z)).addw(m.c.w);
    }
    void mul(const matrix3x4 &n) { mul(*this, n); }

    void mul(const matrix3x3 &rot, const vec &trans, const matrix3x4 &o)
    {
        a = vec4(o.a).mul(rot.a.x).add(vec4(o.b).mul(rot.a.y)).add(vec4(o.c).mul(rot.a.z)).addw(trans.x);
        b = vec4(o.a).mul(rot.b.x).add(vec4(o.b).mul(rot.b.y)).add(vec4(o.c).mul(rot.b.z)).addw(trans.y);
        c = vec4(o.a).mul(rot.c.x).add(vec4(o.b).mul(rot.c.y)).add(vec4(o.c).mul(rot.c.z)).addw(trans.z);
    }

    void mulorient(const matrix3x3 &m)
    {
        vec ra = vec(a).mul(m.a.x).add(vec(b).mul(m.a.y)).add(vec(c).mul(m.a.z)),
            rb = vec(a).mul(m.b.x).add(vec(b).mul(m.b.y)).add(vec(c).mul(m.b.z)),
            rc = vec(a).mul(m.c.x).add(vec(b).mul(m.c.y)).add(vec(c).mul(m.c.z));
        a.setxyz(ra);
        b.setxyz(rb);
        c.setxyz(rc);
    }

    void mulorient(const matrix3x3 &m, const dualquat &base)
    {
        vec trans = transformnormal(base.gettranslation());
        translate(trans.sub(m.transform(trans)));
        mulorient(m);
    }

    void transpose(const matrix3x4 &o)
    {
        a = vec4(o.a.x, o.b.x, o.c.x, -(o.a.x*o.a.w + o.b.x*o.b.w + o.c.x*o.c.w));
        b = vec4(o.a.y, o.b.y, o.c.y, -(o.a.y*o.a.w + o.b.y*o.b.w + o.c.y*o.c.w));
        c = vec4(o.a.z, o.b.z, o.c.z, -(o.a.z*o.a.w + o.b.z*o.b.w + o.c.z*o.c.w));
    }

    void transposemul(const matrix3x3 &rot, const vec &trans, const matrix3x4 &o)
    {
        a = vec4(o.a).mul(rot.a.x).add(vec4(o.b).mul(rot.b.x)).add(vec4(o.c).mul(rot.c.x)).addw(trans.x);
        b = vec4(o.a).mul(rot.a.y).add(vec4(o.b).mul(rot.b.y)).add(vec4(o.c).mul(rot.c.y)).addw(trans.y);
        c = vec4(o.a).mul(rot.a.z).add(vec4(o.b).mul(rot.b.z)).add(vec4(o.c).mul(rot.c.z)).addw(trans.z);
    } 

    void transposemul(const matrix3x4 &m, const matrix3x4 &n)
    {
        float tx = m.a.x*m.a.w + m.b.x*m.b.w + m.c.x*m.c.w,
              ty = m.a.y*m.a.w + m.b.y*m.b.w + m.c.y*m.c.w,
              tz = m.a.z*m.a.w + m.b.z*m.b.w + m.c.z*m.c.w;
        a = vec4(n.a).mul(m.a.x).add(vec4(n.b).mul(m.b.x)).add(vec4(n.c).mul(m.c.x)).subw(tx);
        b = vec4(n.a).mul(m.a.y).add(vec4(n.b).mul(m.b.y)).add(vec4(n.c).mul(m.c.y)).subw(ty);
        c = vec4(n.a).mul(m.a.z).add(vec4(n.b).mul(m.b.z)).add(vec4(n.c).mul(m.c.z)).subw(tz);
    }

    void rotate(float angle, const vec &d)
    {
        rotate(cosf(angle), sinf(angle), d);
    }

    void rotate(float ck, float sk, const vec &d)
    {
        a = vec4(d.x*d.x*(1-ck)+ck, d.x*d.y*(1-ck)-d.z*sk, d.x*d.z*(1-ck)+d.y*sk, 0);
        b = vec4(d.y*d.x*(1-ck)+d.z*sk, d.y*d.y*(1-ck)+ck, d.y*d.z*(1-ck)-d.x*sk, 0);
        c = vec4(d.x*d.z*(1-ck)-d.y*sk, d.y*d.z*(1-ck)+d.x*sk, d.z*d.z*(1-ck)+ck, 0);
    }

    #define ROTVEC(V, m, n) \
    { \
        float m = V.m, n = V.n; \
        V.m = m*ck + n*sk; \
        V.n = n*ck - m*sk; \
    }

    void rotate_around_x(float angle)
    {
        float ck = cosf(angle), sk = sinf(angle);
        ROTVEC(a, y, z);
        ROTVEC(b, y, z);
        ROTVEC(c, y, z);
    }

    void rotate_around_y(float angle)
    {
        float ck = cosf(angle), sk = sinf(angle);
        ROTVEC(a, z, x);
        ROTVEC(b, z, x);
        ROTVEC(c, z, x);
    }

    void rotate_around_z(float angle)
    {
        float ck = cosf(angle), sk = sinf(angle);
        ROTVEC(a, x, y);
        ROTVEC(b, x, y);
        ROTVEC(c, x, y);
    }

    #undef ROTVEC

    vec transform(const vec &o) const { return vec(a.dot(o), b.dot(o), c.dot(o)); }
    vec transposedtransform(const vec &o) const
    {
        vec p = o;
        p.x -= a.w;
        p.y -= b.w;
        p.z -= c.w;
        return vec(a.x*p.x + b.x*p.y + c.x*p.z,
                   a.y*p.x + b.y*p.y + c.y*p.z,
                   a.z*p.x + b.z*p.y + c.z*p.z);
    }
    vec transformnormal(const vec &o) const { return vec(a.dot3(o), b.dot3(o), c.dot3(o)); }
    vec transposedtransformnormal(const vec &o) const
    {
        return vec(a.x*o.x + b.x*o.y + c.x*o.z,
                   a.y*o.x + b.y*o.y + c.y*o.z,
                   a.z*o.x + b.z*o.y + c.z*o.z);
    }

    float getscale() const { return a.magnitude3(); }
    vec gettranslation() const { return vec(a.w, b.w, c.w); }
};

inline dualquat::dualquat(const matrix3x4 &m) : real(m)
{
    dual.x =  0.5f*( m.a.w*real.w + m.b.w*real.z - m.c.w*real.y);
    dual.y =  0.5f*(-m.a.w*real.z + m.b.w*real.w + m.c.w*real.x);
    dual.z =  0.5f*( m.a.w*real.y - m.b.w*real.x + m.c.w*real.w);
    dual.w = -0.5f*( m.a.w*real.x + m.b.w*real.y + m.c.w*real.z);
}

struct plane : vec
{
    float offset;

    float dist(const vec &p) const { return dot(p)+offset; }
    bool operator==(const plane &p) const { return x==p.x && y==p.y && z==p.z && offset==p.offset; }
    bool operator!=(const plane &p) const { return x!=p.x || y!=p.y || z!=p.z || offset!=p.offset; }

    plane() {}
    plane(const vec &c, float off) : vec(c), offset(off) {} 
    plane(const vec4 &p) : vec(p), offset(p.w) {}
    plane(int d, float off)
    {
        x = y = z = 0.0f;
        v[d] = 1.0f;
        offset = -off;
    }
    plane(float a, float b, float c, float d) : vec(a, b, c), offset(d) {}

    void toplane(const vec &n, const vec &p)
    {
        x = n.x; y = n.y; z = n.z;
        offset = -dot(p);
    }

    bool toplane(const vec &a, const vec &b, const vec &c)
    {
        cross(vec(b).sub(a), vec(c).sub(a));
        float mag = magnitude();
        if(!mag) return false;
        div(mag);
        offset = -dot(a);
        return true;
    }

    bool rayintersect(const vec &o, const vec &ray, float &dist)
    {
        float cosalpha = dot(ray);
        if(cosalpha==0) return false;
        float deltac = offset+dot(o);
        dist -= deltac/cosalpha;
        return true;
    }

    plane &reflectz(float rz)
    {
        offset += 2*rz*z;
        z = -z;
        return *this; 
    }

    plane &invert()
    {
        neg();
        offset = -offset;
        return *this;
    }

    plane &scale(float k)
    {
        mul(k);
        return *this;
    }

    plane &translate(const vec &p)
    {
        offset += dot(p);
        return *this;
    }

    plane &normalize()
    {
        float mag = magnitude();
        div(mag);
        offset /= mag; 
        return *this;
    }

    float zintersect(const vec &p) const { return -(x*p.x+y*p.y+offset)/z; }
    float zdelta(const vec &p) const { return -(x*p.x+y*p.y)/z; }
    float zdist(const vec &p) const { return p.z-zintersect(p); }
};

struct triangle
{
    vec a, b, c;

    triangle(const vec &a, const vec &b, const vec &c) : a(a), b(b), c(c) {}
    triangle() {}

    triangle &add(const vec &o) { a.add(o); b.add(o); c.add(o); return *this; }
    triangle &sub(const vec &o) { a.sub(o); b.sub(o); c.sub(o); return *this; }

    bool operator==(const triangle &t) const { return a == t.a && b == t.b && c == t.c; }
};

/**

Sauerbraten uses 3 different linear coordinate systems
which are oriented around each of the axis dimensions.

So any point within the game can be defined by four coordinates: (d, x, y, z)

d is the reference axis dimension
x is the coordinate of the ROW dimension
y is the coordinate of the COL dimension
z is the coordinate of the reference dimension (DEPTH)

typically, if d is not used, then it is implicitly the Z dimension.
ie: d=z => x=x, y=y, z=z

**/

// DIM: X=0 Y=1 Z=2.
const int R[3]  = {1, 2, 0}; // row
const int C[3]  = {2, 0, 1}; // col
const int D[3]  = {0, 1, 2}; // depth

struct ivec
{
    union
    {
        struct { int x, y, z; };
        struct { int r, g, b; };
        int v[3];
    };

    ivec() {}
    ivec(const vec &v) : x(int(v.x)), y(int(v.y)), z(int(v.z)) {}
    explicit ivec(int i)
    {
        x = ((i&1)>>0);
        y = ((i&2)>>1);
        z = ((i&4)>>2);
    }
    ivec(int a, int b, int c) : x(a), y(b), z(c) {}
    ivec(int d, int row, int col, int depth)
    {
        v[R[d]] = row;
        v[C[d]] = col;
        v[D[d]] = depth;
    }
    ivec(int i, int cx, int cy, int cz, int size)
    {
        x = cx+((i&1)>>0)*size;
        y = cy+((i&2)>>1)*size;
        z = cz+((i&4)>>2)*size;
    }
    vec tovec() const { return vec(x, y, z); }
    int toint() const { return (x>0?1:0) + (y>0?2:0) + (z>0?4:0); }

    int &operator[](int i)       { return v[i]; }
    int  operator[](int i) const { return v[i]; }

    //int idx(int i) { return v[i]; }
    bool operator==(const ivec &v) const { return x==v.x && y==v.y && z==v.z; }
    bool operator!=(const ivec &v) const { return x!=v.x || y!=v.y || z!=v.z; }
    bool iszero() const { return x==0 && y==0 && z==0; }
    ivec &shl(int n) { x<<= n; y<<= n; z<<= n; return *this; }
    ivec &shr(int n) { x>>= n; y>>= n; z>>= n; return *this; }
    ivec &mul(int n) { x *= n; y *= n; z *= n; return *this; }
    ivec &div(int n) { x /= n; y /= n; z /= n; return *this; }
    ivec &add(int n) { x += n; y += n; z += n; return *this; }
    ivec &sub(int n) { x -= n; y -= n; z -= n; return *this; }
    ivec &mul(const ivec &v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    ivec &div(const ivec &v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
    ivec &add(const ivec &v) { x += v.x; y += v.y; z += v.z; return *this; }
    ivec &sub(const ivec &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    ivec &mask(int n) { x &= n; y &= n; z &= n; return *this; }
    ivec &neg() { return mul(-1); }
    ivec &min(const ivec &o) { x = ::min(x, o.x); y = ::min(y, o.y); z = ::min(z, o.z); return *this; }
    ivec &max(const ivec &o) { x = ::max(x, o.x); y = ::max(y, o.y); z = ::max(z, o.z); return *this; }
    ivec &min(int n) { x = ::min(x, n); y = ::min(y, n); z = ::min(z, n); return *this; }
    ivec &max(int n) { x = ::max(x, n); y = ::max(y, n); z = ::max(z, n); return *this; }
    ivec &abs() { x = ::abs(x); y = ::abs(y); z = ::abs(z); return *this; }
    ivec &cross(const ivec &a, const ivec &b) { x = a.y*b.z-a.z*b.y; y = a.z*b.x-a.x*b.z; z = a.x*b.y-a.y*b.x; return *this; }
    int dot(const ivec &o) const { return x*o.x + y*o.y + z*o.z; }
    float dist(const plane &p) const { return x*p.x + y*p.y + z*p.z + p.offset; }
};

static inline bool htcmp(const ivec &x, const ivec &y)
{
    return x == y;
}  

static inline uint hthash(const ivec &k)
{
    return k.x^k.y^k.z;
}  

struct bvec
{
    union
    {
        struct { uchar x, y, z; };
        struct { uchar r, g, b; };
        uchar v[3];
    };

    bvec() {}
    bvec(uchar x, uchar y, uchar z) : x(x), y(y), z(z) {}
    bvec(const vec &v) : x((uchar)((v.x+1)*255/2)), y((uchar)((v.y+1)*255/2)), z((uchar)((v.z+1)*255/2)) {}

    uchar &operator[](int i)       { return v[i]; }
    uchar  operator[](int i) const { return v[i]; }

    bool operator==(const bvec &v) const { return x==v.x && y==v.y && z==v.z; }
    bool operator!=(const bvec &v) const { return x!=v.x || y!=v.y || z!=v.z; }

    bool iszero() const { return x==0 && y==0 && z==0; }

    vec tovec() const { return vec(x*(2.0f/255.0f)-1.0f, y*(2.0f/255.0f)-1.0f, z*(2.0f/255.0f)-1.0f); }

    bvec &normalize()
    {
        vec n(x-127.5f, y-127.5f, z-127.5f);
        float mag = 127.5f/n.magnitude();
        x = uchar(n.x*mag+127.5f);
        y = uchar(n.y*mag+127.5f);
        z = uchar(n.z*mag+127.5f);
        return *this;
    }

    void lerp(const bvec &a, const bvec &b, float t) { x = uchar(a.x + (b.x-a.x)*t); y = uchar(a.y + (b.y-a.y)*t); z = uchar(a.z + (b.z-a.z)*t); }

    void flip() { x -= 128; y -= 128; z -= 128; }
};

struct glmatrixf
{
    float v[16];

    glmatrixf() {}
    glmatrixf(const float *m) { memcpy(v, m, sizeof(v)); }
    glmatrixf(const vec4 &a, const vec4 &b, const vec4 &c, const vec4 &d = vec4(0, 0, 0, 1))
    {
        v[0]  = a.x; v[1] =  b.x; v[2]  = c.x; v[3]  = d.x;
        v[4]  = a.y; v[5] =  b.y; v[6]  = c.y; v[7]  = d.y;
        v[8]  = a.z; v[9] =  b.z; v[10] = c.z; v[11] = d.z;
        v[12] = a.w; v[13] = b.w; v[14] = c.w; v[15] = d.w;
    }
    glmatrixf(const matrix3x4 &m)
    {
        v[0]  = m.a.x; v[1] =  m.b.x; v[2]  = m.c.x; v[3]  = 0.0f;
        v[4]  = m.a.y; v[5] =  m.b.y; v[6]  = m.c.y; v[7]  = 0.0f;
        v[8]  = m.a.z; v[9] =  m.b.z; v[10] = m.c.z; v[11] = 0.0f;
        v[12] = m.a.w; v[13] = m.b.w; v[14] = m.c.w; v[15] = 1.0f;
    }

    float operator[](int i) const { return v[i]; }
    float &operator[](int i) { return v[i]; }

    #define ROTVEC(A, B) \
    { \
        float a = A, b = B; \
        A = a*c + b*s; \
        B = b*c - a*s; \
    }

    void rotate_around_x(float angle)
    {
        float c = cosf(angle), s = sinf(angle);
        ROTVEC(v[4], v[8]);
        ROTVEC(v[5], v[9]);
        ROTVEC(v[6], v[10]);
    }

    void rotate_around_y(float angle)
    {
        float c = cosf(angle), s = sinf(angle);
        ROTVEC(v[8], v[0]);
        ROTVEC(v[9], v[1]);
        ROTVEC(v[10], v[2]);
    }

    void rotate_around_z(float angle)
    {
        float c = cosf(angle), s = sinf(angle);
        ROTVEC(v[0], v[4]);
        ROTVEC(v[1], v[5]);
        ROTVEC(v[2], v[6]);
    }

    #undef ROTVEC

    void rotate(float c, float s, const vec &d)
    {
        vec c1(d.x*d.x*(1-c)+c, d.y*d.x*(1-c)+d.z*s, d.x*d.z*(1-c)-d.y*s),
            c2(d.x*d.y*(1-c)-d.z*s, d.y*d.y*(1-c)+c, d.y*d.z*(1-c)+d.x*s),
            c3(d.x*d.z*(1-c)+d.y*s, d.y*d.z*(1-c)-d.x*s, d.z*d.z*(1-c)+c);

        vec r1(v[0], v[4], v[8]);
        v[0] = r1.dot(c1);
        v[4] = r1.dot(c2);
        v[8] = r1.dot(c3);

        vec r2(v[1], v[5], v[9]);
        v[1] = r2.dot(c1);
        v[5] = r2.dot(c2);
        v[9] = r2.dot(c3);

        vec r3(v[2], v[6], v[10]);
        v[2] = r3.dot(c1);
        v[6] = r3.dot(c2);
        v[10] = r3.dot(c3);
    }

    void rotate(float angle, const vec &d)
    {
        float c = cosf(angle), s = sinf(angle);
        rotate(c, s, d);
    }

    #define MULMAT(row, col) \
       v[col + row] = x[row]*y[col] + x[row + 4]*y[col + 1] + x[row + 8]*y[col + 2] + x[row + 12]*y[col + 3];

    template<class XT, class YT>
    void mul(const XT x[16], const YT y[16])
    {
        MULMAT(0, 0); MULMAT(1, 0); MULMAT(2, 0); MULMAT(3, 0);
        MULMAT(0, 4); MULMAT(1, 4); MULMAT(2, 4); MULMAT(3, 4);
        MULMAT(0, 8); MULMAT(1, 8); MULMAT(2, 8); MULMAT(3, 8);
        MULMAT(0, 12); MULMAT(1, 12); MULMAT(2, 12); MULMAT(3, 12);
    }

    #undef MULMAT

    void mul(const glmatrixf &x, const glmatrixf &y)
    {
        mul(x.v, y.v);
    }
    void mul(const glmatrixf &y) { mul(glmatrixf(*this), y); }

    void identity()
    {
        static const float m[16] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        memcpy(v, m, sizeof(v));
    }

    void translate(float x, float y, float z)
    {
        v[12] += x;
        v[13] += y;
        v[14] += z;
    }

    void translate(const vec &o)
    {
        translate(o.x, o.y, o.z);
    }

    void transformedtranslate(float x, float y, float z, float scale = 1)
    {
        v[12] += (x*v[0] + y*v[4] + z*v[8])*scale;
        v[13] += (x*v[1] + y*v[5] + z*v[9])*scale;
        v[14] += (x*v[2] + y*v[6] + z*v[10])*scale;
    }

    void transformedtranslate(const vec &p, float scale = 1)
    {
        transformedtranslate(p.x, p.y, p.z, scale);
    }

    void scale(float x, float y, float z)
    {
        v[0] *= x; v[1] *= x; v[2] *= x; v[3] *= x;
        v[4] *= y; v[5] *= y; v[6] *= y; v[7] *= y;
        v[8] *= z; v[9] *= z; v[10] *= z; v[11] *= z;
    }

    void reflectz(float z)
    {
        v[12] += 2*z*v[8];
        v[13] += 2*z*v[9];
        v[14] += 2*z*v[10];
        v[15] += 2*z*v[11];
        v[8] = -v[8]; v[9] = -v[9]; v[10] = -v[10]; v[11] = -v[11];
    }

    void projective(float zscale = 0.5f, float zoffset = 0.5f)
    {
        loopi(2) loopj(4) v[i + j*4] = 0.5f*(v[i + j*4] + v[3 + j*4]); 
        loopj(4) v[2 + j*4] = zscale*v[2 + j*4] + zoffset*v[3 + j*4];
    }

    void transpose()
    {
        swap(v[1], v[4]); swap(v[2], v[8]); swap(v[3], v[12]);
        swap(v[6], v[9]); swap(v[7], v[13]);
        swap(v[11], v[14]);
    }

    void frustum(float left, float right, float bottom, float top, float znear, float zfar)
    {
        float width = right - left, height = top - bottom, zrange = znear - zfar;
        v[0] = 2*znear/width; v[4] = 0;              v[8] = (right + left)/width;   v[12] = 0;
        v[1] = 0;             v[5] = 2*znear/height; v[9] = (top + bottom)/height;  v[13] = 0;
        v[2] = 0;             v[6] = 0;              v[10] = (zfar + znear)/zrange; v[14] = 2*znear*zfar/zrange;
        v[3] = 0;             v[7] = 0;              v[11] = -1;                    v[15] = 0;
    }

    void perspective(float fovy, float aspect, float znear, float zfar)
    {
        float ydist = znear * tan(fovy/2*RAD), xdist = ydist * aspect;
        frustum(-xdist, xdist, -ydist, ydist, znear, zfar);
    }

    void clip(const plane &p, const glmatrixf &m)
    {
        float x = ((p.x<0 ? -1 : (p.x>0 ? 1 : 0)) + m.v[8]) / m.v[0],
              y = ((p.y<0 ? -1 : (p.y>0 ? 1 : 0)) + m.v[9]) / m.v[5],
              w = (1 + m.v[10]) / m.v[14],
            scale = 2 / (x*p.x + y*p.y - p.z + w*p.offset);
        memcpy(v, m.v, sizeof(v));
        v[2] = p.x*scale;
        v[6] = p.y*scale;
        v[10] = p.z*scale + 1.0f;
        v[14] = p.offset*scale;
    }
            
    float transformx(const vec &p) const
    {
        return p.x*v[0] + p.y*v[4] + p.z*v[8] + v[12];
    }

    float transformy(const vec &p) const
    {
        return p.x*v[1] + p.y*v[5] + p.z*v[9] + v[13];
    }

    float transformz(const vec &p) const
    {
        return p.x*v[2] + p.y*v[6] + p.z*v[10] + v[14];
    }

    float transformw(const vec &p) const
    {
        return p.x*v[3] + p.y*v[7] + p.z*v[11] + v[15];
    }

    float transformx(const vec4 &p) const
    {
        return p.x*v[0] + p.y*v[4] + p.z*v[8] + p.w*v[12];
    }

    float transformy(const vec4 &p) const
    {
        return p.x*v[1] + p.y*v[5] + p.z*v[9] + p.w*v[13];
    }

    float transformz(const vec4 &p) const
    {
        return p.x*v[2] + p.y*v[6] + p.z*v[10] + p.w*v[14];
    }

    float transformw(const vec4 &p) const
    {
        return p.x*v[3] + p.y*v[7] + p.z*v[11] + p.w*v[15];
    }

    template<class T> void transform(const T &in, vec &out) const
    {
        out.x = transformx(in);
        out.y = transformy(in);
        out.z = transformz(in);
    }

    template<class T> void transform(const T &in, vec4 &out) const
    {
        out.x = transformx(in);
        out.y = transformy(in);
        out.z = transformz(in);
        out.w = transformw(in);
    }

    template<class T> vec perspectivetransform(const T &in) const
    {
        return vec(transformx(in), transformy(in), transformz(in)).div(transformw(in));
    }

    void transformnormal(const vec &in, vec &out) const
    {
        out.x = in.x*v[0] + in.y*v[4] + in.z*v[8];
        out.y = in.x*v[1] + in.y*v[5] + in.z*v[9];
        out.z = in.x*v[2] + in.y*v[6] + in.z*v[10];
    }

    void transposedtransform(const vec &in, vec &out) const
    {
        vec p(in.x - v[12], in.y - v[13], in.z - v[14]);
        out.x = p.x*v[0] + p.y*v[1] + p.z*v[2];
        out.y = p.x*v[4] + p.y*v[5] + p.z*v[6];
        out.z = p.x*v[8] + p.y*v[9] + p.z*v[10];
    }

    void transposedtransformnormal(const vec &in, vec &out) const
    {
        out.x = in.x*v[0] + in.y*v[1] + in.z*v[2];
        out.y = in.x*v[4] + in.y*v[5] + in.z*v[6];
        out.z = in.x*v[8] + in.y*v[9] + in.z*v[10];
    }

    void transposedtransform(const plane &in, plane &out) const
    {
        out.x      = in.x*v[0] + in.y*v[1] + in.z*v[2] + in.offset*v[3];
        out.y      = in.x*v[4] + in.y*v[5] + in.z*v[6] + in.offset*v[7];
        out.z      = in.x*v[8] + in.y*v[9] + in.z*v[10] + in.offset*v[11];
        out.offset = in.x*v[12] + in.y*v[13] + in.z*v[14] + in.offset*v[15];
    }

    float getscale() const
    {
        return sqrtf(v[0]*v[0] + v[4]*v[4] + v[8]*v[8]);
    }

    vec gettranslation() const
    {
        return vec(v[12], v[13], v[14]);
    }

    vec4 getrow(int i) const { return vec4(v[i], v[i+4], v[i+8], v[i+12]); }
    vec4 getcolumn(int i) const { i *= 4; return vec4(v[i], v[i+1], v[i+2], v[i+3]); }

    float determinant() const;
    void adjoint(const glmatrixf &m);
    bool invert(const glmatrixf &m, float mindet = 1.0e-10f);
};

extern bool raysphereintersect(const vec &center, float radius, const vec &o, const vec &ray, float &dist);
extern bool rayrectintersect(const vec &b, const vec &s, const vec &o, const vec &ray, float &dist, int &orient);
extern bool linecylinderintersect(const vec &from, const vec &to, const vec &start, const vec &end, float radius, float &dist);

extern const vec2 sincos360[];

