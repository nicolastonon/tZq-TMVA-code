#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <vector>
#include <sys/stat.h> // to be able to use mkdir
#include <sstream>
#include <fstream>
#include <algorithm>

#include <TROOT.h>
#include "TSystem.h"
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

//Use stat function (from library sys/stat) to check if a file exists
bool Check_File_Existence(const TString& name)
{
  struct stat buffer;
  return (stat (name.Data(), &buffer) == 0); //true if file exists
}



//------------------------------------------------------
// ######## ##     ## ######## ########     ###     ######  ########
// ##        ##   ##     ##    ##     ##   ## ##   ##    ##    ##
// ##         ## ##      ##    ##     ##  ##   ##  ##          ##
// ######      ###       ##    ########  ##     ## ##          ##
// ##         ## ##      ##    ##   ##   ######### ##          ##
// ##        ##   ##     ##    ##    ##  ##     ## ##    ##    ##
// ######## ##     ##    ##    ##     ## ##     ##  ######     ##
//
// ########  ##     ## ##    ##    ##    ## ########
// ##     ## ##     ## ###   ##    ###   ## ##     ##
// ##     ## ##     ## ####  ##    ####  ## ##     ##
// ########  ##     ## ## ## ##    ## ## ## ########
// ##   ##   ##     ## ##  ####    ##  #### ##   ##
// ##    ##  ##     ## ##   ###    ##   ### ##    ##
// ##     ##  #######  ##    ##    ##    ## ##     ##
//------------------------------------------------------


