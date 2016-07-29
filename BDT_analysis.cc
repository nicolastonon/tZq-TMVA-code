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
    set_v_cut_name.push_back("METpt");                  set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    set_v_cut_name.push_back("mTW");                    set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    set_v_cut_name.push_back("NJets");                  set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(true);
    set_v_cut_name.push_back("NBJets");                 set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(true);
    set_v_cut_name.push_back("AddLepPT");               set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    //WARNING : CAN ONLY USE "< X" FOR ISO !! (because if 3rd lepton has opposite flavour --> iso=-1 --> event wouldn't pass cut)
    set_v_cut_name.push_back("AdditionalMuonIso");      set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
    set_v_cut_name.push_back("AdditionalEleIso");       set_v_cut_def.push_back("");            set_v_cut_IsUsedForBDT.push_back(false);
//-------------------

    //Used to re-scale every weights in the code by a lumi factor. (NB : default value is 2015 / 7.6.x lumi = 2.26 !)
    double set_luminosity = 2.26; //in fb-1

    //Use MC fakes or data-driven fakes)
    bool fakes_from_data = true;

    //Templates
    int nofbin_templates = 5; //Binning to be used for *template* production
    bool fakes_summed_channels = true; //Sum uuu/eeu & eee/uue --> Double the fake stat.! Only possible for Templates (not for CR plots)
    bool real_data_templates = false; //If true, use real data sample to create *templates* (BDT, mTW, ...) / else, use pseudodata !

    //If true, activates only the "optimization" part (@ end of file)
    bool do_optimization_scan = false;

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
//NB : treat leaves/variables "Weight" and "Channel" separately

    thevarlist.push_back("m3l");
    thevarlist.push_back("ZCandMass");
    thevarlist.push_back("btagDiscri"); //NB : for now, if NBJets == 0 --> btagDiscri is constant --> Need to desactivate it !
    thevarlist.push_back("dRAddLepJet");
    thevarlist.push_back("dRAddLepClosestJet");
    thevarlist.push_back("deltaPhilb");
    thevarlist.push_back("Zpt");
    thevarlist.push_back("ZEta");
    thevarlist.push_back("asym");
    thevarlist.push_back("etaQ");
    thevarlist.push_back("AddLepETA");
    thevarlist.push_back("LeadJetPT");
    thevarlist.push_back("LeadJetEta");
    thevarlist.push_back("dPhiZMET");
    thevarlist.push_back("dPhiZAddLep");
    thevarlist.push_back("dRZAddLep");
    thevarlist.push_back("ptQ");
    thevarlist.push_back("dRjj");

    // mtop not properly reconstructed yet
    //thevarlist.push_back("mtop");
    //thevarlist.push_back("dRAddLepBFromTop");
    //thevarlist.push_back("dRZTop");
    //thevarlist.push_back("TopPT");

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
    thesystlist.push_back("JER__plus"); thesystlist.push_back("JER__minus");
    thesystlist.push_back("JES__plus"); thesystlist.push_back("JES__minus");

