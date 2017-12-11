#include "theMVAtool.h"
#include "Func_other.h" //Helper functions

using namespace std;

int main(int argc, char **argv) //Can choose region (tZq/WZ/ttZ) at execution
{
    if(argc > 2 || ( strcmp(argv[1],"tZq") && strcmp(argv[1],"ttZ") && strcmp(argv[1],"WZ") ) ) //String comparison -- strcmp returns 0 if both contents are equal
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

//----------------------
    //--- NEW : set to TRUE (and run code with arg. "tZq") to run in region "=1bjet, =0light". Then merge with other templates and use dedicated datacard
    bool SR_new_0light = false;
    //-- NEW : set to TRUE if want to have combined templates (mTW+BDT) in SR rather than simply BDT
    bool use_mTWandBDT_SR = false;
    //-- NEW : set to TRUE if want to use dedicated BDT (including Fakes) in WZ region insteand of mTW
    bool use_BDT_WZregion = false;
    //-- NEW : set to TRUE if want to create a BDT trained to separate fakes from the rest in SR
    bool use_BDTfake_SR = false;
//----------------------


    //Matrix Element Method ==> TRUE
    bool include_MEM_variables = true;


    //To keep only low of high-BDT events
    bool cut_on_BDTtZq = false; //FOR BLINDING SIGNAL REGION (--> observed signif.)
    bool cut_on_BDTfakeSR = false; //To cut on dedicated BDTfake in SR
        bool keep_high_BDT_events = true; //if false : keep only low BDT events (blind)
        double cut_BDT_value = 0.;
        bool define_cut_auto = false; //define cut based on bkg/sig contamination


    //Luminosity
    //NOTE: this value is compared to a hard-coded value in theMVAtool.cc & a rescaling factor is computed in case they are different
    //NB2 : careful to Fakes rescaling ! If interested in different lumi for prediction, you should probably comment the automatic reweighting and do it manually
    double set_luminosity = 35.862; //Moriond 2017 - 35.862fb

    //Fakes
    bool fakes_from_data = true; //Data-driven fakes (MC fakes : obsolete in most functions!!)

    //Templates options
    int nofbin_templates = 10; //Templates binning ==> 10 bins
    bool real_data_templates = true; //Else, pseudodata (obsolete)

    //Outputs
    TString format = ".pdf"; //'.png' or '.pdf' only
    bool draw_preliminary_label = false; //choose to add or not a label 'Preliminary' (needed for PAS)
    bool combine_naming_convention = true; //To write histograms with Combine names (else, follow Theta conventions)
    //NB : if set to false, some functions might now work



    //----------------------
    //If true, activates only the "optimization" part (@ end of file)
    bool do_optimization_cuts = false;
    bool do_optim_RemoveBDTVars1By1 = false;
    bool do_optim_RemoveWorstBDTVars = false; //HARD CODED VARIABLES TO REMOVE !! CAREFUL !!
    //----------------------

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
//To dis-activate a cut, just set it to "". Use "==" for equality.
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4"); set_v_cut_IsUsedForBDT.push_back(false);

    std::vector<TString > set_v_cut_name;
    std::vector<TString > set_v_cut_def;
    std::vector<bool > set_v_cut_IsUsedForBDT;

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


//-------------------

// ---- Specify here the cuts that you wish to apply to all/specific regions ---


    set_v_cut_name.push_back("ContainsBadJet");  set_v_cut_def.push_back("==0"); set_v_cut_IsUsedForBDT.push_back(false);
    set_v_cut_name.push_back("fourthLep10");  set_v_cut_def.push_back("==0"); set_v_cut_IsUsedForBDT.push_back(false);

    // set_v_cut_name.push_back("ZMass");  set_v_cut_def.push_back(">81 && <101"); set_v_cut_IsUsedForBDT.push_back(false);


    if(!isWZ)
    {
        // set_v_cut_name.push_back("METpt");      set_v_cut_def.push_back(">10");            set_v_cut_IsUsedForBDT.push_back(false);
        // set_v_cut_name.push_back("mTW");      set_v_cut_def.push_back(">10");            set_v_cut_IsUsedForBDT.push_back(false);
    }

//-------------------


//--- DON'T CHANGE Jets cuts here (define the 3 different regions)
//But you can add some cuts which are region-dependant !
    if(!isWZ && !isttZ) //Default selection is Signal Region : 1<NJets<4 && NBJets == 1
    {
        if(!SR_new_0light)
        {
            // set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back("==3");     set_v_cut_IsUsedForBDT.push_back(false);
            set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1 && <4");
            if(use_BDTfake_SR) set_v_cut_IsUsedForBDT.push_back(false);
            else set_v_cut_IsUsedForBDT.push_back(true);
            set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back("==1");          set_v_cut_IsUsedForBDT.push_back(false);
        }
        else
        {
            set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back("==1");     set_v_cut_IsUsedForBDT.push_back(false);
            set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back("==1");          set_v_cut_IsUsedForBDT.push_back(false);
        }
    }
    if(isWZ) //WZ CR Region : NJets > 0 && NBJets == 0
    {
        set_v_cut_name.push_back("NJets");      set_v_cut_def.push_back(">0");          set_v_cut_IsUsedForBDT.push_back(false);
        set_v_cut_name.push_back("NBJets");     set_v_cut_def.push_back("== 0");        set_v_cut_IsUsedForBDT.push_back(false);
    }
    if(isttZ) //ttZ CR Region : NJets>1 && NBJets>1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1");           set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back(">1");           set_v_cut_IsUsedForBDT.push_back(false);
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

    //--- IPHC : Ntuples Interfaced for MEM, divided in 2 sets (WZ region and ttZ+tZq regions)
    if(isWZ) 	dir_ntuples="input_ntuples/ntuples_WZ"; //Without MEM (empty vars)
    else 		dir_ntuples="input_ntuples/ntuples_MEM"; //With MEM
    t_name = "Tree";

    //--- Other
    // if(isWZ) 	dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_WZ/ntuples_lowpT_mediumBTAG"; //Without MEM (empty vars)
    // else 		dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_readyForMEM/ntuples_lowpT_mediumBTAG"; //With MEM
    // if(isWZ) 	dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_WZ/ntuples_lowpT_lowBTAG"; //Without MEM (empty vars)
    // else 		dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_readyForMEM/ntuples_lowpT_lowBTAG"; //With MEM
    // t_name = "Tree";

    //--- CIEMAT's Ntuples ---
    // dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/input_ntuples";
    // dir_ntuples="/home/nico/Bureau/these/tZq/MEM_Interfacing/input_ntuples/JES_split";
    // t_name = "Default";



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
    thechannellist.push_back("uue");
    thechannellist.push_back("eeu");
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

    //Signal --- must be placed before backgrounds --- NB : name hard-coded in some functions !
    thesamplelist.push_back("tZqmcNLO");        v_color.push_back(kGreen+1);

    //BKG
    thesamplelist.push_back("WZL");             v_color.push_back(920); //grey
    thesamplelist.push_back("WZB");             v_color.push_back(921); //grey
    thesamplelist.push_back("WZC");             v_color.push_back(922); //grey

    // thesamplelist.push_back("WZ");                 v_color.push_back(922); //grey


    thesamplelist.push_back("ZZ");              v_color.push_back(kYellow+1);
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed-4);
    thesamplelist.push_back("ttW");             v_color.push_back(kRed+3);//Keep ttW & ttH samples together (coloring)
    thesamplelist.push_back("ttH");             v_color.push_back(kRed+3);
    thesamplelist.push_back("STtWll");          v_color.push_back(kOrange+1);

    //FAKES
    if(fakes_from_data)
    {
        thesamplelist.push_back("FakesElectron");           v_color.push_back(kAzure-1); //Data-driven (DD)  //-- Fake lepton can be any of 3l
        thesamplelist.push_back("FakesMuon");           v_color.push_back(kAzure-1); //Data-driven (DD)  //-- Fake lepton can be any of 3l
    }

    //WARNING : OBSOLETE -- don't use MC fakes (or update code) -- must be last samples !
    else
    {
        thesamplelist.push_back("DY");              v_color.push_back(kAzure-2); //MC
        thesamplelist.push_back("TT");              v_color.push_back(kAzure-2); //MC
        // thesamplelist.push_back("WW");              v_color.push_back(kYellow); //MC
    }

    //NEW -- CUSTOM COLORS -- FOR TESTING PURPOSE ONLY !
    //-- Idea : take good-looking/efficient color palettes from web and apply it manually
    bool use_custom_colors = false;

    if(use_custom_colors)
    {
        TColor* col = new TColor(1700, 141./255., 211./255., 199./255.);
        // col.SetRGB(141./255., 211./255., 199./255.);
        // col->SetRGB(251./255., 128./255., 114./255.);
        // col->SetRGB(0.1, 0.2, 0.3);
        cout<<col->GetNumber()<<endl;

        vector<TColor*> v_custom_colors(thesamplelist.size());
        v_custom_colors[0] = new TColor(9001, 166./255., 206./255., 227./255.);
        v_custom_colors[1] = new TColor(9002, 31./255., 120./255., 180./255.);
        v_custom_colors[2] = new TColor(9003, 178./255., 223./255., 138./255.);
        v_custom_colors[3] = new TColor(9004, 51./255., 160./255., 44./255.);
        v_custom_colors[4] = new TColor(9005, 251./255., 251./255., 153./255.);
        v_custom_colors[5] = new TColor(9006, 227./255., 26./255., 28./255.);

        v_color[0] = v_custom_colors[0]->GetNumber();
        v_color[4] = v_custom_colors[1]->GetNumber();
        v_color[5] = v_custom_colors[2]->GetNumber();

        v_color[6] = v_custom_colors[3]->GetNumber();
        v_color[7] = v_custom_colors[3]->GetNumber();

        v_color[8] = v_custom_colors[4]->GetNumber();

        v_color[9] = v_custom_colors[5]->GetNumber();
        v_color[10] = v_custom_colors[5]->GetNumber();
    }

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
    std::vector<TString > thevarlist_BDTfakeSR; //Variables used in BDTfakeSR

