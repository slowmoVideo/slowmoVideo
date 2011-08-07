#!/bin/bash
# Creates a bzipped archive containing the necessary executables and resource files.

(
    cd slowmoVideo
    if [ ! -e build ]
    then
	mkdir build
    fi
    cd build 
    cmake ..
    make -j3 install
)

(
    cd V3D
    if [ ! -e build ]
    then
	mkdir build
    fi
    cd build 
    cmake ..
    make -j3 install
)

dirname="slowmoVideo-$(install/bin/slowmoInfo version)-$(install/bin/slowmoInfo platform)-$(install/bin/slowmoInfo bits)"
filename="${dirname}-$(date -u +%b%d).tar.bz2"
echo "Dirname: ${dirname}"
if [ -e ${dirname} ]
then
    rm -rf ${dirname}
else
    echo "${dirname} does not exist yet."
fi

mkdir ${dirname}
mkdir ${dirname}/bin

(
cd install/bin
if [ $? == 0 ]
then
    cp slowmoUI flowBuilder visualizeFlow ../../${dirname}/bin
    cp -r res ../../${dirname}/bin/
else
    echo "install/bin/ not found, exiting"
    exit -1
fi
)

tar cvjf ${filename} ${dirname}

rm -rf ${dirname}
