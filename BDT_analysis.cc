#include "theMVAtool.h"

using namespace std;

int main()
{
//-------------------
    std::vector<TString> thesamplelist;
    std::vector<TString > thevarlist; //Variables used in BDT
	std::vector<TString > thesystlist;
    std::vector<TString > thechannellist;
    std::vector<TString > set_v_cut_name;
    std::vector<TString > set_v_cut_def;
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

//Specify here the cuts that you wish to apply (propagated to training, reading, ...). To dis-activate a cut, just set it to "".
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">1");
//NB : * WZ CR --> >0j & ==0bj // * ttZ CR --> >1j & >1bj // * SR ---> >0j & ==1bj // ...
//-------------------
    set_v_cut_name.push_back("METpt");  set_v_cut_def.push_back("");
    set_v_cut_name.push_back("mTW");    set_v_cut_def.push_back("");
    set_v_cut_name.push_back("NJets");  set_v_cut_def.push_back("");
    set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back("");
//-------------------

    //Used to re-scale every weights in the code by a lumi factor. (NB : default value is 2015 / 7.6.x lumi = 2.26 !)
    double set_luminosity = 2.26; //in fb-1

    //Binning to be used for *template* production
    int nofbin_templates = 5;

    //Use MC fakes or data-driven fakes)
    bool fakes_from_data = true;

    //If true, use real data sample to create *templates* (BDT, mTW, ...) / else, use pseudodata !
    bool real_data_templates = false;

    //If true, creates templates for different regions & sets of cuts --> Optimization studies. Cuts can be tuned below, in the "Optimization" part.
    bool do_optimization_scan = false; //Not properly implemented in the main() yet !

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
    //thesamplelist.push_back("ttW");           v_color.push_back(kRed+1);

    //FAKES
    thesamplelist.push_back("Fakes");           v_color.push_back(kAzure-2); //Data-driven (DD)
    //-- THESE 3 SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    thesamplelist.push_back("DYjets");          v_color.push_back(kAzure-2); //MC
    thesamplelist.push_back("TT");              v_color.push_back(kRed-1); //MC
    thesamplelist.push_back("WW");              v_color.push_back(kYellow); //MC
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
    thevarlist.push_back("m3l");
    thevarlist.push_back("ZCandMass");
    thevarlist.push_back("btagDiscri"); //NB : for now, if NBJets == 0 --> btagDiscri is constant --> Need to desactivate it !
    thevarlist.push_back("deltaPhilb");
    thevarlist.push_back("Zpt");
    thevarlist.push_back("ZEta");
    thevarlist.push_back("asym");
    thevarlist.push_back("etaQ");
    thevarlist.push_back("AddLepPT");
    thevarlist.push_back("AddLepETA");
    thevarlist.push_back("LeadJetPT");
    thevarlist.push_back("LeadJetEta");
    thevarlist.push_back("dPhiZMET");
    thevarlist.push_back("dPhiZAddLep");
    thevarlist.push_back("dRZAddLep");
    thevarlist.push_back("ptQ");
    thevarlist.push_back("dRjj");
    //thevarlist.push_back("mtop"); // mtop not properly reconstructed yet
    //thevarlist.push_back("dRAddLepBFromTop");
    //thevarlist.push_back("dRZTop");
    //thevarlist.push_back("TopPT");
    //NB : treat leaves/variables "Weight" and "Channel" separately
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
    thesystlist.push_back(""); //Always keep it activated

    //Affect the variable distributions
    /*thesystlist.push_back("JER__plus");
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
    thesystlist.push_back("EleEff__minus");*/
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

    //#############################################
    //  CREATE INSTANCE OF CLASS & INITIALIZE
    //#############################################
    theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, nofbin_templates); if(MVAtool->stop_program) {return 1;}
    MVAtool->Set_Luminosity(set_luminosity);

    //#############################################
    // TRAINING
    //#############################################
    for(int i=0; i<thechannellist.size(); i++)
    {
        TString bdt_type = "BDT"; //'BDT' or 'BDTttZ' depending on the region (for theta disambiguation)
        //MVAtool->Train_Test_Evaluate(thechannellist[i], bdt_type);
    }

    //#############################################
    //  READING --- TEMPLATES CREATION
    //#############################################
    TString template_name = "mTW"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
    bool fakes_summed_channels = false; //Sum uuu/eeu & eee/uue --> Double the fake stat. !
    //MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels);

    if(!real_data_templates) {MVAtool->Generate_PseudoData_Histograms_For_Templates(template_name);}

    //#############################################
    //  CONTROL TREES & HISTOGRAMS
    //#############################################
    float cut_BDT_CR = MVAtool->Determine_Control_Cut();
    bool cut_on_BDT = false; bool use_pseudodata_CR_plots = false;

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


