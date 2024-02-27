// implementation of class DArray
#include "DArray.h"
#include <cassert>
#include <iostream>

// default constructor
DArray::DArray() 
{
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue)
	: m_nSize(nSize), m_pData(new double[nSize]) {
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr)
	: m_nSize(arr.m_nSize), m_pData(new double[arr.m_nSize]) {
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
	m_nSize = 0;
}

// free the array
void DArray::Free() {
	delete[] m_pData;
	m_pData = nullptr;
	m_nSize = 0;
}

// get the size of the array
int DArray::GetSize() const {
	return m_nSize; // you should return a correct value
}

// set the size of the array
void DArray::SetSize(int nSize) {
	if (nSize == m_nSize) return;
	double* p = new double[nSize];
	if (nSize < m_nSize) {
		for (int i = 0; i < nSize; i++) {
			p[i] = m_pData[i];
		}
	}
	else {
		for (int i = 0; i < nSize; i++) {
			p[i] = i >= m_nSize ? 0 : m_pData[i];
		}
	}

	m_nSize = nSize;
	delete[] m_pData;
	m_pData = p;
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

// overload operator '[]'
const double& DArray::operator[](int nIndex) const {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	double* p = new double[++m_nSize];
	for (int i = 0; i < m_nSize - 1; i++) {
		p[i] = m_pData[i];
	}
	delete[] m_pData;
	m_pData = p;
	m_pData[m_nSize - 1] = dValue;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	assert(nIndex < m_nSize and nIndex >= 0 and "index out of range");
	double* p = new double[--m_nSize];
	int i = 0, j = 0;
	while (i < m_nSize) {
		if (j == nIndex) j++;
		p[i] = m_pData[j];
		i++; j++;
	}
	delete[] m_pData;
	m_pData = p;
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) {
	assert(nIndex <= m_nSize and nIndex >= 0 and "index out of range");
	double* p = new double[++m_nSize];
	int i = 0, j = 0;
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

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	m_nSize = arr.m_nSize;
	m_pData = new double[arr.m_nSize];
	for (int i = 0; i < m_nSize; i++) {
		m_pData[i] = arr.m_pData[i];
	}
	return *this;
}
