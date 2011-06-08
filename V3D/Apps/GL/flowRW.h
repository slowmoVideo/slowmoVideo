#ifndef FLOWRW_SV_H
#define FLOWRW_SV_H

#include <string>

class FlowRW_sV
{
public:
    enum Format { Format_RGB };

    static void save(std::string filename, const int width, const int height, const float *data, Format format = Format_RGB);
    static float* load(std::string filename, int &out_width, int &out_height);

private:
    static const std::string m_magicNumber;
    static const char m_version;
};

#endif // FLOWRW_SV_H
