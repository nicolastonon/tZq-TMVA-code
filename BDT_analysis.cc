#include "theMVAtool.h"

using namespace std;

int main()
{
    //Used to re-scale every weights in the code by a lumi factor. (NB : default value is 2015 / 7.6.x lumi = 2.26 !)
    double set_luminosity = 12.9; //in fb-1
    //double set_luminosity = 100; //in fb-1

    //Use MC fakes or data-driven fakes)
    bool fakes_from_data = true;

    //Templates
    int nofbin_templates = 10; //Binning to be used for *template* production
    bool fakes_summed_channels = false; //Sum uuu/eeu & eee/uue --> Double the fake stat.! Only possible for Templates (not for CR plots)
    bool real_data_templates = true; //If true, use real data sample to create *templates* (BDT, mTW, ...) / else, use pseudodata !

    //If true, activates only the "optimization" part (@ end of file)
    bool do_optimization_scan = false;

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
//                 _                       _     _
//    ___    ___  | |_      ___    _ __   | |_  (_)   ___    _ __    ___
//   / __|  / _ \ | __|    / _ \  | '_ \  | __| | |  / _ \  | '_ \  / __|
//   \__ \ |  __/ | |_    | (_) | | |_) | | |_  | | | (_) | | | | | \__ \
//   |___/  \___|  \__|    \___/  | .__/   \__| |_|  \___/  |_| |_| |___/
//                                |_|
//-----------------------------------------------------------------------------------------

//Specify here the cuts that you wish to apply (propagated to training, reading, ...). To dis-activate a cut, just set it to "". Use "==" for equality. Don't use "||".
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4");
//NB : * WZ CR --> >0j & ==0bj // * ttZ CR --> >1j & >1bj // * SR ---> >0j & ==1bj //

//-------------------
    //set_v_cut_name.push_back("mTW");                    set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false); //code fix - don't need it anymore
    //set_v_cut_name.push_back("METpt");                  set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);

    //set_v_cut_name.push_back("NJets");                  set_v_cut_def.push_back(">1");           set_v_cut_IsUsedForBDT.push_back(true);
    //set_v_cut_name.push_back("NJets");                  set_v_cut_def.push_back(">1 && <4");     set_v_cut_IsUsedForBDT.push_back(true);
    //set_v_cut_name.push_back("NJets");                  set_v_cut_def.push_back(">1 ");          set_v_cut_IsUsedForBDT.push_back(true);

    //set_v_cut_name.push_back("NBJets");                 set_v_cut_def.push_back("==0");         set_v_cut_IsUsedForBDT.push_back(false);
    //set_v_cut_name.push_back("NBJets");                 set_v_cut_def.push_back("==1");         set_v_cut_IsUsedForBDT.push_back(true);
    //set_v_cut_name.push_back("NBJets");                 set_v_cut_def.push_back(">1");          set_v_cut_IsUsedForBDT.push_back(true);

    //set_v_cut_name.push_back("AddLepPT");               set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(true);
    //WARNING : CAN ONLY USE "< X" FOR ISO !! (because if 3rd lepton has opposite flavour --> iso=-1 --> event wouldn't pass cut)
    //set_v_cut_name.push_back("AdditionalMuonIso");      set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    //set_v_cut_name.push_back("AdditionalEleIso");       set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
//-------------------

    //bool isttZ = true;
    bool isttZ = false;
    //bool isWZ = true;
    bool isWZ = false;

    if(!isWZ && !isttZ) //Default selection is Signal Region : 1<NJets<4 && NBJets == 1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1 && <4");     set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back("==1");          set_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant
    }
    if(isWZ) //WZ CR Region : NJets > 0 && NBJets == 0
    {
        //set_v_cut_name.push_back("NJets");    set_v_cut_def.push_back(">1");      set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NJets");      set_v_cut_def.push_back(">0");      set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");     set_v_cut_def.push_back(" == 0");   set_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant
    }
    if(isttZ) //ttZ CR Region : NJets>1 && NBJets>1
    {
        set_v_cut_name.push_back("NJets");     set_v_cut_def.push_back(">1");     set_v_cut_IsUsedForBDT.push_back(true);
        set_v_cut_name.push_back("NBJets");    set_v_cut_def.push_back(">1");     set_v_cut_IsUsedForBDT.push_back(true);
    }


