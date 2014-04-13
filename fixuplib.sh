#!/bin/bash
# script to fixup additionnal library on osx

prog=$1.app/Contents/MacOS/$1
lib=$1.app/Contents/Frameworks/lib

function fixup_dylib
{
if test -h $1 ; then
        echo "Skipping $1 because it is a symlink"
        return 0
    fi
    
    dy_check=$(otool -D $1 | tail -n 1 | grep "is not an object file")
    if [[ $dy_check != "" ]] ; then
        echo "Skipping $3 because it is not a library"
        return 0
    fi

#get dylib name
thislib=$(otool -D $1 | tail -1)
thisLibraryFixed=$(echo "$thislib" | sed "s/^lib/@executable_path\/..\/Frameworks\/lib/g")

# fixing ref lib
echo "fixing referening libs"
sharedLib=$(otool -L $1 | grep -v executable | grep -v "/usr/lib" | grep -v "/System/Library" | awk '{print $1}')
for lib in $sharedLib 
do
	newlibrary=$(echo $lib | sed "s/^lib/@executable_path\/..\/Frameworks\/lib/g")
	install_name_tool -change $lib  $newlibrary $1
done

# fixing id
echo "making execute relative :  $thislib $thisLibraryFixed"
install_name_tool -id $thisLibraryFixed $1
return 1

}


function fixup_exe 
{
    if test -h $1 ; then
        echo "Skipping $3 because it is a symlink"
        return 0
    fi

sharedLib=$(otool -L $1 | grep -v executable | grep -v "/usr/lib" | grep -v "/System/Library" | awk '{print $1}')
for lib in $sharedLib 
do
	newlibrary=$(echo $lib | sed "s/^lib/@executable_path\/..\/Frameworks\/lib/g")
	install_name_tool -change $lib  $newlibrary $1
done
return 1
}

for lib in  $lib/*
do
	fixup_dylib "$lib"
done

fixup_exe "$prog"

