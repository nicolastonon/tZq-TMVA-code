#!/usr/bin/python
import fileinput
import os
import sys, getopt
import string

import os


trigsyst=""
FakeEl_rateEl=""
FakeMu_rateMu=""
FakeEl_shapeEl=""
FakeMu_shapeMu=""
fakes_NP_value = "";
fake_bkg = "";
double_uncert = "";

FakeRateMu_ch="";
FakeRateEl_ch="";

total = len(sys.argv)
cmdargs = str(sys.argv)

channel = str(sys.argv[1])
theVar = str(sys.argv[2])
theFiletoRead= sys.argv[3]
systList = str(sys.argv[4])
fakes_NP_value = str(sys.argv[5])
double_uncert = str(sys.argv[6])
# fake_bkg = str(sys.argv[6])

fileToSearch = "Template_Datacard.txt"

# fakes_NP_value = "5.0";
# fake_bkg = "3.0";

if (channel!="uuu" and channel!="uue" and channel!="eeu" and channel!="eee"):
    print "wrong channel"
    print "channel should be uuu, uue, eeu or uuu"
    exit()

if channel=="uuu":
    FakeEl_rateEl="-"
    FakeRateMu_ch="FakeRateMu_uuu"
    if fakes_NP_value=="0":
        if double_uncert=="y":
            FakeMu_rateMu="2.2"
        else:
            FakeMu_rateMu="1.6"
    else:
        FakeMu_rateMu=fakes_NP_value

    FakeEl_shapeEl="-"
    FakeMu_shapeMu="1"
    trigsyst = "1.01"
    fileToSearch = "Template_Datacard_uuu.txt"

elif channel=="uue":
    FakeRateMu_ch="FakeRateMu_uue"
    FakeRateEl_ch="FakeRateEl_uue"

    if fakes_NP_value=="0":
        if double_uncert=="y":
            FakeMu_rateMu="1.5"
            FakeEl_rateEl="1.5"
        else:
            FakeMu_rateMu="1.25"
            FakeEl_rateEl="1.25"

    else:
        FakeMu_rateMu=fakes_NP_value
        FakeEl_rateEl=fakes_NP_value

    FakeEl_shapeEl="1"
    FakeMu_shapeMu="1"
    trigsyst = "1.01"

elif channel=="eeu":
    FakeRateMu_ch="FakeRateMu_eeu"
    FakeRateEl_ch="FakeRateEl_eeu"

    if fakes_NP_value=="0":
        if double_uncert=="y":
            FakeMu_rateMu="2.5"
            FakeEl_rateEl="2.5"
        else:
            FakeMu_rateMu="1.75"
            FakeEl_rateEl="1.75"

    else:
        FakeMu_rateMu=fakes_NP_value
        FakeEl_rateEl=fakes_NP_value

    FakeEl_shapeEl="1"
    FakeMu_shapeMu="1"
    trigsyst = "1.01"

elif channel=="eee":
    FakeMu_rateMu="-"
    FakeRateEl_ch="FakeRateEl_eee"

    if fakes_NP_value=="0":
        if double_uncert=="y":
            FakeEl_rateEl="1.5"
        else:
            FakeEl_rateEl="1.25"

    else:
        FakeEl_rateEl=fakes_NP_value

    FakeEl_shapeEl="1"
    FakeMu_shapeMu="-"
    trigsyst = "1.02"
    fileToSearch = "Template_Datacard_eee.txt"


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

s = s.replace("FakeEl_rateEl", FakeEl_rateEl)
s = s.replace("FakeMu_rateMu", FakeMu_rateMu)

s = s.replace("FakeEl_shapeEl", FakeEl_shapeEl)
s = s.replace("FakeMu_shapeMu", FakeMu_shapeMu)

s = s.replace("FakeRateMu_ch", FakeRateMu_ch)
s = s.replace("FakeRateEl_ch", FakeRateEl_ch)

s = s.replace("Fake_bkg", fake_bkg)

s = s.replace("trigsyst", trigsyst)

s = s.replace("filetoread", theFiletoRead)

print 'datacard created'

f = open("datacard_"+channel+"_"+theVar+".txt", 'w')
f.write(s)
f.close()
