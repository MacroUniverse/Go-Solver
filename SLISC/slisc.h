// type and class definitions and dependencies
// this header file can be used alone

#pragma once

#ifndef NDEBUG
// this will not check the last index
#define _CHECKBOUNDS_
#endif

// all the system #include's we'll ever need
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <vector>
#include <limits>
#include <string.h>

namespace slisc
{

// Scalar types

typedef const int Int_I; // 32 bit integer
typedef int Int;
typedef int &Int_O, &Int_IO;
typedef const unsigned int Uint_I;
typedef unsigned int Uint;
typedef unsigned int &Uint_O, &Uint_IO;

#ifdef _MSC_VER
typedef const __int64 Llong_I; // 64 bit integer
typedef __int64 Llong;
typedef __int64 &Llong_O, &Llong_IO;
typedef const unsigned __int64 Ullong_I;
typedef unsigned __int64 Ullong;
typedef unsigned __int64 &Ullong_O, &Ullong_IO;
#else
typedef const long long int Llong_I; // 64 bit integer
typedef long long int Llong;
typedef long long int &Llong_O, &Llong_IO;
typedef const unsigned long long int Ullong_I;
typedef unsigned long long int Ullong;
typedef unsigned long long int &Ullong_O, &Ullong_IO;
#endif

#ifndef _USE_Int_AS_LONG
typedef Llong Long;
#else
typedef Int Long;
#endif
typedef const Long Long_I;
typedef Long;
typedef Long &Long_O, &Long_IO;

typedef const char Char_I; // 8 bit integer
typedef char Char;
typedef char &Char_O, &Char_IO;
typedef const unsigned char Uchar_I;
typedef unsigned char Uchar;
typedef unsigned char &Uchar_O, &Uchar_IO;

typedef const double Doub_I; // default floating type
typedef double Doub;
typedef double &Doub_O, &Doub_IO;

typedef const long double &Ldoub_I;
typedef long double Ldoub;
typedef long double &Ldoub_O, &Ldoub_IO;

typedef const std::complex<double> &Comp_I;
typedef std::complex<double> Comp;
typedef std::complex<double> &Comp_O, &Comp_IO;

typedef const bool Bool_I;
typedef bool Bool;
typedef bool &Bool_O, &Bool_IO;

// NaN definition
static const Doub NaN = std::numeric_limits<Doub>::quiet_NaN();

// === constants ===

const Doub PI = 3.14159265358979323;
const Doub E = 2.71828182845904524;
const Comp I(0., 1.);

// report error and pause execution
#define error(str) do{std::cout << "error: " << __FILE__ << ": line " << __LINE__ << ": " << str << std::endl; getchar();} while(0)

#define warning(str) do{std::cout << "warning: " << __FILE__ << ": line " << __LINE__ << ": " << str << std::endl;} while(0)

template<class T>
inline void memset(T *dest, const T val, Long_I n)
{
	T *end = dest + n;
	for (; dest < end; ++dest)
		*dest = val;
}

// For cuSLISC project
#ifdef _CUSLISC_
template <class T> class Gvector;
template <class T> class Gmatrix;
template <class T> class Gmat3d;
#endif

// Base Class for vector/matrix
template <class T>
class Vbase
{
protected:
	Long m_N; // number of elements
	T *m_p; // pointer to the first element
	inline void move(Vbase &rhs);
public:
	Vbase() : m_N(0), m_p(nullptr) {}
	explicit Vbase(Long_I N) : m_N(N), m_p(new T[N]) {}
	T* ptr() { return m_p; } // get pointer
	inline const T* ptr() const { return m_p; }
	inline Long_I size() const { return m_N; }
	inline void resize(Long_I N);
	inline T & operator()(Long_I i);
	inline const T & operator()(Long_I i) const;
	inline T& end(); // last element
	inline const T& end() const;
	inline T& end(Long_I i);
	inline const T& end(Long_I i) const;
	~Vbase() { if (m_p) delete m_p; }
};

template <class T>
inline void Vbase<T>::resize(Long_I N)
{
	if (N != m_N) {
		if (m_p != nullptr) delete[] m_p;
		m_N = N;
		m_p = N > 0 ? new T[N] : nullptr;
	}
}

template <class T>
inline void Vbase<T>::move(Vbase &rhs)
{
	if (m_p != nullptr) delete[] m_p;
	m_N = rhs.m_N; rhs.m_N = 0;
	m_p = rhs.m_p; rhs.m_p = nullptr;
}

template <class T>
inline T & Vbase<T>::operator()(Long_I i)
{
#ifdef _CHECKBOUNDS_
if (i<0 || i>=m_N)
	error("Vbase subscript out of bounds");
#endif
	return m_p[i];
}

template <class T>
inline const T & Vbase<T>::operator()(Long_I i) const
{
#ifdef _CHECKBOUNDS_
	if (i<0 || i>=m_N)
		error("Vbase subscript out of bounds");
#endif
	return m_p[i];
}

template <class T>
inline T & Vbase<T>::end()
{
#ifdef _CHECKBOUNDS_
	if (m_N < 1)
		error("Using end() for empty object");
#endif
	return m_p[m_N-1];
}

template <class T>
inline const T & Vbase<T>::end() const
{
#ifdef _CHECKBOUNDS_
	if (m_N < 1)
		error("Using end() for empty object");
#endif
	return m_p[m_N-1];
}

template <class T>
inline T& Vbase<T>::end(Long_I i)
{
#ifdef _CHECKBOUNDS_
	if (i <= 0 || i > m_N)
		error("index out of bound");
#endif
	return m_p[m_N-i];
}

template <class T>
inline const T& Vbase<T>::end(Long_I i) const
{
#ifdef _CHECKBOUNDS_
	if (i <= 0 || i > m_N)
		error("index out of bound");
#endif
	return m_p[m_N-i];
}

// Vector Class

template <class T>
class Vector : public Vbase<T>
{
public:
	typedef Vbase<T> Base;
	using Base::m_p;
	using Base::m_N;
	Vector() {}
	explicit Vector(Long_I N): Base(N) {}
	Vector(Long_I N, const T &a) //initialize to constant value
	: Vector(N) { memset(m_p,a,N); }
	Vector(Long_I N, const T *a) // Initialize to array
	: Vector(N) { memcpy(m_p, a, N*sizeof(T)); }
	Vector(const Vector &rhs);	// Copy constructor forbidden
	inline Vector & operator=(const Vector &rhs);	// copy assignment
	inline Vector & operator=(const T &rhs);  // assign to constant value
#ifdef _CUSLISC_
	Vector & operator=(const Gvector<T> &rhs) // copy from GPU vector
	{ rhs.get(*this); return *this; }
#endif
	inline void operator<<(Vector &rhs); // move data and rhs.resize(0)
	inline T & operator[](Long_I i);	//i'th element
	inline const T & operator[](Long_I i) const;
	inline void resize(Long_I N) {Base::resize(N);} // resize (contents not preserved)
	template <class T1>
	void resize(const Vector<T1> &v) {resize(v.size());}
};

template <class T>
Vector<T>::Vector(const Vector<T> &rhs)
{
	error("Copy constructor or move constructor is forbidden, use reference "
		 "argument for function input or output, and use \"=\" to copy!");
}

template <class T>
inline Vector<T> & Vector<T>::operator=(const Vector<T> &rhs)
{
	if (this == &rhs) error("self assignment is forbidden!");
	resize(rhs);
	memcpy(m_p, rhs.m_p, m_N*sizeof(T));
	return *this;
}

template <class T>
inline Vector<T>& Vector<T>::operator=(const T &rhs)
{
	if (m_N) memset(m_p, rhs, m_N);
	return *this;
}

template <class T>
inline void Vector<T>::operator<<(Vector<T> &rhs)
{
	if (this == &rhs) error("self move is forbidden!");
	Base::move(rhs);
}

template <class T>
inline T & Vector<T>::operator[](Long_I i)
{
#ifdef _CHECKBOUNDS_
if (i<0 || i>=m_N)
	error("Vector subscript out of bounds");
#endif
	return m_p[i];
}

template <class T>
inline const T & Vector<T>::operator[](Long_I i) const
{
#ifdef _CHECKBOUNDS_
if (i<0 || i>=m_N)
	error("Vector subscript out of bounds");
#endif
	return m_p[i];
}

// Matrix Class

template <class T>
class Matrix : public Vbase<T>
{
private:
	typedef Vbase<T> Base;
	using Base::m_p;
	using Base::m_N;
	Long m_Nr, m_Nc;
public:
	using Base::ptr;
	using Base::operator();
	Matrix();
	Matrix(Long_I Nr, Long_I Nc);
	Matrix(Long_I Nr, Long_I Nc, const T &s);	//Initialize to constant
	Matrix(Long_I Nr, Long_I Nc, const T *ptr);	// Initialize to array
	Matrix(const Matrix &rhs);		// Copy constructor
	inline Matrix & operator=(const Matrix &rhs);	// copy assignment
	inline Matrix & operator=(const T &rhs);
#ifdef _CUSLISC_
	Matrix & operator=(const Gmatrix<T> &rhs) // copy from GPU vector
	{ rhs.get(*this); return *this; }
#endif
	inline void operator<<(Matrix &rhs); // move data and rhs.resize(0, 0)
	inline T& operator()(Long_I i, Long_I j); // double indexing
	inline const T& operator()(Long_I i, Long_I j) const;
	const T *ptr(Long_I i) const; // pointer to the beginning of a row
	T *ptr(Long_I i);
	inline Long nrows() const;
	inline Long ncols() const;
	inline void resize(Long_I Nr, Long_I Nc); // resize (contents not preserved)
	template <class T1>
	inline void resize(const Matrix<T1> &a);
};

template <class T>
Matrix<T>::Matrix() : m_Nr(0), m_Nc(0) {}

template <class T>
Matrix<T>::Matrix(Long_I Nr, Long_I Nc) : Base(Nr*Nc), m_Nr(Nr), m_Nc(Nc) {}

template <class T>
Matrix<T>::Matrix(Long_I Nr, Long_I Nc, const T &s) : Matrix(Nr, Nc)
{ memset(m_p, s, m_N); }

template <class T>
Matrix<T>::Matrix(Long_I Nr, Long_I Nc, const T *ptr) : Matrix(Nr, Nc)
{ memcpy(m_p, ptr, m_N*sizeof(T)); }

template <class T>
Matrix<T>::Matrix(const Matrix<T> &rhs) : Matrix()
{
	error("Copy constructor or move constructor is forbidden, use reference argument for function input or output, and use \"=\" to copy!");
}

template <class T>
inline Matrix<T> & Matrix<T>::operator=(const Matrix<T> &rhs)
{
	if (this == &rhs) error("self assignment is forbidden!");
	resize(rhs.m_Nr, rhs.m_Nc);
	memcpy(m_p, rhs.m_p, m_N*sizeof(T));
	return *this;
}

template <class T>
inline Matrix<T> & Matrix<T>::operator=(const T &rhs)
{
	if (m_N) memset(m_p, rhs, m_N);
	return *this;
}

template <class T>
inline void Matrix<T>::operator<<(Matrix<T> &rhs)
{
	if (this == &rhs) error("self move is forbidden!");
	Base::move(rhs);
	m_Nr = rhs.m_Nr; m_Nc = rhs.m_Nc;
	rhs.m_Nr = rhs.m_Nc = 0;
}

template <class T>
inline T& Matrix<T>::operator()(Long_I i, Long_I j)
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_Nr || j < 0 || j >= m_Nc)
		error("Matrix subscript out of bounds");
