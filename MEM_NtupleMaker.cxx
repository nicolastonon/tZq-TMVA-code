#include <fstream>
#include <iostream>
#include <math.h>
#include <sys/stat.h> // to be able to use mkdir

#include "TH1F.h"
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TObject.h"
#include "TCanvas.h"

#define kCat_3l_2b_2j 0
#define kCat_3l_1b_2j 1
#define kCat_3l_2b_1j 2
#define kCat_3l_1b_1j 3
#define kCat_3l_2b_0j 4

#include "MEM_NtupleMaker.h"
#include "MuTree.h"

using namespace std;

//Overloaded constructor (default)
MEM_NtupleMaker::MEM_NtupleMaker(TString samplename, vector<TString> BDT_variables, vector<TString> v_variablesToCreate, vector<TString> weight_syst_list, vector<TString> tree_syst_list, TString region_choice, double set_CSV_threshold, double set_eta_threshold, TString set_tmp_dir)
{
  if(region_choice != "MEM" && region_choice != "WZ" && region_choice != "1btag") {cout<<endl<<BOLD( FRED("ERROR : choose WZ or (tZq+ttZ) region !") )<<endl;}
  region = region_choice;

  tmp_dir = set_tmp_dir;
  if(tmp_dir != "") cout<<endl<<"Input taken from dir. "<<tmp_dir<<endl;
  if(tmp_dir != "") cout<<"Output written in dir. "<<tmp_dir<<endl;

  CSV_threshold = set_CSV_threshold; eta_threshold = set_eta_threshold;

  mkdir("output_ntuples/ntuples_readyForMEM/",0755);
  mkdir("output_ntuples/ntuples_WZ/",0755);

  mkdir(("output_ntuples/ntuples_1btag/"+tmp_dir).Data(),0755);
  mkdir(("output_ntuples/ntuples_readyForMEM/"+tmp_dir).Data(),0755);
  mkdir(("output_ntuples/ntuples_WZ/"+tmp_dir).Data(),0755);

  if(region == "MEM")      output_dir = "output_ntuples/ntuples_readyForMEM/";
  else if(region == "WZ")  output_dir = "output_ntuples/ntuples_WZ/";
  else if(region == "1btag")  output_dir = "output_ntuples/ntuples_1btag/";
  else {return;}

  if(tmp_dir != "") output_dir+= tmp_dir + "/";

  //Initialize member vectors
  for(int isyst=0; isyst<weight_syst_list.size(); isyst++)
  {
    if(samplename=="Data" || samplename=="Fakes") {break;} //No syst. for data

    v_syst_weight.push_back(weight_syst_list[isyst]);
    v_syst_float.push_back(0);
  }
  for(int ivar=0; ivar<BDT_variables.size(); ivar++)
  {
    BDTvar_list.push_back(BDT_variables[ivar]);

    if(BDTvar_list[ivar] == "fourthLep10" || BDTvar_list[ivar] == "fourthLep15") BDTvar_floats.push_back(0); //0 by default (~boolean)
    else BDTvar_floats.push_back(-999);
  }
  for(int itree=0; itree<tree_syst_list.size(); itree++)
  {
    v_syst_tree.push_back(tree_syst_list[itree]);
  }
  for(int ivar=0; ivar<v_variablesToCreate.size(); ivar++)
  {
    this->v_variablesToCreate.push_back(v_variablesToCreate[ivar]);
    v_variablesToCreate_float.push_back(-999);
  }



  Init();
  //Need to initialize ptrs passed as branch addresses
  vSelectedElectrons = new vector<ciemat::Electron>;
  vSelectedMuons = new vector<ciemat::Muon>;
  vSelectedJets = new vector<ciemat::Jet>;
  METCollection = 0;

  TString filename = output_dir + "FCNCNTuple_" + samplename+".root";

  f_output = new TFile(filename.Data(), "RECREATE");

  cout<<endl<<"Initialization & Tree preparation --- Done !"<<endl;
}

//Destructor
MEM_NtupleMaker::~MEM_NtupleMaker()
{
  delete vSelectedElectrons; delete vSelectedMuons; delete vSelectedJets;

  // tree_output->Delete();
  f_output->Close();
  f_output->Delete();
}


//-------------------------------------------
// ####    ##    ##    ####    ########
//  ##     ###   ##     ##        ##
//  ##     ####  ##     ##        ##
//  ##     ## ## ##     ##        ##
//  ##     ##  ####     ##        ##
//  ##     ##   ###     ##        ##
// ####    ##    ##    ####       ##
//-------------------------------------------

void MEM_NtupleMaker::Init()
{
  //4-vectors initialized at (0,0,0,0)

  catJets = -1;
  is_3l_TTZ_CR = false;
  is_3l_WZ_CR  = false;
  is_3l_TZQ_SR = false;
  is_3l_1BTAG_SR = false;
  mc_ttZhypAllowed = 0;

  //Leptons
  multilepton_Lepton1_Id = -999; multilepton_Lepton2_Id = -999; multilepton_Lepton3_Id = -999; multilepton_Lepton4_Id = -999;

  //b-jet
  multilepton_Bjet1_Id = -999; multilepton_Bjet2_Id = -999;
  multilepton_Bjet1_CSV = -999; multilepton_Bjet2_CSV = -999;

  //jet
  multilepton_JetHighestPt1_Id = -999; multilepton_JetHighestPt2_Id = -999; multilepton_JetClosestMw1_Id = -999; multilepton_JetClosestMw2_Id = -999; multilepton_JetHighestEta1_Id = -999; multilepton_JetHighestEta2_Id = -999;
  multilepton_JetHighestPt1_CSV = -999; multilepton_JetHighestPt2_CSV = -999; multilepton_JetClosestMw1_CSV = -999; multilepton_JetClosestMw2_CSV = -999; multilepton_JetHighestEta1_CSV = -999; multilepton_JetHighestEta2_CSV = -999;

  //Not used in tZq analysis, but needed by MEM code
//-----------------
  multilepton_JetHighestPt1_2ndPair_Id = -999;  multilepton_JetHighestPt2_2ndPair_Id = -999;
  multilepton_JetClosestMw1_2ndPair_Id = -999;  multilepton_JetClosestMw2_2ndPair_Id = -999;
  multilepton_JetLowestMjj1_Id = -999;          multilepton_JetLowestMjj2_Id = -999;
  multilepton_JetLowestMjj1_2ndPair_Id = -999;  multilepton_JetLowestMjj2_2ndPair_Id = -999;

  multilepton_JetHighestPt1_2ndPair_CSV = -999;  multilepton_JetHighestPt2_2ndPair_CSV = -999;
  multilepton_JetClosestMw1_2ndPair_CSV = -999;  multilepton_JetClosestMw2_2ndPair_CSV = -999;
  multilepton_JetLowestMjj1_CSV = -999;          multilepton_JetLowestMjj2_CSV = -999;
  multilepton_JetLowestMjj1_2ndPair_CSV = -999;  multilepton_JetLowestMjj2_2ndPair_CSV = -999;

  multilepton_Lepton1_DeltaR_Matched = -999;    multilepton_Lepton2_DeltaR_Matched = -999;    multilepton_Lepton3_DeltaR_Matched = -999;  multilepton_Lepton4_DeltaR_Matched = -999;
  multilepton_Lepton1_Label_Matched = -999;     multilepton_Lepton2_Label_Matched = -999;     multilepton_Lepton3_Label_Matched = -999;   multilepton_Lepton4_Label_Matched = -999;
  multilepton_Lepton1_Id_Matched = -999;        multilepton_Lepton2_Id_Matched = -999;        multilepton_Lepton3_Id_Matched = -999;      multilepton_Lepton4_Id_Matched = -999;
  multilepton_Bjet1_DeltaR_Matched = -999;      multilepton_Bjet2_DeltaR_Matched = -999;
  multilepton_Bjet1_Label_Matched = -999;       multilepton_Bjet2_Label_Matched = -999;
  multilepton_Bjet1_Id_Matched = -999;          multilepton_Bjet2_Id_Matched = -999;

  multilepton_h0_Label = -999;            	   multilepton_t1_Label = -999; 	                multilepton_t2_Label = -999;
  multilepton_h0_Id = -999;       	           multilepton_t1_Id = -999; 		                  multilepton_t2_Id = -999;

//-----------------

  //OTHER
  Weight = 0;
  Channel = -999;
  NJets = -999;
  NBJets = -999;
  mTW = -999;
  EvtNr = -999;
  RunNr = -999;
  // AdditionalMuonIso = -999;
  // AdditionalEleIso = -999;

  // tZ_pT = -999;
  // tZ_mass = -999;
  // bj_mass_leadingJet = -999;
  // bj_mass_subleadingJet = -999;
  // bj_mass_leadingJet_pT40 = -999;
  // bj_mass_leadingJet_pT50 = -999;
  // bj_mass_leadingJet_pTlight40 = -999;
  // bj_mass_leadingJet_pTlight50 = -999;
  // bj_mass_leadingJet_etaCut = -999;

  for(int ivar=0; ivar<v_variablesToCreate.size(); ivar++)
  {
    v_variablesToCreate_float[ivar] = -999;
  }


  //Re-initialize vector contents
  for(int isyst=0; isyst<v_syst_float.size(); isyst++)
  {
    v_syst_float[isyst] = 0;
  }
  for(int ivar=0; ivar<BDTvar_floats.size(); ivar++)
  {
    if(BDTvar_list[ivar] == "fourthLep10" || BDTvar_list[ivar] == "fourthLep15") BDTvar_floats[ivar]=0;
    else BDTvar_floats[ivar]=-999;
  }
}


//-------------------------------------------
// ########  ########  ######## ########     ###    ########  ########        #######  ##     ## ######## ########  ##     ## ########
// ##     ## ##     ## ##       ##     ##   ## ##   ##     ## ##             ##     ## ##     ##    ##    ##     ## ##     ##    ##
// ##     ## ##     ## ##       ##     ##  ##   ##  ##     ## ##             ##     ## ##     ##    ##    ##     ## ##     ##    ##
// ########  ########  ######   ########  ##     ## ########  ######         ##     ## ##     ##    ##    ########  ##     ##    ##
// ##        ##   ##   ##       ##        ######### ##   ##   ##             ##     ## ##     ##    ##    ##        ##     ##    ##
// ##        ##    ##  ##       ##        ##     ## ##    ##  ##             ##     ## ##     ##    ##    ##        ##     ##    ##
// ##        ##     ## ######## ##        ##     ## ##     ## ########        #######   #######     ##    ##         #######     ##
//-------------------------------------------
/*
C : a character string terminated by the 0 character
B : an 8 bit signed integer (Char_t)
b : an 8 bit integer (UChar_t)
S : a 16 bit signed integer (Short_t)
s : a 16 bit integer (UShort_t)
I : a 32 bit signed integer (Int_t)
i : a 32 bit integer (UInt_t)
F : a 32 bit floating point (Float_t)
D : a 64 bit floating point (Double_t)
L : a 64 bit signed integer (Long64_t)
l : a 64 bit integer (ULong64_t)
O : [the letter o, not a zero] a boolean (Bool_t)
*/

/**
 * Prepare output Tree (creates branches & set addresses)
 */
