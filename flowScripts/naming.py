import re

pattern = re.compile('\D+(?P<name>\d+)\D+')
def frameID(name) :
    o = pattern.match(name)
    if o == None :
        print("No number found in %s." % name)
        return None
    else :
        return o.group('name')

def nameForwardFlow(left, right) :
    return "flow_%s_%s.png" % (frameID(left), frameID(right))

def nameBackwardFlow(left, right) :
    return "flow_%s_%s_back.png" % (frameID(left), frameID(right))
