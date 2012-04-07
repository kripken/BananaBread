#include "cube.h"

///////////////////////// cryptography /////////////////////////////////

/* Based off the reference implementation of Tiger, a cryptographically
 * secure 192 bit hash function by Ross Anderson and Eli Biham. More info at:
 * http://www.cs.technion.ac.il/~biham/Reports/Tiger/
 */

#define TIGER_PASSES 3

namespace tiger
{
    typedef unsigned long long int chunk;

    union hashval
    {
        uchar bytes[3*8];
        chunk chunks[3];
    };

    chunk sboxes[4*256];

    void compress(const chunk *str, chunk state[3])
    {
        chunk a, b, c;
        chunk aa, bb, cc;
        chunk x0, x1, x2, x3, x4, x5, x6, x7;

        a = state[0];
        b = state[1];
        c = state[2];

        x0=str[0]; x1=str[1]; x2=str[2]; x3=str[3];
        x4=str[4]; x5=str[5]; x6=str[6]; x7=str[7];

        aa = a;
        bb = b;
        cc = c;

        loop(pass_no, TIGER_PASSES)
        {
            if(pass_no)
            {
                x0 -= x7 ^ 0xA5A5A5A5A5A5A5A5ULL; x1 ^= x0; x2 += x1; x3 -= x2 ^ ((~x1)<<19);
                x4 ^= x3; x5 += x4; x6 -= x5 ^ ((~x4)>>23); x7 ^= x6;
                x0 += x7; x1 -= x0 ^ ((~x7)<<19); x2 ^= x1; x3 += x2;
                x4 -= x3 ^ ((~x2)>>23); x5 ^= x4; x6 += x5; x7 -= x6 ^ 0x0123456789ABCDEFULL;
            }

#define sb1 (sboxes)
#define sb2 (sboxes+256)
#define sb3 (sboxes+256*2)
#define sb4 (sboxes+256*3)

#define round(a, b, c, x) \
      c ^= x; \
      a -= sb1[((c)>>(0*8))&0xFF] ^ sb2[((c)>>(2*8))&0xFF] ^ \
       sb3[((c)>>(4*8))&0xFF] ^ sb4[((c)>>(6*8))&0xFF] ; \
      b += sb4[((c)>>(1*8))&0xFF] ^ sb3[((c)>>(3*8))&0xFF] ^ \
       sb2[((c)>>(5*8))&0xFF] ^ sb1[((c)>>(7*8))&0xFF] ; \
      b *= mul;

            uint mul = !pass_no ? 5 : (pass_no==1 ? 7 : 9);
            round(a, b, c, x0) round(b, c, a, x1) round(c, a, b, x2) round(a, b, c, x3)
            round(b, c, a, x4) round(c, a, b, x5) round(a, b, c, x6) round(b, c, a, x7)

            chunk tmp = a; a = c; c = b; b = tmp;
        }

        a ^= aa;
        b -= bb;
        c += cc;

        state[0] = a;
        state[1] = b;
        state[2] = c;
    }

    void gensboxes()
    {
        const char *str = "Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham";
        chunk state[3] = { 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL, 0xF096A5B4C3B2E187ULL };
        uchar temp[64];

        if(!*(const uchar *)&islittleendian) loopj(64) temp[j^7] = str[j];
        else loopj(64) temp[j] = str[j];
        loopi(1024) loop(col, 8) ((uchar *)&sboxes[i])[col] = i&0xFF;

        int abc = 2;
        loop(pass, 5) loopi(256) for(int sb = 0; sb < 1024; sb += 256)
        {
            abc++;
            if(abc >= 3) { abc = 0; compress((chunk *)temp, state); }
            loop(col, 8)
            {
                uchar val = ((uchar *)&sboxes[sb+i])[col];
                ((uchar *)&sboxes[sb+i])[col] = ((uchar *)&sboxes[sb + ((uchar *)&state[abc])[col]])[col];
                ((uchar *)&sboxes[sb + ((uchar *)&state[abc])[col]])[col] = val;
            }
        }
    }