void MEM_NtupleMaker::Prepare_Tree(TString samplename, TTree* tree_output, TString tree_name)
{
  tree_output->Branch("Weight",&Weight,"Weight/F");
  tree_output->Branch("Channel",&Channel,"Channel/F");
  tree_output->Branch("NJets",&NJets,"NJets/F");
  tree_output->Branch("NBJets",&NBJets,"NBJets/F");
  tree_output->Branch("mTW",&mTW,"mTW/F");
  tree_output->Branch("EvtNr",&EvtNr,"EvtNr/F");
  tree_output->Branch("RunNr",&RunNr,"RunNr/F");
  // tree_output->Branch("AdditionalMuonIso",&AdditionalMuonIso,"AdditionalMuonIso/F");
  // tree_output->Branch("AdditionalEleIso",&AdditionalEleIso,"AdditionalEleIso/F");

  for(int ivar=0; ivar<v_variablesToCreate.size(); ivar++)
  {
    tree_output->Branch(v_variablesToCreate[ivar],&v_variablesToCreate_float[ivar],(v_variablesToCreate[ivar]+"/F").Data() );
  }


  // tree_output->Branch("tZ_pT",&tZ_pT,"tZ_pT/F");
  // tree_output->Branch("tZ_mass",&tZ_mass,"tZ_mass/F");
  // tree_output->Branch("bj_mass_leadingJet",&bj_mass_leadingJet,"bj_mass_leadingJet/F");
  // tree_output->Branch("bj_mass_subleadingJet",&bj_mass_subleadingJet,"bj_mass_subleadingJet/F");
  // tree_output->Branch("bj_mass_leadingJet_pT40",&bj_mass_leadingJet_pT40,"bj_mass_leadingJet_pT40/F");
  // tree_output->Branch("bj_mass_leadingJet_pT50",&bj_mass_leadingJet_pT50,"bj_mass_leadingJet_pT50/F");
  // tree_output->Branch("bj_mass_leadingJet_pTlight40",&bj_mass_leadingJet_pTlight40,"bj_mass_leadingJet_pTlight40/F");
  // tree_output->Branch("bj_mass_leadingJet_pTlight50",&bj_mass_leadingJet_pTlight50,"bj_mass_leadingJet_pTlight50/F");
  // tree_output->Branch("bj_mass_leadingJet_etaCut",&bj_mass_leadingJet_etaCut,"bj_mass_leadingJet_etaCut/F");

  tree_output->Branch("catJets",&catJets,"catJets/I");

  tree_output->Branch("is_3l_TTZ_CR",&is_3l_TTZ_CR,"is_3l_TTZ_CR/B");
  tree_output->Branch("is_3l_WZ_CR",&is_3l_WZ_CR,"is_3l_WZ_CR/B");
  tree_output->Branch("is_3l_TZQ_SR",&is_3l_TZQ_SR,"is_3l_TZQ_SR/B");
  tree_output->Branch("is_3l_1BTAG_SR",&is_3l_1BTAG_SR,"is_3l_1BTAG_SR/B");

  //leptons
  tree_output->Branch("multilepton_Lepton1_Id",&multilepton_Lepton1_Id,"multilepton_Lepton1_Id/I");
  tree_output->Branch("multilepton_Lepton1_P4","TLorentzVector", &multilepton_Lepton1_P4);
  tree_output->Branch("multilepton_Lepton2_Id",&multilepton_Lepton2_Id,"multilepton_Lepton2_Id/I");
  tree_output->Branch("multilepton_Lepton2_P4","TLorentzVector",&multilepton_Lepton2_P4);
  tree_output->Branch("multilepton_Lepton3_Id",&multilepton_Lepton3_Id,"multilepton_Lepton3_Id/I");
  tree_output->Branch("multilepton_Lepton3_P4","TLorentzVector",&multilepton_Lepton3_P4);
  tree_output->Branch("multilepton_Lepton4_Id",&multilepton_Lepton4_Id,"multilepton_Lepton4_Id/I");
  tree_output->Branch("multilepton_Lepton4_P4","TLorentzVector",&multilepton_Lepton4_P4);

  //b-jet
  tree_output->Branch("multilepton_Bjet1_Id",&multilepton_Bjet1_Id,"multilepton_Bjet1_Id/I");
  tree_output->Branch("multilepton_Bjet1_P4","TLorentzVector",&multilepton_Bjet1_P4);
  tree_output->Branch("multilepton_Bjet1_CSV",&multilepton_Bjet1_CSV,"multilepton_Bjet1_CSV/F");

  tree_output->Branch("multilepton_Bjet2_Id",&multilepton_Bjet2_Id,"multilepton_Bjet2_Id/I");
  tree_output->Branch("multilepton_Bjet2_P4","TLorentzVector",&multilepton_Bjet2_P4);
  tree_output->Branch("multilepton_Bjet2_CSV",&multilepton_Bjet2_CSV,"multilepton_Bjet2_CSV/F");

  //jet
  tree_output->Branch("multilepton_JetHighestPt1_Id",&multilepton_JetHighestPt1_Id,"multilepton_JetHighestPt1_Id/I");
  tree_output->Branch("multilepton_JetHighestPt1_P4","TLorentzVector",&multilepton_JetHighestPt1_P4);
  tree_output->Branch("multilepton_JetHighestPt1_CSV",&multilepton_JetHighestPt1_CSV,"multilepton_JetHighestPt1_CSV/F");

  tree_output->Branch("multilepton_JetHighestPt2_Id",&multilepton_JetHighestPt2_Id,"multilepton_JetHighestPt2_Id/I");
  tree_output->Branch("multilepton_JetHighestPt2_P4","TLorentzVector",&multilepton_JetHighestPt2_P4);
  tree_output->Branch("multilepton_JetHighestPt2_CSV",&multilepton_JetHighestPt2_CSV,"multilepton_JetHighestPt2_CSV/F");

  tree_output->Branch("multilepton_JetClosestMw1_Id",&multilepton_JetClosestMw1_Id,"multilepton_JetClosestMw1_Id/I");
  tree_output->Branch("multilepton_JetClosestMw1_P4","TLorentzVector",&multilepton_JetClosestMw1_P4);
  tree_output->Branch("multilepton_JetClosestMw1_CSV",&multilepton_JetClosestMw1_CSV,"multilepton_JetClosestMw1_CSV/F");

  tree_output->Branch("multilepton_JetClosestMw2_Id",&multilepton_JetClosestMw2_Id,"multilepton_JetClosestMw2_Id/I");
  tree_output->Branch("multilepton_JetClosestMw2_P4","TLorentzVector",&multilepton_JetClosestMw2_P4);
  tree_output->Branch("multilepton_JetClosestMw2_CSV",&multilepton_JetClosestMw2_CSV,"multilepton_JetClosestMw2_CSV/F");

  tree_output->Branch("multilepton_JetHighestEta1_Id",&multilepton_JetHighestEta1_Id,"multilepton_JetHighestEta1_Id/I");
  tree_output->Branch("multilepton_JetHighestEta1_P4","TLorentzVector",&multilepton_JetHighestEta1_P4);
  tree_output->Branch("multilepton_JetHighestEta1_CSV",&multilepton_JetHighestEta1_CSV,"multilepton_JetHighestEta1_CSV/F");

  tree_output->Branch("multilepton_JetHighestEta2_Id",&multilepton_JetHighestEta2_Id,"multilepton_JetHighestEta2_Id/I");
  tree_output->Branch("multilepton_JetHighestEta2_P4","TLorentzVector",&multilepton_JetHighestEta2_P4);
  tree_output->Branch("multilepton_JetHighestEta2_CSV",&multilepton_JetHighestEta2_CSV,"multilepton_JetHighestEta2_CSV/F");

  //Neded by MEM code
//---------------------
  tree_output->Branch("multilepton_JetHighestPt1_2ndPair_Id",&multilepton_JetHighestPt1_2ndPair_Id,"multilepton_JetHighestPt1_2ndPair_Id/I");
  tree_output->Branch("multilepton_JetHighestPt2_2ndPair_Id",&multilepton_JetHighestPt2_2ndPair_Id,"multilepton_JetHighestPt2_2ndPair_Id/I");
  tree_output->Branch("multilepton_JetClosestMw1_2ndPair_Id",&multilepton_JetClosestMw1_2ndPair_Id,"multilepton_JetClosestMw1_2ndPair_Id/I");
  tree_output->Branch("multilepton_JetClosestMw2_2ndPair_Id",&multilepton_JetClosestMw2_2ndPair_Id,"multilepton_JetClosestMw2_2ndPair_Id/I");
  tree_output->Branch("multilepton_JetLowestMjj1_Id",&multilepton_JetLowestMjj1_Id,"multilepton_JetLowestMjj1_Id/I");
  tree_output->Branch("multilepton_JetLowestMjj2_Id",&multilepton_JetLowestMjj2_Id,"multilepton_JetLowestMjj2_Id/I");
  tree_output->Branch("multilepton_JetLowestMjj1_2ndPair_Id",&multilepton_JetLowestMjj1_2ndPair_Id,"multilepton_JetLowestMjj1_2ndPair_Id/I");
  tree_output->Branch("multilepton_JetLowestMjj2_2ndPair_Id",&multilepton_JetLowestMjj2_2ndPair_Id,"multilepton_JetLowestMjj2_2ndPair_Id/I");
  tree_output->Branch("multilepton_Lepton1_Id_Matched",&multilepton_Lepton1_Id_Matched,"multilepton_Lepton1_Id_Matched/I");
  tree_output->Branch("multilepton_Lepton2_Id_Matched",&multilepton_Lepton2_Id_Matched,"multilepton_Lepton2_Id_Matched/I");
  tree_output->Branch("multilepton_Lepton3_Id_Matched",&multilepton_Lepton3_Id_Matched,"multilepton_Lepton3_Id_Matched/I");
  tree_output->Branch("multilepton_Lepton4_Id_Matched",&multilepton_Lepton4_Id_Matched,"multilepton_Lepton4_Id_Matched/I");
  tree_output->Branch("multilepton_Bjet1_Id_Matched",&multilepton_Bjet1_Id_Matched,"multilepton_Bjet1_Id_Matched/I");
  tree_output->Branch("multilepton_Bjet2_Id_Matched",&multilepton_Bjet2_Id_Matched,"multilepton_Bjet2_Id_Matched/I");
  tree_output->Branch("multilepton_Lepton1_Label_Matched",&multilepton_Lepton1_Label_Matched,"multilepton_Lepton1_Label_Matched/I");
  tree_output->Branch("multilepton_Lepton2_Label_Matched",&multilepton_Lepton2_Label_Matched,"multilepton_Lepton2_Label_Matched/I");
  tree_output->Branch("multilepton_Lepton3_Label_Matched",&multilepton_Lepton3_Label_Matched,"multilepton_Lepton3_Label_Matched/I");
  tree_output->Branch("multilepton_Lepton4_Label_Matched",&multilepton_Lepton4_Label_Matched,"multilepton_Lepton4_Label_Matched/I");
  tree_output->Branch("multilepton_Bjet1_Label_Matched",&multilepton_Bjet1_Label_Matched,"multilepton_Bjet1_Label_Matched/I");
  tree_output->Branch("multilepton_Bjet2_Label_Matched",&multilepton_Bjet2_Label_Matched,"multilepton_Bjet2_Label_Matched/I");
  tree_output->Branch("multilepton_h0_Label",&multilepton_h0_Label,"multilepton_h0_Label/I");
  tree_output->Branch("multilepton_t1_Label",&multilepton_t1_Label,"multilepton_t1_Label/I");
  tree_output->Branch("multilepton_t2_Label",&multilepton_t2_Label,"multilepton_t2_Label/I");
  tree_output->Branch("multilepton_h0_Id",&multilepton_h0_Id,"multilepton_h0_Id/I");
  tree_output->Branch("multilepton_t1_Id",&multilepton_t1_Id,"multilepton_t1_Id/I");
  tree_output->Branch("multilepton_t2_Id",&multilepton_t2_Id,"multilepton_t2_Id/I");

  tree_output->Branch("multilepton_JetHighestPt1_2ndPair_CSV",&multilepton_JetHighestPt1_2ndPair_CSV,"multilepton_JetHighestPt1_2ndPair_CSV/F");
  tree_output->Branch("multilepton_JetHighestPt2_2ndPair_CSV",&multilepton_JetHighestPt2_2ndPair_CSV,"multilepton_JetHighestPt2_2ndPair_CSV/F");
  tree_output->Branch("multilepton_JetClosestMw1_2ndPair_CSV",&multilepton_JetClosestMw1_2ndPair_CSV,"multilepton_JetClosestMw1_2ndPair_CSV/F");
  tree_output->Branch("multilepton_JetClosestMw2_2ndPair_CSV",&multilepton_JetClosestMw2_2ndPair_CSV,"multilepton_JetClosestMw2_2ndPair_CSV/F");
  tree_output->Branch("multilepton_JetLowestMjj1_CSV",&multilepton_JetLowestMjj1_CSV,"multilepton_JetLowestMjj1_CSV/F");
  tree_output->Branch("multilepton_JetLowestMjj2_CSV",&multilepton_JetLowestMjj2_CSV,"multilepton_JetLowestMjj2_CSV/F");
  tree_output->Branch("multilepton_JetLowestMjj1_2ndPair_CSV",&multilepton_JetLowestMjj1_2ndPair_CSV,"multilepton_JetLowestMjj1_2ndPair_CSV/F");
  tree_output->Branch("multilepton_JetLowestMjj2_2ndPair_CSV",&multilepton_JetLowestMjj2_2ndPair_CSV,"multilepton_JetLowestMjj2_2ndPair_CSV/F");
  tree_output->Branch("multilepton_Lepton1_DeltaR_Matched",&multilepton_Lepton1_DeltaR_Matched,"multilepton_Lepton1_DeltaR_Matched/F");
  tree_output->Branch("multilepton_Lepton2_DeltaR_Matched",&multilepton_Lepton2_DeltaR_Matched,"multilepton_Lepton2_DeltaR_Matched/F");
  tree_output->Branch("multilepton_Lepton3_DeltaR_Matched",&multilepton_Lepton3_DeltaR_Matched,"multilepton_Lepton3_DeltaR_Matched/F");
  tree_output->Branch("multilepton_Lepton4_DeltaR_Matched",&multilepton_Lepton4_DeltaR_Matched,"multilepton_Lepton4_DeltaR_Matched/F");
  tree_output->Branch("multilepton_Bjet1_DeltaR_Matched",&multilepton_Bjet1_DeltaR_Matched,"multilepton_Bjet1_DeltaR_Matched/F");
  tree_output->Branch("multilepton_Bjet2_DeltaR_Matched",&multilepton_Bjet2_DeltaR_Matched,"multilepton_Bjet2_DeltaR_Matched/F");

  tree_output->Branch("multilepton_JetHighestPt1_2ndPair_P4","TLorentzVector",&multilepton_JetHighestPt1_2ndPair_P4);
  tree_output->Branch("multilepton_JetHighestPt2_2ndPair_P4","TLorentzVector",&multilepton_JetHighestPt2_2ndPair_P4);
  tree_output->Branch("multilepton_JetClosestMw1_2ndPair_P4","TLorentzVector",&multilepton_JetClosestMw1_2ndPair_P4);
  tree_output->Branch("multilepton_JetClosestMw2_2ndPair_P4","TLorentzVector",&multilepton_JetClosestMw2_2ndPair_P4);
  tree_output->Branch("multilepton_JetLowestMjj1_P4","TLorentzVector",&multilepton_JetLowestMjj1_P4);
  tree_output->Branch("multilepton_JetLowestMjj2_P4","TLorentzVector",&multilepton_JetLowestMjj2_P4);
  tree_output->Branch("multilepton_JetLowestMjj1_2ndPair_P4","TLorentzVector",&multilepton_JetLowestMjj1_2ndPair_P4);
  tree_output->Branch("multilepton_JetLowestMjj2_2ndPair_P4","TLorentzVector",&multilepton_JetLowestMjj2_2ndPair_P4);
  tree_output->Branch("multilepton_Lepton1_P4_Matched","TLorentzVector",&multilepton_Lepton1_P4_Matched);
  tree_output->Branch("multilepton_Lepton2_P4_Matched","TLorentzVector",&multilepton_Lepton2_P4_Matched);
  tree_output->Branch("multilepton_Lepton3_P4_Matched","TLorentzVector",&multilepton_Lepton3_P4_Matched);
  tree_output->Branch("multilepton_Lepton4_P4_Matched","TLorentzVector",&multilepton_Lepton4_P4_Matched);
  tree_output->Branch("multilepton_Bjet1_P4_Matched","TLorentzVector",&multilepton_Bjet1_P4_Matched);
  tree_output->Branch("multilepton_Bjet2_P4_Matched","TLorentzVector",&multilepton_Bjet2_P4_Matched);

  tree_output->Branch("multilepton_Ptot","TLorentzVector",&multilepton_Ptot);
  tree_output->Branch("multilepton_h0_P4","TLorentzVector",&multilepton_h0_P4);
  tree_output->Branch("multilepton_t1_P4","TLorentzVector",&multilepton_t1_P4);
  tree_output->Branch("multilepton_t2_P4","TLorentzVector",&multilepton_t2_P4);
//---------------------

  //met
  tree_output->Branch("multilepton_mET", "TLorentzVector",&multilepton_mET);
  tree_output->Branch("multilepton_mHT",&multilepton_mHT,"multilepton_mHT/F"); //metsumet

  tree_output->Branch("mc_ttZhypAllowed",&mc_ttZhypAllowed,"mc_ttZhypAllowed/I");

  //BDT
  for(int ivar=0; ivar<BDTvar_list.size(); ivar++)
  {
    tree_output->Branch(BDTvar_list[ivar].Data(),&BDTvar_floats[ivar],(BDTvar_list[ivar]+"/F").Data());
  }

  //Systematics
  if(!samplename.Contains("Data") && !samplename.Contains("Fakes") && tree_name == "Tree") //Not in Data/Fakes. Not for JES/JER/Fakes syst. trees
  {
    for(int isyst=0; isyst<v_syst_weight.size(); isyst++)
    {
      tree_output->Branch(v_syst_weight[isyst].Data(),&v_syst_float[isyst],(v_syst_weight[isyst]+"/F").Data() );
    }
  }

  return;
}



