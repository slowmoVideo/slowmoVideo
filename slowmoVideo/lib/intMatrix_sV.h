#ifndef INTMATRIX_SV_H
#define INTMATRIX_SV_H

class IntMatrix_sV
{
public:
    IntMatrix_sV(int width, int height, int channels);
    ~IntMatrix_sV();

    int width() const;
    int height() const;
    int channels() const;

    void operator +=(const unsigned char *bytes);
    void operator /=(int divisor);

    unsigned char* toBytesArray() const;
    const int* data() const;

private:
    int m_width;
    int m_height;
    int m_channels;
    int *m_data;
};

#endif // INTMATRIX_SV_H
