#ifndef LINMATH_H
#define LINMATH_H

#include <math.h>

#ifdef _MSC_VER
#define inline __inline
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float to_degrees(float radians)
{
  return radians * (180.0 / M_PI);
}

static inline float to_radians(float degrees)
{
  return degrees * (M_PI / 180.0);
}

static inline float lerp(float p1, float p2, float amount) {
  return (p2 * amount) + (p1 * (1 - amount));
}

static inline float clamp(float x, float lower, float upper) {
  return fmin(upper, fmax(x, lower));
}

#define LINMATH_H_DEFINE_VEC(n) \
typedef float vec##n[n]; \
static inline void vec##n##_add(vec##n r, vec##n const a, vec##n const b) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = a[i] + b[i]; \
} \
static inline void vec##n##_sub(vec##n r, vec##n const a, vec##n const b) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = a[i] - b[i]; \
} \
static inline void vec##n##_scale(vec##n r, vec##n const v, float const s) \
{ \
	int i; \
	for(i=0; i<n; ++i) \
		r[i] = v[i] * s; \
} \
static inline float vec##n##_mul_inner(vec##n const a, vec##n const b) \
{ \
	float p = 0.; \
	int i; \
	for(i=0; i<n; ++i) \
		p += b[i]*a[i]; \
	return p; \
} \
static inline float vec##n##_len(vec##n const v) \
{ \
	return (float) sqrt(vec##n##_mul_inner(v,v)); \
} \
static inline void vec##n##_zero(vec##n r) \
{ \
  for(int i=0; i<n; ++i) \
    r[i] = 0; \
} \
static inline void vec##n##_copy(vec##n r, vec##n const v) \
{ \
  for(int i=0; i<n; ++i) \
    r[i] = v[i]; \
} \
static inline void vec##n##_norm(vec##n r, vec##n const v) \
{ \
  float len = vec##n##_len(v); \
  if (len == 0) { \
    vec##n##_copy(r, v); \
    return; \
  } \
	float k = 1.f / vec##n##_len(v); \
	vec##n##_scale(r, v, k); \
} \
static inline float vec##n##_dot(vec##n const a, vec##n const b) \
{ \
  float v = 0.0f; \
  for(int i=0; i<n; ++i) \
    v += a[i] * b[i]; \
  return v; \
} \
static inline void vec##n##_lerp(vec##n r, vec##n const a, vec##n const b, float amount) \
{ \
  for(int i=0; i<n; ++i) \
    r[i] = lerp(a[i], b[i], amount); \
} \

LINMATH_H_DEFINE_VEC(2)
LINMATH_H_DEFINE_VEC(3)
LINMATH_H_DEFINE_VEC(4)

static inline void vec2_rotate(vec2 r, vec2 const v, float angle)
{
  r[0] = v[0]*cosf(angle) - v[1]*sinf(angle);
  r[1] = v[0]*sinf(angle) + v[1]*cosf(angle);
}

static inline void vec3_mul_cross(vec3 r, vec3 const a, vec3 const b)
{
	r[0] = a[1]*b[2] - a[2]*b[1];
	r[1] = a[2]*b[0] - a[0]*b[2];
	r[2] = a[0]*b[1] - a[1]*b[0];
}

static inline void vec3_reflect(vec3 r, vec3 const v, vec3 const n)
{
	float p  = 2.f*vec3_mul_inner(v, n);
	int i;
	for(i=0;i<3;++i)
		r[i] = v[i] - p*n[i];
}

static inline float vec3_angle_between(vec3 a, vec3 b, vec3 up) {
  vec3 n;
  vec3_mul_cross(n, a, up);
  float s = vec3_dot(b, n);

  float angle = acos(vec3_dot(a, b) / (vec3_len(a) * vec3_len(b)));
  if (s < 0) {
    return angle;
  } else {
    return (2 * M_PI) - angle;
  }
}

static inline void vec4_mul_cross(vec4 r, vec4 a, vec4 b)
{
	r[0] = a[1]*b[2] - a[2]*b[1];
	r[1] = a[2]*b[0] - a[0]*b[2];
	r[2] = a[0]*b[1] - a[1]*b[0];
	r[3] = 1.f;
}

static inline void vec4_reflect(vec4 r, vec4 v, vec4 n)
{
	float p  = 2.f*vec4_mul_inner(v, n);
	int i;
	for(i=0;i<4;++i)
		r[i] = v[i] - p*n[i];
}

typedef vec4 mat4[4];
static inline void mat4_identity(mat4 M)
{
	int i, j;
	for(i=0; i<4; ++i)
		for(j=0; j<4; ++j)
			M[i][j] = i==j ? 1.f : 0.f;
}
static inline void mat4_copy(mat4 M, mat4 N)
{
	int i, j;
	for(i=0; i<4; ++i)
		for(j=0; j<4; ++j)
			M[i][j] = N[i][j];
}
static inline void mat4_row(vec4 r, mat4 M, int i)
{
	int k;
	for(k=0; k<4; ++k)
		r[k] = M[k][i];
}
static inline void mat4_col(vec4 r, mat4 M, int i)
{
	int k;
	for(k=0; k<4; ++k)
		r[k] = M[i][k];
}
static inline void mat4_transpose(mat4 M, mat4 N)
{
	int i, j;
	for(j=0; j<4; ++j)
		for(i=0; i<4; ++i)
			M[i][j] = N[j][i];
}
static inline void mat4_add(mat4 M, mat4 a, mat4 b)
{
	int i;
	for(i=0; i<4; ++i)
		vec4_add(M[i], a[i], b[i]);
}
static inline void mat4_sub(mat4 M, mat4 a, mat4 b)
{
	int i;
	for(i=0; i<4; ++i)
		vec4_sub(M[i], a[i], b[i]);
}
static inline void mat4_scale(mat4 M, mat4 a, float k)
{
	int i;
	for(i=0; i<3; ++i)
		vec4_scale(M[i], a[i], k);
}
static inline void mat4_scale_aniso(mat4 M, mat4 a, float x, float y, float z)
{
	int i;
	vec4_scale(M[0], a[0], x);
	vec4_scale(M[1], a[1], y);
	vec4_scale(M[2], a[2], z);
	for(i = 0; i < 4; ++i) {
		M[3][i] = a[3][i];
	}
}
static inline void mat4_mul(mat4 M, mat4 a, mat4 b)
{
	mat4 temp;
	int k, r, c;
	for(c=0; c<4; ++c) for(r=0; r<4; ++r) {
		temp[c][r] = 0.f;
		for(k=0; k<4; ++k)
			temp[c][r] += a[k][r] * b[c][k];
	}
	mat4_copy(M, temp);
}
static inline void mat4_mul_vec4(vec4 r, mat4 M, vec4 v)
{
	int i, j;
	for(j=0; j<4; ++j) {
		r[j] = 0.f;
		for(i=0; i<4; ++i)
			r[j] += M[i][j] * v[i];
	}
}
static inline void mat4_translate(mat4 T, float x, float y, float z)
{
	mat4_identity(T);
	T[3][0] = x;
	T[3][1] = y;
	T[3][2] = z;
}
static inline void mat4_translate_in_place(mat4 M, float x, float y, float z)
{
	vec4 t = {x, y, z, 0};
	vec4 r;
	int i;
	for (i = 0; i < 4; ++i) {
		mat4_row(r, M, i);
		M[3][i] += vec4_mul_inner(r, t);
	}
}
static inline void mat4_from_vec3_mul_outer(mat4 M, vec3 a, vec3 b)
{
	int i, j;
	for(i=0; i<4; ++i) for(j=0; j<4; ++j)
		M[i][j] = i<3 && j<3 ? a[i] * b[j] : 0.f;
}
static inline void mat4_rotate(mat4 R, mat4 M, float x, float y, float z, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	vec3 u = {x, y, z};

	if(vec3_len(u) > 1e-4) {
		mat4 T, C, S = {{0}};

		vec3_norm(u, u);
		mat4_from_vec3_mul_outer(T, u, u);

		S[1][2] =  u[0];
		S[2][1] = -u[0];
		S[2][0] =  u[1];
		S[0][2] = -u[1];
		S[0][1] =  u[2];
		S[1][0] = -u[2];

		mat4_scale(S, S, s);

		mat4_identity(C);
		mat4_sub(C, C, T);

		mat4_scale(C, C, c);

		mat4_add(T, T, C);
		mat4_add(T, T, S);

		T[3][3] = 1.;
		mat4_mul(R, M, T);
	} else {
		mat4_copy(R, M);
	}
}
static inline void mat4_rotate_X(mat4 Q, mat4 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 R = {
		{1.f, 0.f, 0.f, 0.f},
		{0.f,   c,   s, 0.f},
		{0.f,  -s,   c, 0.f},
		{0.f, 0.f, 0.f, 1.f}
	};
	mat4_mul(Q, M, R);
}
static inline void mat4_rotate_Y(mat4 Q, mat4 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 R = {
		{   c, 0.f,   s, 0.f},
		{ 0.f, 1.f, 0.f, 0.f},
		{  -s, 0.f,   c, 0.f},
		{ 0.f, 0.f, 0.f, 1.f}
	};
	mat4_mul(Q, M, R);
}
static inline void mat4_rotate_Z(mat4 Q, mat4 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 R = {
		{   c,   s, 0.f, 0.f},
		{  -s,   c, 0.f, 0.f},
		{ 0.f, 0.f, 1.f, 0.f},
		{ 0.f, 0.f, 0.f, 1.f}
	};
	mat4_mul(Q, M, R);
}
static inline void mat4_invert(mat4 T, mat4 M)
{
	float idet;
	float s[6];
	float c[6];
	s[0] = M[0][0]*M[1][1] - M[1][0]*M[0][1];
	s[1] = M[0][0]*M[1][2] - M[1][0]*M[0][2];
	s[2] = M[0][0]*M[1][3] - M[1][0]*M[0][3];
	s[3] = M[0][1]*M[1][2] - M[1][1]*M[0][2];
	s[4] = M[0][1]*M[1][3] - M[1][1]*M[0][3];
	s[5] = M[0][2]*M[1][3] - M[1][2]*M[0][3];

	c[0] = M[2][0]*M[3][1] - M[3][0]*M[2][1];
	c[1] = M[2][0]*M[3][2] - M[3][0]*M[2][2];
	c[2] = M[2][0]*M[3][3] - M[3][0]*M[2][3];
	c[3] = M[2][1]*M[3][2] - M[3][1]*M[2][2];
	c[4] = M[2][1]*M[3][3] - M[3][1]*M[2][3];
	c[5] = M[2][2]*M[3][3] - M[3][2]*M[2][3];

	/* Assumes it is invertible */
	idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

	T[0][0] = ( M[1][1] * c[5] - M[1][2] * c[4] + M[1][3] * c[3]) * idet;
	T[0][1] = (-M[0][1] * c[5] + M[0][2] * c[4] - M[0][3] * c[3]) * idet;
	T[0][2] = ( M[3][1] * s[5] - M[3][2] * s[4] + M[3][3] * s[3]) * idet;
	T[0][3] = (-M[2][1] * s[5] + M[2][2] * s[4] - M[2][3] * s[3]) * idet;

	T[1][0] = (-M[1][0] * c[5] + M[1][2] * c[2] - M[1][3] * c[1]) * idet;
	T[1][1] = ( M[0][0] * c[5] - M[0][2] * c[2] + M[0][3] * c[1]) * idet;
	T[1][2] = (-M[3][0] * s[5] + M[3][2] * s[2] - M[3][3] * s[1]) * idet;
	T[1][3] = ( M[2][0] * s[5] - M[2][2] * s[2] + M[2][3] * s[1]) * idet;

	T[2][0] = ( M[1][0] * c[4] - M[1][1] * c[2] + M[1][3] * c[0]) * idet;
	T[2][1] = (-M[0][0] * c[4] + M[0][1] * c[2] - M[0][3] * c[0]) * idet;
	T[2][2] = ( M[3][0] * s[4] - M[3][1] * s[2] + M[3][3] * s[0]) * idet;
	T[2][3] = (-M[2][0] * s[4] + M[2][1] * s[2] - M[2][3] * s[0]) * idet;

	T[3][0] = (-M[1][0] * c[3] + M[1][1] * c[1] - M[1][2] * c[0]) * idet;
	T[3][1] = ( M[0][0] * c[3] - M[0][1] * c[1] + M[0][2] * c[0]) * idet;
	T[3][2] = (-M[3][0] * s[3] + M[3][1] * s[1] - M[3][2] * s[0]) * idet;
	T[3][3] = ( M[2][0] * s[3] - M[2][1] * s[1] + M[2][2] * s[0]) * idet;
}
static inline void mat4_orthonormalize(mat4 R, mat4 M)
{
	float s = 1.;
	vec3 h;

	mat4_copy(R, M);
	vec3_norm(R[2], R[2]);

	s = vec3_mul_inner(R[1], R[2]);
	vec3_scale(h, R[2], s);
	vec3_sub(R[1], R[1], h);
	vec3_norm(R[2], R[2]);

	s = vec3_mul_inner(R[1], R[2]);
	vec3_scale(h, R[2], s);
	vec3_sub(R[1], R[1], h);
	vec3_norm(R[1], R[1]);

	s = vec3_mul_inner(R[0], R[1]);
	vec3_scale(h, R[1], s);
	vec3_sub(R[0], R[0], h);
	vec3_norm(R[0], R[0]);
}

static inline void mat4_frustum(mat4 M, float l, float r, float b, float t, float n, float f)
{
	M[0][0] = 2.f*n/(r-l);
	M[0][1] = M[0][2] = M[0][3] = 0.f;

	M[1][1] = 2.f*n/(t-b);
	M[1][0] = M[1][2] = M[1][3] = 0.f;

	M[2][0] = (r+l)/(r-l);
	M[2][1] = (t+b)/(t-b);
	M[2][2] = -(f+n)/(f-n);
	M[2][3] = -1.f;

	M[3][2] = -2.f*(f*n)/(f-n);
	M[3][0] = M[3][1] = M[3][3] = 0.f;
}
static inline void mat4_ortho(mat4 M, float l, float r, float b, float t, float n, float f)
{
	M[0][0] = 2.f/(r-l);
	M[0][1] = M[0][2] = M[0][3] = 0.f;

	M[1][1] = 2.f/(t-b);
	M[1][0] = M[1][2] = M[1][3] = 0.f;

	M[2][2] = -2.f/(f-n);
	M[2][0] = M[2][1] = M[2][3] = 0.f;

	M[3][0] = -(r+l)/(r-l);
	M[3][1] = -(t+b)/(t-b);
	M[3][2] = -(f+n)/(f-n);
	M[3][3] = 1.f;
}
static inline void mat4_perspective(mat4 m, float y_fov, float aspect, float n, float f)
{
	/* NOTE: Degrees are an unhandy unit to work with.
	 * linmath.h uses radians for everything! */
	float const a = 1.f / (float) tan(y_fov / 2.f);

	m[0][0] = a / aspect;
	m[0][1] = 0.f;
	m[0][2] = 0.f;
	m[0][3] = 0.f;

	m[1][0] = 0.f;
	m[1][1] = a;
	m[1][2] = 0.f;
	m[1][3] = 0.f;

	m[2][0] = 0.f;
	m[2][1] = 0.f;
	m[2][2] = -((f + n) / (f - n));
	m[2][3] = -1.f;

	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = -((2.f * f * n) / (f - n));
	m[3][3] = 0.f;
}
static inline void mat4_look_at(mat4 m, vec3 eye, vec3 center, vec3 up)
{
	/* Adapted from Android's OpenGL Matrix.java.                        */
	/* See the OpenGL GLUT documentation for gluLookAt for a description */
	/* of the algorithm. We implement it in a straightforward way:       */

	/* TODO: The negation of of can be spared by swapping the order of
	 *       operands in the following cross products in the right way. */
	vec3 f;
	vec3 s;
	vec3 t;

	vec3_sub(f, center, eye);
	vec3_norm(f, f);

	vec3_mul_cross(s, f, up);
	vec3_norm(s, s);

	vec3_mul_cross(t, s, f);

	m[0][0] =  s[0];
	m[0][1] =  t[0];
	m[0][2] = -f[0];
	m[0][3] =   0.f;

	m[1][0] =  s[1];
	m[1][1] =  t[1];
	m[1][2] = -f[1];
	m[1][3] =   0.f;

	m[2][0] =  s[2];
	m[2][1] =  t[2];
	m[2][2] = -f[2];
	m[2][3] =   0.f;

	m[3][0] =  0.f;
	m[3][1] =  0.f;
	m[3][2] =  0.f;
	m[3][3] =  1.f;

	mat4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);
}

