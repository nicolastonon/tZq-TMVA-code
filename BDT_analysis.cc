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


    //Used to re-scale every weights in the code by a lumi factor (in fb-1) //NOTE -- W.R.T. MORIOND 2017 LUMI !!
    // double set_luminosity = 12.9; //Summer 2016
    double set_luminosity = 35.68; //Moriond 2017


    //Use MC fakes or data-driven fakes)
    bool fakes_from_data = true;
    //Templates
    int nofbin_templates = 10; //NOTE : to be optimized --- Binning to be used for *template* production
    bool fakes_summed_channels = true; //Sum uuu+eeu & eee+uue --> Double the fake stat.!
    bool real_data_templates = true; //If true, use real data sample to create *templates* (BDT, mTW, ...) / else, use pseudodata !
    bool use_ttZMad_training = false; //TRAINING - Use either Madgraph or aMC@NLO sample for ttZ

    TString format = ".png"; //.png or .pdf only

//-------------------
    std::vector<TString> thesamplelist;
    std::vector<TString > thevarlist; //Variables used in BDT
    std::vector<TString > thevarlist_ttZ; //Variables used in BDT
    std::vector<TString > thesystlist;
    std::vector<TString > thechannellist;
    std::vector<TString > set_v_cut_name;
    std::vector<TString > set_v_cut_def;
    std::vector<bool > set_v_cut_IsUsedForBDT;
    std::vector<int> v_color; //sample <-> color
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

//-------------------
//------Specify here the cuts that you wish to apply to all 3 regions.
//To dis-activate a cut, just set it to "". Use "==" for equality. Don't use "||".
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4");

    //NOTE : can only use "< X" for iso (because if 3rd lepton has opposite flavour --> iso=-1 --> event wouldn't pass cut)
    //set_v_cut_name.push_back("AdditionalMuonIso");      set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    //set_v_cut_name.push_back("AdditionalEleIso");       set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
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

        // set_v_cut_name.push_back("mTW");        set_v_cut_def.push_back(">10");         set_v_cut_IsUsedForBDT.push_back(false); //Mara
        // set_v_cut_name.push_back("METpt");      set_v_cut_def.push_back(">10");         set_v_cut_IsUsedForBDT.push_back(false); //Mara
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

        // set_v_cut_name.push_back("mTW");       set_v_cut_def.push_back(">10");          set_v_cut_IsUsedForBDT.push_back(false); //Mara
        // set_v_cut_name.push_back("METpt");     set_v_cut_def.push_back(">10");          set_v_cut_IsUsedForBDT.push_back(false); //Mara
    }
//---------------------




//---------------------------------------------------------------------------
//  ######  ##     ##    ###    ##    ## ##    ## ######## ##        ######
// ##    ## ##     ##   ## ##   ###   ## ###   ## ##       ##       ##    ##
// ##       ##     ##  ##   ##  ####  ## ####  ## ##       ##       ##
// ##       ######### ##     ## ## ## ## ## ## ## ######   ##        ######
// ##       ##     ## ######### ##  #### ##  #### ##       ##             ##
// ##    ## ##     ## ##     ## ##   ### ##   ### ##       ##       ##    ##
//  ######  ##     ## ##     ## ##    ## ##    ## ######## ########  ######
//---------------------------------------------------------------------------

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


