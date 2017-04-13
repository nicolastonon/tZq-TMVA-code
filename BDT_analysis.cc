#include "theMVAtool.h"
#include "Func_other.h" //Helper functions

using namespace std;

int main(int argc, char **argv) //Can choose region (tZq/WZ/ttZ) at execution
{
    if(argc > 2 || (strcmp(argv[1],"tZq") && strcmp(argv[1],"ttZ") && strcmp(argv[1],"WZ") ) ) //String comparison -- strcmp returns 0 if both contents are equal
    {
        cout<<BOLD(FRED("Error : wrong arguments at execution !"))<<endl;
        cout<<"argc = "<<argc<<endl;
        cout<<"argv[1] = "<<argv[1]<<endl;
        return 1;
    }

//---------------------------------------------------------------------------
//  #######  ########  ######## ####  #######  ##    ##  ######
// ##     ## ##     ##    ##     ##  ##     ## ###   ## ##    ##
// ##     ## ##     ##    ##     ##  ##     ## ####  ## ##
// ##     ## ########     ##     ##  ##     ## ## ## ##  ######
// ##     ## ##           ##     ##  ##     ## ##  ####       ##
// ##     ## ##           ##     ##  ##     ## ##   ### ##    ##
//  #######  ##           ##    ####  #######  ##    ##  ######
//---------------------------------------------------------------------------


    //If true, activates only the "optimization" part (@ end of file)
    bool do_optimization_cuts = false;
    bool RemoveBDTvars_CreateTemplates_ExtractSignif = false;


    //Luminosity -- NB : A SCALE FACTOR IS COMPUTED W.R.T MORIOND2017 LUMI !!!
    double set_luminosity = 35.68; //Moriond 2017
    // double set_luminosity = 12.9; //Summer 2016
    // double set_luminosity = 35.68 - 8.605; //Moriond 2017 without Run H
    // double set_luminosity = 8.605; //Run H only

    //Matrix Element Method
    bool include_MEM_variables = false;


    //Training
    bool use_ttZaMCatNLO_training = true; //Choose ttZ training sample (false --> Madgraph sample)

    //Templates
    int nofbin_templates = 10; //Templates binning (to be optimized)
    bool real_data_templates = true; //If true, use real data sample to create templates (BDT, mTW, ...) / else, use pseudodata !
    bool cut_on_BDT = false; //Apply cut on BDT values --> Don't look signal region !

    //Fakes
    bool fakes_from_data = true; //Use MC fakes or data-driven fakes)
    bool fakes_summed_channels = true; //Sum uuu+eeu & eee+uue --> Double the fake stat. (artificially)!

    //Outputs
    TString format = ".png"; //.png or .pdf only
    bool combine_naming_convention = true; //To write histograms with Combine names (else, follow Theta conventions)

//-------------------



//-----------------------------------------------------------------------------------------
// ########  ########  ######   ####  #######  ##    ##         ####           ######  ##     ## ########  ######
// ##     ## ##       ##    ##   ##  ##     ## ###   ##        ##  ##         ##    ## ##     ##    ##    ##    ##
// ##     ## ##       ##         ##  ##     ## ####  ##         ####          ##       ##     ##    ##    ##
// ########  ######   ##   ####  ##  ##     ## ## ## ##        ####           ##       ##     ##    ##     ######
// ##   ##   ##       ##    ##   ##  ##     ## ##  ####       ##  ## ##       ##       ##     ##    ##          ##
// ##    ##  ##       ##    ##   ##  ##     ## ##   ###       ##   ##         ##    ## ##     ##    ##    ##    ##
// ##     ## ########  ######   ####  #######  ##    ##        ####  ##        ######   #######     ##     ######
//-----------------------------------------------------------------------------------------
//Specify here the cuts that you wish to apply to all 3 regions.
//To dis-activate a cut, just set it to "". Use "==" for equality.
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4"); set_v_cut_IsUsedForBDT.push_back(false);

    std::vector<TString > set_v_cut_name;
    std::vector<TString > set_v_cut_def;
    std::vector<bool > set_v_cut_IsUsedForBDT;

//-------------------
    // set_v_cut_name.push_back("");      set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);}

//-------------------


// CAN CHOOSE REGION EITHER HERE IN CODE, OR AT EXECUTION !

    //Region selection -- manual (set 1 region to TRUE)
    bool tZq_region = false;
    bool ttZ_region = false;
    bool WZ_region = false;



    //Manual region selection is overwritten if arguments are given during execution
    if(argc==2)
    {
        if(!strcmp(argv[1],"tZq")) {tZq_region = true; WZ_region = false; ttZ_region = false;}
        else if(!strcmp(argv[1],"WZ")) {tZq_region = false; WZ_region = true; ttZ_region = false;}
        else if(!strcmp(argv[1],"ttZ")) {tZq_region = false; WZ_region = false; ttZ_region = true;}
    }

    if( (!tZq_region && !WZ_region && !ttZ_region) || (tZq_region && WZ_region) || (tZq_region && ttZ_region) || (WZ_region && ttZ_region) ) {cout<<__LINE__<<" BDT_analysis : Problem : wrong region ! Exit"<<endl; return 0;}

    //--- DON'T CHANGE THIS : Region choice automatized from here !
    bool isttZ = false; bool isWZ = false;
    if(WZ_region) {isWZ = true;}
    else if(ttZ_region) {isttZ = true;}



//--- DON'T CHANGE Jets cuts here (define the 3 different regions)
//But you can add some cuts which are region-dependant !
    if(!isWZ && !isttZ) //Default selection is Signal Region : 1<NJets<4 && NBJets == 1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1 && <4");     set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back("==1");          set_v_cut_IsUsedForBDT.push_back(true); //NB : cst -> not actually used in BDT
    }
    if(isWZ) //WZ CR Region : NJets > 0 && NBJets == 0
    {
        set_v_cut_name.push_back("NJets");      set_v_cut_def.push_back(">0");          set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");     set_v_cut_def.push_back("== 0");        set_v_cut_IsUsedForBDT.push_back(true); //NB : cst -> not actually used in BDT
    }
    if(isttZ) //ttZ CR Region : NJets>1 && NBJets>1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1");           set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back(">1");           set_v_cut_IsUsedForBDT.push_back(true);
    }

