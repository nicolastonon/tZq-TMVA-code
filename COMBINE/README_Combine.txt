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
-----------------------------------

- Follow instructions from the main README.

- Move to COMBINE/ dir.

- /!\ NB : The 'templates' dir. must contain a file named 'Combine_Input_SUFFIX.root', to which the datacards are pointing to. This file contains [3 regions * 4 channels] = 12 nominal templates (+ all the systematics shifted templates), from which Combine will perform the template fit. Make sure that you're using the right input file. (read the other README.txt to know how to create the templates)
- For Template files, we use the convention : 'Combine_Input_Signal.root' (where signal = 'tZq', 'ttZ', 'tZqANDttZ', 'tZqANDFakes').


-----------------------------------
########     ###    ########    ###     ######     ###    ########  ########   ######
##     ##   ## ##      ##      ## ##   ##    ##   ## ##   ##     ## ##     ## ##    ##
##     ##  ##   ##     ##     ##   ##  ##        ##   ##  ##     ## ##     ## ##
##     ## ##     ##    ##    ##     ## ##       ##     ## ########  ##     ##  ######
##     ## #########    ##    ######### ##       ######### ##   ##   ##     ##       ##
##     ## ##     ##    ##    ##     ## ##    ## ##     ## ##    ##  ##     ## ##    ##
########  ##     ##    ##    ##     ##  ######  ##     ## ##     ## ########   ######
-----------------------------------

3) Move to 'datacards' dir. This directory contains codes for generating/combining all needed datacards. It is from here that we will run the Combine commands.


4) 'Create_Script_Datacard_Generation.cc' generates the scripts to create datacards either to perform the template fit, or to obtain postfit distributions of all input variables, or to get both. It will also ask you if you want to include the systematics, what is your signal (tZq/ttZ/both), and in what region you want to obtain the postfit distributions of input variables.
- Compile code via Makefile :
# make

- NB : in the source code you can modify the variable list (if you're interested in getting postfit distributions of the BDT variables), the datacard name, etc.


5) The created script 'Create_Script_Datacard_Generation.exe' asks input from the user to automatically create a script which will generate the desired datacard :
# ./Create_Script_Datacard_Generation.exe

6) The created script 'Generate... .sh' generates a combined datacard 'COMBINED_datacard_suffix' (either for the template it or to get postfit distributions of input vars) :
# ./Generate... .exe

- NB : could also use directly the 'Generate_Datacards.py' script to generate manually the single datacard you want, with this syntax :
# python generateDatacards.py [CHANNEL] [VARIABLE] [FILE_CONTAINING_HISTOS] [syst_choice] [signal_choice]
((check Generate_Datacards.py to understand what values the options can take))
((the 'Generate... .sh' executables run this script for all channels & variables !))


7) Finally, you can add MC statistical uncertainties manually if you want to (not done in Combine by default) via the CombineHarvester 'AddBinbyBin.py' script. It will add stat. MC error to each histogram's bin. It will ask input from the user in order to look for the right datacard name :
# python addBinbyBin

- NB 1 : This adds a LOT of nuisance parameters (NP) to the fit (1 per histogram bin/process/channel/region) ! That's why the script contains an option 'SetAddThreshold' which you should set e.g. to 0.05, so that a NP is added only if the bin stat. relative uncertainty is >= 5% of total error !
- NB 2 : As a result, the fit takes a LOT more time to converge while the MC stat. uncertainties have been added.


-----------------------------------
 ######   #######  ##     ## ##     ##    ###    ##    ## ########   ######
##    ## ##     ## ###   ### ###   ###   ## ##   ###   ## ##     ## ##    ##
##       ##     ## #### #### #### ####  ##   ##  ####  ## ##     ## ##
##       ##     ## ## ### ## ## ### ## ##     ## ## ## ## ##     ##  ######
##       ##     ## ##     ## ##     ## ######### ##  #### ##     ##       ##
##    ## ##     ## ##     ## ##     ## ##     ## ##   ### ##     ## ##    ##
 ######   #######  ##     ## ##     ## ##     ## ##    ## ########   ######
-----------------------------------
/!\ Make sure you're using the right datacard name in the commands /!\

5)- Compute the a-priori EXPECTED significance w/ a Profile Likelihood :
# combine -M ProfileLikelihood --significance -t -1 --expectSignal=1 COMBINED_datacard.txt
NB : "-t -1" ==> Use Asimov Dataset ; to use toys instead, use "-t N", with N number of toys
NB : for a-posteriori expected signif (uses data & MC), add --toysFreq

- Compute the OBSERVED significance w/ a Profile Likelihood :
# combine -M ProfileLikelihood --signif --cminDefaultMinimizerType=Minuit2 COMBINED_datacard.txt

NB : option -v X (with X>=2) for verbose mode. With X=4, can access postfit POI values & get warning/error messages !



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


