#include "theMVAtool.h"

using namespace std;

int main()
{
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
    thesamplelist.push_back("ttZ");             v_color.push_back(kRed+1);
    //thesamplelist.push_back("ST_tW");         v_color.push_back(kBlack);
    //thesamplelist.push_back("ST_tW_antitop"); v_color.push_back(kBlack);
    //thesamplelist.push_back("ttW");           v_color.push_back(kRed+1);

    //FAKES -- THESE 3 SAMPLES MUST BE THE LAST OF THE LIST FOR THE READER TO KNOW WHAT ARE THE FAKE SAMPLES !
    thesamplelist.push_back("DYjets");             v_color.push_back(kAzure-2);
    thesamplelist.push_back("TT");                 v_color.push_back(kRed-1);
    thesamplelist.push_back("WW");                 v_color.push_back(kYellow+2);


//-------------------
//NB : treat leaves/variables "Weight" and "Channel" separately
//--- ORDER OF FIRST VARIABLES IS IMPORTANT FOR CUT (cf. Read())
    thevarlist.push_back("METpt");
    thevarlist.push_back("mTW"); //--not useful
    /*thevarlist.push_back("NJets");
    thevarlist.push_back("NBJets");

    thevarlist.push_back("ZCandMass"); //--not useful
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
    thevarlist.push_back("dRjj");*/
    //thevarlist.push_back("mtop"); // mtop not properly reconstructed yet
    //thevarlist.push_back("dRZTop");
    //thevarlist.push_back("TopPT");

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
    thesystlist.push_back("MuEff__plus");
    thesystlist.push_back("MuEff__minus");
    thesystlist.push_back("EleEff__plus");
    thesystlist.push_back("EleEff__minus");

//Specify here the cuts that you wish to apply (propagated to training, reading, ...)
//Can use the functions to set these cuts to the CR & SR defined values
//Ex : string set_MET_cut = ">30"; To dis-activate cut, just set it to "";
//-----------------------
    string set_MET_cut = ">30";
    string set_mTW_cut = "";
    string set_NJets_cut = "";
    string set_NBJets_cut = "";
//-----------------------

//-------------------
//-------------------

    theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color);
    MVAtool->Set_Variable_Cuts(set_MET_cut, set_mTW_cut, set_NJets_cut, set_NBJets_cut); //Set cuts on variables
    float cut_BDT_CR = -0.199; //Cutting value on the BDT discriminant to determine the BDT-CR. Obtained via Determine_Control_Cut()

//---Function calls

    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Train_Test_Evaluate(thechannellist[i]);
    }

    //MVAtool->Read(true); //input boolean decides whether or not fakes are to be used

    //cut_BDT_CR = MVAtool->Determine_Control_Cut(); MVAtool->Create_Control_Trees(cut_BDT_CR);

    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Generate_Pseudo_Data_Histograms(thechannellist[i]);

        MVAtool->Create_Control_Histograms(thechannellist[i]); cout<<"#### CHANNEL "<<thechannellist[i]<<" ####"<<endl; //Very long because runs on all vars, all samples, all syst !

        //MVAtool->Generate_Pseudo_Data_Histograms_CR(thechannellist[i]);

        //MVAtool->Plot_BDT_Templates(thechannellist[i]);

        MVAtool->Draw_Control_Plots(thechannellist[i], false);
    }

    MVAtool->Draw_Control_Plots("", true); //Sum of 4 channels
    //MVAtool->Plot_BDT_Templates_allchannels(); //Sum of 4 channels
}
