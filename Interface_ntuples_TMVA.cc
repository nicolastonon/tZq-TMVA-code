#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <vector>
#include <sys/stat.h> // to be able to use mkdir
#include <sstream>

#include <TROOT.h>
#include <TFile.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TString.h"
#include "TColor.h"
#include "TCut.h"

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

using namespace std;

//Convert an int into a TString -- helper func
TString Convert_Number_To_TString(int number)
{
	stringstream ss;
	ss << number;
	TString ts = ss.str();
	return ts;
}

/**
 * Take ntuples containing all MEM vars + analysis vars & outputs ntuple containing only vars needed for rest of the analysis (also take log of MEM vars, etc.)
 * @param sample     sample name
 * @param thevarlist list of vars NOT related to MEM to be in the output ntuple
 * @param MEMvarlist list of vars RELATED to MEM to be in the output ntuple
 */
void Modify_Ntuples(TString sample, vector<TString> thevarlist, vector<TString> MEMvarlist, vector<TString> weight_syst_list, vector<TString> tree_syst_list, TString MEM_or_WZ)
{
	vector<float> v_floats; //Store all other variables as floats
	vector<double> v_double_MEM; //Store MEM variables as doubles
	vector<float> v_floats_MEM; //Needed to convert MEM variables from doubles to floats (needed by BDT code as floats)
	vector<float> v_floats_modif; //Store all other variables in floats
	vector<float> v_floats_syst; //Store syst weights

	//Input
	TString input_filename;

	// FIXME -- Use "withMEM" ntuples in ttZ/tZq regions
	// if(MEM_or_WZ == "MEM") 		input_filename = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_withMEM/FCNCNTuple_" + sample +  ".root";
	// FIXME -- Use "readyForMEM" (no MEM) in ttZ/tZq regions
	if(MEM_or_WZ == "MEM") 		input_filename = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_readyForMEM/FCNCNTuple_" + sample +  ".root";

	else if(MEM_or_WZ == "WZ") input_filename = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_WZ/FCNCNTuple_" + sample +  ".root";

	TFile* f_input = 0;
  	f_input = new TFile(input_filename.Data()); if(!f_input || f_input->IsZombie() ) {cout<<"Can't find input file !"<<endl; return;}

	//Output
	mkdir("input_ntuples/ntuples_MEM",0755);
	mkdir("input_ntuples/ntuples_WZ",0755);

	TString output_filename;
	if(MEM_or_WZ == "MEM") output_filename 	= "input_ntuples/ntuples_MEM/FCNCNTuple_" + sample +  ".root";
	else if(MEM_or_WZ == "WZ") output_filename 	= "input_ntuples/ntuples_WZ/FCNCNTuple_" + sample +  ".root";
	else {cout<<"ERROR -- abort !"<<endl; return;}

	cout<<endl<<endl<<"--- Creating new Ntuple from "<<input_filename.Data()<<endl;

	TFile* f_output = TFile::Open( output_filename, "RECREATE" );

	for(int itreesyst = 0; itreesyst<tree_syst_list.size(); itreesyst++)
	{
		//Only 'Tree' tree for data and fakes (+'Fakes' syst tree for fakes)
	    if( (sample.Contains("Data") && tree_syst_list[itreesyst] != "Tree") || ( sample.Contains("Fakes") && tree_syst_list[itreesyst] != "Tree" && !tree_syst_list[itreesyst].Contains("Fakes") ) ) {continue;}
	    if( tree_syst_list[itreesyst].Contains("Fakes") && !sample.Contains("Fakes") ) {continue;} //'Fakes' syst. tree only for Fakes sample

	    cout<<FGRN("SYSTEMATIC TREE NAME : ")<<tree_syst_list[itreesyst]<<endl;

		TTree* tree_modif = 0;
		tree_modif = new TTree(tree_syst_list[itreesyst].Data(), "");

		int n_MEMvars = 4; //FIXME -- adapt here the # of final MEM vars to be implemented

		//Allocate memory/elements to vectors
		//NOTE : don't do it in same loop as Branch() !! (push_back can change vector address -->segfault)
		for(int i=0; i<thevarlist.size(); i++)
		{
			v_floats.push_back(-999);
		}
		for(int i=0; i<MEMvarlist.size(); i++)
		{
			v_floats_MEM.push_back(-999);
			v_double_MEM.push_back(-999);
		}
		for(int i=0; i<n_MEMvars; i++)
		{
			v_floats_modif.push_back(-999);
		}
		for(int i=0; i<weight_syst_list.size(); i++)
		{
			if(sample=="Data" || sample=="Fakes" || tree_syst_list[itreesyst] != "Tree") break;
			v_floats_syst.push_back(-999);
		}

		//Prepare output tree
		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			tree_modif->Branch(thevarlist[ivar].Data(),&v_floats[ivar],(thevarlist[ivar]+"/F").Data());
		}
		for(int ivar=0; ivar<MEMvarlist.size(); ivar++)
		{
			tree_modif->Branch(MEMvarlist[ivar].Data(),&v_floats_MEM[ivar],(MEMvarlist[ivar]+"/F").Data());
		}
		for(int ivar=0; ivar<n_MEMvars; ivar++)
		{
			tree_modif->Branch(("MEMvar_"+Convert_Number_To_TString(ivar)).Data(),&v_floats_modif[ivar],("MEMvar_"+Convert_Number_To_TString(ivar)+"/F").Data());
		}
		//Systematics renaming for Combine
		for(int isyst=0; isyst<weight_syst_list.size(); isyst++)
		{
			if(sample.Contains("Data") || sample.Contains("Fakes") || tree_syst_list[itreesyst] != "Tree") break; //No weight syst


			//--- Copy systematic name & modify it for Combine conventions

			// TString systname_tmp = weight_syst_list[isyst];
			// if(systname_tmp.Contains("__plus"))
			// {
			// 	int i = systname_tmp.Index("__plus"); //Find index of substring
			// 	systname_tmp.Remove(i); //Remove substring
			// 	systname_tmp+= "Up"; //Add Combine syst. suffix
			// }
			// else if(systname_tmp.Contains("__minus"))
			// {
			// 	int i = systname_tmp.Index("__minus"); //Find index of substring
			// 	systname_tmp.Remove(i); //Remove substring
			// 	systname_tmp+= "Down"; //Add Combine syst. suffix
			// }

			// tree_modif->Branch(systname_tmp.Data(),&v_floats_syst[isyst],(systname_tmp+"/F").Data());
			tree_modif->Branch(weight_syst_list[isyst].Data(),&v_floats_syst[isyst],(weight_syst_list[isyst]+"/F").Data());
		}

		//Set Branch Addresses
		TTree* t_input = 0;
		t_input = (TTree*) f_input->Get(tree_syst_list[itreesyst].Data()); if(!t_input) {cout<<"Tree not found !"<<endl; return;}

		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			t_input->SetBranchAddress(thevarlist[ivar].Data(), &v_floats[ivar]);
		}
		for(int ivar=0; ivar<MEMvarlist.size(); ivar++)
		{
			if(MEM_or_WZ == "MEM") t_input->SetBranchAddress(MEMvarlist[ivar].Data(), &v_double_MEM[ivar]); //For SetBranchAddress, need to use doubles
		}
		for(int ivar=0; ivar<weight_syst_list.size(); ivar++)
		{
			if(sample=="Data" || sample=="Fakes" || tree_syst_list[itreesyst] != "Tree")  break;

			t_input->SetBranchAddress(weight_syst_list[ivar].Data(), &v_floats_syst[ivar]); //NOTE : new syst branches have different names (but associated to same floats)
		}

		//Event loop : modify vars, fill output tree
		int nentries = t_input->GetEntries();
		for(int ientry=0; ientry<nentries; ientry++)
		{
			if(ientry%10000==0) {cout<<ientry<<" / "<<nentries<<endl;}

			t_input->GetEntry(ientry);

			// cout<<"v_floats[0] / "<<thevarlist[0].Data()<<" = "<<v_floats[0]<<endl;

			//FIXME -- Change Here if want to use "readyForMEM" (no MEM) rather than "withMEM" ntuples in ttZ/tZq regions
			if(MEM_or_WZ == "MEM")
			{
				//NOTE : MEMvar list is :
				// MEMvarlist.push_back("mc_mem_ttz_weight");
				// MEMvarlist.push_back("mc_mem_tllj_weight");
				// MEMvarlist.push_back("mc_mem_tllj_weight_kinmaxint");
				// MEMvarlist.push_back("mc_mem_ttz_weight_kinmaxint");
				// MEMvarlist.push_back("mc_mem_ttz_tllj_likelihood");

				// thevarlist.push_back("-log((3.89464e-13*mc_mem_ttz_weight) / (3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"); //0
				// thevarlist.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //1
				// thevarlist.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //2

				// thevarlist_ttZ.push_back("-log(mc_mem_ttz_tllj_likelihood)"); //3
				// thevarlist_ttZ.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //1
				// thevarlist_ttZ.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //2

				v_floats_modif[0] = -log( (3.89464e-13*v_double_MEM[0]) / (3.89464e-13*v_double_MEM[0] + 0.17993*v_double_MEM[1]) );
				v_floats_modif[1] = log(v_double_MEM[2]);
				v_floats_modif[2] = log(v_double_MEM[3]);
				v_floats_modif[3] = -log(v_double_MEM[4]);

				for(int i=0; i<v_double_MEM.size(); i++)
				{
					v_floats_MEM[i] = (float) v_double_MEM[i]; //Convert from doubles to float --> Store as floats in output ntuple
				}
			}

			tree_modif->Fill();
		}


		f_output->cd();
	  	tree_modif->Write(); //Write output tree in output ntuple

	  	tree_modif->Delete();
	} //end tree syst loop


	f_input->Close(); f_input->Delete();
	f_output->Close(); f_output->Delete();

	return;
}




