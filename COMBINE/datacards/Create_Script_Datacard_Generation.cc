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

void Script_Datacards_InputVars(char include_systematics, double fake_rate, int WZ_template, int SR_template, char add_region, char double_uncert)
{
	if( (include_systematics != 'y' && include_systematics != 'n') ) {cout<<"Wrong arguments ! Abort !"<<endl; return;}

	TString systList;
	if(include_systematics == 'y') {systList = "allSyst";}
	else if(include_systematics == 'n') {systList = "noSyst";}


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
	if(region_choice=="tZq" || region_choice=="WZ")
	{
		var_list.push_back("btagDiscri");
		var_list.push_back("dRAddLepClosestJet");
		var_list.push_back("dPhiAddLepB");
		var_list.push_back("ZEta");
		var_list.push_back("mtop");
		var_list.push_back("AddLepAsym");
		var_list.push_back("etaQ");
		var_list.push_back("AddLepETA");
		var_list.push_back("LeadJetEta");
		var_list.push_back("dPhiZAddLep");
		var_list.push_back("dRjj");
		var_list.push_back("ptQ");
		var_list.push_back("NJets");

		if(region_choice=="tZq")
		{
			var_list.push_back("MEMvar_0");
			var_list.push_back("MEMvar_1");
			var_list.push_back("MEMvar_2");
			var_list.push_back("MEMvar_8");
		}
	}
	else
	{
		var_list.push_back("btagDiscri");
	    var_list.push_back("dRAddLepQ");
	    var_list.push_back("dRAddLepClosestJet");
	    var_list.push_back("Zpt");
	    var_list.push_back("ZEta");
	    var_list.push_back("AddLepAsym");
	    var_list.push_back("etaQ");
	    var_list.push_back("ptQ");
	    var_list.push_back("dPhiZAddLep");
	    var_list.push_back("dRjj");
	    var_list.push_back("mtop");
		var_list.push_back("dRZTop");
		var_list.push_back("NJets");

		var_list.push_back("MEMvar_1");
		var_list.push_back("MEMvar_3");
	}


	vector<TString> chan_list;
	chan_list.push_back("uuu");
	chan_list.push_back("eeu");
	chan_list.push_back("uue");
	chan_list.push_back("eee");

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<"python Generate_Datacards.py " + chan_list[ichan] + " " + var_list[ivar] + " " + file_histos + " " + systList + " "<<fake_rate<<endl;
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

	file_out<<"> COMBINED_datacard_InputVars_tZq";
	if(systList == "noSyst") file_out<<"_noSyst";
	file_out<<".txt"<<endl<<endl;

	system("chmod 755 Generate_Datacards_InputVars.sh");

	file_out<<"mv datacard_*.txt datacards_InputVars/";

	cout<<FGRN("---> Created script Generate_Datacards_InputVars.sh, for region "<<region_choice<<" !")<<endl;

	return;
}




void Script_Datacards_TemplateFit(char include_systematics, double fake_rate, int WZ_template, int SR_template, char add_region, char double_uncert)
{
	if( (include_systematics != 'y' && include_systematics != 'n') ) {cout<<"Wrong arguments ! Abort !"<<endl; return;}

	TString systList;
	if(include_systematics == 'y') {systList = "allSyst";}
	else if(include_systematics == 'n') {systList = "noSyst";}


	TString file_histos = "../templates/Combine_Input.root";

	cout<<"---> Will use filepath : "<<file_histos<<endl<<endl;

	ofstream file_out("Generate_Datacards_TemplateFit.sh");

	vector<TString> var_list;

	if(SR_template==0) var_list.push_back("BDT");
	else if(SR_template==1) var_list.push_back("mTWandBDT");
	if(add_region=='y') var_list.push_back("BDT0l");
	var_list.push_back("BDTttZ");
	if(WZ_template==0) var_list.push_back("mTW");
	else if(WZ_template==1) var_list.push_back("BDTfake");

	vector<TString> chan_list;
	chan_list.push_back("uuu");
	chan_list.push_back("eeu");
	chan_list.push_back("uue");
	chan_list.push_back("eee");

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<"python Generate_Datacards.py " + chan_list[ichan] + " " + var_list[ivar] + " " + file_histos + " " + systList + " "<<fake_rate<<" "<<double_uncert<<endl;
		}

		file_out<<endl;
	}

	file_out<<endl<<endl<<endl;

	file_out <<"mkdir datacards_TemplateFit"<<endl;

	file_out<<endl<<endl<<endl;

	//Default datacard
	file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";
	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		for(int ichan=0; ichan<chan_list.size(); ichan++)
		{
			file_out<<var_list[ivar] + "_" + chan_list[ichan] + "=datacard_"+chan_list[ichan]+"_"+var_list[ivar]+".txt ";
		}
	}

	TString output_name = "COMBINED_datacard_TemplateFit_tZq";
	if(systList == "noSyst") output_name+= "_noSyst";
	output_name+= ".txt";

	file_out<<"> "<<output_name<<endl<<endl;


	//datacard for single channels
	for(int ichan=0; ichan<chan_list.size(); ichan++)
	{
		file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

		for(int ivar=0; ivar<var_list.size(); ivar++)
		{
			file_out<<var_list[ivar] + "_" + chan_list[ichan] + "=datacard_"+chan_list[ichan]+"_"+var_list[ivar]+".txt ";
		}

		output_name = "COMBINED_datacard_TemplateFit_tZq_";
		if(systList == "noSyst") output_name+= "noSyst_";
		output_name+= chan_list[ichan]+".txt";

		file_out<<"> "<<output_name<<endl<<endl;

		file_out<<"mv "<<output_name<<" datacards_TemplateFit/"<<endl<<endl;
	}


	//datacards for 2 regions at a time
	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

		for(int jvar=0; jvar<var_list.size(); jvar++)
		{
			if(jvar==ivar) {continue;}
			for(int ichan=0; ichan<chan_list.size(); ichan++)
			{
				file_out<<var_list[jvar] + "_" + chan_list[ichan] + "=datacard_"+chan_list[ichan]+"_"+var_list[jvar]+".txt ";
			}
		}

		TString output_name = "COMBINED_datacard_TemplateFit_tZq";
		if(systList == "noSyst") output_name+= "_noSyst";
		output_name+= "_no_" + var_list[ivar];


		output_name+= ".txt";

		file_out<<"> "<<output_name<<endl<<endl;

		file_out<<"mv "<<output_name<<" datacards_TemplateFit/"<<endl<<endl;
	}


	system("chmod 755 Generate_Datacards_TemplateFit.sh");



	file_out<<"mv datacard_*.txt datacards_TemplateFit/";

	cout<<FGRN("Created script Generate_Datacards_TemplateFit.sh !")<<endl;

	return;
}