//-------------------
//Sample order is important in function Read (so it knows which are the fake samples it must sum) and in Draw_Control_Plots (see explanation in code)
    //DATA --- THE DATA SAMPLE MUST BE UNIQUE AND IN FIRST POSITION
    thesamplelist.push_back("Data");

    //Signal --- must be placed before backgrounds
    thesamplelist.push_back("tZq");             v_color.push_back(kGreen+2);
    //
    // //BKG
    // thesamplelist.push_back("WZjets");          v_color.push_back(11); //grey
    thesamplelist.push_back("WZl");          v_color.push_back(920); //grey
    thesamplelist.push_back("WZb");          v_color.push_back(921); //grey
    thesamplelist.push_back("WZc");          v_color.push_back(922); //grey
    thesamplelist.push_back("ZZ");              v_color.push_back(kYellow);
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed); //Keep 3 'red' samples together for plots
    thesamplelist.push_back("ttW");             v_color.push_back(kRed);
    thesamplelist.push_back("ttH");             v_color.push_back(kRed);
    thesamplelist.push_back("SingleTop");       v_color.push_back(kBlack);

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

    //thevarlist.push_back("mTW");
    //thevarlist.push_back("m3l");
    //thevarlist.push_back("ZCandMass");
    //thevarlist.push_back("leadingLeptonPT");
    //thevarlist.push_back("MAddLepB");
    //thevarlist.push_back("dPhiAddLepQ");
    //thevarlist.push_back("dPhiZMET");
    //thevarlist.push_back("dRZTop");
    //thevarlist.push_back("TopPT");
    //thevarlist.push_back("tZq_mass");
    //thevarlist.push_back("tZq_eta");
    // thevarlist.push_back("tq_mass");
    //thevarlist.push_back("tq_pT"); // strong correlation with ZpT
    //thevarlist.push_back("tq_eta");
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

    //thevarlist_ttZ.push_back("tZq_mass");
    // thevarlist_ttZ.push_back("LeadJetEta");
    // thevarlist_ttZ.push_back("dPhiAddLepB");
    // thevarlist_ttZ.push_back("dPhiAddLepQ");
    // thevarlist_ttZ.push_back("leadingLeptonPT");
    // thevarlist_ttZ.push_back("dPhiZMET");
    //thevarlist_ttZ.push_back("dRAddLepBFromTop");
    //thevarlist_ttZ.push_back("tZq_eta");
    //thevarlist_ttZ.push_back("tq_mass");
    //thevarlist_ttZ.push_back("tq_pT"); // strong correlation with ZpT
    // thevarlist_ttZ.push_back("tq_eta");
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
//NOTE : having the same variable in more than one variable vectors (thevarlist, set_v_cut_name & v_add_var_names) can cause problem later, because address of branch can be linked to only *one* variable via SetBranchAddress

//This is supposed to be taken care of in the constructor of the class (remove variable in other vectors if found multiple times), but be careful !
//NOTE : not tested yet !!

    vector<TString> v_add_var_names;
    v_add_var_names.push_back("mTW"); //FIXME -- bug CR plotting
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

//----------------
//-- CHOOSE IF YOU WANT COMBINE/THETA/NO SYSTEMATICS !
    //0 = no syst, 1 = theta, 2 = combine !
    int i_systematics_choice = 0;

    //--- General names of systematics
    vector<TString> systematics_names_tmp;
    //--- Affect the variable distributions
    systematics_names_tmp.push_back("JER");
    systematics_names_tmp.push_back("JES");
    // systematics_names_tmp.push_back("Fakes");

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

