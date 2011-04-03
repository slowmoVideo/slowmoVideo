
from optparse import OptionParser

parser.add_option("-i", "--input", dest="inDir", help="Input Directory containing the video's frames", metavar="DIR")
parser.add_option("-f", "--flow", dest="flowDir", help="Input Directory containing the optical flow frames", metavar="DIR")
parser.add_option("-o", "--output", dest="outDir", help="Output Directory", metavar="DIR")
parser.add_option("--slowmo", dest="slowmoExecutable", help="Executable for slowmoVideo")

(options, args) = parser.parse_args()

frames = os.listDir(options.inDir)

prev = None
for frame in frames :
    if prev != None :
        print(frame)
    prev = frame