typedef float quat[4];
static inline void quat_copy(quat a, const quat b)
{
  for(int i=0; i<4; ++i)
    a[i] = b[i];
}
static inline void quat_identity(quat q)
{
	q[0] = q[1] = q[2] = 0.f;
	q[3] = 1.f;
}
static inline void quat_add(quat r, quat a, quat b)
{
	int i;
	for(i=0; i<4; ++i)
		r[i] = a[i] + b[i];
}
static inline void quat_sub(quat r, quat a, quat b)
{
	int i;
	for(i=0; i<4; ++i)
		r[i] = a[i] - b[i];
}
static inline void quat_mul(quat r, quat p, quat q)
{
  quat m = {
    (p[3] * q[0]) + (p[0] * q[3]) + (p[1] * q[2]) + (p[2] * q[1]),
    (p[3] * q[1]) + (p[0] * q[2]) + (p[1] * q[3]) + (p[2] * q[0]),
    (p[3] * q[2]) + (p[0] * q[1]) + (p[1] * q[0]) + (p[2] * q[3]),
    (p[3] * q[3]) + (p[0] * q[0]) + (p[1] * q[1]) + (p[2] * q[2]),
  };
  vec4_copy(r, m);
}
static inline void quat_scale(quat r, quat v, float s)
{
	int i;
	for(i=0; i<4; ++i)
		r[i] = v[i] * s;
}
static inline float quat_inner_product(quat a, quat b)
{
	float p = 0.f;
	int i;
	for(i=0; i<4; ++i)
		p += b[i]*a[i];
	return p;
}
static inline void quat_conj(quat r, quat q)
{
	int i;
	for(i=0; i<3; ++i)
		r[i] = -q[i];
	r[3] = q[3];
}
static inline void quat_rotate(quat r, float angle, vec3 axis) {
	int i;
	vec3 v;
	vec3_scale(v, axis, sinf(angle / 2));
	for(i=0; i<3; ++i)
		r[i] = v[i];
	r[3] = cosf(angle / 2);
}
#define quat_norm vec4_norm
static inline void quat_mul_vec3(vec3 r, quat q, vec3 v)
{
/*
 * Method by Fabian 'ryg' Giessen (of Farbrausch)
t = 2 * cross(q.xyz, v)
v' = v + q.w * t + cross(q.xyz, t)
 */
	vec3 t = {q[0], q[1], q[2]};
	vec3 u = {q[0], q[1], q[2]};

	vec3_mul_cross(t, t, v);
	vec3_scale(t, t, 2);

	vec3_mul_cross(u, u, t);
	vec3_scale(t, t, q[3]);

	vec3_add(r, v, t);
	vec3_add(r, r, u);
}
static inline void mat4_from_quat(mat4 M, const quat q)
{
	float a = q[3];
	float b = q[0];
	float c = q[1];
	float d = q[2];
	float a2 = a*a;
	float b2 = b*b;
	float c2 = c*c;
	float d2 = d*d;

	M[0][0] = a2 + b2 - c2 - d2;
	M[0][1] = 2.f*(b*c + a*d);
	M[0][2] = 2.f*(b*d - a*c);
	M[0][3] = 0.f;

	M[1][0] = 2*(b*c - a*d);
	M[1][1] = a2 - b2 + c2 - d2;
	M[1][2] = 2.f*(c*d + a*b);
	M[1][3] = 0.f;

	M[2][0] = 2.f*(b*d + a*c);
	M[2][1] = 2.f*(c*d - a*b);
	M[2][2] = a2 - b2 - c2 + d2;
	M[2][3] = 0.f;

	M[3][0] = M[3][1] = M[3][2] = 0.f;
	M[3][3] = 1.f;
}

