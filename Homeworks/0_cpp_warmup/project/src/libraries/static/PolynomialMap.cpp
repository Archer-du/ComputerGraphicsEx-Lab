#include "PolynomialMap.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>

using namespace std;

PolynomialMap::PolynomialMap(const PolynomialMap& other) {
    m_Polynomial = other.m_Polynomial;
}

PolynomialMap::PolynomialMap(const string& file) {
    ReadFromFile(file);
}

PolynomialMap::PolynomialMap(const double* cof, const int* deg, int n) {
    for (int i = 0; i < n; i++) {
        AddOneTerm(deg[i], cof[i]);
    }
}

PolynomialMap::PolynomialMap(const vector<int>& deg, const vector<double>& cof) {
    for (int i = 0; i < deg.size(); i++) {
        AddOneTerm(deg[i], cof[i]);
    }
}

double PolynomialMap::coff(int i) const {
    auto it = m_Polynomial.find(i);
    if (it == m_Polynomial.end()) return 0;
    return it->second;
}

double& PolynomialMap::coff(int i) {
    return m_Polynomial[i];
}

void PolynomialMap::compress() {
    auto it = m_Polynomial.find(0);
    while (it != m_Polynomial.end()) {
        m_Polynomial.erase(it);
        it = m_Polynomial.find(0);
    }
}

PolynomialMap PolynomialMap::operator+(const PolynomialMap& right) const {
    PolynomialMap result;
    for (auto& p : m_Polynomial) {
        result.AddOneTerm(p.first, p.second);
    }
    for (auto& p : right.m_Polynomial) {
        result.AddOneTerm(p.first, p.second);
    }
    result.compress();
    return result;
}

PolynomialMap PolynomialMap::operator-(const PolynomialMap& right) const {
    PolynomialMap result;
    for (auto& p : m_Polynomial) {
        result.AddOneTerm(p.first, p.second);
    }
    for (auto& p : right.m_Polynomial) {
        result.AddOneTerm(p.first, - p.second);
    }
    result.compress();
    return result;
}

PolynomialMap PolynomialMap::operator*(const PolynomialMap& right) const {
    PolynomialMap result;
    for (auto& p : m_Polynomial) {
        for (auto& q : right.m_Polynomial) {
            result.AddOneTerm(p.first + q.first, p.second * q.second);
        }
    }
    return result;
}

PolynomialMap& PolynomialMap::operator=(const PolynomialMap& right) {
    m_Polynomial = right.m_Polynomial;
    return *this;
}

void PolynomialMap::Print() const {
    for (auto& it : m_Polynomial) {
        cout << it.first << " " << it.second << endl;
    }
    cout << endl;
}

bool PolynomialMap::ReadFromFile(const string& path) {
    
    ifstream inputFile(path);
    if (!inputFile.is_open()) {
        cerr << "fail opening file" << endl;
        return false;
    }
    string indentifier;
    int itemNum;
    inputFile >> indentifier >> itemNum;
    for (int i = 0; i < itemNum; i++) {
        int deg; double cof;
        inputFile >> deg >> cof;
        AddOneTerm(deg, cof);
    }
    inputFile.close();
    return true;
}

void PolynomialMap::AddOneTerm(int deg, double cof) {
    if (m_Polynomial.find(deg) != m_Polynomial.end()) {
        coff(deg) += cof;
    }
    else {
        coff(deg) = cof;
    }
}
