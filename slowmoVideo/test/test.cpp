#include <iostream>

#include "test.h"
#include "../project/project_sV.h"
#include "../project/xmlProjectRW_sV.h"

Test::Test()
{
}


int main()
{
    Project_sV proj("/data/Videos/2010-09-14-DSC_5111.AVI", "/tmp");
    XmlProjectRW_sV writer;
    writer.saveProject(&proj, "/tmp/test.sVproj");
}