    void hash(const uchar *str, int length, hashval &val)
    {
        static bool init = false;
        if(!init) { gensboxes(); init = true; }

        uchar temp[64];

        val.chunks[0] = 0x0123456789ABCDEFULL;
        val.chunks[1] = 0xFEDCBA9876543210ULL;
        val.chunks[2] = 0xF096A5B4C3B2E187ULL;

        int i = length;
        for(; i >= 64; i -= 64, str += 64)
        {
            if(!*(const uchar *)&islittleendian)
            {
                loopj(64) temp[j^7] = str[j];
                compress((chunk *)temp, val.chunks);
            }
            else compress((chunk *)str, val.chunks);
        }

        int j;
        if(!*(const uchar *)&islittleendian)
        {
            for(j = 0; j < i; j++) temp[j^7] = str[j];
            temp[j^7] = 0x01;
            while(++j&7) temp[j^7] = 0;
        }
        else
        {
            for(j = 0; j < i; j++) temp[j] = str[j];
            temp[j] = 0x01;
            while(++j&7) temp[j] = 0;
        }

        if(j > 56)
        {
            while(j < 64) temp[j++] = 0;
            compress((chunk *)temp, val.chunks);
            j = 0;
        }
        while(j < 56) temp[j++] = 0;
        *(chunk *)(temp+56) = (chunk)length<<3;
        compress((chunk *)temp, val.chunks);
        if(!*(const uchar *)&islittleendian)
        {
            loopk(3) 
            {
                uchar *c = &val.bytes[k*sizeof(chunk)];
                loopl(sizeof(chunk)/2) swap(c[l], c[sizeof(chunk)-1-l]);
            }
        }
    }
}

/* Elliptic curve cryptography based on NIST DSS prime curves. */

#define BI_DIGIT_BITS 16
#define BI_DIGIT_MASK ((1<<BI_DIGIT_BITS)-1)

