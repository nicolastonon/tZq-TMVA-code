//---------------------------------------------------------------------------
// ########    ##     ##    ##     ##       ###           ######      #######     ########     ########
//    ##       ###   ###    ##     ##      ## ##         ##    ##    ##     ##    ##     ##    ##
//    ##       #### ####    ##     ##     ##   ##        ##          ##     ##    ##     ##    ##
//    ##       ## ### ##    ##     ##    ##     ##       ##          ##     ##    ##     ##    ######
//    ##       ##     ##     ##   ##     #########       ##          ##     ##    ##     ##    ##
//    ##       ##     ##      ## ##      ##     ##       ##    ##    ##     ##    ##     ##    ##
//    ##       ##     ##       ###       ##     ##        ######      #######     ########     ########
//---------------------------------------------------------------------------
//by Nicolas Tonon (IPHC)

#include "theMVAtool.h"

using namespace std;


//---------------------------------------------------------------------------
// ####    ##    ##    ####    ########
//  ##     ###   ##     ##        ##
//  ##     ####  ##     ##        ##
//  ##     ## ## ##     ##        ##
//  ##     ##  ####     ##        ##
//  ##     ##   ###     ##        ##
// ####    ##    ##    ####       ##
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Overloaded Constructor -- Initialize all the class members
 */
theMVAtool::theMVAtool(std::vector<TString > thevarlist, std::vector<TString > thesamplelist, std::vector<TString > thesystlist, std::vector<TString > thechanlist, vector<int> v_color, std::vector<TString > set_v_cut_name, std::vector<TString > set_v_cut_def, std::vector<bool > set_v_cut_IsUsedForBDT, vector<TString> v_add_vars, int nofbin_templates, bool in_isttZ, bool in_isWZ, TString extension_format, bool set_combine_naming_convention, TString set_dir_ntuples, TString set_t_name)
{
	cout<<endl<<endl;

	if(extension_format==".png" || extension_format==".pdf") {this->format = extension_format;}
	else {cout<<"ERROR : WRONG OUTPUT FORMAT ! EXIT !"<<endl; stop_program = true;}


	dbgMode = false;

	for(int i=0; i<thechanlist.size(); i++)
	{
		channel_list.push_back(thechanlist[i]);
	}

	for(int i=0; i<set_v_cut_name.size(); i++) //Region cuts vars (e.g. NJets)
	{
		v_cut_name.push_back(set_v_cut_name[i]);
		v_cut_def.push_back(set_v_cut_def[i]);
		v_cut_IsUsedForBDT.push_back(set_v_cut_IsUsedForBDT[i]);
		v_cut_float.push_back(-999);

		//NOTE : it is a problem if a variable is present in different list, because it will cause SetBranchAddress conflicts (only the last SetBranchAddress to a branch will work)
		//---> If a variable is present in 2 lists, erase it from other lists !
		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			if(thevarlist[ivar] == set_v_cut_name[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<thevarlist[ivar]<<" from vector thevarlist (possible conflict) !"<<endl;
				thevarlist.erase(thevarlist.begin() + ivar);
			}
		}
		for(int ivar=0; ivar<v_add_vars.size(); ivar++)
		{
			if(v_add_vars[ivar] == set_v_cut_name[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<v_add_vars[ivar]<<" from vector v_add_vars (possible conflict) !"<<endl;
				v_add_vars.erase(v_add_vars.begin() + ivar);
			}
		}
	}
	for(int i=0; i<thevarlist.size(); i++) //TMVA vars
	{
		var_list.push_back(thevarlist[i]);
		var_list_floats.push_back(-999);

		for(int ivar=0; ivar<v_add_vars.size(); ivar++)
		{
			if(v_add_vars[ivar] == thevarlist[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<v_add_vars[ivar]<<" from vector v_add_vars (possible conflict) !"<<endl;
				v_add_vars.erase(v_add_vars.begin() + ivar);
			}
		}
	}
	for(int i=0; i<v_add_vars.size(); i++) //Additional vars, only for CR plots
	{
		v_add_var_names.push_back(v_add_vars[i]);
		v_add_var_floats.push_back(-999);
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

	nbin = nofbin_templates;

	stop_program = false;

	//Make sure that the "==" sign is written properly, or rewrite it
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if( v_cut_def[ivar].Contains("=") && !v_cut_def[ivar].Contains("==") && !v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains(">") )
		{
			v_cut_def[ivar] = "==" + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));

			cout<<endl<<BOLD(FBLU("##################################"))<<endl;
			cout<<"--- Changed cut on "<<v_cut_name[ivar]<<" to: "<<v_cut_def[ivar]<<" ---"<<endl;
			cout<<BOLD(FBLU("##################################"))<<endl<<endl;
		}
	}

	//Store the "cut name" that will be written as a suffix in the name of each output file
	this->filename_suffix = "";
	TString tmp = "";
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if( (v_cut_name[ivar]=="METpt" || v_cut_name[ivar]=="mTW") && v_cut_def[ivar] == ">0") {continue;} //Useless cuts

		if(v_cut_def[ivar] != "")
		{
			if(v_cut_def[ivar].Contains("||") ) {continue;} //Don't add the 'or' conditions in filename
			else if(!v_cut_def[ivar].Contains("&&")) //Single condition
			{
				tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));
			}
			else //Double '&&' condition
			{
				TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
				tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
				//tmp+= "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
				tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
			}
			this->filename_suffix+= tmp;
		}
	}

	if(dbgMode) cout<<"suffix = "<<this->filename_suffix<<endl;

	isttZ = in_isttZ;
	isWZ = in_isWZ;

	if(isttZ && isWZ) {cout<<"ERROR : wrong region ! Exit"<<endl;  stop_program=true;}

	//Ntuples path
	dir_ntuples = set_dir_ntuples;
	t_name = set_t_name;


	combine_naming_convention = set_combine_naming_convention;

	luminosity_rescale = 1;

	cout<<endl<<endl;
}



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Compute the luminosity re-scaling factor (MC),  to be used thoughout the code
 * @param desired_luminosity [Value of the desired lumi in fb-1]
 */
void theMVAtool::Set_Luminosity(double desired_luminosity)
{
	// double current_luminosity = 12.9; //Summer 2016 lumi
	double current_luminosity = 35.68; //Moriond 2017 //CHANGED
	this->luminosity_rescale = desired_luminosity / current_luminosity;


	cout<<endl<<BOLD(FBLU("##################################"))<<endl;
	cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<current_luminosity<<" = "<<luminosity_rescale<<" ! ---"<<endl;
	cout<<BOLD(FBLU("##################################"))<<endl<<endl;
}











//---------------------------------------------------------------------------
// ########    ########        ###       ####    ##    ##    ####    ##    ##     ######
//    ##       ##     ##      ## ##       ##     ###   ##     ##     ###   ##    ##    ##
//    ##       ##     ##     ##   ##      ##     ####  ##     ##     ####  ##    ##
//    ##       ########     ##     ##     ##     ## ## ##     ##     ## ## ##    ##   ####
//    ##       ##   ##      #########     ##     ##  ####     ##     ##  ####    ##    ##
//    ##       ##    ##     ##     ##     ##     ##   ###     ##     ##   ###    ##    ##
//    ##       ##     ##    ##     ##    ####    ##    ##    ####    ##    ##     ######
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Train, test and evaluate the BDT with signal and bkg MC
 * @param channel
 * @param bdt_type           [BDT or BDT_ttZ (SR or CR)]
 * @param use_ttZaMCatNLO    [Use ttZmc@NLO sample for training (even if contains negative weights)]
 * @param write_ranking_info [Save variable ranking info in file or not (must set to false if optimizing BDT variable list)]
 */
void theMVAtool::Train_Test_Evaluate(TString channel, TString bdt_type, bool use_ttZaMCatNLO, bool write_ranking_info)
{
	if(isWZ) {cout<<BOLD(FRED("Error ! You are trying to train a BDT in the WZ Control Region ! Abort"))<<endl; return;}

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("---TRAINING ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	cout<<endl<<BOLD(FGRN("USE ONLY ttZ / WZ / ZZ / tZq FOR TRAINING!"))<<endl<<endl<<endl;

	if(!use_ttZaMCatNLO) cout<<BOLD(FGRN("USE TTZ MADGRAPH SAMPLE !"))<<endl<<endl;
	else cout<<BOLD(FGRN("USE TTZ aMC@NLO SAMPLE !"))<<endl<<endl;


	//---------------------------------------------------------------
    // This loads the TMVA libraries
    TMVA::Tools::Instance();

	//Allows to bypass a protection in TMVA::TransformationHandler, cf. description in source file:
	// if there are too many input variables, the creation of correlations plots blows up memory and basically kills the TMVA execution --> avoid above critical number (which can be user defined)
	(TMVA::gConfig().GetVariablePlotting()).fMaxNumOfAllowedVariablesForScatterPlots = 300;


	mkdir("outputs",0777);

	TString output_file_name = "outputs/" + bdt_type;
	if(channel != "") {output_file_name+= "_" + channel;}
	output_file_name+= this->filename_suffix;
	output_file_name+= ".root";
	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );

	// Create the factory object
	TMVA::Factory* factory = new TMVA::Factory(bdt_type.Data(), output_file, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification" );

	// Define the input variables that shall be used for the MVA training
	for(int i=0; i<var_list.size(); i++)
	{
		factory->AddVariable(var_list[i].Data(), 'F');
	}
	//Choose if the cut variables are used in BDT or not
	//NOTE : spectator vars are not used for training/evalution, but possible to check their correlations, etc.
	for(int i=0; i<v_cut_name.size(); i++)
	{
		if (dbgMode) cout<<"Is "<<v_cut_name[i]<<" used ? "<<(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("=="))<<endl;

		if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {factory->AddVariable(v_cut_name[i].Data(), 'F');}
		else {factory->AddSpectator(v_cut_name[i].Data(), v_cut_name[i].Data(), 'F');}
	}
	for(int i=0; i<v_add_var_names.size(); i++)
	{
		factory->AddSpectator(v_add_var_names[i].Data(), v_add_var_names[i].Data(), 'F');
	}

    TFile *f(0);
 	std::vector<TFile *> files_to_close;

	for(int isample=0; isample<sample_list.size(); isample++)
    {
		if(!sample_list[isample].Contains("WZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("ZZ") && !sample_list[isample].Contains("tZq") ) {continue;} //Use only ttZ, WZ, ZZ, tZq Ntuples for training BDTs

        // Read training and test data
        // --- Register the training and test trees
		TString inputfile;
		inputfile = dir_ntuples + "/FCNCNTuple_" + sample_list[isample] + ".root";
		if(!use_ttZaMCatNLO && sample_list[isample] == "ttZ") {inputfile = dir_ntuples + "/FCNCNTuple_ttZMad.root";}

	    TFile* file_input = 0;
		file_input = TFile::Open(inputfile.Data() );
		if(!file_input) {cout<<BOLD(FRED(<<inputfile.Data()<<" not found!"))<<endl; continue;}
		files_to_close.push_back(file_input);

		TTree* tree = 0;
		tree = (TTree*) file_input->Get(t_name.Data());
		if(tree==0)
		{
			cout<<BOLD(FRED("ERROR :"))<<" file "<<inputfile.Data()<<" --> *tree = 0 !"<<endl; continue;
		}


        // global event weights per tree (see below for setting event-wise weights)
        Double_t signalWeight     = 1.0;
        Double_t backgroundWeight = 1.0;

    //-- Choose between absolute/relative weights for training
		// if(sample_list[isample].Contains("tZq")) {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "fabs(Weight)" );}
        // else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "fabs(Weight)" );}

		// if(sample_list[isample].Contains("tZq")) {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "Weight" );}
        // else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "Weight" );}


		//Use relative weights in signal region & absolute weights in ttZ CR (needed to avoid overtraining !)
		if(isttZ && !isWZ)
		{
			if(sample_list[isample].Contains("tZq")) {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "fabs(Weight)" );}
			else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "fabs(Weight)" );}
			cout<<FYEL("-- Using *ABSOLUTE* weights (BDTttZ) --")<<endl;
		}
		else if(!isWZ && !isttZ)
		{
			if(sample_list[isample].Contains("tZq")) {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "Weight" );}
			else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "Weight" );}
			cout<<FYEL("-- Using *RELATIVE* weights (BDT tZq) --")<<endl;
		}
		else {cout<<BOLD(FRED("Error ! You are trying to train a BDT in the wrong region ! Abort"))<<endl; return;}
    }



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
	TString tmp = "";

	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if(v_cut_def[ivar] != "") //NB : no need to break conditions in 2 ?
		{
			if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) {tmp+= v_cut_name[ivar] + v_cut_def[ivar];} //If cut contains only 1 condition
			else if(v_cut_def[ivar].Contains("&&") && v_cut_def[ivar].Contains("||")) {cout<<BOLD(FRED("ERROR ! Wrong cut definition !"))<<endl;}
			else if(v_cut_def[ivar].Contains("&&") )//If '&&' in the cut, break it in 2
			{
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
				tmp+= " && ";
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
			}
			else if(v_cut_def[ivar].Contains("||") )//If '||' in the cut, break it in 2
			{
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
				tmp+= " || ";
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
			}
		}

		//Complicated way of concatenating the TStrings
		if((ivar+1) < v_cut_name.size() && v_cut_def[ivar+1] != "")
		{
			for(int i=0; i<ivar+1; i++)
			{
				if(v_cut_def[i] != "") {tmp += " && "; break;}
			}
		}

		//cout<<"tmp = "<<tmp<<endl;
	}

	if(dbgMode) cout<<"Total cut chain : "<<tmp<<endl;

	if(tmp != "") {mycuts+= tmp; mycutb+= tmp;}


//--------------------------------

    // Tell the factory how to use the training and testing events    //
    // If no numbers of events are given, half of the events in the tree are used for training, and the other half for testing:
	factory->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V");


	TString method_title = channel + this->filename_suffix; //So that the output weights are labelled differently for each channel

	//Boosted Decision Trees -- FIXME : choose right method !

	//--- Adaptive Boost (old method)
	// if(isttZ) factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=15:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );
	// else factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=20:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );

	// factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=20:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );


	//--- Gradient Boost, used by ttH --> USE THIS ONE (can still optimize ?)
	// factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:V:NTrees=200:nCuts=200:MaxDepth=2:BoostType=Grad:Shrinkage=0.10:IgnoreNegWeightsInTraining=True");




//--- TO TAKE NEGATIVE WEIGHTS INTO ACCOUNT (ttZ aMC@NLO) (From Mathias) - Need to use option IgnoreEventsWithNegWeightsInTraining ?
	//Adaboost
	// factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:BoostType=AdaBoost:AdaBoostBeta=0.4:PruneMethod=CostComplexity:PruneStrength=7:SeparationType=CrossEntropy:MaxDepth=3:nCuts=40:NodePurityLimit=0.5:NTrees=1000:MinNodeSize=1%:NegWeightTreatment=InverseBoostNegWeights" ); //Negweight treatment -> much longer


	// NOTE : to be used for now
	factory->BookMethod(TMVA::Types::kBDT,method_title.Data(),"!H:!V:NTrees=200:nCuts=200:MaxDepth=2:BoostType=Grad:Shrinkage=0.4:IgnoreNegWeightsInTraining=True");



	//FIXME
	// factory->BookMethod(TMVA::Types::kBDT,method_title.Data(),"!H:!V:NTrees=300:nCuts=300:MaxDepth=3:BoostType=Grad:Shrinkage=0.4:IgnoreNegWeightsInTraining=True");


//--------------------------------------
	output_file->cd();


	mkdir("outputs/Rankings", 0777); //Dir. containing variable ranking infos
	if(!isWZ && !isttZ) mkdir("outputs/Rankings/BDT", 0777);
	else if(!isWZ && isttZ) mkdir("outputs/Rankings/BDTttZ", 0777);

	TString ranking_file_path;
	if(!isWZ && !isttZ) ranking_file_path = "outputs/Rankings/BDT/RANKING_" + bdt_type + "_" + channel + ".txt";
	else if(!isWZ && isttZ) ranking_file_path = "outputs/Rankings/BDTttZ/RANKING_" + bdt_type + "_" + channel + ".txt";
	if(write_ranking_info) cout<<endl<<endl<<endl<<FBLU("NB : Temporarily redirecting standard output to file '"<<ranking_file_path<<"' in order to save Ranking Info !!")<<endl<<endl<<endl;

	std::ofstream out("ranking_info_tmp.txt"); //Temporary name
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	if(write_ranking_info) std::cout.rdbuf(out.rdbuf()); //redirect std::cout to text file --> Ranking info will be saved !

    // Train MVAs using the set of training events
    factory->TrainAllMethods();

	if(write_ranking_info) std::cout.rdbuf(coutbuf); //reset to standard output again

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

    delete output_file;
    delete factory;

	for(unsigned int i=0; i<files_to_close.size(); i++) {files_to_close[i]->Close(); delete files_to_close[i];}


	if(write_ranking_info)
	{
		MoveFile("./ranking_info_tmp.txt", ranking_file_path);
		Extract_Ranking_Info(ranking_file_path, bdt_type, channel); //Extract only ranking info from TMVA output
	}
	else {system("rm ./ranking_info_tmp.txt");} //Else remove the temporary ranking file

	return;
}










//---------------------------------------------------------------------------
// ########     ########                ######      ######        ###       ##          ####    ##    ##     ######
// ##     ##    ##                     ##    ##    ##    ##      ## ##      ##           ##     ###   ##    ##    ##
// ##     ##    ##                     ##          ##           ##   ##     ##           ##     ####  ##    ##
// ########     ######      #######     ######     ##          ##     ##    ##           ##     ## ## ##    ##   ####
// ##   ##      ##                           ##    ##          #########    ##           ##     ##  ####    ##    ##
// ##    ##     ##                     ##    ##    ##    ##    ##     ##    ##           ##     ##   ###    ##    ##
// ##     ##    ########                ######      ######     ##     ##    ########    ####    ##    ##     ######
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * [theMVAtool::Compute_Fake_SF description]
 * @param  f               [File containing mTW templates]
 * @param  fakeLep_flavour [Fake lepton flavour for which we compute the SF	]
 */
