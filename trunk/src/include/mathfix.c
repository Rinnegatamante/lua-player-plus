#include <math.h>

#include "mathfix.h"

#define PI   3.14159265358979f
#define PI_2 1.57079632679489f

#define radToDeg(x) ((x)*180.f/PI)
#define degToRad(x) ((x)*PI/180.f)

float MathAbs(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vabs.s   S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));

	return result;
}

float MathCeil(float x)
{
	float result;
	
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2iu.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

float MathFloor(float x)
{
	float result;
	
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2id.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

float MathAtan(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vmul.s   S001, S000, S000\n"
		"vadd.s   S001, S001, S001[1]\n"
		"vrsq.s   S001, S001\n"
		"vmul.s   S000, S000, S001\n"
		"vasin.s  S000, S000\n"
		"vcst.s   S001, VFPU_PI_2\n"
		"vmul.s   S000, S000, S001\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));

	return result;
}

float MathAtan2(float y, float x)
{
	float r;

	if (MathAbs(x) >= MathAbs(y))
	{
		r = MathAtan(y/x);

		if (x < 0.0f)
			r += (y >= 0.0f ? PI : -PI);
	}
	else
	{
		r = -MathAtan(x/y);
		r += (y < 0.0f ? -PI_2 : PI_2);
	}

	return r;
}

float MathSqrt(float x)
{
	float result;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vsqrt.s S000, S000\n"
		"mfv     %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

float MathCos(float rad)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_2_PI\n"
        "vmul.s  S000, S000, S001\n"
        "vcos.s  S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(rad));
    return result;
}

float MathSin(float rad)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_2_PI\n"
        "vmul.s  S000, S000, S001\n"
        "vsin.s  S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(rad));
    return result;
}

float MathAcos(float x)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_PI_2\n"
        "vasin.s S000, S000\n"
        "vocp.s  S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float MathAsin(float x)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_PI_2\n"
        "vasin.s S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float MathCosh(float x)
{
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmov.s   S002, S000[|x|]\n"
		"vmul.s   S002, S001, S002\n"
		"vexp2.s  S002, S002\n"
		"vrcp.s   S003, S002\n"
		"vadd.s   S002, S002, S003\n"
		"vmul.s   S002, S002, S002[1/2]\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x));
	return result;
}

float MathExp(float x)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_LN2\n"
        "vrcp.s  S001, S001\n"
        "vmul.s  S000, S000, S001\n"
        "vexp2.s S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float MathFmax(float x, float y)
{
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vmax.s   S002, S000, S001\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

float MathFmin(float x, float y)
{
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vmin.s   S002, S000, S001\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

float MathFmod(float x, float y)
{
	float result;
	// return x-y*((int)(x/y));
	__asm__ volatile (
		"mtv       %2, S001\n"
		"mtv       %1, S000\n"
		"vrcp.s    S002, S001\n"
		"vmul.s    S003, S000, S002\n"
		"vf2iz.s   S002, S003, 0\n"
		"vi2f.s    S003, S002, 0\n"
		"vmul.s    S003, S003, S001\n"
		"vsub.s    S000, S000, S003\n"
		"mfv       %0, S000\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

float MathLog(float x)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_LOG2E\n"
        "vrcp.s  S001, S001\n"
        "vlog2.s S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float MathLog2(float x)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vlog2.s S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
	
    return result;
}

float MathLog10(float x)
{
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_LOG2TEN\n"
        "vrcp.s  S001, S001\n"
        "vlog2.s S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float MathPow(float x, float y)
{
	float result;
	// result = exp2f(y * log2f(x));
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vlog2.s  S001, S001\n"
		"vmul.s   S000, S000, S001\n"
		"vexp2.s  S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

float MathPow2(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vexp2.s  S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

float MathTrunc(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2iz.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

float MathRound(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2in.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

void MathSrand(unsigned int x)
{
	__asm__ volatile (
		"mtv %0, S000\n"
		"vrnds.s S000"
		: "=r"(x));
}

float MathRandFloat(float min, float max)
{
    float result;
    __asm__ volatile (
		"mtv      %1, S000\n"
        "mtv      %2, S001\n"
        "vsub.s   S001, S001, S000\n"
        "vrndf1.s S002\n"
        "vone.s	  S003\n"
        "vsub.s   S002, S002, S003\n"
        "vmul.s   S001, S002, S001\n"
        "vadd.s   S000, S000, S001\n"
        "mfv      %0, S000\n"
        : "=r"(result) : "r"(min), "r"(max));
    return result;
}

int MathRandInt(float min, float max)
{
    float result = MathRandFloat(min, max);
	
	return (int)result;
}

void MathSincos(float r, float *s, float *c)
{
	__asm__ volatile (
		"mtv      %2, S002\n"
		"vcst.s   S003, VFPU_2_PI\n"
		"vmul.s   S002, S002, S003\n"
		"vrot.p   C000, S002, [s, c]\n"
		"mfv      %0, S000\n"
		"mfv      %1, S001\n"
	: "=r"(*s), "=r"(*c): "r"(r));
}

float MathSinh(float x)
{
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmov.s   S002, S000[|x|]\n"
		"vcmp.s   NE, S000, S002\n"
        "vmul.s   S002, S001, S002\n"
        "vexp2.s  S002, S002\n"
        "vrcp.s   S003, S002\n"
        "vsub.s   S002, S002, S003\n"
        "vmul.s   S002, S002, S002[1/2]\n"
        "vcmov.s  S002, S002[-x], 0\n"
        "mfv      %0, S002\n"
	: "=r"(result) : "r"(x));
	return result;
}

float MathTan(float x)
{
	float result;
	// result = sin(x)/cos(x);
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_2_PI\n"
        "vmul.s   S000, S000, S001\n"
        "vrot.p   C002, S000, [s, c]\n"
        "vdiv.s   S000, S002, S003\n"
        "mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

float MathTanh(float x)
{
	float result;
	//y = exp(x+x);
	//return (y-1)/(y+1);
	__asm__ volatile (
		"mtv      %0, S000\n"
		"vadd.s   S000, S000, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmul.s   S000, S000, S001\n"
        "vexp2.s  S000, S000\n"
        "vone.s   S001\n"
        "vbfy1.p  C002, C000\n"
        "vdiv.s   S000, S003, S002\n"
        "mfv      %0, S000\n"
	: "=r"(result): "r"(x));
	return result;
}

float MathInvSqrt(float x)
{
	float result;
	
	// return 1.0f/sqrtf(x);
	
	__asm__ volatile (
		"mtv		%0, S000\n"
		"vrsq.s		S000, S000\n"
		"mfv		%0, S000\n"
	: "=r"(result): "r"(x));
	return result;
}

float MathDegToRad(float x)
{
     return degToRad(x);
}

float MathRadToDeg(float x)
{
     return radToDeg(x);
}

ScePspFVector3 *MathVecAdd3f(ScePspFVector3 *result, const ScePspFVector3 *vec1, const ScePspFVector3 *vec2)
{	
	/*result->x = vec1->x + vec2->x;
	result->y = vec1->y + vec2->y;
	result->z = vec1->z + vec2->z;*/
	
	__asm__ volatile(
		".set			push\n"
		".set			noreorder\n"
		"lv.s			S000, 0 + %1\n"
		"lv.s			S001, 4 + %1\n"
		"lv.s			S002, 8 + %1\n"
		"lv.s			S010, 0 + %2\n"
		"lv.s			S011, 4 + %2\n"
		"lv.s			S012, 8 + %2\n"
		"vadd.t			C000, C000, C010\n"
		"sv.s			S000, 0 + %0\n"
		"sv.s			S001, 4 + %0\n"
		"sv.s			S002, 8 + %0\n"
		".set			pop\n"
		: "=m"(*result) : "m"(*vec1), "m"(*vec2));
	
	return result;
}

ScePspFVector3 *MathVecSub3f(ScePspFVector3 *result, const ScePspFVector3 *vec1, const ScePspFVector3 *vec2)
{	
	/*result->x = vec1->x - vec2->x;
	result->y = vec1->y - vec2->y;
	result->z = vec1->z - vec2->z;*/
	
	__asm__ volatile(
		".set			push\n"
		".set			noreorder\n"
		"lv.s			S000, 0 + %1\n"
		"lv.s			S001, 4 + %1\n"
		"lv.s			S002, 8 + %1\n"
		"lv.s			S010, 0 + %2\n"
		"lv.s			S011, 4 + %2\n"
		"lv.s			S012, 8 + %2\n"
		"vsub.t			C000, C000, C010\n"
		"sv.s			S000, 0 + %0\n"
		"sv.s			S001, 4 + %0\n"
		"sv.s			S002, 8 + %0\n"
		".set			pop\n"
		: "=m"(*result) : "m"(*vec1), "m"(*vec2));
	
	return result;
}

int MathVecCompare3f(const ScePspFVector3 *vec1, const ScePspFVector3 *vec2)
{
	return(vec1->x == vec2->x && vec1->y == vec2->y && vec1->z == vec2->z);
}

ScePspFVector3 *MathVecScale3f(ScePspFVector3 *result, const ScePspFVector3 *vec, float scalar)
{
	/*ScePspFVector3 w;
	w.x = u.x*scalar;
	w.y = u.y*scalar;
	//w.z = u.z*scalar;
	w.z = 0.0f;
	return w;*/
	
	__asm__ volatile (
		".set			push\n"
		".set			noreorder\n"
		"mfc1			$8,   %2\n"
		"mtv			$8,   S010\n"
		"lv.s			S000, 0 + %1\n"
		"lv.s			S001, 4 + %1\n"
		"lv.s			S002, 8 + %1\n"
		"vscl.t			C000, C000, S010\n"
		"sv.s			S000, 0 + %0\n"
		"sv.s			S001, 4 + %0\n"
		"sv.s			S002, 8 + %0\n"
		".set			pop\n"
		: "=m"(*result) : "m"(*vec), "f"(scalar): "$8");
		
	return result;
}

float MathVecDot3f(const ScePspFVector3 *vec1, const ScePspFVector3 *vec2)
{
	//return(vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z);
	
	float result;
	
	__asm__ volatile (
		".set			push\n"
		".set			noreorder\n"
		"lv.s			S000, 0 + %1\n"
		"lv.s			S001, 4 + %1\n"
		"lv.s			S002, 8 + %1\n"
		"lv.s			s010, 0 + %2\n"
		"lv.s			s011, 4 + %2\n"
		"lv.s			s012, 8 + %2\n"
		"vdot.t			S000, C000, C010\n"
		"sv.s			S000, %0\n"
		".set			pop\n"
		: "=m"(result) : "m"(*vec1), "m"(*vec2));

	return result;
	
}

float MathVecLength3f(const ScePspFVector3 *vec)
{
	//return(MathSqrtf(MathVecDot3f(u, u)));
	
	float result;
	
	__asm__ volatile (
		".set			push\n"
		".set			noreorder\n"
		"lv.s			S000, 0 + %1\n"
		"lv.s			S001, 4 + %1\n"
		"lv.s			S002, 8 + %1\n"
		"vdot.t			S000, C000, C000\n"
		"vsqrt.s		S000, S000\n"
		"sv.s			S000, %0\n"
		".set			pop\n"
		: "=m"(result) : "m"(*vec));

	return result;
}

ScePspFVector3 *MathVecNormalize3f(ScePspFVector3 *result, const ScePspFVector3 *vec)
{
/*
	ScePspFVector3 w;
	float leninv;

	if(!(u->x == 0.0f && u->y == 0.0f && u->z == 0.0f))
	{
		leninv = MathVecLengthInv3f(*u);

		u->x *= leninv;
		u->y *= leninv;
		u->z *= leninv;
	}
*/
	__asm__ volatile(
		".set			push\n"
		".set			noreorder\n"
		"lv.s			S000, 0 + %1\n"
		"lv.s			S001, 4 + %1\n"
		"lv.s			S002, 8 + %1\n"
		"vdot.t			S010, C000, C000\n"
		"vzero.s		S011\n"
		"vcmp.s			EZ, S010\n"
		"vrsq.s			S010, S010\n"
		"vcmovt.s		S010, S011, 0\n"
		"vscl.t			C000[-1:1,-1:1,-1:1], C000, S010\n"
		"sv.s			S000, 0 + %0\n"
		"sv.s			S001, 4 + %0\n"
		"sv.s			S002, 8 + %0\n"
		".set			pop\n"
		: "=m"(*result) : "m"(*vec));
		
	return result;
}

float MathVecAngle3f(const ScePspFVector3 *vec1, const ScePspFVector3 *vec2)
{
	ScePspFVector3 s;
	ScePspFVector3 t;
	
	MathVecNormalize3f(&s, vec1);
	
	MathVecNormalize3f(&t, vec2);
	
	return MathAcos(MathVecDot3f(&s, &t));
}

ScePspFVector3 *MathVecRotateZ3f(const ScePspFVector3 *vec, float a, ScePspFVector3 *result)
{
	result->x = vec->x * MathCos(a) - vec->y * MathSin(a);
	result->y = vec->x * MathSin(a) + vec->y * MathCos(a);
	result->z = vec->z;
	
	return result;
}

void MathRectClear(Rect2 *rect)
{
	rect->clean = 1;
}

void MathRectSetRadius(Rect2 *rect, float x, float y, float r)
{
	rect->x1 = x-r;
	rect->x2 = x+r;
	rect->y1 = y-r;
	rect->y2 = y+r;
	rect->clean = 0;
}

void MathRectEncapsulate(Rect2 *rect, float x, float y)
{
	if(rect->clean)
	{
		rect->x1 = rect->x2 = x;
		rect->y1 = rect->y2 = y;
		rect->clean = 0;
	}
	else
	{
		if(x < rect->x1) rect->x1 = x;
		if(x > rect->x2) rect->x2 = x;
		if(y < rect->y1) rect->y1 = y;
		if(y > rect->y2) rect->y2 = y;
	}
}

int MathRectTestPoint(Rect2 *rect, float x, float y)
{
	if(x>=rect->x1 && x<rect->x2 && y>=rect->y1 && y<rect->y2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int MathRectIntersect(Rect2 *rect1, Rect2 *rect2)
{
	if(fabsf(rect1->x1 + rect1->x2 - rect2->x1 - rect2->x2) < (rect1->x2 - rect1->x1 + rect2->x2 - rect2->x1))
	{
		if(fabsf(rect1->y1 + rect1->y2 - rect2->y1 - rect2->y2) < (rect1->y2 - rect1->y1 + rect2->y2 - rect2->y1))
			return 1;
	}
	
	return 0;
}