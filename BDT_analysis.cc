#include "theMVAtool.h"

using namespace std;

int main()
{
//-----------------------------------------------------------------------------------------
//                 _                       _     _
//    ___    ___  | |_      ___    _ __   | |_  (_)   ___    _ __    ___
//   / __|  / _ \ | __|    / _ \  | '_ \  | __| | |  / _ \  | '_ \  / __|
//   \__ \ |  __/ | |_    | (_) | | |_) | | |_  | | | (_) | | | | | \__ \
//   |___/  \___|  \__|    \___/  | .__/   \__| |_|  \___/  |_| |_| |___/
//                                |_|
//-----------------------------------------------------------------------------------------

    //Specify here the cuts that you wish to apply (propagated to training, reading, ...)
    //Can use the functions to set these cuts to the CR & SR defined values
    //Ex : string set_MET_cut = ">30"; To dis-activate cut, just set it to "";
//-----------------------
    TString set_MET_cut = "";
    TString set_mTW_cut = "";
    TString set_NJets_cut = ""; //ONLY STRICT SIGN (> / < / ==)
    TString set_NBJets_cut = ""; //ONLY STRICT SIGN (> / < / ==)

//-------------------
    //Used to re-scale every weights in the code by a lumi factor. (NB : default value is 2015 / 7.6.x lumi = 2.26 !)
    double set_luminosity = 2.26; //in fb-1

    //Binning to be used for template production
    int nofbin_templates = 5;

    //Use MC fakes or data-driven fakes)
    bool fakes_from_data = true;

    //If true, use real data sample to create *templates* (BDT, mTW, ...) / else, use pseudodata !
    bool real_data_templates = false;

//-----------------------------------------------------------------------------------------
//    _   _         _
//   | | (_)  ___  | |_   ___
//   | | | | / __| | __| / __|
//   | | | | \__ \ | |_  \__ \
//   |_| |_| |___/  \__| |___/
//
//-----------------------------------------------------------------------------------------

//-------------------
    std::vector<TString> thesamplelist;
    std::vector<TString > thevarlist; //Variables used in BDT
    std::vector<TString > thecutvarlist; //Variables not used in BDT - only to cut
	std::vector<TString > thesystlist;
	std::vector<TString > thechannellist;
    std::vector<int> v_color; //sample <-> color
//-------------------

//-------------------
    thechannellist.push_back("uuu");
    thechannellist.push_back("uue");
    thechannellist.push_back("eeu");
    thechannellist.push_back("eee");
//-------------------

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
    //thesamplelist.push_back("ttW");           v_color.push_back(kRed+1);

    //FAKES
    thesamplelist.push_back("Fakes");           v_color.push_back(kAzure-2);
    //-- THESE 3 SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    thesamplelist.push_back("DYjets");          v_color.push_back(kAzure-2);
    thesamplelist.push_back("TT");              v_color.push_back(kRed-1);
    thesamplelist.push_back("WW");              v_color.push_back(kYellow);
//-------------------

//-------------------
    //Can cut on these variables --> Need to treat them separately in code
    //NB : the handling of these 4 variables is hard-coded (contrary to the others). If add another "cut var", need to propagate it properly in the code
    thecutvarlist.push_back("METpt");
    thecutvarlist.push_back("mTW");
    thecutvarlist.push_back("NJets");
    thecutvarlist.push_back("NBJets");
//-------------------
    //thevarlist.push_back("m3l"); //MUST BE IN FIRST POSITION -- NB : not implemented in ttZ yet
    thevarlist.push_back("ZCandMass");
    thevarlist.push_back("deltaPhilb");
    thevarlist.push_back("Zpt");
    thevarlist.push_back("ZEta");
    thevarlist.push_back("asym");
    thevarlist.push_back("btagDiscri");
    thevarlist.push_back("etaQ");
    thevarlist.push_back("AddLepPT");
    thevarlist.push_back("AddLepETA");
    thevarlist.push_back("LeadJetPT");
    thevarlist.push_back("LeadJetEta");
    thevarlist.push_back("dPhiZMET");
    thevarlist.push_back("dPhiZAddLep");
    thevarlist.push_back("dRAddLepBFromTop");
    thevarlist.push_back("dRZAddLep");
    thevarlist.push_back("ptQ");
    thevarlist.push_back("dRjj");
    //thevarlist.push_back("mtop"); // mtop not properly reconstructed yet
    //thevarlist.push_back("dRZTop");
    //thevarlist.push_back("TopPT");
    //NB : treat leaves/variables "Weight" and "Channel" separately
//-------------------

//-------------------
    thesystlist.push_back(""); //Always keep it activated

    //Affect the variable distributions
    thesystlist.push_back("JER__plus");
    thesystlist.push_back("JER__minus");
    thesystlist.push_back("JES__plus");
    thesystlist.push_back("JES__minus");

    //Affect the event weight
    //thesystlist.push_back("Q2__plus"); //cf. Reader : not included properly in ttZ yet !
    //thesystlist.push_back("Q2__minus");
    thesystlist.push_back("PU__plus");
    thesystlist.push_back("PU__minus");
    thesystlist.push_back("MuEff__plus");
    thesystlist.push_back("MuEff__minus");
    thesystlist.push_back("EleEff__plus");
    thesystlist.push_back("EleEff__minus");
//-------------------

//-----------------------------------------------------------------------------------------
//     __                          _     _                                    _   _
//    / _|  _   _   _ __     ___  | |_  (_)   ___    _ __       ___    __ _  | | | |  ___
//   | |_  | | | | | '_ \   / __| | __| | |  / _ \  | '_ \     / __|  / _` | | | | | / __|
//   |  _| | |_| | | | | | | (__  | |_  | | | (_) | | | | |   | (__  | (_| | | | | | \__ \
//   |_|    \__,_| |_| |_|  \___|  \__| |_|  \___/  |_| |_|    \___|  \__,_| |_| |_| |___/
//
//-----------------------------------------------------------------------------------------
    //(NB : Train_Test_Evaluate, Read, and Create_Control_Trees all need to be run on the same variable list)

    //#############################################
    //  CREATE INSTANCE OF CLASS & INITIALIZE
    //#############################################
    theMVAtool* MVAtool = new theMVAtool(thevarlist, thecutvarlist, thesamplelist, thesystlist, thechannellist, v_color, nofbin_templates);
    MVAtool->Set_Variable_Cuts(set_MET_cut, set_mTW_cut, set_NJets_cut, set_NBJets_cut); if(MVAtool->stop_program) {return 1;}
    MVAtool->Set_Luminosity(set_luminosity);

    //#############################################
    // TRAINING
    //#############################################
    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Train_Test_Evaluate(thechannellist[i]);
    }

    //#############################################
    //  READING --- TEMPLATES CREATION
    //#############################################
    TString template_name = "BDT"; //Either 'BDT' or 'mTW' or 'm3l'
    //MVAtool->Read(template_name, fakes_from_data, real_data_templates);

    if(!real_data_templates) {MVAtool->Generate_PseudoData_Histograms_For_Templates(template_name);}

    //#############################################
    //  CONTROL TREES & HISTOGRAMS
    //#############################################
    float cut_BDT_CR = MVAtool->Determine_Control_Cut();
    bool cut_on_BDT = false; bool use_pseudodata_CR_plots = false;

    //MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDT, cut_BDT_CR, use_pseudodata_CR_plots);
    //MVAtool->Create_Control_Histograms(fakes_from_data);
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


//-------------------------------------------
//    _____   _   _   ____
//   | ____| | \ | | |  _ \
//   |  _|   |  \| | | | | |
//   | |___  | |\  | | |_| |
//   |_____| |_| \_| |____/
//
//-------------------------------------------
}
