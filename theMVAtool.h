#ifndef theMVAtool_h
#define theMVAtool_h

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

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TString.h"
#include "TColor.h"
#include "TCut.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TLegend.h"
#include "TRandom.h"
#include "TLatex.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"
#include "TRandom3.h"
#include "TRandom1.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TGaxis.h"
#include "TLeaf.h"
#include "TFractionFitter.h"
#include "TVirtualFitter.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/Config.h"

#include <cassert> 	//Can be used to terminate program if argument is not true. Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

#include "Func_other.h" //Helper functions


using namespace std;

class theMVAtool
{

	public :

//Methods
	theMVAtool(vector<TString >, vector<TString >, vector<TString>, vector<TString>, vector<int>, vector<TString>, vector<TString>, vector<bool>, vector<TString>, int, bool, bool, TString, bool, TString, TString);
	~theMVAtool() {delete reader;}; //Free memory
	void Set_Luminosity(double); //Set the luminosity re-scaling factor to be used thoughout the code
	void Fill_Zpt_Vectors(); //Initialize the vectors with SFs for Zpt reweighting

	void Train_Test_Evaluate(TString, TString, bool, bool); //Train, Test, Evaluate BDT with MC samples
	double Compute_Fake_SF(TFile*, TString); //Compute SF to rescale data Fakes (using TFractionFitter & mTW templates)
	void Rescale_Fake_Histograms(TString); //Rescale Fake histograms with SFs computed with TFractionFitter (uses scaleFakes.cc function)
	double Get_Zpt_Reweighting_SF(int, int, double); //Returns SF for Fakes event (Zpt reweighting)
	int Read(TString, bool, bool, bool, bool = false, double = 999); //Produce templates of BDT or mTW
	float Determine_Control_Cut(); //Determine at which discriminant value the cut should be applied, in order to keep mainly bkg
	void Create_Control_Trees(bool, bool, double, bool, bool); //Create new trees with events passing the cuts
	void Create_Control_Histograms(bool, bool, bool, bool); //Use the trees created with Create_Control_Trees to create histograms in same file
	int Generate_PseudoData_Histograms_For_Control_Plots(bool); //Idem, for replacing data and be able to plot control plots
	int Generate_PseudoData_Templates(TString); //Generate pseudo-data from templates -> can simulate template fit without looking at real data
	int Draw_Control_Plots(TString, bool, bool, bool, bool); //Draw control plots from the histograms obtained with Create_Control_Histograms()
	int Plot_Prefit_Templates(TString, TString, bool, bool); //Plot prefit templates (given to Combine)
	int Plot_Postfit_Templates(TString, TString, bool, bool); //Plot postfit templates from Combine output
	void Convert_Templates_Theta(); //Takes Reader file, and modifies all histograms names to comply with Theta conventions
	float Compute_Combine_Significance_From_TemplateFile(TString, TString, TString, bool, bool); //Moves file to proper directory, runs Likelihood Fit, reads & returns result
	void Superpose_With_Without_MEM_Templates(TString, TString, bool); //Superpose prefit template distributions with or without MEM
	void Draw_Template_With_Systematic_Variation(TString, TString, TString, TString); //Shows the variation of 1 systematic on 1 template (e.g. compare JES/nominal in tZq)
	void Superpose_Shapes_Fakes_Signal(TString, TString, bool, bool); //Superpose prefit templates for : signal, fakes, other
	void Rebin_Template_File(TString, int); //Rebin all the templates in input file
	void Compare_Negative_Or_Absolute_Weight_Effect_On_Distributions(TString, bool);


	//Obsolete functions
	// int Fit_Fake_Templates(TString, TString); //Fit the fake templates
	// int Create_Fake_Templates_From_Fit(TString, TString); //Create new template from fit or original template (no empty bin)

//Members
	TMVA::Reader *reader;

	std::vector<TString> var_list; std::vector<float> var_list_floats; //TMVA variables
	std::vector<TString> v_add_var_names; vector<float> v_add_var_floats; //Additional vars only for CR plots
	std::vector<TString> v_cut_name; std::vector<TString> v_cut_def; std::vector<float> v_cut_float; std::vector<bool> v_cut_IsUsedForBDT; //Variables for region cuts (e.g. NJets, ...)
	std::vector<TString> sample_list;
	std::vector<TString> syst_list;
	std::vector<TString> channel_list;
	std::vector<TString> stringv_list;

 //  	std::vector<float> theWeights_PDF;
	std::vector<int> colorVector;

	int nbin; //Control number of bins in BDT histogram
	double luminosity_rescale; //Rescale weights to desired lumi
 	bool isttZ; //Work in ttZ CR
	bool isWZ;
	TString filename_suffix; //add sufix to filename for clarity
	TString filename_suffix_noJet; //suffix without NJet variables
	TString dir_ntuples; //path to input ntuples
	TString t_name; //name of tree to be used in input ntuples

	bool dbgMode  ;
	bool stop_program;

	TString format; //Format extension for plots (pdf of png?)
	bool combine_naming_convention; //Choose b/w Combine naming convention (true) or Theta ones (false)

	double v_Zpt_el_tZq[10], v_Zpt_el_ttZ[10], v_Zpt_el_WZ1jet[10], v_Zpt_el_WZ2jet[10], v_Zpt_el_WZ3morejet[10], v_Zpt_mu_tZq[10], v_Zpt_mu_ttZ[10], v_Zpt_mu_WZ1jet[10], v_Zpt_mu_WZ2jet[10], v_Zpt_mu_WZ3morejet[10]; //For Zpt reweighting
};

#endif
