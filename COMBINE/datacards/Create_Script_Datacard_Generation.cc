//g++ Create_Script_Datacard_Generation.cc -o Create_Script_Datacard_Generation.exe `root-config --cflags --glibs`

/* BASH COLORS */
#define RST   "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#include <iostream>
#include <fstream>
#include <vector>
#include "TString.h"

using namespace std;

void Script_Datacards_InputVars(char include_systematics, int signal_choice)
{
	if( (include_systematics != 'y' && include_systematics != 'n') || (signal_choice != 0 && signal_choice != 1 && signal_choice !=2 && signal_choice !=3)  ) {cout<<"Wrong arguments ! Abort !"<<endl; return;}

	TString systList;
	if(include_systematics == 'y') {systList = "allSyst";}
	else if(include_systematics == 'n') {systList = "noSyst";}
	TString signal;
	if(signal_choice == 0) {signal = "tZq";}
	else if(signal_choice == 1) {signal = "ttZ";}
	else if(signal_choice == 2) {signal = "tZqANDttZ";}
	else if(signal_choice == 3) {signal = "tZqANDFakes";}

	TString file_histos = "";
	TString region_choice = "";

	while(region_choice != "WZ" && region_choice != "ttZ" && region_choice != "tZq")
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<endl<<FYEL("--- In what region do you wish to plot postfit input variables ? (Type 'tZq', 'ttZ' or 'WZ' )")<<endl;
		cin>>region_choice;
	}

	if(region_choice=="WZ") file_histos = "../templates/Control_Histograms_NJetsMin0_NBJetsEq0.root";
	else if(region_choice=="tZq") file_histos = "../templates/Control_Histograms_NJetsMin1Max4_NBJetsEq1.root";
	else if(region_choice=="ttZ") file_histos = "../templates/Control_Histograms_NJetsMin1_NBJetsMin1.root";

	cout<<"---> Will use filepath : "<<file_histos<<endl<<endl;

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
			file_out<<"python Generate_Datacards.py " + chan_list[ichan] + " " + var_list[ivar] + " " + file_histos + " " + systList + " " + signal<<endl;
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

	cout<<FGRN("---> Created script Generate_Datacards_InputVars.sh, for region "<<region_choice<<" !")<<endl;

	return;
}




void Script_Datacards_TemplateFit(char include_systematics, int signal_choice)
{
	if( (include_systematics != 'y' && include_systematics != 'n') || (signal_choice != 0 && signal_choice != 1 && signal_choice !=2 && signal_choice !=3)  ) {cout<<"Wrong arguments ! Abort !"<<endl; return;}

	TString systList;
	if(include_systematics == 'y') {systList = "allSyst";}
	else if(include_systematics == 'n') {systList = "noSyst";}
	TString signal;
	if(signal_choice == 0) {signal = "tZq";}
	else if(signal_choice == 1) {signal = "ttZ";}
	else if(signal_choice == 2) {signal = "tZqANDttZ";}
	else if(signal_choice == 3) {signal = "tZqANDFakes";}

	TString file_histos = "../templates/Combine_Input.root";

	cout<<"---> Will use filepath : "<<file_histos<<endl<<endl;

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
			file_out<<"python Generate_Datacards.py " + chan_list[ichan] + " " + var_list[ivar] + " " + file_histos + " " + systList + " " + signal<<endl;
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

	cout<<FGRN("Created script Generate_Datacards_TemplateFit.sh !")<<endl;

	return;
}






int main()
{
	char include_systematics = 'n';
	int signal_choice = -1;
	char datacard_template_fit = 'n';
	char datacard_inputVars = 'n';

	cout<<BOLD(FBLU("### Will create script for generation of combined Datacard ###"))<<endl<<endl;

	cout<<FYEL("--- Do you want to include all the systematic nuisances in the datacards ? (y/n)")<<endl;
	cin>>include_systematics;
	while(include_systematics != 'y' && include_systematics != 'n')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>include_systematics;
	}

	cout<<endl<<FYEL("--- What processes do you want to let free in the fit ?"<<endl<<"['0' = tZq / '1' = ttZ / '2' = tZq & ttZ / '3' = tZq & Fakes]")<<endl;
	cin>>signal_choice;
	while(signal_choice != 0 && signal_choice != 1 && signal_choice != 2 && signal_choice != 3)
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<"Wrong answer ! Need to choose b/w 0, 1 or 2 ! Retry :"<<endl;
		cin>>signal_choice;
	}

	cout<<FYEL("--- Do you want to create datacard for a Template Fit ? (y/n) ")<<endl;
	cin>>datacard_template_fit;
	while(datacard_template_fit != 'y' && datacard_template_fit != 'n')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>datacard_template_fit;
	}

	cout<<FYEL("--- Do you want to create datacard for Postfit Distrib. of Input Vars ? (y/n) ")<<endl;
	cin>>datacard_inputVars;
	while(datacard_inputVars != 'y' && datacard_inputVars != 'n')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>datacard_inputVars;
	}


	if(datacard_template_fit == 'y') Script_Datacards_TemplateFit(include_systematics, signal_choice);

	if( datacard_inputVars == 'y' ) Script_Datacards_InputVars(include_systematics, signal_choice);

	return 0;
}
