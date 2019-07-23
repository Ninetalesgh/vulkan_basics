#include "math/matrix.h"


float4x4 look_at_matrix(float3 eye, float3 target, float3 up)
{
  const float3 f = normalize(target - eye);
  const float3 s = normalize(cross(up, f));
  const float3 u = cross(f, s);
  float4x4 result;
  result[0][0] = s.x;
  result[1][0] = s.y;
  result[2][0] = s.z;
  result[3][0] = -dot(s, eye);
  result[0][1] = u.x;
  result[1][1] = u.y;
  result[2][1] = u.z;
  result[3][1] = -dot(u, eye);
  result[0][2] = f.x;
  result[1][2] = f.y;
  result[2][2] = f.z;
  result[3][2] = -dot(f, eye);

  result[0][3] = 0;
  result[1][3] = 0;
  result[2][3] = 0;
  result[3][3] = 1;

  return result;
}

float4x4 projection_matrix(float _window_width, float _window_height, float _fov_radians, float _z_near, float _z_far)
{
  float tanHalfFovy = tanf(_fov_radians * 0.5f);
  float aspectRatio = _window_width / _window_height;

  float4x4 m = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);

  m[0][0] = 1.0f / (aspectRatio * tanHalfFovy);
  m[1][1] = 1.0f / (tanHalfFovy);
  m[2][2] = (_z_far) / (_z_far - _z_near);
  m[2][3] = 1.0f;//copy z value to w for perpsective divide
  m[3][2] = -(_z_far * _z_near) / (_z_far - _z_near);
  m[3][3] = 0.0f;
  return m;
}

float4x4 rotation_matrix_x(float angle)
{
  float4x4 res = float4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, cosf(angle), -sinf(angle), 0.0f,
    0.0f, sinf(angle), cosf(angle), 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
  return res;
}

float4x4 rotation_matrix_y(float angle)
{
  float4x4 res = float4x4(
    cosf(angle), 0.0f, sinf(angle), 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    -sinf(angle), 0.0f, cosf(angle), 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
  return res;
}

float4x4 rotation_matrix_z(float angle)
{
  float4x4 res = float4x4(
    cosf(angle), -sinf(angle), 0.0f, 0.0f,
    sinf(angle), cosf(angle), 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
  return res;
}