#endif
	return m_p[m_Nc*i+j];
}

template <class T>
inline const T& Matrix<T>::operator()(Long_I i, Long_I j) const
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_Nr || j < 0 || j >= m_Nc)
		error("Matrix subscript out of bounds");
#endif
	return m_p[m_Nc*i+j];
}

template <class T>
inline const T * Matrix<T>::ptr(Long_I i) const
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_Nr)
		error("Matrix subscript out of bounds");
#endif
	return m_p + m_Nc*i;
}

template <class T>
inline T * Matrix<T>::ptr(Long_I i)
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_Nr)
		error("Matrix subscript out of bounds");
#endif
	return m_p + m_Nc*i;
}

template <class T>
inline Long Matrix<T>::nrows() const
{ return m_Nr; }

template <class T>
inline Long Matrix<T>::ncols() const
{ return m_Nc; }

template <class T>
inline void Matrix<T>::resize(Long_I Nr, Long_I Nc)
{
	if (Nr != m_Nr || Nc != m_Nc) {
		Base::resize(Nr*Nc);
		m_Nr = Nr; m_Nc = Nc;
	}
}

template <class T>
template <class T1>
inline void Matrix<T>::resize(const Matrix<T1> &a)
{ resize(a.nrows(), a.ncols()); }

// Column major Matrix Class

