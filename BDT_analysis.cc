#include "theMVAtool.h"

using namespace std;

int main()
{
    //Used to re-scale every weights in the code by a lumi factor. (NB : default value is 2015 / 7.6.x lumi = 2.26 !)
    double set_luminosity = 2.26; //in fb-1

//-----------------------------------------------------------------------------------------
//    _   _         _
//   | | (_)  ___  | |_   ___
//   | | | | / __| | __| / __|
//   | | | | \__ \ | |_  \__ \
//   |_| |_| |___/  \__| |___/
//
//-----------------------------------------------------------------------------------------
    std::vector<TString> thesamplelist;
	std::vector<TString > thevarlist;
	std::vector<TString > thesystlist;
	std::vector<TString > thechannellist;
    std::vector<int> v_color; //sample <-> color

//-------------------
    thechannellist.push_back("uuu");
    thechannellist.push_back("uue");
    thechannellist.push_back("eeu");
    thechannellist.push_back("eee");

//-------------------
//Sample order is important in function Read (so it knows which are the fake samples it must sum) and in Draw_Control_Plots (see explanation in code)
    //DATA --- THE DATA SAMPLE MUST BE UNIQUE AND IN FIRST POSITION
    thesamplelist.push_back("Data");

    //Signal
    thesamplelist.push_back("tZq");             v_color.push_back(kGreen+2);

    //BKG
    thesamplelist.push_back("WZ");              v_color.push_back(11);
    thesamplelist.push_back("ZZ");              v_color.push_back(kYellow);
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed);
    //thesamplelist.push_back("ST_tW");         v_color.push_back(kBlack);
    //thesamplelist.push_back("ST_tW_antitop"); v_color.push_back(kBlack);
    //thesamplelist.push_back("ttW");           v_color.push_back(kRed+1);

    //FAKES
    //thesamplelist.push_back("Fakes");             v_color.push_back(kAzure-2);
    //-- THESE 3 SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    thesamplelist.push_back("DYjets");             v_color.push_back(kAzure-2);
    thesamplelist.push_back("TT");                 v_color.push_back(kRed-1);
    thesamplelist.push_back("WW");                 v_color.push_back(kYellow);


//-------------------
//NB : treat leaves/variables "Weight" and "Channel" separately
//ORDER OF FIRST VARIABLES IS IMPORTANT FOR CUT (cf. Read() - use a vector of variables -> ordered)
//----------------------------
    thevarlist.push_back("METpt");
    thevarlist.push_back("mTW");
    thevarlist.push_back("NJets");
    thevarlist.push_back("NBJets");
//----------------------------
    thevarlist.push_back("ZCandMass"); //useless
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
    thevarlist.push_back("ptQ"); //--not useful
    thevarlist.push_back("dRjj");
    //thevarlist.push_back("mtop"); // mtop not properly reconstructed yet
    //thevarlist.push_back("dRZTop");
    //thevarlist.push_back("TopPT");*/

//-------------------
    thesystlist.push_back("");

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


//-----------------------------------------------------------------------------------------
//                   _                    _                 _     _
//     ___   _   _  | |_     ___    ___  | |   ___    ___  | |_  (_)   ___    _ __
//    / __| | | | | | __|   / __|  / _ \ | |  / _ \  / __| | __| | |  / _ \  | '_ \
//   | (__  | |_| | | |_    \__ \ |  __/ | | |  __/ | (__  | |_  | | | (_) | | | | |
//    \___|  \__,_|  \__|   |___/  \___| |_|  \___|  \___|  \__| |_|  \___/  |_| |_|
//
//-----------------------------------------------------------------------------------------

//Specify here the cuts that you wish to apply (propagated to training, reading, ...)
//Can use the functions to set these cuts to the CR & SR defined values
//Ex : string set_MET_cut = ">30"; To dis-activate cut, just set it to "";
//-----------------------
    string set_MET_cut = ">30";
    string set_mTW_cut = "";
    string set_NJets_cut = "";
    string set_NBJets_cut = "";

//-------------------
//Create instance of the class, and initialize it

    theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color);
    MVAtool->Set_Variable_Cuts(set_MET_cut, set_mTW_cut, set_NJets_cut, set_NBJets_cut); //Set cuts on variables
    MVAtool->Set_Luminosity(set_luminosity);

//-----------------------------------------------------------------------------------------
//     __                          _     _                                    _   _
//    / _|  _   _   _ __     ___  | |_  (_)   ___    _ __       ___    __ _  | | | |  ___
//   | |_  | | | | | '_ \   / __| | __| | |  / _ \  | '_ \     / __|  / _` | | | | | / __|
//   |  _| | |_| | | | | | | (__  | |_  | | | (_) | | | | |   | (__  | (_| | | | | | \__ \
//   |_|    \__,_| |_| |_|  \___|  \__| |_|  \___/  |_| |_|    \___|  \__,_| |_| |_| |___/
//
//-----------------------------------------------------------------------------------------
    //(NB : Train_Test_Evaluate, Read, and Create_Control_Trees all need to be run on the same variable list)

    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Train_Test_Evaluate(thechannellist[i]); //TRAINING
    }

    MVAtool->Read("BDT", 1, "output_Reader"); //READING   //0 -> No fakes || 1 -> Fakes from MC || 2 -> Fakes from data//

    float cut_BDT_CR = MVAtool->Determine_Control_Cut(); //Determine where to cut to create BDT_CR
    bool cut_on_BDT = false; MVAtool->Create_Control_Trees(cut_on_BDT, cut_BDT_CR); //Fills a tree with events passing the cuts

    for(int i=0; i<thechannellist.size(); i++)
    {
        MVAtool->Create_Control_Histograms(thechannellist[i]); //Creates histograms from the control tree [LONG !]
        //MVAtool->Generate_Pseudo_Data_Histograms_CR(thechannellist[i]); //Generate pseudo-data to test control plots in BDT_CT

        //MVAtool->Generate_Pseudo_Data_Histograms(thechannellist[i]); //Generates pseudo-data to to test template fit of BDT in SR --> stay blind
        //MVAtool->Plot_BDT_Templates(thechannellist[i]); //Plot the BDT distributions of MC & pseudo-data, to check that pseudo data makes sense

        MVAtool->Draw_Control_Plots(thechannellist[i], false); //Draw plots for the BDT CR (uses the control histograms)
    }

    //MVAtool->Plot_BDT_Templates_allchannels(); //Sum of 4 channels
    MVAtool->Draw_Control_Plots("", true); //Sum of 4 channels
}
