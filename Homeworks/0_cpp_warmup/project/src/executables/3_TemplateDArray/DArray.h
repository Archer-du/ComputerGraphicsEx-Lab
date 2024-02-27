#pragma once
#include <cassert>
#include <iostream>

// interfaces of Dynamic Array class DArray
template <class T>
class DArray {
public:
	DArray(); // default constructor
	DArray(int nSize, T dValue = 0); // set an array with default values
	DArray(const DArray& arr); // copy constructor
	~DArray(); // deconstructor

	void Print() const; // print the elements of the array

	int GetSize() const; // get the size of the array
	void SetSize(int nSize); // set the size of the array

	const T& GetAt(int nIndex) const; // get an element at an index
	void SetAt(int nIndex, T dValue); // set the value of an element

	T& operator[](int nIndex); // overload operator '[]'
	const T& operator[](int nIndex) const; // overload operator '[]'

	void PushBack(T dValue); // add a new element at the end of the array
	void DeleteAt(int nIndex); // delete an element at some index
	void InsertAt(int nIndex, T dValue); // insert a new element at some index

	DArray& operator = (const DArray& arr); //overload operator '='

private:
	T* m_pData; // the pointer to the array memory
	int m_nSize; // the size of the array
	int m_nMax;

private:
	void Init(); // initilize the array
	void Free(); // free the array
	void Reserve(int nSize); // allocate enough memory
};


// implementation of class DArray
// default constructor
template <class T>
DArray<T>::DArray()
{
	Init();
}

// set an array with default values
template <class T>
DArray<T>::DArray(int nSize, T dValue)
	: m_nSize(nSize) {
	m_nMax = nSize;
	m_pData = new T[nSize];
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = dValue;
	}
}

template <class T>
DArray<T>::DArray(const DArray& arr)
	: m_nSize(arr.m_nSize), m_pData(new T[arr.m_nMax]), m_nMax(arr.m_nMax) {
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = arr.m_pData[i];
	}
}

// deconstructor
template <class T>
DArray<T>::~DArray() {
	Free();
}

// display the elements of the array
template <class T>
void DArray<T>::Print() const {
	for (int i = 0; i < m_nSize; i++) {
		std::cout << m_pData[i] << " ";
	}
	std::cout << std::endl;
}

// initilize the array
template <class T>
void DArray<T>::Init() {
	m_pData = nullptr;
	m_nMax = m_nSize = 0;
}

// free the array
template <class T>
void DArray<T>::Free() {
	if (m_pData != nullptr)
		delete[] m_pData;
	m_pData = nullptr;
	m_nMax = m_nSize = 0;
}

template <class T>
void DArray<T>::Reserve(int nSize) {
	if (nSize < m_nMax) return;
	m_nMax = nSize;
	T* p = new T[nSize];
	for (int i = 0; i < m_nSize; i++) {
		p[i] = m_pData[i];
	}
	delete[] m_pData;
	m_pData = p;
}

// get the size of the array
template <class T>
int DArray<T>::GetSize() const {
	return m_nSize; // you should return a correct value
}

// set the size of the array
template <class T>
void DArray<T>::SetSize(int nSize) {
	if (nSize == m_nSize) return;
	if (nSize > m_nSize) {
		if (nSize > m_nMax) {
			m_nMax = nSize;
			T* p = new T[m_nMax];
			for (int i = 0; i < m_nSize; i++) {
				p[i] = m_pData[i];
			}
			delete[] m_pData;
			m_pData = p;
		}
		for (int i = m_nSize; i < nSize; i++) {
			m_pData[i] = 0;
		}
	}
	m_nSize = nSize;
}

// get an element at an index
template <class T>
const T& DArray<T>::GetAt(int nIndex) const {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}

// set the value of an element 
template <class T>
void DArray<T>::SetAt(int nIndex, T dValue) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	m_pData[nIndex] = dValue;
}

template <class T>
T& DArray<T>::operator[](int nIndex) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}
// overload operator '[]'
template <class T>
const T& DArray<T>::operator[](int nIndex) const {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
template <class T>
void DArray<T>::PushBack(T dValue) {
	if (m_nSize + 1 > m_nMax) {
		m_nMax = m_nMax == 0 ? 1 : m_nMax * 2;
		T* p = new T[m_nMax];
		for (int i = 0; i < m_nSize; i++) {
			p[i] = m_pData[i];
		}
		delete[] m_pData;
		m_pData = p;
	}
	m_pData[m_nSize++] = dValue;
}

// delete an element at some index
template <class T>
void DArray<T>::DeleteAt(int nIndex) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	for (int i = nIndex + 1; i < m_nSize; i++) {
		m_pData[i - 1] = m_pData[i];
	}
	m_nSize--;
}

// insert a new element at some index
template <class T>
void DArray<T>::InsertAt(int nIndex, T dValue) {
	assert(nIndex <= m_nSize and nIndex >= 0 and "index out of range");
	if (m_nSize + 1 > m_nMax) {
		m_nMax = m_nMax == 0 ? 1 : m_nMax * 2;
		T* p = new T[m_nMax];
		int i = 0, j = 0;
		m_nSize++;
		while (i < m_nSize) {
			if (i == nIndex) {
				p[i++] = dValue;
				continue;
			}
			p[i] = m_pData[j];
			i++; j++;
		}
		delete[] m_pData;
		m_pData = p;
	}
	else {
		for (int i = m_nSize; i > nIndex; i--) {
			m_pData[i] = m_pData[i - 1];
		}
		m_nSize++;
		m_pData[nIndex] = dValue;
	}
}

// overload operator '='
template <class T>
DArray<T>& DArray<T>::operator = (const DArray& arr) {
	m_nSize = arr.m_nSize;
	m_nMax = arr.m_nMax;
	m_pData = new T[arr.m_nMax];
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = arr.m_pData[i];
	}
	return *this;
}
