#include "theMVAtool.h"

using namespace std;

int main()
{
    std::vector<TString> thesamplelist;
	std::vector<TString > thevarlist;
	std::vector<TString > thesystlist;
	std::vector<TString > thechannellist;
    std::vector<int> v_color;

//-------------------
    //thechannellist.push_back("");
    thechannellist.push_back("uuu");
    thechannellist.push_back("uue");
    thechannellist.push_back("eeu");
    thechannellist.push_back("eee");

//-------------------
    //BKG
    //thesamplelist.push_back("ttW"); v_color.push_back(kRed+1);
    thesamplelist.push_back("WZJets3LNu");    v_color.push_back(11);
    //thesamplelist.push_back("DYMll50");       v_color.push_back(kAzure-2);
    //thesamplelist.push_back("DY10To50");      v_color.push_back(kAzure-2);
    thesamplelist.push_back("ZZ");            v_color.push_back(kYellow);
    //thesamplelist.push_back("TTbar");         v_color.push_back(kRed-1);
    thesamplelist.push_back("ttZ");           v_color.push_back(kRed+1);
    thesamplelist.push_back("WW");            v_color.push_back(kYellow+2);
    //thesamplelist.push_back("ST_tW");         v_color.push_back(kBlack);
    //thesamplelist.push_back("ST_tW_antitop"); v_color.push_back(kBlack);

    //Signal
    thesamplelist.push_back("tZq"); v_color.push_back(kGreen+2);

    //Don't use DY nor tt in BDT
    //thesamplelist.push_back("DYjets"); v_color.push_back(kAzure-2);
    //thesamplelist.push_back("TT"); v_color.push_back(kRed-1);
    //thesamplelist.push_back("WW");

    //thesamplelist.push_back(""); //Add Data here

//-------------------
//NB : treat leaves/variables "Weight" and "Channel" separately
    thevarlist.push_back("mTW");
    thevarlist.push_back("METpt");
    thevarlist.push_back("ZCandMass");
    thevarlist.push_back("deltaPhilb");
    thevarlist.push_back("Zpt");
    thevarlist.push_back("ZEta");
    thevarlist.push_back("asym");
    thevarlist.push_back("mtop");
    thevarlist.push_back("btagDiscri");
    thevarlist.push_back("btagDiscri_subleading");
    thevarlist.push_back("etaQ");

    //-------------------
    thesystlist.push_back("");
    //thesystlist.push_back("JERup");
    //thesystlist.push_back("JERdn");

    TString inputfile = "Ntuples/tZq_treeNew.root";
    //TString inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + "_" + channel + "_" + ".root";
    TFile* file_input = TFile::Open( inputfile );
    if (!file_input)
    {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
    }
    std::cout << "--- Using input file: " << file_input->GetName() << std::endl;

//-------------------
//-------------------
//Function calls

    theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color);

    double cut = -0.199; //Determined via function Determine_Control_Cut

    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Train_Test_Evaluate(file_input, thechannellist[i]);
    }

    //MVAtool->Read(file_input);

    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Determine_Control_Cut(thechannellist[i]);

        //MVAtool->Create_Control_Trees(file_input, thechannellist[i], cut);

        //MVAtool->Create_Control_Histograms(thechannellist[i]);

        //MVAtool->Draw_Control_Plots(thechannellist[i], false); //Not possible yet, crontrol histogram not separated by channel

        //MVAtool->Generate_Pseudo_Data_Histograms(thechannellist[i]); //NB : not possible yet (not enough MC statistic)
    }

    MVAtool->Draw_Control_Plots("", true);
}