//--- AUTOMATIZED, don't change here
    //Actual vector of systematic names we will use (NOTE : naming depends on framework to be used for template fit)
    thesystlist.push_back(""); //Nominal -- KEEP this line

    if(i_systematics_choice == 1)
    {
        for(int isyst=0; isyst<systematics_names_tmp.size(); isyst++)
        {
            thesystlist.push_back( systematics_names_tmp[isyst] + "__plus" );
            thesystlist.push_back( systematics_names_tmp[isyst] + "__minus" );
        }
    }
    else if(i_systematics_choice == 2)
    {
        for(int isyst=0; isyst<systematics_names_tmp.size(); isyst++)
        {
            thesystlist.push_back( systematics_names_tmp[isyst] + "Up" );
            thesystlist.push_back( systematics_names_tmp[isyst] + "Down" );
        }
    }
    else if(i_systematics_choice != 0) {cout<<"Wrong systematics choice ! Abort"<<endl; return 1;}









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
        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################
        std::vector<TString > thevarlist_tmp;
        if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
        else       thevarlist_tmp = thevarlist;
        theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity);


        TString template_name = "BDT"; //BDT in Signal Region
        if(isttZ)  template_name = "BDTttZ"; //BDT in ttZ Control region
        if(isWZ)   template_name = "mTW"; //Template for WZ Control region


        //#############################################
        // TRAINING
        //#############################################
        for(int i=0; i<thechannellist.size(); i++)
        {

            if(!isWZ)
            {
                // MVAtool->Train_Test_Evaluate(thechannellist[i], template_name, use_ttZMad_training);
            }
        }

        //#############################################
        //  TEMPLATES CREATION
        //#############################################

        // MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels);


        //#############################################
        //  CONTROL TREES & HISTOGRAMS
        //#############################################
        float cut_BDT_value = -99;
        bool use_pseudodata_CR_plots = false;
        bool cut_on_BDT = false;
        if(isWZ)  cut_on_BDT = false; //No BDT in WZ CR
        if(cut_on_BDT) {cut_BDT_value = MVAtool->Determine_Control_Cut();}

        // MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDT, cut_BDT_value, use_pseudodata_CR_plots);
        //
        // MVAtool->Create_Control_Histograms(fakes_from_data, use_pseudodata_CR_plots, fakes_summed_channels); //NOTE : very long ! You should only activate necessary syst./vars !


        //#############################################
        //  DRAW PLOTS
        //#############################################
        bool draw_input_vars = true;
        bool draw_templates = false;

        bool postfit = false; //Decide if want prefit OR combine postfit plots of input vars (NB : different files)

//--------------- Automatized from booleans
        for(int i=0; i<thechannellist.size(); i++) //SINGLE CHANNELS
        {
            if(draw_input_vars) MVAtool->Draw_Control_Plots(thechannellist[i], fakes_from_data, false, postfit); //Draw plots for the BDT CR
            if(draw_templates)
            {
                if(!postfit) MVAtool->Plot_Templates(thechannellist[i], template_name, false); //Plot the prefit templates
                else MVAtool->Plot_Templates_from_Combine(thechannellist[i], template_name, false); //Postfit templates from Combine file
            }
            // MVAtool->Compare_Negative_Weights_Effect_On_Distributions(thechannellist[i], false);
        }

        // --- ALL CHANNELS

        if(draw_input_vars) MVAtool->Draw_Control_Plots("", fakes_from_data, true, postfit);
        if(draw_templates)
        {
            if(!postfit) MVAtool->Plot_Templates("all", template_name, true); //Plot the prefit templates
            else MVAtool->Plot_Templates_from_Combine("all", template_name, true); //Postfit templates from Combine file
        }
        // MVAtool->Compare_Negative_Weights_Effect_On_Distributions("", true);
//-----------------


        //#############################################
        //  Convert Templates names for Theta
        //#############################################

        // MVAtool->Convert_Templates_Theta();


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


                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################
                std::vector<TString > thevarlist_tmp;
                if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
                else       thevarlist_tmp = thevarlist;
                theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name_optim, set_v_cut_def_optim, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);


                TString template_name = "BDT"; //BDT in Signal Region
                if(isttZ)  template_name = "BDTttZ"; //BDT in ttZ Control region
                if(isWZ)   template_name = "mTW"; //Template for WZ Control region



                //#############################################
                // TRAINING
                //#############################################
                for(int i=0; i<thechannellist.size(); i++)
                {

                    if(!isWZ)
                    {
                        // MVAtool->Train_Test_Evaluate(thechannellist[i], template_name, use_ttZMad_training);
                    }
                }

                //#############################################
                //  READING --- TEMPLATES CREATION
                //#############################################

                MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels);




                MVAtool->~theMVAtool(); //Delete object
            } //end second scanned variable loop
        } //end first scanned variable loop

    } //End optimization Scan


  return 0;
}
