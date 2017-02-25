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

    double set_luminosity = 12.9; //Used to re-scale every weights in the code by a lumi factor (in fb-1)
    //Use MC fakes or data-driven fakes)
    bool fakes_from_data = true;
    //Templates
    int nofbin_templates = 10; //NOTE : to be optimized --- Binning to be used for *template* production
    bool fakes_summed_channels = true; //Sum uuu+eeu & eee+uue --> Double the fake stat.! //NOTE : only available for templates (not for plots)
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

//Specify here the cuts that you wish to apply (propagated to training, reading, ...). To dis-activate a cut, just set it to "". Use "==" for equality. Don't use "||".
//--- ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4");

//-------------------
    //set_v_cut_name.push_back("mTW");                    set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false); //code fix - don't need it anymore
    //set_v_cut_name.push_back("METpt");                  set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    //set_v_cut_name.push_back("AddLepPT");               set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(true);
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

    if(!isWZ && !isttZ) //Default selection is Signal Region : 1<NJets<4 && NBJets == 1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1 && <4");     set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back("==1");          set_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant
    }
    if(isWZ) //WZ CR Region : NJets > 0 && NBJets == 0
    {
        //set_v_cut_name.push_back("NJets");    set_v_cut_def.push_back(">1");      set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NJets");      set_v_cut_def.push_back(">0");      set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");     set_v_cut_def.push_back("== 0");   set_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant
    }
    if(isttZ) //ttZ CR Region : NJets>1 && NBJets>1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1");     set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back(">1");     set_v_cut_IsUsedForBDT.push_back(true);
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

    //BKG
    thesamplelist.push_back("WZjets");          v_color.push_back(11); //grey
    thesamplelist.push_back("ZZ");              v_color.push_back(kYellow);
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed); //Keep 3 'red' samples together for plots
    thesamplelist.push_back("ttW");             v_color.push_back(kRed);
    thesamplelist.push_back("ttH");             v_color.push_back(kRed);

    //FAKES
    thesamplelist.push_back("Fakes");           v_color.push_back(kAzure-2); //Data-driven (DD)

    //-- THESE SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    //WARNING : OBSOLETE -- don't use MC fakes (or update code)
    // thesamplelist.push_back("DYjets");          v_color.push_back(kAzure-2); //MC
    // thesamplelist.push_back("TT");              v_color.push_back(kRed-1); //MC
    // thesamplelist.push_back("WW");              v_color.push_back(kYellow); //MC
    // thesamplelist.push_back("SingleTop");       v_color.push_back(kBlack); //MC -- not taken into account in code

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
    vector<TString> v_add_var_names;
    v_add_var_names.push_back("mTW");
    // v_add_var_names.push_back("METpt"); //NB : need to include it in interfacing code



//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------

    //0 = no syst, 1 = theta, 2 = combine !
    int i_systematics_choice = 0;

    thesystlist.push_back(""); //Nominal -- KEEP this line


//THETA CONVENTION NAMING -- USE IT ONLY TO WORK DIRECTLY ON MARA'S NTUPLE
// --> Can't use systs in Combine !!
//-------------------
    if(i_systematics_choice==1)
    {
    //Affect the variable distributions
    thesystlist.push_back("JER__plus"); thesystlist.push_back("JER__minus");
    thesystlist.push_back("JES__plus"); thesystlist.push_back("JES__minus");
    thesystlist.push_back("Fakes__plus"); thesystlist.push_back("Fakes__minus");
    //Affect the event weight
    // thesystlist.push_back("Q2__plus"); thesystlist.push_back("Q2__minus");

    thesystlist.push_back("pdf__plus"); thesystlist.push_back("pdf__minus");
    thesystlist.push_back("PU__plus"); thesystlist.push_back("PU__minus");
    thesystlist.push_back("MuEff__plus"); thesystlist.push_back("MuEff__minus");
    thesystlist.push_back("EleEff__plus"); thesystlist.push_back("EleEff__minus");
    thesystlist.push_back("LFcont__plus"); thesystlist.push_back("LFcont__minus");
    thesystlist.push_back("HFstats1__plus"); thesystlist.push_back("HFstats1__minus");
    thesystlist.push_back("HFstats2__plus"); thesystlist.push_back("HFstats2__minus");
    thesystlist.push_back("CFerr1__plus"); thesystlist.push_back("CFerr1__minus");
    thesystlist.push_back("CFerr2__plus"); thesystlist.push_back("CFerr2__minus");
    thesystlist.push_back("HFcont__plus"); thesystlist.push_back("HFcont__minus");
    thesystlist.push_back("LFstats1__plus"); thesystlist.push_back("LFstats1__minus");
    thesystlist.push_back("LFstats2__plus"); thesystlist.push_back("LFstats2__minus");
    }


