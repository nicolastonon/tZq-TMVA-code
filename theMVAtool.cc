#include "theMVAtool.h"
#include "Func_other.h"

#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TLegend.h"
#include "TRandom.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"
#include "TRandom3.h"

//-----------------------------------------------------------------------------------------
//    _____   __  __  __     __     _         ____    ___    ____    _____
//   |_   _| |  \/  | \ \   / /    / \       / ___|  / _ \  |  _ \  | ____|
//     | |   | |\/| |  \ \ / /    / _ \     | |     | | | | | | | | |  _|
//     | |   | |  | |   \ V /    / ___ \    | |___  | |_| | | |_| | | |___
//     |_|   |_|  |_|    \_/    /_/   \_\    \____|  \___/  |____/  |_____|
//
//-----------------------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------------------
//    _           _   _     _           _   _                 _     _
//   (_)  _ __   (_) | |_  (_)   __ _  | | (_)  ____   __ _  | |_  (_)   ___    _ __
//   | | | '_ \  | | | __| | |  / _` | | | | | |_  /  / _` | | __| | |  / _ \  | '_ \
//   | | | | | | | | | |_  | | | (_| | | | | |  / /  | (_| | | |_  | | | (_) | | | | |
//   |_| |_| |_| |_|  \__| |_|  \__,_| |_| |_| /___|  \__,_|  \__| |_|  \___/  |_| |_|
//
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Default Constructor
theMVAtool::theMVAtool()
{
	//channel_list.push_back("");

	sample_list.push_back("tZq");
	sample_list.push_back("WZ");

	var_list.push_back("mTW"); vec_variables.push_back(0); colorVector.push_back(0);
	var_list.push_back("METpt"); vec_variables.push_back(0); colorVector.push_back(0);
	var_list.push_back("ZCandMass"); vec_variables.push_back(0); colorVector.push_back(0);

	syst_list.push_back("");

	reader = new TMVA::Reader( "!Color:!Silent" );

	nbin = 10;

	cut_MET=""; cut_mTW=""; cut_NJets=""; cut_NBJets="";

	stop_program = false;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Overloaded Constructor
theMVAtool::theMVAtool(std::vector<TString > thevarlist, std::vector<TString > thesamplelist, std::vector<TString > thesystlist, std::vector<TString > thechanlist, vector<int> v_color)
{
	for(int i=0; i<thechanlist.size(); i++)
	{
		channel_list.push_back(thechanlist[i]);
	}
	for(int i=0; i<thevarlist.size(); i++)
	{
		var_list.push_back(thevarlist[i]);
		vec_variables.push_back(0);
	}
	for(int i=0; i<thesamplelist.size(); i++)
	{
		sample_list.push_back(thesamplelist[i]);
	}
	for(int i=0; i<thesystlist.size(); i++)
	{
		syst_list.push_back(thesystlist[i]);
	}
	for(int i=0; i<v_color.size(); i++)
	{
		colorVector.push_back(v_color[i]);
	}

	reader = new TMVA::Reader( "!Color:!Silent" );

	nbin = 10;

	cut_MET=""; cut_mTW=""; cut_NJets=""; cut_NBJets=""; filename_suffix = "";

	stop_program = false;
}



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Choose which cuts to apply on BDT variables
void theMVAtool::Set_Variable_Cuts(TString set_MET_cut, TString set_mTW_cut, TString set_NJets_cut, TString set_NBJets_cut)
{
	//Set cuts
	cut_MET=set_MET_cut;
	cut_mTW=set_mTW_cut;
	cut_NJets=set_NJets_cut;
	cut_NBJets=set_NBJets_cut;

	//Jets cuts need to be "strict", in order to avoid several naming issues
	if(cut_NJets.Contains("<=") || cut_NJets.Contains(">=") || cut_NBJets.Contains("<=") || cut_NBJets.Contains(">="))
	{
		cout<<endl<<endl<<"*** ERROR : jet cuts need to be defined strictly (no >= or <=) ! ***"<<endl<<endl;
		stop_program = true;
	}
	//Make sure that the "equal to" sign is written properly
	else
	{
		if(cut_NJets.Contains("=") && !cut_NJets.Contains("=="))
		{
			cut_NJets = "==" + Convert_Number_To_TString(Find_Number_In_TString(cut_NJets));
		}
		//Make sure that the "equal to" sign is written properly
		if(cut_NBJets.Contains("=") && !cut_NBJets.Contains("=="))
		{
			cut_NBJets = "==" + Convert_Number_To_TString(Find_Number_In_TString(cut_NBJets));
		}
	}

	//Store the "cut name" that will be written as a suffix in the name of each output file
	filename_suffix = "";
	TString tmp = "";
	if(cut_MET != "")
	{
		tmp = "_MET" + Convert_Number_To_TString(Find_Number_In_TString(cut_MET));
		filename_suffix+= tmp;
	}
	if(cut_mTW != "")
	{
		tmp = "_mTW" + Convert_Number_To_TString(Find_Number_In_TString(cut_mTW));
		filename_suffix+= tmp;
	}
	if(cut_NJets != "")
	{
		tmp = "_NJets" + Convert_Sign_To_Word(cut_NJets) + Convert_Number_To_TString(Find_Number_In_TString(cut_NJets));
		filename_suffix+= tmp;
	}
	if(cut_NBJets != "")
	{
		tmp = "_NBJets" + Convert_Sign_To_Word(cut_NJets) + Convert_Number_To_TString(Find_Number_In_TString(cut_NBJets));
		filename_suffix+= tmp;
	}
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Set the luminosity re-scaling factor to be used thoughout the code
void theMVAtool::Set_Luminosity(double desired_luminosity = 2.26)
{
	double current_luminosity = 2.26; //2015 - 7.6.x Ntuples --- TO BE CHANGED IN 8.0 !
	this->luminosity_rescale = desired_luminosity / current_luminosity;

	cout<<endl<<endl<<endl<<endl<<endl<<"############################################"<<endl;
	cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<current_luminosity<<" = "<<luminosity_rescale<<" ! ---"<<endl<<endl;
	cout<<"############################################"<<endl<<endl<<endl<<endl<<endl<<endl;
}

//-----------------------------------------------------------------------------------------
//    _                    _           _
//   | |_   _ __    __ _  (_)  _ __   (_)  _ __     __ _
//   | __| | '__|  / _` | | | | '_ \  | | | '_ \   / _` |
//   | |_  | |    | (_| | | | | | | | | | | | | | | (_| |
//    \__| |_|     \__,_| |_| |_| |_| |_| |_| |_|  \__, |
//                                                 |___/
//-----------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Train, test and evaluate the BDT with signal and bkg MC
void theMVAtool::Train_Test_Evaluate(TString channel)
{
	//---------------------------------------------------------------
    // This loads the TMVA libraries
    TMVA::Tools::Instance();

	TString output_file_name = "outputs/BDT";
	if(channel != "") {output_file_name+= "_" + channel;}
	output_file_name+= filename_suffix;
	output_file_name+= ".root";
	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );

	// Create the factory object
	TMVA::Factory* factory = new TMVA::Factory( "BDT", output_file, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

    // Define the input variables that shall be used for the MVA training
	for(int i=0; i<var_list.size(); i++)
	{
		factory->AddVariable(var_list[i].Data(), 'F');
	}

    TFile *f(0);

	for(int isample=0; isample<sample_list.size(); isample++)
    {
		if(sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("Data") || sample_list[isample].Contains("Fakes")) {cout<<"Train only on MC without fakes -- ignored sample"<<endl; continue;} //Train only on MC

        // Read training and test data
        // --- Register the training and test trees

		TString inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
	    TFile* file_input = TFile::Open( inputfile.Data() );

		TTree* tree = (TTree*)file_input->Get("Default");

        // global event weights per tree (see below for setting event-wise weights)
        Double_t signalWeight     = 1.0;
        Double_t backgroundWeight = 1.0;

        // You can add an arbitrary number of signal or background trees
		//NB : can't account for luminosity rescaling here, but it is not necessary for the training (only relative weights matter ?)
		if(sample_list[isample] == "tZq") {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "fabs(Weight)" );}
        else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "fabs(Weight)" );}
		//if(sample_list[isample] == "tZq") {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "Weight" );}
        //else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "Weight" );}
    }

	cout<<"////////WARNING : TAKE *ABSOLUTE* VALUES OF WEIGHTS////////"<<endl;

	// Apply additional cuts on the signal and background samples (can be different)
	TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
	TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

	if(channel != "all")
    {
        if(channel == "uuu")         		{mycuts = "Channel==0"; mycutb = "Channel==0";}
        else if(channel == "uue" )     		{mycuts = "Channel==1"; mycutb = "Channel==1";}
        else if(channel == "eeu"  )     	{mycuts = "Channel==2"; mycutb = "Channel==2";}
        else if(channel == "eee"   )     	{mycuts = "Channel==3"; mycutb = "Channel==3";}
        else 								{cout << "WARNING : wrong channel name while training " << endl;}
    }

//--------------------------------
//--- Apply cuts during training
    TString tmp;
    //Cuts for signal -- automatically adds to previous cut : (cut1) && (cut2)
    if(cut_MET != "") {tmp = "METpt"     + cut_MET;    mycuts+=tmp; mycutb+=tmp;}
    if(cut_mTW != "") {tmp = "mTW"       + cut_mTW;    mycuts+=tmp; mycutb+=tmp;}
	//Can't use a constant variable in BDT (creates fatal error)
    if(cut_NJets != "" && !cut_NJets.Contains("==")) {tmp = "NJets"   + cut_NJets;  mycuts+=tmp; mycutb+=tmp;}
    if(cut_NBJets != "" && !cut_NBJets.Contains("==")) {tmp = "NBJets" + cut_NBJets; mycuts+=tmp; mycutb+=tmp;}