//-------------------------------------------
//     ___            _     _               _                 _     _
//    / _ \   _ __   | |_  (_)  _ __ ___   (_)  ____   __ _  | |_  (_)   ___    _ __
//   | | | | | '_ \  | __| | | | '_ ` _ \  | | |_  /  / _` | | __| | |  / _ \  | '_ \
//   | |_| | | |_) | | |_  | | | | | | | | | |  / /  | (_| | | |_  | | | (_) | | | | |
//    \___/  | .__/   \__| |_| |_| |_| |_| |_| /___|  \__,_|  \__| |_|  \___/  |_| |_|
//           |_|
//-------------------------------------------

//NB : not properly implemented yet !

/*
    //Use different cut vectors
    scan_v_cut_name.push_back("METpt");  scan_v_cut_def.push_back("");
    scan_v_cut_name.push_back("mTW");    scan_v_cut_def.push_back("");
    scan_v_cut_name.push_back("NJets");  scan_v_cut_def.push_back("");
    scan_v_cut_name.push_back("NBJets"); scan_v_cut_def.push_back("");

    int i_NJets = -1; int i_NBJets = -1;
    //Find the index of each cut variable needed for the scan
    for(int i=0; i<v_cut_name.size(); i++)
    {
        if(v_cut_name[i] == "NJets") {i_NJets = i; break;}
        else if(v_cut_name[i] == "NJets") {i_NBJets = i; break;}
    }
*/
    if(do_optimization_scan)
    {

        for(int iscan = 0; iscan < 1; iscan++)
        {
            //#############################################
            //               WZ CR Templates
            //#############################################
            theMVAtool* MVAtool_WZ = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, nofbin_templates); if(MVAtool_WZ->stop_program) {return 1;}
            MVAtool_WZ->Set_Luminosity(set_luminosity);

            TString template_name_WZ = "mTW"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
            bool fakes_summed_channels_WZ = false; //Sum uuu/eeu & eee/uue --> Double the fake stat. !
            MVAtool_WZ->Read(template_name_WZ, fakes_from_data, real_data_templates, fakes_summed_channels_WZ);
            if(!real_data_templates) {MVAtool_WZ->Generate_PseudoData_Histograms_For_Templates(template_name_WZ);}

            for(int i=0; i<thechannellist.size(); i++)
            {
                MVAtool_WZ->Plot_Templates(thechannellist[i], template_name_WZ, false); //Plot the BDT distributions of MC & pseudo-data templates
            }
            MVAtool_WZ->Plot_Templates("", template_name_WZ, true); //Sum of 4 channels

            MVAtool_WZ->~theMVAtool();

            //#############################################
            //               ttZ CR Templates
            //#############################################
            theMVAtool* MVAtool_ttZ = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, nofbin_templates); if(MVAtool_ttZ->stop_program) {return 1;}
            MVAtool_ttZ->Set_Luminosity(set_luminosity);

            for(int i=0; i<thechannellist.size(); i++)
            {
                TString bdt_type_ttZ = "BDTttZ"; //'BDT' or 'BDTttZ'
                MVAtool_ttZ->Train_Test_Evaluate(thechannellist[i], bdt_type_ttZ);
            }

            TString template_name_ttZ = "BDTttZ"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
            bool fakes_summed_channels_ttZ = false; //Sum uuu/eeu & eee/uue --> Double the fake stat. !
            MVAtool_ttZ->Read(template_name_ttZ, fakes_from_data, real_data_templates, fakes_summed_channels_ttZ);
            if(!real_data_templates) {MVAtool_ttZ->Generate_PseudoData_Histograms_For_Templates(template_name_ttZ);}

            for(int i=0; i<thechannellist.size(); i++)
            {
                MVAtool_ttZ->Plot_Templates(thechannellist[i], template_name_ttZ, false); //Plot the BDT distributions of MC & pseudo-data templates
            }
            MVAtool_ttZ->Plot_Templates("", template_name_ttZ, true); //Sum of 4 channels

            MVAtool_ttZ->~theMVAtool();

            //#############################################
            //               tZq SR Templates
            //#############################################
            theMVAtool* MVAtool_tZq = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, nofbin_templates); if(MVAtool_tZq->stop_program) {return 1;}
            MVAtool_tZq->Set_Luminosity(set_luminosity);

            for(int i=0; i<thechannellist.size(); i++)
            {
                TString bdt_type_tZq = "BDT"; //'BDT' or 'BDTttZ'
                MVAtool_tZq->Train_Test_Evaluate(thechannellist[i], bdt_type_tZq);
            }

            TString template_name_tZq = "BDT"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
            bool fakes_summed_channels_tZq = false; //Sum uuu/eeu & eee/uue --> Double the fake stat. !
            MVAtool_tZq->Read(template_name_tZq, fakes_from_data, real_data_templates, fakes_summed_channels_tZq);
            if(!real_data_templates) {MVAtool_tZq->Generate_PseudoData_Histograms_For_Templates(template_name_tZq);}

            for(int i=0; i<thechannellist.size(); i++)
            {
                MVAtool_tZq->Plot_Templates(thechannellist[i], template_name_tZq, false); //Plot the BDT distributions of MC & pseudo-data templates
            }
            MVAtool_tZq->Plot_Templates("", template_name_tZq, true); //Sum of 4 channels

            MVAtool_tZq->~theMVAtool();
        }
    }


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