//------------------------ for tZq
    thevarlist.push_back("etaQ");
    thevarlist.push_back("LeadJetEta");
    if(!SR_new_0light) thevarlist.push_back("dRjj"); //CHANGED
    if(!use_BDT_WZregion || !isWZ) thevarlist.push_back("btagDiscri");
    thevarlist.push_back("ZEta");
    thevarlist.push_back("mtop");
    thevarlist.push_back("AddLepAsym");
    thevarlist.push_back("AddLepETA");
    thevarlist.push_back("ptQ");
    thevarlist.push_back("dPhiZAddLep");
    if(!use_BDT_WZregion || !isWZ) thevarlist.push_back("dPhiAddLepB");
    thevarlist.push_back("dRAddLepClosestJet");


//-- After optimization, removed 4 vars :
    // thevarlist.push_back("dRAddLepQ");
    // thevarlist.push_back("dRZAddLep");
    // thevarlist.push_back("Zpt");
    // thevarlist.push_back("tZq_pT");

    if(include_MEM_variables && !isWZ)
    {
        thevarlist.push_back("MEMvar_0"); //Likelihood ratio of MEM weigt (S/S+B ?), with x-sec scaling factor
        thevarlist.push_back("MEMvar_1"); //Kinematic Fit Score
        thevarlist.push_back("MEMvar_2"); //Kinematic Fit Score
        thevarlist.push_back("MEMvar_8");
    }


//--- MEM variables definitions :

    // thevarlist.push_back("-log((3.89464e-13*mc_mem_ttz_weight) / (3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"); //MEMvar_0
    // thevarlist.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //MEMvar_1
    // thevarlist.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //MEMvar_2
    // thevarlist_ttZ.push_back("-log(mc_mem_ttz_tllj_likelihood)"); //MEMvar_3
    // thevarlist.push_back("log(mc_mem_wzjj_weight)"); //MEMvar_4
    // thevarlist.push_back("log(mc_mem_wzjj_weight_kinmaxint)"); //MEMvar_5
    // thevarlist.push_back("-log((0.017*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight) / (0.017*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))");//MEMvar_6
    // thevarlist.push_back("-log((0.017*mc_mem_wzjj_weight) / (0.017*mc_mem_wzjj_weight + 0.17993*mc_mem_tllj_weight) )"); //MEMvar_7
    // discriminant_TZQ_TTZandWZ_SR = "-log((0.017e-10*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight) / (0.017e-10*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"; //MEMvar_8