//--------------------------------

    // Tell the factory how to use the training and testing events    //
    // If no numbers of events are given, half of the events in the tree are used for training, and the other half for testing:
    factory->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );

	TString method_title = channel + filename_suffix; //So that the output weights are labelled differently for each channel

    // Boosted Decision Trees // Adaptive Boost
	//factory->BookMethod( TMVA::Types::kBDT, method_title.Data(),    "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );
	factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=15:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );

	output_file->cd();

    // Train MVAs using the set of training events
    factory->TrainAllMethods();
    // ---- Evaluate all MVAs using the set of test events
    factory->TestAllMethods();
    // ----- Evaluate and compare performance of all configured MVAs
    factory->EvaluateAllMethods();
	//NB : Test & Evaluation recap in the output files

    // --------------------------------------------------------------

    // Save the output
    output_file->Close();

    std::cout << "==> Wrote root file: " << output_file->GetName() << std::endl;
    std::cout << "==> TMVA is done!" << std::endl;

    delete factory;

    // Launch the GUI for the root macros    //NB : write interactively in the ROOT environment --> TMVA::TMVAGui("output.root")
    //TMVA::TMVAGui(output_file_name);
}




//-----------------------------------------------------------------------------------------
//                                               _   _
//    _ __    ___           ___    ___    __ _  | | (_)  _ __     __ _
//   | '__|  / _ \  _____  / __|  / __|  / _` | | | | | | '_ \   / _` |
//   | |    |  __/ |_____| \__ \ | (__  | (_| | | | | | | | | | | (_| |
//   |_|     \___|         |___/  \___|  \__,_| |_| |_| |_| |_|  \__, |
//                                                               |___/
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Computes ratio of fakes in MC compared to data, to re-scale mTW template of fakes from data in Read()
std::pair<double, double> theMVAtool::Compute_Fake_Ratio()
{
	vector<TString> MC_fake_samples_list;
	MC_fake_samples_list.push_back("DY");
	MC_fake_samples_list.push_back("TT");
	MC_fake_samples_list.push_back("WW");

	//FAKES FROM MC
	double integral_MC_fake_mu = 0;
	double integral_MC_fake_el = 0;
	for(int i=0; i<MC_fake_samples_list.size(); i++)
	{
		TString inputfile = "Ntuples/FCNCNTuple_" + MC_fake_samples_list[i] + ".root";
		TFile* file_fake = TFile::Open( inputfile.Data() );

		TTree* tree = (TTree*) file_fake->Get("Default");
		float weight = 0; float i_channel = 9;

		tree->SetBranchAddress("Weight", &weight);
		tree->SetBranchAddress("Channel", &i_channel);

		for(int ientry=0; ientry<tree->GetEntries(); ientry++)
		{
			weight = 0; i_channel = 9;
			tree->GetEntry(ientry);

			//Fake muon
			if(i_channel == 0 || i_channel == 2)
			{
				integral_MC_fake_mu+= weight;
			}
			else if(i_channel == 1 || i_channel == 3)
			{
				integral_MC_fake_el+= weight;
			}
			else {cout<<__LINE__<<" --- ERROR : wrong channel value ! ---"<<endl;}
		}
	}

	//FAKES FROM DATA
	double integral_data_fake_mu = 0;
	double integral_data_fake_el = 0;
	TString inputfile = "Ntuples/FCNCNTuple_Fakes.root";
	TFile* file_fake = TFile::Open( inputfile.Data() );

	TTree* tree = (TTree*) file_fake->Get("Default");
	double weight = 0; double i_channel = 9;

	tree->SetBranchAddress("Weight", &weight);
	tree->SetBranchAddress("Channel", &i_channel);

	for(int ientry=0; ientry<tree->GetEntries(); ientry++)
	{
		weight = 0; i_channel = 9;

		tree->GetEntry(ientry);

		//Fake muon
		if(i_channel == 0 || i_channel == 2)
		{
			integral_data_fake_mu+= weight;
		}
		else if(i_channel == 1 || i_channel == 3)
		{
			integral_data_fake_el+= weight;
		}
		else {cout<<__LINE__<<" --- ERROR : wrong channel value ! ---"<<endl;}
	}

	std::pair<double, double> the_return;
	the_return.first = integral_MC_fake_mu / integral_data_fake_mu;
	the_return.second = integral_MC_fake_el / integral_data_fake_el;

	return the_return;
}




//-----------------------------------------------------------------------------------------
//                              _   _
//    _ __    ___    __ _    __| | (_)  _ __     __ _
//   | '__|  / _ \  / _` |  / _` | | | | '_ \   / _` |
//   | |    |  __/ | (_| | | (_| | | | | | | | | (_| |
//   |_|     \___|  \__,_|  \__,_| |_| |_| |_|  \__, |
//                                              |___/
//-----------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//FIXME : need to replace "Fakes" with name of Ntuple containing fakes from data !

