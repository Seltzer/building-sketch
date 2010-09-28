// No #include guard for a reason

// DO NOT INCLUDE THIS FILE! 
// This file constains most of the member functions needed for a specialisation
// of vec. So, it has to be included once for each specialisation, with S defined
// to the number of dimensions the particular specialisation has. 

#if !defined(S) || S < 1
	#error A vector must have at least 1 dimension
#endif

template<class T>
class vec<T, S>
{
public:
	inline vec() {}
	template<class U>
	inline vec(const vec<U, S>& parent);
	inline vec(const T init);
#if S == 2
	inline vec(const T a, const T b);
#endif
#if S == 3
	inline vec(const T a, const T b, const T c);
#endif
#if S == 4
	inline vec(const T a, const T b, const T c, const T d);
#endif
	inline vec(const T init[S]);
	inline ~vec() {}

	//Member functions
	inline T length() const;
	inline T sqlength() const;
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

	union
	{
		struct
		{
			T x;
#if S >= 2 
			T y;
#endif
#if S >= 3
			T z;
#endif
#if S >= 4
			T w;
#endif
		};
		struct
		{
			T r;
#if S >= 2
			T g;
#endif
#if S >= 3
			T b;
#endif
#if S >= 4
			T a;
#endif
		};
		T data[S];
	};
};

template<class T>
template<class U>
inline vec<T, S>::vec(const vec<U, S>& parent)
{
	for (int i = 0; i < S; i++)
		data[i] = parent.data[i];
}

template<class T>
inline vec<T, S>::vec(const T init)
{
	for (int i = 0; i < S; i++)
		data[i] = init;
}

template<class T>
inline vec<T, S>::vec(const T init[S])
{
	for (int i = 0; i < S; i++)
		data[i] = init[i];
}

template<class T>
#if S == 2
inline vec<T, 2>::vec(const T x, const T y)
#endif
#if S == 3
inline vec<T, 3>::vec(const T x, const T y, const T z)
#endif
#if S == 4
inline vec<T, 4>::vec(const T x, const T y, const T z, const T w)
#endif
{
	data[0] = x;
#if S >= 2
	data[1] = y;
#endif
#if S >= 3
	data[2] = z;
#endif
#if S >= 4
	data[3] = w;
#endif
}


template<class T>
inline T vec<T, S>::length() const
{
	T out = data[0] * data[0];
	for (int i = 1; i < S; i++)
		out += data[i] * data[i];
	return sqrt(out);
}

template<class T>
inline T vec<T, S>::sqlength() const
{
	T out = data[0] * data[0];
	for (int i = 1; i < S; i++)
		out += data[i] * data[i];
	return out;
}

template<class T>
inline void vec<T, S>::normalise()
{
	float invLeng = 1.0f / (float)length();
	for (int i = 0; i < S; i++)
		data[i] *= invLeng;
}

#ifdef VEC_TOSTRING
template<class T>
inline std::string vec<T, S>::tostring() const
{
	std::stringstream strm;
	strm << '(' << data[0]; //Stream in (x
	for (int i = 1; i < S; i++)
		strm << ", " << data[i]; //Stream in , n
	strm << ')'; //Stream in )

	return strm.str();
}
#endif //VEC_TOSTRING

template<class T>
inline __m128 vec<T, S>::getSSE()
{
	__m128 sse;
	for (int i = 0; i < S; i++)
		((float*)&sse)[i] = data[i];
	return sse;
}

template<class T>
template<class U>
inline vec<T, S>& vec<T, S>::operator=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] = RHS[i];

	return *this;
}

template<class T>
template<class U>
inline vec<T, S>& vec<T, S>::operator=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] = RHS;

	return *this;
}

template<class T>
inline T& vec<T, S>::operator[](int Index)
{
	//assert(Index >= 0); //Bounds checking
	//assert(Index < S);

	return data[Index];
}

template<class T>
inline const T& vec<T, S>::operator[](int Index) const
{
	//assert(Index >= 0); //Bounds checking
	//assert(Index < S);

	return data[Index];
}

//Unary operators

//Addition
template<class T>
template<class U>
inline void vec<T, S>::operator+=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] += RHS[i];
}

template<class T>
template<class U>
inline void vec<T, S>::operator+=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] += RHS;
}

//Multiplication
template<class T>
template<class U>
inline void vec<T, S>::operator*=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] *= RHS[i];
}

template<class T>
template<class U>
inline void vec<T, S>::operator*=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] *= RHS;
}

//Subtraction
template<class T>
template<class U>
inline void vec<T, S>::operator-=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] -= RHS[i];
}

template<class T>
template<class U>
inline void vec<T, S>::operator-=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] -= RHS;
}

//Division
template<class T>
template<class U>
inline void vec<T, S>::operator/=(const vec<U, S>& RHS)
{
	for (int i = 0; i < S; i++)
		data[i] /= RHS[i];
}

template<class T>
template<class U>
inline void vec<T, S>::operator/=(const U RHS)
{
	for (int i = 0; i < S; i++)
		data[i] /= RHS;
}

//Negation?
template<class T>
inline vec<T, S> vec<T, S>::operator-() const
{
	vec<T, S> out;
	for (int i = 0; i < S; i++)
		out[i] = -data[i];
	return out;
}