10) To produce the postfit histograms for any input variables, we need the file 'mlfit.root' produced by the Maximum Likelihood Fit, a datacard 'COMBINED_datacard_TemplateFit' created via the previous script 'Generate... .sh', and a file containing all the prefit templates of these input variables (just like the files produced via the function 'Create_Control_Histograms') !
- NB : you need to specify yourself the name of the template fit in 'Create_Script_Datacard_Generation.cc' (+ compile/execute it, then execute the created script --> Final datacard with correct file name)
# PostFitShapes -d COMBINED_datacard_TemplateFit.txt -o PostfitInputVars.root -f outputs/mlfit.root:fit_s --postfit --sampling --print



11) To do the break-down of systematics : (Complete instructions here : https://indico.cern.ch/event/577649/contributions/2388797/attachments/1380376/2098158/HComb-Tutorial-Nov16-Impacts.pdf)

- The commands are :

text2workspace.py COMBINED_datacard_TemplateFit.txt
combineTool.py -M Impacts -d COMBINED_datacard_TemplateFit.root -m 125 --doInitialFit
combineTool.py -M Impacts -d COMBINED_datacard_TemplateFit.root -m 125 --doFits --parallel 4
combineTool.py -M Impacts -d COMBINED_datacard_TemplateFit.root -m 125 -o impacts.json
plotImpacts.py -i impacts.json -o impacts


12) To estimate stat. uncertainty contribution : (Complete instructions here : https://indico.cern.ch/event/577649/contributions/2388797/attachments/1380376/2098158/HComb-Tutorial-Nov16-Impacts.pdf)

- The commands are :

combine -M MultiDimFit --algo grid --points 50 --rMin -1 --rMax 4  COMBINED_datacard_TemplateFit_tZq.root -m 125 -n nominal --expectSignal=1 -t -1
combine -M MultiDimFit --algo none --rMin -1 --rMax 4  COMBINED_datacard_TemplateFit_tZq.root -m 125 -n bestfit  --saveWorkspace --expectSignal=1 -t -1
combine -M MultiDimFit --algo grid --points 50 --rMin -1 --rMax 4 -m 125 -n stat higgsCombinebestfit.MultiDimFit.mH125.root --snapshotName MultiDimFit --freezeNuisances all --expectSignal=1 -t -1
./plot1DScan_mine.py --others 'higgsCombinestat.MultiDimFit.mH125.root:Freeze all:2' --breakdown syst,stat higgsCombinenominal.MultiDimFit.mH125.root


13) To compute EXPECTED (cf. '-t -1' options) significance SIMULTANEOUSLY for ttZ & tZq :
text2workspace.py -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel  --PO verbose --PO 'map=.*/tZqmcNLO:r_tZqmcNLO[1,0,10]' --PO 'map=.*/ttZ:r_ttZ[1,0,20]' COMBINED_datacard_TemplateFit_tZqANDttZ.txt -o toy-2d.root
combine toy-2d.root  -M HybridNew --onlyTestStat --testStat=PL --singlePoint r_tZqmcNLO=0 --redefineSignalPOIs r_tZqmcNLO --expectSignal=1 -t -1
combine toy-2d.root -M ProfileLikelihood --signif --redefineSignalPOIs r_tZqmcNLO --setPhysicsModelParameters r_rZqmcNLO=1,r_ttZ=1 --expectSignal=1 -t -1
combine workspace.root -M HybridNew --onlyTestStat --testStat=PL --singlePoint r_tZq=1,r_ttZ=1
combine workspace.root -M HybridNew --onlyTestStat --testStat=PL --singlePoint r_tZq=1,r_ttZ=1 --setPhysicsModelParameters r_rZqmcNLO=1,r_ttZ=1 --expectSignal=1 -t -1





-----------------------------------
##    ##  #######  ######## ########  ######
###   ## ##     ##    ##    ##       ##    ##
####  ## ##     ##    ##    ##       ##
## ## ## ##     ##    ##    ######    ######
##  #### ##     ##    ##    ##             ##
##   ### ##     ##    ##    ##       ##    ##
##    ##  #######     ##    ########  ######
-----------------------------------
//Some additionnal infos


- A shape analysis relies not only on the expected event yields for the signals and backgrounds, but also on the distribution of those events in some discriminating variable. This approach is often convenient with respect to a counting experiment, especially when the background cannot be predicted reliably a-priori, since the information from the shape allows a better discrimination between a signal-like and a background-like excess, and provides an in-situ normalization of the background.

• cf. https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopZSMFCNC13TeV#Statistical_combination_tool

-  Convention of the Histograms : $CHANNEL__$PROCESS     $CHANNEL__$PROCESS__$SYSTEMATIC

* Combine Harvester is a top-level CMSSW Package which contains some additional features. Among those, it can be used to add statistical uncertainties bin per bin (not done by default by combine) with a Barlow-Beeston-like approach. Also, it allows to propagate the changes of the nuisance parameters (NPs) to all input variables of the BDTs (not only the templates used for the fit).


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


-----------------------------------
