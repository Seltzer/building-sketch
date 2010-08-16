#ifndef TYPES_H
#define TYPES_H

const double D_PI = 3.14159265358979323846264; // How hard is it to find a pi constant around here?

#define VEC_TOSTRING

#include "vec.h"
#define S 2
#include "vecspec.inl"
#undef S
#define S 3
#include "vecspec.inl"
#undef S
#define S 4
#include "vecspec.inl"
#undef S

//#include "Matrix.h"

// Macros for defining types such as float3 as vec<float, 3>
#define VECTYPEDEF(type)							\
	typedef vec<type, 2> type##2;					\
	typedef vec<type, 3> type##3;					\
	typedef vec<type, 4> type##4;

#define VECTYPEDEF_SIGN(type)						\
	typedef vec<type, 2> type##2;					\
	typedef vec<unsigned type, 2> u##type##2;		\
	typedef vec<type, 3> type##3;					\
	typedef vec<unsigned type, 3> u##type##3;		\
	typedef vec<type, 4> type##4;					\
	typedef vec<unsigned type, 4> u##type##4;

// Define simple names for common vector data types and sizes
VECTYPEDEF(float);
VECTYPEDEF(double);
VECTYPEDEF_SIGN(int);
VECTYPEDEF_SIGN(long);
VECTYPEDEF_SIGN(short);
VECTYPEDEF_SIGN(char);

#undef VECTYPEDEF
#undef VECTYPEDEF_SIGN

typedef uchar4 Colour;

#endif //TYPES_H
