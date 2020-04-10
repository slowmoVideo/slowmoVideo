#ifndef KERNEL_SV_H
#define KERNEL_SV_H

#include "flowField_sV.h"
#include <ostream>

/**
  Kernel for convolution

  This time following the Rule of Three: http://en.wikipedia.org/wiki/Rule_of_three_%28C%2B%2B_programming%29
  */
class Kernel_sV
{
public:
    Kernel_sV(int radiusX, int radiusY);
    Kernel_sV(const Kernel_sV &other);
    ~Kernel_sV();

    void gauss();

    int rX() const;
    int rY() const;
    float& operator()(int dx, int dy) const;

    Kernel_sV& operator=(const Kernel_sV &other);

private:
    int m_radiusX;
    int m_radiusY;
    int m_nElements;

    /// Data storage in row major order (line-wise)
    float *m_data;
};

std::ostream& operator <<(std::ostream &cout, const Kernel_sV& kernel);

#endif // KERNEL_SV_H