template <class T>
class Cmat : public Vbase<T>
{
private:
	typedef Vbase<T> Base;
	using Base::m_p;
	using Base::m_N;
	Long m_Nr, m_Nc;
public:
	using Base::operator();
	Cmat();
	Cmat(Long_I Nr, Long_I Nc);
	Cmat(Long_I Nr, Long_I Nc, const T &s);	//Initialize to constant
	Cmat(Long_I Nr, Long_I Nc, const T *ptr);	// Initialize to array
	Cmat(const Cmat &rhs);		// Copy constructor
	inline Cmat & operator=(const Cmat &rhs);	// copy assignment
	inline Cmat & operator=(const T &rhs);
	inline void operator<<(Cmat &rhs); // move data and rhs.resize(0, 0)
	inline T& operator()(Long_I i, Long_I j);	// double indexing
	inline const T& operator()(Long_I i, Long_I j) const;
	inline Long nrows() const;
	inline Long ncols() const;
	inline void resize(Long_I Nr, Long_I Nc); // resize (contents not preserved)
	template <class T1>
	inline void resize(const Cmat<T1> &a);
	~Cmat() {};
};

template <class T>
Cmat<T>::Cmat() : m_Nr(0), m_Nc(0) {}

template <class T>
Cmat<T>::Cmat(Long_I Nr, Long_I Nc) : Base(Nr*Nc), m_Nr(Nr), m_Nc(Nc) {}

