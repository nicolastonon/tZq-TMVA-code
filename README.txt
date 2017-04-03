_____________________________________________________________________________
########  ########    ###    ########  ##     ## ########
##     ## ##         ## ##   ##     ## ###   ### ##
##     ## ##        ##   ##  ##     ## #### #### ##
########  ######   ##     ## ##     ## ## ### ## ######
##   ##   ##       ######### ##     ## ##     ## ##
##    ##  ##       ##     ## ##     ## ##     ## ##
##     ## ######## ##     ## ########  ##     ## ########

by Nicolas TONON, IPHC
_____________________________________________________________________________


This GitHub repository contains the TMVA & CombineHiggsTool codes used in the tZq->3l SM analysis @ 13 TeV.

####################
--- WARNINGS ---
####################

/!\ In BDT_analysis.cc
- Be careful to set properly all the configurable paths, booleans & other options in the main !!!
- Also notice that all function calls are done via dedicated booleans (cf. section "Function Calls" in code) !

((- Also : make sure to keep all the independant codes synchronized : sample list, etc. (e.g. if you want to use interfacing code, standalone code 'scaleFakes.cc', etc.) ))


_____________________________________________________________________________
  #####  ####### ####### #     # ######
 #     # #          #    #     # #     #
 #       #          #    #     # #     #
  #####  #####      #    #     # ######
       # #          #    #     # #
 #     # #          #    #     # #
  #####  #######    #     #####  #
_____________________________________________________________________________

1) If you want to use the Combine Framework, first install it following these instructions : (to install 'HiggsAnalysis' + 'CombineHarvester'):
http://cms-analysis.github.io/CombineHarvester/index.html#getting-started

2) Choose the location where you will copy the code. If you will use Combine, the code must be in a subdir. of CMSSW_7_4_7/src (e.g. CMSSW_7_4_7/src/HiggsAnalysis/CombinedLimit/tzq_analysis/)

