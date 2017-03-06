//g++ Create_Script_Datacard_Generation.cc -o Create_Script_Datacard_Generation.exe `root-config --cflags --glibs`

#include <iostream>
#include <fstream>
#include <vector>
#include "TString.h"

using namespace std;

void Script_Datacards_InputVars()
{
	TString file_histos = "";
	TString region_choice = "";

	while(region_choice != "WZ" && region_choice != "ttZ" && region_choice != "tZq")
	{
		cout<<"In what region do you wish to plot postfit variables ? (tZq/ttZ/WZ)"<<endl;
		cin>>region_choice;
	}

	if(region_choice=="WZ") file_histos = "../templates/Control_Histograms_NJetsMin0_NBJetsEq0_ScaledFakes.root";
	else if(region_choice=="tZq") file_histos = "../templates/Control_Histograms_NJetsMin1Max4_NBJetsEq1_ScaledFakes.root";
	else if(region_choice=="ttZ") file_histos = "../templates/Control_Histograms_NJetsMin1_NBJetsMin1_ScaledFakes.root";

	cout<<"---> Filepath in datacard : "<<file_histos<<endl<<endl;

	ofstream file_out("Generate_Datacards_InputVars.sh");

	vector<TString> var_list;
	var_list.push_back("NJets");
	var_list.push_back("NBJets");
	var_list.push_back("mTW");

	var_list.push_back("btagDiscri");
	var_list.push_back("dRAddLepQ");
	var_list.push_back("dRAddLepClosestJet");
	var_list.push_back("ZEta");
	var_list.push_back("Zpt");
	var_list.push_back("mtop");
	var_list.push_back("AddLepAsym");
	var_list.push_back("etaQ");
	var_list.push_back("AddLepETA");
	var_list.push_back("dPhiZAddLep");
	var_list.push_back("dRZAddLep"); // --> little discrim --> to be included
	var_list.push_back("dRjj");
	var_list.push_back("ptQ"); // --> little discrim
	var_list.push_back("tZq_pT");


	vector<TString> chan_list;
	chan_list.push_back("uuu");
	chan_list.push_back("eeu");
	chan_list.push_back("uue");
	chan_list.push_back("eee");

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<"python Generate_Datacards.py " + chan_list[ichan] + " " + var_list[ivar] + " " + file_histos<<endl;
		}

		file_out<<endl;
	}

	file_out<<endl<<endl;

	file_out <<"mkdir datacards_InputVars"<<endl;

	file_out<<endl<<endl<<endl;

	file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<var_list[ivar] + "_" + chan_list[ichan] + "=datacard_"+chan_list[ichan]+"_"+var_list[ivar]+".txt ";
		}
	}

	file_out<<"> COMBINED_datacard_InputVars.txt"<<endl;

	system("chmod 755 Generate_Datacards_InputVars.sh");

	file_out<<"mv datacard_*.txt datacards_InputVars/";

	return;
}




void Script_Datacards_TemplateFit()
{
	TString file_histos = "../templates/Combine_Input_ScaledFakes.root";

	ofstream file_out("Generate_Datacards_TemplateFit.sh");

	vector<TString> var_list;

	var_list.push_back("BDT");
	var_list.push_back("BDTttZ");
	var_list.push_back("mTW");

	vector<TString> chan_list;
	chan_list.push_back("uuu");
	chan_list.push_back("eeu");
	chan_list.push_back("uue");
	chan_list.push_back("eee");

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<"python Generate_Datacards.py " + chan_list[ichan] + " " + var_list[ivar] + " " + file_histos<<endl;
		}

		file_out<<endl;
	}

	file_out<<endl<<endl<<endl;

	file_out <<"mkdir datacards_TemplateFit"<<endl;

	file_out<<endl<<endl<<endl;

	file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<var_list[ivar] + "_" + chan_list[ichan] + "=datacard_"+chan_list[ichan]+"_"+var_list[ivar]+".txt ";
		}
	}

	file_out<<"> COMBINED_datacard_TemplateFit.txt"<<endl;

	system("chmod 755 Generate_Datacards_TemplateFit.sh");

	file_out<<"mv datacard_*.txt datacards_TemplateFit/";

	return;
}






int main()
{
	Script_Datacards_TemplateFit();

	Script_Datacards_InputVars();

	return 0;
}