//-----------------------------------------------------------------------------------------
//           _                                      _
//     ___  | |__     __ _   _ __    _ __     ___  | |  ___
//    / __| | '_ \   / _` | | '_ \  | '_ \   / _ \ | | / __|
//   | (__  | | | | | (_| | | | | | | | | | |  __/ | | \__ \
//    \___| |_| |_|  \__,_| |_| |_| |_| |_|  \___| |_| |___/
//
//-----------------------------------------------------------------------------------------

//-------------------
    thechannellist.push_back("uuu");
    thechannellist.push_back("uue");
    thechannellist.push_back("eeu");
    thechannellist.push_back("eee");
//-------------------

//-----------------------------------------------------------------------------------------
//                                      _
//    ___    __ _   _ __ ___    _ __   | |   ___   ___
//   / __|  / _` | | '_ ` _ \  | '_ \  | |  / _ \ / __|
//   \__ \ | (_| | | | | | | | | |_) | | | |  __/ \__ \
//   |___/  \__,_| |_| |_| |_| | .__/  |_|  \___| |___/
//                             |_|
//-----------------------------------------------------------------------------------------

//-------------------
//Sample order is important in function Read (so it knows which are the fake samples it must sum) and in Draw_Control_Plots (see explanation in code)
    //DATA --- THE DATA SAMPLE MUST BE UNIQUE AND IN FIRST POSITION
    thesamplelist.push_back("Data");

    //Signal
    thesamplelist.push_back("tZq");             v_color.push_back(kGreen+2);

    //BKG
    thesamplelist.push_back("WZjets");          v_color.push_back(11);
    thesamplelist.push_back("ZZ");              v_color.push_back(kYellow);
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed);
    //thesamplelist.push_back("ST_tW");         v_color.push_back(kBlack);
    //thesamplelist.push_back("ST_tW_antitop"); v_color.push_back(kBlack);
    thesamplelist.push_back("ttW");           v_color.push_back(kRed+1);

    //FAKES
    thesamplelist.push_back("DataFakes");           v_color.push_back(kAzure-2); //Data-driven (DD)
    //-- THESE 3 SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    thesamplelist.push_back("DYjets");          v_color.push_back(kAzure-2); //MC
    thesamplelist.push_back("TT");              v_color.push_back(kRed-1); //MC
    thesamplelist.push_back("WW");              v_color.push_back(kYellow); //MC

    /*
    thesamplelist.push_back("DYjetsFakes");          v_color.push_back(kAzure-2); //MC
    thesamplelist.push_back("TTFakes");              v_color.push_back(kRed-1); //MC
    thesamplelist.push_back("WWFakes");              v_color.push_back(kYellow); //MC
    */

//-------------------

//-----------------------------------------------------------------------------------------
//    ____    ____    _____                            _           _       _
//   | __ )  |  _ \  |_   _|   __   __   __ _   _ __  (_)   __ _  | |__   | |   ___   ___
//   |  _ \  | | | |   | |     \ \ / /  / _` | | '__| | |  / _` | | '_ \  | |  / _ \ / __|
//   | |_) | | |_| |   | |      \ V /  | (_| | | |    | | | (_| | | |_) | | | |  __/ \__ \
//   |____/  |____/    |_|       \_/    \__,_| |_|    |_|  \__,_| |_.__/  |_|  \___| |___/
//
//-----------------------------------------------------------------------------------------

//-------------------
//NB : treat leaves/variables "Weight" and "Channel" separately

    //thevarlist.push_back("mTW");
    //thevarlist.push_back("m3l");
    //thevarlist.push_back("ZCandMass");
    thevarlist.push_back("btagDiscri");
    //thevarlist.push_back("dRAddLepQ");
    thevarlist.push_back("dRAddLepClosestJet");
    //thevarlist.push_back("dPhiAddLepB");
    thevarlist.push_back("ZEta");

    thevarlist.push_back("Zpt");
    thevarlist.push_back("mtop");

    //thevarlist.push_back("AddLepAsym");
    thevarlist.push_back("etaQ");
    thevarlist.push_back("AddLepETA");

    thevarlist.push_back("LeadJetEta");
    thevarlist.push_back("dPhiZAddLep");
    thevarlist.push_back("dRZAddLep"); // --> little discrim
    thevarlist.push_back("dRjj");
    thevarlist.push_back("ptQ"); // --> little discrim
    thevarlist.push_back("tZq_pT");

    //thevarlist.push_back("leadingLeptonPT");
    //thevarlist.push_back("MAddLepB");
    //thevarlist.push_back("dPhiAddLepQ");
    //thevarlist.push_back("dPhiZMET");// low discri power
    //thevarlist.push_back("dRZTop");

    /// --- removing topPT
    //thevarlist.push_back("TopPT");
    //thevarlist.push_back("tZq_mass");
    //thevarlist.push_back("tZq_eta");
    // thevarlist.push_back("tq_mass");
    //thevarlist.push_back("tq_pT"); // strong correlation with ZpT
    //thevarlist.push_back("tq_eta");