/**
 * Read 2 files : 1 file is a Control_Tree on which a BDT cut has been applied. The other is a "ready_forMEM" file. For each event from Control_Tree, we loop on the events of the other file and try to find a match based on the equality of 3 variables. When a match is found, the EvtNr of corresponding event (in "ready_forMEM" file) is saved in a .txt File
 * --> For each sample, we get a list of the EvtNr of events passing the cut
 * @param sample
 */
 void Extract_RunNr_From_HightBDT_Events(TString sample)
{
 	//File containing the EvtNr information (NB : use this rather than CIEMAT ntuple, because this is skimmed)
 	TString f_readyForMEM_path = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_readyForMEM/FCNCNTuple_"+sample+".root";
 	if(!Check_File_Existence(f_readyForMEM_path)) {cout<<f_readyForMEM_path<<" doesn't exist!"<<endl; return;}
 	TFile* f_readyForMEM = TFile::Open( f_readyForMEM_path);

 	//File containing the information "which events pass the cut on BDT?"
 	TString f_withMEM_path = "/home/nico/root/tmva/test/outputs/Control_Trees_NJetsMin1Max4_NBJetsEq1_CutBDT.root";
 	if(!Check_File_Existence(f_withMEM_path)) {cout<<f_withMEM_path<<" doesn't exist!"<<endl; return;}
 	TFile* f_withMEM = TFile::Open(f_withMEM_path);

 	TTree* t_readyForMEM = (TTree*) f_readyForMEM->Get("Tree");
 	TTree* t_withMEM = (TTree*) f_withMEM->Get(("Control_"+sample).Data());

 	Float_t NJets, NBJets, EvtNr, RunNr, mTW, btagDiscri, mtop;
 	Float_t NJets_MEM, NBJets_MEM, mTW_MEM, btagDiscri_MEM, mtop_MEM;

 	// t_readyForMEM->SetBranchAddress("NJets", &NJets);
 	// t_readyForMEM->SetBranchAddress("NBJets", &NBJets);
 	t_readyForMEM->SetBranchAddress("EvtNr", &EvtNr);
 	t_readyForMEM->SetBranchAddress("RunNr", &RunNr);
 	t_readyForMEM->SetBranchAddress("mTW", &mTW);
 	t_readyForMEM->SetBranchAddress("mtop", &mtop);
 	t_readyForMEM->SetBranchAddress("btagDiscri", &btagDiscri);

 	// t_withMEM->SetBranchAddress("NJets", &NJets_MEM);
 	// t_withMEM->SetBranchAddress("NBJets", &NBJets_MEM);
 	t_withMEM->SetBranchAddress("mTW", &mTW_MEM);
 	t_withMEM->SetBranchAddress("mtop", &mtop_MEM);
 	t_withMEM->SetBranchAddress("btagDiscri", &btagDiscri_MEM);

 	// if(t_readyForMEM->GetEntries() != t_withMEM->GetEntries())
 	// {
 		// cout<<"ERROR : different numbers of entries!!"<<endl;
 		// cout<<"t_readyForMEM->GetEntries() "<<t_readyForMEM->GetEntries()<<endl;
 		// cout<<"t_withMEM->GetEntries() "<<t_withMEM->GetEntries()<<endl;
 		// return;
 	// }


 	int nentries = t_readyForMEM->GetEntries();
 	int nentries_MEM = t_withMEM->GetEntries();

 	cout<<endl<<"*** STARTING FILLING VECTORS ***"<<endl;

 	//Rather than looping on "ready_forMEM" file, we store the 4 relevant variables (3 for comparison, 1 is EvtNr) into vectors ; this is probably faster than reading a file, and allows to remove events already matched
 	vector<Float_t> v_btagDiscri, v_mTW, v_EvtNr, v_RunNr, v_mtop;
 	for(int ientry=0; ientry<nentries; ientry++)
 	{
 		if(ientry%10000==0) {cout<<"-- "<<ientry<<" / "<<nentries<<endl;}


 		EvtNr=-999; btagDiscri = -999; mTW=-999; mtop=-999; RunNr=-999;
 		t_readyForMEM->GetEntry(ientry);

 		v_btagDiscri.push_back(btagDiscri);
 		v_EvtNr.push_back(EvtNr);
 		v_RunNr.push_back(RunNr);
 		v_mTW.push_back(mTW);
 		v_mtop.push_back(mtop);
 	}


 	cout<<endl<<"*** STARTING TO MATCH EVENTS *** "<<endl;

 	vector<double> v_sorted_EvtNr, v_EvtNr_passCut, v_RunNr_passCut;

 	// for(int ientry_MEM=50; ientry_MEM<60; ientry_MEM++)
 	for(int ientry_MEM=0; ientry_MEM<nentries_MEM; ientry_MEM++)
 	{
 		// cout<<"--- Entry "<<ientry_MEM<<" / "<<nentries_MEM<<endl;
 		if(ientry_MEM%10000==0) {cout<<"-- "<<ientry_MEM<<" / "<<nentries_MEM<<endl;}

 		// NJets_MEM=-999; NBJets_MEM=-999;
 		btagDiscri_MEM=-999; mTW_MEM=-999; mtop_MEM=-999;
 		t_withMEM->GetEntry(ientry_MEM);


 		//For each event passing the cut on BDT, loop on vectors and try to find a match, i.e. an event with the same values for 3 different variables (arbitrary)
 		for(int i=0; i<v_mTW.size(); i++)
 		{
 			if(v_mTW[i] == mTW_MEM) //If event is matched (same mTW)
 			{
 				if(v_btagDiscri[i]==btagDiscri_MEM ) //Make sure another variable is also matched
 				{
 					if(v_mtop[i]==mtop_MEM)
 					{
 						/*cout<<"-------"<<ientry_MEM<<"-------"<<endl;
 						cout<<"mTW = "<<v_mTW[i]<<endl;
 						cout<<"mTW_MEM = "<<mTW_MEM<<endl;
 						cout<<"mtop = "<<v_mtop[i]<<endl;
 						cout<<"mtop_MEM = "<<mtop_MEM<<endl;
 						cout<<"btagDiscri = "<<v_btagDiscri[i]<<endl;
 						cout<<"btagDiscri_MEM = "<<btagDiscri_MEM<<endl;
 						cout<<"EvtNr = "<<v_EvtNr[i]<<endl;*/


 						// t_EvtNr->Fill();
 						// cout.precision(10);
 						// cout<<v_EvtNr[i]<<endl;
 						// file_out.precision(10);
 						// file_out<<v_EvtNr[i]<<endl;


 						// v_sorted_EvtNr.push_back(v_EvtNr[i]); //if want to sort EvtNr ; not possible if we also extract RunNr info


 						v_EvtNr_passCut.push_back(v_EvtNr[i]);
 						v_RunNr_passCut.push_back(v_RunNr[i]);

 						v_mTW.erase(v_mTW.begin() + i);
 						v_mtop.erase(v_mtop.begin() + i);
 						v_btagDiscri.erase(v_btagDiscri.begin() + i);
 						v_EvtNr.erase(v_EvtNr.begin() + i);
 						v_RunNr.erase(v_RunNr.begin() + i);
 						break;
 					}
 				}
 			}


 			if(i==v_mTW.size()-1) //No match found
 			{
 				cout<<BOLD(FRED("Problem : event can't be matched !"))<<endl;

 				// cout<<"MET_MEM = "<<mtop_MEM<<endl;
 				// cout<<"mTW_MEM = "<<mTW_MEM<<endl;
 				// cout<<"btagDiscri_MEM = "<<btagDiscri_MEM<<endl;
 			}
 		}


 		// if(ientry_MEM==nentries_MEM-1)
 		// {
 		// 	for(int i=0; i<v_mtop.size(); i++)
 		// 	{
 		// 		if(fabs(v_mtop[i]-mtop_MEM) > 1) {continue;}
 		// 		if(v_mTW[i]!=mTW_MEM) {continue;}
 		//
 		// 		cout<<"-------"<<endl;
 		// 		cout<<"mTW = "<<v_mTW[i]<<endl;
 		// 		cout<<"mtop = "<<v_mtop[i]<<endl;
 		// 		cout<<"btagDiscri = "<<v_btagDiscri[i]<<endl;
 		// 	}
 		// }
 	}


 	// cout<<"-- Sorting Event numbers -- "<<endl;
 	// std::sort(v_sorted_EvtNr.begin(),v_sorted_EvtNr.end()); //Use sorting algorithm

 	mkdir("./EvtNr", 0777);
 	ofstream file_out(("EvtNr/"+sample+"_EvtNr.txt").Data());

 	file_out.precision(10);
 	// for(int i=0; i<v_sorted_EvtNr.size(); i++)
 	// {
 	// 	file_out<<v_sorted_EvtNr[i]<<endl; //Write Event numbers
 	// }

 	for(int i=0; i<v_EvtNr_passCut.size(); i++)
 	{
 		file_out<<v_EvtNr_passCut[i]; //Write Event numbers
 		file_out<<" "<<v_RunNr_passCut[i]<<endl; //Write Run numbers
 	}


 	return;
}








