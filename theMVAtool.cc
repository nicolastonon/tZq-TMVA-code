#include "theMVAtool.h"

#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"

using namespace std;


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
theMVAtool::theMVAtool()
{
	//channel_list.push_back("");

	sample_list.push_back("tZq");
	sample_list.push_back("WZ");

	var_list.push_back("mTW"); vec_variables.push_back(0); colorVector.push_back(0);
	var_list.push_back("METpt"); vec_variables.push_back(0); colorVector.push_back(0);
	var_list.push_back("ZCandMass"); vec_variables.push_back(0); colorVector.push_back(0);

	syst_list.push_back("");

	// --- Boosted Decision Trees
	Use["BDT"]             = 1; // uses Adaptive Boost
	Use["BDTG"]            = 0; // uses Gradient Boost

	reader = new TMVA::Reader( "!Color:!Silent" );

	nbin = 50;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
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

	// --- Boosted Decision Trees
	Use["BDT"]             = 1; // uses Adaptive Boost
	Use["BDTG"]            = 0; // uses Gradient Boost

	reader = new TMVA::Reader( "!Color:!Silent" );

	nbin = 50;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Set_MVA_Methods(string method, bool isActivated)
{
	Use[method] = isActivated;

	if(isActivated) cout<<"You activated the method"<<method<<endl;
	else cout<<"You dis-activated the method"<<method<<endl;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Train_Test_Evaluate(TString channel)
{
	//---------------------------------------------------------------
    // This loads the library
    TMVA::Tools::Instance();

	TString output_file_name = "outputs/output_Train";
	if(channel != "") {output_file_name+= "_" + channel;}
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
		if(sample_list[isample].Contains("Data")) {continue;} //Train only on MC

        // Read training and test data
        f = 0;
		TString inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
		//TString inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + "_" + channel + "_" + ".root";
        f = TFile::Open( inputfile );

        if (!f)
        {
          std::cout << "ERROR: could not open data file" << std::endl;
          exit(1);
        }
        std::cout << "--- Using input file: " << f->GetName() << std::endl;

        // --- Register the training and test trees
        TTree* tree = (TTree*)f->Get("Default");

        // global event weights per tree (see below for setting event-wise weights)
        Double_t signalWeight     = 1.0;
        Double_t backgroundWeight = 1.0;

        // You can add an arbitrary number of signal or background trees
        if(sample_list[isample] == "tZq") {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "Weight" );}
        else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "Weight" );}
    }

    // Apply additional cuts on the signal and background samples (can be different)
    TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
    TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

    // Tell the factory how to use the training and testing events    //
    // If no numbers of events are given, half of the events in the tree are used for training, and the other half for testing:
    factory->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
