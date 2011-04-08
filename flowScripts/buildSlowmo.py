
import os
import signal
from optparse import OptionParser

from naming import *


parser = OptionParser()
parser.add_option("-i", "--input", dest="inDir", help="Input Directory containing the video's frames", metavar="DIR")
parser.add_option("-f", "--flow", dest="flowDir", help="Input Directory containing the optical flow frames", metavar="DIR")
parser.add_option("-o", "--output", dest="outDir", help="Output Directory", metavar="DIR")
parser.add_option("--slowmo", dest="slowmoExecutable", help="Executable for slowmoVideo")
(options, args) = parser.parse_args()


def handler(signum, frame) :
    print("Received signal %s at frame %s. Terminating." % (signum, frame))
    exit(signum)
signal.signal(signal.SIGINT, handler)


frames = os.listdir(options.inDir)
frames.sort()

counter = 0

prev = None
for frame in frames :
    if frameID(frame) != None :
        if prev != None :
            leftFrame = options.inDir + os.sep + prev
            rightFrame = options.inDir + os.sep + frame
            forwardFlow = options.flowDir + os.sep + nameForwardFlow(prev, frame)
            backwardFlow = options.flowDir + os.sep + nameBackwardFlow(prev, frame)
            
            cmd = "%s twoway %s %s %s %s %s/f%%1.png %s" % (options.slowmoExecutable, leftFrame, rightFrame, forwardFlow, backwardFlow, options.outDir, counter)
            print("Command: %s" % cmd)
            ret = os.system(cmd)
            if ret != 0 :
                exit(ret)
            counter += 26
        prev = frame