//-------------------------------------------
//  ######     ########    ##          ########     ######     ########       ########           ##    ########    ########     ######
// ##    ##    ##          ##          ##          ##    ##       ##          ##     ##          ##    ##             ##       ##    ##
// ##          ##          ##          ##          ##             ##          ##     ##          ##    ##             ##       ##
//  ######     ######      ##          ######      ##             ##          ########           ##    ######         ##        ######
//       ##    ##          ##          ##          ##             ##          ##     ##    ##    ##    ##             ##             ##
// ##    ##    ##          ##          ##          ##    ##       ##          ##     ##    ##    ##    ##             ##       ##    ##
//  ######     ########    ########    ########     ######        ##          ########      ######     ########       ##        ######
//-------------------------------------------

/**
//Selects the two highest b-tag jets. If only one b-tag, selects just this one.
 * @param vSelectedJets     Contains all selected jets (b & light )
 * @param BjetSel           Ordering criterion
 * @param ibsel1            Return bjet index 1
 * @param ibsel2            Return bjet index 2
 * @param doSelectOnlyBjets True if consider only bjets in vector
 */
void MEM_NtupleMaker::SelectBjets(std::vector<ciemat::Jet>* vSelectedJets, std::string BjetSel, int &ibsel1, int &ibsel2, bool doSelectOnlyBjets=true)
{
  int ib1=-1, ib2=-1;

  if (BjetSel=="HighestBtagDiscrim")
  {
    Float_t btag_max=-9999, btag_max2=-9999;
    for (int ib=0; ib<vSelectedJets->size(); ib++)
    {
      // if (doSelectOnlyBjets && (vSelectedJets->at(ib).btagCSV < 0.8484 || fabs(vSelectedJets->at(ib).eta) > eta_threshold) ) {continue;} //medium btag WP
      if (doSelectOnlyBjets && (vSelectedJets->at(ib).btagCSV < CSV_threshold || fabs(vSelectedJets->at(ib).eta) > eta_threshold) ) {continue;} //FIXME -- make sure bjet def is correct

      if (vSelectedJets->at(ib).btagCSV>btag_max)
      {
          btag_max2 = btag_max;
          ib2 = ib1;
          btag_max = vSelectedJets->at(ib).btagCSV;
          ib1 = ib;
      }
      else if (vSelectedJets->at(ib).btagCSV<btag_max && vSelectedJets->at(ib).btagCSV>btag_max2)
      {
          btag_max2 = vSelectedJets->at(ib).btagCSV;
          ib2 = ib;
      }
    }
  }
  /*
  if (BjetSel=="BtagHighestPt")
  {
    float pt_max=0, pt_max2=0;
    for (int ib=0; ib<vSelectedJets->size(); ib++)
    {
        //if (vSelectedJets->at(ib).btagCSV<0.423) continue;
        if (vSelectedJets->at(ib).pt>pt_max){
            pt_max2 = pt_max;
            ib2 = ib1;
            pt_max = vSelectedJets->at(ib).pt;
            ib1 = ib;
        }
        if (vSelectedJets->at(ib).pt<pt_max && vSelectedJets->at(ib).pt>pt_max2){
            pt_max2 = vSelectedJets->at(ib).pt;
            ib2 = ib;
        }
    }
  }*/

  ibsel1 = ib1;
  ibsel2 = ib2;
}


//-------------------------------------------
//  #######     ########     ########     ########    ########        ##          ########    ########     ########     #######     ##    ##     ######
// ##     ##    ##     ##    ##     ##    ##          ##     ##       ##          ##          ##     ##       ##       ##     ##    ###   ##    ##    ##
// ##     ##    ##     ##    ##     ##    ##          ##     ##       ##          ##          ##     ##       ##       ##     ##    ####  ##    ##
// ##     ##    ########     ##     ##    ######      ########        ##          ######      ########        ##       ##     ##    ## ## ##     ######
// ##     ##    ##   ##      ##     ##    ##          ##   ##         ##          ##          ##              ##       ##     ##    ##  ####          ##
// ##     ##    ##    ##     ##     ##    ##          ##    ##        ##          ##          ##              ##       ##     ##    ##   ###    ##    ##
//  #######     ##     ##    ########     ########    ##     ##       ########    ########    ##              ##        #######     ##    ##     ######
//-------------------------------------------

/**
 * Order leptons (muons & electrons) by highest Pt
 * @param channel            [eee,eeu,uue,uuu]
 * @param vSelectedMuons     [Contains 0 to 3 electrons passing the object selection]
 * @param vSelectedElectrons [Contains 0 to 3 muons passing the object selection]
 */
