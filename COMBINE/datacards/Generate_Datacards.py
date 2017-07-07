#!/usr/bin/python
import fileinput
import os
import sys, getopt
import string

import os

fileToSearch = "Template_Datacard.txt"

trigsyst=""
FakeEl_rateMu=""
FakeMu_rateMu=""
FakeEl_shapeMu=""
FakeMu_shapeMu=""

total = len(sys.argv)
cmdargs = str(sys.argv)

channel = str(sys.argv[1])
theVar = str(sys.argv[2])
theFiletoRead= sys.argv[3]
systList = str(sys.argv[4])


fakes_NP_value = "5.0";


if (channel!="uuu" and channel!="uue" and channel!="eeu" and channel!="eee"):
    print "wrong channel"
    print "channel should be uuu, uue, eeu or uuu"
    exit()

if channel=="uuu":
    FakeEl_rateElelif="-"
    FakeMu_rateMuelif=fakes_NP_value
    FakeEl_shapeElelif="-"
    FakeMu_shapeMuelif="1"
    trigsyst = "1.01"

elif channel=="uue":
    FakeEl_rateElelif=fakes_NP_value
    FakeMu_rateMuelif=fakes_NP_value
    FakeEl_shapeElelif="1"
    FakeMu_shapeMuelif="1"
    trigsyst = "1.01"

elif channel=="eeu":
    FakeEl_rateElelif=fakes_NP_value
    FakeMu_rateMuelif=fakes_NP_value
    FakeEl_shapeElelif="1"
    FakeMu_shapeMuelif="1"
    trigsyst = "1.01"

elif channel=="eee":
    FakeEl_rateElelif=fakes_NP_value
    FakeMu_rateMuelif="-"
    FakeEl_shapeElelif="1"
    FakeMu_shapeMuelif="-"
    trigsyst = "1.02"


if(systList!="allSyst" and systList!="noSyst"):
    print "Wrong systList value ! should be allSyst or noSyst"
    exit()
if systList=="allSyst":
    shapeSyst = ""
if systList == "noSyst":
    shapeSyst = "#"


print 'creating datacard for channel '+channel+' and variable '+theVar
s = open( fileToSearch).read()
s = s.replace("[*]", shapeSyst)
s = s.replace("chan", channel)
s = s.replace("var",theVar)

s = s.replace("FakeEl_rateMu", FakeEl_rateMu)
s = s.replace("FakeMu_rateMu", FakeMu_rateMu)

s = s.replace("FakeEl_shapeMu", FakeEl_shapeMu)
s = s.replace("FakeMu_shapeMu", FakeMu_shapeMu)

s = s.replace("trigsyst", trigsyst)

print 'datacard created'

f = open("datacard_"+channel+"_"+theVar+".txt", 'w')
f.write(s)
f.close()