//------------------------ for ttZ
    thevarlist_ttZ.push_back("dRjj");
    thevarlist_ttZ.push_back("etaQ");
    thevarlist_ttZ.push_back("ptQ");
    thevarlist_ttZ.push_back("btagDiscri");
    thevarlist_ttZ.push_back("dRAddLepClosestJet");
    thevarlist_ttZ.push_back("Zpt");
    thevarlist_ttZ.push_back("AddLepAsym");
    thevarlist_ttZ.push_back("mtop");
    thevarlist_ttZ.push_back("dRZTop");
    thevarlist_ttZ.push_back("dRAddLepQ");
    thevarlist_ttZ.push_back("ZEta");
    thevarlist_ttZ.push_back("dPhiZAddLep");

//--- After optimization, removed 8 variables : (+NBJets & MEMvar_2)
    // thevarlist_ttZ.push_back("AddLepETA");
    // thevarlist_ttZ.push_back("dRZAddLep"); //
    // thevarlist_ttZ.push_back("dRAddLepB");
    // thevarlist_ttZ.push_back("TopPT"); //
    // thevarlist_ttZ.push_back("m3l");
    // thevarlist_ttZ.push_back("tZq_pT");

    if(include_MEM_variables){
        thevarlist_ttZ.push_back("MEMvar_1"); //Kinematic Fit Score
        thevarlist_ttZ.push_back("MEMvar_3"); //Likelihood ratio of MEM weight (S/S+B ?)

        // thevarlist_ttZ.push_back("MEMvar_2"); //Kinematic Fit Score
    }


    //CHANGED --- NEW BDT IN SR TO SEPARATE FAKES FROM SIGNAL (then cut on it)
    thevarlist_BDTfakeSR.push_back("mTW");
    thevarlist_BDTfakeSR.push_back("thirdLeptonPT");
    thevarlist_BDTfakeSR.push_back("etaQ");
    thevarlist_BDTfakeSR.push_back("LeadingJetNonB_pT");
    thevarlist_BDTfakeSR.push_back("secondLeptonPT");
    // thevarlist_BDTfakeSR.push_back("METpt");
    // thevarlist_BDTfakeSR.push_back("btagDiscri");
    // thevarlist_BDTfakeSR.push_back("leadingLeptonPT");
    // thevarlist_BDTfakeSR.push_back("ptQ");
    // thevarlist_BDTfakeSR.push_back("dRAddLepB");

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
//Can add additionnal vars which are NOT used in TMVA NOR for cuts, only for CR plots !
//NOTE : Branch can be linked to only *one* variable via SetBranchAddress ; if additional variable is already present in other variable vector, it is removed from this vector !

    vector<TString> v_add_var_names;

    v_add_var_names.push_back("mTW");
    v_add_var_names.push_back("METpt");
    v_add_var_names.push_back("TopPT");
    v_add_var_names.push_back("Zpt");
    v_add_var_names.push_back("AdditionalMuonIso");
    v_add_var_names.push_back("AdditionalEleIso");

    //--- NOT PRESENT IN CIEMAT NTUPLES
    if(t_name == "Tree")
    // if(t_name == "Tree" || t_name == "Default")
    {
        v_add_var_names.push_back("tZ_pT");
        v_add_var_names.push_back("tZ_mass");
        v_add_var_names.push_back("bj_mass_leadingJet");
        v_add_var_names.push_back("bj_mass_subleadingJet");
        v_add_var_names.push_back("bj_mass_leadingJet_pT40");
        v_add_var_names.push_back("bj_mass_leadingJet_pT50");
        v_add_var_names.push_back("bj_mass_leadingJet_pTlight40");
        v_add_var_names.push_back("bj_mass_leadingJet_pTlight50");
        v_add_var_names.push_back("bj_mass_leadingJet_etaCut");
        v_add_var_names.push_back("LeadingJetCSV");
        v_add_var_names.push_back("SecondJetCSV");
    }


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

//--- Stored in vectors (class members) - only for data-driven Fakes
    // if(fakes_from_data) {systematics_names_tmp.push_back("ZptReweight");} //---OBSOLETE

//--- Stored in separate Ntuples (tZq only)
    systematics_names_tmp.push_back("PSscale");

