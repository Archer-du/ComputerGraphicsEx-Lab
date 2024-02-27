// implementation of class DArray
#include "DArray.h"
#include <cassert>
#include <iostream>

// implementation of class DArray
// default constructor
DArray::DArray()
{
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue)
	: m_nSize(nSize) {
	m_nMax = nSize;
	m_pData = new double[nSize];
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr)
	: m_nSize(arr.m_nSize), m_pData(new double[arr.m_nMax]), m_nMax(arr.m_nMax) {
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = arr.m_pData[i];
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	for (int i = 0; i < m_nSize; i++) {
		std::cout << m_pData[i] << " ";
	}
	std::cout << std::endl;
}

// initilize the array
void DArray::Init() {
	m_pData = nullptr;
	m_nMax = m_nSize = 0;
}

// free the array
void DArray::Free() {
	if (m_pData != nullptr)
		delete[] m_pData;
	m_pData = nullptr;
	m_nMax = m_nSize = 0;
}

void DArray::Reserve(int nSize) {
	if (nSize < m_nMax) return;
	m_nMax = nSize;
	double* p = new double[nSize];
	for (int i = 0; i < m_nSize; i++) {
		p[i] = m_pData[i];
	}
	delete[] m_pData;
	m_pData = p;
}

// get the size of the array
int DArray::GetSize() const {
	return m_nSize; // you should return a correct value
}

// set the size of the array
void DArray::SetSize(int nSize) {
	if (nSize == m_nSize) return;
	if (nSize > m_nSize) {
		if (nSize > m_nMax) {
			m_nMax = nSize;
			double* p = new double[m_nMax];
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
const double& DArray::GetAt(int nIndex) const {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	m_pData[nIndex] = dValue;
}

double& DArray::operator[](int nIndex) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}
// overload operator '[]'
const double& DArray::operator[](int nIndex) const {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	if (m_nSize + 1 > m_nMax) {
		m_nMax = m_nMax == 0 ? 1 : m_nMax * 2;
		double* p = new double[m_nMax];
		for (int i = 0; i < m_nSize; i++) {
			p[i] = m_pData[i];
		}
		delete[] m_pData;
		m_pData = p;
	}
	m_pData[m_nSize++] = dValue;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	for (int i = nIndex + 1; i < m_nSize; i++) {
		m_pData[i - 1] = m_pData[i];
	}
	m_nSize--;
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) {
	assert(nIndex <= m_nSize and nIndex >= 0 and "index out of range");
	if (m_nSize + 1 > m_nMax) {
		m_nMax = m_nMax == 0 ? 1 : m_nMax * 2;
		double* p = new double[m_nMax];
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
DArray& DArray::operator = (const DArray& arr) {
	m_nSize = arr.m_nSize;
	m_nMax = arr.m_nMax;
	m_pData = new double[arr.m_nMax];
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = arr.m_pData[i];
	}
	return *this;
}