//---------------------



//---------------------------------------------------------------------------
// ########  #### ########            ########     ###    ######## ##     ##
// ##     ##  ##  ##     ##           ##     ##   ## ##      ##    ##     ##
// ##     ##  ##  ##     ##           ##     ##  ##   ##     ##    ##     ##
// ##     ##  ##  ########            ########  ##     ##    ##    #########
// ##     ##  ##  ##   ##             ##        #########    ##    ##     ##
// ##     ##  ##  ##    ##  ###       ##        ##     ##    ##    ##     ##
// ########  #### ##     ## ###       ##        ##     ##    ##    ##     ##
//---------------------------------------------------------------------------
//Set here the path of the directory containing all the Ntuples.
//NB : we define the path here, because Ntuples might be placed in different directories depending on the region we're interested in
//e.g. to include MEM in analysis, need to have 2 different sets of Ntuples : one for WZ region (no MEM), and one for ttZ+tZq regions (with MEM) !

    TString dir_ntuples; TString t_name;

//FIXME --- BE SURE TO CHOOSE PROPER FILEPATHS, TREE NAME !


    if(include_MEM_variables) //--- IPHC : Ntuples Interfaced for MEM, divided in 2 sets (WZ region and ttZ+tZq regions)
    {
        if(isWZ) 	dir_ntuples="input_ntuples/ntuples_WZ"; //Without MEM (empty vars)
        else 		dir_ntuples="input_ntuples/ntuples_MEM"; //With MEM
        t_name = "Tree";
    }

    else //--- CIEMAT : Default Ntuples
    {
        dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/input_ntuples";
        t_name = "Default";
    }




//---------------------------------------------------------------------------
//  ######  ##     ##    ###    ##    ## ##    ## ######## ##        ######
// ##    ## ##     ##   ## ##   ###   ## ###   ## ##       ##       ##    ##
// ##       ##     ##  ##   ##  ####  ## ####  ## ##       ##       ##
// ##       ######### ##     ## ## ## ## ## ## ## ######   ##        ######
// ##       ##     ## ######### ##  #### ##  #### ##       ##             ##
// ##    ## ##     ## ##     ## ##   ### ##   ### ##       ##       ##    ##
//  ######  ##     ## ##     ## ##    ## ##    ## ######## ########  ######
//---------------------------------------------------------------------------
    std::vector<TString > thechannellist;

    thechannellist.push_back("uuu");
    thechannellist.push_back("eeu");
    thechannellist.push_back("uue");
    thechannellist.push_back("eee");

//---------------------------------------------------------------------------
//  ######     ###    ##     ## ########  ##       ########  ######
// ##    ##   ## ##   ###   ### ##     ## ##       ##       ##    ##
// ##        ##   ##  #### #### ##     ## ##       ##       ##
//  ######  ##     ## ## ### ## ########  ##       ######    ######
//       ## ######### ##     ## ##        ##       ##             ##
// ##    ## ##     ## ##     ## ##        ##       ##       ##    ##
//  ######  ##     ## ##     ## ##        ######## ########  ######
//---------------------------------------------------------------------------
    std::vector<TString> thesamplelist;
    std::vector<int> v_color; //sample <-> color

//-------------------
    //DATA --- THE DATA SAMPLE MUST BE UNIQUE AND IN FIRST POSITION
    thesamplelist.push_back("Data");

    //Signal --- must be placed before backgrounds
    thesamplelist.push_back("tZqmcNLO");             v_color.push_back(kGreen+2);

    //BKG
    thesamplelist.push_back("WZL");             v_color.push_back(920); //grey
    thesamplelist.push_back("WZB");             v_color.push_back(921); //grey
    thesamplelist.push_back("WZC");             v_color.push_back(922); //grey
    thesamplelist.push_back("ZZ");              v_color.push_back(kYellow);
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed); //Keep 3 'red' samples together for plots
    thesamplelist.push_back("ttW");             v_color.push_back(kRed);
    thesamplelist.push_back("ttH");             v_color.push_back(kRed);
    thesamplelist.push_back("STtWll");          v_color.push_back(kBlack);
    // thesamplelist.push_back("WZjets");          v_color.push_back(11); //grey

    //FAKES
    thesamplelist.push_back("Fakes");           v_color.push_back(kAzure-2); //Data-driven (DD)

    //-- THESE SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    //WARNING : OBSOLETE -- don't use MC fakes (or update code)
    // thesamplelist.push_back("DYjets");          v_color.push_back(kAzure-2); //MC
    // thesamplelist.push_back("TT");              v_color.push_back(kRed-1); //MC
    // thesamplelist.push_back("WW");              v_color.push_back(kYellow); //MC

//-------------------


//---------------------------------------------------------------------------
// ########  ########  ########       ##     ##    ###    ########   ######
// ##     ## ##     ##    ##          ##     ##   ## ##   ##     ## ##    ##
// ##     ## ##     ##    ##          ##     ##  ##   ##  ##     ## ##
// ########  ##     ##    ##          ##     ## ##     ## ########   ######
// ##     ## ##     ##    ##           ##   ##  ######### ##   ##         ##
// ##     ## ##     ##    ##            ## ##   ##     ## ##    ##  ##    ##
// ########  ########     ##             ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
    std::vector<TString > thevarlist; //Variables used in BDT
    std::vector<TString > thevarlist_ttZ; //Variables used in BDTttZ