//------------------------ FOR TTZ (different vector)
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
    // thevarlist_ttZ.push_back("LeadJetEta");
    thevarlist_ttZ.push_back("dPhiZAddLep");
    thevarlist_ttZ.push_back("dRZAddLep");
    thevarlist_ttZ.push_back("dRjj");
    thevarlist_ttZ.push_back("mtop");
    thevarlist_ttZ.push_back("dRZTop");
    thevarlist_ttZ.push_back("TopPT"); // low discri power
    //thevarlist_ttZ.push_back("tZq_mass");
    thevarlist_ttZ.push_back("tZq_pT");
    thevarlist_ttZ.push_back("m3l");

    // thevarlist_ttZ.push_back("dPhiAddLepB");
    // thevarlist_ttZ.push_back("dPhiAddLepQ");
    // thevarlist_ttZ.push_back("leadingLeptonPT");
    // thevarlist_ttZ.push_back("dPhiZMET");

    //thevarlist_ttZ.push_back("dRAddLepBFromTop");
    //thevarlist_ttZ.push_back("tZq_eta");
    //thevarlist_ttZ.push_back("tq_mass");
    //thevarlist_ttZ.push_back("tq_pT"); // strong correlation with ZpT
    // thevarlist_ttZ.push_back("tq_eta");
//-------------------

  //-----------------------------------------------------------------------------------------
  //                        _                                _     _
  //    ___   _   _   ___  | |_    ___   _ __ ___     __ _  | |_  (_)   ___   ___
  //   / __| | | | | / __| | __|  / _ \ | '_ ` _ \   / _` | | __| | |  / __| / __|
  //   \__ \ | |_| | \__ \ | |_  |  __/ | | | | | | | (_| | | |_  | | | (__  \__ \
  //   |___/  \__, | |___/  \__|  \___| |_| |_| |_|  \__,_|  \__| |_|  \___| |___/
  //          |___/
  //-----------------------------------------------------------------------------------------

//-------------------
    thesystlist.push_back(""); //Nominal -- keep this line

    //Affect the variable distributions
    thesystlist.push_back("JER__plus"); thesystlist.push_back("JER__minus");
    thesystlist.push_back("JES__plus"); thesystlist.push_back("JES__minus");
    thesystlist.push_back("Fakes__plus"); thesystlist.push_back("Fakes__minus");
    //Affect the event weight
    thesystlist.push_back("Q2__plus"); thesystlist.push_back("Q2__minus"); //NB : not included in ttZMad --> Use ttZ Madgraph for training, amcatnlo for the rest
/*    thesystlist.push_back("PU__plus"); thesystlist.push_back("PU__minus"); //FIXME -- disactivated for faster execution
    thesystlist.push_back("MuEff__plus"); thesystlist.push_back("MuEff__minus");
    thesystlist.push_back("EleEff__plus"); thesystlist.push_back("EleEff__minus");
    thesystlist.push_back("pdf__plus"); thesystlist.push_back("pdf__minus");
    thesystlist.push_back("LFcont__plus"); thesystlist.push_back("LFcont__minus");
    thesystlist.push_back("HFstats1__plus"); thesystlist.push_back("HFstats1__minus");
    thesystlist.push_back("HFstats2__plus"); thesystlist.push_back("HFstats2__minus");
    thesystlist.push_back("CFerr1__plus"); thesystlist.push_back("CFerr1__minus");
    thesystlist.push_back("CFerr2__plus"); thesystlist.push_back("CFerr2__minus");
    thesystlist.push_back("HFcont__plus"); thesystlist.push_back("HFcont__minus");
    thesystlist.push_back("LFstats1__plus"); thesystlist.push_back("LFstats1__minus");
    thesystlist.push_back("LFstats2__plus"); thesystlist.push_back("LFstats2__minus");*/