template <class T>
Cmat<T>::Cmat(Long_I Nr, Long_I Nc, const T &s) : Cmat(Nr, Nc)
{ memset(m_p, s, m_N); }

template <class T>
Cmat<T>::Cmat(Long_I Nr, Long_I Nc, const T *ptr) : Cmat(Nr, Nc)
{ memcpy(m_p, ptr, m_N*sizeof(T)); }

template <class T>
Cmat<T>::Cmat(const Cmat<T> &rhs)
{
	error("Copy constructor or move constructor is forbidden, use reference argument for function input or output, and use \"=\" to copy!");
}

template <class T>
inline Cmat<T> & Cmat<T>::operator=(const Cmat<T> &rhs)
{
	if (this == &rhs) error("self assignment is forbidden!");
	resize(rhs.m_Nr, rhs.m_Nc);
	memcpy(m_p, rhs.m_p, m_N*sizeof(T));
	return *this;
}

template <class T>
inline Cmat<T> & Cmat<T>::operator=(const T &rhs)
{
	if (m_N) memset(m_p, rhs, m_N);
	return *this;
}

template <class T>
inline void Cmat<T>::operator<<(Cmat<T> &rhs)
{
	if (this == &rhs) error("self move is forbidden!");
	Base::move(rhs);
	m_Nr = rhs.m_Nr; m_Nc = rhs.m_Nc;
	rhs.m_Nr = rhs.m_Nc = 0;
}

template <class T>
inline T& Cmat<T>::operator()(Long_I i, Long_I j)
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_Nr || j < 0 || j >= m_Nc)
		error("Matrix subscript out of bounds");
#endif
	return m_p[i+m_Nr*j];
}

template <class T>
inline const T& Cmat<T>::operator()(Long_I i, Long_I j) const
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_Nr || j < 0 || j >= m_Nc)
		error("Matrix subscript out of bounds");
#endif
	return m_p[i+m_Nr*j];
}

template <class T>
inline Long Cmat<T>::nrows() const
{ return m_Nr; }

template <class T>
inline Long Cmat<T>::ncols() const
{ return m_Nc; }