//Reader function. Uses output from training (weights, ...) and read samples to create distributions of the BDT discriminant *OR* mTW
//fakes_mode : 0 -> No fakes || 1 -> Fakes from MC || 2 -> Fakes from data//
void theMVAtool::Read(TString template_name = "BDT", int fakes_mode = 1)
{
	cout<<endl<<"#####################"<<endl;
	if(template_name == "BDT") {cout<<"--- Producing BDT templates ---"<<endl;}
	else if(template_name == "mTW") {cout<<"--- Producing mTW templates ---"<<endl;}
	else {cout<<"--- ERROR : invalid template_name value !"<<endl;}

	if(fakes_mode == 0) {cout<<"--- NOT USING FAKES ---"<<endl;}
	else if(fakes_mode == 1) {cout<<"--- Using fakes from MC ---"<<endl;}
	else if(fakes_mode == 2) {cout<<"--- Using fakes from data ---"<<endl;}
	else {cout<<"ERROR : invalid fakes_method value !"<<endl;}
	cout<<"#####################"<<endl<<endl;

	TString output_file_name = "outputs/Reader"+ filename_suffix + ".root";
	TFile* file_output = TFile::Open( output_file_name, "RECREATE" );

	TH1F *hist_BDT(0), *hist_BDTG(0);

	reader = new TMVA::Reader( "!Color:!Silent" );

	// Name & adress of local variables which carry the updated input values during the event loop
	// - the variable names MUST corresponds in name and type to those given in the weight file(s) used
	for(int i=0; i<var_list.size(); i++)
	{
		reader->AddVariable(var_list[i].Data(), &(vec_variables[i])); //cout<<"Added variable "<<var_list[i]<<endl;
	}

	// --- Book the MVA methods
	TString dir    = "weights/";

	// Book MVA method
	if(template_name == "BDT")
	{
		for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
		{
		  TString MVA_method_name = "BDT_" + channel_list[ichan] + filename_suffix + TString(" method");
		  TString weightfile = dir + "BDT_" + channel_list[ichan] + filename_suffix + TString(".weights.xml"); //Contains weights related to BDT branches
		  reader->BookMVA( MVA_method_name, weightfile );
		}
	}
	//cout<<__LINE__<<endl;

	TH1F *hist_uuu = 0, *hist_uue = 0, *hist_eeu = 0, *hist_eee = 0;
	TH1F *h_sum_fake = 0;

	// --- Systematics loop
	for(int isyst=0; isyst<syst_list.size(); isyst++)
	{
		//Loop on samples, syst., events ---> Fill histogram/channel --> Write()
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			if(fakes_mode == 0 && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("Fakes")) ) {continue;} //No fakes
			else if(fakes_mode == 1 && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
			else if(fakes_mode == 2 && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

			if(sample_list[isample].Contains("Data") && syst_list[isyst]!="") {continue;}
			else if(sample_list[isample].Contains("ttZ") && syst_list[isyst].Contains("Q2")) {continue;} //bug for Q2 in ttZ for the moment
			else if(sample_list[isample].Contains("Fakes") && syst_list[isyst] != "") {continue;} //No syst for fakes from data

			TString inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
			TFile* file_input = TFile::Open( inputfile.Data() );

			std::cout << "--- Select "<<sample_list[isample]<<" sample" << std::endl;

			if(fakes_mode == 0 || (fakes_mode == 1 && isample <= (sample_list.size() - 3)) ) //If sample is fake, don't reinitialize histos -> sum fakes
			{
				// Book output histograms
				if (template_name == "BDT") //create histogram for each channel (-1 = bkg, +1 = signal)
				{
					hist_uuu     = new TH1F( "MVA_BDT_uuu",           "MVA_BDT_uuu",           nbin, -1, 1 );
					hist_uue     = new TH1F( "MVA_BDT_uue",           "MVA_BDT_uue",           nbin, -1, 1 );
					hist_eeu     = new TH1F( "MVA_BDT_eeu",           "MVA_BDT_eeu",           nbin, -1, 1 );
					hist_eee     = new TH1F( "MVA_BDT_eee",           "MVA_BDT_eee",           nbin, -1, 1 );
				}
				else if (template_name == "mTW")
				{
					hist_uuu     = new TH1F( "mTW_uuu",           "mTW_uuu",           nbin, 0, 100 );
					hist_uue     = new TH1F( "mTW_uue",           "mTW_uue",           nbin, 0, 100 );
					hist_eeu     = new TH1F( "mTW_eeu",           "mTW_eeu",           nbin, 0, 100 );
					hist_eee     = new TH1F( "mTW_eee",           "mTW_eee",           nbin, 0, 100 );
				}

				hist_uuu->Sumw2(); hist_uue->Sumw2(); hist_eeu->Sumw2(); hist_eee->Sumw2();
			}


			TTree* tree(0);
			TString tree_name = "";

			//For some systematics, need a different tree (many variables change)
			if(syst_list[isyst]== "JER__plus" || syst_list[isyst] == "JER__minus" || syst_list[isyst]== "JES__plus" || syst_list[isyst] == "JES__minus") {tree = (TTree*) file_input->Get(syst_list[isyst].Data());}
			else {tree = (TTree*) file_input->Get("Default");}
			//cout<<__LINE__<<endl;

			// Prepare the event tree
			for(int i=0; i<var_list.size(); i++)
			{
				tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
			}
			float i_channel; tree->SetBranchAddress("Channel", &i_channel);

			float weight;
			//For some systematics, only the weight changes
			if(syst_list[isyst] == "" || syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES"))	{tree->SetBranchAddress("Weight", &weight);}
			else if(syst_list[isyst] == "PU__plus") 		tree->SetBranchAddress("PU__plus", &weight);
			else if(syst_list[isyst] == "PU__minus")		tree->SetBranchAddress("PU__minus", &weight);
			else if(syst_list[isyst] == "Q2__plus") 		tree->SetBranchAddress("Q2__plus", &weight);
			else if(syst_list[isyst] == "Q2__minus") 		tree->SetBranchAddress("Q2__minus", &weight);
			else if(syst_list[isyst] == "MuEff__plus") 		tree->SetBranchAddress("MuEff__plus", &weight);
			else if(syst_list[isyst] == "MuEff__minus") 	tree->SetBranchAddress("MuEff__minus", &weight);
			else if(syst_list[isyst] == "EleEff__plus") 	tree->SetBranchAddress("EleEff__plus", &weight);
			else if(syst_list[isyst] == "EleEff__minus")	tree->SetBranchAddress("EleEff__minus", &weight);
			else {cout<<"Problem : Wrong systematic name"<<endl;}

			std::cout << "--- Processing: " << tree->GetEntries() << " events" << std::endl;

			//------------------------------------------------------------
			// --- Event loop
			for(int ievt=0; ievt<tree->GetEntries(); ievt++)
			{
				weight = 0; i_channel = 9;

				tree->GetEntry(ievt);

//------------------------------------------------------------
//------------------------------------------------------------
//---- Apply cuts on Reader here --- VARIABLES ORDER IS IMPORTANT
				float METpt = vec_variables[0];
				float mTW = vec_variables[1];
				float NJets = vec_variables[2];
				float NBJets = vec_variables[3];

				float cut_tmp = 0;

				if(cut_MET != "")
				{
					cut_tmp = Find_Number_In_TString(cut_MET);
					if(cut_MET.Contains(">=") && METpt < cut_tmp) {continue;}
					else if(cut_MET.Contains("<=") && METpt > cut_tmp) {continue;}
					else if(cut_MET.Contains(">") && METpt <= cut_tmp) {continue;}
					else if(cut_MET.Contains("<") && METpt >= cut_tmp) {continue;}
					//else if(cut_MET.Contains("==") && METpt != cut_tmp) {continue;}
				}
				if(cut_mTW != "")
				{
					cut_tmp = Find_Number_In_TString(cut_mTW);
					if(cut_mTW.Contains(">=") && mTW < cut_tmp) {continue;}
					else if(cut_mTW.Contains("<=") && mTW > cut_tmp) {continue;}
					else if(cut_mTW.Contains(">") && mTW <= cut_tmp) {continue;}
					else if(cut_mTW.Contains("<") && mTW >= cut_tmp) {continue;}
					//else if(cut_mTW.Contains("==") && mTW != cut_tmp) {continue;}
				}
				if(cut_NJets != "")
				{
					cut_tmp = Find_Number_In_TString(cut_NJets);
					if(cut_NJets.Contains(">=") && NJets < cut_tmp) {continue;}
					else if(cut_NJets.Contains("<=") && NJets > cut_tmp) {continue;}
					else if(cut_NJets.Contains(">") && NJets <= cut_tmp) {continue;}
					else if(cut_NJets.Contains("<") && NJets >= cut_tmp) {continue;}
					else if(cut_NJets.Contains("==") && NJets != cut_tmp) {continue;}
				}
				if(cut_NBJets != "")
				{
					cut_tmp = Find_Number_In_TString(cut_NBJets);
					if(cut_NBJets.Contains(">=") && NBJets < cut_tmp) {continue;}
					else if(cut_NBJets.Contains("<=") && NBJets > cut_tmp) {continue;}
					else if(cut_NBJets.Contains(">") && NBJets <= cut_tmp) {continue;}
					else if(cut_NBJets.Contains("<") && NBJets >= cut_tmp) {continue;}
					else if(cut_NBJets.Contains("==") && NBJets != cut_tmp) {continue;}
				}
//------------------------------------------------------------
//------------------------------------------------------------

				weight*= luminosity_rescale; //Re-scale to desired luminosity

				// --- Return the MVA outputs and fill into histograms
				if (template_name == "BDT")
				{
					if(i_channel == 0) {hist_uuu->Fill( reader->EvaluateMVA( "BDT_uuu"+filename_suffix+ " method"), weight);}					else if(i_channel == 1) {hist_uue->Fill( reader->EvaluateMVA( "BDT_uue"+filename_suffix+" method"), weight);}
					else if(i_channel == 2) {hist_eeu->Fill( reader->EvaluateMVA( "BDT_eeu"+filename_suffix+" method"), weight);}
					else if(i_channel == 3) {hist_eee->Fill( reader->EvaluateMVA( "BDT_eee"+filename_suffix+" method"), weight);}
					else if(i_channel == 9 || weight == 0) {cout<<__LINE__<<" : problem"<<endl;}
				}
				// --- Return the MVA outputs and fill into histograms
				else if (template_name == "mTW")
				{
					if(i_channel == 0) 		{hist_uuu->Fill( mTW, weight);}
					else if(i_channel == 1) {hist_uue->Fill( mTW, weight);}
					else if(i_channel == 2) {hist_eeu->Fill( mTW, weight);}
					else if(i_channel == 3) {hist_eee->Fill( mTW, weight);}
					else if(i_channel == 9 || weight == 0) {cout<<__LINE__<<" : problem"<<endl;}
				}
			}

			//If fakes from data, re-scale histos to expected MC fake yields via function Compute_Fake_Ratio
			std::pair<double, double> fake_ratios;
			if(fakes_mode == 2) {fake_ratios = Compute_Fake_Ratio();} //Only if use data-driven fakes

			if(template_name == "mTW" && fakes_mode == 2 && sample_list[isample].Contains("Fakes") )
			{
				hist_uuu->Scale(fake_ratios.first); hist_eeu->Scale(fake_ratios.first);
				hist_uue->Scale(fake_ratios.second); hist_eee->Scale(fake_ratios.second);
			}

			// --- Write histograms
			file_output->cd();

			//NB : theta name convention = <observable>__<process>[__<uncertainty>__(plus,minus)]
			TString output_histo_name = "";
			TString syst_name = "";
			if(syst_list[isyst] != "") syst_name = "__" + syst_list[isyst];

			if(fakes_mode == 1 && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is fake, don't reinitialize histos -> sum 3 MC fake samples
			{
				continue;
			}
			else if(fakes_mode == 1 && isample == (sample_list.size() - 1) ) //Last fake MC sample -> write the summed fake histo
			{
				if (template_name == "BDT")
				{
					output_histo_name = "BDT_uuu__FakeMu" + syst_name;
					hist_uuu->Write(output_histo_name.Data());
					output_histo_name = "BDT_uue__FakeEl" + syst_name;
					hist_uue->Write(output_histo_name.Data());
					output_histo_name = "BDT_eeu__FakeMu" + syst_name;
					hist_eeu->Write(output_histo_name.Data());
					output_histo_name = "BDT_eee__FakeEl" + syst_name;
					hist_eee->Write(output_histo_name.Data());
				}
				else if (template_name == "mTW")
				{
					output_histo_name = "mTW_uuu__FakeMu" + syst_name;
					hist_uuu->Write(output_histo_name.Data());
					output_histo_name = "mTW_uue__FakeEl" + syst_name;
					hist_uue->Write(output_histo_name.Data());
					output_histo_name = "mTW_eeu__FakeMu" + syst_name;
					hist_eeu->Write(output_histo_name.Data());
					output_histo_name = "mTW_eee__FakeEl" + syst_name;
					hist_eee->Write(output_histo_name.Data());
				}
			}
			else //If fakes are not considered, if we use fakes from data, or if sample is not fake --> write directly !
			{
				if (template_name == "BDT")
				{
					output_histo_name = "BDT_uuu__" + sample_list[isample] + syst_name;
					hist_uuu->Write(output_histo_name.Data());
					output_histo_name = "BDT_uue__" + sample_list[isample] + syst_name;
					hist_uue->Write(output_histo_name.Data());
					output_histo_name = "BDT_eeu__" + sample_list[isample] + syst_name;
					hist_eeu->Write(output_histo_name.Data());
					output_histo_name = "BDT_eee__" + sample_list[isample] + syst_name;
					hist_eee->Write(output_histo_name.Data());
				}
				else if (template_name == "mTW")
				{
					output_histo_name = "mTW_uuu__" + sample_list[isample] + syst_name;
					hist_uuu->Write(output_histo_name.Data());
					output_histo_name = "mTW_uue__" + sample_list[isample] + syst_name;
					hist_uue->Write(output_histo_name.Data());
					output_histo_name = "mTW_eeu__" + sample_list[isample] + syst_name;
					hist_eeu->Write(output_histo_name.Data());
					output_histo_name = "mTW_eee__" + sample_list[isample] + syst_name;
					hist_eee->Write(output_histo_name.Data());
				}
			}

			if(fakes_mode != 1 || (fakes_mode == 1 && isample < (sample_list.size() - 3)) || (fakes_mode == 1 && isample == (sample_list.size() - 1) && isyst == (syst_list.size() - 1) )) //Only delete if histos are not reused
			{
				//cout<<"deleting dynamic histograms"<<endl;
				delete hist_uuu; delete hist_uue; delete hist_eeu; delete hist_eee;
			}

			//cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
		} //end sample loop
		cout<<"Done with "<<syst_list[isyst]<<" syst"<<endl;
	} //end syst loop

	file_output->Close();

	std::cout << "--- Created root file: \""<<file_output->GetName()<<"\" containing the output histograms" << std::endl;

	std::cout << "==> Reader() is done!" << std::endl << std::endl;
}



//-----------------------------------------------------------------------------------------
//    ____    ____    _____      ____   ____
//   | __ )  |  _ \  |_   _|    / ___| |  _ \
//   |  _ \  | | | |   | |     | |     | |_) |
//   | |_) | | |_| |   | |     | |___  |  _ <
//   |____/  |____/    |_|      \____| |_| \_\
//
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Read the histograms output from Read() and determine cut value on discriminant to obtain desired signal efficiency
// --> Can use this value as input parameter in Read() to create additionnal "control histograms"
float theMVAtool::Determine_Control_Cut()
{
	TString input_file_name = "outputs/Reader" + filename_suffix + ".root";
	TFile* f = TFile::Open(input_file_name.Data());

	TH1F *h_sum_bkg(0), *h_sig(0), *h_tmp(0);

	TString input_histo_name = "";

	for(int isample=0; isample<sample_list.size(); isample++)
	{
		cout<<"--- Use sample "<<sample_list[isample]<<endl;

		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			if(sample_list[isample].Contains("Data")) {continue;}

			h_tmp = 0;

			input_histo_name = "BDT_" + channel_list[ichan] + "__" + sample_list[isample];

			if(!f->GetListOfKeys()->Contains(input_histo_name.Data())) {continue;}

			h_tmp = (TH1F*) f->Get(input_histo_name.Data())->Clone();

			if(sample_list[isample] == "tZq")
			{
				if(h_sig == 0) {h_sig = (TH1F*) h_tmp->Clone();}
				else {h_sig->Add(h_tmp);}
			}
			else
			{
				if(h_sum_bkg == 0) {h_sum_bkg = (TH1F*) h_tmp->Clone();}
				else {h_sum_bkg->Add(h_tmp);}
			}
		} //end channel loop
	} //end sample loop

	//S+B histogram
	TH1F* h_total = (TH1F*) h_sum_bkg->Clone();
	h_total->Add(h_sig);

	//Normalization
	//h_total->Scale(1/h_total->Integral()); //Integral() : Return integral of bin contents in range [binx1,binx2] (inclusive !)
	//h_sum_bkg->Scale(1/h_sum_bkg->Integral());
	//h_sig->Scale(1/h_sig->Integral());

	double cut = 1; //initialize cut scan at maximum value = +1
	double sig_over_total = 100; //initialize ratio at unreasonable value
	int bin_cut = -1; //initialize to false value
	double step = 2./nbin; //scanning step - defined as bin width

	//Try different cuts, see which one minimizes sig/bkg (and keeping enough bkg events)
	for(double cut_tmp = ( (step*(nbin)/2) - step/10); cut_tmp>-1; cut_tmp-=step) //Signal is on the right, so go "from right to left"
	//NB : weird initialization value because FindBin seem to behave strangely when you give it a value that corresponds to the edge of a bin ; this way (w/ '-step/10') it HAS to choose b/w 2 bins !
	//So we first substract ' - step /10' to make sure it chooses the correct bin, and then after the scan loop we re-add '+ step/10' to obtain the valid cut value at the bin *EDGE*
	{
		int bin_tmp = h_sum_bkg->GetXaxis()->FindBin(cut_tmp); //Bin corresponding to the tested cut
		//cout<<"cut tmp = "<<cut_tmp<<", bin is "<<bin_tmp<<endl;

		//Minimize S/S+B while making sure that we retain enough bkg events to make plots (here : 'at least 60% of the initial bkg events')
		if( (h_sig->Integral(1, bin_tmp) / h_total->Integral(1, bin_tmp)) <= sig_over_total && (h_sum_bkg->Integral(1, bin_tmp) / h_sum_bkg->Integral()) >= 0.6 )
		{
			cut = cut_tmp;
			bin_cut = bin_tmp;
			sig_over_total = h_sig->Integral(1, bin_cut) / h_total->Integral(1, bin_cut);
		}
	}

	cut+= step/10; //Re-correct the cut value (cf. explanation above)
	if(fabs(cut) < pow(10, -10)) {cut = 0;} //Avoid that 'cut' takes non-null very tiny values, like 10^-17, that don't make sense

	TCanvas* c = new TCanvas("c", "Signal VS Background");
	gStyle->SetOptStat(0);
	h_sum_bkg->GetXaxis()->SetTitle("Discriminant");
	h_sum_bkg->SetTitle("Signal VS Background");
	h_sum_bkg->SetLineColor(kBlue);
	h_sig->SetLineColor(kGreen);
	h_sum_bkg->Draw("HIST");
	h_sig->Draw("HIST SAME");

	TLegend* leg = new TLegend(0.7,0.75,0.88,0.85);
    leg->SetHeader("");
    leg->AddEntry(h_sig,"Signal","L");
    leg->AddEntry("h_sum_bkg","Background","L");
    leg->Draw();

	//Draw vertical line at cut value
	TLine* l = new TLine(cut, 0, cut, h_sum_bkg->GetMaximum());
	l->SetLineWidth(3);
	l->Draw("");

	c->SaveAs("outputs/Signal_Background_BDT"+filename_suffix+".png");

	cout<<"---------------------------------------"<<endl;
	cout<<"* Cut Value = "<<cut<<endl;
	cout<<"-> BDT_CR defined w/ all events inside bins [1 ; "<<bin_cut<<"] of the BDT distribution!"<<endl;
	//Since histograms are binned, need to apply the MVA cut at a *bin edge* in order to correctly estimate efficiency via Integral(bin_low, bin_up) !
	//cout<<"---> Cut at upper edge of associated bin = "<<h_sum_bkg->GetBinLowEdge(bin_cut+1)<<" !"<<endl;
	cout<<"* Signal integral = "<<h_sig->Integral(1, bin_cut)<<" / Total integral "<<h_total->Integral(1, bin_cut);
	cout<<" --> Sig/Bkg = "<<sig_over_total<<endl;
	cout<<"---------------------------------------"<<endl<<endl;

	//for(int i=0; i<h_sig->GetNbinsX(); i++) {cout<<"bin content "<<i+1<<" = "<<h_sig->GetBinContent(i+1)<<endl;} //If want to verify that the computed signal is computed correctly

	f->Close(); delete c; delete leg; delete l;

	return cut;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Similar to Read(). Takes cut value as input parameter, and outputs histograms containing only events verifying BDT<cut (mainly bkg events) --> Create histogram with these events for further control studies
void theMVAtool::Create_Control_Trees(bool cut_on_BDT, double cut = 0)
{
	if(cut_on_BDT) {cout<<endl<<"--- Creating control tree WITH cut on BDT value ---"<<endl<<" Cut value = "<<cut<<endl<<endl;}
	else if(!cut_on_BDT) {cout<<endl<<"--- Creating control tree WITHOUT cut on BDT value ---"<<endl<<endl;}

	reader = new TMVA::Reader( "!Color:!Silent" );

	// Name & adress of local variables which carry the updated input values during the event loop
	// - the variable names MUST corresponds in name and type to those given in the weight file(s) used
	for(int i=0; i<var_list.size(); i++)
	{
		reader->AddVariable(var_list[i].Data(), &(vec_variables[i])); //cout<<"Added variable "<<var_list[i]<<endl;
	}

	// --- Book the MVA methods
	TString dir    = "weights/";

	// Book method
	for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
	{
	  TString MVA_method_name = "BDT_" + channel_list[ichan] + filename_suffix + TString(" method");
	  TString weightfile = dir + "BDT_" + channel_list[ichan] + filename_suffix + TString(".weights.xml");
	  reader->BookMVA( MVA_method_name, weightfile );
	}



//---Loop on histograms
	for(int isyst = 0; isyst < syst_list.size(); isyst++)
	{
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			//std::cout << "--- Select "<<sample_list[isample]<<" sample" << std::endl;

			TString inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
			TFile* file_input = TFile::Open( inputfile.Data() );

			if(sample_list[isample].Contains("Data") && syst_list[isyst]!="") {continue;}
			else if(sample_list[isample].Contains("ttZ") && syst_list[isyst].Contains("Q2")) {continue;} //problem with madgraph ttZ sample

			TString output_file_name = "outputs/Control_Trees" + filename_suffix + ".root";
			TFile* output_file = TFile::Open( output_file_name, "UPDATE" );

			//Create new tree, that will be filled only with events verifying MVA<cut
			TTree *tree(0), *tree_control(0);
			tree_control = new TTree("tree_control", "Control Tree");

			float weight; float i_channel;
			for(int ivar=0; ivar<var_list.size(); ivar++)
			{
				TString var_type = var_list[ivar] + "/F";
				tree_control->Branch(var_list[ivar].Data(), &(vec_variables[ivar]), var_type.Data());
			}
			tree_control->Branch("Weight", &weight, "weight/F"); //Call it "Weight", even if use special weight for systematics
			tree_control->Branch("Channel", &i_channel, "i_channel/F");

			//For some systematics, need a different tree (many variables change)
			if(syst_list[isyst]== "JER__plus" || syst_list[isyst] == "JER__minus" || syst_list[isyst]== "JES__plus" || syst_list[isyst] == "JES__minus") {tree = (TTree*) file_input->Get(syst_list[isyst].Data());}
			else {tree = (TTree*) file_input->Get("Default");}

			for(int i=0; i<var_list.size(); i++)
			{
				tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
			}

			 tree->SetBranchAddress("Channel", &i_channel);

			//For some systematics, only the weight changes
			if(syst_list[isyst] == "PU__plus") tree->SetBranchAddress("PU__plus", &weight);
			else if(syst_list[isyst] == "PU__minus") tree->SetBranchAddress("PU__minus", &weight);
			else if(syst_list[isyst] == "Q2__plus") tree->SetBranchAddress("Q2__plus", &weight);
			else if(syst_list[isyst] == "Q2__minus") tree->SetBranchAddress("Q2__minus", &weight);
			else if(syst_list[isyst] == "MuEff__plus") tree->SetBranchAddress("MuEff__plus", &weight);
			else if(syst_list[isyst] == "MuEff__minus") tree->SetBranchAddress("MuEff__minus", &weight);
			else if(syst_list[isyst] == "EleEff__plus") tree->SetBranchAddress("EleEff__plus", &weight);
			else if(syst_list[isyst] == "EleEff__minus") tree->SetBranchAddress("EleEff__minus", &weight);
			else tree->SetBranchAddress("Weight", &weight);


			//std::cout << "--- Processing: " << tree->GetEntries() << " events" << std::endl;

			// --- Event loop
			for(int ievt=0; ievt<tree->GetEntries(); ievt++)
			{
				weight = 0; i_channel = 9;

				tree->GetEntry(ievt); //Fills vec_variables

//------------------------------------------------------------			cout<<"MET_cut is set to : "<<cut_MET<<endl;

//------------------------------------------------------------
//---- Apply cuts (tuned in main()), to be in sync with output_Reader ! --- VARIABLES ORDER IS IMPORTANT
				float METpt = vec_variables[0];
				float mTW = vec_variables[1];
				float NJets = vec_variables[2];
				float NBJets = vec_variables[3];

				float cut_tmp = 0;

				if(cut_MET != "")
				{
					cut_tmp = Find_Number_In_TString(cut_MET);
					if(cut_MET.Contains(">=") && METpt < cut_tmp) {continue;}
					else if(cut_MET.Contains("<=") && METpt > cut_tmp) {continue;}
					else if(cut_MET.Contains(">") && METpt <= cut_tmp) {continue;}
					else if(cut_MET.Contains("<") && METpt >= cut_tmp) {continue;}
					//else if(cut_MET.Contains("==") && METpt != cut_tmp) {continue;}
				}
				if(cut_mTW != "")
				{
					cut_tmp = Find_Number_In_TString(cut_mTW);
					if(cut_mTW.Contains(">=") && mTW < cut_tmp) {continue;}
					else if(cut_mTW.Contains("<=") && mTW > cut_tmp) {continue;}
					else if(cut_mTW.Contains(">") && mTW <= cut_tmp) {continue;}
					else if(cut_mTW.Contains("<") && mTW >= cut_tmp) {continue;}
					//else if(cut_mTW.Contains("==") && mTW != cut_tmp) {continue;}
				}
				if(cut_NJets != "")
				{
					cut_tmp = Find_Number_In_TString(cut_NJets);
					if(cut_NJets.Contains(">=") && NJets < cut_tmp) {continue;}
					else if(cut_NJets.Contains("<=") && NJets > cut_tmp) {continue;}
					else if(cut_NJets.Contains(">") && NJets <= cut_tmp) {continue;}
					else if(cut_NJets.Contains("<") && NJets >= cut_tmp) {continue;}
					else if(cut_NJets.Contains("==") && NJets != cut_tmp) {continue;}
				}
				if(cut_NBJets != "")
				{
					cut_tmp = Find_Number_In_TString(cut_NBJets);
					if(cut_NBJets.Contains(">=") && NBJets < cut_tmp) {continue;}
					else if(cut_NBJets.Contains("<=") && NBJets > cut_tmp) {continue;}
					else if(cut_NBJets.Contains(">") && NBJets <= cut_tmp) {continue;}
					else if(cut_NBJets.Contains("<") && NBJets >= cut_tmp) {continue;}
					else if(cut_NBJets.Contains("==") && NBJets != cut_tmp) {continue;}
				}
//------------------------------------------------------------
//------------------------------------------------------------

				if(cut_on_BDT)
				{
					if(i_channel == 0 && reader->EvaluateMVA( "BDT_uuu"+filename_suffix+" method") > cut) 		{continue;}
					else if(i_channel == 1 && reader->EvaluateMVA( "BDT_uue"+filename_suffix+" method") > cut) 	{continue;}
					else if(i_channel == 2 && reader->EvaluateMVA( "BDT_eeu"+filename_suffix+" method") > cut) 	{continue;}
					else if(i_channel == 3 && reader->EvaluateMVA( "BDT_eee"+filename_suffix+" method") > cut) 	{continue;}
				}

				weight*= luminosity_rescale; //Re-scale to desired luminosity

				tree_control->Fill();

			} //end event loop

			// --- Write histograms

			output_file->cd();

			//NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
			TString output_tree_name = "Control_" + sample_list[isample];
			if (syst_list[isyst] != "") {output_tree_name+= "_" + syst_list[isyst];}

			tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);

			delete tree_control;
			output_file->Close();
			//cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
		} //end sample loop
		cout<<"Done with "<<syst_list[isyst]<<" syst"<<endl;
	} //end syst loop

	std::cout << "--- Created root file containing the output trees" << std::endl;
	std::cout << "==> Create_Control_Trees() is done!" << std::endl << std::endl;
}


