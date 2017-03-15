~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
### README FILE W/ MOST BASIC STEPS TO RUN COMBINE ###
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Combine Twiki : https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideHiggsAnalysisCombinedLimit
- A few remarks on Combine / datacards at the end of this README

-----------------------------------
#### ##    ##  ######  ########    ###    ##       ##          ###    ######## ####  #######  ##    ##
 ##  ###   ## ##    ##    ##      ## ##   ##       ##         ## ##      ##     ##  ##     ## ###   ##
 ##  ####  ## ##          ##     ##   ##  ##       ##        ##   ##     ##     ##  ##     ## ####  ##
 ##  ## ## ##  ######     ##    ##     ## ##       ##       ##     ##    ##     ##  ##     ## ## ## ##
 ##  ##  ####       ##    ##    ######### ##       ##       #########    ##     ##  ##     ## ##  ####
 ##  ##   ### ##    ##    ##    ##     ## ##       ##       ##     ##    ##     ##  ##     ## ##   ###
#### ##    ##  ######     ##    ##     ## ######## ######## ##     ##    ##    ####  #######  ##    ##

- Follow instructions here (HiggsAnalysis + CombineHarvester):
http://cms-analysis.github.io/CombineHarvester/index.html#getting-started



-----------------------------------
 ######  ######## ######## ##     ## ########
##    ## ##          ##    ##     ## ##     ##
##       ##          ##    ##     ## ##     ##
 ######  ######      ##    ##     ## ########
      ## ##          ##    ##     ## ##
##    ## ##          ##    ##     ## ##
 ######  ########    ##     #######  ##

1) Once you have installed both HiggsAnalysis & CombineHarvester, move to where you want to put the Combine codes (NB : it must be in a subdir. of CMSSW_7_4_7/src (e.g. CMSSW_7_4_7/src/HiggsAnalysis/CombinedLimit/tzq_analysis/)

2) Copy the necessary codes & files from GitHub (https://github.com/nicolastonon/tZq-TMVA-code).
/!\ You can either clone the entire repository (containing TMVA code, COMBINE folder, + a few codes which are not needed) ; or, if you just want to get the COMBINE directory (e.g. because you already have the TMVA code in some other location), perform a 'sparse checkout' to download only this dir. :
# git init .
# git remote add -f origin https://github.com/nicolastonon/tZq-TMVA-code
# git config core.sparseCheckout true
# echo "COMBINE/" >> .git/info/sparse-checkout
# git pull origin master

- /!\ NB : The 'templates' dir. must contain a file named 'Combine_Input_ScaledFakes.root', to which the datacards are pointing to. This file contains [3 regions * 4 channels] = 12 nominal templates (+ all the systematics shifted templates), from which Combine will perform the template fit. Make sure that you're using the right input file. (read the other README.txt to know how to create the templates)


3) Move to 'datacards' dir. This directory contains codes for generating/combining all needed datacards. It is from here that we will run the Combine commands.


4) In 'Create_Script_Datacard_Generation.cc' :
- This code generates the scripts to create datacards both for template fit & to obtain postfit distributions of all input variables.
- In the code, you can change the path of the histogram file the datacards will point to.
- Make sure the variable list is up-to-date.
- Compile code & execute it :
# g++ Create_Script_Datacard_Generation.cc -o Create_Script_Datacard_Generation.exe `root-config --cflags --glibs`


5) The created script 'Create_Script_Datacard_Generation.exe' automatically generates the appropriate datacards & combines them into 'COMBINED_datacard_suffix' :
# ./Create_Script_Datacard_Generation.exe

6) The 2 created scripts 'Generate... .sh' generate combined datacards either for the template it or to get postfit distributions of input vars :
# ./Generate... .exe


- NB : could also use directly the 'Generate_Datacards.py' script to generate manually the single datacard you want, with this syntax :
# python generateDatacards.py CHANNEL VARIABLE FILE_CONTAINING_HISTOS
((the 'Generate... .sh' executables run this script for all channels & variables !))

-----------------------------------
 ######   #######  ##     ## ##     ##    ###    ##    ## ########   ######
##    ## ##     ## ###   ### ###   ###   ## ##   ###   ## ##     ## ##    ##
##       ##     ## #### #### #### ####  ##   ##  ####  ## ##     ## ##
##       ##     ## ## ### ## ## ### ## ##     ## ## ## ## ##     ##  ######
##       ##     ## ##     ## ##     ## ######### ##  #### ##     ##       ##
##    ## ##     ## ##     ## ##     ## ##     ## ##   ### ##     ## ##    ##
 ######   #######  ##     ## ##     ## ##     ## ##    ## ########   ######