//--- Stored in separate Trees
    systematics_names_tmp.push_back("JER");
    systematics_names_tmp.push_back("JES");
    systematics_names_tmp.push_back("Fakes");
    // systematics_names_tmp.push_back("JESFWD"); //FIXME

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
        thesystlist.push_back( systematics_names_tmp[isyst] + "__plus" ); //Theta convention is used in Ntuples
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


    if(!do_optimization_cuts && !do_optim_RemoveBDTVars1By1)
    {

//*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !
//Some additional functions can be activated "by hand" at the end of this scope

//-----------------    TRAINING
        bool train_BDT = false; //Train BDT (if region is tZq or ttZ)

//-----------------    TEMPLATES CREATION
        bool create_templates = false; //Create templates in selected region (NB : to cut on BDT value, use dedicated boolean in 'OPTIONS' section)

//-----------------    CONTROL HISTOGRAMS
        bool create_control_histograms = false; //Create histograms of input variables, needed to make plots of these variables -- Takes time !

//-----------------    PLOTS
        bool draw_input_vars = false ; //Plot input variables
        bool draw_templates = false; //Plot templates (mTW/BDT/BDTttZ)

        bool postfit = false; //Decide if want prefit OR combine postfit plots (NB : use different files)

//-----------------    OTHER
        // bool convert_templates_for_theta = false; //Use this if you already produced template files with Combine conventions, and want to convert them to Theta

//-----------------



//---> AUTOMATIZED FUNCTION CALLS FROM BOOLEANS

        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################

        //Initialization
        std::vector<TString > thevarlist_tmp;
        if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
        else if(!isWZ && !isttZ && use_BDTfake_SR) {thevarlist_tmp = thevarlist_BDTfakeSR;}
        else {thevarlist_tmp = thevarlist;}

        theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity);

        //Template
        TString template_name = "BDT"; //BDT output in Signal Region
        if(!isttZ && !isWZ)
        {
            if(SR_new_0light) template_name = "BDT0l";
            else if(use_mTWandBDT_SR) template_name = "mTWandBDT";
            else if(use_BDTfake_SR) template_name = "BDTfakeSR";
        };
        if(isttZ)  template_name = "BDTttZ"; //BDTttZ output in ttZ Control region
        if(isWZ)
        {
            template_name = "mTW"; //mTW distribution in WZ Control region
            if(use_BDT_WZregion) template_name = "BDTfake";
        }

        //BDT cut
        // float cut_BDT_value = -99; //Initialization -- used to cut on BDT & stay blind
        bool cut_on_BDTtZq_tmp = cut_on_BDTtZq; //Hard-coded : 1 boolean needed for production and the other for plotting
        if(isWZ || isttZ)  cut_on_BDTtZq_tmp = false; //No BDT in WZ CR ; & don't cut on BDTttZ for now

        //#############################################
        // TRAINING
        //#############################################

        //Train BDTs in all 4 channels
        for(int i=0; i<thechannellist.size(); i++)
        {
            if(train_BDT && template_name.Contains("BDT"))
            {
                MVAtool->Train_Test_Evaluate(thechannellist[i], template_name, true);
            }
            // MVAtool->Train_Test_Evaluate("all", template_name, true);

        }


        //#############################################
        //  TEMPLATES CREATION
        //#############################################

        if(create_templates)
        {
            if(cut_on_BDTtZq_tmp && define_cut_auto) {cut_BDT_value = MVAtool->Determine_Control_Cut();}
            MVAtool->Read(template_name, fakes_from_data, real_data_templates, cut_on_BDTtZq_tmp, keep_high_BDT_events, cut_BDT_value, cut_on_BDTfakeSR);
        }

        //#############################################
        //  CONTROL TREES & HISTOGRAMS
        //#############################################

        //Create trees and then histograms, which can later be used to create plots
        if(create_control_histograms)
        {
            if(cut_on_BDTtZq_tmp && define_cut_auto) {cut_BDT_value = MVAtool->Determine_Control_Cut();}

            MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDTtZq_tmp, cut_BDT_value, !real_data_templates, keep_high_BDT_events);

            MVAtool->Create_Control_Histograms(fakes_from_data, false, cut_on_BDTtZq_tmp);
        }

        //#############################################
        //  DRAW PLOTS
        //#############################################

        // --- SINGLE CHANNELS
        for(int i=0; i<thechannellist.size(); i++)
        {
            if(draw_input_vars) MVAtool->Draw_Control_Plots(thechannellist[i], fakes_from_data, false, postfit, cut_on_BDTtZq); //Draw plots for the BDT CR
            if(draw_templates)
            {
                if(!postfit) MVAtool->Plot_Prefit_Templates(thechannellist[i], template_name, false, cut_on_BDTtZq); //Plot the prefit templates
                else MVAtool->Plot_Postfit_Templates(thechannellist[i], template_name, false, cut_on_BDTtZq); //Postfit templates from Combine file
            }
        }

        // --- ALL CHANNELS
        if(draw_input_vars) MVAtool->Draw_Control_Plots("", fakes_from_data, true, postfit, cut_on_BDTtZq);
        if(draw_templates)
        {
            if(!postfit) MVAtool->Plot_Prefit_Templates("all", template_name, true, cut_on_BDTtZq); //Plot the prefit templates
            else MVAtool->Plot_Postfit_Templates("all", template_name, true, cut_on_BDTtZq); //Postfit templates from Combine file
        }

        //#############################################
        //  Convert Templates names for Theta
        //#############################################
        // if(convert_templates_for_theta) {MVAtool->Convert_Templates_Theta();}



        //-----------------
        //#############################################
        //  Additional functions
        //#############################################

        // TString file_to_rescale = "outputs/Combine_Input.root";
        // TString file_to_rescale = "./outputs/Reader_"+template_name+MVAtool->filename_suffix+"_unScaled.root";
        // TString file_to_rescale = "./outputs/Reader_BDTfakeSR_badMuonEq0_ContainsBadJetEq0_fourthLep10Eq0_NJetsMin1Max4_NBJetsEq1.root";
        // MVAtool->Rescale_Fake_Histograms(file_to_rescale, "mTW"); //To rescale manually the fakes in a template file -- Make sure it wasn't rescaled yet !!

        for(int ichan=0; ichan<thechannellist.size(); ichan++)
        {
            // MVAtool->Superpose_With_Without_MEM_Templates(template_name, thechannellist[ichan], true);

            // MVAtool->Superpose_Shapes_Fakes_Signal(template_name, thechannellist[ichan], true, true, true);

            // MVAtool->Draw_Template_With_Systematic_Variation(thechannellist[ichan], "mTW", "WZL", "JES");

            // MVAtool->Compare_Negative_Or_Absolute_Weight_Effect_On_Distributions(thechannellist[ichan], false);

            // MVAtool->Fit_mTW_SR(thechannellist[ichan]);
        }


        // MVAtool->Superpose_With_Without_MEM_Templates(template_name, "allchan", true);

        // MVAtool->Superpose_Shapes_Fakes_Signal(template_name, "allchan", true, true, true);

        // MVAtool->Rebin_Template_File("./outputs/binning/Combine_Input_40Bins_HalfStat.root", 10);

        // MVAtool->Compare_Negative_Or_Absolute_Weight_Effect_On_Distributions("allchan", true);

        // vector<bool> v;
        // MVAtool->Vector_isEventPassingBDTfakeSRCut(v, "Data", "", cut_BDT_value, keep_high_BDT_events);

        // MVAtool->Draw_Control_Plots_ForPaper();
        // MVAtool->Draw_Control_Plots_ForPaper_WZ();
        // MVAtool->Postfit_Templates_Paper();

        // MVAtool->Rescale_JES();
        // MVAtool->Count_Events();
        // MVAtool->Histograms_For_Denis();
        MVAtool->Postfit_Templates_Paper_SinglePlot();

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

 // !! Before running the scan remotely (with "screen"), verify that :
 //the cut list, the path of the ntuples, the syst. are ON, the 2 mTW template file are already in the 'outputs' dir. (same files used for all points), the datacard is already generated, etc.

    //---------------------------------------------
    //---------------------------------------------
    //---------------------------------------------



