#include "PolynomialList.h"
#include <iostream>
#include <fstream>

using namespace std;

PolynomialList::PolynomialList(const PolynomialList& other) {
    m_Polynomial = other.m_Polynomial;
}

PolynomialList::PolynomialList(const string& file) {
    ReadFromFile(file);
}

PolynomialList::PolynomialList(const double* cof, const int* deg, int n) {
    for (int i = 0; i < n; i++) {
        AddOneTerm(Term(deg[i], cof[i]));
    }
}

PolynomialList::PolynomialList(const vector<int>& deg, const vector<double>& cof) {
    for (int i = 0; i < deg.size(); i++) {
        AddOneTerm(Term(deg[i], cof[i]));
    }
}

double PolynomialList::coff(int i) const {
    for (auto& term : m_Polynomial) {
        if (term.deg < i) break;
        if (term.deg == i) return term.cof;
    }
    return 0; // you should return a correct value
}

double& PolynomialList::coff(int i) {
    return AddOneTerm(Term(i, 0)).cof;
}

void PolynomialList::compress() {
    auto it = m_Polynomial.begin();
    while (it != m_Polynomial.end()) {
        if (it->cof == 0) {
            it = m_Polynomial.erase(it);
        }
        else ++it;
    }
}

PolynomialList PolynomialList::operator+(const PolynomialList& right) const {
    PolynomialList result;
    for (auto& p : m_Polynomial) {
        result.AddOneTerm({ p.deg, p.cof });
    }
    for (auto& p : right.m_Polynomial) {
        result.AddOneTerm({ p.deg, p.cof });
    }
    result.compress();
    return result;
}

PolynomialList PolynomialList::operator-(const PolynomialList& right) const {
    PolynomialList result;
    for (auto& p : m_Polynomial) {
        result.AddOneTerm({ p.deg, p.cof });
    }
    for (auto& p : right.m_Polynomial) {
        result.AddOneTerm({ p.deg, - p.cof });
    }
    result.compress();
    return result;
}

PolynomialList PolynomialList::operator*(const PolynomialList& right) const {
    PolynomialList result;
    for (auto& p : m_Polynomial) {
        for (auto& q : right.m_Polynomial) {
            Term term({ p.deg + q.deg, p.cof * q.cof });
            result.AddOneTerm(term);
        }
    }
    return result;
}

PolynomialList& PolynomialList::operator=(const PolynomialList& right) {
    m_Polynomial = right.m_Polynomial;
    return *this;
}

void PolynomialList::Print() const {
    for (auto& it : m_Polynomial) {
        cout << it.deg << " " << it.cof << endl;
    }
    cout << endl;
}

bool PolynomialList::ReadFromFile(const string& path) {
    ifstream inputFile(path);
    if (!inputFile.is_open()) {
        cerr << "fail opening file" << endl;
        return false;
    }
    string indentifier;
    int itemNum;
    inputFile >> indentifier >> itemNum;
    for (int i = 0; i < itemNum; i++) {
        Term term;
        inputFile >> term.deg >> term.cof;
        AddOneTerm(term);
    }
    inputFile.close();
    return true;
}

PolynomialList::Term& PolynomialList::AddOneTerm(const Term& term) {
    for (auto it = m_Polynomial.begin(); it != m_Polynomial.end(); ++it) {
        if (it->deg == term.deg) {
            it->cof += term.cof;
            return *it;
        }

        if (it->deg > term.deg) {
            return *m_Polynomial.insert(it, term);
        }
    }
    m_Polynomial.push_back(term);
    return m_Polynomial.back();
}