double theMVAtool::Compute_Fake_SF(TFile* f, TString fakeLep_flavour)
{
  if(fakeLep_flavour != "u" && fakeLep_flavour != "e") {cout<<__LINE__<<" : "<<"Wrong name for flavour of fake lepton ('e' or 'u') !"; return 0;}

  vector<TString> channels_same_flavour;
  if(fakeLep_flavour == "u")
  {
    channels_same_flavour.push_back("uuu");
    channels_same_flavour.push_back("eeu");
  }
  else if(fakeLep_flavour == "e")
  {
    channels_same_flavour.push_back("eee");
    channels_same_flavour.push_back("uue");
  }


  std::vector<TString> listSum;
  for(int isample=0; isample<sample_list.size(); isample++)
  {
    if(sample_list[isample] == "Data" || sample_list[isample].Contains("Fake") ) {continue;} //Treated separately
    TString name_tmp = "mTW_uuu__"+sample_list[isample];
    if(!f ->GetListOfKeys()->Contains( name_tmp.Data() ) ) {cout<<name_tmp.Data()<<" not found !"<<endl;  continue;}

    listSum.push_back(sample_list[isample]);
  }
  cout<<endl<<endl;

  vector<TString> channelfake;
  if(fakeLep_flavour == "u")
  {
    channelfake.push_back("FakeMuMuMu");
    channelfake.push_back("FakeElElMu");
  }
  else if(fakeLep_flavour == "e")
  {
    channelfake.push_back("FakeElElEl");
    channelfake.push_back("FakeMuMuEl");
  }

  f->cd();
  TH1F * hdata, *hsum, * hfake, *h_tmp;

  //Treat Data / Fakes / Other samples separately
  TString myHist;
  for(int ichan=0; ichan<channels_same_flavour.size(); ichan++)
  {
    //DATA
    if(combine_naming_convention) myHist = "mTW_" + channels_same_flavour[ichan] + "__data_obs";
    else myHist = "mTW_" + channels_same_flavour[ichan] + "__DATA";

    if ( !f->GetListOfKeys()->Contains( myHist.Data() ) ) {cout<<myHist.Data()<<" not found !"<<endl; return 0;}
    h_tmp = (TH1F*)f->Get( myHist);
    if(ichan==0) {hdata = (TH1F*) h_tmp->Clone();}
    else {hdata->Add(h_tmp);}

    //FAKES
    myHist = "mTW_" + channels_same_flavour[ichan] + "__" + channelfake[ichan];
    if ( !f->GetListOfKeys()->Contains( myHist.Data() ) )  {cout<<myHist.Data()<<" not found !"<<endl; return 0;}
    h_tmp = (TH1F*)f->Get( myHist);
    if(ichan==0) {hfake = (TH1F*) h_tmp->Clone();}
    else {hfake->Add(h_tmp);}

    //MC
    for (int i = 0; i<listSum.size(); i++)
    {
      myHist = "mTW_" + channels_same_flavour[ichan] + "__" + listSum[i];
      if ( !f->GetListOfKeys()->Contains( myHist.Data() ) ) {cout<<myHist.Data()<<" not found !"<<endl; return 0;}
      h_tmp = (TH1F*)f->Get( myHist);
      if(ichan==0 && i==0) {hsum = (TH1F*) h_tmp->Clone();}
      else {hsum->Add(h_tmp);}
    }
  }

  // hfake->Print(); hsum->Print(); hdata->Print();

  TObjArray *mc = new TObjArray(2); //Create array of MC samples -- differentiate fakes from rest
  mc->Add(hfake); //Param 0
  mc->Add(hsum); //Param 1

  TFractionFitter* fit = new TFractionFitter(hdata, mc, "Q"); //'Q' for quiet

  //Constrain backgrounds which are not fake (NB : because we're only interested in fitting the fakes to the data here!)
  double fracmc = hsum->Integral()/hdata->Integral() ;
  fit->Constrain(0, 0.001, pow(10,6) ); //Constrain param 0 (fakes integral) between 0 & 1
  if(fracmc>0) fit->Constrain(1, fracmc*0.99, fracmc*1.01); //Constrain param 1 (other MC samples integrals)


  TFitResultPtr r = fit->Fit(); //Create smart ptr to TFitResult --> can access fit properties

  TCanvas* c1 = new TCanvas("c1");

  TH1F* result = (TH1F*) fit->GetPlot();
  hdata->Draw("Ep");
  result->Draw("same");

  mkdir("plots",0777); //Create directory if inexistant

  c1->SaveAs(("plots/ScaleFakes_"+fakeLep_flavour+".png").Data()); //Save fit plot

  double fakes_postfit = r->Parameter(0); //Parameter 0 <--> Fitted Fraction of mc[0] == Fakes

  // cout<<endl<<"Fake lepton flavour "<<fakeLep_flavour<<" : Fraction of Fakes fitted from data = "<<fakes_postfit*100<<" %"<<endl;

  delete mc; delete fit; delete c1;

  return (fakes_postfit/hfake->Integral() ) * hdata->Integral();
}




/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


/**
 * Rescale Fake histograms with SFs computed with TFractionFitter
 * @param file_to_rescale_name [Name of file containing fake histograms to be rescaled]
 */
void theMVAtool::Rescale_Fake_Histograms(TString file_to_rescale_name)
{
	TFile * file_mTW_templates_unscaled = 0;

	TString file_mTW_templates_unscaled_PATH;
	Long_t *id,*size,*flags,*modtime;
	file_mTW_templates_unscaled_PATH = "outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root"; //mTW unrescaled Template file  => Used to compute the Fakes SFs

	file_mTW_templates_unscaled = TFile::Open(file_mTW_templates_unscaled_PATH, "READ"); //File containing the templates, from which can compute fake ratio
	if(!file_mTW_templates_unscaled) {cout<<FRED(<<file_mTW_templates_unscaled_PATH.Data()<<" not found! Can't compute Fake Ratio -- Abort")<<endl; return;}

	double SF_FakeMu = Compute_Fake_SF(file_mTW_templates_unscaled, 'u');
	double SF_FakeEl = Compute_Fake_SF(file_mTW_templates_unscaled, 'e');

	cout<<endl<<BOLD(FYEL("--- Re-scaling the Fake histograms ---"))<<endl;
	cout<<FYEL("Muon SF = "<<SF_FakeMu<<"")<<endl;
	cout<<FYEL("Electron SF = "<<SF_FakeEl<<"")<<endl;

	delete file_mTW_templates_unscaled;

	vector<TString> total_var_list;
	//Templates
	total_var_list.push_back("mTW");
	total_var_list.push_back("BDT");
	total_var_list.push_back("BDTttZ");

	//Other vars
	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		if(var_list[ivar] == "mTW") {continue;} //already added
		total_var_list.push_back(var_list[ivar]);
	}
	for(int ivar=0; ivar<v_add_var_names.size(); ivar++)
	{
		if(v_add_var_names[ivar] == "mTW") {continue;} //already added
		total_var_list.push_back(v_add_var_names[ivar]);
	}
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if(v_cut_name[ivar] == "mTW") {continue;} //already added
		total_var_list.push_back(v_cut_name[ivar]);
	}

	TFile * file_to_rescale = 0;

	//Special case : for mTW template file, we need an 'unscaled' version so that we can compute the Fakes SFs from there
	//--> If file to be rescaled contains the original mTW templates, do the rescaling in a new copy of the file
	if(file_to_rescale_name.Contains("_unScaled.root") )
	{
		TString copy_command = "cp " + file_to_rescale_name;

		int index = 0;
		index = file_to_rescale_name.Index("_unScaled.root"); //Find index of substring
		file_to_rescale_name.Remove(index); //Remove substring
		file_to_rescale_name+= ".root"; //Add desired suffix

		copy_command+= " " + file_to_rescale_name;
		system(copy_command.Data()); //Copy the original (unrescaled) template file first
	}


	file_to_rescale = TFile::Open(file_to_rescale_name.Data(), "UPDATE"); //NOTE : update mode --> overwrite fake histograms
	if(!file_to_rescale) {cout<<FRED(<<file_to_rescale_name.Data()<<" not found! -- Abort")<<endl; return;}


	//Define list of systematics here, so the function always look for the corresponding histos
	vector<TString> syst_fakes;
	syst_fakes.push_back(""); //Nominal
	if(combine_naming_convention) //Combine
	{
		syst_fakes.push_back("FakesUp");
		syst_fakes.push_back("FakesDown");
	}
	else  //Theta
	{
		syst_fakes.push_back("Fakes__plus");
		syst_fakes.push_back("Fakes__minus");
	}

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		TString Fakename;
		if(channel_list[ichan] == "uuu") Fakename = "__FakeMuMuMu";
		else if(channel_list[ichan] == "uue") Fakename = "__FakeMuMuEl";
		else if(channel_list[ichan] == "eeu") Fakename = "__FakeElElMu";
		else if(channel_list[ichan] == "eee") Fakename = "__FakeElElEl";

		for(int isyst=0; isyst<syst_fakes.size(); isyst++)
		{
			TString systname = "";
			if(syst_fakes[isyst] != "") systname = "__" + syst_fakes[isyst];

			for(int ivar=0; ivar<total_var_list.size(); ivar++)
			{
				TString histo_name = total_var_list[ivar] + "_" + channel_list[ichan] + Fakename + systname;
				if(!file_to_rescale->GetListOfKeys()->Contains(histo_name.Data()) ) {continue;}

				TH1F* h_tmp = (TH1F*) file_to_rescale->Get(histo_name);

				if(h_tmp->Integral() < 50)
				{
					cout<<BOLD(FMAG("Warning : "<<histo_name<<"->Integral() = "<<h_tmp->Integral()<<" < 50 ! Are you sure the fakes were not re-scaled already ?"))<<endl<<endl;
				}

				if(channel_list[ichan] == "uuu" || channel_list[ichan] == "eeu") h_tmp->Scale(SF_FakeMu);
				else if(channel_list[ichan] == "eee" || channel_list[ichan] == "uue") h_tmp->Scale(SF_FakeEl);

				file_to_rescale->cd();
				h_tmp->Write(histo_name, TObject::kOverwrite);

				delete h_tmp;
			}
		}
	}

	delete file_to_rescale;

	return;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////











//---------------------------------------------------------------------------
//  ######  ########  ########    ###    ######## ########       ######## ######## ##     ## ########  ##          ###    ######## ########  ######
// ##    ## ##     ## ##         ## ##      ##    ##                ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
// ##       ##     ## ##        ##   ##     ##    ##                ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
// ##       ########  ######   ##     ##    ##    ######            ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
// ##       ##   ##   ##       #########    ##    ##                ##    ##       ##     ## ##        ##       #########    ##    ##             ##
// ##    ## ##    ##  ##       ##     ##    ##    ##                ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//  ######  ##     ## ######## ##     ##    ##    ########          ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Reader function. Uses output from training (weights, ...) and read samples to create distributions of the BDT discriminant *OR* mTW
 * @param  template_name         BDT, BDTttZ or mTW
 * @param  fakes_from_data       If true, use fakes from data sample
 * @param  real_data             If true, use real data ; else, looks for pseudodata
 * @param  fakes_summed_channels Sum uuu+eeu & eee+uue for fakes
 * @param  cut_on_BDT            Cut value on BDT (-> cran create templates in CR)
 */
int theMVAtool::Read(TString template_name, bool fakes_from_data, bool real_data, bool fakes_summed_channels, bool cut_on_BDT /*=false*/, double BDT_cut_value /*=999*/)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;}
	else {cout<<BOLD(FRED("--- ERROR : invalid template_name value ! Exit !"))<<endl; cout<<"Valid names are : BDT/BDTttZ/mTW !"<<endl; return 0;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
	else {cout<<FYEL("--- Using fakes from data ---")<<endl;}
	if(!real_data) {cout<<FYEL("--- Not using real data ---")<<endl<<endl;}
	else {cout<<FYEL("--- Using REAL data ---")<<endl<<endl;}
	if(fakes_summed_channels) {cout<<FYEL("--- Fakes : summing channels 2 by 2 (artificial stat. increase) ---")<<endl<<endl;}
	if(cut_on_BDT) {cout<<FYEL("--- Creating templates WITH cut on BDT value // Cut value = "<<BDT_cut_value<<" ----")<<endl; if(BDT_cut_value==-99) {return 1;} }

	TString output_file_name = "outputs/Reader_" + template_name + this->filename_suffix;
	if(cut_on_BDT) output_file_name+= "_CutBDT";
	//Cf. Fakes rescaling function : need to have an 'unscaled' version of the mTW template to compute the Fakes SFs
	if(template_name == "mTW") output_file_name+= "_unScaled.root"; //Add suffix to distinguish it : this file will be used to compute SFs
	else output_file_name+= ".root";

	TFile* file_output = TFile::Open( output_file_name, "RECREATE" );

	TH1::SetDefaultSumw2();

	mkdir("outputs",0777);

	delete reader; //Free any previous allocated memory, and re-initialize reader
	reader = new TMVA::Reader( "!Color:!Silent" ); //CHANGED

	// Name & adress of local variables which carry the updated input values during the event loop
	// - the variable names MUST corresponds in name and type to those given in the weight file(s) used
	for(int i=0; i<var_list.size(); i++)
	{
		reader->AddVariable(var_list[i].Data(), &var_list_floats[i]); //cout<<"Added variable "<<var_list[i]<<endl;
	}
	for(int i=0; i<v_cut_name.size(); i++)
	{
		if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);}
		else {reader->AddSpectator(v_cut_name[i].Data(), &v_cut_float[i]);}
	}
	for(int i=0; i<v_add_var_names.size(); i++)
	{
		reader->AddSpectator(v_add_var_names[i].Data(), &v_add_var_floats[i]);
	}

	// --- Book the MVA methods
	TString dir    = "weights/";
	if(template_name == "BDT" || template_name == "BDTttZ" || cut_on_BDT) //Need to book method if want BDT template or if cut on BDT
	{
		TString MVA_method_name = "";
		TString weightfile = "";

		for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
		{
			TString template_name_MVA = template_name; //BDT or BDTttZ

			if(cut_on_BDT && !template_name.Contains("BDT")) {template_name_MVA = "BDT";} //Even if template is mTW, need to book BDT method if want to apply cut on BDT!

			MVA_method_name = template_name_MVA + "_" + channel_list[ichan] + this->filename_suffix + TString(" method");
			weightfile = dir + template_name_MVA + "_" + channel_list[ichan] + this->filename_suffix + TString(".weights.xml");

			reader->BookMVA(MVA_method_name, weightfile);
		}
	}

	TFile* file_input;
	TTree* tree(0);
	TTree* tree_passTrig = 0;

	TH1F *hist_uuu = 0, *hist_uue = 0, *hist_eeu = 0, *hist_eee = 0;
	TH1F *h_sum_fake = 0;


	// --- Systematics loop
	for(int isyst=0; isyst<syst_list.size(); isyst++)
	{
		cout<<endl<<endl<<FGRN("Systematic "<<syst_list[isyst]<<" ("<<isyst+1<<"/"<<syst_list.size()<<") :")<<endl;

		//Loop on samples, syst., events ---> Fill histogram/channel --> Write()
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			if(!fakes_from_data && sample_list[isample] == "Fakes" ) {continue;} //Fakes from MC only
			else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

			if(!real_data && sample_list[isample] == "Data") {continue;} //Don't use the real data

			if( (sample_list[isample]=="Data" && syst_list[isyst]!="")
				|| (sample_list[isample].Contains("Fakes") && (syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") ) ) ) {continue;} //Only 'Fakes' syst. taken into account for Fakes

			if( syst_list[isyst].Contains("Fakes") && !sample_list[isample].Contains("Fakes"))   {continue;} //"Fakes" syst only in fakes samples

			if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}

			if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only


			TString inputfile = dir_ntuples + "/FCNCNTuple_" + sample_list[isample] + ".root";

			//For these systematics, info stored in separate ntuples ! (events are not the same)
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQup.root";}
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__minus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQdw.root";}
			else if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "Hadron__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqhwpp.root";} //For minus, take nominal //FIXME


			file_input = 0;
			file_input = TFile::Open( inputfile.Data() );
			if(!file_input) {cout<<inputfile.Data()<<" not found!"<<endl; continue;}
			if(dbgMode) std::cout << "--- Select "<<sample_list[isample]<<" sample" << __LINE__ <<std::endl;

			if(fakes_from_data || (!fakes_from_data && isample <= (sample_list.size() - 3)) ) //MC Fakes : Don't reinitialize histos -> sum !
			{

				// Book output histograms
				if (template_name == "BDT" || template_name == "BDTttZ") //create histogram for each channel (-1 = bkg, +1 = signal)
				{
					hist_uuu     = new TH1F( (template_name+"_uuu").Data(),           (template_name+"_uuu").Data(),           nbin, -1, 1 );
					hist_uue     = new TH1F( (template_name+"_uue").Data(),           (template_name+"_uue").Data(),           nbin, -1, 1 );
					hist_eeu     = new TH1F( (template_name+"_eeu").Data(),           (template_name+"_eeu").Data(),           nbin, -1, 1 );
					hist_eee     = new TH1F( (template_name+"_eee").Data(),           (template_name+"_eee").Data(),           nbin, -1, 1 );
				}
				else if (template_name == "mTW")
				{
					hist_uuu     = new TH1F( "mTW_uuu",           "mTW_uuu",           nbin, 0., 250 );
					hist_uue     = new TH1F( "mTW_uue",           "mTW_uue",           nbin, 0., 250 );
					hist_eeu     = new TH1F( "mTW_eeu",           "mTW_eeu",           nbin, 0., 250 );
					hist_eee     = new TH1F( "mTW_eee",           "mTW_eee",           nbin, 0., 250 );
				}
			}

			tree = 0;
			tree_passTrig = 0;

			//For JES & JER systematics, need a different tree (modify the variables distributions' shapes)
			if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") )
			{
				tree = (TTree*) file_input->Get(syst_list[isyst].Data());

				tree_passTrig = (TTree*) file_input->Get(t_name.Data()); //passTrig info is in Default Tree only
			}
			else {tree = (TTree*) file_input->Get(t_name.Data());}

//--- Prepare the event tree -- Set Branch Addresses
//WARNING : the last SetBranchAddress overrides the previous ones !! Be careful not to associate branches twice !

			for(int i=0; i<v_add_var_names.size(); i++)
			{
				if(v_add_var_names[i] == "passTrig" || v_add_var_names[i] == "RunNr") {continue;} //special vars
				tree->SetBranchAddress(v_add_var_names[i].Data(), &v_add_var_floats[i]);
			}
			for(int i=0; i<var_list.size(); i++)
			{
				if(var_list[i] == "passTrig" || var_list[i] == "RunNr") {continue;}
				tree->SetBranchAddress(var_list[i].Data(), &var_list_floats[i]);
			}
			for(int i=0; i<v_cut_name.size(); i++)
			{
				if(v_cut_name[i] == "passTrig" || v_cut_name[i] == "RunNr") {continue;}
				tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);
			}

			float mTW = -666; tree->SetBranchAddress("mTW", &mTW);
			float i_channel = 9; tree->SetBranchAddress("Channel", &i_channel);

			float passTrig = -999;
			float RunNr = -999;
			if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes"))
			{
				tree_passTrig->SetBranchAddress("passTrig", &passTrig);
				tree_passTrig->SetBranchAddress("RunNr", &RunNr);
			}
			else
			{
				tree->SetBranchAddress("passTrig", &passTrig);
				tree->SetBranchAddress("RunNr", &RunNr);
			}

			float weight;
			//For all other systematics, only the events weights change
			if(syst_list[isyst] == "" || syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") || syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron"))	{tree->SetBranchAddress("Weight", &weight);}
			else {tree->SetBranchAddress(syst_list[isyst].Data(), &weight);}

			cout<<endl<< "--- "<<sample_list[isample]<<" : Processing: " << tree->GetEntries() << " events" << std::endl;


