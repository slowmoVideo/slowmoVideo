/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowRW_sV.h"
#include "flowField_sV.h"

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

FlowField_sV* FlowRW_sV::load(std::string filename)
{
    std::ifstream file(filename.c_str(), std::ios_base::in | std::ios_base::binary);

    char *magic = new char[m_magicNumber.size()];
    char version;
    file.read(magic, sizeof(char)*m_magicNumber.size());
    file.read((char*) &version, sizeof(char));

    std::cout << "Magic number: " << magic << ", version: " << (int)version << std::endl;

    int width, height;
    file.read((char*) &width, sizeof(int));
    file.read((char*) &height, sizeof(int));
    if (file.rdstate() != std::ios::goodbit) {
        std::cerr << "Failed to read width/height from " << filename << "." << std::endl;
        return NULL;
    }

    FlowField_sV *field = new FlowField_sV(width, height);

    file.read((char*) field->data(), sizeof(float)*field->dataSize());
    if (file.rdstate() != std::ios::goodbit) {
        std::cerr << "Failed to read data from " << filename << "." << std::endl;
        delete field;
        return NULL;
    }

    file.close();

    return field;
}
