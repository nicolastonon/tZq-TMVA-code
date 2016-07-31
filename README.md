############################
#                          #
#         TMVA CODE        #
#                          #
############################

- FIXME / ADD : ...

_____________________________________________________________________________
* This GitHub repository contains the BDT-framework used in the tZq-SM analysis @ 13 TeV.

############################
#                          #
#         FILES            #
#                          #
############################

* The files are :
    - theMVAtool.h & theMVAtool.cc --- this is a class which contains all the methods necessary to the BDT procedure (training, testing, reading = producing BDT templates, creating control trees/histos/plots, generating pseudo-data to stay blind, etc.) ;
    - BDT_analysis.cc --- contains the "int main()" function. Most things can be configured from there (samples, channels, variables, systematics, function calls) ;
    - Makefile --- compiles everything into "./main" ;
    - Func_other.h --- can be used to store secondary functions.

* The two main functionnalities are to create Templates (for theta template fit) and control plots :

    - For templates : [[use Train_Test_Evaluate() if you want BDT Templates]] -> *READ()* -> [[Generate_PseudoData_Histograms_For_Templates() if want to use pseudodata]] -> *Plot_Templates()* ;
    - For CR plots : [[use Train_Test_Evaluate() + Read() + Determine_Control_Cut() if want to cut on BDT]] -> *Create_Control_Trees()* -> *Create_Control_Histograms()* -> *Draw_Control_Plots()*.
_____________________________________________________________________________

/!\ : The code looks for the Ntuples in ./Ntuples/, writes the plots in ./plots/ and writes te output files in ./outputs/

- NB : the Makefile is basic, so as long as these files are in the same directory, it should work anywhere.
/!\ Only make sure to change the "LFLAGS" line in Makefile, so that it looks for the MVA libraries in *your* folder !

/!\ : The code is mostly automatized ---> The most important thing before you execute the code is to make sure you configured the main() correctly to do what you want !
- The options' purposes should be pretty straightforward, but make sure you are consistant within your choices !
Ex : if you're interested in BDT templates in the ttZ CR --> Apply cuts corresponding to the SR, set "bdt_type = BDT_ttZ" for training and "template_name = "BDT_ttZ" for Reading, etc. !
_____________________________________________________________________________
### BDT_analysis.cc : Configuration, int main(), function calls ###

* Configuration options :
    - List of 'cut variables'. These vars will either be "active" or "spectator" in BDT, depending if they're constant or not. Treated separately than 'simple' BDT vars for optimization of loops. Each of these variables must have an entry in v_cut_name/v_cut_def/v_cut_IsUsedForBDT in the same order (an entry is made in v_cut_float in constructor).
    (NB : for ex. if you set NBJets == 0, then btagDiscri becomes constant, so you need to remove it from the BDT variables list !) ;

    - set_luminosity : choose the luminosity in fb-1. Computes a re-scaling factor by which *histograms* are re-scaled ;
    - nofbin_templates : choose the number of bins for template histograms ;
    - fakes_from_data : choose b/w MC or data-driven fakes ;
    - fakes_summed_channels : choose if want to sum channels 2 by 2 wrt the flavour of 3rd lepton (artificial way of ~doubling the stat.)
    - real_data_templates : choose if want to obtain the "DATA" templates from real data or pseudodata ;
    - do_optimization_scan : activates the part of the main() which loops on fuctions for optimization studies. Needs better implementation/configurability

* Set the lists of variables, samples, systematics, channels
    - /!\ : Always include the 3 MC fake samples (DY, TT, WW) at the end of the sample_list, so they are recognized as such ;
    - /!\ : different namings "BDT" & "BDTttZ" --> disambiguation depending on the region, so that theta runs correctly.

############################
#                          #
#      FUNCTION CALLS      #
#                          #
############################

//--- Create instance & Initialize
-> Apply the choices to the class instance.

//--- Training
* Train_Test_Evaluate() : does what it says. Apply the cuts during training. Creates a BDT for each of the 4 channels (in "outputs/BDT{ttZ}_CHANNEL_CUTNAMES.root").

//--- Read & Create Templates
* Read() : performs the Reading. Apply the cuts. Creates the templates (in "outputs/Reader_TEMPLATENAME_CUTNAMES.root").
    - NB : possible to choose b/w "BDT", "BDTttZ", "mTW" or "m3l" templates for now.
    - NB : if fakes_from_data is true, calls Compute_Fake_Ratio() to re-scale the data-driven fakes to the MC fakes yield --> for theta convergence mainly.
    - NB : if fakes_summed_channels is true, sums channels uuu/eeu & eee/uue for DD-fakes sample => ~ double the stat. ! (Making the hypothesis that the template behavior is almost entirely depending on the flavour of the 3rd lepton).
    - NB : if real_data_templates is false, doesn't run on the Data sample, and calls pseudodata function.
* Generate_PseudoData_Histograms_For_Templates() : sums all MC templates in the output file of Read(), generates pseudo-data histograms from them, and writes them in the same file under name "DATA" (theta convention).

//--- Control Trees & Histograms
* Determine_Control_Cut() : reads the output file from Read(). If it contains the BDT templates, determines an appropriate cut value on the BDT discriminant to retain mostly background.
* Create_Control_Trees() : apply the cuts. If specified, cuts also on BDT value determined previously. Fills new trees w/ events passing these cuts (in "outputs/Control_Trees_CUTNAMES.root").
* Create_Control_Histograms() : creates histograms from the control trees, for all channels/samples/variables/syst (in "output/Create_Control_Histograms_CUTNAMES.root"). This can be very long. If for basic checks only, disactivate all syst. & most vars. !
* Generate_PseudoData_Histograms_For_Control_Plots() : for each histo previously created, generates pseudo-data histograms from them, and writes them in the same file under name "Data".

//--- Draw Plots
    - NB : Can choose if sum 4 channels or not. Plots are in /plots repository.
* Draw_Control_Plots() : uses the control histograms to draw control plots.
* Plot_Templates() : uses the histograms from the Read() output file containing the templates & draws them.



_____________________________________________________________________________
############################
#                          #
#           END            #
#                          #
############################