//Create histograms from control trees (in same file)
//NB : no separation by channel is made ! It is possible but it would require to create 4 times histograms at beginning...
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Create_Control_Histograms(TString channel)
{
	TString input_file_name = "outputs/Control_Trees" + filename_suffix + ".root";
	TString output_file_name = "outputs/Control_Histograms" + filename_suffix + ".root";
	TFile* f_input = TFile::Open( input_file_name ); TFile* f_output = TFile::Open( output_file_name, "UPDATE" );
	TTree* tree = 0;
	TH1F* h_tmp = 0;

	int binning = 5;

	int nof_histos_to_create = ((sample_list.size() - 1) * var_list.size() * syst_list.size()) + var_list.size();

	cout<<"################################"<<endl;
	cout<<" *** CHANNEL "<<channel<<" ***"<<endl;
	cout<<endl<<"--- Going to create "<<nof_histos_to_create<<" histograms ! (* 4 channels = "<<nof_histos_to_create*4<<" in total)"<<endl;
	cout<<"--- This might take a while... !"<<endl<<endl;
	cout<<"################################"<<endl;

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		cout<<"--- Processing variable : "<<var_list[ivar]<<endl;
		//Info contained in tree leaves. Need to create histograms first
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			cout<<"--- Processing "<<sample_list[isample]<<endl<<endl;

			for(int isyst=0; isyst<syst_list.size(); isyst++)
			{
				if(sample_list[isample].Contains("Data") && syst_list[isyst] != "") {continue;}

				h_tmp = 0;
				if(var_list[ivar] == "mTW") 							{h_tmp = new TH1F( "","", binning, 10, 130 );}
				else if(var_list[ivar] == "METpt")						{h_tmp = new TH1F( "","", binning, -0., 120 );}
				else if(var_list[ivar] == "ZCandMass") 					{h_tmp = new TH1F( "","", binning, 70, 110 );}
				else if(var_list[ivar] == "deltaPhilb") 				{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "Zpt") 						{h_tmp = new TH1F( "","", binning, 0, 150 );}
				else if(var_list[ivar] == "ZEta")			 			{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "asym") 						{h_tmp = new TH1F( "","", binning, -3, 3 );}
				else if(var_list[ivar] == "mtop") 						{h_tmp = new TH1F( "","", binning, 60, 210 );}
				else if(var_list[ivar] == "btagDiscri") 				{h_tmp = new TH1F( "","", binning, 0.4, 2.4 );}
				else if(var_list[ivar] == "btagDiscri_subleading")		{h_tmp = new TH1F( "","", binning, 0, 1 );}
				else if(var_list[ivar] == "etaQ")						{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "NBJets")						{h_tmp = new TH1F( "","", 3, 0, 3 );}
				else if(var_list[ivar] == "AddLepPT")					{h_tmp = new TH1F( "","", binning, 0, 150 );}
				else if(var_list[ivar] == "AddLepETA")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "LeadJetPT")					{h_tmp = new TH1F( "","", binning, 0, 150 );}
				else if(var_list[ivar] == "LeadJetEta")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "dPhiZMET")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "dPhiZAddLep")				{h_tmp = new TH1F( "","", binning, -4, 4 );}
				else if(var_list[ivar] == "dRAddLepBFromTop")			{h_tmp = new TH1F( "","", binning, 0, 1 );}
				else if(var_list[ivar] == "dRZAddLep")					{h_tmp = new TH1F( "","", binning, 0, 1 );}
				else if(var_list[ivar] == "dRZTop")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
				else if(var_list[ivar] == "TopPT")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
				else if(var_list[ivar] == "NJets")						{h_tmp = new TH1F( "","", 5, 1, 6 );}
				else if(var_list[ivar] == "ptQ")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
				else if(var_list[ivar] == "dRjj")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
				else {cout<<"Unknown variable"<<endl;}

				h_tmp->Sumw2(); //force the storage and computation of the sum of the square of weights per bin (rather than just take srt(bin_content))

				TString tree_name = "Control_" + sample_list[isample];
				//tree_name+= "_" + channel;
				if(syst_list[isyst] != "") {tree_name+= "_" + syst_list[isyst];}
				if(!f_input->GetListOfKeys()->Contains(tree_name.Data())) {cout<<tree_name<<" : problem"<<endl; continue;}
				tree = (TTree*) f_input->Get(tree_name.Data());
				//cout<<__LINE__<<endl;

				int tree_nentries = tree->GetEntries();

				for(int ientry = 0; ientry<tree_nentries; ientry++)
				{
					float weight = 0, tmp = 0, i_channel = 9;

					tree->SetBranchAddress(var_list[ivar], &tmp); //One variable at a time
					tree->SetBranchAddress("Weight", &weight);
					tree->SetBranchAddress("Channel", &i_channel);

					tree->GetEntry(ientry); //Read event

					//NB : No need to re-apply variables cuts here, as the control_tree is only filled with events that pass these cuts

					if(channel == "uuu" && i_channel!= 0) {continue;}
					else if(channel == "uue" && i_channel!= 1) {continue;}
					else if(channel == "eeu" && i_channel!= 2) {continue;}
					else if(channel == "eee" && i_channel!= 3) {continue;}

					h_tmp->Fill(tmp, weight); //Fill histogram -- weight already re-scaled to desired lumi in Create_Control_Trees !
				}

				TString output_histo_name = "Control_"+ channel + "_" + var_list[ivar];
				output_histo_name+= "_" + sample_list[isample];
				//output_histo_name+= "_" + channel;
				if(syst_list[isyst] != "") {output_histo_name+= "_" + syst_list[isyst];}
				f_output->cd();
				h_tmp->Write(output_histo_name.Data(), TObject::kOverwrite);
			}
		}
	}

	f_input->Close();
	f_output->Close();
}


