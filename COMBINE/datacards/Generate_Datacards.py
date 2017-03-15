#!/usr/bin/python
import fileinput
import os
import sys, getopt
import string

import os

fileToSearch = "Template_Datacard_allSyst.txt"
#fileToSearch = "Template_Datacard_noSyst.txt"

theFakeLLL = ""
theFLLL_chNumb = "-1"
theFakeRate_type = ""
total = len(sys.argv)
cmdargs = str(sys.argv)

print ("Script name: %s" % str(sys.argv[0]))
print ("First argument: %s" % str(sys.argv[1]))
print ("Second argument: %s" % str(sys.argv[2]))
theFiletoRead= sys.argv[3]

channel = str(sys.argv[1])
theVar = str(sys.argv[2])

if (channel!="uuu" and channel!="uue" and channel!="eeu" and channel!="eee"):
    print "wrong channel"
    print "channel should be uuu, uue, eeu or uuu"
    exit()

if channel=="uuu":
    theFakeLLL = "FakeMuMuMu"
    theFLLL_chNumb = "10"
    theFakeRate_type = "FakeRate_mu"
if channel=="uue":
    theFakeLLL = "FakeMuMuEl"
    theFLLL_chNumb = "11"
    theFakeRate_type = "FakeRate_el"
if channel=="eeu":
    theFakeLLL = "FakeElElMu"
    theFLLL_chNumb = "12"
    theFakeRate_type = "FakeRate_mu"
if channel=="eee":
    theFakeLLL = "FakeElElEl"
    theFLLL_chNumb= "13"
    theFakeRate_type = "FakeRate_el"

print 'creating datacard for channel '+channel+' and variable '+theVar
s = open( fileToSearch).read()
s = s.replace("chan", channel)
s = s.replace("var",theVar)
s = s.replace("FakeLLL",theFakeLLL )
s = s.replace("FLLL_chNumb", theFLLL_chNumb)
s = s.replace("FakeRate_type",    theFakeRate_type)
s = s.replace("filetoread", theFiletoRead)

print 'datacard created'

f = open("datacard_"+channel+"_"+theVar+".txt", 'w')
f.write(s)
f.close()
