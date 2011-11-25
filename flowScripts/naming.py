#!/usr/bin/env python3

import re

pattern = re.compile('\D+(?P<name>\d+)\D+')
def frameID(name) :
    o = pattern.match(name)
    if o == None :
        print("No number found in %s." % name)
        return None
    else :
        return o.group('name')

def nameForwardFlow(left, right, lambdaValue) :
    return "forward-lambda{:.2f}_{}-{}.sVflow".format(lambdaValue, frameID(left), frameID(right))

def nameBackwardFlow(left, right, lambdaValue) :
    return "backward-lambda{:.2f}_{}-{}.sVflow".format(lambdaValue, frameID(right), frameID(left))