//-----------------------------------------------------------------------------------------
//                                     _               _           _
//    _ __    ___    ___   _   _    __| |   ___     __| |   __ _  | |_    __ _
//   | '_ \  / __|  / _ \ | | | |  / _` |  / _ \   / _` |  / _` | | __|  / _` |
//   | |_) | \__ \ |  __/ | |_| | | (_| | | (_) | | (_| | | (_| | | |_  | (_| |
//   | .__/  |___/  \___|  \__,_|  \__,_|  \___/   \__,_|  \__,_|  \__|  \__,_|
//   |_|
//-----------------------------------------------------------------------------------------

//Generates pseudo data histograms in the "control histograms file". Warning : named 'Data' !
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Generate_PseudoData_Histograms_For_Control_Plots(TString channel)
{
	TRandom3 therand(0);

	TString pseudodata_input_name = "outputs/Control_Histograms" + filename_suffix + ".root";
    TFile* file = TFile::Open( pseudodata_input_name.Data(), "UPDATE");
	cout<<endl<<"--- GENERATION OF PSEUDODATA IN "<<file->GetName()<<" ! ---"<<endl<<endl;

	TH1F *h_sum = 0, *h_tmp = 0;

	//file_input->ls(); //output the content of the file

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		cout<<"--- "<<var_list[ivar]<<endl;

		h_sum = 0;

		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			//if(sample_list[isample] != "WZ" && sample_list[isample] != "ttZ" && sample_list[isample] != "ZZ" && sample_list[isample] != "tZq") {continue;}
			if(sample_list[isample].Contains("Data")) {continue;}

			h_tmp = 0;
			TString histo_name = "Control_" + channel + "_" + var_list[ivar] + "_" + sample_list[isample];
			if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}
			h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
			if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
			else {h_sum->Add(h_tmp);}
		}

		int nofbins = h_sum->GetNbinsX();

		for(int i=0; i<nofbins; i++)
		{
			int bin_content = h_sum->GetBinContent(i+1); cout<<"Initial content = "<<bin_content<<endl;
			int new_bin_content = therand.Poisson(bin_content); cout<<"New content = "<<new_bin_content<<endl;
			h_sum->SetBinContent(i+1, new_bin_content);
		}

		file->cd();
		TString output_histo_name = "Control_" + channel + "_" + var_list[ivar] + "_Data";
		h_sum->Write(output_histo_name, TObject::kOverwrite);
	}

	file->Close();

	cout<<"--- Done wth generation of pseudo-data for CR"<<endl;
}