void MEM_NtupleMaker::OrderLeptons(TString channel, vector<ciemat::Muon>* vSelectedMuons, vector<ciemat::Electron>* vSelectedElectrons)
{
  //Create temporary variables which do not distinguish b/w el & mu
  int id1=-999, id2=-999, id3=-999;
  TLorentzVector lep1, lep2, lep3;

  const int ID_mu = -13, ID_el=-11; //See PDGID numbering scheme. Negative values so that multiplying by charge gives correct result (ex: muon ANTIparticle --> ID = -13)

  //Depending on channel, fill the TLorentzVectors from the different lepton vectors
  if(channel=="uuu")
  {
    id1 = ID_mu * vSelectedMuons->at(0).charge; lep1.SetPtEtaPhiE(vSelectedMuons->at(0).pt, vSelectedMuons->at(0).eta, vSelectedMuons->at(0).phi, vSelectedMuons->at(0).energy);
    id2 = ID_mu * vSelectedMuons->at(1).charge; lep2.SetPtEtaPhiE(vSelectedMuons->at(1).pt, vSelectedMuons->at(1).eta, vSelectedMuons->at(1).phi, vSelectedMuons->at(1).energy);
    id3 = ID_mu * vSelectedMuons->at(2).charge; lep3.SetPtEtaPhiE(vSelectedMuons->at(2).pt, vSelectedMuons->at(2).eta, vSelectedMuons->at(2).phi, vSelectedMuons->at(2).energy);
  }
  else if(channel=="uue")
  {
    id1 = ID_mu * vSelectedMuons->at(0).charge; lep1.SetPtEtaPhiE(vSelectedMuons->at(0).pt, vSelectedMuons->at(0).eta, vSelectedMuons->at(0).phi, vSelectedMuons->at(0).energy);
    id2 = ID_mu * vSelectedMuons->at(1).charge; lep2.SetPtEtaPhiE(vSelectedMuons->at(1).pt, vSelectedMuons->at(1).eta, vSelectedMuons->at(1).phi, vSelectedMuons->at(1).energy);
    id3 = ID_el * vSelectedElectrons->at(0).charge; lep3.SetPtEtaPhiE(vSelectedElectrons->at(0).pt, vSelectedElectrons->at(0).eta, vSelectedElectrons->at(0).phi, vSelectedElectrons->at(0).energy);
  }
  else if(channel=="eeu")
  {
    id1 = ID_el * vSelectedElectrons->at(0).charge; lep1.SetPtEtaPhiE(vSelectedElectrons->at(0).pt, vSelectedElectrons->at(0).eta, vSelectedElectrons->at(0).phi, vSelectedElectrons->at(0).energy);
    id2 = ID_el * vSelectedElectrons->at(1).charge; lep2.SetPtEtaPhiE(vSelectedElectrons->at(1).pt, vSelectedElectrons->at(1).eta, vSelectedElectrons->at(1).phi, vSelectedElectrons->at(1).energy);
    id3 = ID_mu * vSelectedMuons->at(0).charge; lep3.SetPtEtaPhiE(vSelectedMuons->at(0).pt, vSelectedMuons->at(0).eta, vSelectedMuons->at(0).phi, vSelectedMuons->at(0).energy);
  }
  else if(channel=="eee")
  {
    id1 = ID_el * vSelectedElectrons->at(0).charge; lep1.SetPtEtaPhiE(vSelectedElectrons->at(0).pt, vSelectedElectrons->at(0).eta, vSelectedElectrons->at(0).phi, vSelectedElectrons->at(0).energy);
    id2 = ID_el * vSelectedElectrons->at(1).charge; lep2.SetPtEtaPhiE(vSelectedElectrons->at(1).pt, vSelectedElectrons->at(1).eta, vSelectedElectrons->at(1).phi, vSelectedElectrons->at(1).energy);
    id3 = ID_el * vSelectedElectrons->at(2).charge; lep3.SetPtEtaPhiE(vSelectedElectrons->at(2).pt, vSelectedElectrons->at(2).eta, vSelectedElectrons->at(2).phi, vSelectedElectrons->at(2).energy);
  }
  else {cout<<endl<<BOLD( FRED("Wrong channel value !") )<<endl;}

  // cout<<"id1 = "<<id1<<endl;
  // cout<<"lep1.Pt()="<<lep1.Pt()<<endl;
  // cout<<"lep2.Pt()="<<lep2.Pt()<<endl;
  // cout<<"lep3.Pt()="<<lep3.Pt()<<endl;

  //Then order the lepton TLorentzVectors by highest Pt & Fill the NtupleMaker class' lepton TLorentzVectors accordingly
  if(lep1.Pt() > lep2.Pt() && lep1.Pt() > lep3.Pt() )
  {
    multilepton_Lepton1_Id = id1; multilepton_Lepton1_P4.SetPtEtaPhiE(lep1.Pt(), lep1.Eta(), lep1.Phi(), lep1.E());

    if(lep2.Pt() > lep3.Pt()) //1,2,3
    {
      multilepton_Lepton2_Id = id2; multilepton_Lepton2_P4.SetPtEtaPhiE(lep2.Pt(), lep2.Eta(), lep2.Phi(), lep2.E());
      multilepton_Lepton3_Id = id3; multilepton_Lepton3_P4.SetPtEtaPhiE(lep3.Pt(), lep3.Eta(), lep3.Phi(), lep3.E());
    }
    else //1,3,2
    {
      multilepton_Lepton2_Id = id3; multilepton_Lepton2_P4.SetPtEtaPhiE(lep3.Pt(), lep3.Eta(), lep3.Phi(), lep3.E());
      multilepton_Lepton3_Id = id2; multilepton_Lepton3_P4.SetPtEtaPhiE(lep2.Pt(), lep2.Eta(), lep2.Phi(), lep2.E());
    }
  }
  else if(lep3.Pt() > lep2.Pt() && lep3.Pt() > lep1.Pt() )
  {
    multilepton_Lepton1_Id = id3; multilepton_Lepton1_P4.SetPtEtaPhiE(lep3.Pt(), lep3.Eta(), lep3.Phi(), lep3.E());

    if(lep2.Pt() > lep1.Pt()) //3,2,1
    {
      multilepton_Lepton2_Id = id2; multilepton_Lepton2_P4.SetPtEtaPhiE(lep2.Pt(), lep2.Eta(), lep2.Phi(), lep2.E());
      multilepton_Lepton3_Id = id1; multilepton_Lepton3_P4.SetPtEtaPhiE(lep1.Pt(), lep1.Eta(), lep1.Phi(), lep1.E());
    }
    else //3,1,2
    {
      multilepton_Lepton2_Id = id1; multilepton_Lepton2_P4.SetPtEtaPhiE(lep1.Pt(), lep1.Eta(), lep1.Phi(), lep1.E());
      multilepton_Lepton3_Id = id2; multilepton_Lepton3_P4.SetPtEtaPhiE(lep2.Pt(), lep2.Eta(), lep2.Phi(), lep2.E());
    }
  }
  else if(lep2.Pt() > lep1.Pt() && lep2.Pt() > lep3.Pt() )
  {
    multilepton_Lepton1_Id = id2; multilepton_Lepton1_P4.SetPtEtaPhiE(lep2.Pt(), lep2.Eta(), lep2.Phi(), lep2.E());

    if(lep1.Pt() > lep3.Pt()) //2,1,3
    {
      multilepton_Lepton2_Id = id1; multilepton_Lepton2_P4.SetPtEtaPhiE(lep1.Pt(), lep1.Eta(), lep1.Phi(), lep1.E());
      multilepton_Lepton3_Id = id3; multilepton_Lepton3_P4.SetPtEtaPhiE(lep3.Pt(), lep3.Eta(), lep3.Phi(), lep3.E());
    }
    else //2,3,1
    {
      multilepton_Lepton2_Id = id3; multilepton_Lepton2_P4.SetPtEtaPhiE(lep3.Pt(), lep3.Eta(), lep3.Phi(), lep3.E());
      multilepton_Lepton3_Id = id1; multilepton_Lepton3_P4.SetPtEtaPhiE(lep1.Pt(), lep1.Eta(), lep1.Phi(), lep1.E());
    }
  }
  else {cout<<__LINE__<<" : Problem !"<<endl;}
}



//-------------------------------------------
//  #######     ########     ########     ########    ########              ##    ########    ########     ######
// ##     ##    ##     ##    ##     ##    ##          ##     ##             ##    ##             ##       ##    ##
// ##     ##    ##     ##    ##     ##    ##          ##     ##             ##    ##             ##       ##
// ##     ##    ########     ##     ##    ######      ########              ##    ######         ##        ######
// ##     ##    ##   ##      ##     ##    ##          ##   ##         ##    ##    ##             ##             ##
// ##     ##    ##    ##     ##     ##    ##          ##    ##        ##    ##    ##             ##       ##    ##
//  #######     ##     ##    ########     ########    ##     ##        ######     ########       ##        ######
//-------------------------------------------
/**
 * Determine highest pt, 2nd highest pt, highest eta jets & dijet with inv. mass closest to W mass
 * @param vSelectedJets [Contains all jets (btagged or not) passing the object selection]
 */
void MEM_NtupleMaker::OrderJets(vector<ciemat::Jet>* vSelectedJets, const int ib1, const int ib2, int &ij1, int &ij2, int &ik1, int &ik2, int &ie1, int &ie2)
{
  TLorentzVector Pjet1, Pjet2; //tmp
  float pt_max=0, pt_max2=0;
  float diffmass_min = 10000;
  float eta_max=0, eta_max2=0;

  for(int ij=0; ij<vSelectedJets->size(); ij++)
  {
    if (ij==ib1 || ij==ib2) continue; //Don't take bjets into account

    if (vSelectedJets->at(ij).pt > pt_max ) //Highest pT
    {
      pt_max2 = pt_max;
      ij2 = ij1;
      pt_max = vSelectedJets->at(ij).pt;
      ij1 = ij;
    }
    else if(vSelectedJets->at(ij).pt < pt_max && vSelectedJets->at(ij).pt > pt_max2) //2nd Highest pT
    {
      pt_max2 = vSelectedJets->at(ij).pt;
      ij2 = ij;
    }

    if(fabs(vSelectedJets->at(ij).eta) > eta_max ) //Highest eta
    {
      eta_max2 = eta_max;
      ie2 = ie1;
      eta_max = fabs(vSelectedJets->at(ij).eta);
      ie1 = ij;
    }
    else if(fabs(vSelectedJets->at(ij).eta) < eta_max && fabs(vSelectedJets->at(ij).eta ) > eta_max2) //2nd Highest eta
    {
      eta_max2 = fabs(vSelectedJets->at(ij).eta);
      ie2 = ij;
    }

    for (int ik=ij+1; ik<vSelectedJets->size(); ik++) //Dijet w/ lowest (m - mW)
    {
      //if (ik==ij) continue; //Pair 2 jets
      if (ik==ib1 || ik==ib2) continue; //Don't take bjets into account
      Pjet1.SetPtEtaPhiE(vSelectedJets->at(ij).pt, vSelectedJets->at(ij).eta, vSelectedJets->at(ij).phi, vSelectedJets->at(ij).energy);
      Pjet2.SetPtEtaPhiE(vSelectedJets->at(ik).pt, vSelectedJets->at(ik).eta, vSelectedJets->at(ik).phi, vSelectedJets->at(ik).energy);

      if (TMath::Abs((Pjet1+Pjet2).M()-80.419)<diffmass_min) //Lowest (m-mW) difference
      {
        ik1=ij;
        ik2=ik;
        diffmass_min = TMath::Abs((Pjet1+Pjet2).M()-80.419);
      }
    }
  }

  //cout<<ij1<<", "<<ij2<<", "<<ik1<<", "<<ik2<<", "<<ie1<<", "<<ie2<<endl;
}






//-------------------------------------------
// ##     ##    ### ########                ## ###      ##     ##    ###    ########
// ###   ###   ##   ##     ##               ##   ##     ##     ##   ## ##   ##     ##
// #### ####  ##    ##     ##               ##    ##    ##     ##  ##   ##  ##     ##
// ## ### ##  ##    ########  #######       ##    ##    ##     ## ##     ## ########
// ##     ##  ##    ##     ##         ##    ##    ##     ##   ##  ######### ##   ##
// ##     ##   ##   ##     ##         ##    ##   ##       ## ##   ##     ## ##    ##  ###
// ##     ##    ### ########           ######  ###         ###    ##     ## ##     ## ###
//-------------------------------------------

// Compute manually the invariant mass of bjet + light jet (either leading or subleading, in case there are 2) in SR
/* NOTE : the last argument determines in which conditions we compute the invariant mass :
- 0 : use leading Jet
- 1 : use subleading Jet
- 2 : require pT>40 for btag and light
- 3 : require pT>50 for btag and light
- 4 : require pT>40 for light only
- 5 : require pT>50 for light only
- 6 : require eta OUTSIDE [2.69;3] for light only
==> Fill different variables
 */
double MEM_NtupleMaker::Compute_mbj(vector<ciemat::Jet>* vSelectedJets, int bjet_index, bool is_tZq_region, int var_choice)
{
  if(!is_tZq_region) {return -1;} //Only compute value in tZq region (single bjet)

  double result=-1;
  int jet_index=-99;

  TLorentzVector b, l;

  //in SR, only 1 bjet
  b.SetPtEtaPhiE(vSelectedJets->at(bjet_index).pt, vSelectedJets->at(bjet_index).eta, vSelectedJets->at(bjet_index).phi, vSelectedJets->at(bjet_index).energy);

  //if only 1 light jet
  if(vSelectedJets->size() == 2)
  {
    if(var_choice == 1) {return -1;} //No subleading jet in case where there's only 1 light jet

    if(bjet_index==0) {l.SetPtEtaPhiE(vSelectedJets->at(1).pt, vSelectedJets->at(1).eta, vSelectedJets->at(1).phi, vSelectedJets->at(1).energy);}
    else if(bjet_index==1) {l.SetPtEtaPhiE(vSelectedJets->at(0).pt, vSelectedJets->at(0).eta, vSelectedJets->at(0).phi, vSelectedJets->at(0).energy);}
    else {cout<<"ERROR : wrong bjet index ? abort ! "<<bjet_index<<endl; return -99;}
  }
  else if(vSelectedJets->size() == 3)
  {
    double pt1=0, pt2=0;

    if(bjet_index==0)
    {
      pt1 = vSelectedJets->at(1).pt; pt2 = vSelectedJets->at(2).pt;

      if(var_choice != 1)
      {
        if(pt1>pt2) {jet_index = 1;}
        else {jet_index = 2;}
      }
      else
      {
        if(pt1>pt2) {jet_index = 2;}
        else {jet_index = 1;}
      }
    }
    else if(bjet_index==1)
    {
      pt1 = vSelectedJets->at(0).pt; pt2 = vSelectedJets->at(2).pt;

      if(var_choice != 1)
      {
        if(pt1>pt2) {jet_index = 0;}
        else {jet_index = 2;}
      }
      else
      {
        if(pt1>pt2) {jet_index = 2;}
        else {jet_index = 0;}
      }
    }
    else if(bjet_index==2)
    {
      pt1 = vSelectedJets->at(0).pt; pt2 = vSelectedJets->at(1).pt;

      if(var_choice !=1)
      {
        if(pt1>pt2) {jet_index = 0;}
        else {jet_index = 1;}
      }
      else
      {
        if(pt1>pt2) {jet_index = 1;}
        else {jet_index = 0;}
      }
    }
    else {cout<<"ERROR : wrong bjet index ? abort ! "<<bjet_index<<endl; return -99;}

    l.SetPtEtaPhiE(vSelectedJets->at(jet_index).pt, vSelectedJets->at(jet_index).eta, vSelectedJets->at(jet_index).phi, vSelectedJets->at(jet_index).energy);
  }
  else {cout<<"ERROR : wrong NJets value in SR ? abort !"<<endl; return -99;}

  if(var_choice==2 && (l.Pt()<40 || b.Pt()<40 ) ) {return -1;}
  else if(var_choice==3 && (l.Pt()<50 || b.Pt()<50 ) ) {return -1;}
  else if(var_choice==4 && l.Pt()<40) {return -1;}
  else if(var_choice==5 && l.Pt()<50) {return -1;}
  else if(var_choice==6 && fabs(l.Eta())>2.69 && fabs(l.Eta())<3 ) {return -1;}

  result = (b+l).M();

  return result;
}