//--- Apply MET&mTW cuts, Train BDT + produce templates (tZq & ttZ regions only), then merge with the 2 other nominal templates files
//--- Then compute expected significance FOR EACH CHANNEL SEPARATELY, and store the results in file. Thus we get for each channel the set of cuts which maximizes the significance

    if(do_optimization_cuts) // NB : need to choose cuts range, & set boolean = true (top of code)
    {
        TString filename_suffix_tmp;
        TString filename_suffix_noJet; //Without the jet suffixes

        //Derive the correct suffix according to the current set of cuts applied
        for(int ivar=0; ivar<set_v_cut_name.size(); ivar++)
        {
            if(set_v_cut_def[ivar] != "")
            {
                TString tmp;

                if(set_v_cut_def[ivar].Contains("||") ) {continue;} //Don't add the 'or' conditions in filename
                else if(!set_v_cut_def[ivar].Contains("&&")) //Single condition
                {
                    tmp = "_" + set_v_cut_name[ivar] + Convert_Sign_To_Word(set_v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(set_v_cut_def[ivar]));
                }
                else //Double '&&' condition
                {
                    TString cut1 = Break_Cuts_In_Two(set_v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(set_v_cut_def[ivar]).second;
                    tmp = "_" + set_v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                    tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
                }

                filename_suffix_tmp+= tmp;
                if(set_v_cut_name[ivar] == "NJets" || set_v_cut_name[ivar] == "NBJets") {continue;}
                filename_suffix_noJet+= tmp;
            }
        }

        if(!Check_File_Existence("./outputs/Reader_mTW"+filename_suffix_noJet+"_NJetsMin0_NBJetsEq0_unScaled.root") ) //needed for fakes scaling, etc.
        {
            cout<<endl<<BOLD(FRED("./outputs/Reader_mTW"+filename_suffix_noJet+"_NJetsMin0_NBJetsEq0_unScaled.root missing BUT needed for Fakes rescaling ! Abort"))<<endl<<endl; return 0;
        }

        if(isWZ) {cout<<endl<<BOLD(FRED("No MET/mTW cuts in the WZ control region ! Abort"))<<endl<<endl; return 0;}



        //#############################################
        //  SET THE CUT DEFINITIONS ON WHICH YOU WANT TO LOOP
        //#############################################

        //For scan -- NOTE : different values for each region

        TString cut1_name = "METpt";
        vector<TString> v_cut1_values;
        // v_cut1_values.push_back(">0");


        TString cut2_name = "mTW";
        vector<TString> v_cut2_values;
        // v_cut2_values.push_back(">0");

        //-- 2D scan of MET & mTW
        //--- Change here the points to scan !!
        for(int icut=0; icut<=30; icut+=10)
        {
            TString cut_def = ">" + Convert_Number_To_TString(icut);

            v_cut1_values.push_back(cut_def);
        }
        for(int icut=0; icut<=60; icut+=15)
        {
            TString cut_def = ">" + Convert_Number_To_TString(icut);

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
                if( ((cut1_name=="mTW" || cut1_name=="METpt") && v_cut1_values[icut1]==">0") && ((cut2_name=="mTW" || cut2_name=="METpt") && v_cut2_values[icut2]==">0") )
                {
                    continue; //This is equivalent to the "nominal" samples without cuts
                }

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
                theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name_optim, set_v_cut_def_optim, set_v_cut_IsUsedForOptim_BDTvar, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
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
                        MVAtool->Train_Test_Evaluate(thechannellist[ichan], template_name, false);
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
                    MVAtool->Read(template_name, fakes_from_data, real_data_templates, false, keep_high_BDT_events, -99, cut_on_BDTfakeSR);
                }

                MoveFile( ("./outputs/Reader_"+template_name+MVAtool->filename_suffix+".root"), ("outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2])) ) ); //Move file


                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object

            } //end second scanned variable loop
        } //end first scanned variable loop







        //-------------------------
        //CREATE COMBINE INPUT FILES, RUN COMBINE ON ALL FILES, STORE SIGNIFICANCE VALUES

        bool use_syst = true;
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
        theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity); //only used to call a class-function, initialization not important

        for(int icut1 = 0; icut1 < v_cut1_values.size(); icut1++)
        {
    	    for(int icut2 = 0; icut2 < v_cut2_values.size(); icut2++)
    	    {
                if( ((cut1_name=="mTW" || cut1_name=="METpt") && v_cut1_values[icut1]==">0") && ((cut2_name=="mTW" || cut2_name=="METpt") && v_cut2_values[icut2]==">0") )
                {
                    continue; //This is equivalent to the "nominal" samples without cuts
                }

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

                TString file_BDT_path = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1"+filename_suffix_tmp+".root";
                TString file_BDTttZ_path = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1"+filename_suffix_tmp+".root";

                if(!Check_File_Existence(file_BDT_path) ) {cout<<file_BDT_path<<" not found ! "<<BOLD(FRED("CONTINUE!"))<<endl;}
                if(!Check_File_Existence(file_BDTttZ_path) ) {cout<<file_BDTttZ_path<<" not found ! "<<BOLD(FRED("CONTINUE!"))<<endl;}

                TString combine_file_path = "outputs/optim_cuts/"+cut1_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut1_values[icut1]))+cut2_name+Convert_Number_To_TString(Find_Number_In_TString(v_cut2_values[icut2]))+"/Combine_Input.root";

                system( ("hadd -f "+combine_file_path+" ./outputs/Reader_mTW"+filename_suffix_noJet+"_NJetsMin0_NBJetsEq0.root "+file_BDT_path + " " + file_BDTttZ_path).Data() );


                TString output_text = cut1_name+v_cut1_values[icut1]+"&&"+cut2_name+v_cut2_values[icut2];

                //--- To compute significance for each channel separately
                // file_out<<endl<<endl<<output_text<<" : "<<endl;
                // for(int ichan=0; ichan<thechannellist.size(); ichan++)
                {
                    // float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( combine_file_path, signal, thechannellist[ichan], expected, use_syst); //Separately for each chan
                    // file_out<<thechannellist[ichan]<<" ---> "<<signif<<endl;
                }

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( combine_file_path, signal, "", expected, use_syst);

                file_out<<output_text<<" ---> "<<signif<<endl;
            }
        }

        delete MVAtool;

        Order_Cuts_By_Decreasing_Signif_Loss(file_significances.Data() );

    } //End optimization Scan





    //---------------------------------------------
    //---------------------------------------------
    //---------------------------------------------





