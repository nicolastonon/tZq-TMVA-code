//by Nicolas (inspired from Jeremy's code)

#ifndef theMVAtool_h
#define theMVAtool_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH2.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TString.h"
#include "TColor.h"
#include "TStopwatch.h"
#include "TCut.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <cmath>

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

class theMVAtool
{

public :

//Methods
	theMVAtool();
	theMVAtool(std::vector<TString >, std::vector<TString >, std::vector<TString>, std::vector<TString>, std::vector<int>);
	~theMVAtool(){delete reader;};

	void Set_MVA_Methods(std::string, bool); //Could be used to choose the MVA method. Actually, only use BDT
	void Train_Test_Evaluate(TFile*, TString); //Train, Test, Evaluate BDT with MC samples
	void Read(TFile*); //Apply BDT on samples --> Distributions of discriminant for each sample
	void Determine_Control_Cut(TString); //Determine at which discriminant value the cut should be applied, in order to keep mainly bkg
	void Create_Control_Trees(TFile*, TString, double); //Use the found cut value to copy events passing the cut (bkg) in new trees
	void Create_Control_Histograms(TString); //Use the trees created with Create_Control_Trees to create histograms in same file
	void Draw_Control_Plots(TString, bool); //Draw control plots from the histograms obtained with Create_Control_Histograms()
	void Generate_Pseudo_Data_Histograms(TFile*, TString); //Generate pseudo-data from templates -> can simulate template fit without looking at real data


//Members
	TMVA::Reader *reader;

	std::map<std::string,int> Use; //Map which can be modified to choose the MVA method. Not useful, only use BDT
	std::vector<TString> sample_list;
	std::vector<TString> data_list;
	std::vector<TString> var_list;
	std::vector<TString> syst_list;
	std::vector<TString> channel_list;

	std::vector<float> vec_variables; //Contains as many floats as there are variables in var_list
	std::vector<int> colorVector;

	int nbin; //Control number of bins in BDT histogram
};

#endif