//-------------------------------------------
// ######## ########         ########  ########    ##     ##    ###    ########
//    ##         ##          ##     ##    ##       ##     ##   ## ##   ##     ##
//    ##        ##           ##     ##    ##       ##     ##  ##   ##  ##     ##
//    ##       ##            ########     ##       ##     ## ##     ## ########
//    ##      ##             ##           ##        ##   ##  ######### ##   ##
//    ##     ##              ##           ##         ## ##   ##     ## ##    ##  ###
//    ##    ######## ####### ##           ##          ###    ##     ## ##     ## ###
//-------------------------------------------

/**
 * Compute manually the pT or mass of the top+Z system, from TLorentzVectors, in tZq region events
 * @param  vSelectedElectrons
 * @param  vSelectedMuons
 * @param  vSelectedJets
 * @param  bjet_index         index of bjet (only 1 in tZq region)
 * @param  is_tZq_region
 * @return                    computed value for tZ_pT or tZq_mass
 */
double MEM_NtupleMaker::Compute_tZ_pT(vector<ciemat::Electron>* vSelectedElectrons, vector<ciemat::Muon>* vSelectedMuons, vector<ciemat::Jet>* vSelectedJets, ciemat::MET* METCollection, int bjet_index, bool is_tZq_region, bool return_pT)
{
  if(!is_tZq_region) {return -99;} //Only compute value in tZq region (single bjet)

  double result=-99;

  TLorentzVector lep1, lep2, lep3, bjet, MET;
  TLorentzVector lepZ1, lepZ2, lepW;
  int charge_lep1 = 0;
  int charge_lep2 = 0;
  int charge_lep3 = 0;

  MET.SetPtEtaPhiE( METCollection[0].met, 0, METCollection[0].phi, METCollection[0].met); //neglect neutrino mass
  bjet.SetPtEtaPhiE( vSelectedJets->at(bjet_index).pt, vSelectedJets->at(bjet_index).eta, vSelectedJets->at(bjet_index).phi, vSelectedJets->at(bjet_index).energy);


  //3 muons && 0 electrons
  if(vSelectedMuons->size() == 3 && vSelectedElectrons->size()==0)
  {
    lep1.SetPtEtaPhiM(vSelectedMuons->at(0).pt,     vSelectedMuons->at(0).eta,     vSelectedMuons->at(0).phi, 0.106);
    lep2.SetPtEtaPhiM(vSelectedMuons->at(1).pt,     vSelectedMuons->at(1).eta,     vSelectedMuons->at(1).phi, 0.106);
    lep3.SetPtEtaPhiM(vSelectedMuons->at(2).pt,     vSelectedMuons->at(2).eta,     vSelectedMuons->at(2).phi, 0.106);
    charge_lep1 = vSelectedMuons->at(0).charge ;
    charge_lep2 = vSelectedMuons->at(1).charge ;
    charge_lep3 = vSelectedMuons->at(2).charge ;

    if(charge_lep1 != charge_lep2)
    {
      if(charge_lep1 == charge_lep3)
      {
        if( fabs( (lep1+lep2).M() - 91) < fabs( (lep2+lep3).M() - 91) )
        {
          lepZ1 = lep1;
          lepZ2 = lep2;
          lepW  = lep3;
        }
        else
        {
          lepZ1 = lep3;
          lepZ2 = lep2;
          lepW  = lep1;
        }
      }

      else if (charge_lep2 == charge_lep3)
      {
        if( fabs( (lep1+lep2).M() - 91) < fabs( (lep1+lep3).M() - 91) )
        {
          lepZ1 = lep1;
          lepZ2 = lep2;
          lepW  = lep3;
        }
        else
        {
          lepZ1 = lep3;
          lepZ2 = lep1;
          lepW  = lep2;
        }
      }
    }

    else //charge.lept1 == charge.lept2
    {
      if(charge_lep1 != charge_lep3)
      {
        if( fabs( (lep1+lep3).M() - 91) < fabs( (lep2+lep3).M() - 91) )
        {
          lepZ1 = lep1;
          lepZ2 = lep3;
          lepW  = lep2;
        }
        else
        {
          lepZ1 = lep3;
          lepZ2 = lep2;
          lepW  = lep1;
        }
      }
    }
  }

//0 muons && 3 electrons
  else if(vSelectedMuons->size() == 0 && vSelectedElectrons->size()==3)
  {
    lep1.SetPtEtaPhiM(vSelectedElectrons->at(0).pt, vSelectedElectrons->at(0).eta, vSelectedElectrons->at(0).phi, 511*pow(10,-6));
    lep2.SetPtEtaPhiM(vSelectedElectrons->at(1).pt, vSelectedElectrons->at(1).eta, vSelectedElectrons->at(1).phi, 511*pow(10,-6));
    lep3.SetPtEtaPhiM(vSelectedElectrons->at(2).pt, vSelectedElectrons->at(2).eta, vSelectedElectrons->at(2).phi, 511*pow(10,-6));
    charge_lep1 = vSelectedElectrons->at(0).charge ;
    charge_lep2 = vSelectedElectrons->at(1).charge ;
    charge_lep3 = vSelectedElectrons->at(2).charge ;

    if(charge_lep1 != charge_lep2)
    {
      if(charge_lep1 == charge_lep3)
      {
        if( fabs( (lep1+lep2).M() - 91) < fabs( (lep2+lep3).M() - 91) )
        {
          lepZ1 = lep1;
          lepZ2 = lep2;
          lepW  = lep3;
        }
        else
        {
          lepZ1 = lep3;
          lepZ2 = lep2;
          lepW  = lep1;
        }
      }

      else if (charge_lep2 == charge_lep3)
      {
        if( fabs( (lep1+lep2).M() - 91) < fabs( (lep1+lep3).M() - 91) )
        {
          lepZ1 = lep1;
          lepZ2 = lep2;
          lepW  = lep3;
        }
        else
        {
          lepZ1 = lep3;
          lepZ2 = lep1;
          lepW  = lep2;
        }
      }
    }

    else //charge.lept1 == charge.lept2
    {
      if(charge_lep1 != charge_lep3)
      {
        if( fabs( (lep1+lep3).M() - 91) < fabs( (lep2+lep3).M() - 91) )
        {
          lepZ1 = lep1;
          lepZ2 = lep3;
          lepW  = lep2;
        }
        else
        {
          lepZ1 = lep3;
          lepZ2 = lep2;
          lepW  = lep1;
        }
      }
    }
  }


//1 muon && 2 electrons
  else if(vSelectedElectrons->size() == 2 && vSelectedMuons->size()==1)
  {
    lep1.SetPtEtaPhiM(vSelectedElectrons->at(0).pt, vSelectedElectrons->at(0).eta, vSelectedElectrons->at(0).phi, 511*pow(10,-6));
    lep2.SetPtEtaPhiM(vSelectedElectrons->at(1).pt, vSelectedElectrons->at(1).eta, vSelectedElectrons->at(1).phi, 511*pow(10,-6));
    lep3.SetPtEtaPhiM(vSelectedMuons->at(0).pt, vSelectedMuons->at(0).eta, vSelectedMuons->at(0).phi, 0.106);
    charge_lep1 = vSelectedElectrons->at(0).charge ;
    charge_lep2 = vSelectedElectrons->at(1).charge ;
    charge_lep3 = vSelectedMuons->at(0).charge ;

    if(charge_lep1 != charge_lep2)
    {
      lepZ1 = lep1;
      lepZ2 = lep2;
      lepW  = lep3;
    }
  }

//2 muons && 1 electron
  else if(vSelectedElectrons->size() == 1 && vSelectedMuons->size()==2)
  {
    lep1.SetPtEtaPhiM(vSelectedMuons->at(0).pt, vSelectedMuons->at(0).eta, vSelectedMuons->at(0).phi, 0.106);
    lep2.SetPtEtaPhiM(vSelectedMuons->at(1).pt, vSelectedMuons->at(1).eta, vSelectedMuons->at(1).phi, 0.106);
    lep3.SetPtEtaPhiM(vSelectedElectrons->at(0).pt, vSelectedElectrons->at(0).eta, vSelectedElectrons->at(0).phi, 511*pow(10,-6));
    charge_lep1 = vSelectedMuons->at(0).charge ;
    charge_lep2 = vSelectedMuons->at(1).charge ;
    charge_lep3 = vSelectedElectrons->at(0).charge ;

    if(charge_lep1 != charge_lep2)
    {
      lepZ1 = lep1;
      lepZ2 = lep2;
      lepW  = lep3;
    }
  }




  //---- Create TLorentzVectors for Top and Z
  TLorentzVector Z, top;
  Z.SetPtEtaPhiM( (lepZ1+lepZ2).Pt(), (lepZ1+lepZ2).Eta(), (lepZ1+lepZ2).Phi(), (lepZ1+lepZ2).M() );
  top.SetPtEtaPhiM( (lepW+MET+bjet).Pt(), (lepW+MET+bjet).Eta(), (lepW+MET+bjet).Phi(), (lepW+MET+bjet).M() );

  if(return_pT) result = (Z+top).Pt();
  else result = (Z+top).M();


  return result;
}

























//-------------------------------------------
//  ######  ########  ########    ###    ######## ########       ##    ## ######## ##     ## ########  ##       ########
// ##    ## ##     ## ##         ## ##      ##    ##             ###   ##    ##    ##     ## ##     ## ##       ##
// ##       ##     ## ##        ##   ##     ##    ##             ####  ##    ##    ##     ## ##     ## ##       ##
// ##       ########  ######   ##     ##    ##    ######         ## ## ##    ##    ##     ## ########  ##       ######
// ##       ##   ##   ##       #########    ##    ##             ##  ####    ##    ##     ## ##        ##       ##
// ##    ## ##    ##  ##       ##     ##    ##    ##             ##   ###    ##    ##     ## ##        ##       ##
//  ######  ##     ## ######## ##     ##    ##    ########       ##    ##    ##     #######  ##        ######## ########
//-------------------------------------------