3) Copy the necessary codes & files from GitHub (https://github.com/nicolastonon/tZq-TMVA-code).
/!\ You can either clone the entire repository (containing TMVA code, COMBINE folder, + a few codes which are not needed) ; or, if you just want to get the COMBINE directory (e.g. because you already have the TMVA code in some other location), perform a 'sparse checkout' to download only this dir. :
# git init .
# git remote add -f origin https://github.com/nicolastonon/tZq-TMVA-code
(# git config core.sparseCheckout true)
(# echo "COMBINE/" >> .git/info/sparse-checkout)
# git pull origin master


_____________________________________________________________________________

 ###### #####    ##   #    # ###### #    #  ####  #####  #    #
 #      #    #  #  #  ##  ## #      #    # #    # #    # #   #
 #####  #    # #    # # ## # #####  #    # #    # #    # ####
 #      #####  ###### #    # #      # ## # #    # #####  #  #
 #      #   #  #    # #    # #      ##  ## #    # #   #  #   #
 #      #    # #    # #    # ###### #    #  ####  #    # #    #
_____________________________________________________________________________

* All codes are compiled via the Makefile. (/!\ Modify the path to your TMVA libraries !)

* All input Ntuples are placed in 'input_ntuples'. /!\ Currently, we use 2 different sets of Ntuples : for WZ region (no MEM), and ttZ+tZq regions (w/ MEM). This can be modified in the constructor of theMVAtool ('dir_ntuples' member), e.g. if you want to use a single set of ntuples for all 3 regions.

* All output .root files are placed in 'outputs/' (e.g. templates, control histograms, ...).

* All plots are placed in 'plots/'.

* All TMVA BDT weight files are placed in 'weights/'

* For disambiguation, output files (templates, BDT, ...) contain a suffix in their names, indicating which set of cuts they correspond to. These suffix are used throughout the code to make sure the right input files are used.
---> e.g. 'Reader_mTW_NJetsMin0_NBJetsEq0.root' is the file containing the templates in the WZ regions (suffix <-> No bjets, at least 1 light jet).

* NOTE : the regions / template we use are :
- WZ Control Region --> == 0 bjet, > 0 total jets --> mTW templates
- ttZ Control Region --> > 1 bjet, > 1 total jets --> 'BDTttZ' templates
- tZq Signal Region --> == 1 bjet, >1 && <4 total jets --> 'BDT' templates
_____________________________________________________________________________
  ####      ####     #####     ######     ####
 #    #    #    #    #    #    #         #
 #         #    #    #    #    #####      ####
 #         #    #    #    #    #              #
 #    #    #    #    #    #    #         #    #
  ####      ####     #####     ######     ####
_____________________________________________________________________________

* The main code is contained in the theMVAtool user-class (--> theMVAtool.cc/h).
This class contains all the necessary functions to perform our Template Fit Analysis (e.g. BDT training, Template generation, CR plots generation, etc.).


* The main() function is contained in 'BDT_analysis.cc'. It is there that everything must be configured to obtain what you want. The main sections are :

--- OPTIONS => Set the different parameters of the analysis (e.g. : # of template bins).

--- REGION & CUTS => There are 3 regions in our analysis : CR WZ, CR ttZ & SR tZq. You can select it manually here, or specify the region at execution (e.g. './BDT_analysis.exe tZq'). This will automatically cut on the numbers of jets/b-jets accordingly. You can also add other cuts yourself (e.g. MET > 20) in the regions you want.

--- DIR. PATH => /!\ Here you need to specify your own path to your Ntuples !! Be sure to also choose the proper default tree naming ! (check it in Ntuples)

--- CHANNELS => ...
--- SAMPLES => ...
--- SYSTEMATICS => ...

--- BDT VARS => Specify the variables that are going to be used by BDT. (NB : only for tZq/ttZ regions)

--- OTHER VARS => Can add additional variables not used in BDT, only for control plots.

--- FUNCTION CALLS => This is where the functions are called. Simply set to 'TRUE' the booleans activating the functions you want to use. Then all the function calls are made automatically from there.


--- OPTIMIZATION => This is a simple loop on 2 sets of cuts, which allows to produce templates in the selected region with different cuts ==> Can then perform template fit with all the different files, and see which set of cuts maximizes the significance !
- NB : only does training + template creation (needed for comparison b/w cut sets)
- NB : to use the optimization loop, need to set corresponding boolean in 'Options' section ;
- NB : can easily loop on more than two sets of cuts, by adding loops !

---------------------------

* The './auto_BDTanalysis_3regions' script executes automatically the activated functions, in all 3 regions.
==> USE THIS SCRIPT WHEN YOU WANT TO RUN CODE IN ALL 3 REGIONS !

* The 'merge_templates_combine.sh' script combines the 3 'Reader...' (= template) files
=> This gives the input file we're going to feed to Combine !
- NOTE 1 : move this script to your 'outputs' dir. for it to work !
- NOTE 2 : script 'merge_templates_theta.sh' does the same thing, but for Reader files produced with theta naming conventions (__Theta.root files produced via function 'Convert_Templates_Theta')


* Func_other.h can be used to store secondary/helper functions.

((* Other codes are not described/used here))



_____________________________________________________________________________
 ####### #     # #     #  #####  ####### ### ####### #     #  #####
 #       #     # ##    # #     #    #     #  #     # ##    # #     #
 #       #     # # #   # #          #     #  #     # # #   # #
 #####   #     # #  #  # #          #     #  #     # #  #  #  #####
 #       #     # #   # # #          #     #  #     # #   # #       #
 #       #     # #    ## #     #    #     #  #     # #    ## #     #
 #        #####  #     #  #####     #    ### ####### #     #  #####
_____________________________________________________________________________

The main functions of theMVAtool are :

--- TRAINING

* Train_Test_Evaluate : Trains a BDT from list of variables, in chosen region. Creates weight files (necessary to then compute BDT discriminants) & TMVA control '.root' files.

--- RE-SCALING

* Compute_Fake_SF & Rescale_Fake_Histograms : use TFractionFitter class to re-scale fakes to data, constraining all other backgrounds. This is needed in order to get a reasonable prefit normalization of the fakes (so that the control plots make sense, and the template fit will converge correctly).


--- Templates Creation

* Read() : This function creates an output file of the type 'Reader_templatename_suffix.root', containing the template histograms for the chosen region.


--- Control Trees & Histograms

* Create_Control_Trees : creates an output file of the type 'Control_Trees_suffix.root', containing 1 tree for each sample. Each tree contains all the variables distributions. They are filled with events from the chosen region ; in the ttZ & tZq regions (which contain signal & where BDTs have been trained), it is possible to perform a cut on the BDT discriminant in order to later create control plots.

* Create_Control_Histograms : creates an output file of the type 'Control_Histograms_sufix.root', containing 1 histogram for all samples/channels/variables/systematics, obtained from the 'Control_Tree' file.
- NB : /!\ When systematics are included, running this function is highly CPU-intensive !


--- Draw Plots
- Use booleans to choose b/w prefit or postfit plots, and b/w plots of templates or of BDT input variables
(NB : different files are used in each case !)

* Draw_Control_Plots : convert all prefit histograms from the 'Control_Histograms' file into plots. If boolean 'postfit' is TRUE, creates postfit plots from the CombineHarvester 'test.root' file.

* Plot_Templates : creates plots from the root file containing all prefit templates (the one we feed to Combine).

* Plot_Templates_from_Combine : creates plots from the 'mlfit.root' file created by Combine when a Maximum Likelihood Fit is performed (with the right options).
NOTE : You need to retrieve this input file yourself, and move it to 'outputs/'.

* ...


--- Convert Templates names for Theta

* Convert_Templates_Theta : takes Combine templates files in input (located in dir. 'outputs/'), and simply changes the name of the histograms to follow Theta conventions (creates new file).




_____________________________________________________________________________
  #####                                             #             #       ####### #     # #     #    #
 #     #  ####  #    # #####  # #    # ######      #               #         #    ##   ## #     #   # #
 #       #    # ##  ## #    # # ##   # #          #                 #        #    # # # # #     #  #   #
 #       #    # # ## # #####  # # #  # #####     #    ##### #####    #       #    #  #  # #     # #     #
 #       #    # #    # #    # # #  # # #          #                 #        #    #     #  #   #  #######
 #     # #    # #    # #    # # #   ## #           #               #         #    #     #   # #   #     #
  #####   ####  #    # #####  # #    # ######       #             #          #    #     #    #    #     #
_____________________________________________________________________________

### The basic 'complete' chain of commands/actions one should perform to do the Template Fit is the following :

1) Configure 'BDT_analysis.cc' properly : options, samples, systematics, etc.
(NOTE : on first use, you should at least give a look to every options)

2) Activate 'Train_Test_Evaluate' & 'Read' functions. Run the 'auto_BDTanalysis_3regions.sh' script (= all 3 regions)
---> Creates 3 'Reader' files in 'outputs/'.

3) Move to 'outputs/' & run the script 'merge_templates_combine.sh'
---> Creates file Combine_Input.root from the 3 previous files.
===> This is the input file we're going to feed to Combine !


----------- CHECK THE COMBINE README.TXT FOR THE NEXT STEPS -----------
5) Move this file to <your_combine_folder>/templates/ .