template<int BI_DIGITS> struct bigint
{
    typedef ushort digit;
    typedef uint dbldigit;

    int len;
    digit digits[BI_DIGITS];

    bigint() {}
    bigint(digit n) { if(n) { len = 1; digits[0] = n; } else len = 0; }
    bigint(const char *s) { parse(s); }
    template<int Y_DIGITS> bigint(const bigint<Y_DIGITS> &y) { *this = y; }

    static int parsedigits(ushort *digits, int maxlen, const char *s)
    {
        int slen = 0;
        while(isxdigit(s[slen])) slen++;
        int len = (slen+2*sizeof(ushort)-1)/(2*sizeof(ushort));
        if(len>maxlen) return 0;
        memset(digits, 0, len*sizeof(ushort));
        loopi(slen)
        {
            int c = s[slen-i-1];
            if(isalpha(c)) c = toupper(c) - 'A' + 10;
            else if(isdigit(c)) c -= '0';
            else return 0;
            digits[i/(2*sizeof(ushort))] |= c<<(4*(i%(2*sizeof(ushort))));
        }
        return len;
    }

    void parse(const char *s)
    {
        len = parsedigits(digits, BI_DIGITS, s);
        shrink();
    }

    void zero() { len = 0; }

    void print(stream *out) const
    {
        vector<char> buf;
        printdigits(buf);
        out->write(buf.getbuf(), buf.length());
    }

    void printdigits(vector<char> &buf) const
    {
        loopi(len)
        {
            digit d = digits[len-i-1];
            loopj(BI_DIGIT_BITS/4)
            {
                uint shift = BI_DIGIT_BITS - (j+1)*4;
                int val = (d >> shift) & 0xF;
                if(val < 10) buf.add('0' + val);
                else buf.add('a' + val - 10);
            }
        }
    }

    template<int Y_DIGITS> bigint &operator=(const bigint<Y_DIGITS> &y)
    {
        len = y.len;
        memcpy(digits, y.digits, len*sizeof(digit));
        return *this;
    }

    bool iszero() const { return !len; }
    bool isone() const { return len==1 && digits[0]==1; }

    int numbits() const
    {
        if(!len) return 0;
        int bits = len*BI_DIGIT_BITS;
        digit last = digits[len-1], mask = 1<<(BI_DIGIT_BITS-1);
        while(mask)
        {
            if(last&mask) return bits;
            bits--;
            mask >>= 1;
        }
        return 0;
    }

    bool hasbit(int n) const { return n/BI_DIGIT_BITS < len && ((digits[n/BI_DIGIT_BITS]>>(n%BI_DIGIT_BITS))&1); }

    template<int X_DIGITS, int Y_DIGITS> bigint &add(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        dbldigit carry = 0;
        int maxlen = max(x.len, y.len), i;
        for(i = 0; i < y.len || carry; i++)
        {
             carry += (i < x.len ? (dbldigit)x.digits[i] : 0) + (i < y.len ? (dbldigit)y.digits[i] : 0);
             digits[i] = (digit)carry;
             carry >>= BI_DIGIT_BITS;
        }
        if(i < x.len && this != &x) memcpy(&digits[i], &x.digits[i], (x.len - i)*sizeof(digit));
        len = max(i, maxlen);
        return *this;
    }
    template<int Y_DIGITS> bigint &add(const bigint<Y_DIGITS> &y) { return add(*this, y); }

    template<int X_DIGITS, int Y_DIGITS> bigint &sub(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        ASSERT(x >= y);
        dbldigit borrow = 0;
        int i;
        for(i = 0; i < y.len || borrow; i++)
        {
             borrow = (1<<BI_DIGIT_BITS) + (dbldigit)x.digits[i] - (i<y.len ? (dbldigit)y.digits[i] : 0) - borrow;
             digits[i] = (digit)borrow;
             borrow = (borrow>>BI_DIGIT_BITS)^1;
        }
        if(i < x.len && this != &x) memcpy(&digits[i], &x.digits[i], (x.len - i)*sizeof(digit));
        len = x.len;
        shrink();
        return *this;
    }
    template<int Y_DIGITS> bigint &sub(const bigint<Y_DIGITS> &y) { return sub(*this, y); }

    void shrink() { while(len && !digits[len-1]) len--; }

    template<int X_DIGITS, int Y_DIGITS> bigint &mul(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        if(!x.len || !y.len) { len = 0; return *this; }
        memset(digits, 0, y.len*sizeof(digit));
        loopi(x.len)
        {
            dbldigit carry = 0;
            loopj(y.len)
            {
                carry += (dbldigit)x.digits[i] * (dbldigit)y.digits[j] + (dbldigit)digits[i+j];
                digits[i+j] = (digit)carry;
                carry >>= BI_DIGIT_BITS;
            }
            digits[i+y.len] = carry;
        }
        len = x.len + y.len;
        shrink();
        return *this;
    }

    template<int X_DIGITS> bigint &rshift(const bigint<X_DIGITS> &x, int n)
    {
        if(!len || !n) return *this;
        int dig = (n-1)/BI_DIGIT_BITS;
        n = ((n-1) % BI_DIGIT_BITS)+1;
        digit carry = digit(x.digits[dig]>>n);
        loopi(len-dig-1)
        {
            digit tmp = x.digits[i+dig+1];
            digits[i] = digit((tmp<<(BI_DIGIT_BITS-n)) | carry);
            carry = digit(tmp>>n);
        }
        digits[len-dig-1] = carry;
        len -= dig + (n>>BI_DIGIT_BITS);
        shrink();
        return *this;
    }
    bigint &rshift(int n) { return rshift(*this, n); }

    template<int X_DIGITS> bigint &lshift(const bigint<X_DIGITS> &x, int n)
    {
        if(!len || !n) return *this;
        int dig = n/BI_DIGIT_BITS;
        n %= BI_DIGIT_BITS;
        digit carry = 0;
        for(int i = len-1; i>=0; i--)
        {
            digit tmp = x.digits[i];
            digits[i+dig] = digit((tmp<<n) | carry);
            carry = digit(tmp>>(BI_DIGIT_BITS-n));
        }
        len += dig;
        if(carry) digits[len++] = carry;
        if(dig) memset(digits, 0, dig*sizeof(digit));
        return *this;
    }
    bigint &lshift(int n) { return lshift(*this, n); }

    template<int Y_DIGITS> bool operator==(const bigint<Y_DIGITS> &y) const
    {
        if(len!=y.len) return false;
        for(int i = len-1; i>=0; i--) if(digits[i]!=y.digits[i]) return false;
        return true;
    }
    template<int Y_DIGITS> bool operator!=(const bigint<Y_DIGITS> &y) const { return !(*this==y); }
    template<int Y_DIGITS> bool operator<(const bigint<Y_DIGITS> &y) const
    {
        if(len<y.len) return true;
        if(len>y.len) return false;
        for(int i = len-1; i>=0; i--)
        {
            if(digits[i]<y.digits[i]) return true;
            if(digits[i]>y.digits[i]) return false;
        }
        return false;
    }
    template<int Y_DIGITS> bool operator>(const bigint<Y_DIGITS> &y) const { return y<*this; }
    template<int Y_DIGITS> bool operator<=(const bigint<Y_DIGITS> &y) const { return !(y<*this); }
    template<int Y_DIGITS> bool operator>=(const bigint<Y_DIGITS> &y) const { return !(*this<y); }
};

