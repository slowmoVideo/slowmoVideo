#include "flowRW.h"

#include <iostream>
#include <fstream>

const std::string FlowRW_sV::m_magicNumber = "flow_sV";
const char FlowRW_sV::m_version = 1;

void FlowRW_sV::save(std::string filename, const int width, const int height, const float *data, Format format)
{
    std::ofstream file(filename.c_str(), std::ios_base::out | std::ios_base::binary);
    file.write((char*) m_magicNumber.c_str(), m_magicNumber.length()*sizeof(char));
    file.write((char*) &m_version, sizeof(char));
    file.write((char*) &width, sizeof(int));
    file.write((char*) &height, sizeof(int));

    switch (format) {
    case Format_RGB:
    default:
        int pos = 0;
        for (int i = 0; i < width*height; i++) {
            file.write((char*) &data[pos++], sizeof(float));
            file.write((char*) &data[pos++], sizeof(float));
            pos++;
        }
    }
    file.close();
}

float* FlowRW_sV::load(std::string filename, int &out_width, int &out_height)
{
    std::ifstream file(filename.c_str(), std::ios_base::in | std::ios_base::binary);

    file.read((char*) &out_width, sizeof(int));
    file.read((char*) &out_height, sizeof(int));
    if (file.rdstate() != std::ios::goodbit) {
        std::cerr << "Failed to read width/height from " << filename << "." << std::endl;
    }

    float *data = new float[out_width*out_height];

    file.read((char*) data, sizeof(float)*out_width*out_height);
    if (file.rdstate() != std::ios::goodbit) {
        std::cerr << "Failed to read data from " << filename << "." << std::endl;
    }

    file.close();

    return data;
}