//------------------------ for tZq
    thevarlist.push_back("btagDiscri");
    thevarlist.push_back("dRAddLepQ");
    thevarlist.push_back("dRAddLepClosestJet");
    thevarlist.push_back("dPhiAddLepB");
    thevarlist.push_back("ZEta");
    thevarlist.push_back("Zpt");
    thevarlist.push_back("mtop");
    thevarlist.push_back("AddLepAsym");
    thevarlist.push_back("etaQ");
    thevarlist.push_back("AddLepETA");
    thevarlist.push_back("LeadJetEta");
    thevarlist.push_back("dPhiZAddLep");
    thevarlist.push_back("dRZAddLep");
    thevarlist.push_back("dRjj");
    thevarlist.push_back("ptQ");
    thevarlist.push_back("tZq_pT");

    if(include_MEM_variables){
    thevarlist.push_back("MEMvar_0"); //Likelihood ratio of MEM weigt (S/S+B ?), with x-sec scaling factor //FIXME --use it instead of (6)
    thevarlist.push_back("MEMvar_1"); //Kinematic Fit Score
    thevarlist.push_back("MEMvar_2"); //Kinematic Fit Score

    //--- NEW VARIABLES
    // thevarlist.push_back("MEMvar_4"); //Kinematic Fit Score -- ~ 100% correlated to 5 ?
    // thevarlist.push_back("MEMvar_5"); //Kinematic Fit Score
    thevarlist.push_back("MEMvar_6"); //try 0 + 7 instead
    // thevarlist.push_back("MEMvar_7");
    }

    // thevarlist.push_back("-log((3.89464e-13*mc_mem_ttz_weight) / (3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"); //MEMvar_0
    // thevarlist.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //MEMvar_1
    // thevarlist.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //MEMvar_2
    // thevarlist.push_back("log(mc_mem_wzjj_weight)"); //MEMvar_4
    // thevarlist.push_back("log(mc_mem_wzjj_weight_kinmaxint)"); //MEMvar_5
    // thevarlist.push_back("-log((0.017*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight) / (0.017*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))");//MEMvar_6
    // thevarlist.push_back("-log((0.017*mc_mem_wzjj_weight) / (0.017*mc_mem_wzjj_weight + 0.17993*mc_mem_tllj_weight) )"); //MEMvar_7



//------------------------ for ttZ
    thevarlist_ttZ.push_back("btagDiscri");
    thevarlist_ttZ.push_back("dRAddLepQ");
    thevarlist_ttZ.push_back("dRAddLepB");
    thevarlist_ttZ.push_back("dRAddLepClosestJet");
    thevarlist_ttZ.push_back("Zpt");
    thevarlist_ttZ.push_back("ZEta");
    thevarlist_ttZ.push_back("AddLepAsym");
    thevarlist_ttZ.push_back("etaQ");
    thevarlist_ttZ.push_back("ptQ"); // strong correlation with LeadJetPt
    thevarlist_ttZ.push_back("AddLepETA");
    thevarlist_ttZ.push_back("dPhiZAddLep");
    thevarlist_ttZ.push_back("dRZAddLep"); // --> little discrim --> to be included
    thevarlist_ttZ.push_back("dRjj");
    thevarlist_ttZ.push_back("mtop");
    thevarlist_ttZ.push_back("TopPT"); // low discri power
    thevarlist_ttZ.push_back("m3l");
    thevarlist_ttZ.push_back("dRZTop");
    thevarlist_ttZ.push_back("tZq_pT");

    if(include_MEM_variables){
    thevarlist_ttZ.push_back("MEMvar_1"); //Kinematic Fit Score
    thevarlist_ttZ.push_back("MEMvar_2"); //Kinematic Fit Score
    thevarlist_ttZ.push_back("MEMvar_3"); //Likelihood ratio of MEM weigt (S/S+B ?)
    }

    // thevarlist_ttZ.push_back("-log(mc_mem_ttz_tllj_likelihood)"); //MEMvar_3
    // thevarlist_ttZ.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //MEMvar_1
    // thevarlist_ttZ.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //MEMvar_2

//-------------------




//---------------------------------------------------------------------------
//  #######  ######## ##     ## ######## ########       ##     ##    ###    ########   ######
// ##     ##    ##    ##     ## ##       ##     ##      ##     ##   ## ##   ##     ## ##    ##
// ##     ##    ##    ##     ## ##       ##     ##      ##     ##  ##   ##  ##     ## ##
// ##     ##    ##    ######### ######   ########       ##     ## ##     ## ########   ######
// ##     ##    ##    ##     ## ##       ##   ##         ##   ##  ######### ##   ##         ##
// ##     ##    ##    ##     ## ##       ##    ##         ## ##   ##     ## ##    ##  ##    ##
//  #######     ##    ##     ## ######## ##     ##         ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
//Can add additionnal vars which are NOT used in TMVA NOR for cuts, only for CR plots
//NOTE : Branch can be linked to only *one* variable via SetBranchAddress ; if add. variable is present in other variables vectors, it is removed from this vector !


    vector<TString> v_add_var_names;
    // v_add_var_names.push_back("mTW");
    // if(!include_MEM_variables) v_add_var_names.push_back("METpt");



//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------

    bool use_systematics = false;
//----------------


//--- General names of systematics
    vector<TString> systematics_names_tmp;

    //--- Stored in separate Ntuples (tZq only)
    systematics_names_tmp.push_back("PSscale");
    systematics_names_tmp.push_back("Hadron");

    //--- Stored in separate Trees
    systematics_names_tmp.push_back("JER");
    systematics_names_tmp.push_back("JES");
    systematics_names_tmp.push_back("Fakes");

    //--- Stored as separate weights
    systematics_names_tmp.push_back("Q2");
    systematics_names_tmp.push_back("pdf");
    systematics_names_tmp.push_back("PU");
    systematics_names_tmp.push_back("MuEff");
    systematics_names_tmp.push_back("EleEff");
    systematics_names_tmp.push_back("LFcont");
    systematics_names_tmp.push_back("HFstats1");
    systematics_names_tmp.push_back("HFstats2");
    systematics_names_tmp.push_back("CFerr1");
    systematics_names_tmp.push_back("CFerr2");
    systematics_names_tmp.push_back("HFcont");
    systematics_names_tmp.push_back("LFstats1");
    systematics_names_tmp.push_back("LFstats2");
