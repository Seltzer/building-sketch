#ifndef VEC_INL
#define VEC_INL

#include <assert.h>

//Constructors

template<class T, int S>
template<class U>
inline vec<T, S>::vec(const vec<U, S>& parent)
{
	for (int i = 0; i < S; i++)
		data[i] = parent.data[i];
}

template<class T, int S>
inline vec<T, S>::vec(const T init)
{
	for (int i = 0; i < S; i++)
		data[i] = init;
}

template<class T, int S>
inline vec<T, S>::vec(const T init[S])
{
	for (int i = 0; i < S; i++)
		data[i] = init[i];
}




//Member functions

template<class T, int S>
inline T& vec<T, S>::x()
{
	assert(S >= 1);
	return data[0];
}

template<class T, int S>
inline const T& vec<T, S>::x() const
{
	assert(S >= 1);
	return data[0];
}

template<class T, int S>
inline T& vec<T, S>::y()
{
	assert(S >= 2);
	return data[1];
}

template<class T, int S>
inline const T& vec<T, S>::y() const
{
	assert(S >= 2);
	return data[1];
}

template<class T, int S>
inline T& vec<T, S>::z()
{
	assert(S >= 3);
	return data[2];
}

template<class T, int S>
inline const T& vec<T, S>::z() const
{
	assert(S >= 3);
	return data[2];
}

template<class T, int S>
inline T& vec<T, S>::w()
{
	assert(S >= 4);
	return data[3];
}

template<class T, int S>
inline const T& vec<T, S>::w() const
{
	assert(S >= 4);
	return data[3];
}

template<class T, int S>
inline T& vec<T, S>::r()
{
	assert(S >= 1);
	return data[0];
}

template<class T, int S>
inline const T& vec<T, S>::r() const
{
	assert(S >= 1);
	return data[0];
}

template<class T, int S>
inline T& vec<T, S>::g()
{
	assert(S >= 2);
	return data[1];
}

template<class T, int S>
inline const T& vec<T, S>::g() const
{
	assert(S >= 2);
	return data[1];
}

template<class T, int S>
inline T& vec<T, S>::b()
{
	assert(S >= 3);
	return data[2];
}

template<class T, int S>
inline const T& vec<T, S>::b() const
{
	assert(S >= 3);
	return data[2];
}

template<class T, int S>
inline T& vec<T, S>::a()
{
	assert(S >= 4);
	return data[3];
}

template<class T, int S>
inline const T& vec<T, S>::a() const
{
	assert(S >= 4);
	return data[3];
}


template<class T, int S>
inline T vec<T, S>::length() const
{
	assert(S >= 1); //Man, lame optimisation. Save an assignement and an addition. Yay.

	T out = data[0] * data[0];
	for (int i = 1; i < S; i++)
		out += data[i] * data[i];
	return sqrtf(out);
}

template<class T, int S>
inline T vec<T, S>::sqlength() const
{
	assert(S >= 1); //Man, lame optimisation. Save an assignement and an addition. Yay.

	T out = data[0] * data[0];
	for (int i = 1; i < S; i++)
		out += data[i] * data[i];
	return out;
}

template<class T, int S>
inline void vec<T, S>::normalise()
{
	double invLeng = 1.0 / length();
	for (int i = 0; i < S; i++)
		data[i] *= invLeng;
}

#ifdef VEC_TOSTRING
template<class T, int S>
inline std::string vec<T, S>::tostring() const
{
	assert(S >= 1); //Is simpler to implement assuming vector has at least 1 component

	std::stringstream strm;
	strm << '(' << data[0]; //Stream in (x
	for (int i = 1; i < S; i++)
		strm << ", " << data[i]; //Stream in , n
	strm << ')'; //Stream in )

	return strm.str();
}
#endif //VEC_TOSTRING

template<class T, int S>
inline __m128 vec<T, S>::getSSE()
{
	__m128 sse;
	for (int i = 0; i < S; i++)
		((float*)&sse)[i] = data[i];
	return sse;
}


//Member operators

template<class T, int S>
template<class U>
inline vec<T, S>& vec<T, S>::operator=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] = RHS[i];

	return *this;
}

template<class T, int S>
template<class U>
inline vec<T, S>& vec<T, S>::operator=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] = RHS;

	return *this;
}

template<class T, int S>
inline T& vec<T, S>::operator[](int Index)
{
	assert(Index >= 0); //Bounds checking
	assert(Index < S);

	return data[Index];
}

template<class T, int S>
inline const T& vec<T, S>::operator[](int Index) const
{
	assert(Index >= 0); //Bounds checking
	assert(Index < S);

	return data[Index];
}


//Unary operators

//Addition
template<class T, int S>
template<class U>
inline void vec<T, S>::operator+=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] += RHS[i];
}

template<class T, int S>
template<class U>
inline void vec<T, S>::operator+=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] += RHS;
}

//Multiplication
template<class T, int S>
template<class U>
inline void vec<T, S>::operator*=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] *= RHS[i];
}

template<class T, int S>
template<class U>
inline void vec<T, S>::operator*=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] *= RHS;
}

//Subtraction
template<class T, int S>
template<class U>
inline void vec<T, S>::operator-=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] -= RHS[i];
}

template<class T, int S>
template<class U>
inline void vec<T, S>::operator-=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] -= RHS;
}

//Division
template<class T, int S>
template<class U>
inline void vec<T, S>::operator/=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] /= RHS[i];
}

template<class T, int S>
template<class U>
inline void vec<T, S>::operator/=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] /= RHS;
}

//Negation?
template<class T, int S>
inline vec<T, S> vec<T, S>::operator-() const
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out[i] = -data[i];
	return out;
}

#endif //VEC_INL
