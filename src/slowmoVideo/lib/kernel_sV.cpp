#include "kernel_sV.h"

#include <algorithm>
#include <cmath>

Kernel_sV::Kernel_sV(int radiusX, int radiusY) :
    m_radiusX(radiusX),
    m_radiusY(radiusY),
    m_nElements((2*m_radiusX+1)*(2*m_radiusY+1))
{
    m_data = new float[m_nElements];
}

Kernel_sV::Kernel_sV(const Kernel_sV &other) :
    m_radiusX(other.m_radiusX),
    m_radiusY(other.m_radiusY),
    m_nElements((2*m_radiusX+1)*(2*m_radiusY+1))
{
    m_data = new float[m_nElements];
    std::copy(other.m_data, other.m_data+m_nElements, m_data);
}

Kernel_sV::~Kernel_sV()
{
    delete[] m_data;
}

int Kernel_sV::rX() const
{
    return m_radiusX;
}
int Kernel_sV::rY() const
{
    return m_radiusY;
}

void Kernel_sV::gauss()
{
    float r;
    for (int dx = -m_radiusX; dx <= m_radiusX; dx++) {
        for (int dy = -m_radiusY; dy <= m_radiusY; dy++) {
            r = std::sqrt(
                        std::pow(dx/float(m_radiusX),2) +
                        std::pow(dy/float(m_radiusY),2)
                        );
            (*this)(dx, dy) = std::exp(-std::pow(r*2, 2));
        }
    }
}

float& Kernel_sV::operator ()(int dx, int dy) const
{
    int x = dx + m_radiusX;
    int y = dy + m_radiusY;
    int width = 2*m_radiusX+1;
    return m_data[y*width + x];
}

Kernel_sV& Kernel_sV::operator =(const Kernel_sV &other)
{
    m_radiusX = other.m_radiusX;
    m_radiusY = other.m_radiusY;
    m_nElements = (2*m_radiusX+1)*(2*m_radiusY+1);

    delete[] m_data;
    m_data = new float[m_nElements];
    std::copy(other.m_data, other.m_data+m_nElements, m_data);
    return *this;
}

std::ostream& operator <<(std::ostream &cout, const Kernel_sV& kernel)
{
    int prec = cout.precision();
    cout.precision(4);
    cout.setf(std::ios::fixed,std::ios::floatfield);
    cout << "Kernel with radius " << kernel.rX() << "," << kernel.rY()
         << " at memory location " << &kernel << ":" << std::endl;
    for (int dy = -kernel.rY(); dy <= kernel.rY(); dy++) {
        for (int dx = -kernel.rX(); dx <= kernel.rX(); dx++) {
            cout << kernel(dx, dy) << " ";
        }
        cout << std::endl;
    }
    cout.precision(prec);
    return cout;
}