void MEM_NtupleMaker::NtupleMaker(TString samplename)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Creating "<<samplename<<" Ntuple ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;

  int nevents_1mediumBjet = 0; int n_looseBjet_total = 0; int n_mediumBjet_total = 0;

  TString filepath = "./input_ntuples/";
  if(tmp_dir != "") filepath+= tmp_dir + "/";
  filepath+= "FCNCNTuple_" +samplename+ ".root"; //FOR LOCAL EXECUTION

  TFile* f_input = 0;
  f_input = new TFile(filepath.Data());
  if(!f_input || f_input->IsZombie()) {cout<<"Can't find input file !"<<endl; remove( (output_dir + "FCNCNTuple_" + samplename+".root").Data() ); return;} //Erase output file if bug


  for(int itreesyst=0; itreesyst<v_syst_tree.size(); itreesyst++) //Loop on 'tree' systematics (Default, JER, JES)
  {
    //Only 'Default' tree for data and fakes (+'Fakes' syst tree for fakes)
    if( (samplename.Contains("Data") && v_syst_tree[itreesyst] != "Default") || ( samplename.Contains("Fakes") && v_syst_tree[itreesyst] != "Default" && !v_syst_tree[itreesyst].Contains("Fakes") ) ) {continue;}
    if( v_syst_tree[itreesyst].Contains("Fakes") && !samplename.Contains("Fakes") && !samplename.Contains("DY") && !samplename.Contains("TT") ) {continue;} //'Fakes' syst. tree only for Fakes sample

    cout<<FGRN("SYSTEMATIC TREE NAME : ")<<v_syst_tree[itreesyst]<<endl;



//--- Input Trees
    TTree* t_MEM_input = 0; //Tree containing all info necessary for MEM (Jets, leptons, MET, etc.)
    if(v_syst_tree[itreesyst] == "Fakes__plus") t_MEM_input = (TTree*) f_input->Get("MEMInfoPlus");
    else if(v_syst_tree[itreesyst] == "Fakes__minus") t_MEM_input = (TTree*) f_input->Get("MEMInfoMinus");
    else t_MEM_input = (TTree*) f_input->Get("MEMInfo");

    if(t_MEM_input == 0) {cout<<"Can't find tree 'MEMCollection' !"<<endl; return;}

    TTree* t_vars_input = 0; //Tree containing all the BDT vars and more
    t_vars_input = (TTree*) f_input->Get( v_syst_tree[itreesyst] ); if(t_vars_input == 0) {cout<<"Can't find tree '"<<v_syst_tree[itreesyst]<<"' !"<<endl; continue;}



//--- Output Trees
    f_output->cd(); TString output_tree_name = v_syst_tree[itreesyst];
    if(v_syst_tree[itreesyst] == "Default")           output_tree_name = "Tree";


    // else if(v_syst_tree[itreesyst] == "JER__plus")    output_tree_name = "JERUp";
    // else if(v_syst_tree[itreesyst] == "JER__minus")   output_tree_name = "JERDown";
    // else if(v_syst_tree[itreesyst] == "JES__plus")    output_tree_name = "JESUp";
    // else if(v_syst_tree[itreesyst] == "JES__minus")   output_tree_name = "JESDown";
    // else if(v_syst_tree[itreesyst] == "Fakes__plus")  output_tree_name = "FakesUp";
    // else if(v_syst_tree[itreesyst] == "Fakes__minus") output_tree_name = "FakesDown";
    // else {cout<<__LINE__<<BOLD(FRED("Wrong systematic Tree name ! Abort"))<<endl; return;}




    TTree* tree_output = new TTree(output_tree_name.Data(), "");
    Prepare_Tree(samplename, tree_output, output_tree_name);

  //--------------------
  //MEMCollection Tree
    t_MEM_input->SetBranchAddress("SelectedElectron", &vSelectedElectrons);
    t_MEM_input->SetBranchAddress("SelectedMuon",     &vSelectedMuons);

    if(v_syst_tree[itreesyst] == "Default" || v_syst_tree[itreesyst].Contains("Fakes") ) {t_MEM_input->SetBranchAddress("SelectedJet",      &vSelectedJets);}
    else if(v_syst_tree[itreesyst] == "JER__plus") {t_MEM_input->SetBranchAddress("SelectedJetResUp",      &vSelectedJets);}
    else if(v_syst_tree[itreesyst] == "JER__minus") {t_MEM_input->SetBranchAddress("SelectedJetResDw",      &vSelectedJets);}
    else if(v_syst_tree[itreesyst] == "JES__plus") {t_MEM_input->SetBranchAddress("SelectedJetJesUp",      &vSelectedJets);}
    else if(v_syst_tree[itreesyst] == "JES__minus") {t_MEM_input->SetBranchAddress("SelectedJetJesDw",      &vSelectedJets);}
    else {cout<<__LINE__<<BOLD(FRED("Wrong systematic Tree name ! Abort"))<<endl; return;}

    t_MEM_input->SetBranchAddress("METCollection",    &METCollection);

    if(v_syst_tree[itreesyst] == "Default" || v_syst_tree[itreesyst].Contains("Fakes") ) {t_MEM_input->SetBranchAddress("METCollection",    &METCollection);}
    else if(v_syst_tree[itreesyst] == "JER__plus")   {t_MEM_input->SetBranchAddress("METCollectionResUp",    &METCollection);}
    else if(v_syst_tree[itreesyst] == "JER__minus")  {t_MEM_input->SetBranchAddress("METCollectionResDw",    &METCollection);}
    else if(v_syst_tree[itreesyst] == "JES__plus")   {t_MEM_input->SetBranchAddress("METCollectionJesUp",    &METCollection);}
    else if(v_syst_tree[itreesyst] == "JES__minus")  {t_MEM_input->SetBranchAddress("METCollectionJesDw",    &METCollection);}
    else {cout<<BOLD(FRED("Wrong systematic Tree name ! Abort"))<<endl; return;}

  //Default Tree
    //---Other
    t_vars_input->SetBranchAddress("Weight", &Weight);
    t_vars_input->SetBranchAddress("Channel", &Channel);
    t_vars_input->SetBranchAddress("mTW", &mTW);
    t_vars_input->SetBranchAddress("NJets", &NJets);
    t_vars_input->SetBranchAddress("NBJets", &NBJets);
    t_vars_input->SetBranchAddress("EvtNr", &EvtNr);
    t_vars_input->SetBranchAddress("RunNr", &RunNr);
    // t_vars_input->SetBranchAddress("AdditionalMuonIso", &AdditionalMuonIso);
    // t_vars_input->SetBranchAddress("AdditionalEleIso", &AdditionalEleIso);

    //---BDT
    for(int ivar=0; ivar<BDTvar_list.size(); ivar++)
    {
      t_vars_input->SetBranchAddress(BDTvar_list[ivar].Data(), &BDTvar_floats[ivar]);
    }
    //---Systematics
    if(v_syst_tree[itreesyst] == "Default" && samplename != "Data" && samplename != "Fakes")
    {
      for(int isyst=0; isyst<v_syst_weight.size(); isyst++)
      {
        t_vars_input->SetBranchAddress(v_syst_weight[isyst].Data(), &v_syst_float[isyst]);
      }
    }

    // float passTrig = -999; //Needed to keep only good events

    // TTree* t_passTrig = 0;

    // if(samplename != "Data")
    // {
    //   if(v_syst_tree[itreesyst] == "Default") {t_vars_input->SetBranchAddress("passTrig", &passTrig);}
    //   else
    //   {
    //     t_passTrig = (TTree*) f_input->Get( "Default" ); if(t_passTrig == 0) {cout<<"Can't find tree 'Default' !"<<endl; return;}
    //     t_passTrig->SetBranchAddress("passTrig", &passTrig);
    //   }
    // }



  //--------------------
  //--- EVENT LOOP

    int nentries = t_MEM_input->GetEntries();

    if(t_vars_input->GetEntries() != nentries) {cout<<FRED("WARNING  : different nof events in trees !!")<<endl; return;}

    for(int ientry=0; ientry<nentries; ientry++)
    {
      if(ientry%10000==0) {cout << "Number of processed events : " << ientry << "/" << nentries<< std::endl;}

      Init(); //Re-initialize values of all variables

      // passTrig = -999;

      t_MEM_input->GetEntry(ientry);
      t_vars_input->GetEntry(ientry);
      if(samplename != "Data")
      {
        // if(v_syst_tree[itreesyst] != "Default") t_passTrig->GetEntry(ientry);

        // if(passTrig != 1) {continue;}
      }



  //---------------------------
      //Translate channel in TString (easier to debug)
      TString thechannel = "";
      if(Channel == 0) {thechannel="uuu";}
      else if(Channel == 1) {thechannel="uue";}
      else if(Channel == 2) {thechannel="eeu";}
      else if(Channel == 3) {thechannel="eee";}
      else {cout<<endl<<BOLD( FRED("Wrong channel value !") )<<endl; continue;}

  //---------------------------

      int n_mediumBjet = 0;


      //NOTE : Compare njets values -- make sure I use the same def. as Mara
      int NJets_tmp=vSelectedJets->size();
      int NBJets_tmp = 0; //NB : NJets contains both btag and non-btag
      for(int i=0; i<vSelectedJets->size(); i++)
      {
        // if( fabs(vSelectedJets->at(i).eta) <= eta_threshold && vSelectedJets->at(i).btagCSV >= 0.8484) {NBJets_tmp++;} //Medium btag WP
        if( fabs(vSelectedJets->at(i).eta) <= eta_threshold && vSelectedJets->at(i).btagCSV >= CSV_threshold) {NBJets_tmp++;}

        if( fabs(vSelectedJets->at(i).eta) <= eta_threshold && vSelectedJets->at(i).btagCSV >= 0.5426) {n_looseBjet_total++;}
        if( fabs(vSelectedJets->at(i).eta) <= eta_threshold && vSelectedJets->at(i).btagCSV >= 0.8484) {n_mediumBjet++; n_mediumBjet_total++;}
      }

      if(n_mediumBjet==1 && NJets_tmp > 1 && NJets_tmp < 4) nevents_1mediumBjet++;

      if(NJets_tmp != NJets || NBJets_tmp != NBJets)
      {
        cout<<"Disagreement b/w NJets definitions"<<endl;
        cout<<"NJets "<<NJets<<" NJets_tmp "<<NJets_tmp<<" vSelectedJets "<<vSelectedJets->size()<<endl;
        cout<<"NBJets "<<NBJets<<" NBJets_tmp "<<NBJets_tmp<<endl;
        for(int ijet=0; ijet<vSelectedJets->size(); ijet++)
        {
          cout<<vSelectedJets->at(ijet).eta<<" "<<vSelectedJets->at(ijet).btagCSV<<endl;
        }      cout<<endl<<endl<<endl;

        cout<<"--- WRONG BJET DEF ! ABORT"<<endl; return;
      }



  //---------------------------
      //Check Jets numbers

      //--- if want to change btag WP ---
      // if( NBJets_tmp == 1 && NJets > 1 && NJets < 4 ) {is_3l_TZQ_SR = true;}
      // if( NBJets_tmp > 1 && NJets > 1 ) {is_3l_TTZ_CR = true;}
      // if( NBJets_tmp == 0 && NJets > 0 ) {is_3l_WZ_CR = true;}
      // if( NBJets_tmp == 1 && NJets > 0 && NJets < 4 ) {is_3l_1BTAG_SR = true;} //SR + 0light jet events

      if( NBJets == 1 && NJets > 1 && NJets < 4 ) {is_3l_TZQ_SR = true;}
      if( NBJets > 1 && NJets > 1 ) {is_3l_TTZ_CR = true;}
      if( NBJets == 0 && NJets > 0 ) {is_3l_WZ_CR = true;}
      if( NBJets == 1 && NJets > 0 && NJets < 4 ) {is_3l_1BTAG_SR = true;} //SR + 0light jet events

      if(region=="MEM" && !is_3l_TZQ_SR && !is_3l_TTZ_CR) {continue;}
      else if(region=="WZ" && !is_3l_WZ_CR) {continue;}
      else if(region=="1btag" && !is_3l_1BTAG_SR) {continue;}


  //---------------------------
      //Order leptons by highest Pt & Fill TLorentzVectors
      OrderLeptons(thechannel, vSelectedMuons, vSelectedElectrons);

      Float_t lept252525=-999, lept202020=-999, lept202015=-999, lept201515=-999, lept252015=-999, leadingLeptonPT=-999, secondLeptonPT=-999, thirdLeptonPT=-999;
      if(multilepton_Lepton1_P4.Pt() > 25 && multilepton_Lepton2_P4.Pt() > 25 && multilepton_Lepton3_P4.Pt() > 25) {lept252525 = 1;}
      else {lept252525 = 0;}

      if(multilepton_Lepton1_P4.Pt() > 20 && multilepton_Lepton2_P4.Pt() > 20 && multilepton_Lepton3_P4.Pt() > 20) {lept202020 = 1;}
      else {lept202020 = 0;}

      if(multilepton_Lepton1_P4.Pt() > 20 && multilepton_Lepton2_P4.Pt() > 20 && multilepton_Lepton3_P4.Pt() > 15) {lept202015 = 1;}
      else {lept202015 = 0;}

      if(multilepton_Lepton1_P4.Pt() > 20 && multilepton_Lepton2_P4.Pt() > 15 && multilepton_Lepton3_P4.Pt() > 15) {lept201515 = 1;}
      else {lept201515 = 0;}

      if(multilepton_Lepton1_P4.Pt() > 25 && multilepton_Lepton2_P4.Pt() > 20 && multilepton_Lepton3_P4.Pt() > 15) {lept252015 = 1;}
      else {lept252015 = 0;}

      leadingLeptonPT = multilepton_Lepton1_P4.Pt(); secondLeptonPT = multilepton_Lepton2_P4.Pt(); thirdLeptonPT = multilepton_Lepton3_P4.Pt();

  //---------------------------

      std::vector<Float_t> vSelectedJets_CSV; std::vector<Int_t> vSelectedJets_id; //Easier storage of jet Ids and CSVs
      for(int i=0; i<vSelectedJets->size(); i++)
      {
        vSelectedJets_CSV.push_back(vSelectedJets->at(i).btagCSV);
      }
      for(int i=0; i<vSelectedJets->size(); i++)
      {
        vSelectedJets_id.push_back(vSelectedJets->at(i).hadronFlavour);
      }

      //--- Choosing 2 b-jets
      bool doSelectOnlyBjets = true; //Ask CSV>0.46 & eta<=2.4
      int ib1=-1, ib2=-1; //index of bjets

      SelectBjets(vSelectedJets, "HighestBtagDiscrim", ib1, ib2, doSelectOnlyBjets);


      //Fill TLorentzVector
      if (ib1!=-1) {multilepton_Bjet1_Id = vSelectedJets_id[ib1] ; multilepton_Bjet1_CSV = vSelectedJets_CSV[ib1]; multilepton_Bjet1_P4.SetPtEtaPhiE(vSelectedJets->at(ib1).pt, vSelectedJets->at(ib1).eta, vSelectedJets->at(ib1).phi, vSelectedJets->at(ib1).energy);}
      if (ib2!=-1) {multilepton_Bjet2_Id = vSelectedJets_id[ib2]; multilepton_Bjet2_CSV = vSelectedJets_CSV[ib2]; multilepton_Bjet2_P4.SetPtEtaPhiE(vSelectedJets->at(ib2).pt, vSelectedJets->at(ib2).eta, vSelectedJets->at(ib2).phi, vSelectedJets->at(ib2).energy);}

      //--- Choosing Highet Pt, 2nd Highest Pt, Highest Eta jets, & dijet pair w/ mass closest to W mass
      int ij1=-1, ij2=-1, ik1=-1, ik2=-1, ie1=-1, ie2=-1;
      OrderJets(vSelectedJets, ib1, ib2, ij1, ij2, ik1, ik2, ie1, ie2);
      //Fill TLorentzVector
      if(ij1!=-1 && ij2==-1) //1jet
      {
        multilepton_JetHighestPt1_Id = vSelectedJets_id[ij1]; multilepton_JetHighestPt1_CSV = vSelectedJets_CSV[ij1]; multilepton_JetHighestPt1_P4.SetPtEtaPhiE(vSelectedJets->at(ij1).pt, vSelectedJets->at(ij1).eta, vSelectedJets->at(ij1).phi, vSelectedJets->at(ij1).energy);
      }
      else if(ij1!=-1 && ij2!=-1) //2jets
      {
        multilepton_JetHighestPt1_Id = vSelectedJets_id[ij1]; multilepton_JetHighestPt1_CSV = vSelectedJets_CSV[ij1]; multilepton_JetHighestPt1_P4.SetPtEtaPhiE(vSelectedJets->at(ij1).pt, vSelectedJets->at(ij1).eta, vSelectedJets->at(ij1).phi, vSelectedJets->at(ij1).energy);
        multilepton_JetHighestPt2_Id = vSelectedJets_id[ij2]; multilepton_JetHighestPt2_CSV = vSelectedJets_CSV[ij2]; multilepton_JetHighestPt2_P4.SetPtEtaPhiE(vSelectedJets->at(ij2).pt, vSelectedJets->at(ij2).eta, vSelectedJets->at(ij2).phi, vSelectedJets->at(ij2).energy);
      }

      if(ie1!=-1 && ie2==-1) //1jets
      {
        multilepton_JetHighestEta1_Id = vSelectedJets_id[ie1]; multilepton_JetHighestEta1_CSV = vSelectedJets_CSV[ie1]; multilepton_JetHighestEta1_P4.SetPtEtaPhiE(vSelectedJets->at(ie1).pt, vSelectedJets->at(ie1).eta, vSelectedJets->at(ie1).phi, vSelectedJets->at(ie1).energy);
      }
      else if(ie1!=-1 && ie2!=-1) //2jets
      {
        multilepton_JetHighestEta1_Id = vSelectedJets_id[ie1]; multilepton_JetHighestEta1_CSV = vSelectedJets_CSV[ie1]; multilepton_JetHighestEta1_P4.SetPtEtaPhiE(vSelectedJets->at(ie1).pt, vSelectedJets->at(ie1).eta, vSelectedJets->at(ie1).phi, vSelectedJets->at(ie1).energy);
        multilepton_JetHighestEta2_Id = vSelectedJets_id[ie2]; multilepton_JetHighestEta2_CSV = vSelectedJets_CSV[ie2]; multilepton_JetHighestEta2_P4.SetPtEtaPhiE(vSelectedJets->at(ie2).pt, vSelectedJets->at(ie2).eta, vSelectedJets->at(ie2).phi, vSelectedJets->at(ie2).energy);
      }

      if(ik1!=-1 && ik2!=-1) //2jets (pair)
      {
        multilepton_JetClosestMw1_Id = vSelectedJets_id[ik1]; multilepton_JetClosestMw1_CSV = vSelectedJets_CSV[ik1]; multilepton_JetClosestMw1_P4.SetPtEtaPhiE(vSelectedJets->at(ik1).pt, vSelectedJets->at(ik1).eta, vSelectedJets->at(ik1).phi, vSelectedJets->at(ik1).energy);
        multilepton_JetClosestMw2_Id = vSelectedJets_id[ik2]; multilepton_JetClosestMw2_CSV = vSelectedJets_CSV[ik2]; multilepton_JetClosestMw2_P4.SetPtEtaPhiE(vSelectedJets->at(ik2).pt, vSelectedJets->at(ik2).eta, vSelectedJets->at(ik2).phi, vSelectedJets->at(ik2).energy);
      }

  //---------------------------
      //OTHER VARIABLES

      //--- catJets (3l)
      if      (ib1!=-1 && ib2!=-1 && vSelectedJets->size()-2>=2) catJets = kCat_3l_2b_2j; //0
      else if (ib1!=-1 && ib2==-1 && vSelectedJets->size()-1>=2) catJets = kCat_3l_1b_2j; //1
      else if (ib1!=-1 && ib2!=-1 && vSelectedJets->size()-2==1) catJets = kCat_3l_2b_1j; //2
      else if (ib1!=-1 && ib2==-1 && vSelectedJets->size()-1==1) catJets = kCat_3l_1b_1j; //3
      else if (ib1!=-1 && ib2!=-1 && vSelectedJets->size()-2==0) catJets = kCat_3l_2b_0j; //4
      else catJets = -1;

  //--------------------------------
      multilepton_mET.SetPtEtaPhiE( METCollection[0].met, 0, METCollection[0].phi, METCollection[0].met);
      multilepton_mHT = METCollection[0].sumEt;

  //-------------------

      if (multilepton_Lepton1_Id!=-999 && multilepton_Lepton2_Id!=-999 && multilepton_Lepton3_Id!=-999)
      {
        if (multilepton_Lepton1_Id*multilepton_Lepton2_Id>0 && multilepton_Lepton2_Id*multilepton_Lepton3_Id>0) {mc_ttZhypAllowed =-1;}
        else if ( (multilepton_Lepton1_Id==-multilepton_Lepton2_Id)
        || (multilepton_Lepton1_Id==-multilepton_Lepton3_Id)
        || (multilepton_Lepton2_Id==-multilepton_Lepton3_Id) )
        {
          mc_ttZhypAllowed = 1;
        }
      }

  //-------------------
  //  #    # ###### #    #    #    #   ##   #####  #   ##   #####  #      ######  ####
  //  ##   # #      #    #    #    #  #  #  #    # #  #  #  #    # #      #      #
  //  # #  # #####  #    #    #    # #    # #    # # #    # #####  #      #####   ####
  //  #  # # #      # ## #    #    # ###### #####  # ###### #    # #      #           #
  //  #   ## #      ##  ##     #  #  #    # #   #  # #    # #    # #      #      #    #
  //  #    # ###### #    #      ##   #    # #    # # #    # #####  ###### ######  ####
  //-------------------

      //Can compute here some new variables
      Float_t LeadingJetNonB_pT=-999, SecondJetNonB_pT=-999;
      // Float_t ContainsBadJet=0;
      double pT1=-1, pT2=-1;
      for(int ijet=0; ijet<vSelectedJets->size(); ijet++)
      {
        // if(vSelectedJets->at(ijet).pt < 50 && fabs(vSelectedJets->at(ijet).eta) > 2.69 && fabs(vSelectedJets->at(ijet).eta) < 3 ) {ContainsBadJet = 1;} //Loop on all jets

        if(ijet == ib1 || ijet == ib2) {continue;} //don't consider bjets for these vars

        if(vSelectedJets->at(ijet).pt > pT1)
        {
          pT2 = pT1;
          pT1 = vSelectedJets->at(ijet).pt;
        }
        else if(vSelectedJets->at(ijet).pt < pT1 && vSelectedJets->at(ijet).pt > pT2)
        {
          pT2 = vSelectedJets->at(ijet).pt;
        }
      }
      LeadingJetNonB_pT = pT1; SecondJetNonB_pT = pT2;








  //-------------------
      for(int ivar=0; ivar<v_variablesToCreate.size(); ivar++) //FIXME -- define here new variables
      {
        if(v_variablesToCreate[ivar] == "tZ_pT") {v_variablesToCreate_float[ivar] = Compute_tZ_pT(vSelectedElectrons, vSelectedMuons, vSelectedJets, METCollection, ib1, is_3l_TZQ_SR, true);}
        else if(v_variablesToCreate[ivar] == "tZ_mass") {v_variablesToCreate_float[ivar] = Compute_tZ_pT(vSelectedElectrons, vSelectedMuons, vSelectedJets, METCollection, ib1, is_3l_TZQ_SR, false);}
        else if(v_variablesToCreate[ivar] == "bj_mass_leadingJet") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 0);}
        else if(v_variablesToCreate[ivar] == "bj_mass_subleadingJet") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 1);}
        else if(v_variablesToCreate[ivar] == "bj_mass_leadingJet_pT40") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 2);}
        else if(v_variablesToCreate[ivar] == "bj_mass_leadingJet_pT50") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 3);}
        else if(v_variablesToCreate[ivar] == "bj_mass_leadingJet_pTlight40") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 4);}
        else if(v_variablesToCreate[ivar] == "bj_mass_leadingJet_pTlight50") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 5);}
        else if(v_variablesToCreate[ivar] == "bj_mass_leadingJet_etaCut") {v_variablesToCreate_float[ivar] = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 6);}
        else if(v_variablesToCreate[ivar] == "LeadingJetNonB_pT") {v_variablesToCreate_float[ivar] = LeadingJetNonB_pT;}
        else if(v_variablesToCreate[ivar] == "SecondJetNonB_pT") {v_variablesToCreate_float[ivar] = SecondJetNonB_pT;}
        else if(v_variablesToCreate[ivar] == "lept252525") {v_variablesToCreate_float[ivar] = lept252525;}
        else if(v_variablesToCreate[ivar] == "lept202020") {v_variablesToCreate_float[ivar] = lept202020;}
        else if(v_variablesToCreate[ivar] == "lept202015") {v_variablesToCreate_float[ivar] = lept202015;}
        else if(v_variablesToCreate[ivar] == "lept201515") {v_variablesToCreate_float[ivar] = lept201515;}
        else if(v_variablesToCreate[ivar] == "lept252015") {v_variablesToCreate_float[ivar] = lept252015;}
        else if(v_variablesToCreate[ivar] == "leadingLeptonPT") {v_variablesToCreate_float[ivar] = leadingLeptonPT;}
        else if(v_variablesToCreate[ivar] == "secondLeptonPT") {v_variablesToCreate_float[ivar] = secondLeptonPT;}
        else if(v_variablesToCreate[ivar] == "thirdLeptonPT") {v_variablesToCreate_float[ivar] = thirdLeptonPT;}
      }


      // tZ_pT = Compute_tZ_pT(vSelectedElectrons, vSelectedMuons, vSelectedJets, METCollection, ib1, is_3l_TZQ_SR, true);
      // tZ_mass = Compute_tZ_pT(vSelectedElectrons, vSelectedMuons, vSelectedJets, METCollection, ib1, is_3l_TZQ_SR, false);
      // bj_mass_leadingJet = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 0);
      // bj_mass_subleadingJet = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 1);
      // bj_mass_leadingJet_pT40 = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 2);
      // bj_mass_leadingJet_pT50 = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 3);
      // bj_mass_leadingJet_pTlight40 = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 4);
      // bj_mass_leadingJet_pTlight50 = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 5);
      // bj_mass_leadingJet_etaCut = Compute_mbj(vSelectedJets, ib1, is_3l_TZQ_SR, 6);


  //-------------------

      tree_output->Fill();
    }

    // cout<<"----------"<<endl;
    // cout<<"Number of SR events with medium WP : "<<nevents_1mediumBjet<<endl;
    // cout<<"Number of bjets with loose WP : "<<n_looseBjet_total<<endl;
    // cout<<"Number of bjets with medium WP : "<<n_mediumBjet_total<<endl;
    // cout<<"----------"<<endl;


    f_output->cd();
    tree_output->Write();

    // if(v_syst_tree[itreesyst] != "Default") {delete t_passTrig;}

    t_MEM_input->Delete(); t_vars_input->Delete(); tree_output->Delete();
  }//end tree_syst Loop


  delete f_input;

  return ;
}