//Affect the event weight
    thesystlist.push_back("Q2__plus"); thesystlist.push_back("Q2__minus"); //NB : not included in ttZMad --> Use ttZ Madgraph for training, amcatnlo for the rest
    thesystlist.push_back("PU__plus"); thesystlist.push_back("PU__minus");
    thesystlist.push_back("MuEff__plus"); thesystlist.push_back("MuEff__minus");
    thesystlist.push_back("EleEff__plus"); thesystlist.push_back("EleEff__minus");
    //B-tag syst
    /*thesystlist.push_back("btag_lf__plus"); thesystlist.push_back("btag_lf__minus");
    thesystlist.push_back("btag_hf__plus"); thesystlist.push_back("btag_hf__minus");
    thesystlist.push_back("btag_hfstats1__plus"); thesystlist.push_back("btag_hfstats1__minus");
    thesystlist.push_back("btag_lfstats1__plus"); thesystlist.push_back("btag_lfstats1__minus");
    thesystlist.push_back("btag_hfstats2__plus"); thesystlist.push_back("btag_hfstats2__minus");
    thesystlist.push_back("btag_lfstats2__plus"); thesystlist.push_back("btag_lfstats2__minus");
    thesystlist.push_back("btag_cferr1__plus"); thesystlist.push_back("btag_cferr1__minus");
    thesystlist.push_back("btag_cferr2__plus"); thesystlist.push_back("btag_cferr2__minus");*/

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
        theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, nofbin_templates); if(MVAtool->stop_program) {return 1;}
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

        //Different values for each region
        vector<TString> v_NJets_cut_values;
        vector<TString> v_NBJets_cut_values;

        //For scan
        vector<TString> v_METpt_cut_values;
        /*v_METpt_cut_values.push_back(">0");
        v_METpt_cut_values.push_back(">10");
        v_METpt_cut_values.push_back(">20");
        v_METpt_cut_values.push_back(">30");
        v_METpt_cut_values.push_back(">40");
        v_METpt_cut_values.push_back(">50");*/

        vector<TString> v_mTW_cut_values;
        /*v_mTW_cut_values.push_back(">0");
        v_mTW_cut_values.push_back(">10");
        v_mTW_cut_values.push_back(">20");
        v_mTW_cut_values.push_back(">30");
        v_mTW_cut_values.push_back(">40");
        v_mTW_cut_values.push_back(">50");*/

        vector<TString> v_isoMu_cut_values;
        v_isoMu_cut_values.push_back("");


        vector<TString> v_isoEl_cut_values;
        v_isoEl_cut_values.push_back("");


        //#############################################
        //  LOOP ON THE 2 CUT VECTORS YOU WANT TO SCAN
        //#############################################

        //SCAN LOOP
        for(int j = 0; j < v_isoMu_cut_values.size(); j++) //First scanned variable
        {
            for(int k=0; k < v_isoEl_cut_values.size(); k++) //Second scanned variable
            {
                //BE CAREFUL HERE !!! UNCOMMENT THE RELEVANT LINES ONLY, AND MAKE SURE THE (j,k) LOOP ITERATORS ARE ASSOCIATED TO THE CORRECT VECTORS !!!
                //------------------------------

                //#############################################
                //               WZ CR Templates
                //#############################################
                //Use scope delimiters so that I can re-use the exact same vectors names for each region (different scopes)
                {
                    std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
                    scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">0");     scan_v_cut_IsUsedForBDT.push_back(true);
                    scan_v_cut_name.push_back("NBJets");    scan_v_cut_def.push_back("==0");    scan_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant be used in BDT


                    scan_v_cut_name.push_back("METpt"); scan_v_cut_IsUsedForBDT.push_back(false);
                    //scan_v_cut_def.push_back(v_METpt_cut_values[j]);
                    scan_v_cut_def.push_back("");

                    scan_v_cut_name.push_back("mTW"); scan_v_cut_IsUsedForBDT.push_back(false);
                    //scan_v_cut_def.push_back(v_mTW_cut_values[k]);
                    scan_v_cut_def.push_back("");

                    scan_v_cut_name.push_back("AdditionalMuonIso"); scan_v_cut_IsUsedForBDT.push_back(false);
                    scan_v_cut_def.push_back(v_isoMu_cut_values[j]);

                    scan_v_cut_name.push_back("AdditionalEleIso"); scan_v_cut_IsUsedForBDT.push_back(false);
                    scan_v_cut_def.push_back(v_isoEl_cut_values[k]);

                    theMVAtool* MVAtool_WZ = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, nofbin_templates); if(MVAtool_WZ->stop_program) {return 1;}
                    MVAtool_WZ->Set_Luminosity(set_luminosity);

                    TString template_name_WZ = "mTW"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
                    MVAtool_WZ->Read(template_name_WZ, fakes_from_data, real_data_templates, fakes_summed_channels);
                    if(!real_data_templates) {MVAtool_WZ->Generate_PseudoData_Histograms_For_Templates(template_name_WZ);}

                    for(int i=0; i<thechannellist.size(); i++)
                    {
                        MVAtool_WZ->Plot_Templates(thechannellist[i], template_name_WZ, false); //Plot the BDT distributions of MC & pseudo-data templates
                    }
                    MVAtool_WZ->Plot_Templates("", template_name_WZ, true); //Sum of 4 channels
                    MVAtool_WZ->~theMVAtool();
                }
                //#############################################
                //               ttZ CR Templates
                //#############################################
                {
                    std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
                    scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">1");     scan_v_cut_IsUsedForBDT.push_back(true);
                    scan_v_cut_name.push_back("NBJets");    scan_v_cut_def.push_back(">1");     scan_v_cut_IsUsedForBDT.push_back(true);

                    scan_v_cut_name.push_back("METpt"); scan_v_cut_IsUsedForBDT.push_back(false);
                    //scan_v_cut_def.push_back(v_METpt_cut_values[j]);
                    scan_v_cut_def.push_back("");

                    scan_v_cut_name.push_back("mTW"); scan_v_cut_IsUsedForBDT.push_back(false);
                    //scan_v_cut_def.push_back(v_mTW_cut_values[k]);
                    scan_v_cut_def.push_back("");

                    scan_v_cut_name.push_back("AdditionalMuonIso"); scan_v_cut_IsUsedForBDT.push_back(false);
                    scan_v_cut_def.push_back(v_isoMu_cut_values[j]);

                    scan_v_cut_name.push_back("AdditionalEleIso"); scan_v_cut_IsUsedForBDT.push_back(false);
                    scan_v_cut_def.push_back(v_isoEl_cut_values[k]);

                    theMVAtool* MVAtool_ttZ = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, nofbin_templates); if(MVAtool_ttZ->stop_program) {return 1;}
                    MVAtool_ttZ->Set_Luminosity(set_luminosity);

                    for(int i=0; i<thechannellist.size(); i++)
                    {
                        TString bdt_type_ttZ = "BDTttZ"; //'BDT' or 'BDTttZ'
                        MVAtool_ttZ->Train_Test_Evaluate(thechannellist[i], bdt_type_ttZ);
                    }

                    TString template_name_ttZ = "BDTttZ"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
                    MVAtool_ttZ->Read(template_name_ttZ, fakes_from_data, real_data_templates, fakes_summed_channels);
                    if(!real_data_templates) {MVAtool_ttZ->Generate_PseudoData_Histograms_For_Templates(template_name_ttZ);}

                    for(int i=0; i<thechannellist.size(); i++)
                    {
                        MVAtool_ttZ->Plot_Templates(thechannellist[i], template_name_ttZ, false); //Plot the BDT distributions of MC & pseudo-data templates
                    }
                    MVAtool_ttZ->Plot_Templates("", template_name_ttZ, true); //Sum of 4 channels
                    MVAtool_ttZ->~theMVAtool();
                }
                //#############################################
                //               tZq SR Templates
                //#############################################
                {
                    std::vector<TString > scan_v_cut_name; std::vector<TString > scan_v_cut_def; std::vector<bool > scan_v_cut_IsUsedForBDT;
                    scan_v_cut_name.push_back("NJets");     scan_v_cut_def.push_back(">1");     scan_v_cut_IsUsedForBDT.push_back(true);
                    scan_v_cut_name.push_back("NBJets");    scan_v_cut_def.push_back("==1");    scan_v_cut_IsUsedForBDT.push_back(false); //Constant -> cant be used in BDT

                    scan_v_cut_name.push_back("METpt"); scan_v_cut_IsUsedForBDT.push_back(false);
                    //scan_v_cut_def.push_back(v_METpt_cut_values[j]);
                    scan_v_cut_def.push_back("");

                    scan_v_cut_name.push_back("mTW"); scan_v_cut_IsUsedForBDT.push_back(false);
                    //scan_v_cut_def.push_back(v_mTW_cut_values[k]);
                    scan_v_cut_def.push_back("");

                    scan_v_cut_name.push_back("AdditionalMuonIso"); scan_v_cut_IsUsedForBDT.push_back(false);
                    scan_v_cut_def.push_back(v_isoMu_cut_values[j]);

                    scan_v_cut_name.push_back("AdditionalEleIso"); scan_v_cut_IsUsedForBDT.push_back(false);
                    scan_v_cut_def.push_back(v_isoEl_cut_values[k]);

                    theMVAtool* MVAtool_tZq = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, scan_v_cut_name, scan_v_cut_def, scan_v_cut_IsUsedForBDT, nofbin_templates); if(MVAtool_tZq->stop_program) {return 1;}
                    MVAtool_tZq->Set_Luminosity(set_luminosity);

                    for(int i=0; i<thechannellist.size(); i++)
                    {
                        TString bdt_type_tZq = "BDT"; //'BDT' or 'BDTttZ'
                        MVAtool_tZq->Train_Test_Evaluate(thechannellist[i], bdt_type_tZq);
                    }

                    TString template_name_tZq = "BDT"; //Either 'BDT', 'BDTttZ', 'mTW' or 'm3l'
                    MVAtool_tZq->Read(template_name_tZq, fakes_from_data, real_data_templates, fakes_summed_channels);
                    if(!real_data_templates) {MVAtool_tZq->Generate_PseudoData_Histograms_For_Templates(template_name_tZq);}

                    for(int i=0; i<thechannellist.size(); i++)
                    {
                        MVAtool_tZq->Plot_Templates(thechannellist[i], template_name_tZq, false); //Plot the BDT distributions of MC & pseudo-data templates
                    }
                    MVAtool_tZq->Plot_Templates("", template_name_tZq, true); //Sum of 4 channels
                    MVAtool_tZq->~theMVAtool();
                }

            } //end second scanned variable loop
        } //end first scanned variable loop

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
