
from optparse import OptionParser
import os.path
import re

parser = OptionParser()
parser.add_option("-i", "--input", dest="inDir", help="Input Directory", metavar="DIR")
parser.add_option("-o", "--output", dest="outDir", help="Output Directory", metavar="DIR")
parser.add_option("--flow", dest="flowExecutable", help="Executable for optical flow")

(options, args) = parser.parse_args()


if options.inDir == None :
    print("Please set an output directory.")
    exit(-1)
if options.outDir == None :
    print("Please set an input directory.")
    exit(-1)
if options.flowExecutable == None :
    print("Executable missing.")
    exit(-1)

if not os.path.exists(options.inDir) :
    print("Input directory does not exist.")
    exit(-2)
if not os.path.isdir(options.inDir) :
    print("Input directory is not a directory.")
    exit(-2)

files = os.listdir(options.inDir)
files.sort()

if not os.path.exists(options.outDir) :
    print("Oputput directory does not exist.")

pattern = re.compile('\D+(?P<name>\d+)\D+')
o = pattern.match(files[0])
print(o.group('name'))

    
print options