//---------------------------------------------------------------------------
 // # #    # #####  #    # #####    #    #   ##   #####  #   ##   #####  #      ######  ####
 // # ##   # #    # #    #   #      #    #  #  #  #    # #  #  #  #    # #      #      #
 // # # #  # #    # #    #   #      #    # #    # #    # # #    # #####  #      #####   ####
 // # #  # # #####  #    #   #      #    # ###### #####  # ###### #    # #      #           #
 // # #   ## #      #    #   #       #  #  #    # #   #  # #    # #    # #      #      #    #
 // # #    # #       ####    #        ##   #    # #    # # #    # #####  ###### ######  ####
//---------------------------------------------------------------------------

    if(do_optim_RemoveBDTVars1By1)
    {
        TString filename_suffix_tmp;
        TString filename_suffix_noJet; //Without the jet suffixes

        //Derive the correct suffix according to the current set of cuts applied
        for(int ivar=0; ivar<set_v_cut_name.size(); ivar++)
        {
            if( (set_v_cut_name[ivar]=="METpt" || set_v_cut_name[ivar]=="mTW") && set_v_cut_def[ivar] == ">0") {continue;} //Useless cuts
            else if(set_v_cut_name[ivar]=="fourthLep10" || set_v_cut_name[ivar]=="ContainsBadJet") {continue;} // not used in suffix

            if(set_v_cut_def[ivar] != "")
            {
                TString tmp;

                if(set_v_cut_def[ivar].Contains("||") ) {continue;} //Don't add the 'or' conditions in filename
                else if(!set_v_cut_def[ivar].Contains("&&")) //Single condition
                {
                    tmp = "_" + set_v_cut_name[ivar] + Convert_Sign_To_Word(set_v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(set_v_cut_def[ivar]));
                }
                else //Double '&&' condition
                {
                    TString cut1 = Break_Cuts_In_Two(set_v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(set_v_cut_def[ivar]).second;
                    tmp = "_" + set_v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                    tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
                }

                filename_suffix_tmp+= tmp;
                if(set_v_cut_name[ivar] == "NJets" || set_v_cut_name[ivar] == "NBJets") {continue;}
                filename_suffix_noJet+= tmp;
            }
        }

        if(isWZ) {cout<<BOLD(FRED("No BDT in WZ Control Region !"))<<endl; return 0;}
        if(!isWZ && !isttZ && !Check_File_Existence("./outputs/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root")) {cout<<BOLD(FRED("./outputs/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root DOES NOT EXIST ! You first need to create the 'nominal' templates (with complete BDT variable lists), and THEN run the optimization code ! ABORT"))<<endl; return 0;}
        if(isttZ && !Check_File_Existence("./outputs/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root")) {cout<<BOLD(FRED("./outputs/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root DOES NOT EXIST ! You first need to create the 'nominal' templates (with complete BDT variable lists), and THEN run the optimization code ! ABORT"))<<endl; return 0;}

        vector<TString > thevarlist_tmp; //Tmp input vars list, used in loop to remove vars 1 by 1
        if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
        else       thevarlist_tmp = thevarlist;

        if(!Check_File_Existence("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root") ) //needed for fakes scaling
        {
            cout<<endl<<BOLD(FRED("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root not found! (needed to compute data Fakes SF) Abort! "))<<endl<<endl; return 0;
        }

        cout<<BOLD(FGRN("WILL TAKE CURRENT READER FILES LOCATED IN './outputs/' AS NOMINAL TEMPLATES FOR SIGNIFICANCE COMPUTATION !"))<<endl;
        mkdir("outputs/Templates_nominal", 0777);
        CopyFile( ("./outputs/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root"), ("./outputs/Templates_nominal/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root") );
        CopyFile( ("./outputs/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root"), ("./outputs/Templates_nominal/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root") );
        CopyFile( ("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root"), ("./outputs/Templates_nominal/Reader_mTW_NJetsMin0_NBJetsEq0.root") );


        //--- Technical issue : some variables in the cuts vectors are also used in BDT ; need to take them into account as well, to remove them
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

            vector<TString > varlist_optim; //Var list which is actually used
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
                //To remove a var defined within cut vector, set its 'IsUsed' boolean to false
                if(thevarlist_tmp[ivar] == v_cut_name_optim[j])
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

                theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, v_cut_name_optim, set_v_cut_def, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                //#############################################
                // TRAINING
                //#############################################

                MVAtool->Train_Test_Evaluate(thechannellist[ichan], bdt_type, false);

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

                theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, v_cut_name_optim, set_v_cut_def, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
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

                MVAtool->Read(bdt_type, fakes_from_data, real_data_templates, false, keep_high_BDT_events, -99, cut_on_BDTfakeSR);

                MoveFile( ("./outputs/Reader_"+bdt_type+"*"+MVAtool->filename_suffix+".root"), ("./outputs/optim_BDTvar/"+bdt_type+"/without_"+removed_var_name) ); //Move file

                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }// end chan loop

        } //end var removal loop


        //-------------------------
        //CREATE COMBINE INPUT FILES, RUN COMBINE ON ALL FILES, STORE SIGNIFICANCE VALUES
        //-------------------------

        bool use_syst = true;
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

        theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity);

        for(int ivar=0; ivar<thevarlist_tmp.size(); ivar++)
        {
            if(!isWZ && !isttZ)
            {
                system( ("hadd -f ./outputs/optim_BDTvar/BDT/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root  ./outputs/optim_BDTvar/BDT/without_"+thevarlist_tmp[ivar]+"/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root ./outputs/Templates_nominal/Reader_mTW_NJetsMin0_NBJetsEq0.root ./outputs/Templates_nominal/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root").Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( ("./outputs/optim_BDTvar/BDT/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root"), signal, "", expected, use_syst);

                file_out<<thevarlist_tmp[ivar]<<" ---> "<<signif<<endl;
            }
            else if(!isWZ && isttZ)
            {
                system( ("hadd -f ./outputs/optim_BDTvar/BDTttZ/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root  ./outputs/optim_BDTvar/BDTttZ/without_"+thevarlist_tmp[ivar]+"/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root ./outputs/Templates_nominal/Reader_mTW_NJetsMin0_NBJetsEq0.root ./outputs/Templates_nominal/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root").Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( ("./outputs/optim_BDTvar/BDTttZ/without_"+thevarlist_tmp[ivar]+"/Combine_Input.root"), signal, "", expected, use_syst);

                file_out<<thevarlist_tmp[ivar]<<" ---> "<<signif<<endl;
            }
        }

        delete MVAtool;

        Order_BDTvars_By_Decreasing_Signif_Loss(file_significances.Data() );

    } //end optimization loop








    //---------------------------------------------
    //---------------------------------------------
    //---------------------------------------------




    //--- Once the impact of all individual variables has been computed, can remove sequentially the worst variables and see how it impacts results
    if(do_optim_RemoveWorstBDTVars)
    {
        TString filename_suffix_tmp;
        TString filename_suffix_noJet; //Without the jet suffixes

        for(int ivar=0; ivar<set_v_cut_name.size(); ivar++)
        {
            if( (set_v_cut_name[ivar]=="METpt" || set_v_cut_name[ivar]=="mTW") && set_v_cut_def[ivar] == ">0") {continue;} //Useless cuts

            if(set_v_cut_def[ivar] != "")
            {
                TString tmp;

                if(set_v_cut_def[ivar].Contains("||") ) {continue;} //Don't add the 'or' conditions in filename
                else if(!set_v_cut_def[ivar].Contains("&&")) //Single condition
                {
                    tmp = "_" + set_v_cut_name[ivar] + Convert_Sign_To_Word(set_v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(set_v_cut_def[ivar]));
                }
                else //Double '&&' condition
                {
                    TString cut1 = Break_Cuts_In_Two(set_v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(set_v_cut_def[ivar]).second;
                    tmp = "_" + set_v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                    tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
                }

                filename_suffix_tmp+= tmp;
                if(set_v_cut_name[ivar] == "NJets" || set_v_cut_name[ivar] == "NBJets") {continue;}
                filename_suffix_noJet+= tmp;
            }
        }

        if(isWZ) {cout<<BOLD(FRED("No BDT in WZ Control Region !"))<<endl; return 0;}
        if(!isWZ && !isttZ && !Check_File_Existence("./outputs/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root")) {cout<<BOLD(FRED("./outputs/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root DOES NOT EXIST ! You first need to create the 'nominal' templates (with complete BDT variable lists), and THEN run the optimization code ! ABORT"))<<endl; return 0;}
        if(isttZ && !Check_File_Existence("./outputs/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root")) {cout<<BOLD(FRED("./outputs/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root DOES NOT EXIST ! You first need to create the 'nominal' templates (with complete BDT variable lists), and THEN run the optimization code ! ABORT"))<<endl; return 0;}
        if(!Check_File_Existence("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root") ) //needed for fakes scaling
        {
            cout<<endl<<BOLD(FRED("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root not found! (needed to compute data Fakes SF) Abort! "))<<endl<<endl; return 0;
        }

        cout<<BOLD(FGRN("WILL TAKE CURRENT READER FILES LOCATED IN './outputs/' AS NOMINAL TEMPLATES FOR SIGNIFICANCE COMPUTATION !"))<<endl;
        mkdir("outputs/Templates_nominal", 0777);
        CopyFile( ("./outputs/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root"), ("./outputs/Templates_nominal/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root") );
        CopyFile( ("./outputs/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root"), ("./outputs/Templates_nominal/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root") );
        CopyFile( ("./outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root"), ("./outputs/Templates_nominal/Reader_mTW_NJetsMin0_NBJetsEq0.root") );


        //--- NOTE : HARD-CODED !
        //Define here in which order the BDT vars should be sequentially removed
        vector<TString> v_worst_vars;
        if(!isWZ && !isttZ) //tZq SR
        {
            v_worst_vars.push_back("");
        }
        else if(isttZ) //ttZ CR
        {
            v_worst_vars.push_back("");
        }


        TString bdt_type = "BDT";
        if(isttZ) bdt_type = "BDTttZ";

        TString dir_name;

        vector<TString > varlist_optim; //Var list which is actually used
        if(!isWZ && !isttZ) {varlist_optim = thevarlist;}
        else if(isttZ) {varlist_optim = thevarlist_ttZ;}

        vector<bool > set_v_cut_IsUsedForBDT_optim = set_v_cut_IsUsedForBDT;

        for(int ivar=0; ivar<v_worst_vars.size(); ivar++)
        {

            for(int j=0; j<varlist_optim.size(); j++) //Vars defined in input vars vector
            {
                if(v_worst_vars[ivar]==varlist_optim[j])
                {
                    varlist_optim.erase(varlist_optim.begin() + j);
                    break;
                }
            }
            for(int j=0; j<set_v_cut_name.size(); j++) //For vars defined in cut vector
            {
                if(v_worst_vars[ivar]==set_v_cut_name[j])
                {
                    set_v_cut_IsUsedForBDT_optim[j] = false;
                    break;
                }
            }


            for(int ichan=0; ichan<thechannellist.size(); ichan++)
            {
                dir_name = "outputs/optim_BDTvar";
                mkdir(dir_name.Data() , 0777);
                dir_name = "outputs/optim_BDTvar/RemoveWorstVars";
                mkdir(dir_name.Data() , 0777);
                dir_name = "outputs/optim_BDTvar/RemoveWorstVars/"+bdt_type;
                mkdir(dir_name.Data() , 0777);
                dir_name = "outputs/optim_BDTvar/RemoveWorstVars/"+bdt_type+"/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar";
                mkdir(dir_name.Data() , 0777);

                dir_name = "weights/optim_BDTvar";
                mkdir(dir_name.Data() , 0777);
                dir_name = "weights/optim_BDTvar/RemoveWorstVars";
                mkdir(dir_name.Data() , 0777);
                dir_name = "weights/optim_BDTvar/RemoveWorstVars/weights_without_"+Convert_Number_To_TString(ivar+1)+"WorstVar";
                mkdir(dir_name.Data() , 0777);

                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################

                theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                //#############################################
                // TRAINING
                //#############################################

                MVAtool->Train_Test_Evaluate(thechannellist[ichan], bdt_type, false);

                MoveFile( ("./outputs/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".root"), ("outputs/optim_BDTvar/RemoveWorstVars/"+bdt_type+"/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar") ); //Move file -- useless file?

                MoveFile( ("./weights/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C"), ("weights/optim_BDTvar/RemoveWorstVars/weights_without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".class.C") ); //Move weight file to specific dir.
                MoveFile( ("./weights/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml"), ("weights/optim_BDTvar/RemoveWorstVars/weights_without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/"+bdt_type+"_"+thechannellist[ichan]+MVAtool->filename_suffix+".weights.xml") ); //Move weight file to specific dir.

                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }// chan loop

            for(int ichan=0; ichan<thechannellist.size(); ichan++)
            {
                //#############################################
                //  CREATE INSTANCE OF CLASS & INITIALIZE
                //#############################################

                theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT_optim, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
                MVAtool->Set_Luminosity(set_luminosity);

                //#############################################
                //  TEMPLATES CREATION
                //#############################################

                //Reader needs weight files from ALL FOUR channels -> make sure they are all available in dir. weights/
                for(int jchan=0; jchan<thechannellist.size(); jchan++)
                {
                    CopyFile( ("weights/optim_BDTvar/RemoveWorstVars/weights_without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C"), ("./weights/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".class.C") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                    CopyFile( ("weights/optim_BDTvar/RemoveWorstVars/weights_without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml"), ("./weights/"+bdt_type+"_"+thechannellist[jchan]+MVAtool->filename_suffix+".weights.xml") ); //Copy weight file back to the ./weights/ dir. --> found by Reader !
                }

                MVAtool->Read(bdt_type, fakes_from_data, real_data_templates, false, keep_high_BDT_events, -99, cut_on_BDTfakeSR);

                MoveFile( ("./outputs/Reader_"+bdt_type+"*"+MVAtool->filename_suffix+".root"), ("outputs/optim_BDTvar/RemoveWorstVars/"+bdt_type+"/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar") ); //Move file

                //------------------------
                //------------------------
                MVAtool->~theMVAtool(); //Delete object
            }// end chan loop

        } //end var removal loop


        //-------------------------
        //CREATE COMBINE INPUT FILES, RUN COMBINE ON ALL FILES, STORE SIGNIFICANCE VALUES
        //-------------------------

        bool use_syst = true;
        bool expected = true;
        TString signal = "tZq";


        TString file_significances = "BDT_OPTIM_REMOVE_WORST.txt";
        if(isttZ) file_significances = "BDTttZ_OPTIM_REMOVE_WORST.txt";
        ofstream file_out(file_significances.Data() );
        file_out<<" -- Significances computed from Combine, obtained by removing the worst input variables sequentially --"<<endl;
        file_out<<"use_syst = "<<use_syst<<endl;
        file_out<<"expected = "<<expected<<endl;
        file_out<<"signal = "<<signal<<endl<<endl<<endl;

        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################

        theMVAtool* MVAtool = new theMVAtool(varlist_optim, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, v_add_var_names, nofbin_templates, isttZ, isWZ, format, combine_naming_convention, dir_ntuples, t_name, thevarlist_BDTfakeSR, draw_preliminary_label); if(MVAtool->stop_program) {return 1;}
        MVAtool->Set_Luminosity(set_luminosity);

        for(int ivar=0; ivar<v_worst_vars.size(); ivar++)
        {
            if(!isWZ && !isttZ)
            {
                system( ("hadd -f ./outputs/optim_BDTvar/RemoveWorstVars/BDT/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/Combine_Input.root  ./outputs/optim_BDTvar/RemoveWorstVars/BDT/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root ./outputs/Templates_nominal/Reader_mTW_NJetsMin0_NBJetsEq0.root ./outputs/Templates_nominal/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root").Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( ("outputs/optim_BDTvar/RemoveWorstVars/BDT/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/Combine_Input.root"), signal, "", expected, use_syst);

                file_out<<"without "+Convert_Number_To_TString(ivar+1)+" worst vars ---> "<<signif<<endl;
            }
            else if(!isWZ && isttZ)
            {
                system( ("hadd -f ./outputs/optim_BDTvar/RemoveWorstVars/BDTttZ/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/Combine_Input.root  ./outputs/optim_BDTvar/RemoveWorstVars/BDTttZ/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/Reader_BDTttZ"+filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root ./outputs/Templates_nominal/Reader_mTW_NJetsMin0_NBJetsEq0.root ./outputs/Templates_nominal/Reader_BDT"+filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root").Data() );

                float signif = MVAtool->Compute_Combine_Significance_From_TemplateFile( ("./outputs/optim_BDTvar/RemoveWorstVars/BDTttZ/without_"+Convert_Number_To_TString(ivar+1)+"WorstVar/Combine_Input.root"), signal, "", expected, use_syst);

                file_out<<"without "+Convert_Number_To_TString(ivar+1)+" worst vars ---> "<<signif<<endl;
            }
        }

        delete MVAtool;


    } //end optimization loop




  return 0;
}
