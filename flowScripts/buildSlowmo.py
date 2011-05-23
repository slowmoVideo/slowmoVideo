
import os
import signal
from optparse import OptionParser

from naming import *

framerate = 30

parser = OptionParser()
parser.add_option("-i", "--input", dest="inDir", help="Input Directory containing the video's frames", metavar="DIR")
parser.add_option("-f", "--flow", dest="flowDir", help="Input Directory containing the optical flow frames", metavar="DIR")
parser.add_option("-o", "--output", dest="outDir", help="Output Directory", metavar="DIR")
parser.add_option("--slowmo", dest="slowmoExecutable", help="Executable for slowmoVideo")
parser.add_option("--oneway", dest="oneway", action="store_true", help="Use forward flow only")
parser.add_option("--offset", dest="offset", type="int", default=0, help="Frame offset")
(options, args) = parser.parse_args()


def handler(signum, frame) :
    print("Received signal %s at frame %s. Terminating." % (signum, frame))
    exit(signum)
signal.signal(signal.SIGINT, handler)

if not os.path.exists(options.outDir) :
    os.makedirs(options.outDir)

frames = os.listdir(options.inDir)
frames.sort()

counter = 0

prev = None
for frame in frames :
    if frameID(frame) != None and int(frameID(frame)) >= options.offset :
        if prev != None :
            
            counter = int(frameID(frame))*framerate
            
            leftFrame = options.inDir + os.sep + prev
            rightFrame = options.inDir + os.sep + frame
            forwardFlow = options.flowDir + os.sep + nameForwardFlow(prev, frame)
            backwardFlow = options.flowDir + os.sep + nameBackwardFlow(prev, frame)
            
            if options.oneway :
                cmd = "%s forward %s %s %s/f%%1.png %s %s" % (options.slowmoExecutable, leftFrame, forwardFlow,  options.outDir, counter, framerate)
            else :
                cmd = "%s twoway %s %s %s %s %s/f%%1.png %s %s" % (options.slowmoExecutable, leftFrame, rightFrame, forwardFlow, backwardFlow, options.outDir, counter, framerate)
            print("Command: %s" % cmd)
            ret = os.system(cmd)
            if ret != 0 :
                exit(ret)
        prev = frame