//Generate pseudo-data histograms from MC, using TRandom::Poisson to simulate statistical fluctuations
//Used to simulate template fit to pseudo-data, to avoid using real data before pre-approval
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Generate_PseudoData_Histograms_For_Templates(TString channel)
{
	TRandom3 therand(0); //Randomization

	TString pseudodata_input_name = "outputs/Reader" + filename_suffix + ".root";
    TFile* file = TFile::Open( pseudodata_input_name.Data(), "UPDATE");
	cout<<endl<<"--- GENERATION OF PSEUDODATA IN "<<file->GetName()<<" ! ---"<<endl<<endl;

	TH1F *h_sum = 0, *h_tmp = 0;

	//file_input->ls(); //output the content of the file

	for(int isample = 0; isample < sample_list.size(); isample++)
	{
		//if(sample_list[isample] != "WZ" && sample_list[isample] != "ttZ" && sample_list[isample] != "ZZ" && sample_list[isample] != "tZq") {continue;}
		if(sample_list[isample].Contains("Data")) {continue;}

		h_tmp = 0;
		TString histo_name = "BDT_" + channel + "__" + sample_list[isample];
		if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}
		h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
		if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
		else {h_sum->Add(h_tmp);}
	}

	//If find "fake template"
	TString template_fake_name = "";
	if(channel == "uuu" || channel == "eeu") {template_fake_name = "FakeMu";}
	else {template_fake_name = "FakeEl";}
	h_tmp = 0;
	TString histo_name = "BDT_" + channel + "__" + template_fake_name;
	if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found (probably bc fakes are not used)"<<endl;}
	else
	{
		h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
		h_sum->Add(h_tmp);
	}

	int nofbins = h_sum->GetNbinsX();

	for(int i=0; i<nofbins; i++)
	{
		int bin_content = h_sum->GetBinContent(i+1); //cout<<"initial content = "<<bin_content<<endl;
		int new_bin_content = therand.Poisson(bin_content); //cout<<"new content = "<<new_bin_content<<endl;
		h_sum->SetBinContent(i+1, new_bin_content);
	}

	file->cd();
	TString output_histo_name = "BDT_" + channel + "__DATA";
	h_sum->Write(output_histo_name, TObject::kOverwrite);

	file->Close();
	file->Close();

	cout<<"--- Done with generation of pseudo-data"<<endl;
}



//-----------------------------------------------------------------------------------------
//        _                                       _           _
//     __| |  _ __    __ _  __      __    _ __   | |   ___   | |_   ___
//    / _` | | '__|  / _` | \ \ /\ / /   | '_ \  | |  / _ \  | __| / __|
//   | (_| | | |    | (_| |  \ V  V /    | |_) | | | | (_) | | |_  \__ \
//    \__,_| |_|     \__,_|   \_/\_/     | .__/  |_|  \___/   \__| |___/
//                                       |_|
//-----------------------------------------------------------------------------------------