//------------------------------------------------------
//    ###    ########  ########     ##     ##    ###    ########         ########  #######
//   ## ##   ##     ## ##     ##    ##     ##   ## ##   ##     ##           ##    ##     ##
//  ##   ##  ##     ## ##     ##    ##     ##  ##   ##  ##     ##           ##    ##     ##
// ##     ## ##     ## ##     ##    ##     ## ##     ## ########            ##    ##     ##
// ######### ##     ## ##     ##     ##   ##  ######### ##   ##             ##    ##     ##
// ##     ## ##     ## ##     ##      ## ##   ##     ## ##    ##  ###       ##    ##     ##
// ##     ## ########  ########        ###    ##     ## ##     ## ###       ##     #######

// ##     ## ######## ##     ##    ##    ## ######## ##     ## ########  ##       ########
// ###   ### ##       ###   ###    ###   ##    ##    ##     ## ##     ## ##       ##
// #### #### ##       #### ####    ####  ##    ##    ##     ## ##     ## ##       ##
// ## ### ## ######   ## ### ##    ## ## ##    ##    ##     ## ########  ##       ######
// ##     ## ##       ##     ##    ##  ####    ##    ##     ## ##        ##       ##
// ##     ## ##       ##     ##    ##   ###    ##    ##     ## ##        ##       ##
// ##     ## ######## ##     ##    ##    ##    ##     #######  ##        ######## ########
//------------------------------------------------------


