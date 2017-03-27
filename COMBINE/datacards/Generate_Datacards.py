#!/usr/bin/python
import fileinput
import os
import sys, getopt
import string

import os

fileToSearch = "Template_Datacard.txt"

theFakeLLL = "-"
theFLLL_chNumb = "-"
theFakeRate_type = "-"
tZq_chNumb = "-"
ttZ_chNumb = "-"
tZqsyst = "-"
ttZsyst = "-"
shapeSyst = ""
total = len(sys.argv)
cmdargs = str(sys.argv)

print ("Script name: %s" % str(sys.argv[0]))
print ("First argument: %s" % str(sys.argv[1]))
print ("Second argument: %s" % str(sys.argv[2]))

channel = str(sys.argv[1])
theVar = str(sys.argv[2])
theFiletoRead= sys.argv[3]
systList = str(sys.argv[4])
Signal = str(sys.argv[5])

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

FakeRate_Numb = "5.0"
FakeBgd_Numb = "1.3"


if(systList!="allSyst" and systList!="noSyst"):
    print "Wrong systList value ! should be allSyst or noSyst"
    exit()
if systList=="allSyst":
    shapeSyst = ""
if systList == "noSyst":
    shapeSyst = "#"


if(Signal!="tZq" and Signal!="ttZ" and Signal!="tZqANDttZ" and Signal!="tZqANDFakes"):
    print "Wrong Signal value ! should be tZq or ttZ or tZqANDttZ or tZqANDFakes"
    exit()
if Signal=="tZq":
    tZq_chNumb = "0"
    ttZ_chNumb = "3"
    tZqsyst = "-"
    ttZsyst = "1.3"
if Signal=="ttZ":
    tZq_chNumb = "3"
    ttZ_chNumb = "0"
    tZqsyst = "1.3"
    ttZsyst = "-"
if Signal=="tZqANDttZ":
    tZq_chNumb = "0"
    ttZ_chNumb = "-1"
    tZqsyst = "-"
    ttZsyst = "-"
if Signal=="tZqANDFakes":
    tZq_chNumb = "0"
    tZqsyst = "-"
    ttZ_chNumb = "3"
    ttZsyst = "1.3"
    FakeRate_Numb = "-"
    FakeBgd_Numb = "-"

    if channel=="uuu":
        theFLLL_chNumb = "-1"
    if channel=="uue":
        theFLLL_chNumb = "-2"
    if channel=="eeu":
        theFLLL_chNumb = "-3"
    if channel=="eee":
        theFLLL_chNumb= "-4"




print 'creating datacard for channel '+channel+' and variable '+theVar
s = open( fileToSearch).read()
s = s.replace("[*]", shapeSyst)
s = s.replace("chan", channel)
s = s.replace("var",theVar)
s = s.replace("FakeLLL",theFakeLLL )
s = s.replace("FLLL_chNumb", theFLLL_chNumb)
s = s.replace("FakeRate_type",    theFakeRate_type)
s = s.replace("filetoread", theFiletoRead)
s = s.replace("tZq_chNumb", tZq_chNumb)
s = s.replace("ttZ_chNumb", ttZ_chNumb)
s = s.replace("tZqsyst", tZqsyst)
s = s.replace("ttZsyst", ttZsyst)
s = s.replace("FakeRate_Numb", FakeRate_Numb)
s = s.replace("FakeBgd_Numb", FakeBgd_Numb)

print 'datacard created'

f = open("datacard_"+channel+"_"+theVar+".txt", 'w')
f.write(s)
f.close()
