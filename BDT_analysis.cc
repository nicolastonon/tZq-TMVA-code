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
    thechannellist.push_back("");
    //thechannellist.push_back("uuu");
    //thechannellist.push_back("eee");
    //thechannellist.push_back("eeu");
    //thechannellist.push_back("uue");

//-------------------
    thesamplelist.push_back("WZ"); v_color.push_back(11);
    thesamplelist.push_back("DYjets"); v_color.push_back(kAzure-2);
    thesamplelist.push_back("ZZ"); v_color.push_back(kYellow);
    thesamplelist.push_back("TT"); v_color.push_back(kRed-1);
    thesamplelist.push_back("ttW"); v_color.push_back(kRed+1);
    thesamplelist.push_back("ttZ"); v_color.push_back(kRed+1);
    //thesamplelist.push_back("WW");
    thesamplelist.push_back("tZq"); v_color.push_back(kGreen+2);

    //thesamplelist.push_back(""); //Add Data here

//-------------------
    thevarlist.push_back("mTW");
    thevarlist.push_back("METpt");
    thevarlist.push_back("ZCandMass");

    //-------------------
    thesystlist.push_back("");
    thesystlist.push_back("JERup");
    thesystlist.push_back("JERdn");

//-------------------
//-------------------
theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color);

    double cut = 0.08; //Determined via function Determine_Control_Cut

    for(int i=0; i<thechannellist.size(); i++)
    {
        //MVAtool->Train_Test_Evaluate(thechannellist[i]);
        //MVAtool->Read(thechannellist[i]);

        //MVAtool->Determine_Control_Cut(thechannellist[i]);

        //MVAtool->Create_Control_Histograms(thechannellist[i], cut);

        //MVAtool->Draw_Control_Plots(thechannellist[i], false);
    }

    MVAtool->Draw_Control_Plots("", true);
}