//------------------------------------------------------------
// --- START EVENT LOOP ---

			// int n_entries = 100;
			int n_entries = tree->GetEntries();

			for(int ievt=0; ievt<n_entries; ievt++)
			{
				if(ievt%10000==0) cout<<ievt<<" / "<<n_entries<<endl;

				if(dbgMode) {cout<<endl<<"--- Syst "<<syst_list[isyst]<<" / Sample : "<<sample_list[isample]<<endl;}

				weight = 0; i_channel = 9; mTW=-666;
				passTrig = -999; RunNr = -999;

				tree->GetEntry(ievt);
				if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes")) {tree_passTrig->GetEntry(ievt);}

        		bool isChannelToKeep = false;

				// NOTE -- Can simulate signal strength here
				// if(sample_list[isample].Contains("tZq")) {weight*=3;}

		        for(unsigned int ichan=0; ichan < channel_list.size(); ichan++)
				{
					//cout << "channel_list[ichan] " << channel_list[ichan]<< endl;
					//cout << "i_channel " << i_channel << endl;
		          	if(i_channel == 0 && channel_list[ichan] == "uuu")  isChannelToKeep = true;
		          	if(i_channel == 1 && channel_list[ichan] == "uue")  isChannelToKeep = true;
		          	if(i_channel == 2 && channel_list[ichan] == "eeu")  isChannelToKeep = true;
		          	if(i_channel == 3 && channel_list[ichan] == "eee")  isChannelToKeep = true;
				}

				if(!isChannelToKeep) continue;

//------------------------------------------------------------
//------------------------------------------------------------
//---- APPLY CUTS HERE  ----
				float cut_tmp = 0; bool pass_all_cuts = true;


				for(int ivar=0; ivar<v_cut_name.size(); ivar++)
				{
					// NOTE : DO NOT APPLY Additional Lepton ISO CUTS ON FAKES
					if ( sample_list[isample].Contains("Fakes") &&  (v_cut_name[ivar] == "AdditionalMuonIso" || v_cut_name[ivar] == "AdditionalEleIso" ) ) {continue;}

					if(v_cut_def[ivar] == "") {continue;}

					//CHANGED : can't set Branch address of same branch to 2 different variables (only last one will be filled)
					//Since I define myself a 'mTW' variable, it is a problem if one of the cuts is on mTW (because the associated float won't be filled)
					//---> If v_cut_name[i] == "mTW", need to make sure that we use the variable which is filled !
					if(v_cut_name[ivar] == "mTW") {v_cut_float[ivar] = mTW;}
					//Idem for channel value
					if(v_cut_name[ivar] == "Channel") {v_cut_float[ivar] = i_channel;}
					if(v_cut_name[ivar] == "passTrig") {v_cut_float[ivar] = passTrig;}
					if(v_cut_name[ivar] == "RunNr") {v_cut_float[ivar] = RunNr;}

					// cout<<v_cut_name[ivar]<<" "<<v_cut_float[ivar]<<endl;

					if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) //If cut contains only 1 condition
					{
						cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);
						if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break;}
					}
					else if(v_cut_def[ivar].Contains("&&") && v_cut_def[ivar].Contains("||")) {cout<<BOLD(FRED("ERROR ! Wrong cut definition !"))<<endl;}
					else if(v_cut_def[ivar].Contains("&&") )//If '&&' in the cut def, break it in 2
					{
						TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first;
						TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
						//CUT 1
						cut_tmp = Find_Number_In_TString(cut1);
						if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
						else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
						else if(cut1.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
						else if(cut1.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
						else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
						//CUT 2
						cut_tmp = Find_Number_In_TString(cut2);
						if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
						else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
						else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
						else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
						else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
					}
					else if(v_cut_def[ivar].Contains("||") )//If '||' in the cut def, break it in 2
					{
						TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first;
						TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;

						bool pass_cut1 = true; bool pass_cut2 = true; //Need to pass at least 1 cut

						//CUT 1
						cut_tmp = Find_Number_In_TString(cut1);
						if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_cut1 = false;}
						else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_cut1 = false;}
						else if(cut1.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_cut1 = false;}
						else if(cut1.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_cut1 = false;}
						else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_cut1 = false;}
						//CUT 2
						cut_tmp = Find_Number_In_TString(cut2);
						if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_cut2 = false;}
						else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_cut2 = false;}
						else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_cut2 = false;}
						else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_cut2 = false;}
						else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_cut2 = false;}

						if(!pass_cut1 && !pass_cut2) {pass_all_cuts = false; break;}
					}
				}


				if(!pass_all_cuts) {continue;}

				TString MVA_method_name = template_name + "_" + channel_list[i_channel] + this->filename_suffix + " method";
				if(cut_on_BDT && !template_name.Contains("BDT")) {MVA_method_name = "BDT_" + channel_list[i_channel] + this->filename_suffix + " method";}

				if(cut_on_BDT && reader->EvaluateMVA(MVA_method_name.Data() ) > BDT_cut_value) {continue;} //Cut on BDT value


