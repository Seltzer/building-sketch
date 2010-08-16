#ifndef VEC_H
#define VEC_H

//This is a vector (as in trig, not std::vector) class that supports any number
//of any numerical type. Type and number are specified as template parameters
//like so: vec<float, 3> myVec; Where type must be numerical. Functions and
//operators only work on vectors of the same size. But they do work on vectors
//of different types. Be aware, though, that an operation will always return
//the type of the left hand operand.

#include <cmath>
#ifdef VEC_TOSTRING //Optional to avoid unnescessary include.
#include <string>
#include <sstream>
#endif //VEC_TOSTRING

#include <xmmintrin.h>

template<class T, int S> //Type to hold and size of vector
class vec
{
public:
	inline vec() {}
	template<class U>
	inline vec(const vec<U, S>& parent);
	inline explicit vec(const T init); //Sets all components to this for ease of initialisation (to 0 mostly).
	inline vec(const T init[S]);
	inline ~vec() {}

	//Member functions
	inline T& x(); //Accessors. If you don't like the function access, use the specialisations.
	inline const T& x() const;
	inline T& y();
	inline const T& y() const;
	inline T& z();
	inline const T& z() const;
	inline T& w();
	inline const T& w() const;
	inline T& r();
	inline const T& r() const;
	inline T& g();
	inline const T& g() const;
	inline T& b();
	inline const T& b() const;
	inline T& a();
	inline const T& a() const;
	inline T length() const;
	inline T sqlength() const; //Returns length squared (fast).
	inline void normalise();
	#ifdef VEC_TOSTRING
	inline std::string tostring() const; //Returns vector as string like: (x, y, z)
	#endif //VEC_TOSTRING
	inline __m128 getSSE();

	//Member operators
	template<class U>
	inline vec<T, S>& operator=(const vec<U, S>& RHS);
	template<class U>
	inline vec<T, S>& operator=(const U RHS);
	inline T& operator[](int Index);
	inline const T& operator[](int Index) const; //Constant version in case vector itself is constant.

	//Unary operators
	//Addition
	template<class U> //Type of right hand side
	inline void operator+=(const vec<U, S>& RHS);
	template<class U>
	inline void operator+=(const U RHS);
	//Multiplication
	template<class U>
	inline void operator*=(const vec<U, S>& RHS);
	template<class U>
	inline void operator*=(const U RHS);
	//Subtraction
	template<class U>
	inline void operator-=(const vec<U, S>& RHS);
	template<class U>
	inline void operator-=(const U RHS);
	//Division
	template<class U>
	inline void operator/=(const vec<U, S>& RHS);
	template<class U>
	inline void operator/=(const U RHS);
	//Negation?
	inline vec<T, S> operator-() const;

	T data[S]; //I would make it private, but that would mean the operators need
	           //to be friends, and that means the templates get a bit weirder
	           //and don't always compile.
};

#include "vec.inl"

#include "vecops.inl"

#endif //VEC_H