static inline void mat4_mul_quat(mat4 R, mat4 M, quat q)
{
/*  XXX: The way this is written only works for othogonal matrices. */
/* TODO: Take care of non-orthogonal case. */
	quat_mul_vec3(R[0], q, M[0]);
	quat_mul_vec3(R[1], q, M[1]);
	quat_mul_vec3(R[2], q, M[2]);

	R[3][0] = R[3][1] = R[3][2] = 0.f;
	R[3][3] = 1.f;
}

static inline void quat_from_mat4(quat q, mat4 M)
{
  float tr = M[0][0] + M[1][1] + M[2][2];

  if (tr > 0) { 
    float S = sqrt(tr+1.0) * 2; // S=4*qw 
    q[3] = 0.25 * S;
    q[0] = (M[1][2] - M[2][1]) / S;
    q[1] = (M[2][0] - M[0][2]) / S; 
    q[2] = (M[0][1] - M[1][0]) / S; 
  } else if ((M[0][0] > M[1][1])&(M[0][0] > M[2][2])) { 
    float S = sqrt(1.0 + M[0][0] - M[1][1] - M[2][2]) * 2; // S=4*qx 
    q[3] = (M[1][2] - M[2][1]) / S;
    q[0] = 0.25 * S;
    q[1] = (M[1][0] + M[0][1]) / S; 
    q[2] = (M[2][0] + M[0][2]) / S; 
  } else if (M[1][1] > M[2][2]) { 
    float S = sqrt(1.0 + M[1][1] - M[0][0] - M[2][2]) * 2; // S=4*qy
    q[3] = (M[2][0] - M[0][2]) / S;
    q[0] = (M[1][0] + M[0][1]) / S; 
    q[1] = 0.25 * S;
    q[2] = (M[2][1] + M[1][2]) / S; 
  } else { 
    float S = sqrt(1.0 + M[2][2] - M[0][0] - M[1][1]) * 2; // S=4*qz
    q[3] = (M[0][1] - M[1][0]) / S;
    q[0] = (M[2][0] + M[0][2]) / S;
    q[1] = (M[2][1] + M[1][2]) / S;
    q[2] = 0.25 * S;
  }
}