template <class T>
inline void Cmat<T>::resize(Long_I Nr, Long_I Nc)
{
	if (Nr != m_Nr || Nc != m_Nc) {
		Base::resize(Nr*Nc);
		m_Nr = Nr; m_Nc = Nc;
	}
}

template <class T>
template <class T1>
inline void Cmat<T>::resize(const Cmat<T1> &a)
{ resize(a.nrows(), a.ncols()); }

// 3D Matrix Class

template <class T>
class Mat3d : public Vbase<T>
{
private:
	typedef Vbase<T> Base;
	using Base::m_p;
	using Base::m_N;
	Long m_N1;
	Long m_N2;
	Long m_N3;
public:
	using Base::operator();
	using Base::ptr;
	Mat3d();
	Mat3d(Long_I N1, Long_I N2, Long_I N3);
	Mat3d(Long_I N1, Long_I N2, Long_I N3, const T &a);
	Mat3d(const Mat3d &rhs);   // Copy constructor
	inline Mat3d & operator=(const Mat3d &rhs);	// copy assignment
	inline Mat3d & operator=(const T &rhs);
#ifdef _CUSLISC_
	Mat3d & operator=(const Gmat3d<T> &rhs) // copy from GPU vector
	{ rhs.get(*this); return *this; }
#endif
	inline void operator<<(Mat3d &rhs); // move data and rhs.resize(0, 0, 0)
	inline void resize(Long_I N1, Long_I N2, Long_I N3);
	template <class T1>
	inline void resize(const Mat3d<T1> &a);
	inline T & operator()(Long_I i, Long_I j, Long_I k);	//subscripting: pointer to row i
	inline const T & operator()(Long_I i, Long_I j, Long_I k) const;
	const T* ptr(Long_I i, Long_I j) const;
	T* ptr(Long_I i, Long_I j);
	inline Long dim1() const;
	inline Long dim2() const;
	inline Long dim3() const;
};

template <class T>
Mat3d<T>::Mat3d(): m_N1(0), m_N2(0), m_N3(0) {}

template <class T>
Mat3d<T>::Mat3d(Long_I N1, Long_I N2, Long_I N3) : Base(N1*N2*N3), m_N1(N1), m_N2(N2), m_N3(N3) {}

template <class T>
Mat3d<T>::Mat3d(Long_I N1, Long_I N2, Long_I N3, const T &s) : Mat3d(N1, N2, N3)
{ memset(m_p, s, N1*N2*N3); }

template <class T>
Mat3d<T>::Mat3d(const Mat3d<T> &rhs)
{
	error("Copy constructor or move constructor is forbidden, use reference argument for function input or output, and use \"=\" to copy!");
}

template <class T>
inline Mat3d<T> &Mat3d<T>::operator=(const Mat3d<T> &rhs)
{
	if (this == &rhs) error("self assignment is forbidden!");
	resize(rhs.m_N1, rhs.m_N2, rhs.m_N3);
	memcpy(m_p, rhs.m_p, m_N*sizeof(T));
	return *this;
}

template <class T>
inline Mat3d<T> & Mat3d<T>::operator=(const T &rhs)
{
	if (m_N) memset(m_p, rhs, m_N);
	return *this;
}

template <class T>
inline void Mat3d<T>::operator<<(Mat3d<T> &rhs)
{
	if (this == &rhs) error("self move is forbidden!");
	Base::move(rhs);
	m_N1 = rhs.m_N1; m_N2 = rhs.m_N2; m_N3 = rhs.m_N3;
	rhs.m_N1 = rhs.m_N2 = rhs.m_N3 = 0;
}

template <class T>
inline void Mat3d<T>::resize(Long_I N1, Long_I N2, Long_I N3)
{
	if (N1 != m_N1 || N2 != m_N2 || N3 != m_N3) {
		Base::resize(N1*N2*N3);
		m_N1 = N1; m_N2 = N2; m_N3 = N3;
	}
}

template <class T>
template <class T1>
inline void Mat3d<T>::resize(const Mat3d<T1> &a) { resize(a.dim1(), a.dim2(), a.dim3()); }

template <class T>
inline T &Mat3d<T>::operator()(Long_I i, Long_I j, Long_I k)
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_N1 || j < 0 || j >= m_N2 || k < 0 || k >= m_N3)
		error("Matrix subscript out of bounds");