void Add_Variables_To_MEM_Ntuples(TString sample, vector<TString> v_TTrees, vector<TString> v_variables)
{
	//File containing the EvtNr information (NB : use this rather than CIEMAT ntuple, because this is skimmed)
  // TString f_withInfo_path = "/home/nico/Bureau/these/tZq/MEM_Interfacing/input_ntuples/FCNCNTuple_"+sample+".root"; //FIXME
  TString f_withInfo_path = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_readyForMEM/FCNCNTuple_"+sample+".root";
	if(!Check_File_Existence(f_withInfo_path)) {cout<<f_withInfo_path<<" doesn't exist!"<<endl; return;}
	TFile* f_withInfo = TFile::Open( f_withInfo_path);

	//File containing the information "which events pass the cut on BDT?"
	TString f_withMEM_path = "/home/nico/root/tmva/test/input_ntuples/ntuples_MEM/FCNCNTuple_"+sample+".root";
	if(!Check_File_Existence(f_withMEM_path)) {cout<<f_withMEM_path<<" doesn't exist!"<<endl; return;}
	TFile* f_withMEM = TFile::Open(f_withMEM_path, "UPDATE");

  vector<Float_t> v_floats_CIEMAT, v_floats_MEM;
  for(int ivar=0; ivar<v_variables.size(); ivar++)
  {
    v_floats_CIEMAT.push_back(-999);
    v_floats_MEM.push_back(-999);
  }

  for(int itree=0; itree<v_TTrees.size(); itree++)
  {
  	TTree* t_withInfo = 0;

    // if(v_TTrees[itree] == "Tree") t_withInfo = (TTree*) f_withInfo->Get("Default"); //CIEMAT's ntuples
    // else {t_withInfo = (TTree*) f_withInfo->Get(v_TTrees[itree]);}

    t_withInfo = (TTree*) f_withInfo->Get(v_TTrees[itree]);

    if(!t_withInfo) {cout<<"Tree "<<v_TTrees[itree]<<" not found ! Skip"<<endl; continue;}

    Float_t mTW, btagDiscri, mtop,  NJets, NBJets;
    // Float_t AdditionalMuonIso, AdditionalEleIso, EvtNr, RunNr, tZ_pT, tZ_mass, bj_mass_leadingJet, bj_mass_subleadingJet, bj_mass_leadingJet_pT40, bj_mass_leadingJet_pT50, bj_mass_leadingJet_pTlight40, bj_mass_leadingJet_pTlight50, bj_mass_leadingJet_etaCut, LeadingJetCSV, SecondJetCSV;

  	t_withInfo->SetBranchAddress("NJets", &NJets);
  	t_withInfo->SetBranchAddress("NBJets", &NBJets);
  	t_withInfo->SetBranchAddress("mTW", &mTW);
  	t_withInfo->SetBranchAddress("mtop", &mtop);
    t_withInfo->SetBranchAddress("btagDiscri", &btagDiscri);


    // t_withInfo->SetBranchAddress("EvtNr", &EvtNr);
    // t_withInfo->SetBranchAddress("RunNr", &RunNr);
    // t_withInfo->SetBranchAddress("AdditionalMuonIso", &AdditionalMuonIso);
    // t_withInfo->SetBranchAddress("AdditionalEleIso", &AdditionalEleIso);
    // t_withInfo->SetBranchAddress("tZ_pT", &tZ_pT);
    // t_withInfo->SetBranchAddress("tZ_mass", &tZ_mass);
    // t_withInfo->SetBranchAddress("bj_mass_leadingJet", &bj_mass_leadingJet);
    // t_withInfo->SetBranchAddress("bj_mass_subleadingJet", &bj_mass_subleadingJet);
    // t_withInfo->SetBranchAddress("bj_mass_leadingJet_pT40", &bj_mass_leadingJet_pT40);
    // t_withInfo->SetBranchAddress("bj_mass_leadingJet_pT50", &bj_mass_leadingJet_pT50);
    // t_withInfo->SetBranchAddress("bj_mass_leadingJet_pTlight40", &bj_mass_leadingJet_pTlight40);
    // t_withInfo->SetBranchAddress("bj_mass_leadingJet_pTlight50", &bj_mass_leadingJet_pTlight50);
    // t_withInfo->SetBranchAddress("bj_mass_leadingJet_etaCut", &bj_mass_leadingJet_etaCut);
    // t_withInfo->SetBranchAddress("LeadingJetCSV", &LeadingJetCSV);
    // t_withInfo->SetBranchAddress("SecondJetCSV", &SecondJetCSV);

    for(int ivar=0; ivar<v_variables.size(); ivar++)
    {
      t_withInfo->SetBranchAddress(v_variables[ivar], &v_floats_CIEMAT[ivar]);
    }


    TTree* t_withMEM = 0;
    t_withMEM = (TTree*) f_withMEM->Get(v_TTrees[itree]);
    if(!t_withMEM) {cout<<"Tree "<<v_TTrees[itree]<<" not found ! Skip"<<endl; continue;}

    Float_t mTW_MEM, NJets_MEM, NBJets_MEM, btagDiscri_MEM, mtop_MEM;
    // Float_t AdditionalMuonIso_MEM, AdditionalEleIso_MEM, RunNr_MEM, EvtNr_MEM, tZ_pT_MEM, tZ_mass_MEM, bj_mass_leadingJet_MEM, bj_mass_subleadingJet_MEM, bj_mass_leadingJet_pT40_MEM, bj_mass_leadingJet_pT50_MEM, bj_mass_leadingJet_pTlight40_MEM, bj_mass_leadingJet_pTlight50_MEM, bj_mass_leadingJet_etaCut_MEM, LeadingJetCSV_MEM, SecondJetCSV_MEM;

  	t_withMEM->SetBranchAddress("NJets", &NJets_MEM);
  	t_withMEM->SetBranchAddress("NBJets", &NBJets_MEM);
  	t_withMEM->SetBranchAddress("mTW", &mTW_MEM);
  	t_withMEM->SetBranchAddress("mtop", &mtop_MEM);
  	t_withMEM->SetBranchAddress("btagDiscri", &btagDiscri_MEM);


    //Create new branches in tree
    // TBranch *b_AdditionalMuonIso = t_withMEM->Branch("AdditionalMuonIso",&AdditionalMuonIso_MEM,"AdditionalMuonIso/F");
    // TBranch *b_AdditionalEleIso = t_withMEM->Branch("AdditionalEleIso",&AdditionalEleIso_MEM,"AdditionalEleIso/F");
    // TBranch *b_RunNr = t_withMEM->Branch("RunNr",&RunNr_MEM,"RunNr/F");
    // TBranch *b_EvtNr = t_withMEM->Branch("EvtNr",&EvtNr_MEM,"EvtNr/F");
    // TBranch *b_tZ_pT = t_withMEM->Branch("tZ_pT",&tZ_pT_MEM,"tZ_pT/F");
    // TBranch *b_tZ_mass = t_withMEM->Branch("tZ_mass",&tZ_mass_MEM,"tZ_mass/F");
    // TBranch *b_bj_mass_leadingJet = t_withMEM->Branch("bj_mass_leadingJet",&bj_mass_leadingJet_MEM,"bj_mass_leadingJet/F");
    // TBranch *b_bj_mass_subleadingJet = t_withMEM->Branch("bj_mass_subleadingJet",&bj_mass_subleadingJet_MEM,"bj_mass_subleadingJet/F");
    // TBranch *b_bj_mass_leadingJet_pT40 = t_withMEM->Branch("bj_mass_leadingJet_pT40",&bj_mass_leadingJet_pT40_MEM,"bj_mass_leadingJet_pT40/F");
    // TBranch *b_bj_mass_leadingJet_pT50 = t_withMEM->Branch("bj_mass_leadingJet_pT50",&bj_mass_leadingJet_pT50_MEM,"bj_mass_leadingJet_pT50/F");
    // TBranch *b_bj_mass_leadingJet_pTlight40 = t_withMEM->Branch("bj_mass_leadingJet_pTlight40",&bj_mass_leadingJet_pTlight40_MEM,"bj_mass_leadingJet_pTlight40/F");
    // TBranch *b_bj_mass_leadingJet_pTlight50 = t_withMEM->Branch("bj_mass_leadingJet_pTlight50",&bj_mass_leadingJet_pTlight50_MEM,"bj_mass_leadingJet_pTlight50/F");
    // TBranch *b_bj_mass_leadingJet_etaCut = t_withMEM->Branch("bj_mass_leadingJet_etaCut",&bj_mass_leadingJet_etaCut_MEM,"bj_mass_leadingJet_etaCut/F");
    // TBranch *b_LeadingJetCSV = t_withMEM->Branch("LeadingJetCSV",&LeadingJetCSV_MEM,"LeadingJetCSV/F");
    // TBranch *b_SecondJetCSV = t_withMEM->Branch("SecondJetCSV",&SecondJetCSV_MEM,"SecondJetCSV/F");

    vector<TBranch*> v_branches;
    for(int ivar=0; ivar<v_variables.size(); ivar++)
    {
      v_branches.push_back(t_withMEM->Branch(v_variables[ivar],&v_floats_MEM[ivar], (v_variables[ivar]+"/F").Data() ) );
    }



  	int nentries = t_withInfo->GetEntries();
  	int nentries_MEM = t_withMEM->GetEntries();

    cout<<endl<<endl<<FGRN("Tree : ")<<v_TTrees[itree];

  	cout<<endl<<"*** STARTING FILLING VARIABLES VECTORS ***"<<endl;

  	//Rather than looping on "ready_forMEM" file, we store the relevant variables into vectors ; this is probably faster than reading a file, and allows to remove events already matched
  	vector<Float_t> v_btagDiscri, v_mTW, v_mtop;
    // vector<Float_t> v_EvtNr, v_RunNr, v_AdditionalMuonIso, v_AdditionalEleIso, v_tZ_pT, v_tZ_mass, v_bj_mass_leadingJet, v_bj_mass_subleadingJet, v_bj_mass_leadingJet_pT40, v_bj_mass_leadingJet_pT50, v_bj_mass_leadingJet_pTlight40, v_bj_mass_leadingJet_pTlight50, v_bj_mass_leadingJet_etaCut, v_LeadingJetCSV, v_SecondJetCSV;

    vector< vector<Float_t> > v_vectors_floats;
    for(int ivar=0; ivar<v_variables.size(); ivar++)
    {
      vector<Float_t> tmp;
      v_vectors_floats.push_back(tmp);
    }



  	for(int ientry=0; ientry<nentries; ientry++)
  	{
  		if(ientry%10000==0) {cout<<"-- "<<ientry<<" / "<<nentries<<endl;}


  	 btagDiscri = -999; mTW=-999; mtop=-999; NJets=-999; NBJets=-999;
      // 	EvtNr=-999; RunNr=-999; AdditionalEleIso=-999; AdditionalMuonIso=-999; tZ_pT=-999; tZ_mass=-999; bj_mass_leadingJet=-999; bj_mass_subleadingJet=-999; bj_mass_leadingJet_pT40=-999; bj_mass_leadingJet_pT50=-999; bj_mass_leadingJet_pTlight40=-999; bj_mass_leadingJet_pTlight50=-999; bj_mass_leadingJet_etaCut=-999; LeadingJetCSV=-999; SecondJetCSV=-999;


  		t_withInfo->GetEntry(ientry);

      if(!(NJets>1 && NJets<4 && NBJets==1) && !(NJets>1 && NBJets>1) ) {continue;} //not MEM region

  		v_btagDiscri.push_back(btagDiscri);
      v_mTW.push_back(mTW);
      v_mtop.push_back(mtop);

  		// v_EvtNr.push_back(EvtNr);
  		// v_RunNr.push_back(RunNr);
      // v_AdditionalMuonIso.push_back(AdditionalMuonIso);
      // v_AdditionalEleIso.push_back(AdditionalEleIso);
      // v_tZ_pT.push_back(tZ_pT);
      // v_tZ_mass.push_back(tZ_mass);
      // v_bj_mass_leadingJet.push_back(bj_mass_leadingJet);
      // v_bj_mass_subleadingJet.push_back(bj_mass_subleadingJet);
      // v_bj_mass_leadingJet_pT40.push_back(bj_mass_leadingJet_pT40);
      // v_bj_mass_leadingJet_pT50.push_back(bj_mass_leadingJet_pT50);
      // v_bj_mass_leadingJet_pTlight40.push_back(bj_mass_leadingJet_pTlight40);
      // v_bj_mass_leadingJet_pTlight50.push_back(bj_mass_leadingJet_pTlight50);
      // v_bj_mass_leadingJet_etaCut.push_back(bj_mass_leadingJet_etaCut);
      // v_LeadingJetCSV.push_back(LeadingJetCSV);
      // v_SecondJetCSV.push_back(SecondJetCSV);
      for(int ivar=0; ivar<v_variables.size(); ivar++)
      {
        v_vectors_floats[ivar].push_back(v_floats_CIEMAT[ivar]);
      }

  	}


  	cout<<endl<<"*** STARTING TO MATCH EVENTS & FILL NEW BRANCHES IN MEM TREE *** "<<endl;

  	// for(int ientry_MEM=50; ientry_MEM<60; ientry_MEM++)
  	for(int ientry_MEM=0; ientry_MEM<nentries_MEM; ientry_MEM++)
  	{
  		// cout<<"--- Entry "<<ientry_MEM<<" / "<<nentries_MEM<<endl;
  		if(ientry_MEM%10000==0) {cout<<"-- "<<ientry_MEM<<" / "<<nentries_MEM<<endl;}

  		// NJets_MEM=-999; NBJets_MEM=-999;
  		btagDiscri_MEM=-999; mTW_MEM=-999; mtop_MEM=-999; NJets_MEM=-999; NBJets_MEM=-999;
      for(int ivar=0; ivar<v_floats_MEM.size(); ivar++)
      {
        v_floats_MEM[ivar]=-999;
      }

      // AdditionalEleIso_MEM=-999; AdditionalMuonIso_MEM=-999; tZ_pT_MEM=-999; tZ_mass_MEM=-999; bj_mass_leadingJet_MEM=-999; bj_mass_subleadingJet_MEM=-999; bj_mass_leadingJet_pT40_MEM=-999; bj_mass_leadingJet_pT50_MEM=-999; bj_mass_leadingJet_pTlight40_MEM=-999; bj_mass_leadingJet_pTlight50_MEM=-999; bj_mass_leadingJet_etaCut_MEM=-999; LeadingJetCSV_MEM=-999; SecondJetCSV_MEM=-999;
  		t_withMEM->GetEntry(ientry_MEM);


  		//For each event passing the cut on BDT, loop on vectors and try to find a match, i.e. an event with the same values for 3 different variables (arbitrary)
  		for(int i=0; i<v_mTW.size(); i++)
  		{
  			if(v_mTW[i] == mTW_MEM) //If event is matched (same mTW)
  			{
  				if(v_btagDiscri[i]==btagDiscri_MEM ) //Make sure another variable is also matched
  				{
  					if(v_mtop[i]==mtop_MEM)
  					{
  						/*cout<<"-------"<<ientry_MEM<<"-------"<<endl;
  						cout<<"mTW = "<<v_mTW[i]<<endl;
  						cout<<"mTW_MEM = "<<mTW_MEM<<endl;
  						cout<<"mtop = "<<v_mtop[i]<<endl;
  						cout<<"mtop_MEM = "<<mtop_MEM<<endl;
  						cout<<"btagDiscri = "<<v_btagDiscri[i]<<endl;
  						cout<<"btagDiscri_MEM = "<<btagDiscri_MEM<<endl;
  						cout<<"EvtNr = "<<v_EvtNr[i]<<endl;*/


              // AdditionalEleIso_MEM = v_AdditionalEleIso[i];
              // AdditionalMuonIso_MEM = v_AdditionalMuonIso[i];
              // RunNr_MEM = v_RunNr[i];
              // EvtNr_MEM = v_EvtNr[i];
              // tZ_pT_MEM = v_tZ_pT[i];
              // tZ_mass_MEM = v_tZ_mass[i];
              // bj_mass_leadingJet_MEM = v_bj_mass_leadingJet[i];
              // bj_mass_subleadingJet_MEM = v_bj_mass_subleadingJet[i];
              // bj_mass_leadingJet_pT40_MEM = v_bj_mass_leadingJet_pT40[i];
              // bj_mass_leadingJet_pT50_MEM = v_bj_mass_leadingJet_pT50[i];
              // bj_mass_leadingJet_pTlight40_MEM = v_bj_mass_leadingJet_pTlight40[i];
              // bj_mass_leadingJet_pTlight50_MEM = v_bj_mass_leadingJet_pTlight50[i];
              // bj_mass_leadingJet_etaCut_MEM = v_bj_mass_leadingJet_etaCut[i];
              // LeadingJetCSV_MEM = v_LeadingJetCSV[i];
              // SecondJetCSV_MEM = v_SecondJetCSV[i];

              for(int ivar=0; ivar<v_variables.size(); ivar++)
              {
                v_floats_MEM[ivar] = v_vectors_floats[ivar][i];
                v_branches[ivar]->Fill();
              }


              //Fill the new branches
              // b_AdditionalMuonIso->Fill();
              // b_AdditionalEleIso->Fill();
              // b_RunNr->Fill();
              // b_EvtNr->Fill();
              // b_tZ_pT->Fill();
              // b_tZ_mass->Fill();
              // b_bj_mass_leadingJet->Fill();
              // b_bj_mass_subleadingJet->Fill();
              // b_bj_mass_leadingJet_pT40->Fill();
              // b_bj_mass_leadingJet_pT50->Fill();
              // b_bj_mass_leadingJet_pTlight40->Fill();
              // b_bj_mass_leadingJet_pTlight50->Fill();
              // b_bj_mass_leadingJet_etaCut->Fill();
              // b_LeadingJetCSV->Fill();
              // b_SecondJetCSV->Fill();


  						v_mTW.erase(v_mTW.begin() + i);
  						v_mtop.erase(v_mtop.begin() + i);
  						v_btagDiscri.erase(v_btagDiscri.begin() + i);
  						// v_EvtNr.erase(v_EvtNr.begin() + i);
              // v_RunNr.erase(v_RunNr.begin() + i);
              // v_AdditionalMuonIso.erase(v_AdditionalMuonIso.begin() + i);
              // v_AdditionalEleIso.erase(v_AdditionalEleIso.begin() + i);
              // v_tZ_pT.erase(v_tZ_pT.begin() + i);
              // v_tZ_mass.erase(v_tZ_mass.begin() + i);
              // v_bj_mass_leadingJet.erase(v_bj_mass_leadingJet.begin() + i);
              // v_bj_mass_subleadingJet.erase(v_bj_mass_subleadingJet.begin() + i);
              // v_bj_mass_leadingJet_pT40.erase(v_bj_mass_leadingJet_pT40.begin() + i);
              // v_bj_mass_leadingJet_pT50.erase(v_bj_mass_leadingJet_pT50.begin() + i);
              // v_bj_mass_leadingJet_pTlight40.erase(v_bj_mass_leadingJet_pTlight40.begin() + i);
              // v_bj_mass_leadingJet_pTlight50.erase(v_bj_mass_leadingJet_pTlight50.begin() + i);
              // v_bj_mass_leadingJet_etaCut.erase(v_bj_mass_leadingJet_etaCut.begin() + i);
              // v_LeadingJetCSV.erase(v_LeadingJetCSV.begin() + i);
              // v_SecondJetCSV.erase(v_SecondJetCSV.begin() + i);

              for(int ivar=0; ivar<v_variables.size(); ivar++)
              {
                v_vectors_floats[ivar].erase(v_vectors_floats[ivar].begin() + i);
              }


  						break;
  					}
  				}
  			}


  			if(i==v_mTW.size()-1) //No match found
  			{
  				cout<<BOLD(FRED("Problem : event can't be matched !"))<<endl;

  				// cout<<"MET_MEM = "<<mtop_MEM<<endl;
  				// cout<<"mTW_MEM = "<<mTW_MEM<<endl;
  				// cout<<"btagDiscri_MEM = "<<btagDiscri_MEM<<endl;
  			}
  		}


  	}

    t_withMEM->Write("",TObject::kOverwrite); //Replace old 'cycle' (=instance) of the tree with the new (updated) one
    //NB : 'overwrite' will replace old cycle directly (faster) while 'kWriteDelete' will first write the new cycle (safer)

    delete t_withMEM;
    delete t_withInfo;
  }

  delete f_withMEM; delete f_withInfo;

	return;
}