//-------------------


//-----------------------------------------------------------------------------------------
//     __                          _     _                                    _   _
//    / _|  _   _   _ __     ___  | |_  (_)   ___    _ __       ___    __ _  | | | |  ___
//   | |_  | | | | | '_ \   / __| | __| | |  / _ \  | '_ \     / __|  / _` | | | | | / __|
//   |  _| | |_| | | | | | | (__  | |_  | | | (_) | | | | |   | (__  | (_| | | | | | \__ \
//   |_|    \__,_| |_| |_|  \___|  \__| |_|  \___/  |_| |_|    \___|  \__,_| |_| |_| |___/
//
//-----------------------------------------------------------------------------------------
    //(NB : Train_Test_Evaluate, Read, and Create_Control_Trees all need to be run on the full variable list)

    if(!do_optimization_scan)
    {
        //#############################################
        //  CREATE INSTANCE OF CLASS & INITIALIZE
        //#############################################
        std::vector<TString > thevarlist_tmp;
        if(isttZ)  thevarlist_tmp = thevarlist_ttZ;
        else       thevarlist_tmp = thevarlist;
        theMVAtool* MVAtool = new theMVAtool(thevarlist_tmp, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, nofbin_templates, isttZ); if(MVAtool->stop_program) {return 1;}
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
                //MVAtool->Train_Test_Evaluate(thechannellist[i], bdt_type);
            }
        }

        //#############################################
        //  READING --- TEMPLATES CREATION
        //#############################################
        TString template_name = "BDT"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
        if(isWZ)   template_name = "mTW";
        if(isttZ)  template_name = "BDTttZ";

        //MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels);
        if(!real_data_templates) {MVAtool->Generate_PseudoData_Histograms_For_Templates(template_name);}

        TString function = "gaus"; //'gaus' or 'landau'
        //MVAtool->Fit_Fake_Templates(function, "BDT");
        //MVAtool->Create_Fake_Templates_From_Fit(function, "BDT");

        //#############################################
        //  CONTROL TREES & HISTOGRAMS
        //#############################################
        float cut_BDT_CR = -99;
        bool cut_on_BDT = false; if(cut_on_BDT) {cut_BDT_CR = MVAtool->Determine_Control_Cut();}
        if(isWZ || isttZ )  cut_on_BDT = false;
        bool use_pseudodata_CR_plots = false;

        //if(cut_on_BDT) {template_name = "mTW"; MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels, true, cut_BDT_CR);} //Create mTW template w/ cut on BDT

        //MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDT, cut_BDT_CR, use_pseudodata_CR_plots);
        //MVAtool->Create_Control_Histograms(fakes_from_data, use_pseudodata_CR_plots); //NB : very long ! You should only activate necessary syst./var. !
        if(use_pseudodata_CR_plots) {MVAtool->Generate_PseudoData_Histograms_For_Control_Plots(fakes_from_data);}

        //#############################################
        //  DRAW PLOTS
        //#############################################
        for(int i=0; i<thechannellist.size(); i++)
        {
            //MVAtool->Draw_Control_Plots(thechannellist[i], fakes_from_data, false); //Draw plots for the BDT CR
            //MVAtool->Plot_Templates(thechannellist[i], template_name, false); //Plot the BDT distributions of MC & pseudo-data templates
        }

        //MVAtool->Draw_Control_Plots("", fakes_from_data, true); //Sum of 4 channels
        //MVAtool->Plot_Templates("", template_name, true); //Sum of 4 channels
    }