#endif
	return m_p[m_N2*m_N3*i + m_N3*j + k];
}

template <class T>
inline const T &Mat3d<T>::operator()(Long_I i, Long_I j, Long_I k) const
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_N1 || j < 0 || j >= m_N2 || k < 0 || k >= m_N3)
		error("Matrix subscript out of bounds");
#endif
	return m_p[m_N2*m_N3*i + m_N3*j + k];
}

template <class T>
inline const T * Mat3d<T>::ptr(Long_I i, Long_I j) const
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_N1 || j < 0 || j >= m_N2)
		error("Matrix subscript out of bounds");
#endif
	return m_p + m_N2*m_N3*i + m_N3*j;
}

template <class T>
inline T *Mat3d<T>::ptr(Long_I i, Long_I j)
{
#ifdef _CHECKBOUNDS_
	if (i < 0 || i >= m_N1 || j < 0 || j >= m_N2)
		error("Matrix subscript out of bounds");
#endif
	return m_p + m_N2*m_N3*i + m_N3*j;
}

template <class T>
inline Long Mat3d<T>::dim1() const { return m_N1; }

template <class T>
inline Long Mat3d<T>::dim2() const { return m_N2; }

template <class T>
inline Long Mat3d<T>::dim3() const { return m_N3; }

// Matric and vector types

typedef const Vector<Int> &VecInt_I;
typedef Vector<Int> VecInt;
typedef Vector<Int> &VecInt_O, &VecInt_IO;

typedef const Vector<Uint> &VecUint_I;
typedef Vector<Uint> VecUint;
typedef Vector<Uint> &VecUint_O, &VecUint_IO;

typedef const Vector<Long> &VecLong_I;
typedef Vector<Long> VecLong;
typedef Vector<Long> &VecLong_O, &VecLong_IO;

typedef const Vector<Llong> &VecLlong_I;
typedef Vector<Llong> VecLlong;
typedef Vector<Llong> &VecLlong_O, &VecLlong_IO;

typedef const Vector<Ullong> &VecUllong_I;
typedef Vector<Ullong> VecUllong;
typedef Vector<Ullong> &VecUllong_O, &VecUllong_IO;

typedef const Vector<Char> &VecChar_I;
typedef Vector<Char> VecChar;
typedef Vector<Char> &VecChar_O, &VecChar_IO;

typedef const Vector<Char*> &VecCharp_I;
typedef Vector<Char*> VecCharp;
typedef Vector<Char*> &VecCharp_O, &VecCharp_IO;

typedef const Vector<Uchar> &VecUchar_I;
typedef Vector<Uchar> VecUchar;
typedef Vector<Uchar> &VecUchar_O, &VecUchar_IO;

typedef const Vector<Doub> &VecDoub_I;
typedef Vector<Doub> VecDoub;
typedef Vector<Doub> &VecDoub_O, &VecDoub_IO;

typedef const Vector<Doub*> &VecDoubp_I;
typedef Vector<Doub*> VecDoubp;
typedef Vector<Doub*> &VecDoubp_O, &VecDoubp_IO;

typedef const Vector<Comp> &VecComp_I;
typedef Vector<Comp> VecComp;
typedef Vector<Comp> &VecComp_O, &VecComp_IO;

typedef const Vector<Bool> &VecBool_I;
typedef Vector<Bool> VecBool;
typedef Vector<Bool> &VecBool_O, &VecBool_IO;

typedef const Matrix<Int> &MatInt_I;
typedef Matrix<Int> MatInt;
typedef Matrix<Int> &MatInt_O, &MatInt_IO;

typedef const Matrix<Uint> &MatUint_I;
typedef Matrix<Uint> MatUint;
typedef Matrix<Uint> &MatUint_O, &MatUint_IO;

typedef const Matrix<Llong> &MatLlong_I;
typedef Matrix<Llong> MatLlong;
typedef Matrix<Llong> &MatLlong_O, &MatLlong_IO;

typedef const Matrix<Ullong> &MatUllong_I;
typedef Matrix<Ullong> MatUllong;
typedef Matrix<Ullong> &MatUllong_O, &MatUllong_IO;