//COMBINE CONVENTION -- USE THIS TO WORK ON INTERFACED NTUPLES (FOR MEM, ...)
//-------------------
    else if(i_systematics_choice==2)
    {
    //Affect the variable distributions -- NOTE : not available yet
    thesystlist.push_back("JERUp"); thesystlist.push_back("JERDown");
    thesystlist.push_back("JESUp"); thesystlist.push_back("JESDown");
    thesystlist.push_back("FakesUp"); thesystlist.push_back("FakesDown");
    //Affect the event weight
    // thesystlist.push_back("Q2Up"); thesystlist.push_back("Q2Down"); //BUG for now

    thesystlist.push_back("pdfUp"); thesystlist.push_back("pdfDown");
    thesystlist.push_back("PUUp"); thesystlist.push_back("PUDown");
    thesystlist.push_back("MuEffUp"); thesystlist.push_back("MuEffDown");
    thesystlist.push_back("EleEffUp"); thesystlist.push_back("EleEffDown");
    thesystlist.push_back("LFcontUp"); thesystlist.push_back("LFcontDown");
    thesystlist.push_back("HFstats1Up"); thesystlist.push_back("HFstats1Down");
    thesystlist.push_back("HFstats2Up"); thesystlist.push_back("HFstats2Down");
    thesystlist.push_back("CFerr1Up"); thesystlist.push_back("CFerr1Down");
    thesystlist.push_back("CFerr2Up"); thesystlist.push_back("CFerr2Down");
    thesystlist.push_back("HFcontUp"); thesystlist.push_back("HFcontDown");
    thesystlist.push_back("LFstats1Up"); thesystlist.push_back("LFstats1Down");
    thesystlist.push_back("LFstats2Up"); thesystlist.push_back("LFstats2Down");
    }
    else if(i_systematics_choice != 0) {cout<<"Wrong systematics choice ! Abort"<<endl; return 2;}
//-------------------






//---------------------------------------------------------------------------
// ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##        ######     ###    ##       ##        ######
// ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ##       ##    ##   ## ##   ##       ##       ##    ##
// ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ##       ##        ##   ##  ##       ##       ##
// ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##       ##       ##     ## ##       ##        ######
// ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##       ######### ##       ##             ##
// ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ###       ##    ## ##     ## ##       ##       ##    ##
// ##        #######  ##    ##  ######     ##    ####  #######  ##    ##        ######  ##     ## ######## ########  ######
//---------------------------------------------------------------------------

    //(NOTE : Train_Test_Evaluate, Read, and Create_Control_Trees all need to be run on the full variable list)

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

        //#############################################
        // TRAINING
        //#############################################
        for(int i=0; i<thechannellist.size(); i++)
        {
            //'BDT' or 'BDTttZ' depending on the region (for theta disambiguation)
            TString bdt_type = "BDT";
            if(isttZ) bdt_type = "BDTttZ";

            if(!isWZ)
            {
                // MVAtool->Train_Test_Evaluate(thechannellist[i], bdt_type, use_ttZMad_training);
            }
        }

        //#############################################
        //  READING --- TEMPLATES CREATION
        //#############################################
        TString template_name = "BDT"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
        if(isWZ)   template_name = "mTW";
        if(isttZ)  template_name = "BDTttZ";

        // MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels);

        // if(!real_data_templates) {MVAtool->Generate_PseudoData_Templates(template_name);} //NOTE : GENERATE PSEUDODATA ON Combine_Input_ScaledFakes.root (fakes already re-scaled)

        //Fit templates to fill empty bins ('gaus' or 'landau')
        //TString function = "gaus"; MVAtool->Fit_Fake_Templates(function, "BDT"); MVAtool->Create_Fake_Templates_From_Fit(function, "BDT");

        //#############################################
        //  CONTROL TREES & HISTOGRAMS
        //#############################################
        float cut_BDT_value = -99;
        bool use_pseudodata_CR_plots = false;
        bool cut_on_BDT = false;
        if(isWZ)  cut_on_BDT = false; //No BDT in WZ CR

        if(cut_on_BDT) {cut_BDT_value = MVAtool->Determine_Control_Cut();}
        // MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDT, cut_BDT_value, use_pseudodata_CR_plots);
        // //
        // MVAtool->Create_Control_Histograms(fakes_from_data, use_pseudodata_CR_plots, fakes_summed_channels); //NOTE : very long ! You should only activate necessary syst./var. !

        // if(use_pseudodata_CR_plots) {MVAtool->Generate_PseudoData_Histograms_For_Control_Plots(fakes_from_data);}

        //#############################################
        //  DRAW PLOTS
        //#############################################
        bool draw_plots = true; //Draw Control & Template plots
        bool postfit = false; //Decide if want prefit OR combine postfit plots of input vars (NB : different files)

        if(draw_plots)
        {
            for(int i=0; i<thechannellist.size(); i++) //SINGLE CHANNELS
            {
                MVAtool->Draw_Control_Plots(thechannellist[i], fakes_from_data, false, postfit); //Draw plots for the BDT CR
                // MVAtool->Plot_Templates(thechannellist[i], template_name, false); //Plot the prefit templates
                // MVAtool->Plot_Templates_from_Combine(thechannellist[i], template_name, false); //Postfit templates from Combine file
                // MVAtool->Compare_Negative_Weights_Effect_On_Distributions(thechannellist[i], false);
            }

            // --- ALL CHANNELS

            MVAtool->Draw_Control_Plots("", fakes_from_data, true, postfit);
            // MVAtool->Plot_Templates("", template_name, true); //Plot the prefit templates
            // MVAtool->Plot_Templates_from_Combine("", template_name, true); //Postfit templates from Combine file
            // MVAtool->Compare_Negative_Weights_Effect_On_Distributions("", true);
        }
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
        //vector<TString> v_NJets_cut_values;
        //vector<TString> v_NBJets_cut_values;

        vector<TString> v_METpt_cut_values;
        v_METpt_cut_values.push_back(">10");

        vector<TString> v_mTW_cut_values;
        v_mTW_cut_values.push_back(">10");

        vector<TString> v_isoMu_cut_values;
        v_isoMu_cut_values.push_back("<0.075");
        /*v_isoMu_cut_values.push_back("<0.125");
        v_isoMu_cut_values.push_back("<0.100");
        v_isoMu_cut_values.push_back("<0.075");
        v_isoMu_cut_values.push_back("<0.050");
        v_isoMu_cut_values.push_back("<0.025"); */

        vector<TString> v_isoEl_cut_values;
        v_isoEl_cut_values.push_back("");
        /*v_isoEl_cut_values.push_back("<0.040");
        v_isoEl_cut_values.push_back("<0.035");
        v_isoEl_cut_values.push_back("<0.030");
        v_isoEl_cut_values.push_back("<0.025");
        v_isoEl_cut_values.push_back("<0.020");
        v_isoEl_cut_values.push_back("<0.015");
        v_isoEl_cut_values.push_back("<0.010");*/

        //#############################################
        //  LOOP ON THE 2 CUT VECTORS YOU WANT TO SCAN
        //#############################################

