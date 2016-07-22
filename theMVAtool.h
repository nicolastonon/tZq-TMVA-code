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
#include <map>
#include <string>
#include <cmath>
#include<sstream>

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

	void Set_Variable_Cuts(TString, TString, TString, TString); //Set the cut values on some variables (MET, ...)
	void Set_Luminosity(double); //Set the luminosity re-scaling factor to be used thoughout the code
	void Train_Test_Evaluate(TString); //Train, Test, Evaluate BDT with MC samples
	std::pair<double, double> Compute_Fake_Ratio(); //Computes ratio of fakes in MC compared to data, to re-scale mTW template of fakes from data in Read()
	void Read(TString, int); //Produce templates of BDT, mTW (or else ?)
	float Determine_Control_Cut(); //Determine at which discriminant value the cut should be applied, in order to keep mainly bkg
	void Create_Control_Trees(double); //Use the found cut value to copy events passing the cut (bkg) in new trees
	void Create_Control_Histograms(TString); //Use the trees created with Create_Control_Trees to create histograms in same file
	void Draw_Control_Plots(TString, bool); //Draw control plots from the histograms obtained with Create_Control_Histograms()
	void Generate_Pseudo_Data_Histograms(TString); //Generate pseudo-data from templates -> can simulate template fit without looking at real data
	void Generate_Pseudo_Data_Histograms_CR(TString); //Idem, for replacing data and be able to plot "data/mc" in CR
	void Plot_BDT_Templates_allchannels();
	void Plot_BDT_Templates(TString);

//Members
	TMVA::Reader *reader;

	std::vector<TString> sample_list;
	std::vector<TString> data_list;
	std::vector<TString> var_list;
	std::vector<TString> syst_list;
	std::vector<TString> channel_list;

	std::vector<float> vec_variables; //Contains as many floats as there are variables in var_list
	std::vector<int> colorVector;

	int nbin; //Control number of bins in BDT histogram
	double luminosity_rescale;

	TString cut_MET;
	TString cut_mTW;
	TString cut_NJets;
	TString cut_NBJets;
};

#endif