#define GF_BITS         192
#define GF_DIGITS       ((GF_BITS+BI_DIGIT_BITS-1)/BI_DIGIT_BITS)

typedef bigint<GF_DIGITS+1> gfint;

/* NIST prime Galois fields.
 * Currently only supports NIST P-192, where P=2^192-2^64-1.
 */
struct gfield : gfint
{
    static const gfield P;

    gfield() {}
    gfield(digit n) : gfint(n) {}
    gfield(const char *s) : gfint(s) {}

    template<int Y_DIGITS> gfield(const bigint<Y_DIGITS> &y) : gfint(y) {}

    template<int Y_DIGITS> gfield &operator=(const bigint<Y_DIGITS> &y)
    {
        gfint::operator=(y);
        return *this;
    }

    template<int X_DIGITS, int Y_DIGITS> gfield &add(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        gfint::add(x, y);
        if(*this >= P) gfint::sub(*this, P);
        return *this;
    }
    template<int Y_DIGITS> gfield &add(const bigint<Y_DIGITS> &y) { return add(*this, y); }

    template<int X_DIGITS> gfield &mul2(const bigint<X_DIGITS> &x) { return add(x, x); }
    gfield &mul2() { return mul2(*this); }

    template<int X_DIGITS> gfield &div2(const bigint<X_DIGITS> &x)
    {
        if(hasbit(0)) { gfint::add(x, P); rshift(1); }
        else rshift(x, 1);
        return *this;
    }
    gfield &div2() { return div2(*this); }

    template<int X_DIGITS, int Y_DIGITS> gfield &sub(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        if(x < y)
        {
            gfint tmp; /* necessary if this==&y, using this instead would clobber y */
            tmp.add(x, P);
            gfint::sub(tmp, y);
        }
        else gfint::sub(x, y);
        return *this;
    }
    template<int Y_DIGITS> gfield &sub(const bigint<Y_DIGITS> &y) { return sub(*this, y); }

    template<int X_DIGITS> gfield &neg(const bigint<X_DIGITS> &x)
    {
        gfint::sub(P, x);
        return *this;
    }
    gfield &neg() { return neg(*this); }

    template<int X_DIGITS> gfield &square(const bigint<X_DIGITS> &x) { return mul(x, x); }
    gfield &square() { return square(*this); }

    template<int X_DIGITS, int Y_DIGITS> gfield &mul(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        bigint<X_DIGITS+Y_DIGITS> result;
        result.mul(x, y);
        reduce(result);
        return *this;
    }
    template<int Y_DIGITS> gfield &mul(const bigint<Y_DIGITS> &y) { return mul(*this, y); }