//------------------------------------------------------------
//------------------------------------------------------------
//--- CREATE OUTPUT HISTOGRAMS (TEMPLATES) ----

				bool sum_fakes = false;
				if(fakes_summed_channels && (sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW")))
				{
					sum_fakes = true;
				}


				// --- Return the MVA outputs and fill into histograms

				//Same boundaries & nbins for all 4 histos
				double xmax_h = hist_uuu->GetXaxis()->GetXmax();
				int lastbin_h = hist_uuu->GetNbinsX();
				double mva_value = -999;

				//CHANGED -- put overflow in last bin (error treatment ok ?)
				if (template_name == "BDT" || template_name == "BDTttZ")
				{
					//For fakes, can choose to sum channels 2 by 2 to increase statistics
					if(sum_fakes)
					{
						if(i_channel == 0 || i_channel == 2) //uuu & eeu
						{
							mva_value = reader->EvaluateMVA( template_name+"_uuu"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_uuu->Fill(mva_value, weight);}
							else {hist_uuu->SetBinContent(lastbin_h, (hist_uuu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)

							mva_value = reader->EvaluateMVA( template_name+"_eeu"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_eeu->Fill(mva_value, weight);}
							else {hist_eeu->SetBinContent(lastbin_h, (hist_eeu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 1 || i_channel == 3) //eee & uue
						{
							mva_value = reader->EvaluateMVA( template_name+"_eee"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_eee->Fill(mva_value, weight);}
							else {hist_eee->SetBinContent(lastbin_h, (hist_eee->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)

							mva_value = reader->EvaluateMVA( template_name+"_uue"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_uue->Fill(mva_value, weight);}
							else {hist_uue->SetBinContent(lastbin_h, (hist_uue->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
					}
					//If sample is not fake or fakes_summed_channels = false
					else
					{
						if(i_channel == 0)
						{
							mva_value = reader->EvaluateMVA( template_name+"_uuu"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_uuu->Fill(mva_value, weight);}
							else {hist_uuu->SetBinContent(lastbin_h, (hist_uuu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 1)
						{
							mva_value = reader->EvaluateMVA( template_name+"_uue"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_uue->Fill(mva_value, weight);}
							else {hist_uue->SetBinContent(lastbin_h, (hist_uue->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 2)
						{
							mva_value = reader->EvaluateMVA( template_name+"_eeu"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_eeu->Fill(mva_value, weight);}
							else {hist_eeu->SetBinContent(lastbin_h, (hist_eeu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 3)
						{
							mva_value = reader->EvaluateMVA( template_name+"_eee"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_eee->Fill(mva_value, weight);}
							else {hist_eee->SetBinContent(lastbin_h, (hist_eee->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}

						else  {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
					}
				}
				// --- Return the MVA outputs and fill into histograms
				else if (template_name == "mTW")
				{
					//For fakes, can choose to sum channels 2 by 2 to increase statistics
					if(sum_fakes)
					{
						if(i_channel == 0 || i_channel == 2)
						{
							if(mTW < xmax_h ) {hist_uuu->Fill(mTW, weight); hist_eeu->Fill(mTW, weight);}
							else {hist_uuu->SetBinContent(lastbin_h, (hist_uuu->GetBinContent(lastbin_h) + weight) ); hist_eeu->SetBinContent(lastbin_h, (hist_eeu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 1 || i_channel == 3)
						{
							if(mTW < xmax_h ) {hist_eee->Fill(mTW, weight); hist_uue->Fill(mTW, weight);}
							else {hist_eee->SetBinContent(lastbin_h, (hist_eee->GetBinContent(lastbin_h) + weight) ); hist_uue->SetBinContent(lastbin_h, (hist_uue->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
					}
					//If sample is not fake or fakes_summed_channels = false
					else
					{
						if(i_channel == 0)
						{
							if(mTW < xmax_h ) {hist_uuu->Fill(mTW, weight);}
							else {hist_uuu->SetBinContent(lastbin_h, (hist_uuu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 1)
						{
							if(mTW < xmax_h ) {hist_uue->Fill(mTW, weight);}
							else {hist_uue->SetBinContent(lastbin_h, (hist_uue->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 2)
						{
							if(mTW < xmax_h ) {hist_eeu->Fill(mTW, weight);}
							else {hist_eeu->SetBinContent(lastbin_h, (hist_eeu->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}
						else if(i_channel == 3)
						{
							if(mTW < xmax_h ) {hist_eee->Fill(mTW, weight);}
							else {hist_eee->SetBinContent(lastbin_h, (hist_eee->GetBinContent(lastbin_h) + weight) );} //Put overflow in last bin (no info lost)
						}

						else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
					}
				}

			} //end entries loop


//---------

			//Re-scale to desired luminosity, unless it's data
			if(sample_list[isample] != "Data")
			{
				hist_uuu->Scale(luminosity_rescale); hist_uue->Scale(luminosity_rescale); hist_eeu->Scale(luminosity_rescale); hist_eee->Scale(luminosity_rescale);
			}

			// --- Write histograms
			file_output->cd();

			//NB : theta name convention = <observable>__<process>[__<uncertainty>__(plus,minus)]
			TString output_histo_name = "";
			TString syst_name = "";
			if(combine_naming_convention && syst_list[isyst] != "") {syst_name = "__" + Combine_Naming_Convention(syst_list[isyst]);} //Combine naming convention
			else if(syst_list[isyst] != "") {syst_name = "__" + Theta_Naming_Convention(syst_list[isyst]);} //Theta naming convention

			TString sample_name = sample_list[isample];
			// if(real_data && sample_list[isample] == "Data") {sample_name = "DATA";} //THETA CONVENTION
			if(real_data && sample_list[isample] == "Data")
			{
				if(combine_naming_convention) sample_name = "data_obs";
				else sample_name = "DATA";
			}

			if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
			{
				continue;
			}
			else if( (!fakes_from_data && isample == (sample_list.size() - 1)) || (fakes_from_data && sample_list[isample] == "Fakes") ) //Last fake MC sample or data-driven fakes
			{
				output_histo_name = template_name+"_uuu__FakeMuMuMu" + syst_name;
				hist_uuu->Write(output_histo_name.Data());
				output_histo_name = template_name+"_uue__FakeMuMuEl" + syst_name;
				hist_uue->Write(output_histo_name.Data());
				output_histo_name = template_name+"_eeu__FakeElElMu" + syst_name;
				hist_eeu->Write(output_histo_name.Data());
				output_histo_name = template_name+"_eee__FakeElElEl" + syst_name;
				hist_eee->Write(output_histo_name.Data());
			}

			else //If fakes are not considered, or if sample is not fake --> write directly !
			{
				output_histo_name = template_name+"_uuu__" + sample_name + syst_name;
				hist_uuu->Write(output_histo_name.Data());
				output_histo_name = template_name+"_uue__" + sample_name + syst_name;
				hist_uue->Write(output_histo_name.Data());
				output_histo_name = template_name+"_eeu__" + sample_name + syst_name;
				hist_eeu->Write(output_histo_name.Data());
				output_histo_name = template_name+"_eee__" + sample_name + syst_name;
				hist_eee->Write(output_histo_name.Data());
			}

			//cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;

			delete tree; //Free memory
			if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") ) {delete tree_passTrig;}
			delete hist_uuu; delete hist_uue; delete hist_eeu; delete hist_eee; //Free memory
			delete file_input; //CHANGED -- free memory
		} //end sample loop

		if(dbgMode) cout<<"Done with syst : "<<syst_list[isyst]<<endl;
	} 	//end syst loop


	std::cout << "--- Created root file: "<<file_output->GetName()<<" containing the output histograms" << std::endl;
	delete file_output;
	std::cout << "==> Reader() is done!" << std::endl << std::endl;

	Rescale_Fake_Histograms(output_file_name);


	return 0;
}











//---------------------------------------------------------------------------
//  ######  ##     ## ########        #######  ##    ##       ########  ########  ########
// ##    ## ##     ##    ##          ##     ## ###   ##       ##     ## ##     ##    ##
// ##       ##     ##    ##          ##     ## ####  ##       ##     ## ##     ##    ##
// ##       ##     ##    ##          ##     ## ## ## ##       ########  ##     ##    ##
// ##       ##     ##    ##          ##     ## ##  ####       ##     ## ##     ##    ##
// ##    ## ##     ##    ##          ##     ## ##   ###       ##     ## ##     ##    ##
//  ######   #######     ##           #######  ##    ##       ########  ########     ##
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Reads template file and determines a cut value on BDT which maximizes bkg/sig ratio (kill signal to do CR plots/template fit with BDT)
 	--> Can use this cut value as input parameter in Create_Control_Trees()
 */
float theMVAtool::Determine_Control_Cut()
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Determine Control Cut ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	TString input_file_name;
	// if(isttZ) input_file_name = "outputs/Reader_BDTttZ" + this->filename_suffix + ".root";
	// else input_file_name = "outputs/Reader_BDT" + this->filename_suffix + ".root";

	input_file_name = "outputs/Combine_Input.root"; //CHANGED

	TFile* f = 0;
	f = TFile::Open(input_file_name.Data());
	if(f == 0) {cout<<endl<<"--- No templates "<<input_file_name.Data()<<" found -- Can't determine BDT CR cut value !"<<endl<<endl; return 0;}

	TH1F *h_sum_bkg(0), *h_sig(0), *h_tmp(0);

	TString input_histo_name = "";

	for(int isample=0; isample<sample_list.size(); isample++)
	{
		if(sample_list[isample].Contains("Data")) {continue;}
		//cout<<"--- Use sample "<<sample_list[isample]<<endl;

		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			h_tmp = 0;

			if(isttZ) input_histo_name = "BDTttZ_" + channel_list[ichan] + "__";
			else input_histo_name = "BDT_" + channel_list[ichan] + "__";
			if(!sample_list[isample].Contains("Fakes")) {input_histo_name+= sample_list[isample];}
			else //Special names for Fakes
			{
				if(channel_list[ichan]=="uuu") input_histo_name+= "FakeMuMuMu";
				else if(channel_list[ichan]=="uue") input_histo_name+= "FakeMuMuEl";
				else if(channel_list[ichan]=="eeu") input_histo_name+= "FakeElElMu";
				else if(channel_list[ichan]=="eee") input_histo_name+= "FakeElElEl";
			}

			if(!f->GetListOfKeys()->Contains(input_histo_name.Data())) {cout<<input_histo_name.Data()<<" not found !"<<endl; continue;}

			h_tmp = (TH1F*) f->Get(input_histo_name.Data())->Clone();

			if(sample_list[isample].Contains("tZq"))
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

	if( h_sig == 0)    {cout<<endl<<BOLD(FRED("--- Empty signal histogram ! Exit !"))<<endl<<endl; return 0;}
	if(h_sum_bkg == 0) {cout<<endl<<BOLD(FRED("--- Empty background sum histogram ! Exit !"))<<endl<<endl; return 0;}

	//S+B histogram
	TH1F* h_total = (TH1F*) h_sum_bkg->Clone();
	h_total->Add(h_sig);

	//Normalization
	//h_total->Scale(1/h_total->Integral()); //Integral() : Return integral of bin contents in range [binx1,binx2] (inclusive !)
	//h_sum_bkg->Scale(1/h_sum_bkg->Integral());
	//h_sig->Scale(1/h_sig->Integral());

	double sig_over_total = 100; //initialize to unreasonable value
	int bin_cut = -1; //initialize to false value
	int nofbins = h_total->GetNbinsX();

	for(int ibin=nofbins; ibin>0; ibin--)
	{
		// cout<<"ibin = "<<ibin<<" / (h_sig->Integral(1, ibin) / h_sig->Integral()) = "<<(h_sig->Integral(1, ibin) / h_sig->Integral())<<endl;

		//Search the bin w/ lowest sig/total, while keeping enough bkg events (criterion needs to be optimized/tuned)
		// if( (h_sig->Integral(1, ibin) / h_total->Integral(1, ibin)) < sig_over_total && (h_sum_bkg->Integral(1, ibin) / h_sum_bkg->Integral()) >= 0.6 )
		if( (h_sig->Integral(1, ibin) / h_sig->Integral()) <= 0.15 ) //If reject at least 85% signal => CUT
		{
			bin_cut = ibin;
			sig_over_total = h_sig->Integral(1, bin_cut) / h_total->Integral(1,bin_cut);
			break; //Stop there
		}
	}

	double cut = h_total->GetBinLowEdge(bin_cut+1); //Get the BDT cut value to apply to create a BDT CR control tree

	//Create plot to represent the cut on BDT
	TCanvas* c = new TCanvas("c", "BDT distributions");
	gStyle->SetOptStat(0);
	h_sum_bkg->GetXaxis()->SetTitle("Discriminant");
	h_sum_bkg->SetTitle("BDT distributions");
	h_sum_bkg->SetLineColor(kBlue);
	h_sum_bkg->SetLineWidth(2);
	h_sig->SetLineColor(kGreen+2);
	h_sig->SetLineWidth(2);
	h_sum_bkg->Draw("HIST");
	h_sig->Draw("HIST SAME");
	TLegend* leg = new TLegend(0.7,0.75,0.88,0.85);
    leg->SetHeader("");
    leg->AddEntry(h_sig,"Signal","L");
    leg->AddEntry("h_sum_bkg","Background","L");
    leg->Draw();
	//Draw vertical line at cut value
	TLine* l = new TLine(cut, 0, cut, h_sum_bkg->GetMaximum()*1.01);
	l->SetLineWidth(2);
	l->SetLineStyle(3);
	l->Draw("");

	mkdir("plots",0777); //Create directory if inexistant

	c->SaveAs("plots/Signal_Background_BDT"+this->filename_suffix+this->format);

	//Cout some results
	cout<<"---------------------------------------"<<endl;
	cout<<"* Cut Value = "<<cut<<endl;
	cout<<"-> BDT_CR defined w/ all events inside bins [1 ; "<<bin_cut<<"] of the BDT distribution!"<<endl<<endl;

	cout<<"--- Signal Integrals :"<<endl;
	cout<<"Before cut : "<<h_sig->Integral()<<endl;
	cout<<"After cut : "<<h_sig->Integral(1, bin_cut)<<endl;
	cout<<"--- Total Integrals :"<<endl;
	cout<<"Before cut : "<<h_total->Integral()<<endl;
	cout<<"After cut : "<<h_total->Integral(1, bin_cut)<<endl<<endl;

	cout<<"* Signal contamination after cut --> Sig/Total = "<<sig_over_total<<endl;
	cout<<"Bkg(cut) / Bkg(Total) = "<<h_sum_bkg->Integral(1,bin_cut) / h_sum_bkg->Integral()<<endl;
	cout<<"---------------------------------------"<<endl<<endl;

	//for(int i=0; i<h_sig->GetNbinsX(); i++) {cout<<"bin content "<<i+1<<" = "<<h_sig->GetBinContent(i+1)<<endl;} //If want to verify that the signal is computed correctly
	delete  h_sum_bkg; delete h_sig; delete h_tmp;
	delete f;
	delete c; delete leg; delete l;

	return cut;
}













//---------------------------------------------------------------------------
//  ######      ########            ######## ########  ######## ########  ######
// ##    ##     ##     ##              ##    ##     ## ##       ##       ##    ##
// ##           ##     ##              ##    ##     ## ##       ##       ##
// ##           ########               ##    ########  ######   ######    ######
// ##           ##   ##                ##    ##   ##   ##       ##             ##
// ##    ## ### ##    ##  ###          ##    ##    ##  ##       ##       ##    ##
//  ######  ### ##     ## ###          ##    ##     ## ######## ########  ######
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Creates output with trees containing only events verifying BDT<cut (mainly bkg events) --> Then can create histogram with these events for further control studies
 * @param fakes_from_data [If true, use fakes from data sample]
 * @param cut_on_BDT      [If true, cut on BDT to go ton a Control Region]
 * @param cut             [Cut value]
 * @param use_pseudodata  [If true, use pseudodata rather than real data]
 */
void theMVAtool::Create_Control_Trees(bool fakes_from_data, bool cut_on_BDT, double cut, bool use_pseudodata)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Create Control Trees ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
	else {cout<<FYEL("--- Using fakes from data ---")<<endl;}
	if(cut_on_BDT) {cout<<FYEL("--- Creating control tree WITH cut on BDT value // Cut value = "<<cut<<" ----")<<endl;}
	else if(!cut_on_BDT) {cout<<FYEL("--- Creating control tree WITHOUT cut on BDT value ---")<<endl;}

	delete reader; //Free any previous allocated memory, and re-initialize reader
	reader = new TMVA::Reader( "!Color:!Silent" );

	mkdir("outputs",0777);

	// Name & adress of local variables which carry the updated input values during the event loop
	// - the variable names MUST corresponds in name and type to those given in the weight file(s) used
	for(int i=0; i<var_list.size(); i++)
	{
		reader->AddVariable(var_list[i].Data(), &var_list_floats[i]);
	}
	for(int i=0; i<v_cut_name.size(); i++)
	{
		if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);}
		else {reader->AddSpectator(v_cut_name[i].Data(), &v_cut_float[i]);}
	}
	for(int i=0; i<v_add_var_names.size(); i++)
	{
		reader->AddSpectator(v_add_var_names[i].Data(), &v_add_var_floats[i]);
	}

	// --- Book the MVA methods
	TString dir    = "weights/";
	TString BDT_type = "";
	if(isttZ) BDT_type = "BDTttZ";
	else BDT_type = "BDT";

	// Book method
	for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
	{
	  TString MVA_method_name = BDT_type + "_" + channel_list[ichan] + this->filename_suffix + TString(" method");
	  TString weightfile = dir + BDT_type + "_" + channel_list[ichan] + this->filename_suffix + TString(".weights.xml");
	  if(cut_on_BDT) {reader->BookMVA( MVA_method_name, weightfile );}
	}


	bool create_syst_histos = false; //NOTE : for now ,simply disactivate systematics for control plots (too long)
	// if(syst_list.size() != 1)
	// {
	// 	string answer = "";
	// 	cout<<FYEL("Do you want to create histograms for the systematics also ? This will increase")<<BOLD(FYEL(" A LOT "))<<FYEL("the processing time ! -- Type yes/no")<<endl;
	// 	cin>>answer;
	//
	// 	while(answer != "yes" && answer!= "no")
	// 	{
	// 		cout<<"Wrong answer -- Type 'yes' (with syst.) or 'no' (without syst.) !"<<endl;
	// 		cin.clear();
	// 		cin.ignore(1000, '\n');
	// 		cin>>answer;
	// 	}
	//
	// 	if(answer == "yes") create_syst_histos = true;
	// }

	TFile* file_input;
	TTree* tree = 0;


	TString output_file_name = "outputs/Control_Trees" + this->filename_suffix;
	if(cut_on_BDT) output_file_name+= "_CutBDT";
	output_file_name+= ".root";
	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );
	TTree *tree_control = 0;


//---Loop on histograms
	for(int isyst = 0; isyst < syst_list.size(); isyst++)
	{
		if(!create_syst_histos && syst_list[isyst] != "") {continue;} //Don't need systematics for Control plots...

		cout<<"Syst : "<<syst_list[isyst]<<endl;

		for(int isample=0; isample<sample_list.size(); isample++)
		{
			if(use_pseudodata && sample_list[isample] == "Data") {continue;} //Will generate pseudodata instead

			if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
			else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

			if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}


			if( (sample_list[isample] == "Data" && syst_list[isyst]!="")
				|| ( sample_list[isample].Contains("Fakes") && syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") ) ) {continue;} //Data = no syst. -- Fakes = only fake syst.

			if( syst_list[isyst].Contains("Fakes") && !sample_list[isample].Contains("Fakes") )   {continue;} //Fake syst. only for "fakes" samples

			if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only


			TString inputfile = dir_ntuples + "/FCNCNTuple_" + sample_list[isample] + ".root";

			//For these systematics, info stored in separate ntuples ! (events are not the same)
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQup.root";}
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__minus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQdw.root";}
			else if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "Hadron__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqhwpp.root";} //For minus, take nominal //FIXME


			file_input = 0;
			file_input = TFile::Open( inputfile.Data() );
			if(!file_input) {cout<<inputfile.Data()<<" not found!"<<endl; continue;}

			file_input->cd();
			tree = 0;
			tree = new TTree("tree", "input tree"); //CHANGED


			output_file->cd();
			tree_control = 0;
			tree_control = new TTree("tree_control", "Control Tree"); //CHANGED



			for(int ivar=0; ivar<var_list.size(); ivar++)
			{
				TString var_type = var_list[ivar] + "/F";
				tree_control->Branch(var_list[ivar].Data(), &(var_list_floats[ivar]), var_type.Data());
			}
			for(int ivar=0; ivar<v_cut_name.size(); ivar++)
			{
				TString var_type = v_cut_name[ivar] + "/F";
				tree_control->Branch(v_cut_name[ivar].Data(), &v_cut_float[ivar], var_type.Data());
			}
			for(int ivar=0; ivar<v_add_var_names.size(); ivar++)
			{
				TString var_type = v_add_var_names[ivar] + "/F";
				tree_control->Branch(v_add_var_names[ivar].Data(), &v_add_var_floats[ivar], var_type.Data());
			}

			float weight; float i_channel;
			tree_control->Branch("Weight", &weight, "weight/F"); //Give it the same name regardless of the systematic, since we create a separate tree for each syst anyway
			tree_control->Branch("Channel", &i_channel, "i_channel/F");



			//For JES & JER systematics, need a different tree (modify the variables distributions' shapes)
			if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") ) {tree = (TTree*) file_input->Get(syst_list[isyst].Data());}
			else {tree = (TTree*) file_input->Get(t_name.Data());}


			// SetBranchAddress
			for(int i=0; i<var_list.size(); i++)
			{
				tree->SetBranchAddress(var_list[i].Data(), &var_list_floats[i]);
			}
			for(int i=0; i<v_cut_name.size(); i++)
			{
				tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);
			}
			for(int i=0; i<v_add_var_names.size(); i++)
			{
				tree->SetBranchAddress(v_add_var_names[i].Data(), &v_add_var_floats[i]);
			}
			tree->SetBranchAddress("Channel", &i_channel);
			//For all other systematics, only the events weights change
			if( syst_list[isyst] == "" || syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") || syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") )	{tree->SetBranchAddress("Weight", &weight);}
			else {tree->SetBranchAddress(syst_list[isyst].Data(), &weight);}


			// --- Event loop
			for(int ievt=0; ievt<tree->GetEntries(); ievt++)
			{
				weight = 0; i_channel = 9;

				tree->GetEntry(ievt);

				//NOTE -- Can modify weight here (fabs, neg, etc.)
				// if(weight<0) {continue;}
				// weight = fabs(weight);


//------------------------------------------------------------

//------------------------------------------------------------
//------------------- Apply cuts -----------------------------
				float cut_tmp = 0; bool pass_all_cuts = true;


				for(int ivar=0; ivar<v_cut_name.size(); ivar++)
				{
					if(v_cut_def[ivar] != "")
					{
						if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) //If cut contains only 1 condition
						{
							cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);

							if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break;}
							else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break;}
							else if(v_cut_def[ivar].Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break;}
							else if(v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
							else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break;}
						}
						else if(v_cut_def[ivar].Contains("&&") && v_cut_def[ivar].Contains("||")) {cout<<BOLD(FRED("ERROR ! Wrong cut definition !"))<<endl;}
						else if(v_cut_def[ivar].Contains("&&") )//If '&&' in the cut def, break it in 2
						{
							TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first;
							TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
							//CUT 1
							cut_tmp = Find_Number_In_TString(cut1);
							if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
							else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
							else if(cut1.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
							else if(cut1.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
							else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
							//CUT 2
							cut_tmp = Find_Number_In_TString(cut2);
							if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
							else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
							else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
							else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
							else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
						}
						else if(v_cut_def[ivar].Contains("||") )//If '||' in the cut def, break it in 2
						{
							TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first;
							TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;

							bool pass_cut1 = true; bool pass_cut2 = true; //Need to pass at least 1 cut

							//CUT 1
							cut_tmp = Find_Number_In_TString(cut1);
							if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_cut1 = false;}
							else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_cut1 = false;}
							else if(cut1.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_cut1 = false;}
							else if(cut1.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_cut1 = false;}
							else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_cut1 = false;}
							//CUT 2
							cut_tmp = Find_Number_In_TString(cut2);
							if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_cut2 = false;}
							else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_cut2 = false;}
							else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_cut2 = false;}
							else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_cut2 = false;}
							else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_cut2 = false;}

							if(!pass_cut1 && !pass_cut2) {pass_all_cuts = false; break;}
						}
					}
				}


				if(!pass_all_cuts) {continue;}
//------------------------------------------------------------

//------------------------------------------------------------
				if(cut_on_BDT)
				{
					if(i_channel == 0 && reader->EvaluateMVA( BDT_type + "_uuu"+this->filename_suffix+" method") > cut) 		{continue;}
					else if(i_channel == 1 && reader->EvaluateMVA( BDT_type + "_uue"+this->filename_suffix+" method") > cut) 	{continue;}
					else if(i_channel == 2 && reader->EvaluateMVA( BDT_type + "_eeu"+this->filename_suffix+" method") > cut) 	{continue;}
					else if(i_channel == 3 && reader->EvaluateMVA( BDT_type + "_eee"+this->filename_suffix+" method") > cut) 	{continue;}
				}

				tree_control->Fill();
			} //end event loop

			// --- Write histograms

			output_file->cd();

			//NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
			TString output_tree_name = "Control_" + sample_list[isample];
			if (syst_list[isyst] != "")
			{
				if(combine_naming_convention) output_tree_name+= "_" + Combine_Naming_Convention(syst_list[isyst]);
				else output_tree_name+= "_" + Theta_Naming_Convention(syst_list[isyst]);
			}

			tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);

			delete tree_control; delete tree;
			delete file_input;

			//cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
		} //end sample loop
		cout<<"Done with "<<syst_list[isyst]<<" syst"<<endl;

	} //end syst loop

	delete output_file;

	std::cout << "--- Created root file containing the output trees" << std::endl;
	std::cout << "==> Create_Control_Trees() is done!" << std::endl << std::endl;
}











//---------------------------------------------------------------------------
//  ######      ########            ##     ## ####  ######  ########  ######
// ##    ##     ##     ##           ##     ##  ##  ##    ##    ##    ##    ##
// ##           ##     ##           ##     ##  ##  ##          ##    ##
// ##           ########            #########  ##   ######     ##     ######
// ##           ##   ##             ##     ##  ##        ##    ##          ##
// ##    ## ### ##    ##  ###       ##     ##  ##  ##    ##    ##    ##    ##
//  ######  ### ##     ## ###       ##     ## ####  ######     ##     ######
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//NB : no separation by channel is made ! It is possible but it would require to create 4 times histograms at beginning...
/**
 * Create histograms from control trees (in same file)
 * @param fakes_from_data         If true, use fakes from data sample
 * @param use_pseudodata_CR_plots If true, use pseudodata
 */
void theMVAtool::Create_Control_Histograms(bool fakes_from_data, bool use_pseudodata_CR_plots, bool fakes_summed_channels, bool cut_on_BDT)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Create Control Histograms ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
	else {cout<<"--- Using fakes from data ---"<<endl<<endl;}
	if(fakes_summed_channels) {cout<<FYEL("--- Fakes : summing channels 2 by 2 (artificial stat. increase) ---")<<endl<<endl;}


	TString input_file_name = "outputs/Control_Trees" + this->filename_suffix + ".root";
	if(cut_on_BDT) input_file_name = "outputs/Control_Trees" + this->filename_suffix + "_CutBDT.root";
	TString output_file_name = "outputs/Control_Histograms" + this->filename_suffix + ".root";
	if(cut_on_BDT) output_file_name = "outputs/Control_Histograms" + this->filename_suffix + "_CutBDT.root";

	TFile* f_input = 0;
	f_input = TFile::Open( input_file_name.Data() );
	if(!f_input) {cout<<input_file_name.Data()<<" not found ! Abort"<<endl;}
	TFile* f_output = TFile::Open( output_file_name.Data(), "RECREATE" );
	TTree* tree = 0;

	int binning = 10;
	TH1::SetDefaultSumw2();

	//Want to plot ALL variables (inside the 2 different variable vectors !)
	vector<TString> total_var_list;
	for(int i=0; i<v_cut_name.size(); i++)
	{
		total_var_list.push_back(v_cut_name[i].Data());
	}
	for(int i=0; i<var_list.size(); i++)
	{
		total_var_list.push_back(var_list[i].Data());
	}
	for(int i=0; i<v_add_var_names.size(); i++)
	{
		total_var_list.push_back(v_add_var_names[i].Data());
	}

	bool restart_loop = false; //Do it this way so that everytime a 'useless variable' is removed from vector, we restart the whole loop to check for other variables (because doing this changes the index of each element --> restart from scratch)
	do
	{
		restart_loop = false;
		for(int i=0; i<total_var_list.size(); i++)
		{
			if(total_var_list[i] == "passTrig" || total_var_list[i] == "RunNr") {total_var_list.erase(total_var_list.begin() + i); restart_loop = true; break;} //Useless vars
		}
	} while (restart_loop);


	bool create_syst_histos = false; //FIXME -- allow user to choose ?
	// if(syst_list.size() != 1)
	// {
	// 	string answer = "";
	// 	cout<<FYEL("Do you want to create histograms for the systematics also ? This will increase")<<BOLD(FYEL(" A LOT "))<<FYEL("the processing time ! -- Type yes/no")<<endl;
	// 	cin>>answer;
	//
	// 	while(answer != "yes" && answer!= "no")
	// 	{
	// 		cout<<"Wrong answer -- Type 'yes' (with syst.) or 'no' (without syst.) !"<<endl;
	// 		cin.clear();
	// 		cin.ignore(1000, '\n');
	// 		cin>>answer;
	// 	}
	//
	// 	if(answer == "yes") create_syst_histos = true;
	// }

	int nof_histos_to_create = ((sample_list.size() - 1) * total_var_list.size() * syst_list.size()) + total_var_list.size();
	if(!create_syst_histos) nof_histos_to_create = ((sample_list.size() - 1) * total_var_list.size() ) + total_var_list.size();


	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		cout<<endl<<"################################"<<endl;
		cout<<" *** CHANNEL "<<channel_list[ichan]<<" ***"<<endl;
		cout<<endl<<"--- Creation of "<<nof_histos_to_create<<" histograms !"<<endl;
		cout<<"--- This might take a while... !"<<endl;
		cout<<"################################"<<endl<<endl;

		int i_hist_created = 0;

		for(int ivar=0; ivar<total_var_list.size(); ivar++)
		{
			if(dbgMode) cout<<"--- Processing variable : "<<total_var_list[ivar]<<endl;
			//Info contained in tree leaves. Need to create histograms first
			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				if(use_pseudodata_CR_plots && sample_list[isample] == "Data") {continue;}

				bool sum_fakes = false;
				if(fakes_summed_channels && (sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW")))
				{
					sum_fakes = true;
				}

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					if(!create_syst_histos && syst_list[isyst] != "") {continue;} //If don't want to create histos for the systematics

					if(!fakes_from_data && sample_list[isample]=="Fakes") {continue;} //Fakes from MC only
					else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

					if( (sample_list[isample] == "Data" && syst_list[isyst]!="")
						|| ( sample_list[isample].Contains("Fakes") && syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") ) ) {continue;} //Data = no syst. -- Fakes = only fake syst.

					if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}

					if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

					if(i_hist_created%100==0 && i_hist_created != 0) {cout<<"--- "<<i_hist_created<<"/"<<nof_histos_to_create<<endl;}

					TH1F* h_tmp = 0;
					if(total_var_list[ivar] == "mTW") 								{h_tmp = new TH1F( "","", binning, 0, 250 );}
					else if(total_var_list[ivar] == "METpt")						{h_tmp = new TH1F( "","", binning, 0, 200 );}
					else if(total_var_list[ivar] == "m3l")							{h_tmp = new TH1F( "","", binning, 0., 450. );}
					else if(total_var_list[ivar] == "ZMass") 						{h_tmp = new TH1F( "","", binning, 75, 105 );}
					else if(total_var_list[ivar] == "dPhiAddLepB") 					{h_tmp = new TH1F( "","", binning, -3.15, 3.15 );}
					else if(total_var_list[ivar] == "Zpt") 							{h_tmp = new TH1F( "","", binning, 0, 300 );}
					else if(total_var_list[ivar] == "ZEta")			 				{h_tmp = new TH1F( "","", binning, -4, 4 );}
					else if(total_var_list[ivar] == "AddLepAsym") 					{h_tmp = new TH1F( "","", binning, -3, 3 );}
					else if(total_var_list[ivar] == "mtop") 						{h_tmp = new TH1F( "","", binning, 60, 500 );}
					else if(total_var_list[ivar] == "btagDiscri") 					{h_tmp = new TH1F( "","", 10, 0., 1.2 );}
					else if(total_var_list[ivar] == "etaQ")							{h_tmp = new TH1F( "","", binning, -4.5, 4.5 );}
					else if(total_var_list[ivar] == "NBJets")						{h_tmp = new TH1F( "","", 5, 0, 5 );}
					else if(total_var_list[ivar] == "AddLepPT")						{h_tmp = new TH1F( "","", binning, 0, 200 );}
					else if(total_var_list[ivar] == "AddLepETA")					{h_tmp = new TH1F( "","", binning, -2.5, 2.5 );}
					else if(total_var_list[ivar] == "LeadJetPT")					{h_tmp = new TH1F( "","", binning, 0, 300 );}
					else if(total_var_list[ivar] == "LeadJetEta")					{h_tmp = new TH1F( "","", binning, -4.5, 4.5 );}
					else if(total_var_list[ivar] == "dPhiZMET")						{h_tmp = new TH1F( "","", 20, -3.15, 3.15 );}
					else if(total_var_list[ivar] == "dPhiZAddLep")					{h_tmp = new TH1F( "","", binning, -3.15,3.15 );}
					else if(total_var_list[ivar] == "dRAddLepB")					{h_tmp = new TH1F( "","", binning, 0, 5 );}
					else if(total_var_list[ivar] == "dRZAddLep")					{h_tmp = new TH1F( "","", binning, 0, 5 );}
					else if(total_var_list[ivar] == "dRZTop")						{h_tmp = new TH1F( "","", binning, 0, 5 );}
					else if(total_var_list[ivar] == "TopPT")						{h_tmp = new TH1F( "","", binning, 0, 500 );}
					else if(total_var_list[ivar] == "NJets")						{h_tmp = new TH1F( "","", 6, 0, 6 );}
					else if(total_var_list[ivar] == "ptQ")							{h_tmp = new TH1F( "","", binning, 0, 300 );}
					else if(total_var_list[ivar] == "dRjj")							{h_tmp = new TH1F( "","", binning, 0, 5 );}
					else if(total_var_list[ivar] == "AdditionalEleIso")				{h_tmp = new TH1F( "","", binning, 0, 0.5 );}
					else if(total_var_list[ivar] == "AdditionalMuonIso")			{h_tmp = new TH1F( "","", binning, 0, 0.5 );}
					else if(total_var_list[ivar] == "AddLepPT")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
					else if(total_var_list[ivar] == "dRAddLepQ")					{h_tmp = new TH1F( "","", binning, 0, 5 );}
					else if(total_var_list[ivar] == "dRAddLepClosestJet")			{h_tmp = new TH1F( "","", binning, 0, 5 );}
					else if(total_var_list[ivar] == "tZq_pT")						{h_tmp = new TH1F( "","", binning, 0, 300 );}
					else if(total_var_list[ivar] == "tZq_mass")						{h_tmp = new TH1F( "","", binning, 0, 300 );}
					else if(total_var_list[ivar] == "leadingLeptonPT")              {h_tmp = new TH1F( "","", binning, 0, 150 );}
					else if(total_var_list[ivar] == "MAddLepB")                     {h_tmp = new TH1F( "","", binning, 0, 500 );}
					else if(total_var_list[ivar] == "MEMvar_0")						{h_tmp = new TH1F( "","", binning, 0, 10 );}
					else if(total_var_list[ivar] == "MEMvar_1")						{h_tmp = new TH1F( "","", binning, 0, 10 );}
					else if(total_var_list[ivar] == "MEMvar_2")						{h_tmp = new TH1F( "","", binning, 0, 10 );}
					else if(total_var_list[ivar] == "MEMvar_3")						{h_tmp = new TH1F( "","", binning, 0, 10 );}
					else if(total_var_list[ivar] == "RunNr")							{h_tmp = new TH1F( "","", 500, 0, 400000 );}
					// else if(total_var_list[ivar] == "")							{h_tmp = new TH1F( "","", binning, 0, 150 );}

					else {cout<<endl<<"theMVAtool.cc - l."<<__LINE__<<BOLD(FRED(" --> !!Unknown variable!! ")) << total_var_list[ivar] << " Correct name or add it here" <<endl<<endl;}

					TString tree_name = "Control_" + sample_list[isample];
					if(syst_list[isyst] != "") {tree_name+= "_" + syst_list[isyst];}
					if( !f_input->GetListOfKeys()->Contains(tree_name.Data()) && sample_list[isample] != "Data" ) {if(isyst==0 && ivar==0) {cout<<__LINE__<<" : "<<tree_name<<" not found (missing sample?) -- [Stop error messages]"<<endl;} continue;}
					tree = (TTree*) f_input->Get(tree_name.Data());
					// cout<<__LINE__<<endl;

					float weight = 0, tmp = 0, i_channel = 9;
					tree->SetBranchAddress(total_var_list[ivar], &tmp); //One variable at a time
					tree->SetBranchAddress("Weight", &weight);
					tree->SetBranchAddress("Channel", &i_channel);

					int tree_nentries = tree->GetEntries();

					for(int ientry = 0; ientry<tree_nentries; ientry++)
					{
						weight = 0; tmp = 0; i_channel = 9;
						tree->GetEntry(ientry); //Read event

						//NB : No need to re-apply variables cuts here, as the control_tree is only filled with events that pass these cuts

						if(sum_fakes)
						{
							if( (channel_list[ichan] == "uuu" || channel_list[ichan] == "eeu") && (i_channel!= 0 && i_channel!= 2) ) {continue;}
							else if( (channel_list[ichan] == "eee" || channel_list[ichan] == "uue") && (i_channel!= 1 && i_channel!= 3) ) {continue;}
						}
						else
						{
							if(channel_list[ichan] == "uuu" && i_channel!= 0) {continue;}
							else if(channel_list[ichan] == "uue" && i_channel!= 1) {continue;}
							else if(channel_list[ichan] == "eeu" && i_channel!= 2) {continue;}
							else if(channel_list[ichan] == "eee" && i_channel!= 3) {continue;}
							else if(channel_list[ichan] == "9") {cout<<__LINE__<<" : problem !"<<endl;}
						}


						if(tmp < h_tmp->GetXaxis()->GetXmax() ) {h_tmp->Fill(tmp, weight);}
						//CHANGED
						else  {h_tmp->SetBinContent(h_tmp->GetNbinsX(), (h_tmp->GetBinContent(h_tmp->GetNbinsX() ) + weight) );} //Avoid to get overflow because of inappropriate binning --> Put it into last bin instead!
					}

					//Re-scale to desired luminosity, unless it's data
					if(sample_list[isample] != "Data")
					{
						h_tmp->Scale(luminosity_rescale);
					}



					// //FIXME -- remove negative values/ERRORS
					// // if(sample_list[isample].Contains("tZq") && total_var_list[ivar] == "NJets" && channel_list[ichan]=="uuu") cout<<"BEfore / bincontent(5) = "<<h_tmp->GetBinContent(5)<<" , binErrorLow(5) = "<<h_tmp->GetBinErrorLow(5)<<endl;
					// for(int ibin=1; ibin<h_tmp->GetNbinsX()+1; ibin++)
					// {
					// 	if(h_tmp->GetBinContent(ibin) < 0) h_tmp->SetBinContent(ibin, 0);
					// 	if(h_tmp->GetBinContent(ibin) - h_tmp->GetBinErrorLow(ibin) < 0 )
					// 	{
					// 		h_tmp->SetBinError(ibin, h_tmp->GetBinContent(ibin));
					// 	}
					// }

					// if(sample_list[isample].Contains("tZq") && total_var_list[ivar] == "NJets" && channel_list[ichan]=="uuu") cout<<"AFTER / bincontent(5) = "<<h_tmp->GetBinContent(5)<<" , binErrorLow(4) = "<<h_tmp->GetBinErrorLow(5)<<endl;




					TString fake_name = "";
					if(sample_list[isample] == "Fakes")
					{
						if(channel_list[ichan]=="uuu") fake_name = "FakeMuMuMu";
						else if(channel_list[ichan]=="uue") fake_name = "FakeMuMuEl";
						else if(channel_list[ichan]=="eeu") fake_name = "FakeElElMu";
						else if(channel_list[ichan]=="eee") fake_name = "FakeElElEl";
					}


					TString output_histo_name = total_var_list[ivar] + "_" + channel_list[ichan] + "__";

					if(sample_list[isample]=="Data")
					{
						if(combine_naming_convention) output_histo_name+= "data_obs";
						else output_histo_name+= "DATA";
					}
					else if(sample_list[isample]=="Fakes") output_histo_name+= fake_name;
					else output_histo_name+= sample_list[isample];

					if(syst_list[isyst] != "")
					{
						if(combine_naming_convention) output_histo_name+= "__" + Combine_Naming_Convention(syst_list[isyst]);
						else output_histo_name+= "__" + Theta_Naming_Convention(syst_list[isyst]);
					}

					f_output->cd();
					h_tmp->Write(output_histo_name.Data());

					delete h_tmp; delete tree; //FRee memory

					i_hist_created++;
				} //end syst loop
			} //end sample loop
		} //end var loop
	} //end channel loop


	delete f_input;
	delete f_output;


	//CHANGED
	Rescale_Fake_Histograms(output_file_name);

	return ;
}










//---------------------------------------------------------------------------
// ########      ######     ########    ##     ##    ########      #######     ########        ###       ########       ###
// ##     ##    ##    ##    ##          ##     ##    ##     ##    ##     ##    ##     ##      ## ##         ##         ## ##
// ##     ##    ##          ##          ##     ##    ##     ##    ##     ##    ##     ##     ##   ##        ##        ##   ##
// ########      ######     ######      ##     ##    ##     ##    ##     ##    ##     ##    ##     ##       ##       ##     ##
// ##                 ##    ##          ##     ##    ##     ##    ##     ##    ##     ##    #########       ##       #########
// ##           ##    ##    ##          ##     ##    ##     ##    ##     ##    ##     ##    ##     ##       ##       ##     ##
// ##            ######     ########     #######     ########      #######     ########     ##     ##       ##       ##     ##
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Generates pseudo data histograms in the "control histograms file".
 * @param  fakes_from_data If true, use Fakes from data sample
 */
int theMVAtool::Generate_PseudoData_Histograms_For_Control_Plots(bool fakes_from_data)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Generate PseudoData Histos for CR Plots ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
	else {cout<<FYEL("--- Using fakes from data ---")<<endl;}

	TRandom3 therand(0);

	mkdir("outputs",0777);

	TString input_name = "outputs/Control_Histograms" + this->filename_suffix + ".root";
    TFile* file = 0;
	file = TFile::Open( input_name.Data(), "UPDATE");
	if(file == 0) {cout<<BOLD(FRED("--- ERROR : Control_Histograms file not found ! Exit !"))<<endl; return 0;}


	//Want to plot ALL variables (inside the 2 different variable vectors !)
	vector<TString> total_var_list;
	for(int i=0; i<v_cut_name.size(); i++)
	{
		total_var_list.push_back(v_cut_name[i].Data());
	}
	for(int i=0; i<var_list.size(); i++)
	{
		total_var_list.push_back(var_list[i].Data());
	}

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		TH1F *h_sum = 0, *h_tmp = 0;

		for(int ivar=0; ivar<total_var_list.size(); ivar++)
		{
			cout<<"--- "<<total_var_list[ivar]<<endl;

			h_sum = 0;

			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				if(sample_list[isample]=="Data") {continue;} //Not using real data !!
				if(!fakes_from_data && sample_list[isample]=="Fakes" ) {continue;} //Fakes from MC only
				else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

				h_tmp = 0;
				//CHANGED
				// TString histo_name = "Control_" + channel_list[ichan] + "_" + total_var_list[ivar] + "_" + sample_list[isample];
				TString histo_name = total_var_list[ivar]+"_" + channel_list[ichan] + "__" + sample_list[isample];
				if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
				h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
				if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
				else {h_sum->Add(h_tmp);}
			}

			int nofbins = h_sum->GetNbinsX();

			for(int i=1; i<nofbins+1; i++)
			{
				int bin_content = h_sum->GetBinContent(i); //cout<<"Initial content = "<<bin_content<<endl;
				int new_bin_content = therand.Poisson(bin_content); //cout<<"New content = "<<new_bin_content<<endl;
				h_sum->SetBinContent(i, new_bin_content);
				h_sum->SetBinError(i, sqrt(new_bin_content)); //Poissonian error
			}

			file->cd();
			TString output_histo_name;
			if(combine_naming_convention) output_histo_name =  total_var_list[ivar]+"_" + channel_list[ichan] + "__data_obs";
			else output_histo_name =  total_var_list[ivar]+"_" + channel_list[ichan] + "__DATA";

			h_sum->Write(output_histo_name, TObject::kOverwrite);
		} //end var loop
	} //end channel loop

	delete file;

	cout<<"--- Done with generation of pseudo-data for CR"<<endl; return 0;
}











//NOTE : GENERATE PSEUDODATA ON COMBINE INPUT FILE (fakes already re-scaled)
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Generate pseudo-data histograms from MC, using TRandom::Poisson to simulate statistical fluctuations
 	Used to simulate template fit to pseudo-data, to avoid using real data before pre-approval
 * @param  template_name Template name : BDT / BDTttZ / mTW
 */
int theMVAtool::Generate_PseudoData_Templates(TString template_name)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" PseudoData Templates ---")<<endl;}
	else {cout<<BOLD(FRED("--- ERROR : invalid template_name value ! Exit !"))<<endl; return 0;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;


	TRandom1 therand(0); //Randomization

	TString pseudodata_input_name = "outputs/Combine_Input.root";
	// TString pseudodata_input_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root";
    TFile* file = 0;
	file = TFile::Open( pseudodata_input_name.Data(), "UPDATE");
	if(file == 0) {cout<<BOLD(FRED("--- ERROR : Reader file not found ! Exit !"))<<endl; return 0;}

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		TH1F *h_sum = 0, *h_tmp = 0;

		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("DY") ) {continue;}

			h_tmp = 0;
			TString histo_name = template_name + "_" + channel_list[ichan] + "__" + sample_list[isample];
			if(sample_list[isample].Contains("Fakes"))
			{
				TString template_fake_name = "";
				if(      channel_list[ichan] == "uuu") {template_fake_name = "FakeMuMuMu";}
				else if( channel_list[ichan] == "eeu") {template_fake_name = "FakeElElMu";}
				else if( channel_list[ichan] == "uue") {template_fake_name = "FakeMuMuEl";}
				else if( channel_list[ichan] == "eee") {template_fake_name = "FakeElElEl";}

				histo_name = template_name + "_" + channel_list[ichan] + "__" + template_fake_name;
			}

			if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; return 0;}
			h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
			if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
			else {h_sum->Add(h_tmp);}


			// cout<<"chan "<<channel_list[ichan]<<", sample "<<sample_list[isample]<<" : bin[1] = "<<h_sum->GetBinContent(1)<<endl;
		}



		if(h_sum == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; return 0;}
		int nofbins = h_sum->GetNbinsX();

		for(int i=1; i<nofbins+1; i++)
		{
			double bin_content = h_sum->GetBinContent(i); cout<<"bin "<<i<<endl; cout<<"initial content = "<<bin_content<<endl;
			int new_bin_content = therand.Poisson(bin_content); cout<<"new content = "<<new_bin_content<<endl;
			h_sum->SetBinContent(i, new_bin_content);
			h_sum->SetBinError(i, sqrt(new_bin_content)); //Poissonian error
		}

		file->cd();
		TString output_histo_name;
		if(combine_naming_convention) output_histo_name = template_name + "_" + channel_list[ichan] + "__data_obs"; //Combine
		else output_histo_name = template_name + "_" + channel_list[ichan] + "__DATA"; //THETA
		h_sum->Write(output_histo_name, TObject::kOverwrite);

	} //end channel loop

	delete file;
	cout<<"--- Done with generation of pseudo-data"<<endl<<endl; return 0;
}














//-----------------------------------------------------------------------------
// ########  ########     ###    ##      ##        ######  ########        ########  ##        #######  ########  ######
// ##     ## ##     ##   ## ##   ##  ##  ##       ##    ## ##     ##       ##     ## ##       ##     ##    ##    ##    ##
// ##     ## ##     ##  ##   ##  ##  ##  ##       ##       ##     ##       ##     ## ##       ##     ##    ##    ##
// ##     ## ########  ##     ## ##  ##  ##       ##       ########        ########  ##       ##     ##    ##     ######
// ##     ## ##   ##   ######### ##  ##  ##       ##       ##   ##         ##        ##       ##     ##    ##          ##
// ##     ## ##    ##  ##     ## ##  ##  ##       ##    ## ##    ##        ##        ##       ##     ##    ##    ##    ##
// ########  ##     ## ##     ##  ###  ###         ######  ##     ##       ##        ########  #######     ##     ######
//-----------------------------------------------------------------------------------------



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Draw control plots with histograms created with Create_Control_Histograms
 * @param  channel               Channel name
 * @param  fakes_from_data       If true, use Fakes from data sample
 * @param  fakes_summed_channels If true, sum uuu+eeu & eee+uue for fakes
 * @param  allchannels           If true, sum 4 channels
 * @param  postfit           	 Decide if produce prefit OR postfit plots (NB : different files/hist names)
 */
int theMVAtool::Draw_Control_Plots(TString channel, bool fakes_from_data, bool allchannels, bool postfit, bool cut_on_BDT)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Draw Control Plots ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	//CHANGED
	//Decide with boolean if want to produce prefit OR postfit plots.
	//NB : different naming conventions (throughout the function)
	TFile* f = 0; double sig_strength = 1;
	if(postfit) //Take output from combineHarvester
	{
		//CombineHarvester doesn't rescale automatically signal to Signal strength ! Need to do it manually, extracting its value from mlfit.root !
		TString input_file_name = "outputs/mlfit.root";
		f = TFile::Open( input_file_name );
		if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
		TTree* t_mu = 0;
		t_mu = (TTree*) f->Get("tree_fit_sb"); if(!t_mu) {cout<<"No tree found!"; return 0;}
		t_mu->GetEntry(0);
		sig_strength = (double) t_mu->GetLeaf("mu")->GetValue(0);
		cout<<"SIGNAL STRENGTH = "<<sig_strength<<endl;
		delete f; //Free memory

		input_file_name = "outputs/PostfitInputVars.root";
		f = TFile::Open( input_file_name );
		if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
	}
	else
	{
		TString input_file_name = "outputs/Control_Histograms" + this->filename_suffix + ".root";
		if(cut_on_BDT) input_file_name = "outputs/Control_Histograms" + this->filename_suffix + "_CutBDT.root";
		f = TFile::Open( input_file_name );
		if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
	}

	TH1::SetDefaultSumw2();

	mkdir("plots",0777); //Create directory if inexistant

	TH1F *h_tmp = 0, *h_data = 0;
	THStack *stack = 0;

	vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
	thechannellist.push_back("uuu");
	thechannellist.push_back("uue");
	thechannellist.push_back("eeu");
	thechannellist.push_back("eee");

	//Load Canvas definition
	Load_Canvas_Style();

	//Want to plot ALL activated variables (inside the 2 different variable vectors !)
	vector<TString> total_var_list;
	for(int i=0; i<v_cut_name.size(); i++)
	{
		total_var_list.push_back(v_cut_name[i].Data());
	}
	for(int i=0; i<var_list.size(); i++)
	{
		total_var_list.push_back(var_list[i].Data());
	}
	for(int i=0; i<v_add_var_names.size(); i++)
	{
		total_var_list.push_back(v_add_var_names[i].Data());
	}

//---------------------
//Loop on var > chan > sample > syst
//Retrieve histograms, stack, compare, plot
	for(int ivar=0; ivar<total_var_list.size(); ivar++)
	{
		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
		c1->SetBottomMargin(0.3); //Leave space for ratio plot

		h_data = 0; stack = 0;
		vector<TH1F*> v_MC_histo; //Store separately the histos for each MC sample --> stack them after loops

		//TLegend* qw = new TLegend(.80,.60,.95,.90);
		TLegend* qw = new TLegend(.85,.7,0.965,.915);
		qw->SetShadowColor(0);
		qw->SetFillColor(0);
		qw->SetLineColor(0);

		//---------------------------
		//ERROR VECTORS INITIALIZATION
		//---------------------------
		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)

		//Only call this 'random' histogram here in order to get the binning used for the current variable --> can initialize the error vectors !
		//CHANGED
		if (postfit && !f->GetDirectory((total_var_list[ivar] + "_uuu_postfit").Data()) ) continue; //Check histo exists

		TString histo_name = "";
		if(postfit) histo_name =  total_var_list[ivar] + "_uuu_postfit/tZqmcNLO"; //For postfit file only ! //FIXME
		// if(postfit) histo_name =  total_var_list[ivar] + "_uuu_postfit/tZqhwpp"; //For postfit file only ! //FIXME
		else histo_name = total_var_list[ivar] + "_uuu__tZqmcNLO"; //For prefit file
		// else histo_name = total_var_list[ivar] + "_uuu__tZqhwpp"; //For prefit file //FIXME

		//NOTE -- How to use GetListOfKeys when histos are in a directory ?
		if(!postfit && !f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<__LINE__<<" : "<<histo_name<<" : not found ! Continue !"<<endl; continue;}

		h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
		if(!h_tmp) continue;

		int nofbins = h_tmp->GetNbinsX();
		for(int ibin=0; ibin<nofbins; ibin++)
		{
			v_eyl.push_back(0); v_eyh.push_back(0);
			v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
			v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
			v_y.push_back(0);
		}


		//---------------------------
		//RETRIEVE & SUM HISTOGRAMS, SUM ERRORS QUADRATICALLY
		//---------------------------

		vector<TString> MC_samples_legend; //List the MC samples which are actually used (to get correct legend)
		int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
		for(int ichan=0; ichan<thechannellist.size(); ichan++)
		{
			if(!allchannels && channel != thechannellist[ichan]) {continue;} //If plot single channel

			int nof_missing_samples = 0; //is needed to access the right bin of v_MC_histo
			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				// cout<<total_var_list[ivar]<<" / "<<thechannellist[ichan]<<" / "<<sample_list[isample]<<" / "<<endl;

				if(!fakes_from_data && sample_list[isample]=="Fakes" ) {continue;} //Fakes from MC only
				else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

				bool isData = false; if(sample_list[isample] == "Data") isData = true;

				TH1F* histo_nominal = 0; //Nominal histogram <-> syst == "" //Create it here because it's also used in the (syst != "") loop

				h_tmp = 0; //Temporary storage of histogram

				//CHANGED
				// histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample];
				if(postfit)
				{
					histo_name = total_var_list[ivar] + "_" + thechannellist[ichan] + "_postfit/";
					if(isData)
					{
						if(combine_naming_convention) histo_name+= "data_obs";
						else histo_name+= "DATA";
					}
					else if(sample_list[isample] == "Fakes")
					{
						TString fake_name = "";
						if(thechannellist[ichan]=="uuu") fake_name = "FakeMuMuMu";
						else if(thechannellist[ichan]=="uue") fake_name = "FakeMuMuEl";
						else if(thechannellist[ichan]=="eeu") fake_name = "FakeElElMu";
						else if(thechannellist[ichan]=="eee") fake_name = "FakeElElEl";
						histo_name+= fake_name;
					}
					else histo_name+= sample_list[isample];
				}
				else
				{
					histo_name = total_var_list[ivar] + "_" + thechannellist[ichan] + "__";
					if(isData)
					{
						if(combine_naming_convention) histo_name+= "data_obs";
						else histo_name+= "DATA";
					}
					else if(sample_list[isample] == "Fakes")
					{
						TString fake_name = "";
						if(thechannellist[ichan]=="uuu") fake_name = "FakeMuMuMu";
						else if(thechannellist[ichan]=="uue") fake_name = "FakeMuMuEl";
						else if(thechannellist[ichan]=="eeu") fake_name = "FakeElElMu";
						else if(thechannellist[ichan]=="eee") fake_name = "FakeElElEl";
						histo_name+= fake_name;
					}
					else histo_name+= sample_list[isample];
				}

				h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
				if(!h_tmp) continue;
				// cout<<"h_tmp = "<<h_tmp<<endl;

				if(!isData && (!allchannels || ichan==0)) MC_samples_legend.push_back(sample_list[isample]); //Fill vector containing existing MC samples names -- do it only once

				//Protection against TH1Fs with nAn/inf values (messes up plots in weird way, better to prevent it)
				bool containsNAN = false;
				for(int ibin=1; ibin<h_tmp->GetNbinsX()+1; ibin++)
				{
					double bin_tmp = (double) h_tmp->GetBinContent(ibin);
					if(std::isnan(bin_tmp) || std::isinf(bin_tmp)) {containsNAN = true; break;}
				}
				if(containsNAN) {cout<<"Histogram "<<histo_name.Data()<<" contains nAn/inf value ! -- Ignore"<<endl; continue;}

				if(isData) //DATA
				{
					if(niter_chan == 0) {h_data = (TH1F*) h_tmp->Clone();}
					else {h_data->Add(h_tmp);}
					continue;
				}
				else //MC
				{
					//Normally the data sample is included in first position of the list, so v_MC_histo has 'isample-1' contents
					if(isample-1-nof_missing_samples < 0)  {cout<<__LINE__<<BOLD(FRED(" : Try to access wrong address (need at least 2 samples)! Exit !"))<<endl; return 0;}
					// cout<<"isample-1-nof_missing_samples = "<<isample-1-nof_missing_samples<<endl;

					if(postfit && sample_list[isample].Contains("tZq")) {h_tmp->Scale(sig_strength);} //Need to rescale signal manually, not done by CombineHarvester !!

					//Use color vector filled in main()
					h_tmp->SetFillStyle(1001);
					h_tmp->SetFillColor(colorVector[isample-1]);
					h_tmp->SetLineColor(colorVector[isample-1]);

					if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
					else
					{
						if(isample-1-nof_missing_samples >= v_MC_histo.size()) {cout<<__LINE__<<BOLD(FRED(" : ERROR -- wrong number of samples !"))<<endl; continue;}
						else v_MC_histo[isample-1-nof_missing_samples]->Add(h_tmp);
					}

					histo_nominal = (TH1F*) h_tmp->Clone();

					for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
					{
						v_eyl[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.062, 2); //Lumi error = 6.2%
						v_eyh[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.062, 2);
						//MC Stat error
						v_eyl[ibin]+= pow(histo_nominal->GetBinError(ibin+1), 2);
						v_eyh[ibin]+= pow(histo_nominal->GetBinError(ibin+1), 2);

						v_y[ibin]+= histo_nominal->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)

						if(ibin > 0) {continue;} //cout only first bin
						//cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
					}
				}

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					if(isData || (sample_list[isample].Contains("Fakes") && !syst_list[isyst].Contains("Fakes")) ) {continue;}
					if(syst_list[isyst] == "") {continue;} //Already done

					if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}

					if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

					if(postfit) cout<<BOLD(FRED("POSTFIT PLOTS : NO SYST YET ! CHECK NAMING CONVENTIONS FIRST !"))<<endl;

					TH1F* histo_syst = 0; //Store the "systematic histograms"

					//CHANGED
					// histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample] + "_" + syst_list[isyst];

					TString histo_name_syst = histo_name;

					if(postfit || combine_naming_convention) histo_name_syst+= "__" + Combine_Naming_Convention(syst_list[isyst]);
					else histo_name_syst+= "__" + Theta_Naming_Convention(syst_list[isyst]);

					if(syst_list[isyst] == "" && !f->GetListOfKeys()->Contains(histo_name_syst.Data())) {cout<<histo_name_syst<<" : not found !"<<endl; continue;}
					else if(!f->GetListOfKeys()->Contains(histo_name_syst.Data()) ) {continue;} //No error messages if systematics histos not found

					histo_syst = (TH1F*) f->Get(histo_name_syst.Data())->Clone();

					//Add up here the different errors (quadratically), for each bin separately
					for(int ibin=0; ibin<nofbins; ibin++)
					{
						double tmp = 0;

						//For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding bin error
						//--------------------------
						tmp = histo_syst->GetBinContent(ibin+1) - histo_nominal->GetBinContent(ibin+1);
						if(tmp>0) {v_eyh[ibin]+= pow(tmp,2);}
						else if(tmp<0) {v_eyl[ibin]+= pow(tmp,2);}
						//--------------------------

						// if(ibin > 0) {continue;} //cout only first bin
						//cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;

					}
				} //end syst loop
			} //end sample loop

			niter_chan++;
		} //end channel loop


		//---------------------------
		//CREATE STACK (MC)
		//---------------------------
		TH1F* histo_total_MC = 0; //Sum of all MC samples

		//Stack all the MC nominal histograms (contained in v_MC_histo)
		for(int i=0; i<v_MC_histo.size(); i++)
		{
			if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
			else {stack->Add(v_MC_histo[i]);}

			//if(histo_total_MC == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
			if(i == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
			else {histo_total_MC->Add(v_MC_histo[i]);}
			//cout << "**********************  "<< histo_total_MC->Integral() << endl;

			//ADD LEGEND ENTRIES
			if(MC_samples_legend[i] != "ttH" && MC_samples_legend[i] != "ttZ" && MC_samples_legend[i] != "ttW" && !MC_samples_legend[i].Contains("tZq") ) {qw->AddEntry(v_MC_histo[i], MC_samples_legend[i].Data() , "f");}
			else if(MC_samples_legend[i].Contains("tZq") ) {qw->AddEntry(v_MC_histo[i], "tZq" , "f");}
			else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV" , "f");} //Single entry for ttZ+ttW+ttH
		}

		if(h_data != 0) {qw->AddEntry(h_data, "Data" , "ep");}
		else {cout<<__LINE__<<BOLD(FRED(" : h_data is null"))<<endl;}

		if(!stack) {cout<<__LINE__<<BOLD(FRED(" : stack is null"))<<endl;}

		//---------------------------
		//DRAW HISTOS
		//---------------------------

		c1->cd();

		//Set Yaxis maximum & min
		if(h_data != 0 && stack != 0)
		{
			if(h_data->GetMaximum() > stack->GetMaximum() ) {stack->SetMaximum(h_data->GetMaximum()+0.3*h_data->GetMaximum());}
			else stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum());
		}
		stack->SetMinimum(0);

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


		//---------------------------
		//DRAW SYST ERRORS ON PLOT
		//---------------------------

		//Need to take sqrt of total errors
		for(int ibin=0; ibin<nofbins; ibin++)
		{
			v_eyh[ibin] = pow(v_eyh[ibin], 0.5);
			v_eyl[ibin] = pow(v_eyl[ibin], 0.5);

			// if(ibin > 0) {continue;} //cout only first bin
			//cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
		}

		//Use pointers to vectors : need to give the adress of first element (all other elements can then be accessed iteratively)
		double* eyl = &v_eyl[0];
		double* eyh = &v_eyh[0];
		double* exl = &v_exl[0];
		double* exh = &v_exh[0];
		double* x = &v_x[0];
		double* y = &v_y[0];

		//Create TGraphAsymmErrors with the error vectors / (x,y) coordinates --> Can superimpose it on plot
		TGraphAsymmErrors* gr = 0;
		gr = new TGraphAsymmErrors(nofbins,x,y,exl,exh,eyl,eyh);
		gr->SetFillStyle(3005);
		gr->SetFillColor(1);
		gr->Draw("e2 same"); //Superimposes the systematics uncertainties on stack
		//cout << __LINE__ << endl;

		//-------------------
		//CAPTIONS
		//-------------------
		TLatex* latex = new TLatex();
		latex->SetNDC();
		latex->SetTextSize(0.04);
		latex->SetTextAlign(31);
		latex->DrawLatex(0.45, 0.95, "CMS Preliminary");

		TLatex* latex2 = new TLatex();
		latex2->SetNDC();
		latex2->SetTextSize(0.04);
		latex2->SetTextAlign(31);
		//------------------
		// float lumi = 12.9 * luminosity_rescale;
		float lumi = 35.68 * luminosity_rescale; //CHANGED
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
		//DRAW DATA/MC RATIO
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

			TH1::SetDefaultSumw2();

			TH1F * histo_ratio_data = (TH1F*) h_data->Clone();
			//cout << "data integral " << histo_ratio_data->Integral() << endl;
			//cout << "MC integral " << histo_total_MC->Integral() << endl;
			histo_ratio_data->Divide(histo_total_MC); //Ratio -- errors are recomputed

			for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
			{
				if( std::isnan(histo_ratio_data->GetBinContent(ibin)) || std::isinf(histo_ratio_data->GetBinContent(ibin)) ) {histo_ratio_data->SetBinContent(ibin, 1);}
			}


			histo_ratio_data->GetXaxis()->SetTitle(total_var_list[ivar].Data());

			histo_ratio_data->SetMinimum(0.0);
			histo_ratio_data->SetMaximum(2.0);
			histo_ratio_data->GetXaxis()->SetTitleOffset(1.2);
			histo_ratio_data->GetXaxis()->SetLabelSize(0.04);
			histo_ratio_data->GetYaxis()->SetLabelSize(0.03);
			histo_ratio_data->GetYaxis()->SetNdivisions(6);
			histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
			histo_ratio_data->Draw("E1X0"); //Draw ratio points

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

		TString region_name = "";
		if(isWZ) region_name = "WZ_CR";
		else if(isttZ) region_name = "ttZ_CR";
		else region_name = "tZq_SR";

		mkdir("plots/inputVars",0777);
		mkdir( ("plots/inputVars/"+region_name).Data(), 0777);
		if(postfit)
		{
			mkdir( ("plots/inputVars/"+region_name+"/postfit").Data(), 0777);
			mkdir( ("plots/inputVars/"+region_name+"/postfit/allchans").Data(), 0777);
		}
		else
		{
			mkdir(("plots/inputVars/"+region_name+"/prefit").Data(), 0777);
			mkdir(("plots/inputVars/"+region_name+"/prefit/allchans").Data(), 0777);
		}

		//Image name
		TString outputname = "plots/inputVars/"+ region_name + "/";
		if(postfit) outputname+= "postfit/";
		else outputname+= "prefit/";
		if(channel == "" || allchannels) {outputname+= "allchans/";}
		outputname+= total_var_list[ivar]+"_"+channel;
		if(channel == "" || allchannels) {outputname+= "all";}
		if(postfit) outputname+= "_postfit";
		outputname+= this->format;

		// TString outputname = "plots/"+total_var_list[ivar]+"_"+channel+".png";
		// if(channel == "" || allchannels) {outputname = "plots/"+total_var_list[ivar]+"_all.png";}

		// cout << __LINE__ << endl;
		if(c1!= 0) {c1->SaveAs(outputname.Data() );}
		// cout << __LINE__ << endl;

		delete c1; //Must free dinamically-allocated memory
	} //end var loop

	delete f;

	return 0;
}











//-----------------------------------------------------------------------------------------
// ########  ########     ###    ##      ##    ########  ########  ######## ######## #### ########
// ##     ## ##     ##   ## ##   ##  ##  ##    ##     ## ##     ## ##       ##        ##     ##
// ##     ## ##     ##  ##   ##  ##  ##  ##    ##     ## ##     ## ##       ##        ##     ##
// ##     ## ########  ##     ## ##  ##  ##    ########  ########  ######   ######    ##     ##
// ##     ## ##   ##   ######### ##  ##  ##    ##        ##   ##   ##       ##        ##     ##
// ##     ## ##    ##  ##     ## ##  ##  ##    ##        ##    ##  ##       ##        ##     ##
// ########  ##     ## ##     ##  ###  ###     ##        ##     ## ######## ##       ####    ##

// ######## ######## ##     ## ########  ##          ###    ######## ########  ######
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
//    ##    ##       ##     ## ##        ##       #########    ##    ##             ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Plot stacked MC templates .vs. Data PREFIT (from Combine !input! file) NB : systematics are not accounted for
 * @param  channel       Channel name
 * @param  template_name Template name : BDT / BDTttZ / mTW
 * @param  allchannels   If true, sum all channels
 */
int theMVAtool::Plot_Prefit_Templates(TString channel, TString template_name, bool allchannels)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	// TString input_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root";
	TString input_name = "outputs/Combine_Input.root"; //FAKES RE-SCALED
	TFile* file_input = 0;
	file_input = TFile::Open( input_name.Data() );
	if(file_input == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}

	mkdir("plots",0777);
	TH1::SetDefaultSumw2();

	vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
	thechannellist.push_back("uuu");
	thechannellist.push_back("uue");
	thechannellist.push_back("eeu");
	thechannellist.push_back("eee");

	TH1F *h_tmp = 0, *h_sum_data = 0;
	vector<TH1F*> v_MC_histo;

	//TLegend* qw = new TLegend(.80,.60,.95,.90);
	TLegend* qw = new TLegend(.85,.7,0.965,.915);
	qw->SetShadowColor(0);
	qw->SetFillColor(0);
	qw->SetLineColor(0);

	int niter_chan = 0;
	for(int ichan=0; ichan<thechannellist.size(); ichan++)
	{
		if(!allchannels && channel != thechannellist[ichan]) {continue;}

	//--- All MC samples but fakes
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			// if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("Fakes")) {continue;} //See below for fakes
			if(sample_list[isample].Contains("Data") ) {continue;}

			h_tmp = 0;
			TString histo_name = template_name + "_" + thechannellist[ichan] + "__" + sample_list[isample];
			if(!file_input->GetListOfKeys()->Contains(histo_name.Data()) && !sample_list[isample].Contains("Fakes") ) {cout<<histo_name<<" : not found"<<endl; continue;}
			else if(!sample_list[isample].Contains("Fakes"))
			{
				h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();

				//Use color vector filled in main() (use -1 because first sample should be data)
				h_tmp->SetFillStyle(1001);
				h_tmp->SetFillColor(colorVector[isample-1]);
				h_tmp->SetLineColor(colorVector[isample-1]);

				if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
				else {v_MC_histo[isample-1]->Add(h_tmp);}
			}
			else //fakes
			{
				TString template_fake_name = "";

				if(   thechannellist[ichan] == "uuu" )      {template_fake_name = "FakeMuMuMu";}
				else if(   thechannellist[ichan] == "uue" ) {template_fake_name = "FakeMuMuEl";}
				else if(   thechannellist[ichan] == "eeu" ) {template_fake_name = "FakeElElMu";}
				else if(   thechannellist[ichan] == "eee" ) {template_fake_name = "FakeElElEl";}

				histo_name = template_name + "_" + thechannellist[ichan] + "__" + template_fake_name;
				if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
				else
				{
					h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();

					h_tmp->SetFillStyle(1001);
					h_tmp->SetFillColor(colorVector[isample-1]);
					h_tmp->SetLineColor(colorVector[isample-1]);

					if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
					else {v_MC_histo[isample-1]->Add(h_tmp);}
				}
			}
		} //end sample loop

	//--- DATA
		h_tmp = 0;
		TString histo_name;
		if(combine_naming_convention) histo_name = template_name + "_" + thechannellist[ichan] + "__data_obs"; //Combine
		else histo_name = template_name + "_" + thechannellist[ichan] + "__DATA"; //theta

		if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
		else
		{
			h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
			if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
			else {h_sum_data->Add(h_tmp);}
		}

		niter_chan++; //incremented only if allchannels = true
	} //end channel loop

	if(h_sum_data == 0) {cout<<endl<<BOLD(FRED("--- Empty data histogram ! Exit !"))<<endl<<endl; return 0;}

	//Make sure there are no negative bins
	for(int ibin = 1; ibin<h_sum_data->GetNbinsX()+1; ibin++)
	{
		for(int k=0; k<v_MC_histo.size(); k++)
		{
			if(v_MC_histo[k]->GetBinContent(ibin) < 0) {v_MC_histo[k]->SetBinContent(ibin, 0);}
		}
		if(h_sum_data->GetBinContent(ibin) < 0) {h_sum_data->SetBinContent(ibin, 0);}
	}

	//Stack all the MC nominal histograms (contained in v_MC_histo)
	THStack* stack_MC = 0;
	TH1F* histo_total_MC = 0; //Sum of all MC samples

	for(int i=0; i<v_MC_histo.size(); i++)
	{
		if(i == 0) {stack_MC = new THStack; stack_MC->Add(v_MC_histo[i]); histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
		else {stack_MC->Add(v_MC_histo[i]); histo_total_MC->Add(v_MC_histo[i]);}

		//ADD LEGEND ENTRIES
		if(!sample_list[i+1].Contains("ttH") && !sample_list[i+1].Contains("ttZ") && !sample_list[i+1].Contains("ttW") && !sample_list[i+1].Contains("tZq") ) {qw->AddEntry(v_MC_histo[i], sample_list[i+1].Data() , "f");}
		else if(sample_list[i+1].Contains("tZq") ) {qw->AddEntry(v_MC_histo[i], "tZq" , "f");}
		else if(sample_list[i+1] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV" , "f");} //Single entry for ttZ+ttW+ttH
	}

	qw->AddEntry(h_sum_data, "Data" , "lep");

	//Set Yaxis maximum & minimum
	if(h_sum_data->GetMaximum() > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
	else stack_MC->SetMaximum(stack_MC->GetMaximum()+0.3*stack_MC->GetMaximum());
	stack_MC->SetMinimum(0);


	//Canvas definition
	Load_Canvas_Style();
	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
	c1->SetBottomMargin(0.3);

	//Draw stack & data
	stack_MC->Draw("hist");
	h_sum_data->SetMarkerStyle(20);
	h_sum_data->Draw("e0psame");

	TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
	canvas_2->SetTopMargin(0.7);
	canvas_2->SetFillColor(0);
	canvas_2->SetFillStyle(0);
	canvas_2->SetGridy(1);
	canvas_2->Draw();
	canvas_2->cd(0);

	TH1F * histo_ratio_data = (TH1F*) h_sum_data->Clone();
	histo_ratio_data->Divide(histo_total_MC); //Ratio

	histo_ratio_data->SetMinimum(0.0);
	histo_ratio_data->SetMaximum(2.0);
	histo_ratio_data->GetXaxis()->SetTitle(template_name.Data());
	histo_ratio_data->GetXaxis()->SetTitleOffset(1.4);
	histo_ratio_data->GetXaxis()->SetLabelSize(0.04);

	histo_ratio_data->GetYaxis()->SetTitle("Data/MC");
	histo_ratio_data->GetYaxis()->SetTitleOffset(1.4);
	histo_ratio_data->GetYaxis()->SetLabelSize(0.04);
	histo_ratio_data->GetYaxis()->SetNdivisions(6);
	histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
	histo_ratio_data->Draw("E1X0"); //Draw ratio points



	// stack_MC->GetXaxis()->SetTitle(template_name.Data());
	stack_MC->GetXaxis()->SetLabelSize(0.0);
	stack_MC->GetYaxis()->SetTitle("Events");
	c1->Modified();

	//-------------------
	//CAPTIONS
	//-------------------
	TLatex* latex = new TLatex();
	latex->SetNDC();
	latex->SetTextSize(0.04);
	latex->SetTextAlign(31);
	latex->DrawLatex(0.45, 0.95, "CMS Preliminary");

	TLatex* latex2 = new TLatex();
	latex2->SetNDC();
	latex2->SetTextSize(0.04);
	latex2->SetTextAlign(31);
	//------------------
	// float lumi = 12.9 * luminosity_rescale;
	float lumi = 35.68 * luminosity_rescale; //CHANGED
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

	if(isWZ) {info_data+= " - CR WZ";}
	else if(isttZ) {info_data+= " - CR ttZ";}
	else if(!isWZ && !isttZ) {info_data+= " - SR tZq";}

	TLatex* text2 = new TLatex(0, 0, info_data);
	text2->SetNDC();
	text2->SetTextAlign(13);
	text2->SetX(0.19);
	text2->SetY(0.9);
	text2->SetTextFont(42);
	text2->SetTextSize(0.04);
	//text2->SetTextSizePixels(24);// dflt=28
	text2->Draw("same");
	//------------------


	qw->Draw("same");

	mkdir("plots",0777); //Create directory if inexistant

	//Output
	TString output_plot_name = "plots/" + template_name +"_template_" + channel+ this->filename_suffix + this->format;
	if(channel == "" || allchannels) {output_plot_name = "plots/" + template_name +"_template_all" + this->filename_suffix + ".png";}

	c1->SaveAs(output_plot_name.Data());

	delete file_input;

	delete c1;
	delete qw;
	delete stack_MC;

	return 0;
}







//-------------------------------------------------
// ########  ########     ###    ##      ##    ########   #######   ######  ######## ######## #### ########
// ##     ## ##     ##   ## ##   ##  ##  ##    ##     ## ##     ## ##    ##    ##    ##        ##     ##
// ##     ## ##     ##  ##   ##  ##  ##  ##    ##     ## ##     ## ##          ##    ##        ##     ##
// ##     ## ########  ##     ## ##  ##  ##    ########  ##     ##  ######     ##    ######    ##     ##
// ##     ## ##   ##   ######### ##  ##  ##    ##        ##     ##       ##    ##    ##        ##     ##
// ##     ## ##    ##  ##     ## ##  ##  ##    ##        ##     ## ##    ##    ##    ##        ##     ##
// ########  ##     ## ##     ##  ###  ###     ##         #######   ######     ##    ##       ####    ##

// ######## ######## ##     ## ########  ##          ###    ######## ########  ######
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
//    ##    ##       ##     ## ##        ##       #########    ##    ##             ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//-------------------------------------------------




/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Plot stacked MC templates VS data POSTFIT (from Combine !output! file obtained from MaxLikelihood Fit) - NB : systematics are not accounted for
 * @param  channel       Channel name
 * @param  template_name Template name :  BDT / BDTttZ / mTW
 * @param  allchannels   If true, sum all channels
 */
int theMVAtool::Plot_Postfit_Templates(TString channel, TString template_name, bool allchannels)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots from Combine Output ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	//In mlfit.root, data is given as a TGraph. Instead, we take it directly from the Template file given to Combine (un-changed!)
	TString input_name = "outputs/Combine_Input.root"; //FAKES RE-SCALED
	TFile* file_data = 0;
	file_data = TFile::Open( input_name.Data() );
	if(file_data == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}

	input_name = "outputs/mlfit.root";
	TFile* file_input = 0;
	file_input = TFile::Open( input_name.Data() );
	if(file_input == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}


	mkdir("plots",0777);
	TH1::SetDefaultSumw2();

	vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
	thechannellist.push_back("uuu");
	thechannellist.push_back("uue");
	thechannellist.push_back("eeu");
	thechannellist.push_back("eee");

	TH1F *h_tmp = 0, *h_sum_data = 0;
	vector<TH1F*> v_MC_histo;

	TLegend* qw = new TLegend(.85,.7,0.965,.915);
	qw->SetShadowColor(0);
	qw->SetFillColor(0);
	qw->SetLineColor(0);

	int niter_chan = 0;
	for(int ichan=0; ichan<thechannellist.size(); ichan++)
	{
		if(!allchannels && channel != thechannellist[ichan]) {continue;}

	//--- All MC samples but fakes
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			bool isData = false;
			if(sample_list[isample].Contains("Data") ) {isData = true;}

			//--- DATA -- different file
			if(isData)
			{
				h_tmp = 0;
				TString histo_name;
				if(combine_naming_convention) histo_name = template_name + "_" + thechannellist[ichan] + "__data_obs"; //Combine
				else histo_name = template_name + "_" + thechannellist[ichan] + "__DATA"; //Theta

				if(!file_data->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
				else
				{
					h_tmp = (TH1F*) file_data->Get(histo_name.Data())->Clone();
					if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
					else {h_sum_data->Add(h_tmp);}
				}
			}

			else //MC
			{
				TString dir_name = "shapes_fit_s/" + template_name + "_" + thechannellist[ichan] + "/";

				h_tmp = 0;
				TString histo_name = sample_list[isample];
				if(!sample_list[isample].Contains("Fakes"))
				{
					h_tmp = (TH1F*) file_input->Get((dir_name+histo_name).Data())->Clone();

					//Use color vector filled in main() (use -1 because first sample should be data)
					h_tmp->SetFillStyle(1001);
					h_tmp->SetFillColor(colorVector[isample-1]);
					h_tmp->SetLineColor(colorVector[isample-1]);

					if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
					else {v_MC_histo[isample-1]->Add(h_tmp);}
				}
				else //fakes
				{
					TString template_fake_name = "";

					if(   thechannellist[ichan] == "uuu" )      {template_fake_name = "FakeMuMuMu";}
					else if(   thechannellist[ichan] == "uue" ) {template_fake_name = "FakeMuMuEl";}
					else if(   thechannellist[ichan] == "eeu" ) {template_fake_name = "FakeElElMu";}
					else if(   thechannellist[ichan] == "eee" ) {template_fake_name = "FakeElElEl";}

					TString histo_name = template_fake_name;

					h_tmp = (TH1F*) file_input->Get((dir_name+histo_name).Data())->Clone();

					h_tmp->SetFillStyle(1001);
					h_tmp->SetFillColor(colorVector[isample-1]);
					h_tmp->SetLineColor(colorVector[isample-1]);

					if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
					else {v_MC_histo[isample-1]->Add(h_tmp);}
				}
			}

		} //end sample loop

		niter_chan++; //incremented only if allchannels = true
	} //end channel loop


	if(h_sum_data == 0) {cout<<endl<<BOLD(FRED("--- Empty data histogram ! Exit !"))<<endl<<endl; return 0;}

	//Make sure there are no negative bins
	for(int ibin = 1; ibin<h_sum_data->GetNbinsX()+1; ibin++)
	{
		for(int k=0; k<v_MC_histo.size(); k++)
		{
			if(v_MC_histo[k]->GetBinContent(ibin) < 0) {v_MC_histo[k]->SetBinContent(ibin, 0);}
		}
		if(h_sum_data->GetBinContent(ibin) < 0) {h_sum_data->SetBinContent(ibin, 0);}
	}


	//Stack all the MC nominal histograms (contained in v_MC_histo)
	THStack* stack_MC = 0;
	TH1F* histo_total_MC = 0; //Sum of all MC samples

	for(int i=0; i<v_MC_histo.size(); i++)
	{
		if(i == 0) {stack_MC = new THStack; stack_MC->Add(v_MC_histo[i]); histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
		else {stack_MC->Add(v_MC_histo[i]); histo_total_MC->Add(v_MC_histo[i]);}

		//ADD LEGEND ENTRIES
		if(!sample_list[i+1].Contains("ttH") && !sample_list[i+1].Contains("ttZ") && !sample_list[i+1].Contains("ttW") && !sample_list[i+1].Contains("tZq") ) {qw->AddEntry(v_MC_histo[i], sample_list[i+1].Data() , "f");}
		else if(sample_list[i+1].Contains("tZq") ) {qw->AddEntry(v_MC_histo[i], "tZq" , "f");}
		else if(sample_list[i+1] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV" , "f");} //Single entry for ttZ+ttW+ttH
	}


	//Set Yaxis maximum & minimum
	if(h_sum_data->GetMaximum() > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
	else stack_MC->SetMaximum(stack_MC->GetMaximum()+0.3*stack_MC->GetMaximum());
	stack_MC->SetMinimum(0);


	//Canvas definition
	Load_Canvas_Style();
	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
	c1->SetBottomMargin(0.3);

	//Draw stack & data
	stack_MC->Draw("hist");

	//Need to transform data histogram so that it's x-axis complies with combine's one
	double xmax_stack = stack_MC->GetXaxis()->GetXmax();
	double xmin_stack = stack_MC->GetXaxis()->GetXmin();
	double xmax_data = h_sum_data->GetXaxis()->GetXmax();
	double xmin_data = h_sum_data->GetXaxis()->GetXmin();

	// cout<<"BDT_uuu[10] = "


	// cout<<"xmin_stack = "<<xmin_stack<<" / xmax_stack = "<<xmax_stack<<endl;
	// cout<<"xmin_data = "<<xmin_data<<" / xmax_data = "<<xmax_data<<endl;
   	TH1F *h_data_new = new TH1F("","",this->nbin,xmin_stack,xmax_stack);
	for(int ibin = 1; ibin<h_sum_data->GetNbinsX()+1; ibin++)
	{
		double x =  h_sum_data->GetXaxis()->GetBinCenter(ibin);
		double y = 	h_sum_data->GetBinContent(ibin);
		double y_err =	h_sum_data->GetBinError(ibin);
		double xnew = (x-xmin_data) * xmax_stack / (xmax_data - xmin_data); //Transformation
		h_data_new->Fill(xnew,y); h_data_new->SetBinError(ibin, y_err); //CHANGED

		// cout<<"i = "<<ibin<<" x = "<<x<<" xnew = "<<xnew<<" y = "<<y<<endl;
	}

	qw->AddEntry(h_data_new, "Data" , "lep");


	// h_sum_data->SetMarkerStyle(20);
	// h_sum_data->Draw("e0psame");
	h_data_new->SetMarkerStyle(20);
	h_data_new->Draw("e0psame");

	TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
	canvas_2->SetTopMargin(0.7);
	canvas_2->SetFillColor(0);
	canvas_2->SetFillStyle(0);
	canvas_2->SetGridy(1);
	canvas_2->Draw();
	canvas_2->cd(0);

	TH1F * histo_ratio_data = (TH1F*) h_data_new->Clone();
	histo_ratio_data->Divide(histo_total_MC); //Ratio

	histo_ratio_data->SetMinimum(0.0);
	histo_ratio_data->SetMaximum(2.0);
	// histo_ratio_data->GetXaxis()->SetTitle(template_name.Data());
	// histo_ratio_data->GetXaxis()->SetTitleOffset(1.4);
	// histo_ratio_data->GetXaxis()->SetLabelSize(0.04);

	histo_ratio_data->GetYaxis()->SetTitle("Data/MC");
	histo_ratio_data->GetYaxis()->SetTitleOffset(1.4);
	histo_ratio_data->GetYaxis()->SetLabelSize(0.04);
	histo_ratio_data->GetYaxis()->SetNdivisions(6);
	histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
	histo_ratio_data->GetXaxis()->SetTickLength(0);
	histo_ratio_data->GetXaxis()->SetLabelOffset(999);

	histo_ratio_data->Draw("E1X0"); //Draw ratio points

	//SINCE WE MODIFIED THE ORIGINAL AXIS OF THE HISTOS, NEED TO DRAW AN INDEPENDANT AXIS REPRESENTING THE ORIGINAL x VALUES
	TGaxis *axis = new TGaxis(histo_ratio_data->GetXaxis()->GetXmin(),histo_ratio_data->GetMinimum(),histo_ratio_data->GetXaxis()->GetXmax(),histo_ratio_data->GetMinimum(),xmin_data,xmax_data, 510, "+"); // + : tick marks on positive side ; = : label on same side as marks
   axis->SetLineColor(1);
   axis->SetTitle(template_name.Data());
   axis->SetTitleSize(0.04);
   axis->SetTitleOffset(1.4);
   axis->SetTickLength(0.03);
   axis->SetLabelSize(0.03);
   // axis->SetLabelOffset(-0.01);
   axis->Draw("same");

	// stack_MC->GetXaxis()->SetTitle(template_name.Data());
	stack_MC->GetXaxis()->SetLabelSize(0.0);
	stack_MC->GetYaxis()->SetTitle("Events");
	c1->Modified();

	//-------------------
	//CAPTIONS
	//-------------------
	TLatex* latex = new TLatex();
	latex->SetNDC();
	latex->SetTextSize(0.04);
	latex->SetTextAlign(31);
	latex->DrawLatex(0.45, 0.95, "CMS Preliminary");

	TLatex* latex2 = new TLatex();
	latex2->SetNDC();
	latex2->SetTextSize(0.04);
	latex2->SetTextAlign(31);
	//------------------
	// float lumi = 12.9 * luminosity_rescale;
	float lumi = 35.68 * luminosity_rescale; //CHANGED
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

	if(isWZ) {info_data+= " - CR WZ";}
	else if(isttZ) {info_data+= " - CR ttZ";}
	else if(!isWZ && !isttZ) {info_data+= " - SR tZq";}

	TLatex* text2 = new TLatex(0, 0, info_data);
	text2->SetNDC();
	text2->SetTextAlign(13);
	text2->SetX(0.19);
	text2->SetY(0.9);
	text2->SetTextFont(42);
	text2->SetTextSize(0.04);
	//text2->SetTextSizePixels(24);// dflt=28
	text2->Draw("same");
	//------------------


	qw->Draw("same");

	mkdir("plots",0777); //Create directory if inexistant

	//Output
	TString output_plot_name = "plots/postfit_" + template_name +"_template_" + channel+ this->filename_suffix + this->format;
	if(channel == "" || allchannels) {output_plot_name = "plots/postfit_" + template_name +"_template_all" + this->filename_suffix + ".png";}

	c1->SaveAs(output_plot_name.Data());

	delete file_input;
	delete file_data;
	delete c1; delete qw;
	delete stack_MC;

	return 0;
}
















//-----------------------------------------------------------------------------------------
// ######## #### ########    ########    ###    ##    ## ########    ######## ######## ##     ## ########  ##
// ##        ##     ##       ##         ## ##   ##   ##  ##             ##    ##       ###   ### ##     ## ##
// ##        ##     ##       ##        ##   ##  ##  ##   ##             ##    ##       #### #### ##     ## ##
// ######    ##     ##       ######   ##     ## #####    ######         ##    ######   ## ### ## ########  ##
// ##        ##     ##       ##       ######### ##  ##   ##             ##    ##       ##     ## ##        ##
// ##        ##     ##       ##       ##     ## ##   ##  ##             ##    ##       ##     ## ##        ##       ###
// ##       ####    ##       ##       ##     ## ##    ## ########       ##    ######## ##     ## ##        ######## ###
//-----------------------------------------------------------------------------------------


//NB : make sure there is enough stat. to perform a relevant fit --> check manually the output file to check if the fit is correct
/**
 * //Fit the fake templates with a landau ((or gaussian)) distribution (stored in new file)
 //-> Then can generate fake templates based on fit with function Create_Fake_Templates_From_Fit
 * @param  function      Function type : Landau or Gaus
 * @param  template_name Template name : BDT / BDTttZ / mTW
 */
int theMVAtool::Fit_Fake_Templates(TString function, TString template_name="BDT")
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Fit Fake Templates ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	cout<<FYEL("--- Using "<<template_name<<" template ---")<<endl<<endl;
	if(function != "landau" && function != "gaus") {cout<<"Wrong function name ; exit !"; return 0;}

	TString input_file_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root"; //Input = original template
	TString output_file_name = "outputs/Fit_Fake_Template" + template_name + this->filename_suffix + ".root";
	TFile* f_input = TFile::Open( input_file_name ); TFile* f_output = TFile::Open( output_file_name, "RECREATE" );
	TH1::SetDefaultSumw2();

	vector<TString> v_syst;
	v_syst.push_back("");
	v_syst.push_back("__Fakes__plus");
	v_syst.push_back("__Fakes__minus");

	for(int isyst=0; isyst<v_syst.size(); isyst++)
	{
		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			TString fake_channel_suffix = "";

			if(channel_list[ichan] == "uuu") {fake_channel_suffix = "FakeMuMuMu";}
			else if(channel_list[ichan] == "eee") {fake_channel_suffix = "FakeElElEl";}
			else if(channel_list[ichan] == "eeu") {fake_channel_suffix = "FakeElElMu";}
			else if(channel_list[ichan] == "uue") {fake_channel_suffix = "FakeMuMuEl";}

			TString histo_name = template_name + "_" + channel_list[ichan] + "__" + fake_channel_suffix + v_syst[isyst];
			if(!f_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}

			//Retrieve template
			TH1F* h = (TH1F*) f_input->Get(histo_name.Data());

			//Fit the template
			h->Fit(function.Data(), "Q"); //Fit histo with chosen function -- Quiet mode : "Q" option

			//Write in file
			f_output->cd();
			TString output_histo_name = template_name + "_" + channel_list[ichan] + "_" + function + "__" + fake_channel_suffix + v_syst[isyst];
			h->Write(output_histo_name.Data());
		}
	}

	delete f_output;
	delete f_input;

	return 0;
}

/**
 * Use fit from Fit_Fake_Templates to generate fake templates with larger stat. (stored  in new file)
 * @param  function      Function type : landau or gaus
 * @param  template_name Template name : BDT / BDTttZ / mTW
 */
int theMVAtool::Create_Fake_Templates_From_Fit(TString function, TString template_name="BDT")
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Create Fake Templates From Fit ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	cout<<FYEL("--- Using "<<template_name<<" template ---")<<endl<<endl;
	if(function != "landau" && function != "gaus") {cout<<"Wrong function name ; exit !"; return 0;}

	TString input_file_name = "outputs/Fit_Fake_Template" + template_name + this->filename_suffix + ".root";
	TString output_file_name = "outputs/New_Fake_Template" + template_name + this->filename_suffix + ".root"; //Write fit templates in separate file
	//TString output_file_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root"; //Write fit templates in Reader file
	TFile* f_input = TFile::Open( input_file_name ); TFile* f_output = TFile::Open( output_file_name, "RECREATE" );

	cout<<"Fit range : [-1;1]"<<endl;

	TH1::SetDefaultSumw2();

	vector<TString> v_syst;
	v_syst.push_back("");
	v_syst.push_back("__Fakes__plus");
	v_syst.push_back("__Fakes__minus");

	for(int isyst=0; isyst<v_syst.size(); isyst++)
	{
		cout<<"--- Syst = "<<v_syst[isyst]<<endl;
		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			TString fake_channel_suffix = "";

			if(channel_list[ichan] == "uuu") {fake_channel_suffix = "FakeMuMuMu";}
			else if(channel_list[ichan] == "eee") {fake_channel_suffix = "FakeElElEl";}
			else if(channel_list[ichan] == "eeu") {fake_channel_suffix = "FakeElElMu";}
			else if(channel_list[ichan] == "uue") {fake_channel_suffix = "FakeMuMuEl";}

			TString histo_name = "BDT_" + channel_list[ichan] + "_" + function + "__" + fake_channel_suffix + v_syst[isyst];
			if(!f_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}

			//Retrieve template
			TH1F* h_tmp = (TH1F*) f_input->Get(histo_name.Data());

			//Retrieve fit
			TF1* fit = h_tmp->GetFunction(function); //Nominal

			//--- If want to use the fit error for shifted templates
			/*TF1* fit_err_up = h_tmp->GetFunction(function); //shifted
			TF1* fit_err_down = h_tmp->GetFunction(function); //shifted
			For each param, shift the 'shifted fits' by the parameter's error
			for(int i=0; i<fit->GetNpar(); i++)
			{
				fit_err_up->SetParameter(i, fit->GetParameter(i) + fit->GetParError(i));
				fit_err_down->SetParameter(i, fit->GetParameter(i) - fit->GetParError(i));
			}*/

			cout<<"Fit "<<channel_list[ichan]<<" : Mean = "<<setprecision(3)<<fixed<<fit->GetParameter("Mean")<<"      Mean Error = "<<setprecision(3)<<fixed<<fit->GetParError(1)<<"   Error/mean = "<<setprecision(3)<<fixed<<abs(fit->GetParError(1)/fit->GetParameter("Mean"))<<endl;

			//New template histograms -- NB : binning only suited for BDT templates!
			TH1F* h_landau = new TH1F("", "", nbin, -1, 1);
			//TH1F* h_landau_plus = new TH1F("", "", nbin, -1, 1);
			//TH1F* h_landau_minus = new TH1F("", "", nbin, -1, 1);

			//Fill histos with 'nominal' and 'shifted' fits
			for(int i=0; i<pow(10,6); i++)
			{
				h_landau->Fill(fit->GetRandom());
				//h_landau_plus->Fill(fit_err_up->GetRandom());
				//h_landau_minus->Fill(fit_err_down->GetRandom());
			}

			//Renormalisation to original template integral
			histo_name = "BDT_" + channel_list[ichan] + "__" + fake_channel_suffix + v_syst[isyst];
			TString reader_file_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root";
			TFile* f_reader = TFile::Open( reader_file_name );
			if(!f_reader->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
			double template_integral = (double) ( (TH1F*) f_reader->Get(histo_name.Data()) )->Integral();
			h_landau->Scale(template_integral/h_landau->Integral());

			//Write in file
			f_output->cd();
			TString output_histo_name = template_name + "_" + function + "__" + channel_list[ichan] + "__" + fake_channel_suffix + v_syst[isyst];
			h_landau->Write(output_histo_name.Data());
			/*TString output_histo_name_plus = template_name + "_" + channel_list[ichan] + "_" + function + "__" + fake_channel_suffix + "__plus";
			h_landau_plus->Write(output_histo_name_plus.Data());
			TString output_histo_name_minus = template_name + "_" + channel_list[ichan] + "_" + function + "__" + fake_channel_suffix + "__minus";
			h_landau_minus->Write(output_histo_name_minus.Data());*/

			h_landau->Delete(); //h_landau_plus->Delete(); h_landau_minus->Delete();
		}
	}

	delete f_output;
	delete f_input;

	return 0;
}











//-----------------------------------------------------------------------------------------
//  ######   #######  ##    ## ##     ## ######## ########  ########    ######## ##     ## ######## ########    ###
// ##    ## ##     ## ###   ## ##     ## ##       ##     ##    ##          ##    ##     ## ##          ##      ## ##
// ##       ##     ## ####  ## ##     ## ##       ##     ##    ##          ##    ##     ## ##          ##     ##   ##
// ##       ##     ## ## ## ## ##     ## ######   ########     ##          ##    ######### ######      ##    ##     ##
// ##       ##     ## ##  ####  ##   ##  ##       ##   ##      ##          ##    ##     ## ##          ##    #########
// ##    ## ##     ## ##   ###   ## ##   ##       ##    ##     ##          ##    ##     ## ##          ##    ##     ##
//  ######   #######  ##    ##    ###    ######## ##     ##    ##          ##    ##     ## ########    ##    ##     ##
//-----------------------------------------------------------------------------------------

/**
 * [Takes a Reader file from dir. 'outputs/'(containing templates with Combine namings) as input, and creates a file with the same templates but Theta namings]
 */
void theMVAtool::Convert_Templates_Theta()
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Convert Templates for THETA ---")<<endl;
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;

	TString template_name;
	if(!isWZ && !isttZ) {template_name = "BDT";}
	else if(isttZ) {template_name = "BDTttZ";}
	else if(isWZ) {template_name = "mTW";}
	else {cout<<"Error !"<<endl; return;}


	TFile* f_input = 0;
	TString input_file_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root"; //Input = original template (Combine namings)
	f_input = TFile::Open( input_file_name ); if(!f_input) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}
	TFile* f_output = 0;
	TString output_file_name = "outputs/Reader_" + template_name + this->filename_suffix + "_THETA.root"; //Output = templates with THETA namings
	f_output = TFile::Open( output_file_name, "RECREATE" );


	TH1F* h_tmp = 0;

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			for(int isyst=0; isyst<syst_list.size(); isyst++)
			{
				h_tmp = 0;

				if(sample_list[isample].Contains("Data") && syst_list[isyst] != "" ) {continue;}
				if(sample_list[isample].Contains("Fakes") && syst_list[isyst] != "" && !syst_list[isyst].Contains("Fake") ) {continue;}
				if(syst_list[isyst].Contains("Fakes") && sample_list[isample] != "Fakes") {continue;}
				if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf")) ) {continue;}
				if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

				TString histo_name = template_name + "_" + channel_list[ichan] + "__" + sample_list[isample];
				if(sample_list[isample].Contains("Data") )
				{
					histo_name = template_name + "_" + channel_list[ichan] + "__data_obs";
				}
				else if(sample_list[isample].Contains("Fakes") )
				{
					if(channel_list[ichan] == "uuu") {histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeMuMuMu";}
					else if(channel_list[ichan] == "uue") {histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeMuMuEl";}
					else if(channel_list[ichan] == "eeu") {histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeElElMu";}
					else if(channel_list[ichan] == "eee") {histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeElElEl";}
				}

				if(syst_list[isyst] != "") {histo_name+= "__" + Combine_Naming_Convention(syst_list[isyst]);}
				if(!f_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}

				h_tmp = (TH1F*) f_input->Get(histo_name.Data());

				TString output_histo_name = template_name + "_" + channel_list[ichan] + "__" + sample_list[isample];
				if(sample_list[isample].Contains("Data") ) {output_histo_name = template_name + "_" + channel_list[ichan] + "__DATA";}
				else if(sample_list[isample].Contains("Fakes") )
				{
					if(channel_list[ichan] == "uuu") {output_histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeMuMuMu";}
					else if(channel_list[ichan] == "uue") {output_histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeMuMuEl";}
					else if(channel_list[ichan] == "eeu") {output_histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeElElMu";}
					else if(channel_list[ichan] == "eee") {output_histo_name = template_name + "_" + channel_list[ichan] + "__" + "FakeElElEl";}
				}

				if(syst_list[isyst] != "") output_histo_name+= "__" + Theta_Naming_Convention(syst_list[isyst]);

				f_output->cd();
				h_tmp->Write(output_histo_name.Data());
			}
		}
	}

	delete h_tmp; delete f_input; delete f_output;

	return;
}

































// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######

// ######## ########  ######  ######## #### ##    ##  ######
//    ##    ##       ##    ##    ##     ##  ###   ## ##    ##
//    ##    ##       ##          ##     ##  ####  ## ##
//    ##    ######    ######     ##     ##  ## ## ## ##   ####
//    ##    ##             ##    ##     ##  ##  #### ##    ##
//    ##    ##       ##    ##    ##     ##  ##   ### ##    ##
//    ##    ########  ######     ##    #### ##    ##  ######

//    ###    ########  ########    ###
//   ## ##   ##     ## ##         ## ##
//  ##   ##  ##     ## ##        ##   ##
// ##     ## ########  ######   ##     ##
// ######### ##   ##   ##       #########
// ##     ## ##    ##  ##       ##     ##
// ##     ## ##     ## ######## ##     ##

// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######







//-----------------------------------------------------------------------------------------
//  ######   #######  ##     ## ########     ###    ########  ########
// ##    ## ##     ## ###   ### ##     ##   ## ##   ##     ## ##
// ##       ##     ## #### #### ##     ##  ##   ##  ##     ## ##
// ##       ##     ## ## ### ## ########  ##     ## ########  ######
// ##       ##     ## ##     ## ##        ######### ##   ##   ##
// ##    ## ##     ## ##     ## ##        ##     ## ##    ##  ##
//  ######   #######  ##     ## ##        ##     ## ##     ## ########


// ##    ## ########  ######           ##      ## ######## ####  ######   ##     ## ########
// ###   ## ##       ##    ##          ##  ##  ## ##        ##  ##    ##  ##     ##    ##
// ####  ## ##       ##                ##  ##  ## ##        ##  ##        ##     ##    ##
// ## ## ## ######   ##   ####         ##  ##  ## ######    ##  ##   #### #########    ##
// ##  #### ##       ##    ##          ##  ##  ## ##        ##  ##    ##  ##     ##    ##
// ##   ### ##       ##    ##          ##  ##  ## ##        ##  ##    ##  ##     ##    ##
// ##    ## ########  ######            ###  ###  ######## ####  ######   ##     ##    ##
//-----------------------------------------------------------------------------------------

/**
 * Compare shapes of distributions of BDT input variables in 3 cases : with all events, with only positive weights events, and with all events but absolute weights. Output = comparison plots for all vars & 3 main samples (tZq, ttZ, WZ)
 * @param  channel     [Channel name (for single channel plots)]
 * @param  allchannels [single or sum of 4 channels]
 */
/*
void theMVAtool::Compare_Negative_Weights_Effect_On_Distributions(TString channel, bool allchannels)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Compare_Without_Negative_Weights ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	//File containing histograms of input vars, w/ only events w/ weight>0
	TString input_file_name = "outputs/ControlFiles_noNegWeights/Control_Histograms" + this->filename_suffix + ".root";
	TFile* f = 0;
	f = TFile::Open( input_file_name );
	//File containing histograms of input vars, w/ ALL events
	input_file_name = "outputs/ControlFiles_withNegWeights/Control_Histograms" + this->filename_suffix + ".root";
	TFile* f_neg = 0;
	f_neg = TFile::Open( input_file_name );
	//File containing histograms of input vars, w/ ALL events but fabs(weight)
	input_file_name = "outputs/ControlFiles_withAbsWeights/Control_Histograms" + this->filename_suffix + ".root";
	TFile* f_abs = 0;
	f_abs = TFile::Open( input_file_name );

	if(!f || !f_neg || !f_abs) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

	TH1::SetDefaultSumw2();
	mkdir("plots/",0777); //Create directory if inexistant
	mkdir("plots/compare_negweights/",0777); //Create directory if inexistant
	mkdir("plots/compare_negweights/allchan",0777); //Create directory if inexistant

	vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
	thechannellist.push_back("uuu");
	thechannellist.push_back("uue");
	thechannellist.push_back("eeu");
	thechannellist.push_back("eee");

	//3 main samples we want to check
	vector<TString> samples;
	samples.push_back("tZqmcNLO"); //FIXME
	// samples.push_back("WZjets"); //CHANGED
	samples.push_back("WZl");
	samples.push_back("WZb");
	samples.push_back("WZc");
	samples.push_back("ttZ");

	//Load Canvas definition
	Load_Canvas_Style();

	TH1F *h_tmp = 0, *h_tmp_neg = 0, *h_tmp_abs = 0;
	TH1F *h = 0, *h_neg = 0, *h_abs = 0;

	TLegend* qw = 0;

	//Want to plot ALL activated variables (BDT vars + cut vars !)
	vector<TString> total_var_list;
	for(int i=0; i<v_cut_name.size(); i++)
	{
		total_var_list.push_back(v_cut_name[i].Data());
	}
	for(int i=0; i<var_list.size(); i++)
	{
		total_var_list.push_back(var_list[i].Data());
	}

//---------------------
//Loop on var > sample > chan
//Retrieve histograms, plot
	for(int ivar=0; ivar<total_var_list.size(); ivar++)
	{
		for(int isample = 0; isample < samples.size(); isample++)
		{
			TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);

			h = 0; h_neg = 0; h_abs = 0;

			//TLegend* qw = new TLegend(.80,.60,.95,.90);
			qw = new TLegend(.7,.7,0.965,.915);
			qw->SetShadowColor(0);
			qw->SetFillColor(0);
			qw->SetLineColor(0);

			int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
			for(int ichan=0; ichan<thechannellist.size(); ichan++)
			{
				if(!allchannels && channel != thechannellist[ichan]) {continue;} //If plot single channel

				int nof_missing_samples = 0; //is needed to access the right bin of v_MC_histo
				h_tmp = 0; //Temporary storage of histogram
				h_tmp_neg = 0;
				h_tmp_abs = 0;

				//CHANGED
				// TString histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + samples[isample];
				TString histo_name = total_var_list[ivar] + "_" + thechannellist[ichan] + "__" + samples[isample];
				if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
				h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();

				if(!f_neg->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
				h_tmp_neg = (TH1F*) f_neg->Get(histo_name.Data())->Clone();

				if(!f_abs->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
				h_tmp_abs = (TH1F*) f_abs->Get(histo_name.Data())->Clone();


				//Reference = distrib. with ALL events (relative weights)
				h_tmp_neg->SetFillStyle(3345);
				h_tmp_neg->SetFillColor(kRed);
				h_tmp_neg->SetLineColor(kRed);

				h_tmp->SetLineColor(kBlue);

				h_tmp_abs->SetLineColor(kGreen+2);

				if(niter_chan == 0) {h = (TH1F*) h_tmp->Clone(); h_neg = (TH1F*) h_tmp_neg->Clone();  h_abs = (TH1F*) h_tmp_abs->Clone();}
				else //Sum channels
				{
					h->Add(h_tmp); h_neg->Add(h_tmp_neg); h_abs->Add(h_tmp_abs);
				}

			niter_chan++;
			} //end channel loop

			qw->AddEntry(h_neg, "ALL events", "F");
			qw->AddEntry(h, "Events w/ positive weight", "L");
			qw->AddEntry(h_abs, "All events, fabs(weight)", "L");

			h->Scale(1./h->Integral());
			h_neg->Scale(1./h_neg->Integral());
			h_abs->Scale(1./h_abs->Integral());

			if(h->GetMaximum() < h_neg->GetMaximum()) {h->SetMaximum(h->GetMaximum() * 1.2);}
			if(h->GetMaximum() < h_abs->GetMaximum()) {h->SetMaximum(h->GetMaximum() * 1.2);}

			c1->cd();

			TString title = total_var_list[ivar] + " / " + samples[isample] ;
			h->GetXaxis()->SetTitle((title.Data()));

			h->Draw("hist h");
			h_neg->Draw("hist same h");
			h_abs->Draw("hist same h");
			qw->Draw("same");

			//Image name
			TString outputname = "plots/compare_negweights/"+total_var_list[ivar]+"_"+samples[isample]+"_"+channel+this->format;
			if(channel == "" || allchannels) {outputname = "plots/compare_negweights/allchan/"+total_var_list[ivar]+"_" + samples[isample]+"_all"+this->format;}

			if(c1!= 0) {c1->SaveAs(outputname.Data() );}
			//cout << __LINE__ << endl;

			delete c1;
		} //end sample loop

	} //end var loop

	delete h; delete h_neg; delete h_abs;
	delete h_tmp; delete h_tmp_neg; delete h_tmp_abs;
	delete qw;

	return;
}
*/


/**
 * Run Combine on template file (arg), using datacard corresponding to tZq signal
 * @param  path_templatefile path of the templatefile on which to run Combine
 * @return                   computed significance value
 */
float theMVAtool::Compute_Combine_Significance_From_TemplateFile(TString path_templatefile, TString signal, bool expected, bool use_syst)
{
	CopyFile(path_templatefile, "./COMBINE/templates/Combine_Input.root"); //Copy file to templates dir.

	TString datacard_path = "./COMBINE/datacards/COMBINED_datacard_TemplateFit_";
	if(signal=="tZq" || signal=="ttZ" || signal=="tZqANDttZ" || signal=="tZqANDFakes")
	{
		datacard_path+= signal;
	}
	else {cout<<"Wrong signal name ! Abort"<<endl; return 0;}

	if(!use_syst) datacard_path+= "_noSyst";

	datacard_path+=".txt";

	TString f_tmp_name = "significance_expected_info_tmp.txt";

	if(expected) {system( ("combine -M ProfileLikelihood --significance -t -1 --expectSignal=1 "+datacard_path+" | tee "+f_tmp_name).Data() );} //Expected a-priori
	else {system( ("combine -M ProfileLikelihood --signif --cminDefaultMinimizerType=Minuit2 "+datacard_path+" | tee "+f_tmp_name).Data() );} //Observed


	ifstream file_in(f_tmp_name.Data());
	string line;
	TString ts;
	while(!file_in.eof())
	{
		getline(file_in, line);
		ts = line;

		if(!ts.Contains("Significance")) {continue;}

		int index = ts.First(' ');
		ts.Remove(0, index);
		break;
	}

	system( ("rm "+f_tmp_name).Data() ); //Remove tmp file

	return Convert_TString_To_Number(ts);
}
