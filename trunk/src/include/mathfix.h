#ifndef __MATH_H__
#define __MATH_H__

/** @defgroup Math VFPU Math Library
 *  @{
 */
#include <psptypes.h>

/**
* Another rect datatype
*/
typedef struct
{
	float	x1, y1, x2, y2;
	int clean;
} Rect2;

/**
 * Calculate absolute value
 *
 * @param x - Input
 */
float MathAbs(float x);

/**
 * Round value up
 *
 * @param x - Input
 */
float MathCeil(float x);

/**
 * Round value down
 *
 * @param x - Input
 */
float MathFloor(float x);

/**
 * Calculate inverse tangent (arctan)
 *
 * @param x - Input in radians
 */
float MathAtan(float x);

/**
 * Calculate inverse tangent, with quadrant fix-up
 *
 * @param y - Input in radians
 *
 * @param x - Input in radians
 */
float MathAtan2(float y, float x);

/**
 * Calculate square root
 *
 * @param x - Input
 */
float MathSqrt(float x);

/**
 * Calculate cosine
 *
 * @param x - Input in radians
 */
float MathCos(float rad);

/**
 * Calculate sine
 *
 * @param x - Input in radians
 */
float MathSin(float rad);

/**
 * Calculate inverse cosine (arccos)
 *
 * @param x - Input in radians
 */
float MathAcos(float x);

/**
 * Calculate inverse sine (arcsin)
 *
 * @param x - Input in radians
 */
float MathAsin(float x);

/**
 * Calculate hyperbolic cosine
 *
 * @param x - Input in radians
 */
float MathCosh(float x);

/**
 * Calculate exponent
 *
 * @param x - Input in radians
 */
float MathExp(float x);

/**
 * Calculate maximum numeric value
 *
 * @param x - Input
 *
 * @param y - Input
 */
float MathFmax(float x, float y);

/**
 * Calculate minimum numeric value
 *
 * @param x - Input
 *
 * @param y - Input
 */
float MathFmin(float x, float y);

/**
 * Calculate floating point remainder of x/y
 *
 * @param x - Input
 *
 * @param y - Input
 */
float MathFmod(float x, float y);

/**
 * Calculate natural logarithm
 *
 * @param x - Input in radians
 */
float MathLog(float x);

/**
 * Calculate base 2 logarithm
 *
 * @param x - Input in radians
 */
float MathLog2(float x);

/**
 * Calculate base 10 logarithm
 *
 * @param x - Input in radians
 */
float MathLog10(float x);

/**
 * Calculate x raised to the power of y
 *
 * @param x - Number to raise power of
 *
 * @param y - Power to raise x by
 */
float MathPow(float x, float y);

/**
 * Calculate 2 raised to the power of x
 *
 * @param x - Input
 */
float MathPow2(float x);

/**
 * Round to nearest value
 *
 * @param x - Input
 */
float MathRound(float x);

/**
 * Round towards 0
 *
 * @param x - Input
 */
float MathTrunc(float x);

/**
 * Set random generator seed
 *
 * @param x - Seed value
 */
void MathSrand(unsigned int x);

/**
 * Generate random float value
 *
 * @param min - Minimum value to return
 *
 * @param max - Maximum value to return
 *
 * @returns A value between min and max
 */
float MathRandFloat(float min, float max);

/**
 * Generate random int value
 *
 * @param min - Minimum value to return
 *
 * @param max - Maximum value to return
 *
 * @returns A value between min and max
 */
int MathRandInt(float min, float max);

/**
 * Calculate sine and cosine
 *
 * @param r - Input in radians
 *
 * @param s - Pointer to a float for sin result
 *
 * @param c - pointer to a float for cos result
*/
void MathSincos(float r, float *s, float *c);

/**
 * Calculate hyperbolic sine
 *
 * @param x - Input in radians
 */
float MathSinh(float x);

/**
 * Calculate tangent
 *
 * @param x - Input in radians
 */
float MathTan(float x);

/**
 * Calculate hyperbolic tangent
 *
 * @param x - Input in radians
 */
float MathTanh(float x);

/**
* Calculate inverse square root (1/sqrt(x))
*
* @param x - Input value
*/
float MathInvSqrt(float x);

/**
* Calculate radian angle from euler angle
*
* @param x - Input value in degrees
*/
float MathDegToRad(float x);

/**
* Calculate euler angle from radian angle
*
* @param x - Input value in degrees
*/
float MathRadToDeg(float x);

/**
* Normalize an ::ScePSPFVector3
*
* @param result - pointer to an ::ScePSPFVector3 the result gets stored in
*
* @param vec - pointer to an ::ScePSPFVector3 to be normalized
*
* @returns a pointer to an ::ScePSPFVector3 as result
*/
ScePspFVector3 *MathVecNormalize3f(ScePspFVector3 *result, const ScePspFVector3 *vec);