//Draw control plots with histograms created with Create_Control_Histograms
//Inspired from code PlotStack.C (cf. NtupleAnalysis/src/...)
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Draw_Control_Plots(TString channel, bool allchannels)
{
//---------------------
//Definitions
	TString input_file_name = "outputs/Control_Histograms" + filename_suffix + ".root";
	TFile* f = TFile::Open( input_file_name );
	TH1F *h_tmp = 0, *h_data = 0;
	THStack *stack = 0;

	//Variable names to be displayed on plots
	TString title_MET = "Missing E_{T} [GeV]";

	vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
	thechannellist.push_back("uuu");
	thechannellist.push_back("uue");
	thechannellist.push_back("eeu");
	thechannellist.push_back("eee");
	//thechannellist.push_back("");

	//Canvas definition
	Load_Canvas_Style();


//---------------------
//Loop on var > chan > sample > syst
//Retrieve histograms, stack, compare, plot
	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
		c1->SetBottomMargin(0.3);

		h_data = 0; stack = 0;
		vector<TH1F*> v_MC_histo;
		//Idem for each systematics
		vector<TH1F*> v_MC_histo_JER_plus; vector<TH1F*> v_MC_histo_JER_minus;
		vector<TH1F*> v_MC_histo_JES_plus; vector<TH1F*> v_MC_histo_JES_minus;
		vector<TH1F*> v_MC_histo_PU_plus; vector<TH1F*> v_MC_histo_PU_minus;
		vector<TH1F*> v_MC_histo_Q2_plus; vector<TH1F*> v_MC_histo_Q2_minus;
		vector<TH1F*> v_MC_histo_MuEff_plus; vector<TH1F*> v_MC_histo_MuEff_minus;
		vector<TH1F*> v_MC_histo_EleEff_plus; vector<TH1F*> v_MC_histo_EleEff_minus;
		//vector<TH1F*> v_MC_histo_test_plus; vector<TH1F*> v_MC_histo_test_minus;

		TLegend* qw = 0;
		qw = new TLegend(.80,.60,.95,.90);
		qw->SetShadowColor(0);
		qw->SetFillColor(0);
		qw->SetLineColor(0);

		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors

		//---------------------------
		//CREATE STACK (MC) AND DATA HISTO
		//---------------------------

		int niter_chan = 0; //is needed to know if histo must be cloned or added

		for(int ichan=0; ichan<thechannellist.size(); ichan++)
		{
			if(!allchannels && channel != thechannellist[ichan]) {continue;}

			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					bool isData = sample_list[isample].Contains("Data");

					if(isData && syst_list[isyst] != "") {continue;}

					//cout<<thechannellist[ichan]<<" / "<<sample_list[isample]<<" / "<<syst_list[isyst]<<endl;

					h_tmp = 0;

					TString histo_name = "Control_" + thechannellist[ichan] + "_"+ var_list[ivar] + "_" + sample_list[isample];
					if(syst_list[isyst] != "") {histo_name+= "_" + syst_list[isyst];}
					if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}

					h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
					//cout<<"htmp = "<<h_tmp<<endl;
					//cout<<__LINE__<<endl;

					if(isData)
					{
						if(h_data == 0) {h_data = (TH1F*) h_tmp->Clone();}
						else {h_data->Add(h_tmp);}
						continue;
					}

					//SINCE WE USE ONLY ONE SAMPLE_LIST FOR DATA AND MC, IT MAKES A DIFFERENCE WHETHER DATA SAMPLE IS ACTIVATED OR NOT
					//if indeed we also run on data, then for the following samples (= MC) we need to do iterator-= 1 in order to start the MC-dedicated vector at 0 !
					if(sample_list[0].Contains("Data")) {isample = isample - 1;} //If the sample_list contains the data

					h_tmp->SetFillStyle(1001);
					h_tmp->SetFillColor(colorVector[isample]);
					h_tmp->SetLineColor(colorVector[isample]);

					if(!isData && syst_list[isyst] == "" && niter_chan==0)
					{
						v_MC_histo.push_back(h_tmp);

					}

					else if(!isData && syst_list[isyst] == "") //niter_chan != 0
					{
						v_MC_histo[isample]->Add(h_tmp);
					}

					else if(!isData && niter_chan==0) // syst_list[isyst] != ""
					{
						if(syst_list[isyst] == "JER__plus") {v_MC_histo_JER_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "JER__minus") {v_MC_histo_JER_minus.push_back(h_tmp);}
						else if(syst_list[isyst] == "JES__plus") {v_MC_histo_JES_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "JES__minus") {v_MC_histo_JES_minus.push_back(h_tmp);}
						else if(syst_list[isyst] == "PU__plus") {v_MC_histo_PU_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "PU__minus") {v_MC_histo_PU_minus.push_back(h_tmp);}
						else if(syst_list[isyst] == "Q2__plus") {v_MC_histo_Q2_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "Q2__minus") {v_MC_histo_Q2_minus.push_back(h_tmp);}
						else if(syst_list[isyst] == "MuEff__plus") {v_MC_histo_MuEff_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "MuEff__minus") {v_MC_histo_MuEff_minus.push_back(h_tmp);}
						else if(syst_list[isyst] == "EleEff__plus") {v_MC_histo_EleEff_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "EleEff__minus") {v_MC_histo_EleEff_minus.push_back(h_tmp);}
						else if(syst_list[isyst] != "") {cout<<"Unknow systematic name"<<endl;}
					}
					else if(!isData)// syst_list[isyst] != "" && niter_chan != 0
					{
						if(syst_list[isyst] == "JER__plus") {v_MC_histo_JER_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "JER__minus") {v_MC_histo_JER_minus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "JES__plus") {v_MC_histo_JES_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "JES__minus") {v_MC_histo_JES_minus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "PU__plus") {v_MC_histo_PU_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "PU__minus") {v_MC_histo_PU_minus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "Q2__plus") {v_MC_histo_Q2_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "Q2__minus") {v_MC_histo_Q2_minus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "MuEff__plus") {v_MC_histo_MuEff_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "MuEff__minus") {v_MC_histo_MuEff_minus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "EleEff__plus") {v_MC_histo_EleEff_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "EleEff__minus") {v_MC_histo_EleEff_minus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] != "") {cout<<"Unknow systematic name"<<endl;}

					}

					if(sample_list[0].Contains("Data")) {isample = isample + 1;} //If we have de-incremented isample (cf. above), then we need to re-increment it here so we don't get an infinite loop !!

				} //end syst loop

			} //end sample loop

			niter_chan++;

		} //end channel loop

		//Stack all the MC nominal histograms (contained in v_MC_histo)
		for(int i=0; i<v_MC_histo.size(); i++)
		{
			if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
			else {stack->Add(v_MC_histo[i]);}

			//NB : use (i+1) because sample_list also contains the data sample in first position
			if(sample_list[i+1] != "WW" && sample_list[i+1] != "ZZ") {qw->AddEntry(v_MC_histo[i], sample_list[i+1].Data() , "f");} //Need to keep ordered so that sample_list[i] <-> vector_MC[i]
			//else if(sample_list[i] == "ttZ") {qw->AddEntry(v_MC_histo[i+1], "ttV" , "f");}
			else if(sample_list[i+1] == "ZZ") {qw->AddEntry(v_MC_histo[i], "VV" , "f");}
		}

		if(!stack) {cout<<__LINE__<<" : stack is null"<<endl;}

		//---------------------------
		//DRAW HISTOS
		//---------------------------

		c1->cd();

		//Set Yaxis maximum
		if(h_data != 0 && stack != 0)
		{
			if(h_data->GetMaximum() > stack->GetMaximum() ) {stack->SetMaximum(h_data->GetMaximum()+0.3*h_data->GetMaximum());}
			else stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum());
		}

		//Draw stack
		if(stack != 0) {stack->Draw("HIST"); stack->GetXaxis()->SetLabelSize(0.0);}

		//Draw data
		if(h_data != 0)
		{
		  h_data->SetMarkerStyle(20);
		  h_data->SetMarkerSize(1.2);
		  h_data->SetLineColor(1);
		  h_data->Draw("e0psame");
		}
		else {cout<<__LINE__<<" : h_data is null"<<endl;}

		//--------------------------
		//MC SYSTEMATICS PLOT
		//--------------------------
		//Create a temporary TH1F* in order to put all the systematics into it via SetBinError, then create a TGraphError from this TH1F*
		//Also used to compute the Data/MC ratio
		TH1F* histo_syst_MC = 0;
		TH1F* histo_syst_MC_JER_plus = 0;  TH1F* histo_syst_MC_JER_minus = 0;
		TH1F* histo_syst_MC_JES_plus = 0;  TH1F* histo_syst_MC_JES_minus = 0;
		TH1F* histo_syst_MC_PU_plus = 0;  TH1F* histo_syst_MC_PU_minus = 0;
		TH1F* histo_syst_MC_Q2_plus = 0;  TH1F* histo_syst_MC_Q2_minus = 0;
		TH1F* histo_syst_MC_MuEff_plus = 0;  TH1F* histo_syst_MC_MuEff_minus = 0;
		TH1F* histo_syst_MC_EleEff_plus = 0;  TH1F* histo_syst_MC_EleEff_minus = 0;
		//TH1F* histo_syst_MC_test_plus = 0;  TH1F* histo_syst_MC_test_minus = 0;
		for(unsigned int imc=0; imc < v_MC_histo.size(); imc++) //Clone or Add histograms
		{
			if(histo_syst_MC == 0) {histo_syst_MC = (TH1F*) v_MC_histo[imc]->Clone();}
			else {histo_syst_MC->Add(v_MC_histo[imc]);}

			//cout<<"v_MC_histo[imc]->GetBinContent(1) = "<<v_MC_histo[imc]->GetBinContent(1)<<endl;
			//cout<<"histo_syst_MC->GetBinContent(1) = "<<histo_syst_MC->GetBinContent(1)<<endl;

			if(v_MC_histo_JER_plus.size() == v_MC_histo.size()) //If the syst. is taken into account, then both vectors should have same size
			{
				if(histo_syst_MC_JER_plus == 0) {histo_syst_MC_JER_plus = (TH1F*) v_MC_histo_JER_plus[imc]->Clone();}
				else {histo_syst_MC_JER_plus->Add(v_MC_histo_JER_plus[imc]);}
				if(histo_syst_MC_JER_minus == 0) {histo_syst_MC_JER_minus = (TH1F*) v_MC_histo_JER_minus[imc]->Clone();}
				else {histo_syst_MC_JER_minus->Add(v_MC_histo_JER_minus[imc]);}
			}

			if(v_MC_histo_JES_plus.size() == v_MC_histo.size())
			{
				if(histo_syst_MC_JES_plus == 0) {histo_syst_MC_JES_plus = (TH1F*) v_MC_histo_JES_plus[imc]->Clone();}
				else {histo_syst_MC_JES_plus->Add(v_MC_histo_JES_plus[imc]);}
				if(histo_syst_MC_JES_minus == 0) {histo_syst_MC_JES_minus = (TH1F*) v_MC_histo_JES_minus[imc]->Clone();}
				else {histo_syst_MC_JES_minus->Add(v_MC_histo_JES_minus[imc]);}
			}
			if(v_MC_histo_PU_plus.size() == v_MC_histo.size())
			{
				if(histo_syst_MC_PU_plus == 0) {histo_syst_MC_PU_plus = (TH1F*) v_MC_histo_PU_plus[imc]->Clone();}
				else {histo_syst_MC_PU_plus->Add(v_MC_histo_PU_plus[imc]);}
				if(histo_syst_MC_PU_minus == 0) {histo_syst_MC_PU_minus = (TH1F*) v_MC_histo_PU_minus[imc]->Clone();}
				else {histo_syst_MC_PU_minus->Add(v_MC_histo_PU_minus[imc]);}
			}
			if(v_MC_histo_Q2_plus.size() == v_MC_histo.size())
			{
				if(histo_syst_MC_Q2_plus == 0) {histo_syst_MC_Q2_plus = (TH1F*) v_MC_histo_Q2_plus[imc]->Clone();}
				else {histo_syst_MC_Q2_plus->Add(v_MC_histo_Q2_plus[imc]);}
				if(histo_syst_MC_Q2_minus == 0) {histo_syst_MC_Q2_minus = (TH1F*) v_MC_histo_Q2_minus[imc]->Clone();}
				else {histo_syst_MC_Q2_minus->Add(v_MC_histo_Q2_minus[imc]);}
			}
			if(v_MC_histo_MuEff_plus.size() == v_MC_histo.size())
			{
				if(histo_syst_MC_MuEff_plus == 0) {histo_syst_MC_MuEff_plus = (TH1F*) v_MC_histo_MuEff_plus[imc]->Clone();}
				else {histo_syst_MC_MuEff_plus->Add(v_MC_histo_MuEff_plus[imc]);}
				if(histo_syst_MC_MuEff_minus == 0) {histo_syst_MC_MuEff_minus = (TH1F*) v_MC_histo_MuEff_minus[imc]->Clone();}
				else {histo_syst_MC_MuEff_minus->Add(v_MC_histo_MuEff_minus[imc]);}
			}
			if(v_MC_histo_EleEff_plus.size() == v_MC_histo.size())
			{
				if(histo_syst_MC_EleEff_plus == 0) {histo_syst_MC_EleEff_plus = (TH1F*) v_MC_histo_EleEff_plus[imc]->Clone();}
				else {histo_syst_MC_EleEff_plus->Add(v_MC_histo_EleEff_plus[imc]);}
				if(histo_syst_MC_EleEff_minus == 0) {histo_syst_MC_EleEff_minus = (TH1F*) v_MC_histo_EleEff_minus[imc]->Clone();}
				else {histo_syst_MC_EleEff_minus->Add(v_MC_histo_EleEff_minus[imc]);}
			}
		}
		int nofbin = histo_syst_MC->GetNbinsX();

		//Add up here the different errors (quadratically), for each bin separately
		for(int ibin=1; ibin<nofbin+1; ibin++) //Start at bin 1
		{
			double err_up = 0;
			double err_low = 0;
			double tmp = 0;

			//For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding error
			//--------------------------

			//JER
			if(histo_syst_MC_JER_plus != 0)
			{
				tmp = histo_syst_MC_JER_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
				tmp = histo_syst_MC_JER_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
			}
			//JES
			if(histo_syst_MC_JES_plus != 0)
			{
				tmp = histo_syst_MC_JES_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
				tmp = histo_syst_MC_JES_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
			}
			//PU
			if(histo_syst_MC_PU_plus != 0)
			{
				tmp = histo_syst_MC_PU_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
				tmp = histo_syst_MC_PU_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
			}
			//Q2 - Scale
			if(histo_syst_MC_Q2_plus != 0)
			{
				tmp = histo_syst_MC_Q2_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
				tmp = histo_syst_MC_Q2_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
			}
			//Muon Efficiency SF
			if(histo_syst_MC_MuEff_plus != 0)
			{
				tmp = histo_syst_MC_MuEff_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
				tmp = histo_syst_MC_MuEff_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
			}
			//Electron Efficiency SF
			if(histo_syst_MC_EleEff_plus != 0)
			{
				tmp = histo_syst_MC_EleEff_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
				tmp = histo_syst_MC_EleEff_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
				if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
				else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
			}
			//Luminosity (set to 4% of bin content)
			err_up+= pow(histo_syst_MC->GetBinContent(ibin)*0.04, 2);
			err_low+= pow(histo_syst_MC->GetBinContent(ibin)*0.04, 2);
			//MC Statistical uncertainty
			err_up+= 	pow(histo_syst_MC->GetBinError(ibin), 2);
			err_low+= 	pow(histo_syst_MC->GetBinError(ibin), 2);

			//cout<<"histo_syst_MC->GetBinError(ibin) = "<<histo_syst_MC->GetBinError(ibin)<<endl;

			//--------------------------
			//Take sqrt
			err_up = pow(err_up, 0.5); //cout<<"err_up = "<<err_up<<endl;
			err_low = pow(err_low, 0.5); //cout<<"err_low = "<<err_low<<endl;

			//Fill error vectors (one per bin)
			v_eyh.push_back(err_up);
			v_eyl.push_back(err_low);
			v_exl.push_back(histo_syst_MC->GetXaxis()->GetBinWidth(ibin) / 2);
			v_exh.push_back(histo_syst_MC->GetXaxis()->GetBinWidth(ibin) / 2);
			v_x.push_back( (histo_syst_MC->GetXaxis()->GetBinLowEdge(nofbin+1) - histo_syst_MC->GetXaxis()->GetBinLowEdge(1) ) * ((ibin - 0.5)/nofbin) + histo_syst_MC->GetXaxis()->GetBinLowEdge(1));
			v_y.push_back(histo_syst_MC->GetBinContent(ibin)); //see warning above about THStack and negative weights

			//if(ibin > 1) {continue;} //display only first bin
			//cout<<"x = "<<v_x[ibin-1]<<endl;    cout<<", y = "<<v_y[ibin-1]<<endl;    cout<<", eyl = "<<v_eyl[ibin-1]<<endl;    cout<<", eyh = "<<v_eyh[ibin-1]<<endl; cout<<", exl = "<<v_exl[ibin-1]<<endl;    cout<<", exh = "<<v_exh[ibin-1]<<endl;
		}

		//Pointers to vectors : need to give the adress of first element (all other elements then can be accessed iteratively)
		double* eyl = &v_eyl[0];
		double* eyh = &v_eyh[0];
		double* exl = &v_exl[0];
		double* exh = &v_exh[0];
		double* x = &v_x[0];
		double* y = &v_y[0];

		//Create TGraphAsymmErrors with the error vectors / (x,y) coordinates
		TGraphAsymmErrors* gr = 0;
		gr = new TGraphAsymmErrors(nofbin,x,y,exl,exh,eyl,eyh);
		gr->SetFillStyle(3005);
		gr->SetFillColor(1);
		gr->Draw("e2 same"); //Superimposes the systematics uncertainties on stack
		//cout << __LINE__ << endl;


		//-------------------
		//LEGEND AND CAPTIONS
		//-------------------

		TLatex* latex = new TLatex();
		latex->SetNDC();
		latex->SetTextSize(0.04);
		latex->SetTextAlign(31);
		latex->DrawLatex(0.45, 0.95, "CMS Preliminary");

		 // cout << __LINE__ << endl;

		TLatex* latex2 = new TLatex();
		latex2->SetNDC();
		latex2->SetTextSize(0.04);
		latex2->SetTextAlign(31);
		//------------------
		int lumi = 2.26 * luminosity_rescale;
		TString lumi_ts = Convert_Number_To_TString(lumi);
		lumi_ts+= " fb^{-1} at #sqrt{s} = 13 TeV";
		latex2->DrawLatex(0.87, 0.95, lumi_ts.Data());
		//------------------

		TString info_data;
		if (channel=="eee")    info_data = "eee channel";
		else if (channel=="eeu")  info_data = "ee#mu channel";
		else if (channel=="uue")  info_data = "#mu#mu e channel";
		else if (channel=="uuu") info_data = "#mu#mu #mu channel";
		else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";

		TLatex* text2 = new TLatex(0.45,0.98, info_data);
		text2->SetNDC();
		text2->SetTextAlign(13);
		text2->SetX(0.18);
		text2->SetY(0.92);
		text2->SetTextFont(42);
		text2->SetTextSize(0.0610687);
		//text2->SetTextSizePixels(24);// dflt=28
		text2->Draw();

		qw->Draw();


		//--------------------------
		//DATA OVER BACKGROUND RATIO
		//--------------------------
		//Create Data/MC ratio plot (bottom of canvas)
		if(h_data != 0 && v_MC_histo.size() != 0) //Need both data and MC
		{
			TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
			canvas_2->SetTopMargin(0.7);
			canvas_2->SetFillColor(0);
			canvas_2->SetFillStyle(0);
			canvas_2->SetGridy(1);
			canvas_2->Draw();
			canvas_2->cd(0);

			TH1F * histo_ratio_data = (TH1F*) h_data->Clone();

			histo_ratio_data->Divide(histo_syst_MC);

			//if(var_list[ivar] == "METpt")   histo_ratio_data->GetXaxis()->SetTitle(title_MET.Data());
			histo_ratio_data->GetXaxis()->SetTitle(var_list[ivar].Data());

			histo_ratio_data->SetMinimum(0.0);
			histo_ratio_data->SetMaximum(2.0);
			histo_ratio_data->GetXaxis()->SetTitleOffset(1.2);
			histo_ratio_data->GetXaxis()->SetLabelSize(0.04);
			histo_ratio_data->GetYaxis()->SetLabelSize(0.03);
			histo_ratio_data->GetYaxis()->SetNdivisions(6);
			histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
			histo_ratio_data->Draw("E1X0");

			//Copy previous TGraphAsymmErrors
			TGraphAsymmErrors *thegraph_tmp = (TGraphAsymmErrors*) gr->Clone();

			double *theErrorX_h = thegraph_tmp->GetEXhigh();
			double *theErrorY_h = thegraph_tmp->GetEYhigh();
			double *theErrorX_l = thegraph_tmp->GetEXlow();
			double *theErrorY_l = thegraph_tmp->GetEYlow();
			double *theY        = thegraph_tmp->GetY() ;
			double *theX        = thegraph_tmp->GetX() ;

			//Divide error --> ratio
			for(int i=0; i<thegraph_tmp->GetN(); i++)
			{
			  theErrorY_l[i] = theErrorY_l[i]/theY[i];
			  theErrorY_h[i] = theErrorY_h[i]/theY[i];
			  theY[i]=1; //To center the filled area around "1"
			}

			//--> Create new TGraphAsymmErrors
			TGraphAsymmErrors *thegraph_ratio = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
			thegraph_ratio->SetFillStyle(3005);
			thegraph_ratio->SetFillColor(1);

			thegraph_ratio->Draw("e2 same"); //Syst. error for Data/MC ; drawn on canvas2 (Data/MC ratio)
		}

		//Yaxis title
		if(stack!= 0) {stack->GetYaxis()->SetTitleSize(0.04); stack->GetYaxis()->SetTitle("Events");}

		//-------------------
		//OUTPUT
		//-------------------

		//Image name (.png)
		//TString outputname = "plots"+filename_suffix+"/"+var_list[ivar]+"_"+channel+".png";
		TString outputname = "plots/"+var_list[ivar]+"_"+channel+".png";
		if(channel == "" || allchannels) {outputname = "plots/"+var_list[ivar]+"_all.png";}

		//cout << __LINE__ << endl;
		if(c1!= 0) {c1->SaveAs(outputname.Data() );}
		//cout << __LINE__ << endl;

		delete c1; //Must free dinamically-allocated memory
	} //end var loop

}