    template<int RESULT_DIGITS> void reduce(const bigint<RESULT_DIGITS> &result)
    {
#if GF_BITS==192
        len = min(result.len, GF_DIGITS);
        memcpy(digits, result.digits, len*sizeof(digit));
        shrink();

        if(result.len > 192/BI_DIGIT_BITS)
        {
            gfield s;
            memcpy(s.digits, &result.digits[192/BI_DIGIT_BITS], min(result.len-192/BI_DIGIT_BITS, 64/BI_DIGIT_BITS)*sizeof(digit));
            if(result.len < 256/BI_DIGIT_BITS) memset(&s.digits[result.len-192/BI_DIGIT_BITS], 0, (256/BI_DIGIT_BITS-result.len)*sizeof(digit));
            memcpy(&s.digits[64/BI_DIGIT_BITS], s.digits, 64/BI_DIGIT_BITS*sizeof(digit));
            s.len = 128/BI_DIGIT_BITS;
            s.shrink();
            add(s);

            if(result.len > 256/BI_DIGIT_BITS)
            {
                memset(s.digits, 0, 64/BI_DIGIT_BITS*sizeof(digit));
                memcpy(&s.digits[64/BI_DIGIT_BITS], &result.digits[256/BI_DIGIT_BITS], min(result.len-256/BI_DIGIT_BITS, 64/BI_DIGIT_BITS)*sizeof(digit));
                if(result.len < 320/BI_DIGIT_BITS) memset(&s.digits[result.len+(64-256)/BI_DIGIT_BITS], 0, (320/BI_DIGIT_BITS-result.len)*sizeof(digit));
                memcpy(&s.digits[128/BI_DIGIT_BITS], &s.digits[64/BI_DIGIT_BITS], 64/BI_DIGIT_BITS*sizeof(digit));
                s.len = GF_DIGITS;
                s.shrink();
                add(s);

                if(result.len > 320/BI_DIGIT_BITS)
                {
                    memcpy(s.digits, &result.digits[320/BI_DIGIT_BITS], min(result.len-320/BI_DIGIT_BITS, 64/BI_DIGIT_BITS)*sizeof(digit));
                    if(result.len < 384/BI_DIGIT_BITS) memset(&s.digits[result.len-320/BI_DIGIT_BITS], 0, (384/BI_DIGIT_BITS-result.len)*sizeof(digit));
                    memcpy(&s.digits[64/BI_DIGIT_BITS], s.digits, 64/BI_DIGIT_BITS*sizeof(digit));
                    memcpy(&s.digits[128/BI_DIGIT_BITS], s.digits, 64/BI_DIGIT_BITS*sizeof(digit));
                    s.len = GF_DIGITS;
                    s.shrink();
                    add(s);
                }
            }
        }
        else if(*this >= P) gfint::sub(*this, P);
#else
#error Unsupported GF
#endif
    }

    template<int X_DIGITS, int Y_DIGITS> gfield &pow(const bigint<X_DIGITS> &x, const bigint<Y_DIGITS> &y)
    {
        gfield a(x);
        if(y.hasbit(0)) *this = a;
        else
        {
            len = 1;
            digits[0] = 1;
            if(!y.len) return *this;
        }
        for(int i = 1, j = y.numbits(); i < j; i++)
        {
            a.square();
            if(y.hasbit(i)) mul(a);
        }
        return *this;
    }
    template<int Y_DIGITS> gfield &pow(const bigint<Y_DIGITS> &y) { return pow(*this, y); }

    bool invert(const gfield &x)
    {
        if(!x.len) return false;
        gfint u(x), v(P), A((gfint::digit)1), C((gfint::digit)0);
        while(!u.iszero())
        {
            int ushift = 0, ashift = 0;
            while(!u.hasbit(ushift))
            {
                ushift++;
                if(A.hasbit(ashift))
                {
                    if(ashift) { A.rshift(ashift); ashift = 0; }
                    A.add(P);
                }
                ashift++;
            }
            if(ushift) u.rshift(ushift);
            if(ashift) A.rshift(ashift);
            int vshift = 0, cshift = 0;
            while(!v.hasbit(vshift))
            {
                vshift++;
                if(C.hasbit(cshift))
                {
                    if(cshift) { C.rshift(cshift); cshift = 0; }
                    C.add(P);
                }
                cshift++;
            }
            if(vshift) v.rshift(vshift);
            if(cshift) C.rshift(cshift);
            if(u >= v)
            {
                u.sub(v);
                if(A < C) A.add(P);
                A.sub(C);
            }
            else
            {
                v.sub(v, u);
                if(C < A) C.add(P);
                C.sub(A);
            }
        }
        if(C >= P) gfint::sub(C, P);
        else { len = C.len; memcpy(digits, C.digits, len*sizeof(digit)); }
        ASSERT(*this < P);
        return true;
    }
    void invert() { invert(*this); }