/**
* Add two ::ScePSPFVector3
*
* @param result - pointer to an ::ScePSPFVector3 the result gets stored in
*
* @param vec1 - pointer to an ::ScePSPFVector3 to be added
*
* @param vec2 - pointer to an ::ScePSPFVector3 to be added
*
* @returns a pointer to an ::ScePSPFVector3 as result
*/
ScePspFVector3 *MathVecAdd3f(ScePspFVector3 *result, const ScePspFVector3 *vec1, const ScePspFVector3 *vec2);

/**
* Subtract a ::ScePSPFVector3 from another ::ScePSPFVector3
*
* @param result - pointer to an ::ScePSPFVector3 the result gets stored in
*
* @param vec1 - pointer to an ::ScePSPFVector3 as minuend
*
* @param vec2 - pointer to an ::ScePSPFVector3 as subtrahend
*
* @returns a pointer to an ::ScePSPFVector3 as result
*/
ScePspFVector3 *MathVecSub3f(ScePspFVector3 *result, const ScePspFVector3 *vec1, const ScePspFVector3 *vec2);

/**
* Compare two ::ScePSPFVector3
*
* @param vec1 - pointer to an ::ScePSPFVector3 for comparison
*
* @param vec2 - pointer to an ::ScePSPFVector3 for comparison
*
* @returns 1 if vec1 == vec2, else 0
*/
int MathVecCompare3f(const ScePspFVector3 *vec1, const ScePspFVector3 *vec2);

/**
* Scale a ::ScePSPFVector3 by a scalar
*
* @param result - pointer to an ::ScePSPFVector3 the result gets stored in
*
* @param vec - pointer to an ::ScePSPFVector3 for multiplication
*
* @param scalar - float for multiplication
*
* @returns a pointer to an ::ScePSPFVector3 as result
*/
ScePspFVector3 *MathVecScale3f(ScePspFVector3 *result, const ScePspFVector3 *vec, float scalar);

/**
* Calculate the dot product of two ::ScePSPFVector3
*
* @param vec1 - pointer to an ::ScePSPFVector3 as Input
*
* @param vec2 - pointer to an ::ScePSPFVector3 as Input
*
* @returns float result of the dot product of vec1 and vec2
*/
float MathVecDot3f(const ScePspFVector3 *vec1, const ScePspFVector3 *vec2);

/**
* Calculate the length of a ::ScePSPFVector3
*
* @param vec - pointer to an ::ScePSPFVector3 as Input
*
* @returns float result of the length calculation of vec
*/
float MathVecLength3f(const ScePspFVector3 *vec);

/**
* Calculate the angle between two ::ScePSPFVector3
*
* @param vec1 - pointer to an ::ScePSPFVector3 as Input
*
* @param vec2 - pointer to an ::ScePSPFVector3 as Input
*
* @returns float result of the dot product of vec1 and vec2
*/
float MathVecAngle3f(const ScePspFVector3 *vec1, const ScePspFVector3 *vec2);

/**
* Rotate a ::ScePSPFVector3 about the Z-axis
*
* @param vec - pointer to an ::ScePSPFVector3 as Input
*
* @param a - float Input angle in radians
*
* @param result - pointer to an ::ScePSPFVector3 the result gets stored in
*
* @returns pointer to the ::ScePSPFVector3 result
*/
ScePspFVector3 *MathVecRotateZ3f(const ScePspFVector3 *vec, float a, ScePspFVector3 *result);

/**
* Clear a ::Rect2
*
* @param rect - pointer to a ::Rect2 as Input
*/
void MathRectClear(Rect2 *rect);

/**
* Set the coordinates of a ::Rect2
*
* @param rect - pointer to a ::Rect2 to be processed
*
* @param x - x coordinate of the center
*
* @param y - y coordinate of the center
*
* @param r - radius of the rect's inner circle
*/
void MathRectSetRadius(Rect2 *rect, float x, float y, float r);

/**
* Encapsulate a point into a ::Rect2
*
* @param rect - pointer to a ::Rect2 to be processed
*
* @param x - float x coordinate
*
* @param y - float y coordinate
*/
void MathRectEncapsulate(Rect2 *rect, float x, float y);

/**
* Test if a point lies within a ::Rect2
*
* @param rect - pointer to a ::Rect2 as Input
*
* @param x - float x coordinate
*
* @param y - float y coordinate
*
* @returns int 1 if x,y is withing rect, else 0
*/
int MathRectTestPoint(Rect2 *rect, float x, float y);

/**
* Test if two ::Rect2 intersect
*
* @param rect1 - pointer to a ::Rect2 as Input
*
* @param rect2 - pointer to a ::Rect2 as Input
*
* @returns int 1 if rect1 intersects rect2, else 0
*/
int MathRectIntersect(Rect2 *rect1, Rect2 *rect2);

/** @} */

#endif // __MATH_H__