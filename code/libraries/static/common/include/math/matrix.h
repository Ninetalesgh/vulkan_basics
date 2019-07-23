#ifndef COMMON_MATRIX_H
#define COMMON_MATRIX_H

#include <common.h>

#include <cstdint>
#include <math.h>


constexpr float PI = 3.14159265359f;

class int2
{
public:
  int2() : x(0), y(0) {}
  int2(const int2& _) : x(_.x), y(_.y) {}
  int2(int32_t _x, int32_t _y) : x(_x), y(_y) {}


  int32_t x, y;
};


class float4
{
public:
  float4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
  float4(const float4& _) : x(_.x), y(_.y), z(_.z), w(_.w) {}
  float4(float _) : x(_), y(_), z(_), w(_) {}
  float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

  const INLINE float  operator[] (int i) const { return data[i]; }
  INLINE float& operator[] (int i) { return data[i]; }

  friend INLINE float4 operator+(const float4& a, const float4& b) { return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
  friend INLINE float4 operator-(const float4& a, const float4& b) { return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
  friend INLINE float4 operator*(float a, const float4& b) { return float4(a * b.x, a * b.y, a * b.z, a * b.w); }
  friend INLINE float4 operator*(const float4& a, float b) { return float4(a.x * b, a.y * b, a.z * b, a.w * b); }
  friend INLINE float4 operator/(const float4& a, float b) { return float4(a.x / b, a.y / b, a.z / b, a.w / b); }

  INLINE float4& operator+=(const float4& a) { *this = *this + a; return *this; }
  INLINE float4& operator-=(const float4& a) { *this = *this - a; return *this; }
  INLINE float4& operator*=(float a) { *this = *this * a; return *this; }
  INLINE float4& operator/=(float a) { *this = *this / a; return *this; }

  union
  {
    struct { float x, y, z, w; };
    struct { float data[4]; };
  };
};


class float3
{
public:
  float3() : x(0.f), y(0.f), z(0.f) {}
  float3(const float3& _) : x(_.x), y(_.y), z(_.z) {}
  float3(float _) : x(_), y(_), z(_) {}
  float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
  float3(const float4& _) : x(_.x), y(_.y), z(_.z) {}

  INLINE float3	operator-() { return float3(-x, -y, -z); }

  INLINE operator float4() const { return float4(x, y, z, 0.f); }

  float x, y, z;
};


class float2
{
public:
  float2() : x(0.f), y(0.f) {}
  float2(const float2& _) : x(_.x), y(_.y) {}
  float2(float _) : x(_), y(_) {}
  float2(float _x, float _y) : x(_x), y(_y) {}
  float2(const float3& _) : x(_.x), y(_.y) {}
  float2(const float4& _) : x(_.x), y(_.y) {}

  INLINE operator float3() const { return float3(x, y, 0); }
  INLINE operator float4() const { return float4(x, y, 0.f, 0.f); }

  float x, y;
};


struct float4x4
{
public:
  float4x4()
    : i0j0(1.f), i1j0(0.f), i2j0(0.f), i3j0(0.f),
    i0j1(0.f), i1j1(1.f), i2j1(0.f), i3j1(0.f),
    i0j2(0.f), i1j2(0.f), i2j2(1.f), i3j2(0.f),
    i0j3(0.f), i1j3(0.f), i2j3(0.f), i3j3(1.f)
  {}
  float4x4(const float4x4& _)
    : i0j0(_.i0j0), i1j0(_.i1j0), i2j0(_.i2j0), i3j0(_.i3j0),
    i0j1(_.i0j1), i1j1(_.i1j1), i2j1(_.i2j1), i3j1(_.i3j1),
    i0j2(_.i0j2), i1j2(_.i1j2), i2j2(_.i2j2), i3j2(_.i3j2),
    i0j3(_.i0j3), i1j3(_.i1j3), i2j3(_.i2j3), i3j3(_.i3j3)
  {}
  float4x4(float j0i0, float j0i1, float j0i2, float j0i3, float j1i0, float j1i1, float j1i2, float j1i3, float j2i0, float j2i1, float j2i2, float j2i3, float j3i0, float j3i1, float j3i2, float j3i3)
    : i0j0(j0i0), i1j0(j0i1), i2j0(j0i2), i3j0(j0i3),
    i0j1(j1i0), i1j1(j1i1), i2j1(j1i2), i3j1(j1i3),
    i0j2(j2i0), i1j2(j2i1), i2j2(j2i2), i3j2(j2i3),
    i0j3(j3i0), i1j3(j3i1), i2j3(j3i2), i3j3(j3i3)
  {}
  float4x4(const float4& a, const float4& b, const float4& c, const float4& d)
    : j0(a), j1(b), j2(c), j3(d)
  {}

  union
  {
    struct { float4 j0, j1, j2, j3; };
    struct { float4 col[4]; };
    struct { float data[16]; };
    struct
    {
      float i0j0, i1j0, i2j0, i3j0;
      float i0j1, i1j1, i2j1, i3j1;
      float i0j2, i1j2, i2j2, i3j2;
      float i0j3, i1j3, i2j3, i3j3;
    };
  };

  const INLINE float4  operator[](int i) const { return col[i]; }
  INLINE float4& operator[](int i) { return col[i]; }

  INLINE float4x4 operator-() { return float4x4(-i0j0, -i1j0, -i2j0, -i3j0, -i0j1, -i1j1, -i2j1, -i3j1, -i0j2, -i1j2, -i2j2, -i3j2, -i0j3, -i1j3, -i2j3, -i3j3); }
  friend INLINE float4x4 operator+ (const float4x4& a, const float4x4& b) { return float4x4(a.j0 + b.j0, a.j1 + b.j1, a.j2 + b.j2, a.j3 + b.j3); }
  friend INLINE float4x4 operator- (const float4x4& a, const float4x4& b) { return float4x4(a.j0 - b.j0, a.j1 - b.j1, a.j2 - b.j2, a.j3 - b.j3); }

  friend INLINE float4x4 operator* (const float4x4& a, const float4x4& b)
  {					//column 0
    return float4x4(a.data[0] * b.data[0] + a.data[4] * b.data[1] + a.data[8] * b.data[2] + a.data[12] * b.data[3], \
      a.data[1] * b.data[0] + a.data[5] * b.data[1] + a.data[9] * b.data[2] + a.data[13] * b.data[3], \
      a.data[2] * b.data[0] + a.data[6] * b.data[1] + a.data[10] * b.data[2] + a.data[14] * b.data[3], \
      a.data[3] * b.data[0] + a.data[7] * b.data[1] + a.data[11] * b.data[2] + a.data[15] * b.data[3], \
      //column 1  					    				  	    
      a.data[0] * b.data[4] + a.data[4] * b.data[5] + a.data[8] * b.data[6] + a.data[12] * b.data[7], \
      a.data[1] * b.data[4] + a.data[5] * b.data[5] + a.data[9] * b.data[6] + a.data[13] * b.data[7], \
      a.data[2] * b.data[4] + a.data[6] * b.data[5] + a.data[10] * b.data[6] + a.data[14] * b.data[7], \
      a.data[3] * b.data[4] + a.data[7] * b.data[5] + a.data[11] * b.data[6] + a.data[15] * b.data[7], \
      //column 2	 					    
      a.data[0] * b.data[8] + a.data[4] * b.data[9] + a.data[8] * b.data[10] + a.data[12] * b.data[11], \
      a.data[1] * b.data[8] + a.data[5] * b.data[9] + a.data[9] * b.data[10] + a.data[13] * b.data[11], \
      a.data[2] * b.data[8] + a.data[6] * b.data[9] + a.data[10] * b.data[10] + a.data[14] * b.data[11], \
      a.data[3] * b.data[8] + a.data[7] * b.data[9] + a.data[11] * b.data[10] + a.data[15] * b.data[11], \
      //column 3
      a.data[0] * b.data[12] + a.data[4] * b.data[13] + a.data[8] * b.data[14] + a.data[12] * b.data[15], \
      a.data[1] * b.data[12] + a.data[5] * b.data[13] + a.data[9] * b.data[14] + a.data[13] * b.data[15], \
      a.data[2] * b.data[12] + a.data[6] * b.data[13] + a.data[10] * b.data[14] + a.data[14] * b.data[15], \
      a.data[3] * b.data[12] + a.data[7] * b.data[13] + a.data[11] * b.data[14] + a.data[15] * b.data[15]);
  }

};


class quaternion
{
public:
  quaternion() : t(1.f), x(0.f), y(0.f), z(0.f) {}
  quaternion(const quaternion& a) : t(a.t), x(a.x), y(a.y), z(a.z) {}
  quaternion(float real, const float3& imaginary)
    : t(real), v(imaginary) {}
  quaternion(float real, float imaginary1, float imaginary2, float imaginary3)
    : t(real), x(imaginary1), y(imaginary2), z(imaginary3) {}
  quaternion(const float3& euler) {
    float3 c(cos(euler.x * .5f), cos(euler.y * .5f), cos(euler.z * .5f));
    float3 s(sin(euler.x * .5f), sin(euler.y * .5f), sin(euler.z * .5f));
    t = c.x * c.y * c.z + s.x * s.y * s.z;
    x = s.x * c.y * c.z - c.x * s.y * s.z;
    y = c.x * s.y * c.z + s.x * c.y * s.z;
    z = c.x * c.y * s.z - s.x * s.y * c.z;
  }

  union {
    struct {
      float t;
      union {
        struct { float3 v; };
        struct { float x, y, z; };
      };
    };
    struct { float data[4]; };
  };


  friend INLINE quaternion operator* (const quaternion& a, const quaternion& b)
  { 						  //Euler's Four Square Identity            (practically)
    return quaternion((a.t * b.t) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
                      (a.t * b.x) + (a.x * b.t) + (a.y * b.z) - (a.z * b.y),
                      (a.t * b.y) + (a.y * b.t) + (a.z * b.x) - (a.x * b.z),
                      (a.t * b.z) + (a.z * b.t) + (a.x * b.y) - (a.y * b.x));
  }
};


class vertex
{
public:
  vertex()
    : pos(0),
    uv(0)
  {}
  vertex(float3 _pos, float2 _uv)
    :
    pos(_pos),
    uv(_uv) {}
  float3 pos;
  float2 uv;

};


// cross class operators

INLINE float2 operator+(const float2& a, const float2& b) { return float2(a.x + b.x, a.y + b.y); }
INLINE float2 operator-(const float2& a, const float2& b) { return float2(a.x - b.x, a.y - b.y); }
INLINE float2 operator*(float a, const float2& b) { return float2(a * b.x, a * b.y); }
INLINE float2 operator*(const float2& a, float b) { return float2(a.x * b, a.y * b); }
INLINE float2 operator/(const float2& a, float b) { return float2(a.x / b, a.y / b); }

INLINE float3 operator+(const float3& a, const float3& b) { return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
INLINE float3 operator-(const float3& a, const float3& b) { return float3(a.x - b.x, a.y - b.y, a.z - b.z); }
INLINE float3 operator*(float         a, const float3& b) { return float3(a * b.x, a * b.y, a * b.z); }
INLINE float3 operator*(const float3& a, float b) { return float3(a.x * b, a.y * b, a.z * b); }
INLINE float3 operator/(const float3& a, float b) { return float3(a.x / b, a.y / b, a.z / b); }

INLINE float4x4 operator*(float a, const float4x4& b) { return float4x4(a * b.j0, a * b.j1, a * b.j2, a * b.j3); }
INLINE float4x4 operator*(const float4x4& a, float b) { return float4x4(a.j0 * b, a.j1 * b, a.j2 * b, a.j3 * b); }

INLINE quaternion operator / (const quaternion& a, float b) { return quaternion(a.t / b, a.x / b, a.y / b, a.z / b); }

// - - - - - - - - - - - 


//free functions
//TODO naming

INLINE float		  quadrant(const float2& a) { return (a.x * a.x + a.y * a.y); }
INLINE float		  quadrant(const float3& a) { return (a.x * a.x + a.y * a.y + a.z * a.z); }
INLINE float		  quadrant(const float4& a) { return (a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w); }
INLINE float			quadrant(const quaternion& a) { return (a.t * a.t + a.x * a.x + a.y * a.y + a.z * a.z); }

INLINE float		  magnitude(const float2& a) { return sqrt(quadrant(a)); }
INLINE float		  magnitude(const float3& a) { return sqrt(quadrant(a)); }
INLINE float		  magnitude(const float4& a) { return sqrt(quadrant(a)); }
INLINE float      magnitude(const quaternion& a) { return sqrtf(a.t * a.t + a.x * a.x * a.y * a.y * a.z * a.z); }

INLINE float2	  	normalize(const float2& a) { float invm = 1.0f / magnitude(a); return a * invm; }
INLINE float3		  normalize(const float3& a) { float invm = 1.0f / magnitude(a); return a * invm; }
INLINE float4		  normalize(const float4& a) { float invm = 1.0f / magnitude(a); return a * invm; }
INLINE quaternion	normalize(const quaternion& a) { return (a / magnitude(a)); }

INLINE float      dot(const float3& a, const float3& b) { return (a.x * b.x + a.y * b.y + a.z * b.z); }

INLINE float3	  	cross(const float3& a, const float3& b) { return float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }


INLINE quaternion	quaternion_conjugate(const quaternion& a) { return quaternion(a.t, -a.x, -a.y, -a.z); }
INLINE quaternion	quaternion_inverse(const quaternion& a) { return quaternion(quaternion_conjugate(a) / quadrant(a)); }
//Rotate a around b						
INLINE quaternion	quaternion_rotate(const quaternion& a, const quaternion& b) { return quaternion(b * a * quaternion_inverse(b)); }
//rotate a around b
INLINE float3			quaternion_rotate(const float3& a, const quaternion& b)
{
  quaternion c(0, a);
  c = b * c * quaternion_inverse(b);
  return float3(c.x, c.y, c.z);
}

INLINE quaternion	quaternion_from_axis_angle(const float3& a, float angle)
{
  float tmp = sinf(angle / 2.0f);
  float x = a.x * tmp;
  float y = a.y * tmp;
  float z = a.z * tmp;
  float w = cosf(angle / 2.0f);

  return normalize(quaternion(w, x, y, z));
}
float4x4 look_at_matrix(float3 eye, float3 target, float3 up);

float4x4 projection_matrix(float _window_width, float _window_height, float _fov_radians, float _z_near, float _z_far);

float4x4 rotation_matrix_x(float x);

float4x4 rotation_matrix_y(float y);

float4x4 rotation_matrix_z(float z);


#include "../inl/math/matrix.inl"
#endif // !MATRIX_H