    template<int X_DIGITS> static int legendre(const bigint<X_DIGITS> &x)
    {
        static const gfint Psub1div2(gfint(P).sub(bigint<1>(1)).rshift(1));
        gfield L;
        L.pow(x, Psub1div2);
        if(!L.len) return 0;
        if(L.len==1) return 1;
        return -1;
    }
    int legendre() const { return legendre(*this); }

    bool sqrt(const gfield &x)
    {
        if(!x.len) { len = 0; return true; }
#if GF_BITS==224
#error Unsupported GF
#else
        ASSERT((P.digits[0]%4)==3);
        static const gfint Padd1div4(gfint(P).add(bigint<1>(1)).rshift(2));
        switch(legendre(x))
        {
            case 0: len = 0; return true;
            case -1: return false;
            default: pow(x, Padd1div4); return true;
        }
#endif
    }
    bool sqrt() { return sqrt(*this); }
};

struct ecjacobian
{
    static const gfield B;
    static const ecjacobian base;
    static const ecjacobian origin;

    gfield x, y, z;

    ecjacobian() {}
    ecjacobian(const gfield &x, const gfield &y) : x(x), y(y), z(bigint<1>(1)) {}
    ecjacobian(const gfield &x, const gfield &y, const gfield &z) : x(x), y(y), z(z) {}

    void mul2()
    {
        if(z.iszero()) return;
        else if(y.iszero()) { *this = origin; return; }
        gfield a, b, c, d;
        d.sub(x, c.square(z));
        d.mul(c.add(x));
        c.mul2(d).add(d);
        z.mul(y).add(z);
        a.square(y);
        b.mul2(a);
        d.mul2(x).mul(b);
        x.square(c).sub(d).sub(d);
        a.square(b).add(a);
        y.sub(d, x).mul(c).sub(a);
    }

    void add(const ecjacobian &q)
    {
        if(q.z.iszero()) return;
        else if(z.iszero()) { *this = q; return; }
        gfield a, b, c, d, e, f;
        a.square(z);
        b.mul(q.y, a).mul(z);
        a.mul(q.x);
        if(q.z.isone())
        {
            c.add(x, a);
            d.add(y, b);
            a.sub(x, a);
            b.sub(y, b);
        }
        else
        {
            f.mul(y, e.square(q.z)).mul(q.z);
            e.mul(x);
            c.add(e, a);
            d.add(f, b);
            a.sub(e, a);
            b.sub(f, b);
        }
        if(a.iszero()) { if(b.iszero()) mul2(); else *this = origin; return; }
        if(!q.z.isone()) z.mul(q.z);
        z.mul(a);
        x.square(b).sub(f.mul(c, e.square(a)));
        y.sub(f, x).sub(x).mul(b).sub(e.mul(a).mul(d)).div2();
    }

    template<int Q_DIGITS> void mul(const ecjacobian &p, const bigint<Q_DIGITS> q)
    {
        *this = origin;
        for(int i = q.numbits()-1; i >= 0; i--)
        {
            mul2();
            if(q.hasbit(i)) add(p);
        }
    }
    template<int Q_DIGITS> void mul(const bigint<Q_DIGITS> q) { ecjacobian tmp(*this); mul(tmp, q); }

    void normalize()
    {
        if(z.iszero() || z.isone()) return;
        gfield tmp;
        z.invert();
        tmp.square(z);
        x.mul(tmp);
        y.mul(tmp).mul(z);
        z = bigint<1>(1);
    }

    bool calcy(bool ybit)
    {
        gfield y2, tmp;
        y2.square(x).mul(x).sub(tmp.add(x, x).add(x)).add(B);
        if(!y.sqrt(y2)) { y.zero(); return false; }
        if(y.hasbit(0) != ybit) y.neg();
        return true;
    }

    void print(vector<char> &buf)
    {
        normalize();
        buf.add(y.hasbit(0) ? '-' : '+');
        x.printdigits(buf);
    }

    void parse(const char *s)
    {
        bool ybit = *s++ == '-';
        x.parse(s);
        calcy(ybit);
        z = bigint<1>(1);
    }
};

const ecjacobian ecjacobian::origin(gfield((gfield::digit)1), gfield((gfield::digit)1), gfield((gfield::digit)0));

