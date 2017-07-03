#!/usr/bin/env python

import CombineHarvester.CombineTools.ch as ch
from sys import version_info #To obtain python version automatically

py3 = version_info[0] > 2 #creates boolean value for test that Python major version > 2

if py3:
  signal = input("--- What processes are free in the fit ['0' = tZq / '1' = ttZ / '2' = tZq & ttZ / '3' = tZq & Fakes] ? : ")
  systematics = input("--- With systematics ? (y/n) : ")
else:
  signal = raw_input("--- What processes are free in the fit ['0' = tZq / '1' = ttZ / '2' = tZq & ttZ / '3' = tZq & Fakes] ? : ")
  systematics = raw_input("--- With systematics ? (y/n) : ")

if systematics=="y":
    syst_suffix = ""
else:
    syst_suffix = "_noSyst"

if signal=="0":
  signal_name="_tZq"
  signal_name_template=""
if signal=="1":
  signal_name="_ttZ"
  signal_name_template="_ttZ"
if signal=="2":
  signal_name="_tZqANDttZ"
  signal_name_template=""
if signal=="3":
  signal_name="_tZqANDFakes"
  signal_name_template=""



# Create a CombineHarvester instance and parse an existing datacard
# Important to specify the mass, other metadata values like analysis and channel are optional
cmb = ch.CombineHarvester()
cmb.ParseDatacard('COMBINED_datacard_TemplateFit'+signal_name+syst_suffix+'.txt', mass='', analysis='tZq', channel='all')

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
cmb.WriteDatacard('COMBINED_datacard_TemplateFit'+signal_name+syst_suffix+'_MCstat.txt', '../templates/Combine_Input'+signal_name_template+syst_suffix+'_MCstat.root')
















#!/usr/bin/env python

/*
import CombineHarvester.CombineTools.ch as ch
from sys import version_info #To obtain python version automatically

py3 = version_info[0] > 2 #creates boolean value for test that Python major version > 2


# Create a CombineHarvester instance and parse an existing datacard
# Important to specify the mass, other metadata values like analysis and channel are optional
cmb = ch.CombineHarvester()
cmb.ParseDatacard('datacard_input.txt', mass='', analysis='tZq', channel='all')

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
cmb.WriteDatacard('datacard_output.txt', '../template_path.root')
*/
