#include "theMVAtool.h"

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
    bool do_optimization_scan = false;


    //Luminosity -- NB : A SCALE FACTOR IS COMPUTED W.R.T MORIOND2017 LUMI !!!
    double set_luminosity = 35.68; //Moriond 2017
    // double set_luminosity = 12.9; //Summer 2016
    // double set_luminosity = 35.68 - 8.605; //Moriond 2017 without Run H
    // double set_luminosity = 8.605; //Run H only

    //Training
    bool use_ttZaMCatNLO_training = true; //Choose ttZ training sample (false --> Madgraph sample)

    //Templates
    int nofbin_templates = 10; //Templates binning (to be optimized)
    bool real_data_templates = true; //If true, use real data sample to create templates (BDT, mTW, ...) / else, use pseudodata !
    bool cut_on_BDT = true; //Apply cut on BDT values --> Don't look signal region !

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
set_v_cut_name.push_back("passTrig");      set_v_cut_def.push_back("==1");            set_v_cut_IsUsedForBDT.push_back(false);
// set_v_cut_name.push_back("RunNr");      set_v_cut_def.push_back("<280919");            set_v_cut_IsUsedForBDT.push_back(false); //Without Run H
// set_v_cut_name.push_back("RunNr");      set_v_cut_def.push_back(">280919 || ==1");            set_v_cut_IsUsedForBDT.push_back(false); //Run H only
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

    //--- CIEMAT : Default Ntuples
    dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/input_ntuples";
    t_name = "Default";

    //--- IPHC : Ntuples Interfaced for MEM, divided in 2 sets (WZ region and ttZ+tZq regions)
    // if(isWZ) 	dir_ntuples="input_ntuples/ntuples_WZ"; //Without MEM (empty vars)
    // else 		dir_ntuples="input_ntuples/ntuples_MEM"; //With MEM
    // t_name = "Tree";





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
    thesamplelist.push_back("tZq");             v_color.push_back(kGreen+2);

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

    // thevarlist.push_back("MEMvar_0"); //Likelihood ratio of MEM weigt (S/S+B ?), with x-sec scaling factor
    // thevarlist.push_back("MEMvar_1"); //Kinematic Fit Score
    // thevarlist.push_back("MEMvar_2"); //Kinematic Fit Score

    // thevarlist.push_back("-log((3.89464e-13*mc_mem_ttz_weight) / (3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"); //MEMvar_0
    // thevarlist.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //MEMvar_1
    // thevarlist.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //MEMvar_2



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

    // thevarlist_ttZ.push_back("MEMvar_3"); //Likelihood ratio of MEM weigt (S/S+B ?)
    // thevarlist_ttZ.push_back("MEMvar_1"); //Kinematic Fit Score
    // thevarlist_ttZ.push_back("MEMvar_2"); //Kinematic Fit Score

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
    v_add_var_names.push_back("mTW");
    v_add_var_names.push_back("METpt");



//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------

    bool use_systematics = true;
//----------------


//--- General names of systematics
    vector<TString> systematics_names_tmp;
    //--- Affect the variable distributions
    systematics_names_tmp.push_back("JER");
    systematics_names_tmp.push_back("JES");
    systematics_names_tmp.push_back("Fakes");

    //--- Affect the event weight
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


    if(!do_optimization_scan)
    {

//*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !

//-----------------    TRAINING
        bool train_BDT = false; //Train BDT (if region is tZq or ttZ)

//-----------------    TEMPLATES CREATION
        bool create_templates = false; //Create templates in selected region (NB : to cut on BDT value, use dedicated boolean in 'OPTIONS' section)

//-----------------    CONTROL HISTOGRAMS
        bool create_control_histograms = true; //Create histograms of input variables, needed to make plots of these variables -- Takes time !

//-----------------    PLOTS
        bool draw_input_vars = true; //Plot input variables
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
                MVAtool->Train_Test_Evaluate(thechannellist[i], template_name, use_ttZaMCatNLO_training);
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




    if(do_optimization_scan)
    {
        //#############################################
        //  SET THE CUT DEFINITIONS ON WHICH YOU WANT TO LOOP
        //#############################################

        //For scan -- NOTE : different values for each region

        TString cut1_name = "METpt";
        vector<TString> v_cut1_values;
        v_cut1_values.push_back(">0");
        v_cut1_values.push_back(">10");

        TString cut2_name = "mTW";
        vector<TString> v_cut2_values;
        v_cut2_values.push_back(">0");
        v_cut2_values.push_back(">10");


        //#############################################
        //  LOOP ON THE 2 CUT VECTORS YOU WANT TO SCAN
        //#############################################

        //SCAN LOOP
        for(int icut1 = 0; icut1 < v_cut1_values.size(); icut1++)
        {
    	    for(int icut2 = 0; icut2 < v_cut2_values.size(); icut2++)
    	    {
                //--- Add the default cuts which don't depend on the loop : regions definitions, etc. (defined at top of code)
                vector<TString> set_v_cut_name_optim, set_v_cut_def_optim; vector<bool> set_v_cut_IsUsedForBDT_optim;
                for(int icut=0; icut<set_v_cut_name.size(); icut++)
                {
                    set_v_cut_name_optim.push_back(set_v_cut_name[icut]);
                    set_v_cut_def_optim.push_back(set_v_cut_def[icut]);
                    set_v_cut_IsUsedForBDT_optim.push_back(set_v_cut_IsUsedForBDT[icut]);
                }

                //--- Add the temporary cuts defined by the loop
                //Cut 1
                set_v_cut_name_optim.push_back(cut1_name);
                set_v_cut_def_optim.push_back(v_cut1_values[icut1]);
                set_v_cut_IsUsedForBDT_optim.push_back(false);
                //Cut 2
                set_v_cut_name_optim.push_back(cut2_name);
                set_v_cut_def_optim.push_back(v_cut2_values[icut2]);
                set_v_cut_IsUsedForBDT_optim.push_back(false);





        //*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !

            //-----------------    TRAINING
                bool train_BDT = false; //Train BDT (if region is tZq or ttZ)

            //-----------------    TEMPLATES CREATION
                bool create_templates = false; //Create templates in selected region (NB : to cut on BDT value, use dedicated boolean in 'OPTIONS' section)

            //-----------------



        //---> AUTOMATIZED FUNCTION CALLS FROM BOOLEANS
                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################
                std::vector<TString > thevarlist_tmp;
                if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
                else       thevarlist_tmp = thevarlist;
                theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name_optim, set_v_cut_def_optim, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);


                TString template_name = "BDT"; //BDT in Signal Region
                if(isttZ)  template_name = "BDTttZ"; //BDT in ttZ Control region
                if(isWZ)   template_name = "mTW"; //Template for WZ Control region

                float cut_BDT_value = -99;  if(isWZ || isttZ)  cut_on_BDT = false; //No BDT in WZ CR ; & don't cut on BDTttZ for now

                //#############################################
                // TRAINING
                //#############################################

                for(int i=0; i<thechannellist.size(); i++)
                {

                    if(train_BDT && !isWZ)
                    {
                        MVAtool->Train_Test_Evaluate(thechannellist[i], template_name, use_ttZaMCatNLO_training);
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



                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object

            } //end second scanned variable loop

        } //end first scanned variable loop

    } //End optimization Scan


  return 0;
}