int main()
{
//---------------------------------------------------------------------------
//  ######     ###    ##     ## ########  ##       ########  ######
// ##    ##   ## ##   ###   ### ##     ## ##       ##       ##    ##
// ##        ##   ##  #### #### ##     ## ##       ##       ##
//  ######  ##     ## ## ### ## ########  ##       ######    ######
//       ## ######### ##     ## ##        ##       ##             ##
// ##    ## ##     ## ##     ## ##        ##       ##       ##    ##
//  ######  ##     ## ##     ## ##        ######## ########  ######
//---------------------------------------------------------------------------

	vector<TString> sample_list;
	// sample_list.push_back("Data");
	sample_list.push_back("tZq");
	// sample_list.push_back("WZl");
	// sample_list.push_back("WZb");
	// sample_list.push_back("WZc");
	sample_list.push_back("ttZ");
	sample_list.push_back("ttW");
	// sample_list.push_back("ttH");
	// sample_list.push_back("ZZ");
	sample_list.push_back("Fakes");
	sample_list.push_back("STtWll")	;

	// sample_list.push_back("ttZMad");
	// sample_list.push_back("WZjets");
	// sample_list.push_back("");


//---------------------------------------------------------------------------
    //   ##     ##    ###    ########   ######
    //   ##     ##   ## ##   ##     ## ##    ##
    //   ##     ##  ##   ##  ##     ## ##
    //   ##     ## ##     ## ########   ######
    //    ##   ##  ######### ##   ##         ##
    //     ## ##   ##     ## ##    ##  ##    ##
    //      ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------

//NOTE : make sure the variable list is up-to-date compared to Ntuples' contents !!


	vector<TString> thevarlist;
	thevarlist.push_back("Weight");
	thevarlist.push_back("Channel");
	thevarlist.push_back("NJets");
	thevarlist.push_back("NBJets");
	thevarlist.push_back("mTW");
	thevarlist.push_back("METpt");

	thevarlist.push_back("btagDiscri");
	thevarlist.push_back("dRAddLepQ");
	thevarlist.push_back("dRAddLepClosestJet");
	thevarlist.push_back("dPhiAddLepB");
	thevarlist.push_back("ZEta");
	thevarlist.push_back("Zpt");
	thevarlist.push_back("mtop");
	thevarlist.push_back("AddLepAsym");
	thevarlist.push_back("etaQ");
	thevarlist.push_back("AddLepETA");
	thevarlist.push_back("LeadJetEta");
	thevarlist.push_back("dPhiZAddLep");
	thevarlist.push_back("dRZAddLep"); // --> little discrim --> to be included
	thevarlist.push_back("dRjj");
	thevarlist.push_back("ptQ"); // --> little discrim
	thevarlist.push_back("tZq_pT");
	thevarlist.push_back("dRAddLepB");
	thevarlist.push_back("TopPT"); // low discri power
	thevarlist.push_back("m3l");
	thevarlist.push_back("dRZTop");

	//--- New vars
	thevarlist.push_back("MAddLepB");
	thevarlist.push_back("LeadJetPT");
	thevarlist.push_back("dPhiZMET");
	thevarlist.push_back("dPhiZTop");
	thevarlist.push_back("dPhiAddLepQ");
	thevarlist.push_back("TopEta");
	thevarlist.push_back("tZq_mass");
	thevarlist.push_back("tq_mass");
	thevarlist.push_back("tq_pT");
	thevarlist.push_back("tq_eta");
	thevarlist.push_back("AdditionalMuonIso");
	thevarlist.push_back("AdditionalEleIso");
	thevarlist.push_back("FCNCTopMass");
	thevarlist.push_back("CJetPT");
	thevarlist.push_back("dRZCJet");
	thevarlist.push_back("dRlWCJet");
	thevarlist.push_back("dRSMtopFCNCTop");
	thevarlist.push_back("dR_ZBJet");
	thevarlist.push_back("dPhi_ZCJet");
	thevarlist.push_back("dPhi_AddLepCJet");
	thevarlist.push_back("dPhi_SMtopFCNCTop");
	thevarlist.push_back("dPhi_ZBJet");
	thevarlist.push_back("LeadElePT");
	thevarlist.push_back("leadMuPT");
	thevarlist.push_back("NMediumBjets");
	thevarlist.push_back("LeadingJetCSV");
	thevarlist.push_back("SecondJetCSV");
	thevarlist.push_back("leadingLeptonPT");
	thevarlist.push_back("leadingLeptonEta");
	thevarlist.push_back("PV");
	thevarlist.push_back("dupECALcl");
	thevarlist.push_back("hitsNotRep");
	thevarlist.push_back("badMuon");
	thevarlist.push_back("duplMuon");


	vector<TString> MEMvarlist; //FIXME add new MEM vars
	MEMvarlist.push_back("mc_mem_ttz_weight");
	MEMvarlist.push_back("mc_mem_tllj_weight");
	MEMvarlist.push_back("mc_mem_tllj_weight_kinmaxint");
	MEMvarlist.push_back("mc_mem_ttz_weight_kinmaxint");
	MEMvarlist.push_back("mc_mem_ttz_tllj_likelihood");



//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------
//--- Affect the variable distributions
	vector<TString> tree_syst_list;
	tree_syst_list.push_back("Tree"); //NOTE -- KEEP THIS LINE : nominal

	tree_syst_list.push_back("JERUp"); tree_syst_list.push_back("JERDown");
	tree_syst_list.push_back("JESUp"); tree_syst_list.push_back("JESDown");
	tree_syst_list.push_back("FakesUp"); tree_syst_list.push_back("FakesDown");


	vector<TString> weight_syst_list;
//--- Affect the event weight
	weight_syst_list.push_back("Q2__plus"); weight_syst_list.push_back("Q2__minus");
	weight_syst_list.push_back("PU__plus"); weight_syst_list.push_back("PU__minus");
	weight_syst_list.push_back("MuEff__plus"); weight_syst_list.push_back("MuEff__minus");
	weight_syst_list.push_back("EleEff__plus"); weight_syst_list.push_back("EleEff__minus");
	weight_syst_list.push_back("pdf__plus"); weight_syst_list.push_back("pdf__minus");
	weight_syst_list.push_back("LFcont__plus"); weight_syst_list.push_back("LFcont__minus");
	weight_syst_list.push_back("HFstats1__plus"); weight_syst_list.push_back("HFstats1__minus");
	weight_syst_list.push_back("HFstats2__plus"); weight_syst_list.push_back("HFstats2__minus");
	weight_syst_list.push_back("CFerr1__plus"); weight_syst_list.push_back("CFerr1__minus");
	weight_syst_list.push_back("CFerr2__plus"); weight_syst_list.push_back("CFerr2__minus");
	weight_syst_list.push_back("HFcont__plus"); weight_syst_list.push_back("HFcont__minus");
	weight_syst_list.push_back("LFstats1__plus"); weight_syst_list.push_back("LFstats1__minus");
	weight_syst_list.push_back("LFstats2__plus"); weight_syst_list.push_back("LFstats2__minus");
	weight_syst_list.push_back("Weight_noTag"); weight_syst_list.push_back("Weight_noPU");





//Need to differenciate ttZ/tZq & WZ, since MEM can't run in WZ region (not enough jets) ==> Different ntuples
	TString MEM_or_WZ;

//--- Produce ntuples for MEM (ttZ/tZq regions)
	MEM_or_WZ = "MEM";
	for(int isample=0; isample<sample_list.size(); isample++)
	{
		Modify_Ntuples(sample_list[isample].Data(), thevarlist, MEMvarlist, weight_syst_list, tree_syst_list, MEM_or_WZ);
	}

//--- Interface ntuples for WZ CR study (mTW template fit)
	MEM_or_WZ = "WZ";
	for(int isample=0; isample<sample_list.size(); isample++)
	{
		Modify_Ntuples(sample_list[isample].Data(), thevarlist, MEMvarlist, weight_syst_list, tree_syst_list, MEM_or_WZ);
	}

	return 0;
}
