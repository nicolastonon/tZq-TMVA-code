~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
### README FILE W/ MOST BASIC STEPS TO RUN COMBINE ###
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*** SETUP

1) Go to your Combine dir (e.g. : .../login/CMSSW_7_4_7/src/HiggsAnalysis/CombinedLimit/tzq_analysis)

2) Create directories 'datacards' & 'templates'

3) From /afs/cern.ch/user/n/ntonon/public/forJeremy :

--> Copy 'datacards_ttZ', 'datacards_tZq', 'datacards_WZ' to your 'datacards' dir. ; each contains 4 datacards (1/channel)
NB : Basically, we have 1 datacard / per region & channel (3*4=12 in total). Each contains the names of processes, systematics which apply to each of them, etc. We also give the name of the rootfile containing all the templates to fit.

--> Copy 'Combine_Input_ScaledFakes.root' (that's the input file for the fit, w/ all templates in 3 regions) to 'templates' dir.

4) Go to dir. 'datacards'

-----------------------------------
*** COMMANDS

5) First, combine all the datacards together :
# python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py BDT_eee=datacards_tZq/datacard_eee.txt BDT_uuu=datacards_tZq/datacard_uuu.txt BDT_eeu=datacards_tZq/datacard_eeu.txt BDT_uue=datacards_tZq/datacard_uue.txt BDTttZ_eee=datacards_ttZ/datacard_eee.txt BDTttZ_uuu=datacards_ttZ/datacard_uuu.txt BDTttZ_eeu=datacards_ttZ/datacard_eeu.txt BDTttZ_uue=datacards_ttZ/datacard_uue.txt mTW_eee=datacards_WZ/datacard_eee.txt mTW_uuu=datacards_WZ/datacard_uuu.txt mTW_eeu=datacards_WZ/datacard_eeu.txt mTW_uue=datacards_WZ/datacard_uue.txt > COMBINED_datacard.txt

6) Compute the a-priori expected significance w/ a Profile Likelihood :
# combine -M ProfileLikelihood --significance COMBINED_datacard.txt -t -1 --expectSignal=1 -v 4
NB : option -v X (with X>=2) for verbose mode. With X=4, can access postfit POI values & get warning/error messages !
NB : "-t -1" ==> Use Asimov Dataset ; to use toys instead, use "-t N", with N number of toys
NB : for a-posteriori expected signif (uses data & MC), add --toysFreq

7) Perform a Maximum Likelihood Fit (to get access to postfit templates, postfit nuisances, ...) :
NB : /!\ uses real data !
NB : if segfault, try again once or twice (... !)
# combine -M MaxLikelihoodFit --skipBOnlyFit --saveShapes --saveWithUncertainties --plot --out outputs COMBINED_datacard.txt

- With Asimov dataset (doesn't use data but also gives no info... only useful to try if everything runs ?):
# combine -M MaxLikelihoodFit --saveShapes --plot --out outputs --expectSignal=1 -t -1 COMBINED_datacard.txt

==> This should create a 'output' dir. containing .png plots & a 'mlfit.root' file containing prefit/postfit histos, etc.


8) If you want to plot the pull distributions of the nuisance parameters (to see how they changed postfit), go to the dir. containing the mlfit.root file, & use the Combine script :
# python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py -g output.root mlfit.root

==> This will create a 'output.root' file containing all the necessary info on the npoints

9) To save the plots of these pull distributions, you can copy the 'drawCanvas.C' macro (still from /afs/cern.ch/user/n/ntonon/public/forJeremy) into the dir. containing the 'output.root' file.
Executing it should create 2 .png plots containing the useful info on NPs.

-----------------------------------
-----------------------------------
*** OTHER COMMANDS

- Calculate the OBSERVED significance :
combine -M ProfileLikelihood --signif --cminDefaultMinimizerType=Minuit2 datacard.txt

- ...



-----------------------------------
-----------------------------------
*** COMBINE HARVES










~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*** NOTES ON COMBINE/DATACARDS/...

- A shape analysis relies not only on the expected event yields for the signals and backgrounds, but also on the distribution of those events in some discriminating variable. This approach is often convenient with respect to a counting experiment, especially when the background cannot be predicted reliably a-priori, since the information from the shape allows a better discrimination between a signal-like and a background-like excess, and provides an in-situ normalization of the background.

• cf. https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopZSMFCNC13TeV#Statistical_combination_tool

-  Convention of the Histograms : $CHANNEL__$PROCESS     $CHANNEL__$PROCESS__$SYSTEMATIC


--- ABOUT DATACARDS :

-   imax defines the number of final states analyzed (one in this case, but datacards can also contain multiple channels)
    jmax defines the number of independent physical processes whose yields are provided to the code, minus one (i.e. if you have 2 signal processes and 5 background processes, you should put 6)
    kmax defines the number of independent systematical uncertainties (these can also be set to * or -1 which instruct the code to figure out the number from what's in the datacard)

-  For each given source of shape uncertainty, in the part of the datacard  containing shape uncertainties (last block), there must be a row
•  name_ shape _effect for each process and channel
 The effect can be "-" or 0 for no effect, 1 for normal effect,  and possibly something different from 1 to test larger or smaller effects (in that case, the unit gaussian is scaled by that factor before using it as parameter for the interpolation) The block of lines defining the mapping (first block in the datacard) contains one or more rows in the form
•   shapes  process channel file histogram [ histogram_with_systematics ]
  In this line
•  process is any one the process names, or * for all processes, or data_obs for the observed data
 •  channel is any one the process names, or * for all channels
 •  file_, histogram and _histogram_with_systematics identify the names of the files and of the histograms within the file, after doing some replacements (if any are found): •  $PROCESS is replaced with the process name (or "data_obs" for the observed data)
 •  $CHANNEL is replaced with the channel name
 •  $SYSTEMATIC is replaced with the name of the systematic + (Up, Down)
 •  $MASS is replaced with the higgs mass value which is passed as option in the command line used to run the limit tool