//-- BE CAREFUL HERE !! CALL RELEVANT FUNCTIONS ONLY, AND MAKE SURE THE LOOP ITERATORS ARE ASSOCIATED TO THE CORRECT VECTORS !
//------------------------------

        //SCAN LOOP
        //NOTE : there should only be 2 scanned variables (2 loops) -- all the other cuts should be specified explicitely for each region
        for(int h = 0; h < v_METpt_cut_values.size(); h++)
        {
	    for(int i = 0; i < v_mTW_cut_values.size(); i++)
	    {
	    for(int j = 0; j < v_isoMu_cut_values.size(); j++)
		{
		for(int k=0; k < v_isoEl_cut_values.size(); k++)
		{

            //#############################################
            //               WZ CR Templates
            //#############################################
            //Use scope delimiters so that I can re-use the exact same vectors names for each region (different scopes)
		    {
    			std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
    			scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">0");     scan_v_cut_IsUsedForBDT.push_back(true);
    			//scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">1");     scan_v_cut_IsUsedForBDT.push_back(true);
    			scan_v_cut_name.push_back("NBJets");    scan_v_cut_def.push_back("==0");    scan_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant be used in BDT


    			scan_v_cut_name.push_back("METpt"); scan_v_cut_IsUsedForBDT.push_back(false);
    			//scan_v_cut_def.push_back(v_METpt_cut_values[h]);
    			scan_v_cut_def.push_back("");

    			scan_v_cut_name.push_back("mTW"); scan_v_cut_IsUsedForBDT.push_back(false);
    			//scan_v_cut_def.push_back(v_mTW_cut_values[i]);
    			scan_v_cut_def.push_back("");

    			scan_v_cut_name.push_back("AdditionalMuonIso"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_isoMu_cut_values[j]);

    			//scan_v_cut_name.push_back("AdditionalEleIso"); scan_v_cut_IsUsedForBDT.push_back(false);
    			//scan_v_cut_def.push_back(v_isoEl_cut_values[k]);

    			theMVAtool* MVAtool_WZ = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, false, true, format); if(MVAtool_WZ->stop_program) {return 1;}
    			MVAtool_WZ->Set_Luminosity(set_luminosity);

    			TString template_name_WZ = "mTW"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    			MVAtool_WZ->Read(template_name_WZ, fakes_from_data, real_data_templates, fakes_summed_channels);
    			if(!real_data_templates) {MVAtool_WZ->Generate_PseudoData_Templates(template_name_WZ);}

    			/** mmm
    			   for(int i=0; i<thechannellist.size(); i++)
    			   {
    			   //MVAtool_WZ->Plot_Templates(thechannellist[i], template_name_WZ); //Plot the BDT distributions of MC & pseudo-data templates
    			   }
    			**/

    			MVAtool_WZ->~theMVAtool();
		    }

            //#############################################
            //               ttZ CR Templates
            //#############################################
            {
    			std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
    			scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">1"); scan_v_cut_IsUsedForBDT.push_back(true);
    			scan_v_cut_name.push_back("NBJets");    scan_v_cut_def.push_back(">1");     scan_v_cut_IsUsedForBDT.push_back(true);

    			scan_v_cut_name.push_back("METpt"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_METpt_cut_values[h]);
    			//scan_v_cut_def.push_back("");

    			scan_v_cut_name.push_back("mTW"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_mTW_cut_values[i]);
    			//scan_v_cut_def.push_back("");

    			scan_v_cut_name.push_back("AdditionalMuonIso"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_isoMu_cut_values[j]);

    			//scan_v_cut_name.push_back("AdditionalEleIso"); scan_v_cut_IsUsedForBDT.push_back(false);
    			//scan_v_cut_def.push_back(v_isoEl_cut_values[k]);

    			theMVAtool* MVAtool_ttZ = new theMVAtool(thevarlist_ttZ, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, true, false, format); if(MVAtool_ttZ->stop_program) {return 1;}
    			MVAtool_ttZ->Set_Luminosity(set_luminosity);

                TString bdt_type_ttZ = "BDTttZ"; //'BDT' or 'BDTttZ'
    			for(int i=0; i<thechannellist.size(); i++)
    			  {
    			    MVAtool_ttZ->Train_Test_Evaluate(thechannellist[i], bdt_type_ttZ, use_ttZMad_training);
    			  }

    			TString template_name_ttZ = "BDTttZ"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    			MVAtool_ttZ->Read(template_name_ttZ, fakes_from_data, real_data_templates, fakes_summed_channels);
    			if(!real_data_templates) {MVAtool_ttZ->Generate_PseudoData_Templates(template_name_ttZ);}

    			/**
    			   for(int i=0; i<thechannellist.size(); i++)
    			   {
    			   //MVAtool_ttZ->Plot_Templates(thechannellist[i], template_name_ttZ); //Plot the BDT distributions of MC & pseudo-data templates
    			   }
    			**/
    			MVAtool_ttZ->~theMVAtool();
		    }

            //#############################################
            //               tZq SR Templates
            //#############################################
            {
    			std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
    			scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">1 && <4");     scan_v_cut_IsUsedForBDT.push_back(true);
    			scan_v_cut_name.push_back("NBJets");    scan_v_cut_def.push_back("==1");    scan_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant be used in BDT

    			scan_v_cut_name.push_back("METpt"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_METpt_cut_values[h]);
    			//scan_v_cut_def.push_back("");

    			scan_v_cut_name.push_back("mTW"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_mTW_cut_values[i]);
    			//scan_v_cut_def.push_back("");

    			scan_v_cut_name.push_back("AdditionalMuonIso"); scan_v_cut_IsUsedForBDT.push_back(false);
    			scan_v_cut_def.push_back(v_isoMu_cut_values[j]);

    			//scan_v_cut_name.push_back("AdditionalEleIso"); scan_v_cut_IsUsedForBDT.push_back(false);
    			//scan_v_cut_def.push_back(v_isoEl_cut_values[k]);

    			theMVAtool* MVAtool_tZq = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, false, false, format); if(MVAtool_tZq->stop_program) {return 1;}
    			MVAtool_tZq->Set_Luminosity(set_luminosity);

                TString bdt_type_tZq = "BDT"; //'BDT' or 'BDTttZ'
    			for(int i=0; i<thechannellist.size(); i++)
    			  {
    			    MVAtool_tZq->Train_Test_Evaluate(thechannellist[i], bdt_type_tZq, use_ttZMad_training);
    			  }

    			TString template_name_tZq = "BDT"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    			MVAtool_tZq->Read(template_name_tZq, fakes_from_data, real_data_templates, fakes_summed_channels);
    			if(!real_data_templates) {MVAtool_tZq->Generate_PseudoData_Templates(template_name_tZq);}

    			/**  mmm
    			   for(int i=0; i<thechannellist.size(); i++)
    			   {
    			   //MVAtool_tZq->Plot_Templates(thechannellist[i], template_name_tZq); //Plot the BDT distributions of MC & pseudo-data templates
    			   }
    			**/

    			MVAtool_tZq->~theMVAtool();
            }

        } //end second scanned variable loop
        } //end first scanned variable loop
        }
        }
    } //End optimization Scan




  return 0;
}