typedef const Matrix<Char> &MatChar_I;
typedef Matrix<Char> MatChar;
typedef Matrix<Char> &MatChar_O, &MatChar_IO;

typedef const Matrix<Uchar> &MatUchar_I;
typedef Matrix<Uchar> MatUchar;
typedef Matrix<Uchar> &MatUchar_O, &MatUchar_IO;

typedef const Matrix<Doub> &MatDoub_I;
typedef Matrix<Doub> MatDoub;
typedef Matrix<Doub> &MatDoub_O, &MatDoub_IO;

typedef const Matrix<Comp> &MatComp_I;
typedef Matrix<Comp> MatComp;
typedef Matrix<Comp> &MatComp_O, &MatComp_IO;

typedef const Matrix<Bool> &MatBool_I;
typedef Matrix<Bool> MatBool;
typedef Matrix<Bool> &MatBool_O, &MatBool_IO;

typedef const Cmat<Int> &CmatInt_I;
typedef Cmat<Int> CmatInt;
typedef Cmat<Int> &CmatInt_O, &CmatInt_IO;

typedef const Cmat<Uint> &CmatUint_I;
typedef Cmat<Uint> CmatUint;
typedef Cmat<Uint> &CmatUint_O, &CmatUint_IO;

typedef const Cmat<Llong> &CmatLlong_I;
typedef Cmat<Llong> CmatLlong;
typedef Cmat<Llong> &CmatLlong_O, &CmatLlong_IO;

typedef const Cmat<Ullong> &CmatUllong_I;
typedef Cmat<Ullong> CmatUllong;
typedef Cmat<Ullong> &CmatUllong_O, &CmatUllong_IO;

typedef const Cmat<Char> &CmatChar_I;
typedef Cmat<Char> CmatChar;
typedef Cmat<Char> &CmatChar_O, &CmatChar_IO;

typedef const Cmat<Uchar> &CmatUchar_I;
typedef Cmat<Uchar> CmatUchar;
typedef Cmat<Uchar> &CmatUchar_O, &CmatUchar_IO;

typedef const Cmat<Doub> &CmatDoub_I;
typedef Cmat<Doub> CmatDoub;
typedef Cmat<Doub> &CmatDoub_O, &CmatDoub_IO;

typedef const Cmat<Comp> &CmatComp_I;
typedef Cmat<Comp> CmatComp;
typedef Cmat<Comp> &CmatComp_O, &CmatComp_IO;

typedef const Cmat<Bool> &CmatBool_I;
typedef Cmat<Bool> CmatBool;
typedef Cmat<Bool> &CmatBool_O, &CmatBool_IO;

typedef const Mat3d<Doub> &Mat3Doub_I;
typedef Mat3d<Doub> Mat3Doub;
typedef Mat3d<Doub> &Mat3Doub_O, &Mat3Doub_IO;

typedef const Mat3d<Comp> &Mat3Comp_I;
typedef Mat3d<Comp> Mat3Comp;
typedef Mat3d<Comp> &Mat3Comp_O, &Mat3Comp_IO;

// macro-like functions (don't use them in your code ever, write similar utilities in "algorithm.h")

template<class T>
inline T SQR(const T a) { return a*a; }

template<class T>
inline const T &MAX(const T &a, const T &b)
{ return b > a ? (b) : (a); }

inline float MAX(const double &a, const float &b)
{ return b > a ? (b) : float(a); }

inline float MAX(const float &a, const double &b)
{ return b > a ? float(b) : (a); }

template<class T>
inline const T &MIN(const T &a, const T &b)
{ return b < a ? (b) : (a); }

inline float MIN(const double &a, const float &b)
{ return b < a ? (b) : float(a); }

inline float MIN(const float &a, const double &b)
{ return b < a ? float(b) : (a); }

template<class T>
inline T SIGN(const T &a, const T &b)
{ return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a); }

inline float SIGN(const float &a, const double &b)
{ return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a); }

inline float SIGN(const double &a, const float &b)
{ return (float)(b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a)); }

template<class T>
inline void SWAP(T &a, T &b)
{ T dum = a; a = b; b = dum; }

} // namespace slisc