//----------------

//--- Actual vector of systematic names we will use
    vector<TString> thesystlist;
    thesystlist.push_back(""); //Nominal -- KEEP this line

    for(int isyst=0; isyst<systematics_names_tmp.size(); isyst++)
    {
        if(!use_systematics) {break;}
        thesystlist.push_back( systematics_names_tmp[isyst] + "__plus" );
        thesystlist.push_back( systematics_names_tmp[isyst] + "__minus" );
    }



//---------------------------------------------------------------------------
// ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##        ######     ###    ##       ##        ######
// ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ##       ##    ##   ## ##   ##       ##       ##    ##
// ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ##       ##        ##   ##  ##       ##       ##
// ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##       ##       ##     ## ##       ##        ######
// ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##       ######### ##       ##             ##
// ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ###       ##    ## ##     ## ##       ##       ##    ##
// ##        #######  ##    ##  ######     ##    ####  #######  ##    ##        ######  ##     ## ######## ########  ######
//---------------------------------------------------------------------------


    if(!do_optimization_cuts && !RemoveBDTvars_CreateTemplates_ExtractSignif)
    {

//*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !

//-----------------    TRAINING
        bool train_BDT = true; //Train BDT (if region is tZq or ttZ)

//-----------------    TEMPLATES CREATION
        bool create_templates = true; //Create templates in selected region (NB : to cut on BDT value, use dedicated boolean in 'OPTIONS' section)

//-----------------    CONTROL HISTOGRAMS
        bool create_control_histograms = false; //Create histograms of input variables, needed to make plots of these variables -- Takes time !

//-----------------    PLOTS
        bool draw_input_vars = false; //Plot input variables
        bool draw_templates = false; //Plot templates (mTW/BDT/BDTttZ)

        bool postfit = false; //Decide if want prefit OR combine postfit plots (NB : use different files)

//-----------------    OTHER
        bool convert_templates_for_theta = false; //Use this if you already produced template files with Combine conventions, and want to convert them to Theta

//-----------------



//---> AUTOMATIZED FUNCTION CALLS FROM BOOLEANS

        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################
        std::vector<TString > thevarlist_tmp;
        if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
        else       thevarlist_tmp = thevarlist;
        theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity);

        TString template_name = "BDT"; //BDT in Signal Region
        if(isttZ)  template_name = "BDTttZ"; //BDT in ttZ Control region
        if(isWZ)   template_name = "mTW"; //Template for WZ Control region

        float cut_BDT_value = -99;
        if(isWZ || isttZ)  cut_on_BDT = false; //No BDT in WZ CR ; & don't cut on BDTttZ for now

        //#############################################
        // TRAINING
        //#############################################
        for(int i=0; i<thechannellist.size(); i++)
        {
            if(train_BDT && !isWZ)
            {
                MVAtool->Train_Test_Evaluate(thechannellist[i], template_name, use_ttZaMCatNLO_training, true);
            }
        }


        //#############################################
        //  TEMPLATES CREATION
        //#############################################

        if(create_templates)
        {
            if(cut_on_BDT) {cut_BDT_value = MVAtool->Determine_Control_Cut();}
            MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels, cut_on_BDT, cut_BDT_value);
        }

        //#############################################
        //  CONTROL TREES & HISTOGRAMS
        //#############################################
        if(create_control_histograms)
        {
            if(cut_on_BDT) {cut_BDT_value = MVAtool->Determine_Control_Cut();}
            MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDT, cut_BDT_value, false);
            MVAtool->Create_Control_Histograms(fakes_from_data, false, fakes_summed_channels, cut_on_BDT); //NOTE : very long ! You should only activate necessary syst./vars !
        }

        //#############################################
        //  DRAW PLOTS
        //#############################################
        for(int i=0; i<thechannellist.size(); i++) //SINGLE CHANNELS
        {
            if(draw_input_vars) MVAtool->Draw_Control_Plots(thechannellist[i], fakes_from_data, false, postfit, cut_on_BDT); //Draw plots for the BDT CR
            if(draw_templates)
            {
                if(!postfit) MVAtool->Plot_Prefit_Templates(thechannellist[i], template_name, false); //Plot the prefit templates
                else MVAtool->Plot_Postfit_Templates(thechannellist[i], template_name, false); //Postfit templates from Combine file
            }
            // MVAtool->Compare_Negative_Weights_Effect_On_Distributions(thechannellist[i], false);
        }

        // --- ALL CHANNELS
        if(draw_input_vars) MVAtool->Draw_Control_Plots("", fakes_from_data, true, postfit, cut_on_BDT);
        if(draw_templates)
        {
            if(!postfit) MVAtool->Plot_Prefit_Templates("all", template_name, true); //Plot the prefit templates
            else MVAtool->Plot_Postfit_Templates("all", template_name, true); //Postfit templates from Combine file
        }

        //#############################################
        //  Convert Templates names for Theta
        //#############################################
        if(convert_templates_for_theta) {MVAtool->Convert_Templates_Theta();}


        // MVAtool->Rescale_Fake_Histograms("FileToRescale.root"); //To rescale manually the fakes in a template file -- Make sure it wasn't rescaled yet !!

        //-----------------
        MVAtool->~theMVAtool(); //Delete object
    }




















// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######