//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
// ##     ##       ###       ####    ##    ##
// ###   ###      ## ##       ##     ###   ##
// #### ####     ##   ##      ##     ####  ##
// ## ### ##    ##     ##     ##     ## ## ##
// ##     ##    #########     ##     ##  ####
// ##     ##    ##     ##     ##     ##   ###
// ##     ##    ##     ##    ####    ##    ##
//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
//-------------------------------------------




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

  vector<TString> v_samplenames;
  v_samplenames.push_back("Data");
  v_samplenames.push_back("tZqmcNLO");
  v_samplenames.push_back("tZqQup");
  v_samplenames.push_back("tZqQdw");
  v_samplenames.push_back("WZB");
  v_samplenames.push_back("WZL");
  v_samplenames.push_back("WZC");
  v_samplenames.push_back("ttZ");
  v_samplenames.push_back("ZZ");
  v_samplenames.push_back("ttH");
  v_samplenames.push_back("ttW");
  v_samplenames.push_back("STtWll");
  v_samplenames.push_back("FakesElectron");
  v_samplenames.push_back("FakesMuon");

  // v_samplenames.push_back("DY");
  // v_samplenames.push_back("TT");
  // v_samplenames.push_back("FakesNewNew");
  // v_samplenames.push_back("Fakes");
  // v_samplenames.push_back("tZqhwpp");