int main()
{
//-----------------------------------------
 // #    #   ##   #####   ####     #####  ####       ##   #####  #####
 // #    #  #  #  #    # #           #   #    #     #  #  #    # #    #
 // #    # #    # #    #  ####       #   #    #    #    # #    # #    #
 // #    # ###### #####       #      #   #    #    ###### #    # #    #
 //  #  #  #    # #   #  #    #      #   #    #    #    # #    # #    #
 //   ##   #    # #    #  ####       #    ####     #    # #####  #####
//-----------------------------------------

//Vectors containing names of variables to add, as well as floats to store the values from both ntuples (from CIEMAT and IPHC) --> match events
  vector<TString> v_variables;
  v_variables.push_back("EvtNr");
  v_variables.push_back("RunNr");
  v_variables.push_back("AdditionalMuonIso");
  v_variables.push_back("AdditionalEleIso");
  v_variables.push_back("tZ_pT");
  v_variables.push_back("tZ_mass");
  v_variables.push_back("bj_mass_leadingJet");
  v_variables.push_back("bj_mass_subleadingJet");
  v_variables.push_back("bj_mass_leadingJet_pT40");
  v_variables.push_back("bj_mass_leadingJet_pT50");
  v_variables.push_back("bj_mass_leadingJet_pTlight40");
  v_variables.push_back("bj_mass_leadingJet_pTlight50");
  v_variables.push_back("bj_mass_leadingJet_etaCut");
  v_variables.push_back("LeadingJetCSV");
  v_variables.push_back("SecondJetCSV");
  v_variables.push_back("LeadingJetNonB_pT");
  v_variables.push_back("SecondJetNonB_pT");
  v_variables.push_back("ContainsBadJet");



//-----------------------------------------
 //  ####    ##   #    # #####  #      ######  ####
 // #       #  #  ##  ## #    # #      #      #
 //  ####  #    # # ## # #    # #      #####   ####
 //      # ###### #    # #####  #      #           #
 // #    # #    # #    # #      #      #      #    #
 //  ####  #    # #    # #      ###### ######  ####
//-----------------------------------------

	vector<TString> sample_list;
	sample_list.push_back("Data");
  sample_list.push_back("tZqmcNLO");
  sample_list.push_back("tZqQup");
	sample_list.push_back("tZqQdw");
	sample_list.push_back("ttZ");
	sample_list.push_back("ttH");
	sample_list.push_back("ZZ");
	sample_list.push_back("WZL");
	sample_list.push_back("WZB");
	sample_list.push_back("WZC");
	sample_list.push_back("STtWll");
	sample_list.push_back("ttW");
  sample_list.push_back("FakesNewNew");

  // sample_list.push_back("Fakes");
  // sample_list.push_back("tWZ");


//-----------------------------------------
 // ##### ##### #####  ###### ######  ####     #    #   ##   #    # ######  ####
 //   #     #   #    # #      #      #         ##   #  #  #  ##  ## #      #
 //   #     #   #    # #####  #####   ####     # #  # #    # # ## # #####   ####
 //   #     #   #####  #      #           #    #  # # ###### #    # #           #
 //   #     #   #   #  #      #      #    #    #   ## #    # #    # #      #    #
 //   #     #   #    # ###### ######  ####     #    # #    # #    # ######  ####
//-----------------------------------------

  vector<TString> v_TTrees;
  // v_TTrees.push_back("Tree");
  // v_TTrees.push_back("JER__plus");
  // v_TTrees.push_back("JER__minus");
  // v_TTrees.push_back("JES__plus");
  // v_TTrees.push_back("JES__minus");
  // v_TTrees.push_back("Fakes__plus");
  v_TTrees.push_back("Fakes__minus");


//-----------------------------------------
 // ####### #     # #     #  #####  ####### ### ####### #     #     #####     #    #       #        #####
 // #       #     # ##    # #     #    #     #  #     # ##    #    #     #   # #   #       #       #     #
 // #       #     # # #   # #          #     #  #     # # #   #    #        #   #  #       #       #
 // #####   #     # #  #  # #          #     #  #     # #  #  #    #       #     # #       #        #####
 // #       #     # #   # # #          #     #  #     # #   # #    #       ####### #       #             #
 // #       #     # #    ## #     #    #     #  #     # #    ##    #     # #     # #       #       #     #
 // #        #####  #     #  #####     #    ### ####### #     #     #####  #     # ####### #######  #####
//-----------------------------------------

cout<<endl<<BOLD(FYEL("##################################"))<<endl;
cout<<FYEL("--- Event Matching ---")<<endl;
cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	// Extract_RunNr_From_HightBDT_Events("ttW");


	for(int isample=0; isample<sample_list.size(); isample++)
	{
		cout<<endl<<"===== MATCHING EVENTS FOR SAMPLE "<<sample_list[isample]<<" ====="<<endl;

    // Extract_RunNr_From_HightBDT_Events(sample_list[isample]);

    Add_Variables_To_MEM_Ntuples(sample_list[isample], v_TTrees, v_variables);
	}


	return 0;
}