//---------------------------------------------------------------
//     ___            _     _               _                 _     _
//    / _ \   _ __   | |_  (_)  _ __ ___   (_)  ____   __ _  | |_  (_)   ___    _ __
//   | | | | | '_ \  | __| | | | '_ ` _ \  | | |_  /  / _` | | __| | |  / _ \  | '_ \
//   | |_| | | |_) | | |_  | | | | | | | | | |  / /  | (_| | | |_  | | | (_) | | | | |
//    \___/  | .__/   \__| |_| |_| |_| |_| |_| /___|  \__,_|  \__| |_|  \___/  |_| |_|
//           |_|
//---------------------------------------------------------------

    if(do_optimization_scan)
    {
        //#############################################
        //  SET THE CUT DEFINITIONS ON WHICH YOU WANT TO LOOP
        //#############################################

//For scan -- NB : different values for each region
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
        //NB : there should only be 2 scanned variables (2 loops) -- all the other cuts should be specified explicitely for each region
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

    			theMVAtool* MVAtool_WZ = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, nofbin_templates, false); if(MVAtool_WZ->stop_program) {return 1;}
    			MVAtool_WZ->Set_Luminosity(set_luminosity);

    			TString template_name_WZ = "mTW"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    			MVAtool_WZ->Read(template_name_WZ, fakes_from_data, real_data_templates, fakes_summed_channels);
    			if(!real_data_templates) {MVAtool_WZ->Generate_PseudoData_Histograms_For_Templates(template_name_WZ);}

    			/** mmm
    			   for(int i=0; i<thechannellist.size(); i++)
    			   {
    			   //MVAtool_WZ->Plot_Templates(thechannellist[i], template_name_WZ, false); //Plot the BDT distributions of MC & pseudo-data templates
    			   }
    			//MVAtool_WZ->Plot_Templates("", template_name_WZ, true); //Sum of 4 channels
    			**/

    			MVAtool_WZ->~theMVAtool();
		    }

            //#############################################
            //               ttZ CR Templates
            //#############################################
            {
    			std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
    			scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">1");  //FIXME >=4 scan_v_cut_IsUsedForBDT.push_back(true);
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

    			theMVAtool* MVAtool_ttZ = new theMVAtool(thevarlist_ttZ, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, nofbin_templates, true); if(MVAtool_ttZ->stop_program) {return 1;}
    			MVAtool_ttZ->Set_Luminosity(set_luminosity);

                TString bdt_type_ttZ = "BDTttZ"; //'BDT' or 'BDTttZ'
    			for(int i=0; i<thechannellist.size(); i++)
    			  {
    			    MVAtool_ttZ->Train_Test_Evaluate(thechannellist[i], bdt_type_ttZ);
    			  }

    			TString template_name_ttZ = "BDTttZ"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    			MVAtool_ttZ->Read(template_name_ttZ, fakes_from_data, real_data_templates, fakes_summed_channels);
    			if(!real_data_templates) {MVAtool_ttZ->Generate_PseudoData_Histograms_For_Templates(template_name_ttZ);}

    			/**
    			   for(int i=0; i<thechannellist.size(); i++)
    			   {
    			   //MVAtool_ttZ->Plot_Templates(thechannellist[i], template_name_ttZ, false); //Plot the BDT distributions of MC & pseudo-data templates
    			   }
    			**/
    			//MVAtool_ttZ->Plot_Templates("", template_name_ttZ, true); //Sum of 4 channels
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

    			theMVAtool* MVAtool_tZq = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, nofbin_templates, false); if(MVAtool_tZq->stop_program) {return 1;}
    			MVAtool_tZq->Set_Luminosity(set_luminosity);

                TString bdt_type_tZq = "BDT"; //'BDT' or 'BDTttZ'
    			for(int i=0; i<thechannellist.size(); i++)
    			  {
    			    MVAtool_tZq->Train_Test_Evaluate(thechannellist[i], bdt_type_tZq);
    			  }

    			TString template_name_tZq = "BDT"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    			MVAtool_tZq->Read(template_name_tZq, fakes_from_data, real_data_templates, fakes_summed_channels);
    			if(!real_data_templates) {MVAtool_tZq->Generate_PseudoData_Histograms_For_Templates(template_name_tZq);}

    			/**  mmm
    			   for(int i=0; i<thechannellist.size(); i++)
    			   {
    			   //MVAtool_tZq->Plot_Templates(thechannellist[i], template_name_tZq, false); //Plot the BDT distributions of MC & pseudo-data templates
    			   }
    			//MVAtool_tZq->Plot_Templates("", template_name_tZq, true); //Sum of 4 channels
    			**/

    			MVAtool_tZq->~theMVAtool();
            }

        } //end second scanned variable loop
        } //end first scanned variable loop
        }
        }
    } //End optimization Scan


  //-------------------------------------------
  //    _____   _   _   ____
  //   | ____| | \ | | |  _ \
  //   |  _|   |  \| | | | | |
  //   | |___  | |\  | | |_| |
  //   |_____| |_| \_| |____/
  //
  //-------------------------------------------

  return 0;
}
