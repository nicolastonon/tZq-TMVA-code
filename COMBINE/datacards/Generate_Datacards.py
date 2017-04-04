#!/usr/bin/python
import fileinput
import os
import sys, getopt
import string

import os

fileToSearch = "Template_Datacard.txt"

theFlll = "-"
Fakes_ch = ""
theFakeRate_type = "-"
tZq_ch = ""
ttZ_ch = ""
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
    theFlll = "FakeMuMuMu"
    Fakes_ch = "10"
    theFakeRate_type = "FakeRate_mu"
    trigsyst = "1.01"
if channel=="uue":
    theFlll = "FakeMuMuEl"
    Fakes_ch = "11"
    theFakeRate_type = "FakeRate_el"
    trigsyst = "1.01"
if channel=="eeu":
    theFlll = "FakeElElMu"
    Fakes_ch = "12"
    theFakeRate_type = "FakeRate_mu"
    trigsyst = "1.01"
if channel=="eee":
    theFlll = "FakeElElEl"
    Fakes_ch= "13"
    theFakeRate_type = "FakeRate_el"
    trigsyst = "1.02"

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
    tZq_ch = "0"
    ttZ_ch = "3"
    tZqsyst = "-"
    ttZsyst = "1.3"
if Signal=="ttZ":
    tZq_ch = "3"
    ttZ_ch = "0"
    tZqsyst = "1.3"
    ttZsyst = "-"
if Signal=="tZqANDttZ":
    tZq_ch = "0"
    ttZ_ch = "-1"
    tZqsyst = "-"
    ttZsyst = "-"
if Signal=="tZqANDFakes":
    tZq_ch = "0"
    tZqsyst = "-"
    ttZ_ch = "3"
    ttZsyst = "1.3"
    FakeRate_Numb = "-"
    FakeBgd_Numb = "-"

    if channel=="uuu":
        Fakes_ch = "-1"
    if channel=="uue":
        Fakes_ch = "-2"
    if channel=="eeu":
        Fakes_ch = "-3"
    if channel=="eee":
        Fakes_ch= "-4"




print 'creating datacard for channel '+channel+' and variable '+theVar
s = open( fileToSearch).read()
s = s.replace("[*]", shapeSyst)
s = s.replace("chan", channel)
s = s.replace("var",theVar)
s = s.replace("Flll",theFlll )
s = s.replace("Fakes_ch", Fakes_ch)
s = s.replace("FakeRate_type",    theFakeRate_type)
s = s.replace("filetoread", theFiletoRead)
s = s.replace("tZq_ch", tZq_ch)
s = s.replace("ttZ_ch", ttZ_ch)
s = s.replace("tZqsyst", tZqsyst)
s = s.replace("ttZsyst", ttZsyst)
s = s.replace("FakeRate_Numb", FakeRate_Numb)
s = s.replace("FakeBgd_Numb", FakeBgd_Numb)
s = s.replace("trigsyst", trigsyst)

print 'datacard created'

f = open("datacard_"+channel+"_"+theVar+".txt", 'w')
f.write(s)
f.close()