#if GF_BITS==192
const gfield gfield::P("fffffffffffffffffffffffffffffffeffffffffffffffff");
const gfield ecjacobian::B("64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1");
const ecjacobian ecjacobian::base(
    gfield("188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012"),
    gfield("07192b95ffc8da78631011ed6b24cdd573f977a11e794811")
);
#elif GF_BITS==224
const gfield gfield::P("ffffffffffffffffffffffffffffffff000000000000000000000001");
const gfield ecjacobian::B("b4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4");
const ecjacobian ecjacobian::base(
    gfield("b70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21"),
    gfield("bd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34"),
);
#elif GF_BITS==256
const gfield gfield::P("ffffffff00000001000000000000000000000000ffffffffffffffffffffffff");
const gfield ecjacobian::B("5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b");
const ecjacobian ecjacobian::base(
    gfield("6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296"),
    gfield("4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"),
);
#elif GF_BITS==384
const gfield gfield::P("fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffff0000000000000000ffffffff");
const gfield ecjacobian::B("b3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875ac656398d8a2ed19d2a85c8edd3ec2aef");
const ecjacobian ecjacobian::base(
    gfield("aa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a385502f25dbf55296c3a545e3872760ab7"),
    gfield("3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f"),
);
#elif GF_BITS==521
const gfield gfield::P("1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
const gfield ecjacobian::B("051953eb968e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00");
const ecjacobian ecjacobian::base(
    gfield("c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66"),
    gfield("11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650")
);
#else
#error Unsupported GF
#endif

void genprivkey(const char *seed, vector<char> &privstr, vector<char> &pubstr)
{
    tiger::hashval hash;
    tiger::hash((const uchar *)seed, (int)strlen(seed), hash);
    bigint<8*sizeof(hash.bytes)/BI_DIGIT_BITS> privkey;
    memcpy(privkey.digits, hash.bytes, sizeof(privkey.digits));
    privkey.len = 8*sizeof(hash.bytes)/BI_DIGIT_BITS;
    privkey.shrink();
    privkey.printdigits(privstr);
    privstr.add('\0');

    ecjacobian c(ecjacobian::base);
    c.mul(privkey);
    c.normalize();
    c.print(pubstr);
    pubstr.add('\0');
}

bool hashstring(const char *str, char *result, int maxlen)
{
    tiger::hashval hv;
    if(maxlen < 2*(int)sizeof(hv.bytes) + 1) return false;
    tiger::hash((uchar *)str, strlen(str), hv);
    loopi(sizeof(hv.bytes))
    {
        uchar c = hv.bytes[i];
        *result++ = "0123456789abcdef"[c&0xF];
        *result++ = "0123456789abcdef"[c>>4];
    }
    *result = '\0';
    return true;
}

void answerchallenge(const char *privstr, const char *challenge, vector<char> &answerstr)
{
    gfint privkey;
    privkey.parse(privstr);
    ecjacobian answer;
    answer.parse(challenge);
    answer.mul(privkey);
    answer.normalize();
    answer.x.printdigits(answerstr);
    answerstr.add('\0');
}

void *parsepubkey(const char *pubstr)
{
    ecjacobian *pubkey = new ecjacobian;
    pubkey->parse(pubstr);
    return pubkey;
}

void freepubkey(void *pubkey)
{
    delete (ecjacobian *)pubkey;
}

void *genchallenge(void *pubkey, const void *seed, int seedlen, vector<char> &challengestr)
{
    tiger::hashval hash;
    tiger::hash((const uchar *)seed, sizeof(seed), hash);
    gfint challenge;
    memcpy(challenge.digits, hash.bytes, sizeof(challenge.digits));
    challenge.len = 8*sizeof(hash.bytes)/BI_DIGIT_BITS;
    challenge.shrink();

    ecjacobian answer(*(ecjacobian *)pubkey);
    answer.mul(challenge);
    answer.normalize();

    ecjacobian secret(ecjacobian::base);
    secret.mul(challenge);
    secret.normalize();

    secret.print(challengestr);
    challengestr.add('\0');
   
    return new gfield(answer.x);
}

void freechallenge(void *answer)
{
    delete (gfint *)answer;
}

bool checkchallenge(const char *answerstr, void *correct)
{
    gfint answer(answerstr);
    return answer == *(gfint *)correct;
}

