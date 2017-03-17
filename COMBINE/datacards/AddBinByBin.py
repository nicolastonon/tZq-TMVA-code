#!/usr/bin/env python

import CombineHarvester.CombineTools.ch as ch

# Create a CombineHarvester instance and parse an existing datacard
# Important to specify the mass, other metadata values like analysis and channel are optional
cmb = ch.CombineHarvester()
cmb.ParseDatacard('COMBINED_datacard_TemplateFit.txt', mass='', analysis='tZq', channel='all')

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
cmb.WriteDatacard('COMBINED_datacard_TemplateFit_StatError.txt', '../templates/Combine_Input_ScaledFakes_StatError.root')
