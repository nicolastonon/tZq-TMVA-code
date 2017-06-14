#ifndef MEM_H
#define MEM_H

/* BASH COLORS */
#define RST  "\x1B[0m"
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

#include <vector>
#include "TString.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "MuTree.h" //Contains def. of user-classes for Jet, MET, El, Mu, etc. (CIEMAT)

using namespace std;

class MEM_NtupleMaker
{
	public :

		MEM_NtupleMaker(TString, vector<TString>, vector<TString>, vector<TString>, TString, double, double, TString);
		~MEM_NtupleMaker();

		void Init();
		void Prepare_Tree(TString, TTree*, TString);
		void SelectBjets(vector<ciemat::Jet>*, string, int&, int&, bool);
		void OrderLeptons(TString, vector<ciemat::Muon>*, vector<ciemat::Electron>*);
		void OrderJets(vector<ciemat::Jet>*, const int, const int, int&, int&, int&, int&, int&, int&);
		void NtupleMaker(TString);

		TString MEM_or_WZ;
		double CSV_threshold; double eta_threshold;

		TString output_dir;
		TFile* f_output;
		// TTree* tree_output;

		TString tmp_dir; //specific name of input/output dirs (e.g. 'mediumBtag')

	    vector<ciemat::Muon>*     vSelectedMuons;
	    vector<ciemat::Electron>* vSelectedElectrons;
	    vector<ciemat::Jet>*      vSelectedJets;
	    ciemat::MET*              METCollection;

		Int_t catJets;
		Char_t is_3l_TTZ_CR;
		Char_t is_3l_WZ_CR;
		Char_t is_3l_TZQ_SR;

		Int_t mc_ttZhypAllowed;

		//Leptons
		Int_t 			multilepton_Lepton1_Id, multilepton_Lepton2_Id, multilepton_Lepton3_Id, multilepton_Lepton4_Id;
		TLorentzVector 	multilepton_Lepton1_P4, multilepton_Lepton2_P4, multilepton_Lepton3_P4, multilepton_Lepton4_P4;

		//b-jet
		Int_t 			multilepton_Bjet1_Id, multilepton_Bjet2_Id;
		TLorentzVector 	multilepton_Bjet1_P4, multilepton_Bjet2_P4;
		Float_t			multilepton_Bjet1_CSV, multilepton_Bjet2_CSV;

		//jet
		Int_t 			multilepton_JetHighestPt1_Id, multilepton_JetHighestPt2_Id, multilepton_JetClosestMw1_Id, multilepton_JetClosestMw2_Id, multilepton_JetHighestEta1_Id, multilepton_JetHighestEta2_Id;
		TLorentzVector 	multilepton_JetHighestPt1_P4, multilepton_JetHighestPt2_P4, multilepton_JetClosestMw1_P4, multilepton_JetClosestMw2_P4, multilepton_JetHighestEta1_P4, multilepton_JetHighestEta2_P4;
		Float_t 		multilepton_JetHighestPt1_CSV, multilepton_JetHighestPt2_CSV, multilepton_JetClosestMw1_CSV, multilepton_JetClosestMw2_CSV, multilepton_JetHighestEta1_CSV, multilepton_JetHighestEta2_CSV;

//---------------------
		//Not used in tZq analysis, only for practical reasons (ttH format)
		Int_t	 		multilepton_JetHighestPt1_2ndPair_Id, multilepton_JetHighestPt2_2ndPair_Id, multilepton_JetClosestMw1_2ndPair_Id, multilepton_JetClosestMw2_2ndPair_Id, multilepton_JetLowestMjj1_Id, multilepton_JetLowestMjj2_Id, multilepton_JetLowestMjj1_2ndPair_Id, multilepton_JetLowestMjj2_2ndPair_Id;
		TLorentzVector	multilepton_JetHighestPt1_2ndPair_P4, multilepton_JetHighestPt2_2ndPair_P4, multilepton_JetClosestMw1_2ndPair_P4, multilepton_JetClosestMw2_2ndPair_P4, multilepton_JetLowestMjj1_P4, multilepton_JetLowestMjj2_P4, multilepton_JetLowestMjj1_2ndPair_P4, multilepton_JetLowestMjj2_2ndPair_P4;
		Float_t 		multilepton_JetHighestPt1_2ndPair_CSV, multilepton_JetHighestPt2_2ndPair_CSV, multilepton_JetClosestMw1_2ndPair_CSV, multilepton_JetClosestMw2_2ndPair_CSV, multilepton_JetLowestMjj1_CSV, multilepton_JetLowestMjj2_CSV, multilepton_JetLowestMjj1_2ndPair_CSV, multilepton_JetLowestMjj2_2ndPair_CSV;

		Float_t         multilepton_Lepton1_DeltaR_Matched,  multilepton_Lepton2_DeltaR_Matched,    multilepton_Lepton3_DeltaR_Matched,  multilepton_Lepton4_DeltaR_Matched;
        Int_t           multilepton_Lepton1_Label_Matched,   multilepton_Lepton2_Label_Matched,     multilepton_Lepton3_Label_Matched,   multilepton_Lepton4_Label_Matched;
        Int_t           multilepton_Lepton1_Id_Matched,      multilepton_Lepton2_Id_Matched,        multilepton_Lepton3_Id_Matched,      multilepton_Lepton4_Id_Matched;
        TLorentzVector  multilepton_Lepton1_P4_Matched,      multilepton_Lepton2_P4_Matched,        multilepton_Lepton3_P4_Matched,      multilepton_Lepton4_P4_Matched;

		Float_t         multilepton_Bjet1_DeltaR_Matched,   multilepton_Bjet2_DeltaR_Matched;
		Int_t           multilepton_Bjet1_Label_Matched,    multilepton_Bjet2_Label_Matched;
		Int_t           multilepton_Bjet1_Id_Matched,       multilepton_Bjet2_Id_Matched;
		TLorentzVector	multilepton_Bjet1_P4_Matched, 		multilepton_Bjet2_P4_Matched;

		TLorentzVector	multilepton_Ptot, 			multilepton_h0_P4, 		multilepton_t1_P4, 		multilepton_t2_P4;
		Int_t           multilepton_h0_Label,    	multilepton_t1_Label, 	multilepton_t2_Label;
		Int_t           multilepton_h0_Id,       	multilepton_t1_Id, 		multilepton_t2_Id;
//---------------------

		//met
		TLorentzVector multilepton_mET;
	  	Float_t multilepton_mHT;

		//BDT
		vector<TString> BDTvar_list;
		vector<Float_t> BDTvar_floats;

		//OTHER
		Float_t Weight;
		Float_t Channel;
		Float_t NJets;
		Float_t NBJets;
		Float_t mTW;
		Float_t EvtNr;
		Float_t RunNr;

		//Systematics
		vector<TString> v_syst_weight;
		vector<TString> v_syst_tree;
		vector<Float_t> v_syst_float;

};


#endif