cout<<__LINE__<<endl;
    // Boosted Decision Trees
    if (Use["BDT"])  // Adaptive Boost
    factory->BookMethod( TMVA::Types::kBDT, "BDT",    "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );
	cout<<__LINE__<<endl;

    if (Use["BDTG"]) // Gradient Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDTG","!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );
	output_file->cd();

    // Train MVAs using the set of training events
    factory->TrainAllMethods();
cout<<__LINE__<<endl;
    // ---- Evaluate all MVAs using the set of test events
    factory->TestAllMethods();
cout<<__LINE__<<endl;
    // ----- Evaluate and compare performance of all configured MVAs
    factory->EvaluateAllMethods();

    // --------------------------------------------------------------

    // Save the output
    output_file->Close();

    std::cout << "==> Wrote root file: " << output_file->GetName() << std::endl;
    std::cout << "==> TMVA is done!" << std::endl;

    delete factory;

    // Launch the GUI for the root macros    //NB : write interactively in the ROOT environment --> TMVA::TMVAGui("output.root")
    //TMVA::TMVAGui(output_file_name);
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Reader function. Uses output from training (weights, ...) and read samples to create distributions of the BDT discriminant
void theMVAtool::Read(TString channel)
{
	TString output_file_name = "outputs/output_Reader.root";
	//TString output_file_name = "output_Reader_" + channel + ".root";
	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );

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
	TString prefix = "BDT"; // use factory title

	// Book each method activated in the map
	for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++)
	{
		cout<<"it = "<<&it<<endl;
	   if (it->second) //If set to "1"
	   {
		  TString methodName = TString(it->first) + TString(" method");
		  TString weightfile = dir + prefix + TString("_") + TString(it->first) + TString(".weights.xml");
		  //TString weightfile = dir + prefix + TString("_") + TString(it->first) + TString(".weights.xml");
		  reader->BookMVA( methodName, weightfile );
	   }
	}
	//cout<<__LINE__<<endl;


	for(int isample=0; isample<sample_list.size(); isample++)
	{
		TString filename = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
		TFile *f(0);
		TTree* tree(0);

		f = TFile::Open( filename );
		if (!f)
		{
		  std::cout << "ERROR: could not open data file" << std::endl;
		  exit(1);
		}
		std::cout << "--- Using input file: " << f->GetName() << std::endl;

		// Book output histgrams
		hist_BDT = 0; hist_BDTG = 0;
		if (Use["BDT"]) {hist_BDT     = new TH1F( "MVA_BDT",           "MVA_BDT",           nbin, -0., 0.25 );}
		if (Use["BDTG"]) {hist_BDTG    = new TH1F( "MVA_BDTG",          "MVA_BDTG",          nbin, -0., 0.25 );}

		// --- Systematics loop
		for(int isyst=0; isyst<syst_list.size(); isyst++)
		{
			if(sample_list[isample].Contains("Data") && syst_list[isyst]!="") {continue;}

			// Prepare the event tree
			std::cout << "--- Select "<<sample_list[isample]<<" sample" << std::endl;
			if(syst_list[isyst]=="") {tree = (TTree*)f->Get("Default");}
			else {tree = (TTree*)f->Get(syst_list[isyst].Data());}

			for(int i=0; i<var_list.size(); i++)
			{
				tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
			}
			float weight; tree->SetBranchAddress("Weight", &weight);


			std::cout << "--- Processing: " << tree->GetEntries() << " events" << std::endl;
			TStopwatch sw; //This class returns the real and cpu time between the start and stop events.
			sw.Start();

			//cout<<__LINE__<<endl;

			// --- Event loop
			for(int ievt=0; ievt<tree->GetEntries(); ievt++)
			{
				//if(ievt%10000==0) {cout<<endl<<"Event : "<<ievt<<" / "<<tree->GetEntries()<<endl;}

				tree->GetEntry(ievt);

				// --- Return the MVAœ outputs and fill into histograms
				if (Use["BDT"]) {hist_BDT->Fill( reader->EvaluateMVA( "BDT method"), weight);}
				if (Use["BDTG"]) {hist_BDTG->Fill( reader->EvaluateMVA( "BDTG method"), weight);}
			}

			// Get elapsed time
			sw.Stop();
			std::cout << "--- End of event loop: "; sw.Print();

			// --- Write histograms

			output_file->cd();

			//NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
			TString output_histo_name = "BDT__" + sample_list[isample];
			if(channel != "") {output_histo_name+= "_" + channel;}
			TString syst_name;
			if(syst_list[isyst]!="")
			{
				if(syst_list[isyst]!="JERup") syst_name = "JER__plus";
				else if(syst_list[isyst]!="JERdn") syst_name = "JER__minus";
				output_histo_name+= "__" + syst_name;
			}


			if (Use["BDT" ])  {hist_BDT->Write(output_histo_name.Data());}
			if (Use["BDTG" ]) {hist_BDTG->Write(output_histo_name.Data());}
		}

		cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
	}

	output_file->Close();

	std::cout << "--- Created root file: \""<<output_file->GetName()<<"\" containing the MVA output histograms" << std::endl;

	std::cout << "==> Reader() is done!" << std::endl << std::endl;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Read the histograms output from Read() and determine cut value on discriminant to obtain desired signal efficiency
