#!/usr/bin/env python

import CombineHarvester.CombineTools.ch as ch
from sys import version_info #To obtain python version automatically

py3 = version_info[0] > 2 #creates boolean value for test that Python major version > 2

if py3:
  systematics = input("--- Include systematics ? (y/n) : ")
else:
  systematics = raw_input("--- Include systematics ? (y/n) : ")

if py3:
  choice = input("--- Template or input variables ? (0/1) : ")
else:
  choice = raw_input("--- Template or input variables ? (0/1) : ")

if choice=="1":
	if py3:
  		region = input("--- tZq/ttZ/WZ ? : ")
	else:
  		region = raw_input("--- tZq/ttZ/WZ : ")

if systematics=="y":
    syst_suffix = ""
else:
    syst_suffix = "_noSyst"



# Create a CombineHarvester instance and parse an existing datacard
# Important to specify the mass, other metadata values like analysis and channel are optional
cmb = ch.CombineHarvester()
#cmb.ParseDatacard('COMBINED_datacard_TemplateFit_tZq'+syst_suffix+'.txt', mass='', analysis='tZq', channel='all')

if choice=="0":
    cmb.ParseDatacard('COMBINED_datacard_TemplateFit_tZq'+syst_suffix+'.txt', mass='', analysis='tZq', channel='all')
else:
    cmb.ParseDatacard('COMBINED_datacard_InputVars_'+region+syst_suffix+'.txt', mass='', analysis='tZq', channel='all')

bbb = ch.BinByBinFactory()
bbb.SetVerbosity(1)
bbb.SetAddThreshold(0.05) # Only consider bins with the relative error is >= 0.05
#bbb.SetMergeThreshold(0.5) # Merge at most 50% of the total bin error into a single process

# Now actually create the bbb systematics - specifying that these should only be added for
# background processes only
bbb.AddBinByBin(cmb.cp(), cmb)
#bbb.MergeAndAdd(cmb.cp().backgrounds(), cmb)

# Write the datacard back out. Note that the root file containing the histogram inputs is fully
# recreated and will now contain all the Up/Down shapes for the bin-by-bin too
#cmb.WriteDatacard('COMBINED_datacard_TemplateFit_tZq'+syst_suffix+'_MCstat.txt', '../templates/Combine_Input_tZq'+syst_suffix+'_MCstat.root')

if choice=="0":
    cmb.WriteDatacard('COMBINED_datacard_TemplateFit_tZq'+syst_suffix+'_MCstat.txt', '../templates/Combine_Input_tZq'+syst_suffix+'_MCstat.root')
else:
	cmb.WriteDatacard('COMBINED_datacard_InputVars_'+region+syst_suffix+'_MCstat.txt', '../templates/Control_Histograms_MCstat.root')
