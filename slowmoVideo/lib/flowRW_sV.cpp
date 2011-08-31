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

void FlowRW_sV::save(std::string filename, FlowField_sV *flowField)
{
    int width = flowField->width();
    int height = flowField->height();
    std::cout << "Writing flow file " << filename << ": " << width << "x" << height
              << ", version " << (int)m_version << ", magic number " << m_magicNumber << std::endl;

    float *data = flowField->data();
    std::ofstream file(filename.c_str(), std::ios_base::out | std::ios_base::binary);
    file.write((char*) m_magicNumber.c_str(), m_magicNumber.length()*sizeof(char));
    file.write((char*) &m_version, sizeof(char));
    file.write((char*) &width, sizeof(int));
    file.write((char*) &height, sizeof(int));

    file.write((char*) data, sizeof(float)*flowField->dataSize());
    file.close();
}

FlowRW_sV::FlowInfo_sV FlowRW_sV::readInfo(std::string filename)
{
    FlowInfo_sV info;

    std::ifstream file(filename.c_str(), std::ios_base::in | std::ios_base::binary);

    char *magic = new char[m_magicNumber.size()+1];
    magic[m_magicNumber.size()] = 0;
    file.read(magic, sizeof(char)*m_magicNumber.size());
    file.read(&info.version, sizeof(char));
    info.magic = std::string(magic);
    delete[] magic;


    file.read((char*) &info.width, sizeof(int));
    file.read((char*) &info.height, sizeof(int));
    if (file.rdstate() == std::ios::goodbit) {
        if (info.magic.compare(m_magicNumber) == 0) {
            info.valid = true;
        }
        std::cout << "Magic number: " << info.magic << ", version: " << (int)info.version
                  << ", size: " << info.width << "x" << info.height << std::endl;
    } else {
        std::cerr << "Failed to read width/height from " << filename << "." << std::endl;
    }
    file.close();

    return info;
}

FlowField_sV* FlowRW_sV::load(std::string filename) throw(FlowRWError)
{
    std::ifstream file(filename.c_str(), std::ios_base::in | std::ios_base::binary);

    char *magic = new char[m_magicNumber.size()+1];
    magic[m_magicNumber.size()] = 0;
    char version;

    file.read(magic, sizeof(char)*m_magicNumber.size());
    file.read((char*) &version, sizeof(char));

    int width, height;
    file.read((char*) &width, sizeof(int));
    file.read((char*) &height, sizeof(int));
    if (file.rdstate() != std::ios::goodbit) {
        file.close();
        throw FlowRWError("Failed to read width/height from file " + filename);
    }

    FlowField_sV *field = new FlowField_sV(width, height);

    file.read((char*) field->data(), sizeof(float)*field->dataSize());
    if (file.rdstate() != std::ios::goodbit) {
        delete field;
        file.close();
        throw FlowRWError("Failed to read data from file " + filename);
    }
    file.close();

    std::cout << "Read flow file of size " << field->width()
              << "×" << field->height()
              << ". Magic number: " << magic
              << ", version: " << (int)version << std::endl;

    delete[] magic;
    return field;
}