5) Compute the a-priori expected significance w/ a Profile Likelihood :
# combine -M ProfileLikelihood --significance COMBINED_datacard.txt -t -1 --expectSignal=1 -v 4
NB : option -v X (with X>=2) for verbose mode. With X=4, can access postfit POI values & get warning/error messages !
NB : "-t -1" ==> Use Asimov Dataset ; to use toys instead, use "-t N", with N number of toys
NB : for a-posteriori expected signif (uses data & MC), add --toysFreq

6) Perform a Maximum Likelihood Fit (to get access to postfit templates, postfit nuisances, ...) :
NB : need to create outputs dir. first to avoid segfault ;
# combine -M MaxLikelihoodFit --saveShapes --saveWithUncertainties --plot --out outputs COMBINED_datacard.txt

((- With Asimov dataset (useless?) :
# combine -M MaxLikelihoodFit --saveShapes --plot --out outputs --expectSignal=1 -t -1 COMBINED_datacard.txt))

==> This should create a 'output' dir. containing .png plots & a 'mlfit.root' file containing prefit/postfit histos, etc.


8) If you want to plot the pull distributions of the nuisance parameters (to see how they changed postfit), go to the dir. containing the mlfit.root file, & use the Combine script :
# python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py -g nuisances.root mlfit.root

==> This will create a 'nuisances.root' file containing all the necessary info on the NPs

9) To save the plots of these pull distributions, you can copy the 'drawCanvas.C' macro (from 'datacards' dir.) into the dir. containing the 'output.root' file.
--> Executing it should create 2 .png plots containing the useful info on NPs.



-----------------------------------
 ######   #######  ##     ## ########         ##     ##    ###    ########  ##     ## ########  ######  ######## ######## ########
##    ## ##     ## ###   ### ##     ##        ##     ##   ## ##   ##     ## ##     ## ##       ##    ##    ##    ##       ##     ##
##       ##     ## #### #### ##     ##        ##     ##  ##   ##  ##     ## ##     ## ##       ##          ##    ##       ##     ##
##       ##     ## ## ### ## ########         ######### ##     ## ########  ##     ## ######    ######     ##    ######   ########
##       ##     ## ##     ## ##     ##        ##     ## ######### ##   ##    ##   ##  ##             ##    ##    ##       ##   ##
##    ## ##     ## ##     ## ##     ## ###    ##     ## ##     ## ##    ##    ## ##   ##       ##    ##    ##    ##       ##    ##
 ######   #######  ##     ## ########  ###    ##     ## ##     ## ##     ##    ###    ########  ######     ##    ######## ##     ##


(Combine Harvester is a top-level CMSSW Package which contains some additional features. Among those, it can be used to add statistical uncertainties bin per bin (not done by default by combine) with a Barlow-Beeston-like approach. Also, it allows to propagate the changes of the nuisance parameters (NPs) to all input variables of the BDTs (not only the templates used for the fit). )

- AddBinbyBin.py : Add bin by bin statistical MC error to histograms. It uses 'COMBINED_datacard.txt' to  create new datacards taking these errors into account, and also a rootfile :
# python addBinbyBin
(BUG for now ?)

-To produce the postfit histograms for any input variables, we need the file 'mlfit.root' produced by the Maximum Likelihood Fit, a datacard 'COMBINED_datacard_TemplateFit' created via the previous script 'Generate... .sh', and a file containing all the prefit templates of these input variables (just like the files produced via the function 'Create_Control_Histograms') !
NOTE : you need to specify yourself the name of the template fit in 'Create_Script_Datacard_Generation.cc' (+ compile/execute it, then execute the created script --> Final datacard with correct file name)
# PostFitShapes -d COMBINED_datacard_TemplateFit.txt -o PostfitInputVars.root -f outputs/mlfit.root:fit_s --postfit --sampling --print






-----------------------------------
-----------------------------------
*** OTHER COMMANDS

- Calculate the OBSERVED significance :
combine -M ProfileLikelihood --signif --cminDefaultMinimizerType=Minuit2 datacard.txt

- ...






~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*** NOTES ON COMBINE/DATACARDS/...

- A shape analysis relies not only on the expected event yields for the signals and backgrounds, but also on the distribution of those events in some discriminating variable. This approach is often convenient with respect to a counting experiment, especially when the background cannot be predicted reliably a-priori, since the information from the shape allows a better discrimination between a signal-like and a background-like excess, and provides an in-situ normalization of the background.

• cf. https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopZSMFCNC13TeV#Statistical_combination_tool

-  Convention of the Histograms : $CHANNEL__$PROCESS     $CHANNEL__$PROCESS__$SYSTEMATIC


--- ABOUT DATACARDS :

- NB : the basic command to combine 2 datacards describing 2 channels 'var1_uuu'/'var2_uuu' is :
# $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py var1_uuu=datacard1.txt var2_uuu=datacard2.txt  > COMBINED_datacard.txt


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