//---------------------------------------------------------------------------
// ########  ########  ########       ##     ##    ###    ########   ######
// ##     ## ##     ##    ##          ##     ##   ## ##   ##     ## ##    ##
// ##     ## ##     ##    ##          ##     ##  ##   ##  ##     ## ##
// ########  ##     ##    ##          ##     ## ##     ## ########   ######
// ##     ## ##     ##    ##           ##   ##  ######### ##   ##         ##
// ##     ## ##     ##    ##            ## ##   ##     ## ##    ##  ##    ##
// ########  ########     ##             ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
//NOTE : Channel, Weight, mTW, NJets, NBJets are added manually in the code

//NOTE : make sure the variable list is up-to-date compared to Ntuples' contents !!

  vector<TString> BDTvar_list;
  BDTvar_list.push_back("btagDiscri");
  BDTvar_list.push_back("dRAddLepQ");
  BDTvar_list.push_back("dRAddLepClosestJet");
  BDTvar_list.push_back("dPhiAddLepB");
  BDTvar_list.push_back("ZEta");
  BDTvar_list.push_back("Zpt");
  BDTvar_list.push_back("mtop");
  BDTvar_list.push_back("AddLepAsym");
  BDTvar_list.push_back("etaQ");
  BDTvar_list.push_back("AddLepETA");
  BDTvar_list.push_back("LeadJetEta");
  BDTvar_list.push_back("dPhiZAddLep");
  BDTvar_list.push_back("dRZAddLep");
  BDTvar_list.push_back("dRjj");
  BDTvar_list.push_back("ptQ");
  BDTvar_list.push_back("tZq_pT");
  BDTvar_list.push_back("dRAddLepB");
  BDTvar_list.push_back("dRZTop");
  BDTvar_list.push_back("TopPT");
  BDTvar_list.push_back("m3l");
  BDTvar_list.push_back("METpt");
  BDTvar_list.push_back("LeadingJetCSV");
  BDTvar_list.push_back("SecondJetCSV");
  BDTvar_list.push_back("AdditionalMuonIso");
  BDTvar_list.push_back("AdditionalEleIso");
  BDTvar_list.push_back("ContainsBadJet");
  BDTvar_list.push_back("fourthLep10");
  BDTvar_list.push_back("fourthLep15");
  BDTvar_list.push_back("badMuon");
  BDTvar_list.push_back("LeadJetPT");



//---------------------------------------------------------------------------
 //                                                 #####
 // #    #   ##   #####   ####     #####  ####     #     # #####  ######   ##   ##### ######
 // #    #  #  #  #    # #           #   #    #    #       #    # #       #  #    #   #
 // #    # #    # #    #  ####       #   #    #    #       #    # #####  #    #   #   #####
 // #    # ###### #####       #      #   #    #    #       #####  #      ######   #   #
 //  #  #  #    # #   #  #    #      #   #    #    #     # #   #  #      #    #   #   #
 //   ##   #    # #    #  ####       #    ####      #####  #    # ###### #    #   #   ######
//---------------------------------------------------------------------------

  vector<TString> v_variablesToCreate;
  v_variablesToCreate.push_back("tZ_pT");
  v_variablesToCreate.push_back("tZ_mass");
  v_variablesToCreate.push_back("bj_mass_leadingJet");
  v_variablesToCreate.push_back("bj_mass_subleadingJet");
  v_variablesToCreate.push_back("bj_mass_leadingJet_pT40");
  v_variablesToCreate.push_back("bj_mass_leadingJet_pT50");
  v_variablesToCreate.push_back("bj_mass_leadingJet_pTlight40");
  v_variablesToCreate.push_back("bj_mass_leadingJet_pTlight50");
  v_variablesToCreate.push_back("bj_mass_leadingJet_etaCut");
  v_variablesToCreate.push_back("LeadingJetNonB_pT");
  v_variablesToCreate.push_back("SecondJetNonB_pT");

  v_variablesToCreate.push_back("lept252525");
  v_variablesToCreate.push_back("lept202020");
  v_variablesToCreate.push_back("lept202015");
  v_variablesToCreate.push_back("lept201515");
  v_variablesToCreate.push_back("lept252015");
  v_variablesToCreate.push_back("leadingLeptonPT");
  v_variablesToCreate.push_back("secondLeptonPT");
  v_variablesToCreate.push_back("thirdLeptonPT");

  // v_variablesToCreate.push_back("ContainsBadJet");


//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------

  vector<TString> tree_syst_list;
  tree_syst_list.push_back("Default"); //NOTE -- KEEP THIS LINE

//These systematics are stored in different TTrees (not simple weights)
  tree_syst_list.push_back("JES__plus"); tree_syst_list.push_back("JES__minus");
  tree_syst_list.push_back("JER__plus"); tree_syst_list.push_back("JER__minus");
  tree_syst_list.push_back("Fakes__plus"); tree_syst_list.push_back("Fakes__minus");

//----------------
  vector<TString> weight_syst_list;
  weight_syst_list.push_back("Q2__plus"); weight_syst_list.push_back("Q2__minus");
  weight_syst_list.push_back("PU__plus"); weight_syst_list.push_back("PU__minus");
  weight_syst_list.push_back("MuEff__plus"); weight_syst_list.push_back("MuEff__minus");
  weight_syst_list.push_back("EleEff__plus"); weight_syst_list.push_back("EleEff__minus");
  weight_syst_list.push_back("LFcont__plus"); weight_syst_list.push_back("LFcont__minus");
  weight_syst_list.push_back("HFstats1__plus"); weight_syst_list.push_back("HFstats1__minus");
  weight_syst_list.push_back("HFstats2__plus"); weight_syst_list.push_back("HFstats2__minus");
  weight_syst_list.push_back("CFerr1__plus"); weight_syst_list.push_back("CFerr1__minus");
  weight_syst_list.push_back("CFerr2__plus"); weight_syst_list.push_back("CFerr2__minus");
  weight_syst_list.push_back("HFcont__plus"); weight_syst_list.push_back("HFcont__minus");
  weight_syst_list.push_back("LFstats1__plus"); weight_syst_list.push_back("LFstats1__minus");
  weight_syst_list.push_back("LFstats2__plus"); weight_syst_list.push_back("LFstats2__minus");
  weight_syst_list.push_back("pdf__plus"); weight_syst_list.push_back("pdf__minus");
  weight_syst_list.push_back("Weight_noTag"); weight_syst_list.push_back("Weight_noPU");


//---------------------------------------------------------------------------
// ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##        ######     ###    ##       ##        ######
// ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ##       ##    ##   ## ##   ##       ##       ##    ##
// ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ##       ##        ##   ##  ##       ##       ##
// ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##       ##       ##     ## ##       ##        ######
// ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##       ######### ##       ##             ##
// ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ###       ##    ## ##     ## ##       ##       ##    ##
// ##        #######  ##    ##  ######     ##    ####  #######  ##    ##        ######  ##     ## ######## ########  ######
//---------------------------------------------------------------------------
  //Need to differenciate ttZ/tZq & WZ, since MEM can't run in WZ region (not enough jets) ==> Different ntuples
  TString region_choice; //Choose if produce samples which are going to be used for MEM or for WZ CR

  //Bjet definition -- Needed to select btagged jets in Jet vector ! Make sure it's updated !!
  double CSV_threshold = 0.5426; //Loose CSV
  // double CSV_threshold = 0.8484; //Medium CSV

  double eta_threshold = 2.4 ; //No Bjet SF beyond that

  bool do_MEM_regions = true;
  bool do_WZ_region = true;
  bool do_1btag_region = false;

  // TString specific_dir = "ntuples_lowpT"; //specify specific input/output dir. name
  TString specific_dir = ""; //specify specific input/output dir. name



//--- Produce ntuples for MEM (ttZ/tZq regions)
  if(do_MEM_regions)
  {
    region_choice = "MEM";
    for(int isample=0; isample<v_samplenames.size(); isample++)
    {
      MEM_NtupleMaker* theNtupleMaker = new MEM_NtupleMaker(v_samplenames[isample], BDTvar_list, v_variablesToCreate, weight_syst_list, tree_syst_list, region_choice, CSV_threshold, eta_threshold, specific_dir);
      theNtupleMaker->Init();
      theNtupleMaker->NtupleMaker(v_samplenames[isample]);
      theNtupleMaker->~MEM_NtupleMaker();
    }
  }

//--- Produce ntuples for WZ CR study (mTW template fit)
  if(do_WZ_region)
  {
    region_choice = "WZ";
    for(int isample=0; isample<v_samplenames.size(); isample++)
    {
      MEM_NtupleMaker* theNtupleMaker = new MEM_NtupleMaker(v_samplenames[isample], BDTvar_list, v_variablesToCreate, weight_syst_list, tree_syst_list, region_choice, CSV_threshold, eta_threshold, specific_dir);
      theNtupleMaker->Init();
      theNtupleMaker->NtupleMaker(v_samplenames[isample]);
      theNtupleMaker->~MEM_NtupleMaker();
    }
  }

//--- Produce ntuples for WZ CR study (mTW template fit)
  if(do_1btag_region)
  {
    region_choice = "1btag";
    for(int isample=0; isample<v_samplenames.size(); isample++)
    {
      MEM_NtupleMaker* theNtupleMaker = new MEM_NtupleMaker(v_samplenames[isample], BDTvar_list, v_variablesToCreate, weight_syst_list, tree_syst_list, region_choice, CSV_threshold, eta_threshold, specific_dir);
      theNtupleMaker->Init();
      theNtupleMaker->NtupleMaker(v_samplenames[isample]);
      theNtupleMaker->~MEM_NtupleMaker();
    }
  }


	return 0;
}