static inline void quat_slerp(quat r, quat from, quat to, float amount) {
  float scale0, scale1;
  float	afto1[4];
  float cosom = from[0] * to[0] + from[1] * to[1] + from[2] * to[2] + from[3] * to[3];
  
  if ( cosom < 0.0f ) {
    cosom = -cosom; 
    afto1[0] = -to[0];
    afto1[1] = -to[1];
    afto1[2] = -to[2];
    afto1[3] = -to[3];
  } else {
    afto1[0] = to[0];
    afto1[1] = to[1];
    afto1[2] = to[2];
    afto1[3] = to[3];
  }
  
  const float QUATERNION_DELTA_COS_MIN = 0.01f;

  if ( (1.0f - cosom) > QUATERNION_DELTA_COS_MIN ) {
    /* This is a standard case (slerp). */
    float omega = acosf(cosom);
    float sinom = sinf(omega);
    scale0 = sinf((1.0f - amount) * omega) / sinom;
    scale1 = sinf(amount * omega) / sinom;
  } else {
    /* "from" and "to" quaternions are very close */
    /*  so we can do a linear interpolation.      */
    scale0 = 1.0f - amount;
    scale1 = amount;
  }

  r[0] = (scale0 * from[0]) + (scale1 * afto1[0]);
  r[1] = (scale0 * from[1]) + (scale1 * afto1[1]);
  r[2] = (scale0 * from[2]) + (scale1 * afto1[2]);
  r[3] = (scale0 * from[3]) + (scale1 * afto1[3]);
}

#endif
