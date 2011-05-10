#include <iostream>

#include "test.h"
#include "../project/project_sV.h"

Test::Test()
{
}


int main()
{
    Project_sV proj("/data/Videos/2010-09-14-DSC_5111.AVI", "/tmp");
//    proj.extractFrames();
    proj.save("/tmp/test.sVproj");
//    proj.frameAt(1);
}
