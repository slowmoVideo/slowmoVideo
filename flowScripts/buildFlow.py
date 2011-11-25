#!/usr/bin/env python3

import argparse
import os.path
import signal


from naming import *

parser = argparse.ArgumentParser(description="Note that this is a rather old (read: deprecated) sample script. \
It simply builds flow files for all files in the input directory.")
parser.add_argument("-i", "--input", dest="inDir", required=True, help="Input Directory", metavar="DIR")
parser.add_argument("-o", "--output", dest="outDir", required=True, help="Output Directory", metavar="DIR")
parser.add_argument("--flow", dest="flowExecutable", required=True, help="Executable for optical flow")
parser.add_argument("--forward-only", action="store_true", dest="forwardOnly",  help="Calculate forward flow only")
parser.add_argument("--backward-only", action="store_true", dest="backwardOnly",  help="Calculate backward flow only")
parser.add_argument("--force-rebuild", action="store_true", dest="forceRebuild", help="Force rebuild of existing flow images")
parser.add_argument("--offset", dest="offset", type=int, default=0, help="Frame offset")
parser.add_argument("--lambda", dest="lambdaValue", type=float, default=10, help="V3D lambda value")

args = parser.parse_args()


def handler(signum, frame) :
    print("Signal %s received at frame %s. Terminating." % (signum, frame))
    exit(signum)
signal.signal(signal.SIGINT, handler)


if not os.path.exists(args.inDir) :
    print("Input directory does not exist.")
    exit(-2)
if not os.path.isdir(args.inDir) :
    print("Input directory is not a directory.")
    exit(-2)

args.outDir = os.path.abspath(args.outDir)
print("Output files go to %s." % args.outDir)


files = os.listdir(args.inDir)
files.sort()

if not os.path.exists(args.outDir) :
    print("Oputput directory does not exist. Creating it.")
    os.makedirs(args.outDir)


prev = None
for s in files :
    if frameID(s) != None and int(frameID(s)) >= args.offset :
        if prev != None :
            leftFile = args.inDir + os.sep + prev
            rightFile = args.inDir + os.sep + s
            
            if not args.backwardOnly :
                outFile = args.outDir + os.sep + nameForwardFlow(prev, s, args.lambdaValue)
                if not os.path.exists(outFile) or args.forceRebuild :
                    cmd = "%s %s %s %s 10 100" % (args.flowExecutable, leftFile, rightFile, outFile)
                    ret = os.system(cmd)
                    print("%s: Returned %s" % (outFile, ret))
                    if ret == 2 :
                        print("SIGINT received, terminating.")
                        exit(2)
                    elif ret == 65024 :
                        print("Environment variable not set, terminating")
                        exit(65024)
            
            if not args.forwardOnly :
                outFile = args.outDir + os.sep + nameBackwardFlow(prev, s, args.lambdaValue)
                if (not os.path.exists(outFile)) or args.forceRebuild :
                    cmd = "%s %s %s %s 10 100" % (args.flowExecutable, rightFile, leftFile, outFile)
                    ret = os.system(cmd)
                    print("%s: Returned %s" % (outFile, ret))
                    if ret == 2 :
                        print("SIGINT received, terminating.")
                        exit(2)
                    elif ret == 65024 :
                        print("Environment variable not set, terminating")
                        exit(65024)
        prev = s