//---------------------------------------------------------------------------
//  #######  ########  ######## #### ##     ## #### ########    ###    ######## ####  #######  ##    ##
// ##     ## ##     ##    ##     ##  ###   ###  ##       ##    ## ##      ##     ##  ##     ## ###   ##
// ##     ## ##     ##    ##     ##  #### ####  ##      ##    ##   ##     ##     ##  ##     ## ####  ##
// ##     ## ########     ##     ##  ## ### ##  ##     ##    ##     ##    ##     ##  ##     ## ## ## ##
// ##     ## ##           ##     ##  ##     ##  ##    ##     #########    ##     ##  ##     ## ##  ####
// ##     ## ##           ##     ##  ##     ##  ##   ##      ##     ##    ##     ##  ##     ## ##   ###
//  #######  ##           ##    #### ##     ## #### ######## ##     ##    ##    ####  #######  ##    ##
//---------------------------------------------------------------------------
//Loop on variable cuts to find "best-working" templates/region

// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######




//---------------------------------------------------------------------------
 //  ####  #    # #####  ####
 // #    # #    #   #   #
 // #      #    #   #    ####
 // #      #    #   #        #
 // #    # #    #   #   #    #
 //  ####   ####    #    ####
//---------------------------------------------------------------------------


    if(do_optimization_cuts)
    {
        if(isWZ) {cout<<endl<<FBLU("No MET/mTW cuts in the WZ control region ! Abort")<<endl<<endl; return 0;}

        //#############################################
        //  SET THE CUT DEFINITIONS ON WHICH YOU WANT TO LOOP
        //#############################################

        //For scan -- NOTE : different values for each region

        TString cut1_name = "METpt";
        vector<TString> v_cut1_values;


        TString cut2_name = "mTW";
        vector<TString> v_cut2_values;

        //2D scan of MET & mTW
        for(int icut=0; icut<=100; icut+=10)
        {
            TString cut_def = ">" + icut;

            v_cut1_values.push_back(cut_def);
            v_cut2_values.push_back(cut_def);
        }



        //#############################################
        //  LOOP ON THE 2 CUT VECTORS YOU WANT TO SCAN
        //#############################################

        //SCAN LOOP
        for(int icut1 = 0; icut1 < v_cut1_values.size(); icut1++)
        {
    	    for(int icut2 = 0; icut2 < v_cut2_values.size(); icut2++)
    	    {
                //--- Add the default cuts which don't depend on the loop : regions definitions, etc. (defined at top of code)
                vector<TString> set_v_cut_name_optim, set_v_cut_def_optim; vector<bool> set_v_cut_IsUsedForOptim_BDTvar;
                for(int icut=0; icut<set_v_cut_name.size(); icut++)
                {
                    set_v_cut_name_optim.push_back(set_v_cut_name[icut]);
                    set_v_cut_def_optim.push_back(set_v_cut_def[icut]);
                    set_v_cut_IsUsedForOptim_BDTvar.push_back(set_v_cut_IsUsedForBDT[icut]);
                }

                //--- Add the temporary cuts defined by the loop
                //Cut 1
                set_v_cut_name_optim.push_back(cut1_name);
                set_v_cut_def_optim.push_back(v_cut1_values[icut1]);
                set_v_cut_IsUsedForOptim_BDTvar.push_back(false);
                //Cut 2
                set_v_cut_name_optim.push_back(cut2_name);
                set_v_cut_def_optim.push_back(v_cut2_values[icut2]);
                set_v_cut_IsUsedForOptim_BDTvar.push_back(false);



        //*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !

            //-----------------    TRAINING
                bool train_BDT = true; //Train BDT (if region is tZq or ttZ)

            //-----------------    TEMPLATES CREATION
                bool create_templates = true; //Create templates in selected region (NB : to cut on BDT value, use dedicated boolean in 'OPTIONS' section)

            //-----------------



        //---> AUTOMATIZED FUNCTION CALLS FROM BOOLEANS
                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################
                std::vector<TString > thevarlist_tmp;
                if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
                else       thevarlist_tmp = thevarlist;
                theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name_optim, set_v_cut_def_optim, set_v_cut_IsUsedForOptim_BDTvar, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);


                TString template_name = "BDT"; //BDT in Signal Region
                if(isttZ)  template_name = "BDTttZ"; //BDT in ttZ Control region

                TString dir_name;

                //#############################################
                // TRAINING
                //#############################################

                for(int ichan=0; ichan<thechannellist.size(); ichan++)
                {
                    dir_name = "outputs/optim_cuts";
                    mkdir(dir_name.Data() , 0777);
                    dir_name = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]));
                    mkdir(dir_name.Data() , 0777);
                    dir_name = "weights/optim_cuts";
                    mkdir(dir_name.Data() , 0777);

                    dir_name = "./weights/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]));
                    mkdir(dir_name.Data() , 0777);

                    if(train_BDT && !isWZ)
                    {
                        MVAtool->Train_Test_Evaluate(thechannellist[ichan], template_name, use_ttZaMCatNLO_training, false);
                    }


                    MoveFile( ("./outputs/"+template_name+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".root"), ("./outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2])) ) ); //Move file

                    MoveFile( ("./weights/"+template_name+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C"), ("./weights/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/"+template_name+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C") ); //Move weight file to specific dir.
                    MoveFile( ("./weights/"+template_name+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/"+template_name+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml") ); //Move weight file to specific dir.
                }

                //#############################################
                //  TEMPLATES CREATION
                //#############################################

                //Reader needs weight files from ALL FOUR channels -> make sure they are all available in dir. weights/
                for(int jchan=0; jchan<thechannellist.size(); jchan++)
                {
                    CopyFile( ("./weights/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/"+template_name+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C"), ("./weights/"+template_name+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                    CopyFile( ("./weights/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/"+template_name+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/"+template_name+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                }

                if(create_templates)
                {
                    MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels, false, -99);
                }

                MoveFile( ("./outputs/Reader_"+template_name+MVAtool->filename_suffix+".root"), ("outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2])) ) ); //Move file


                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object

            } //end second scanned variable loop
        } //end first scanned variable loop







        //-------------------------
        //CREATE COMBINE INPUT FILES, RUN COMBINE ON ALL FILES, STORE SIGNIFICANCE VALUES

        bool use_syst = false;
        bool expected = true;
        TString signal = "tZq";


        TString file_significances = "CUT_OPTIM_SIGNIFICANCES.txt";
        ofstream file_out(file_significances.Data() );
        file_out<<"-- Significances computed from Combine, obtained varying cuts on "<<cut1_name<<" && "<<cut2_name<<" --"<<endl;
        file_out<<cut1_name<<" : "<<v_cut1_values[0]<<" --> "<<v_cut1_values[v_cut1_values.size()-1]<<endl;
        file_out<<cut2_name<<" : "<<v_cut2_values[0]<<" --> "<<v_cut2_values[v_cut2_values.size()-1]<<endl<<endl;
        file_out<<"use_syst = "<<use_syst<<endl;
        file_out<<"expected = "<<expected<<endl;
        file_out<<"signal = "<<signal<<endl<<endl<<endl;

        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################
        theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity); //only used to call a class-function, initialization not important

        for(int icut1 = 0; icut1 < v_cut1_values.size(); icut1++)
        {
    	    for(int icut2 = 0; icut2 < v_cut2_values.size(); icut2++)
    	    {
                TString filename_suffix_tmp;
                TString tmp = "";
                vector<TString> v_cut_name_sufix; vector<TString> v_cut_def_sufix;
                v_cut_name_sufix.push_back(cut1_name); v_cut_def_sufix.push_back(v_cut1_values[icut1]);
                v_cut_name_sufix.push_back(cut2_name); v_cut_def_sufix.push_back(v_cut2_values[icut2]);

                for(int ivar=0; ivar<v_cut_name_sufix.size(); ivar++)
                {
                    if( (v_cut_name_sufix[ivar]=="METpt" || v_cut_name_sufix[ivar]=="mTW") && v_cut_def_sufix[ivar] == ">0") {continue;} //Useless cuts

                    if(v_cut_def_sufix[ivar] != "")
                    {
                        if(v_cut_def_sufix[ivar].Contains("||") ) {continue;} //Don't add the 'or' conditions in filename
                        else if(!v_cut_def_sufix[ivar].Contains("&&")) //Single condition
                        {
                            tmp = "_" + v_cut_name_sufix[ivar] + Convert_Sign_To_Word(v_cut_def_sufix[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def_sufix[ivar]));
                        }
                        else //Double '&&' condition
                        {
                            TString cut1 = Break_Cuts_In_Two(v_cut_def_sufix[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def_sufix[ivar]).second;
                            tmp = "_" + v_cut_name_sufix[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                            tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
                        }

                        filename_suffix_tmp+= tmp;
                    }
                }

                TString file_BDT_path = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/Reader_BDT_NJetsMin1Max4_NBJetsEq1"+filename_suffix_tmp+".root";
                TString file_BDTttZ_path = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/Reader_BDTttZ_NJetsMin1_NBJetsMin1"+filename_suffix_tmp+".root";

                if(!Check_File_Existence(file_BDT_path) ) {cout<<file_BDT_path<<" not found ! "<<BOLD(FRED("CONTINUE!"))<<endl;}
                if(!Check_File_Existence(file_BDTttZ_path) ) {cout<<file_BDTttZ_path<<" not found ! "<<BOLD(FRED("CONTINUE!"))<<endl;}

                TString combine_file_path = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/Combine_Input.root";

                system( ("hadd -f "+combine_file_path+" ./outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root "+file_BDT_path + " " + file_BDTttZ_path).Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( combine_file_path, signal, expected, use_syst);

                file_out<<cut1_name+v_cut1_values[icut1]<<"&&"<<cut2_name+v_cut2_values[icut2]<<" ---> "<<signif<<endl;
            }
        }

        delete MVAtool;

        Order_Cuts_By_Decreasing_Signif_Loss(file_significances.Data() );

    } //End optimization Scan




//---------------------------------------------------------------------------
 // # #    # #####  #    # #####    #    #   ##   #####  #   ##   #####  #      ######  ####
 // # ##   # #    # #    #   #      #    #  #  #  #    # #  #  #  #    # #      #      #
 // # # #  # #    # #    #   #      #    # #    # #    # # #    # #####  #      #####   ####
 // # #  # # #####  #    #   #      #    # ###### #####  # ###### #    # #      #           #
 // # #   ## #      #    #   #       #  #  #    # #   #  # #    # #    # #      #      #    #
 // # #    # #       ####    #        ##   #    # #    # # #    # #####  ###### ######  ####
//---------------------------------------------------------------------------

    if(RemoveBDTvars_CreateTemplates_ExtractSignif)
    {
        vector<TString > thevarlist_tmp;
        if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
        else       thevarlist_tmp = thevarlist;

        //--- Technical issue : some variables in the cuts vectors are also used in BDT ; need to take them into account
        int n_cutVars_in_BDT = 0;
        for(int i=0; i<set_v_cut_name.size(); i++)
        {
            if(set_v_cut_IsUsedForBDT[i])
            {
                thevarlist_tmp.push_back(set_v_cut_name[i]);
                n_cutVars_in_BDT++;
            }
        }


        TString bdt_type = "BDT";
        if(isttZ) bdt_type = "BDTttZ";

        TString dir_name;

        for(int ivar=0; ivar<thevarlist_tmp.size(); ivar++)
        {
            TString removed_var_name = thevarlist_tmp[ivar];

            vector<TString > varlist_optim;
            for(int j=0; j<thevarlist_tmp.size(); j++)
            {
                if(j==ivar) {continue;} //Remove var by not pushing it into new vector
                varlist_optim.push_back(thevarlist_tmp[j]);
            }

            //--- Also take care of the removal of the BDT variables defined only within the cut vectors!
            vector<TString > v_cut_name_optim = set_v_cut_name;
            vector<bool > set_v_cut_IsUsedForBDT_optim = set_v_cut_IsUsedForBDT;
            for(int j=0; j<n_cutVars_in_BDT; j++)
            {
                if(thevarlist_tmp[ivar+thevarlist_tmp.size()] == v_cut_name_optim[j])
                {
                    set_v_cut_IsUsedForBDT_optim[j] = false;
                    break;
                }

            }

            for(int ichan=0; ichan<thechannellist.size(); ichan++)
            {
                dir_name = "outputs/optim_BDTvar";
                mkdir(dir_name.Data() , 0777);
                dir_name = "outputs/optim_BDTvar/"+bdt_type;
                mkdir(dir_name.Data() , 0777);
                dir_name = "outputs/optim_BDTvar/"+bdt_type+"/without_"+removed_var_name;
                mkdir(dir_name.Data() , 0777);
                dir_name = "weights/optim_BDTvar";
                mkdir(dir_name.Data() , 0777);
                mkdir(("./weights/optim_BDTvar/weights_without_"+removed_var_name).Data(), 0777);

                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################

                theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, v_cut_name_optim, set_v_cut_def, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                //#############################################
                // TRAINING
                //#############################################

                MVAtool->Train_Test_Evaluate(thechannellist[ichan], bdt_type, use_ttZaMCatNLO_training, false);

                MoveFile( ("./outputs/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".root"), ("./outputs/optim_BDTvar/"+bdt_type+"/without_"+removed_var_name) ); //Move file

                MoveFile( ("./weights/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C"), ("./weights/optim_BDTvar/weights_without_"+removed_var_name+"/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C") ); //Move weight file to specific dir.
                MoveFile( ("./weights/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/optim_BDTvar/weights_without_"+removed_var_name+"/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml") ); //Move weight file to specific dir.

                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }

            for(int ichan=0; ichan<thechannellist.size(); ichan++)
            {
                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################

                theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, v_cut_name_optim, set_v_cut_def, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                //#############################################
                //  TEMPLATES CREATION
                //#############################################

                //Reader needs weight files from ALL FOUR channels -> make sure they are all available in dir. weights/
                for(int jchan=0; jchan<thechannellist.size(); jchan++)
                {
                    CopyFile( ("./weights/optim_BDTvar/weights_without_"+removed_var_name+"/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C"), ("./weights/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                    CopyFile( ("./weights/optim_BDTvar/weights_without_"+removed_var_name+"/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                }

                MVAtool->Read(bdt_type, fakes_from_data, real_data_templates, fakes_summed_channels, false, -99);

                MoveFile( ("./outputs/Reader_"+bdt_type+"*"+MVAtool->filename_suffix+".root"), ("./outputs/optim_BDTvar/"+bdt_type+"/without_"+removed_var_name) ); //Move file

                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }// end chan loop

        } //end var removal loop


        //-------------------------
        //CREATE COMBINE INPUT FILES, RUN COMBINE ON ALL FILES, STORE SIGNIFICANCE VALUES
        //-------------------------

        bool use_syst = false;
        bool expected = true;
        TString signal = "tZq";


        TString file_significances = "BDT_OPTIM_SIGNIFICANCES.txt";
        if(isttZ) file_significances = "BDTttZ_OPTIM_SIGNIFICANCES.txt";
        ofstream file_out(file_significances.Data() );
        file_out<<" -- Significances computed from Combine, obtained by removing the BDT vars one by one --"<<endl;
        file_out<<"use_syst = "<<use_syst<<endl;
        file_out<<"expected = "<<expected<<endl;
        file_out<<"signal = "<<signal<<endl<<endl<<endl;

        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################

        theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity);

        for(int ivar=0; ivar<thevarlist_tmp.size(); ivar++)
        {
            if(!isWZ && !isttZ)
            {
                system( ("hadd -f ./outputs/optim_BDTvar/BDT/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root  ./outputs/optim_BDTvar/BDT/without_"+thevarlist_tmp[ivar]+"/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root ./outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root ./outputs/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root").Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( ("./outputs/optim_BDTvar/BDT/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root"), signal, expected, use_syst);

                file_out<<thevarlist_tmp[ivar]<<" ---> "<<signif<<endl;
            }
            else if(!isWZ && isttZ)
            {
                system( ("hadd -f ./outputs/optim_BDTvar/BDTttZ/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root  ./outputs/optim_BDTvar/BDTttZ/without_"+thevarlist_tmp[ivar]+"/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root ./outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root ./outputs/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root").Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( ("./outputs/optim_BDTvar/BDTttZ/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root"), signal, expected, use_syst);

                file_out<<thevarlist_tmp[ivar]<<" ---> "<<signif<<endl;
            }
        }

        delete MVAtool;

        Order_BDTvars_By_Decreasing_Signif_Loss(file_significances.Data() );

    } //end optimization loop



































/*

//---------------------------------------------------------------------------
 // # #    # #####  #    # #####    #    #   ##   #####  #   ##   #####  #      ######  ####
 // # ##   # #    # #    #   #      #    #  #  #  #    # #  #  #  #    # #      #      #
 // # # #  # #    # #    #   #      #    # #    # #    # # #    # #####  #      #####   ####
 // # #  # # #####  #    #   #      #    # ###### #####  # ###### #    # #      #           #
 // # #   ## #      #    #   #       #  #  #    # #   #  # #    # #    # #      #      #    #
 // # #    # #       ####    #        ##   #    # #    # # #    # #####  ###### ######  ####
//---------------------------------------------------------------------------
//First, need to train on complete variable set to create the ranking info file which will be used !

    if(do_optimization_input_variables)
    {
        if(isWZ) {cout<<"No BDT in WZ Control Region !"<<endl; return 0;}

        if(!Check_File_Existence("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root") )
        {
            cout<<endl<<BOLD(FGRN("First need to create mTW template file (to compute data Fakes SF) ! "))<<endl<<endl;

            std::vector<TString > thevarlist_tmp;
            theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
            MVAtool->Set_Luminosity(set_luminosity);

            MVAtool->Read("mTW", fakes_from_data, real_data_templates, fakes_summed_channels, false, -99);

            MoveFile("./outputs/Reader_mTW"+MVAtool->filename_suffix+"_unScaled.root", "./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root"); //Need to add expected suffix, not current region one !

            MVAtool->Rescale_Fake_Histograms("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root");
        }

        int nmax_vars_to_remove = 4;
        bool remove_vars_two_by_two = true;

        TString bdt_type = "BDT";
        if(!isWZ && isttZ) bdt_type = "BDTttZ";

        mkdir("outputs",0777);
        mkdir("outputs/optim_BDTvar",0777);


        //#############################################
        //
        //#############################################

        if(remove_vars_two_by_two && nmax_vars_to_remove%2 != 0) {nmax_vars_to_remove--;}
        int count = 0;
        for(int ivar=0; ivar<nmax_vars_to_remove; ivar++)
        {
            if(remove_vars_two_by_two) {ivar++;}

            count++; //Count effective number of set of BDT variables


            for(int ichan=0; ichan<thechannellist.size(); ichan++)
            {
                vector<TString> v_BDTvar_name; vector<double> v_BDTvar_importance;

                Get_Ranking_Vectors(bdt_type, "uuu", v_BDTvar_name, v_BDTvar_importance);
                // Get_Ranking_Vectors(bdt_type, thechannellist[ichan], v_BDTvar_name, v_BDTvar_importance);

                vector<TString> v_BDTvariables_tmp;
                for(int ivar=0; ivar<v_BDTvar_name.size(); ivar++)
                {
                    v_BDTvariables_tmp.push_back(v_BDTvar_name[ivar]);
                }

                v_BDTvariables_tmp.erase(v_BDTvariables_tmp.begin() + v_BDTvariables_tmp.size() -ivar -1);

                if(ichan==0)
                {
                    mkdir( ("outputs/optim_BDTvar/BDTfiles_"+Convert_Number_To_TString(count)).Data(),0777);
                    TString f_varlist_info_path = "./outputs/optim_BDTvar/BDTfiles_"+Convert_Number_To_TString(count)+"/BDT_varlist_info.txt";
                    ofstream f_varlist_info(f_varlist_info_path.Data());
                    f_varlist_info<<"++ List of variables used for training this BDT :"<<endl<<endl;
                    for(int k=0; k<v_BDTvariables_tmp.size(); k++) {f_varlist_info<<v_BDTvariables_tmp[k]<<endl;}
                    f_varlist_info<<endl<<endl<<"-- Variables which were removed compared to full list :"<<endl;
                    for(int k=v_BDTvariables_tmp.size(); k<v_BDTvar_name.size(); k++) {f_varlist_info<<v_BDTvar_name[k]<<endl;}
                }

                // cout<<endl<<"-----------------"<<endl;
                // cout<<"ivar = "<<ivar<<endl<<endl;
                //
                // for(int j=0; j<v_BDTvariables_tmp.size(); j++)
                // {
                //     cout<<v_BDTvariables_tmp[j]<<endl;
                // }
                // cout<<"-----------------"<<endl;


                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################

                theMVAtool* MVAtool = new theMVAtool(v_BDTvariables_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                float cut_BDT_value = -99;  if(isWZ || isttZ)  cut_on_BDT = false; //No BDT in WZ CR ; & don't cut on BDTttZ for now

                //#############################################
                // TRAINING
                //#############################################

                MVAtool->Train_Test_Evaluate(thechannellist[ichan], bdt_type, use_ttZaMCatNLO_training, false);

                mkdir("./weights/optim_BDTvar", 0777);
                mkdir(("./weights/optim_BDTvar/weights_"+Convert_Number_To_TString(count)).Data(), 0777);

                MoveFile( ("./outputs/"+bdt_type+"_"+thechannellist[ichan]+"*.root"), ("./outputs/optim_BDTvar/BDTfiles_"+Convert_Number_To_TString(count)) ); //Move file
                MoveFile( ("./weights/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C"), ("./weights/optim_BDTvar/weights_"+Convert_Number_To_TString(count)+"/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C") ); //Move weight file to specific dir.
                MoveFile( ("./weights/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/optim_BDTvar/weights_"+Convert_Number_To_TString(count)+"/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml") ); //Move weight file to specific dir.

                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }

            for(int ichan=0; ichan<thechannellist.size(); ichan++)
            {
                vector<TString> v_BDTvar_name; vector<double> v_BDTvar_importance;

                Get_Ranking_Vectors(bdt_type, "uuu", v_BDTvar_name, v_BDTvar_importance);
                // Get_Ranking_Vectors(bdt_type, thechannellist[ichan], v_BDTvar_name, v_BDTvar_importance);

                vector<TString> v_BDTvariables_tmp;
                for(int ivar=0; ivar<v_BDTvar_name.size(); ivar++)
                {
                    v_BDTvariables_tmp.push_back(v_BDTvar_name[ivar]);
                }

                v_BDTvariables_tmp.erase(v_BDTvariables_tmp.begin() + v_BDTvariables_tmp.size() -ivar -1);


                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################

                theMVAtool* MVAtool = new theMVAtool(v_BDTvariables_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                //#############################################
                //  TEMPLATES CREATION
                //#############################################

                //Reader needs weight files from ALL FOUR channels -> make sure they are all available in dir. weights/
                for(int jchan=0; jchan<thechannellist.size(); jchan++)
                {
                    CopyFile( ("./weights/optim_BDTvar/weights_"+Convert_Number_To_TString(count)+"/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C"), ("./weights/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                    CopyFile( ("./weights/optim_BDTvar/weights_"+Convert_Number_To_TString(count)+"/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                }

                MVAtool->Read(bdt_type, fakes_from_data, real_data_templates, fakes_summed_channels, false, -99);

                MoveFile( ("./outputs/Reader_"+bdt_type+"*.root"), ("./outputs/optim_BDTvar/BDTfiles_"+Convert_Number_To_TString(count)) ); //Move file

                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }
        }
    } //End optimization Scan
*/
  return 0;
}