5) Do as instructed on the other README to perform the template fit :
- Generate the Datacards
- Combine them
- Perform a Profile Likelihood Fit to get the observed & expected significance, or a Maximum Likelihood Fit to get the signal strength & the postfit distributions/nuisance parameters.




--- 6) Additionally, if you want to plot the prefit/postfit templates :
NB :  need to have the following files in 'outputs/' dir. : Combine_Input.root (prefit) & mlfit.root obtained from Maximum Likelihood Fit (postfit)

--> Simply set the booleans properly, and run script './auto_BDTanalysis_3regions.sh' (all 3 regions)to draw plots !


--- 7) Additionally, if you want to plot the prefit/postfit distributions of all variables :
NB :  need to have the following files in 'outputs/' dir. : Create_Control_Histograms.root (prefit) & test.root obtained from CombineHarvester (postfit)
NB : to obtain Create_Control_Histograms.root, first need to run functions 'Create_Control_Trees' & 'Create_Control_Histograms' from BDT_analysis.cc ; then run script 'auto_BDTanalysis_3regions.sh'.

--> Simply set the booleans properly, and run script './auto_BDTanalysis_3regions.sh' (all 3 regions) to draw plots !






------------------------------
######## ##     ## ########       ######## ##    ## ########
   ##    ##     ## ##             ##       ###   ## ##     ##
   ##    ##     ## ##             ##       ####  ## ##     ##
   ##    ######### ######         ######   ## ## ## ##     ##
   ##    ##     ## ##             ##       ##  #### ##     ##
   ##    ##     ## ##             ##       ##   ### ##     ##
   ##    ##     ## ########       ######## ##    ## ########
