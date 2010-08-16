#ifndef VECOPS_INL
#define VECOPS_INL

#include <assert.h>

//Dot product
template<class T, class U, int S>
inline T dot(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	assert(S >= 1); //Slight optimisation means it only works with 1 or more component vectors.

	T out = LHS[0] * RHS[0]; //Avoids assign to 0 and you shouldn't dot nothing
	for (int i = 1; i < S; i++)
		out += LHS[i] * RHS[i];
	return out;
}

//Cross product
template<class T, class U, int S>
inline vec<T, S> cross(const vec<T, S>& LHS, const vec<U, S>& RHS) //Only really applies to 3d vectors
{
	assert(S >= 3);

	vec<T, S> out;
	out[0] = LHS[1] * RHS[2] - RHS[1] * LHS[2];
	out[1] = LHS[2] * RHS[0] - RHS[2] * LHS[0];
	out[2] = LHS[0] * RHS[1] - RHS[0] * LHS[1];
	return out;
}

//Get normalised vector
template<class T, int S>
inline vec<T, S> normal(const vec<T, S>& RHS)
{
	assert(S >= 1);

	T sqLength = RHS[0] * RHS[0];
	for (int i = 1; i < S; i++)
		sqLength += RHS[i] * RHS[i];
	double invLength = 1.0 / sqrt(sqLength); //Sacrificing performance for predictable behaviour...

	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out[i] = static_cast<T>(RHS[i] * invLength);
	return out;
}

//Component wise absolute value
template<class T, int S>
inline vec<T, S> abs(vec<T, S> in)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out[i] = abs((T)in[i]);
	return out;
}

//Component wise sqrt
template<class T, int S>
inline vec<T, S> vecSqrt(vec<T, S> in)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out[i] = sqrtf((T)in[i]);
	return out;
}

//Comparison operators

//Equality
template<class T, class U, int S>
inline bool operator==(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	bool result = true;
	for (int i = 0; i < S; i++)
		result = result && (LHS.data[i] == RHS.data[i]);
	return result;
}


//Binary operators

//Addition
template<class T, class U, int S>
inline vec<T, S> operator+(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] + RHS.data[i];
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator+(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] + RHS;
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator+(const T LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS + RHS.data[i];
	return out;
}

//Multiplication
template<class T, class U, int S>
inline vec<T, S> operator*(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] * RHS.data[i];
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator*(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] * RHS;
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator*(const T LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS * RHS.data[i];
	return out;
}

//Subtraction
template<class T, class U, int S>
inline vec<T, S> operator-(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] - RHS.data[i];
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator-(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] - RHS;
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator-(const T LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS - RHS.data[i];
	return out;
}

//Division
template<class T, class U, int S>
inline vec<T, S> operator/(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] / RHS.data[i];
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator/(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS.data[i] / RHS;
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator/(const T LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS / RHS.data[i];
	return out;
}


//Shift
template<class T, class U, int S>
inline vec<T, S> operator>>(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS[i] >> RHS;
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator>>(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS[i] >> RHS[i];
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator<<(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS[i] << RHS;
	return out;
}

template<class T, class U, int S>
inline vec<T, S> operator<<(const vec<T, S>& LHS, const vec<U, S>& RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS[i] << RHS[i];
	return out;
}

//Modulo
template<class T, class U, int S>
inline vec<T, S> operator%(const vec<T, S>& LHS, const U RHS)
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out.data[i] = LHS[i] % RHS;
	return out;
}

#endif //VECOPS_INL