int main()
{
	char include_systematics = 'n';
	char datacard_template_fit = 'n';
	char datacard_inputVars = 'n';
	double fake_rate = 400;
	// double fake_bkg = 30;
	int WZ_template = 0;
	int SR_template = 0;
	char add_region = 'n';
	char double_uncert = 'n';

	cout<<BOLD(FBLU("### Will create script for generation of combined Datacard ###"))<<endl<<endl;

	cout<<FYEL("--- What templates do you want to use in SR region ? (0 : BDT / 1 : mTWandBDT)")<<endl;
	cin>>SR_template;
	while(SR_template != 0 && SR_template != 1)
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 0 or 1 ! Retry :"<<endl;
		cin>>SR_template;
	}

	cout<<FYEL("--- What templates do you want to use in WZ region ? (0 : mTW / 1 : BDTfake)")<<endl;
	cin>>WZ_template;
	while(WZ_template != 0 && WZ_template != 1)
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 0 or 1 ! Retry :"<<endl;
		cin>>WZ_template;
	}

	cout<<FYEL("--- Do you want to add region [1 bjet, 0 light] ? (y/n)")<<endl;
	cin>>add_region;
	while(add_region != 'y' && add_region != 'n')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>add_region;
	}

	cout<<FYEL("--- Do you want to include all the systematic nuisances in the datacards ? (y/n)")<<endl;
	cin>>include_systematics;
	while(include_systematics != 'y' && include_systematics != 'n')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>include_systematics;
	}

	// cout<<FYEL("--- Choose FakeBackgrounds uncertainties (correlated b/w channels) in % : (ex : 30%)")<<endl;
	// cin>>fake_bkg;
	// while(cin.fail() || fake_bkg<0 )
	// {
	// 	cin.clear();
	// 	cin.ignore(1000, '\n');
	//
	// 	cout<<" Wrong answer ! :"<<endl;
	// 	cin>>fake_bkg;
	// }
	// if(fake_bkg != 0) fake_bkg = 1 + (fake_bkg / 100.0);

	cout<<FYEL("--- Choose FakeRates uncertainties (uncorrelated) in % : ('0' <-> use hard-coded values)")<<endl;
	cin>>fake_rate;
	while(cin.fail() || fake_rate<0)
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to choose fake rate > 0 :"<<endl;
		cin>>fake_rate;
	}

	if(fake_rate != 0) fake_rate = 1 + (fake_rate / 100.0);

	if(fake_rate==0)
	{
		cout<<FYEL("--- Double fakeRate uncertainties (wrt to default)? y/n")<<endl;
		cin>>double_uncert;
		while(cin.fail() || (double_uncert!= 'y' && double_uncert!= 'n'))
		{
			cin.clear();
			cin.ignore(1000, '\n');

			cout<<" Wrong answer ! Need to choose y or n :"<<endl;
			cin>>double_uncert;
		}
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


	if(datacard_template_fit == 'y') Script_Datacards_TemplateFit(include_systematics, fake_rate, WZ_template, SR_template, add_region, double_uncert);

	if( datacard_inputVars == 'y' ) Script_Datacards_InputVars(include_systematics, fake_rate, WZ_template, SR_template, add_region, double_uncert);

	return 0;
}