//Plot stacked BDT templates VS pseudo-data --> check if seems OK
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Plot_BDT_Templates(TString channel)
{
	TString input_name = "outputs/Reader" + filename_suffix + ".root";
	TFile* file_input = TFile::Open( input_name.Data() );

	TH1F *h_sum_MC = 0, *h_tmp = 0, *h_sum_data = 0;

	for(int isample = 0; isample < sample_list.size(); isample++)
	{
		if(sample_list[isample].Contains("Data")) {continue;}

		h_tmp = 0;
		TString histo_name = "BDT_" + channel + "__" + sample_list[isample];
		if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}
		h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
		if(h_sum_MC == 0) {h_sum_MC = (TH1F*) h_tmp->Clone();}
		else {h_sum_MC->Add(h_tmp);}
	}

	h_tmp = 0;
	TString histo_name = "BDT_" + channel + "__DATA";
	h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
	if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
	else {h_sum_data->Add(h_tmp);}


	//Canvas definition
	Load_Canvas_Style();

	h_sum_MC->SetLineColor(kRed);

	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
	h_sum_MC->Draw("hist");
	h_sum_data->Draw("epsame");
	TString output_plot_name = "plots/BDT_template"+ filename_suffix + channel + ".png";
	c1->SaveAs(output_plot_name.Data());

	delete c1;
}

//Plot stacked BDT templates VS pseudo-data --> check if seems OK
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Plot_BDT_Templates_allchannels()
{
	TString MC_input_name = "outputs/output_Reader.root";
	TFile* file_input_MC = TFile::Open( MC_input_name.Data() );
	TString pseudodata_input_name = "outputs/output_PseudoData.root";
	TFile* file_input_pseudodata = TFile::Open( pseudodata_input_name.Data() );

	TH1F *h_sum_MC = 0, *h_tmp = 0, *h_sum_data = 0;

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			//if(sample_list[isample] != "WZ" && sample_list[isample] != "ttZ" && sample_list[isample] != "ZZ" && sample_list[isample] != "tZq") {continue;}

			h_tmp = 0;
			TString histo_name = "BDT_" + channel_list[ichan] + "__" + sample_list[isample];
			if(!file_input_MC->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}
			h_tmp = (TH1F*) file_input_MC->Get(histo_name.Data())->Clone();
			if(h_sum_MC == 0) {h_sum_MC = (TH1F*) h_tmp->Clone();}
			else {h_sum_MC->Add(h_tmp);}
		}

		h_tmp = 0;
		TString histo_name = "BDT_" + channel_list[ichan] + "__DATA";
		if(!file_input_pseudodata->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}
		h_tmp = (TH1F*) file_input_pseudodata->Get(histo_name.Data())->Clone();
		if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
		else {h_sum_data->Add(h_tmp);}
	}

	//Canvas definition
	Load_Canvas_Style();

	h_sum_MC->SetLineColor(kRed);

	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
	h_sum_MC->Draw("hist");
	h_sum_data->Draw("epsame");
	c1->SaveAs("plots/plot_bdt_template.png");

	delete c1;
}
