
#include "../lib/defs_sV.hpp"
#include <iostream>

int main(int argc, char *argv[]) 
{
    if (argc-1 > 0) {
        if (strcmp("platform", argv[1]) == 0) {
            std::cout << Version_sV::platform.toStdString();
        } else if (strcmp("version", argv[1]) == 0) {
            std::cout << Version_sV::version.toStdString();
        } else if (strcmp("bits", argv[1]) == 0) {
            std::cout << Version_sV::bits.toStdString();
        } else {
            std::cout << "Argument not recognized; see " << argv[0] << " (without arguments) for a list.\n";
        }
    } else {
        std::cout << "Possible arguments: version, bits, platform\n";
    }
}
