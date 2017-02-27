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

_____________________________________________________________________________

 ###### #####    ##   #    # ###### #    #  ####  #####  #    #
 #      #    #  #  #  ##  ## #      #    # #    # #    # #   #
 #####  #    # #    # # ## # #####  #    # #    # #    # ####
 #      #####  ###### #    # #      # ## # #    # #####  #  #
 #      #   #  #    # #    # #      ##  ## #    # #   #  #   #
 #      #    # #    # #    # ###### #    #  ####  #    # #    #
_____________________________________________________________________________

* All codes are compiled via the Makefile. (/!\ Modify the path to your TMVA libraries)

* All input Ntuples are placed in 'input_ntuples'. /!\ Currently, we use 2 different sets of Ntuples : for WZ region (no MEM), and ttZ+tZq regions (w/ MEM). This can be modified in the constructor of theMVAtool ('dir_ntuples' member), e.g. if you want to use a single set of ntuples for all 3 regions.

* All output .root files are placed in 'outputs/' (e.g. templates, control histograms, ...).

* All plots are placed in 'plots/'.

* All TMVA BDT weight files are placed in 'weights/'

* For disambiguation, output files (templates, BDT, ...) contain a suffix in their names, indicating which set of cuts they correspond to. These suffix are used throughout the code to make sure the right input files are used.
---> e.g. 'Reader_mTW_NJetsMin0_NBJetsEq0.root' is the file containing the templates in the WZ regions (suffix <-> No bjets, at least 1 light jet).

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

((* NOTE : the template we use are :
- WZ Control Region --> 'mTW'
- ttZ Control Region --> 'BDTttZ'
- tZq Signal Region --> 'BDT'))

* The main() function is contained in 'BDT_analysis.cc'. It is there that everything must be configured to obtain what you want. The main sections are :

--- OPTIONS => Set the different parameters of the analysis (e.g. : # of template bins).

--- REGION & CUTS => There are 3 regions in our analysis : CR WZ, CR ttZ & SR tZq. You can select it manually here, or specify the region at execution (e.g. ./BDT_analysis.exe tZq). This will automatically cut on the numbers of jets/b-jets accordingly. You can also add other cuts yourself (e.g. MET > 20).

--- CHANNELS => ...
--- SAMPLES => ...
--- SYSTEMATICS => ...

--- BDT VARS => Specify the variables that are going to be used by BDT. NB : only for tZq/ttZ regions.

--- OTHER VARS => Can add additional variables not used in BDT, only for control plots.

--- FUNCTION CALLS => This is where the functions are called. Simply un-comment the functions you want to use. /!\ Be sure to set properly the few function-dependant options which are initialized there.

((--- OPTIMIZATION => This is a simple loop on the function calls, in all 3 regions, where only the cuts vary. This part can be used in order to find optimized cuts.))



* The './auto_BDTanalysis_3regions' script executes automatically the un-commented functions in all 3 regions.

* The 'combine_templates.sh' script combines the 3 'Reader' (= template) files => This gives the input file we're going to feed to Combine !
NOTE 1 : need to rescale the data-driven Fakes first !
NOTE 2 : move this script to your 'outputs' dir. for it to work !

* 'scaleFakes.cc' is used to re-scale the data-driven fakes to a reasonable initial value, using TFractionFitter. You can either choose manually the file containing the histos to rescale, or at execution (e.g. ./scaleFakes.exe file.root). This will create an output file with the extention '_ScaledFakes.root'.

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

* Train_Test_Evaluate : Trains a BDT from list of variables, in chosen region. Creates weight files (necessary to then compute BDT discriminants) & TMVA control '.root' files.

* Read() : This function creates an output file of the type 'Reader_templatename_suffix.root', containing the template histograms for the chosen region.

* Create_Control_Trees : creates an output file of the type 'Control_Trees_suffix.root', containing 1 tree for each sample. Each tree contains all the variables distributions. They are filled with events from the chosen region ; in the ttZ & tZq regions (which contain signal & where BDTs have been trained), it is possible to perform a cut on the BDT discriminant in order to later create control plots.

* Create_Control_Histograms : creates an output file of the type 'Control_Histograms_sufix.root', containing 1 histogram for all samples/channels/variables/systematics, obtained from the 'Control_Tree' file.
- NB : /!\ When systematics are included, running this function is highly CPU-intensive !

* Draw_Control_Plots : convert all prefit histograms from the 'Control_Histograms' file into plots. If boolean 'postfit' is TRUE, creates postfit plots from the CombineHarvester 'test.root' file.

* Plot_Templates : creates plots from the root file containing all prefit templates (the one we feed to Combine).

* Plot_Templates_from_Combine : creates plots from the 'mlfit.root' file created by Combine when a Maximum Likelihood Fit is performed (with the right options).
NOTE : You need to retrieve this input file yourself, and move it to 'outputs/'.

* ...




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

1) Configure 'BDT_analysis.cc' properly : options, samples, etc.
(NOTE : on first use, you should at least give a look to every options)

2) Activate 'Train_Test_Evaluate' & 'Read' functions. Run the 'auto_BDTanalysis_3regions.sh' script (= all 3 regions)
---> Creates 3 'Reader' files in 'outputs/'.

3) Move to 'outputs/' & run the script 'combine_templates.sh'
---> Creates file Combine_Input_noScale.root from the 3 previous files.

4) Move back to previous dir., and run './scaleFakes.exe outputs/Combine_Input_noScale.root'
---> Creates file Control_Histograms_NJetsMin0_NBJetsEq0_ScaledFakes.root in 'outputs/'.

===> This is the input file we're going to feed to Combine !


----------- CHECK THE COMBINE README.TXT FOR THE NEXT STEPS -----------
5) Move this file to <your_combine_folder>/templates/ .

5) Do as instructed on the other README to perform the template fit :
- Generate the Datacards
- Combine them
- Perform a Profile Likelihood Fit to get the expected significance, or a Maximum Likelihood Fit to get the signal strength & the postfit distributions/nuisance parameters.




--- 6) Additionally, if you want to plot the prefit/postfit templates :
- Move the 'mlfit.root' file you got from Combine to 'outputs/'

PREFIT : In 'BDT_analysis.cc' activate function 'Plot_Templates' (un-comment the line, & set boolean 'draw_plots = true'). Then run script 'auto_BDTanalysis_3regions.sh'

POSTFIT : idem with function 'Plot_Templates_from_Combine'.


--- 7) Additionally, if you want to plot the prefit/postfit distributions of all variables :
- Move the 'test.root' file you got from CombineHarvester to 'outputs/'

PREFIT :
- Activate functions 'Create_Control_Trees' & 'Create_Control_Histograms'. Run the script 'auto_BDTanalysis_3regions.sh'.
---> Both create files in 'outputs/'
- (Disactive previous functions)
- Activate 'Draw_Control_Plots' (un-comment the line, set boolean 'draw_plots = true' & boolean 'postfit = false'). Then run script 'auto_BDTanalysis_3regions.sh'

POSTFIT : repeat last step, with boolean 'draw_plots = true'.





------------------------------
######## ##     ## ########       ######## ##    ## ########
   ##    ##     ## ##             ##       ###   ## ##     ##
   ##    ##     ## ##             ##       ####  ## ##     ##
   ##    ######### ######         ######   ## ## ## ##     ##
   ##    ##     ## ##             ##       ##  #### ##     ##
   ##    ##     ## ##             ##       ##   ### ##     ##
   ##    ##     ## ########       ######## ##    ## ########
