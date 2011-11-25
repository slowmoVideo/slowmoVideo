#!/usr/bin/env python3

import os
import signal
import argparse

from naming import *

parser = argparse.ArgumentParser(description="Deprecated. Used to render output frames. Executable not available anymore.")
parser.add_argument("-i", "--input", dest="inDir", required=True, help="Input Directory containing the video's frames", metavar="DIR")
parser.add_argument("-f", "--flow", dest="flowDir", required=True, help="Input Directory containing the optical flow frames", metavar="DIR")
parser.add_argument("-o", "--output", dest="outDir", required=True, help="Output Directory", metavar="DIR")
parser.add_argument("--slowmo", dest="slowmoExecutable", required=True, help="Executable for slowmoVideo")
parser.add_argument("--oneway", dest="oneway", action="store_true", help="Use forward flow only")
parser.add_argument("--offset", dest="offset", type=int, default=0, help="Frame offset")
parser.add_argument("--lambda", dest="lambdaValue", required=True, type=float, default=10, help="V3D lambda value")
parser.add_argument("--framerate", dest="framerate", type=float, default=30, help="Output frame rate")

args = parser.parse_args()


def handler(signum, frame) :
    print("Received signal %s at frame %s. Terminating." % (signum, frame))
    exit(signum)
signal.signal(signal.SIGINT, handler)

if not os.path.exists(args.outDir) :
    os.makedirs(args.outDir)

frames = os.listdir(args.inDir)
frames.sort()

counter = 0

prev = None
for frame in frames :
    if frameID(frame) != None and int(frameID(frame)) >= args.offset :
        if prev != None :
            
            counter = int(frameID(frame))*args.framerate
            
            leftFrame = args.inDir + os.sep + prev
            rightFrame = args.inDir + os.sep + frame
            forwardFlow = args.flowDir + os.sep + nameForwardFlow(prev, frame, args.lambdaValue)
            backwardFlow = args.flowDir + os.sep + nameBackwardFlow(prev, frame, args.lambdaValue)
            
            if args.oneway :
                cmd = "%s forward %s %s %s/f%%1.png %s %s" % (args.slowmoExecutable, leftFrame, forwardFlow,  args.outDir, counter, args.framerate)
            else :
                cmd = "%s twoway %s %s %s %s %s/f%%1.png %s %s" % (args.slowmoExecutable, leftFrame, rightFrame, forwardFlow, backwardFlow, args.outDir, counter, args.framerate)
            print("Command: %s" % cmd)
            ret = os.system(cmd)
            if ret != 0 :
                exit(ret)
        prev = frame