// --> Can use this value as input parameter in Read() to create additionnal "control histograms"
void theMVAtool::Determine_Control_Cut(TString channel)
{
	TString input_file_name = "outputs/output_Reader.root";
	TFile* f = TFile::Open(input_file_name.Data());

	TH1F *h_sum_bkg(0), *h_sig(0), *h_tmp(0);

	TString input_histo_name = "";

	for(int isample=0; isample<sample_list.size(); isample++)
	{
		if(sample_list[isample].Contains("Data")) {continue;}

		h_tmp = 0;
		input_histo_name = "BDT__" + sample_list[isample];
		if(channel != "") {input_histo_name+= "_" + channel;}

		h_tmp = (TH1F*) f->Get(input_histo_name.Data());
		if(sample_list[isample] == "tZq") {h_sig = (TH1F*) h_tmp->Clone();}
		else if(h_sum_bkg == 0) {h_sum_bkg = (TH1F*) h_tmp->Clone();}
		else {h_sum_bkg->Add(h_tmp);}
	}

	//Normalization
	h_sum_bkg->Scale(1/h_sum_bkg->Integral());
	h_sig->Scale(1/h_sig->Integral());

	double cut = 0;
	double sig_over_bkg = 100;
	int bin_cut = 0;
	double step = 0.0025;

	for(double cut_tmp = 0; cut_tmp<0.2; cut_tmp+=step) //Try different cuts, see which one minimizes sig/bkg (and keeping enough bkg events)
	{
		bin_cut = h_sum_bkg->GetXaxis()->FindBin(cut_tmp);

		if(fmod(cut_tmp, 0.01) < pow(10, -10)) //fmod seems to output very small values (10^-17, ...) rather than 0 ! //cout every 10 iterations
		{
			//cout<<"--- Cut = "<<cut_tmp<<endl;
			//cout<<"Signal integral = "<<h_sig->Integral(1, bin_cut)<<" / Background integral "<<h_sum_bkg->Integral(1, bin_cut)<<endl;
			//cout<<" --> Sig/Bkg = "<<h_sig->Integral(1, bin_cut)/h_sum_bkg->Integral(1, bin_cut)<<endl<<endl;
		}

		if( (h_sig->Integral(1, bin_cut) / h_sum_bkg->Integral(1, bin_cut)) <= sig_over_bkg && h_sum_bkg->Integral(1, bin_cut) >= 0.4 ) //arbitrary criterions for determining suitable cut value
		{
			cut = cut_tmp;
			sig_over_bkg = h_sig->Integral(1, bin_cut) / h_sum_bkg->Integral(1, bin_cut);
		}
	}

	TCanvas* c = new TCanvas("c", "Signal VS Background");
	gStyle->SetOptStat(0);
	h_sum_bkg->GetXaxis()->SetTitle("Discriminant");
	h_sum_bkg->SetTitle("Signal VS Background");
	h_sum_bkg->SetLineColor(kBlue);
	h_sig->SetLineColor(kGreen);
	h_sum_bkg->Draw();
	h_sig->Draw("same");

	TLegend* leg = new TLegend(0.7,0.75,0.88,0.85);
    leg->SetHeader("");
    leg->AddEntry(h_sig,"Signal","L");
    leg->AddEntry("h_sum_bkg","Background","L");
    leg->Draw();

	//Draw vertical line at cut value
	TLine* l = new TLine(cut, 0, cut, h_sum_bkg->GetMaximum());
	l->SetLineWidth(3);
	l->Draw("");

	c->SaveAs("outputs/Signal_Background_BDT.png");

	bin_cut = h_sum_bkg->GetXaxis()->FindBin(cut);
	cout<<"---------------------------------------"<<endl;
	cout<<"* Cut Value = "<<cut<<endl;
	//Since histograms are binned, need to apply the MVA cut at a *bin edge* in order to correctly estimate efficiency via Integral(bin_low, bin_up) !
	cout<<"---> Cut at upper edge of associated bin = "<<h_sum_bkg->GetBinLowEdge(bin_cut+1)<<" !"<<endl;
	cout<<"* Signal integral = "<<h_sig->Integral(1, bin_cut)<<" / Background integral "<<h_sum_bkg->Integral(1, bin_cut)<<endl;
	//cout<<"Signal integral = "<<h_sig->Integral(1, bin_cut+1)<<" / Background integral "<<h_sum_bkg->Integral(1, bin_cut)<<endl;
	//cout<<"Signal integral = "<<h_sig->Integral(1, bin_cut-1)<<" / Background integral "<<h_sum_bkg->Integral(1, bin_cut)<<endl;
	cout<<"---> Sig/Bkg = "<<h_sig->Integral(1, bin_cut)/h_sum_bkg->Integral(1, bin_cut)<<endl;
	cout<<"---------------------------------------"<<endl<<endl;

	//cout<<"signal entries "<<h_sig->GetEntries()<<endl;

	f->Close();
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Similar to Read(). Takes cut value as input parameter, and outputs histograms containing only events verifying BDT<cut (mainly bkg events) --> Create histogram with these events for further control studies
void theMVAtool::Create_Control_Histograms(TString channel, double cut)
{
	TString output_file_name = "outputs/output_Control.root";
	//TString output_file_name = "output_Reader_" + channel + ".root";
	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );
	TTree* tree_control = 0;

	reader = new TMVA::Reader( "!Color:!Silent" );

	// Name & adress of local variables which carry the updated input values during the event loop
	// - the variable names MUST corresponds in name and type to those given in the weight file(s) used
	for(int i=0; i<var_list.size(); i++)
	{
		reader->AddVariable(var_list[i].Data(), &(vec_variables[i])); //cout<<"Added variable "<<var_list[i]<<endl;
	}

	// --- Book the MVA methods
	TString dir    = "weights/";
	TString prefix = "BDT"; // use factory title

	// Book each method activated in the map
	for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++)
	{
		cout<<"it = "<<&it<<endl;
	   if (it->second) //If set to "1"
	   {
		  TString methodName = TString(it->first) + TString(" method"); // "BDT method"
		  TString weightfile = dir + prefix + TString("_") + TString(it->first) + TString(".weights.xml");
		  reader->BookMVA( methodName, weightfile );
	   }
	}
	//cout<<__LINE__<<endl;

	for(int isyst = 0; isyst < syst_list.size(); isyst++)
	{
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			if(sample_list[isample].Contains("Data") && syst_list[isample] != "") {continue;}

			TString filename = "Ntuples/FCNCNTuple_" + sample_list[isample];
			//filename+= "_" + channel;
			filename+= ".root";
			TFile *f(0);
			TTree* tree(0);
			tree_control = 0;
			float weight;

			//Create new tree, that will be filled only with events verifying MVA<cut
			tree_control = new TTree("tree_control", "Tree filled with events verifying MVA<cut");
			for(int ivar=0; ivar<var_list.size(); ivar++)
			{
				TString var_type = var_list[ivar] + "/F";
				tree_control->Branch(var_list[ivar].Data(), &(vec_variables[ivar]), var_type.Data());
			}
			tree_control->Branch("Weight", &weight, "weight/F");


			f = TFile::Open( filename );
			if (!f)
			{
			  std::cout << "ERROR: could not open file" << std::endl;
			  exit(1);
			}
			std::cout << "--- Using input file: " << f->GetName() << std::endl;

			// Prepare the event tree
			std::cout << "--- Select "<<sample_list[isample]<<" sample" << std::endl;

			TString tree_name;
			if(syst_list[isyst] == "") {tree_name = "Default";}
			else {tree_name = syst_list[isyst];}
			tree = (TTree*)f->Get(tree_name.Data());

			for(int i=0; i<var_list.size(); i++)
			{
				tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
			}
			tree->SetBranchAddress("Weight", &weight);

			std::cout << "--- Processing: " << tree->GetEntries() << " events" << std::endl;
			//cout<<__LINE__<<endl;

			// --- Event loop
			for(int ievt=0; ievt<tree->GetEntries(); ievt++)
			{
				//if(ievt%10000==0) {cout<<endl<<"Event : "<<ievt<<" / "<<tree->GetEntries()<<endl;}

				tree->GetEntry(ievt); //Fills vec_variables

				//cout<<"MVA = "<<reader->EvaluateMVA( "BDT method")<<endl;
				if (reader->EvaluateMVA( "BDT method") <= cut)  {tree_control->Fill();}
			}

			// --- Write histograms

			output_file->cd();

			//NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
			TString output_tree_name = "Control_" + sample_list[isample];
			//output_tree_name+= "_" + channel;
			if(syst_list[isyst] != "") {output_tree_name+= "_" + syst_list[isyst];}

			tree_control->Write(output_tree_name.Data());

			cout<<"Control Tree entries : "<<tree_control->GetEntries()<<endl;

			cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
		}
	}

	output_file->Close();

	std::cout << "--- Created root file: \""<<output_file->GetName()<<"\" containing the MVA output histograms" << std::endl;

	//delete tree_control;

	std::cout << "==> Reader() is done!" << std::endl << std::endl;
}


