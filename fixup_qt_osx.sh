#!/bin/bash
# fixup loader for OSX
install_name_tool -change /Users/val/Documents/Sources/qt4/lib/QtScript.framework/Versions/4/QtScript @executable_path/../Frameworks/QtScript.framework/Versions/4/QtScript slowmoInfo 
install_name_tool -change /Users/val/Documents/Sources/qt4/lib/QtCore.framework/Versions/4/QtCore  @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore slowmoInfo 
install_name_tool -change /Users/val/Documents/Sources/qt4/lib/QtTest.framework/Versions/4/QtTest @executable_path/../Frameworks/QtTest.framework/Versions/4/QtTest slowmoInfo 
install_name_tool -change /Users/val/Documents/Sources/qt4/lib/QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml 