//Draw control plots with events contained in output_Control.root, generated by Create_Control_Histograms()
//Much inspired from code PlotStack.C (cf. NtupleAnalysis/src/...)
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Draw_Control_Plots(TString channel, bool allchannels)
{
	TString input_file_name = "outputs/output_Control.root";
	TFile* f = TFile::Open( input_file_name );
	TTree* tree = 0;
	TH1F *h_tmp = 0, *h_data = 0;
	THStack *stack = 0;

	TString title_MET = "Missing E_{T} [GeV]";
	TString title_mTW = "m_{T}(W) [GeV]";
	TString title_ZCandMass = "m_{ll} [GeV]";

	vector<TString> thechannellist;
	//thechannellist.push_back("uuu");
	//thechannellist.push_back("eee");
	//thechannellist.push_back("eeu");
	//thechannellist.push_back("uue");
	thechannellist.push_back("");

	//-----------------------------------
	//CANVAS DEFINITION
	//-----------------------------------

	// For the canvas:
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetCanvasColor(0); // must be kWhite but I dunno how to do that in PyROOT
	gStyle->SetCanvasDefH(600); //Height of canvas
	gStyle->SetCanvasDefW(600); //Width of canvas
	gStyle->SetCanvasDefX(0);   //POsition on screen
	gStyle->SetCanvasDefY(0);

	// For the Pad:
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadColor(0); // kWhite
	gStyle->SetPadGridX(0); //false
	gStyle->SetPadGridY(0); //false
	gStyle->SetGridColor(0);
	gStyle->SetGridStyle(3);
	gStyle->SetGridWidth(1);

	// For the frame:
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameBorderSize(1);
	gStyle->SetFrameFillColor(0);
	gStyle->SetFrameFillStyle(0);
	gStyle->SetFrameLineColor(1);
	gStyle->SetFrameLineStyle(1);
	gStyle->SetFrameLineWidth(1);

	// For the histo:
	gStyle->SetHistLineColor(1);
	gStyle->SetHistLineStyle(0);
	gStyle->SetHistLineWidth(1);
	gStyle->SetEndErrorSize(2);

	//For the fit/function:
	gStyle->SetOptFit(1011);
	gStyle->SetFitFormat("5.4g");
	gStyle->SetFuncColor(2);
	gStyle->SetFuncStyle(1);
	gStyle->SetFuncWidth(1);

	//For the date:
	gStyle->SetOptDate(0);

	// For the statistics box:
	gStyle->SetOptFile(0);
	gStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
	gStyle->SetStatColor(0); // kWhite
	gStyle->SetStatFont(42);
	gStyle->SetStatFontSize(0.04);
	gStyle->SetStatTextColor(1);
	gStyle->SetStatFormat("6.4g");
	gStyle->SetStatBorderSize(1);
	gStyle->SetStatH(0.1);
	gStyle->SetStatW(0.15);

	// Margins:
	gStyle->SetPadTopMargin(0.07);
	gStyle->SetPadBottomMargin(0.13);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadRightMargin(0.03);

	// For the Global title:
	gStyle->SetOptTitle(0);
	gStyle->SetTitleFont(42);
	gStyle->SetTitleColor(1);
	gStyle->SetTitleTextColor(1);
	gStyle->SetTitleFillColor(10);
	gStyle->SetTitleFontSize(0.05);
	// For the axis titles:
	gStyle->SetTitleColor(1, "XYZ");
	gStyle->SetTitleFont(42, "XYZ");
	gStyle->SetTitleSize(0.06, "XYZ");
	gStyle->SetTitleXOffset(0.9);
	gStyle->SetTitleYOffset(1.25);

	// For the axis labels:
	gStyle->SetLabelColor(1, "XYZ");
	gStyle->SetLabelFont(42, "XYZ");
	gStyle->SetLabelOffset(0.007, "XYZ");
	gStyle->SetLabelSize(0.05, "XYZ");

	// For the axis:
	gStyle->SetAxisColor(1, "XYZ");
	gStyle->SetStripDecimals(1); // kTRUE
	gStyle->SetTickLength(0.03, "XYZ");
	gStyle->SetNdivisions(510, "XYZ");
	gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
	gStyle->SetPadTickY(1);

	// Change for log plots:
	gStyle->SetOptLogx(0);
	gStyle->SetOptLogy(0);
	gStyle->SetOptLogz(0);

	// Postscript options:
	gStyle->SetPaperSize(20.,20.);

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
		c1->SetBottomMargin(0.3);

		h_data = 0; stack = 0;
		vector<TH1F*> v_MC_histo;
		//Idem for each systematics
		vector<TH1F*> v_MC_histo_JER_plus; vector<TH1F*> v_MC_histo_JER_minus;

		TLegend* qw = 0;
		qw = new TLegend(.80,.60,.95,.90);
		qw->SetShadowColor(0);
		qw->SetFillColor(0);
		qw->SetLineColor(0);

		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors

		//---------------------------
		//CREATE STACK (MC) AND DATA HISTO
		//---------------------------

		for(int ichan=0; ichan<thechannellist.size(); ichan++)
		{
			if(!allchannels && channel != thechannellist[ichan]) {continue;}

			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				h_tmp = 0;
				if(var_list[ivar] == "mTW") {h_tmp = new TH1F( "","", 60, 10, 130 );}
				else if(var_list[ivar] == "METpt") {h_tmp = new TH1F( "","", 60, -0., 120 );}
				else if(var_list[ivar] == "ZCandMass") {h_tmp = new TH1F( "","", 40, 70, 110 );}
				else {cout<<"Unknown variable"<<endl;}

				TString tree_name = "Control_" + sample_list[isample];
				//tree_name+= "_" + thechannellist[ichan];
				cout<<"--- Processing "<<tree_name<<endl;
				tree = (TTree*) f->Get(tree_name.Data());
				//cout<<__LINE__<<endl;

				int tree_nentries = tree->GetEntries();

				for(int ientry = 0; ientry<tree_nentries; ientry++)
				{
					float weight = 0;
					float tmp = 0;
					tree->SetBranchAddress(var_list[ivar], &tmp);
					tree->SetBranchAddress("Weight", &weight);
					tree->GetEntry(ientry);
					h_tmp->Fill(tmp, weight);
				}

				h_tmp->SetFillStyle(1001);
				h_tmp->SetFillColor(colorVector[isample]);
				h_tmp->SetLineColor(colorVector[isample]);

				if(sample_list[isample].Contains("Data"))
				{
					if(h_data == 0) {h_data = (TH1F*) h_tmp->Clone();}
					else {h_data->Add(h_tmp);}
				}
				else if(ichan==0)
				{
					v_MC_histo.push_back(h_tmp);
				}
				else //ichan != 0
				{
					v_MC_histo[isample]->Add(h_tmp);
				}

				//Create histograms for syst. +/-  -->  Compute uncertainty for each bin after
				//Same as with 'nominal' samples
				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					if(syst_list[isyst] == "") {continue;}

					h_tmp = 0;
					if(var_list[ivar] == "mTW") {h_tmp = new TH1F( "","", 60, 10, 130 );}
					else if(var_list[ivar] == "METpt") {h_tmp = new TH1F( "","", 60, -0., 120 );}
					else if(var_list[ivar] == "ZCandMass") {h_tmp = new TH1F( "","", 40, 70, 110 );}
					else {cout<<"Unknown variable"<<endl;}

					tree_name = "Control_" + sample_list[isample];
					//tree_name+= "_" + thechannellist[ichan];
					tree_name+= "_" + syst_list[isyst];
					cout<<"--- Processing "<<tree_name<<endl;
					tree = (TTree*) f->Get(tree_name.Data());
					//cout<<__LINE__<<endl;

					tree_nentries = tree->GetEntries();

					for(int ientry = 0; ientry<tree_nentries; ientry++)
					{
						float weight = 0;
						float tmp = 0;
						tree->SetBranchAddress(var_list[ivar], &tmp);
						tree->SetBranchAddress("Weight", &weight);
						tree->GetEntry(ientry);
						h_tmp->Fill(tmp, weight);
					}

					if(ichan==0)
					{
						if(syst_list[isyst] == "JERup") {v_MC_histo_JER_plus.push_back(h_tmp);}
						else if(syst_list[isyst] == "JERdn") {v_MC_histo_JER_minus.push_back(h_tmp);}
					}
					else
					{
						if(syst_list[isyst] == "JERup") {v_MC_histo_JER_plus[isample]->Add(h_tmp);}
						else if(syst_list[isyst] == "JERdn") {v_MC_histo_JER_minus[isample]->Add(h_tmp);}
					}
				}
			}
		}


		for(int i=0; i<v_MC_histo.size(); i++)
		{
			if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
			else {stack->Add(v_MC_histo[i]);}

			if(sample_list[i] != "ttZ" && sample_list[i] != "ttW") {qw->AddEntry(v_MC_histo[i], sample_list[i].Data() , "f");} //Need to keep ordered so that sample_list[i] <-> vector_MC[i]
			else if(sample_list[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV" , "f");}
		}

		if(!stack) {cout<<__LINE__<<" : stack is null"<<endl;}

		//---------------------------
		//DRAW HISTOS
		//---------------------------

		c1->cd();

		//SetMaximum
		if(h_data != 0 && stack != 0)
		{
			if(h_data->GetMaximum() > stack->GetMaximum() ) {stack->SetMaximum(h_data->GetMaximum()+0.3*h_data->GetMaximum());}
			else stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum());
		}

		if(stack != 0) {stack->Draw("HIST"); stack->GetXaxis()->SetLabelSize(0.0);}

		if(h_data != 0)
		{
		  h_data->SetMarkerStyle(20);
		  h_data->SetMarkerSize(1.2);
		  h_data->SetLineColor(1);
		  h_data->Draw("epsame");
		}
		else {cout<<__LINE__<<" : h_data is null"<<endl;}


		//--------------------------
		//MC SYSTEMATICS PLOT
		//--------------------------
		//Create a temporary TH1F* in order to put all the systematics into it via SetBinError, then create a TGraphError from this TH1F*
		//Also used to compute the Data/MC ratio
		TH1F* histo_syst_MC = 0;
		TH1F* histo_syst_MC_JER_plus = 0;  TH1F* histo_syst_MC_JER_minus = 0;
		//TH1F* histo_syst_MC_ElectronSF_plus = 0;  TH1F* histo_syst_MC_ElectronSF_minus = 0;

		for(unsigned int imc=0; imc < v_MC_histo.size(); imc++) //Clone or Add histograms
		{
			if(histo_syst_MC == 0) {histo_syst_MC = (TH1F*) v_MC_histo[imc]->Clone();}
			else {histo_syst_MC->Add(v_MC_histo[imc]);}
		}
		for(unsigned int imc=0; imc < v_MC_histo_JER_plus.size(); imc++) //Clone or Add histograms
		{
			if(histo_syst_MC_JER_plus == 0) {histo_syst_MC_JER_plus = (TH1F*) v_MC_histo_JER_plus[imc]->Clone();}
			else {histo_syst_MC_JER_plus->Add(v_MC_histo_JER_plus[imc]);}
			if(histo_syst_MC_JER_minus == 0) {histo_syst_MC_JER_minus = (TH1F*) v_MC_histo_JER_minus[imc]->Clone();}
			else {histo_syst_MC_JER_minus->Add(v_MC_histo_JER_minus[imc]);}
		}

		int nofbin = histo_syst_MC->GetNbinsX();

		//Add up here the different errors (quadratically), for each bin separately
		for(int ibin=1; ibin<nofbin+1; ibin++) //Start at bin 1
		{
			double err_up = 0;
			double err_low = 0;

			err_up += pow(fabs(histo_syst_MC_JER_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin)), 2);
			err_low += pow(fabs(histo_syst_MC->GetBinContent(ibin) - histo_syst_MC_JER_minus->GetBinContent(ibin)), 2);

			//err_up+= pow(fabs(histo_syst_MC_ElectronSF_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin)), 2);
			//err_low+= pow(fabs(histo_syst_MC->GetBinContent(ibin) - histo_syst_MC_ElectronSF_minus->GetBinContent(ibin)), 2);

			err_up+= pow(histo_syst_MC->GetBinContent(ibin)*0.02, 2); //Luminosity
			err_low+= pow(histo_syst_MC->GetBinContent(ibin)*0.02, 2);
			//double err_up = 1, err_low = 1;

			err_up = pow(err_up, 0.5);
			err_low = pow(err_low, 0.5);

			v_eyl.push_back(err_up);
			v_eyh.push_back(err_low);
			v_exl.push_back(histo_syst_MC->GetXaxis()->GetBinWidth(ibin) / 2);
			v_exh.push_back(histo_syst_MC->GetXaxis()->GetBinWidth(ibin) / 2);
			v_x.push_back( (histo_syst_MC->GetXaxis()->GetBinLowEdge(nofbin+1) - histo_syst_MC->GetXaxis()->GetBinLowEdge(1) ) * ((ibin - 0.5)/nofbin) + histo_syst_MC->GetXaxis()->GetBinLowEdge(1));
			v_y.push_back(histo_syst_MC->GetBinContent(ibin)); //see warning above about THStack and negative weights
			//cout<<"x = "<<v_x[ibin-1]<<endl;    cout<<", y = "<<v_y[ibin-1]<<endl;    cout<<", eyl = "<<v_eyl[ibin-1]<<endl;    cout<<", eyh = "<<v_eyh[ibin-1]<<endl; cout<<", exl = "<<v_exl[ibin-1]<<endl;    cout<<", exh = "<<v_exh[ibin-1]<<endl;
		}

		//Pointers to vectors : need to give the adress of first element (all other elements can be accessed iteratively)
		double* eyl = &v_eyl[0];
		double* eyh = &v_eyh[0];
		double* exl = &v_exl[0];
		double* exh = &v_exh[0];
		double* x = &v_x[0];
		double* y = &v_y[0];

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
		latex2->DrawLatex(0.87, 0.95, "2.26 fb^{-1} at #sqrt{s} = 13 TeV");

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

		if(h_data != 0 && v_MC_histo.size() != 0) //Need both data and MC
		{
			//TPad *canvas_2 = new TPad("canvas_2", "canvas_2",0,0.,1.0,0.34);
			TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
			canvas_2->SetTopMargin(0.7);
			canvas_2->SetFillColor(0);
			canvas_2->SetFillStyle(0);
			canvas_2->SetGridy(1);
			canvas_2->Draw();
			canvas_2->cd(0);

			TH1F * histo_ratio_data = (TH1F*) h_data->Clone();

			histo_ratio_data->Divide(histo_syst_MC);
			if(var_list[ivar] == "METpt")   histo_ratio_data->GetXaxis()->SetTitle(title_MET.Data());
			else if(var_list[ivar] == "mTW")   histo_ratio_data->GetXaxis()->SetTitle(title_mTW.Data());
			else if(var_list[ivar] == "ZCandMass")   histo_ratio_data->GetXaxis()->SetTitle(title_ZCandMass.Data());

			histo_ratio_data->SetMinimum(0.0);
			histo_ratio_data->SetMaximum(2.0);

			if(channel_list[0] == "uu") //Dileptonic analysis
			{
			  histo_ratio_data->SetMinimum(0.5);
			  histo_ratio_data->SetMaximum(1.5);
			}

			histo_ratio_data->GetXaxis()->SetTitleOffset(1.2);
			histo_ratio_data->GetXaxis()->SetLabelSize(0.04);
			histo_ratio_data->GetYaxis()->SetLabelSize(0.03);
			histo_ratio_data->GetYaxis()->SetNdivisions(6);
			histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
			histo_ratio_data->Draw("E1X0");

			//TGraphError for the Data/MC ratio plot
			TGraphAsymmErrors *thegraph_tmp = (TGraphAsymmErrors*) gr->Clone();

			double *theErrorX_h = thegraph_tmp->GetEXhigh();
			double *theErrorY_h = thegraph_tmp->GetEYhigh();
			double *theErrorX_l = thegraph_tmp->GetEXlow();
			double *theErrorY_l = thegraph_tmp->GetEYlow();
			double *theY        = thegraph_tmp->GetY() ;
			double *theX        = thegraph_tmp->GetX() ;

			for(int i=0; i<thegraph_tmp->GetN(); i++)
			{
			  theErrorY_l[i] = theErrorY_l[i]/theY[i];
			  theErrorY_h[i] = theErrorY_h[i]/theY[i];
			  theY[i]=1; //To center the filled area around "1"
			}

			TGraphAsymmErrors *thegraph_ratio = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
			thegraph_ratio->SetFillStyle(3005);
			thegraph_ratio->SetFillColor(1);

			thegraph_ratio->Draw("e2 same"); //Syst. error for Data/MC ; drawn on canvas2 (Data/MC ratio)
		}

		if(stack!= 0) {stack->GetYaxis()->SetTitleSize(0.04); stack->GetYaxis()->SetTitle("Events");}

		//-------------------
		//OUTPUT
		//-------------------

		TString outputname = "/plots/"+var_list[ivar]+"_"+channel+".png";
		if(channel == "" || allchannels) {outputname = "plots/"+var_list[ivar]+"_all.png";}

		//cout << __LINE__ << endl;

		if(c1!= 0) {c1->SaveAs(outputname.Data() );}

		//cout << __LINE__ << endl;
		delete c1;
	}
}
