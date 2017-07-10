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
	//Each time a TF1 is created, this automatically creates a structure to store the sum of the squared weights of the entries (used to compute final bin errors)
	//NOTE : if a bin has already been incremented via Fill(), then AddBinContent() & SetBinContent() do not modify it's error !!
	//NOTE : this is why we exclusively modify the bin contents (e.g. put underflow in first bin) using the Fill() function !
	TH1::SetDefaultSumw2();

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

	//Store the "cut name" that will be written as a suffix in the name of each output file
	this->filename_suffix_noJet = "";
	tmp = "";
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if( (v_cut_name[ivar]=="METpt" || v_cut_name[ivar]=="mTW") && v_cut_def[ivar] == ">0") {continue;} //Useless cuts
		if(v_cut_name[ivar]=="NJets" || v_cut_name[ivar]=="NBJets") {continue;}

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
			this->filename_suffix_noJet+= tmp;
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

	Fill_Zpt_Vectors();

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
	double current_luminosity = 35.862; //Moriond 2017 //CHANGED
	this->luminosity_rescale = desired_luminosity / current_luminosity;


	cout<<endl<<BOLD(FBLU("##################################"))<<endl;
	cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<current_luminosity<<" = "<<luminosity_rescale<<" ! ---"<<endl;
	cout<<BOLD(FBLU("##################################"))<<endl<<endl;
}






/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Initialize vectors with SFs used for Zpt reweighting
 */
void theMVAtool::Fill_Zpt_Vectors()
{
	v_Zpt_el_tZq[0]=1.59077;
	v_Zpt_el_tZq[1]=0.977813;
	v_Zpt_el_tZq[2]=0.871409;
	v_Zpt_el_tZq[3]=0.859375;
	v_Zpt_el_tZq[4]=0.967098;
	v_Zpt_el_tZq[5]=1.12737;
	v_Zpt_el_tZq[6]=0.647202;
	v_Zpt_el_tZq[7]=0.635454;
	v_Zpt_el_tZq[8]=1.07226;
	v_Zpt_el_tZq[9]=0.775253;

	v_Zpt_el_ttZ[0]=0.943188;
	v_Zpt_el_ttZ[1]=1.02488;
	v_Zpt_el_ttZ[2]=1.27178;
	v_Zpt_el_ttZ[3]=1.04542;
	v_Zpt_el_ttZ[4]=0.958649;
	v_Zpt_el_ttZ[5]=0.990099;
	v_Zpt_el_ttZ[6]=0.543444;
	v_Zpt_el_ttZ[7]=1.05011;
	v_Zpt_el_ttZ[8]=2.15717;
	v_Zpt_el_ttZ[9]=0.660766;

	v_Zpt_el_WZ1jet[0]=1.79345;
	v_Zpt_el_WZ1jet[1]=1.12125;
	v_Zpt_el_WZ1jet[2]=0.748378;
	v_Zpt_el_WZ1jet[3]=0.582597;
	v_Zpt_el_WZ1jet[4]=0.436228;
	v_Zpt_el_WZ1jet[5]=0.414118;
	v_Zpt_el_WZ1jet[6]=0.432137;
	v_Zpt_el_WZ1jet[7]=0.618596;
	v_Zpt_el_WZ1jet[8]=0.609542;
	v_Zpt_el_WZ1jet[9]=0.526724;

	v_Zpt_el_WZ2jet[0]=1.63767;
	v_Zpt_el_WZ2jet[1]=1.20519;
	v_Zpt_el_WZ2jet[2]=0.857887;
	v_Zpt_el_WZ2jet[3]=0.710257;
	v_Zpt_el_WZ2jet[4]=0.840998;
	v_Zpt_el_WZ2jet[5]=0.639715;
	v_Zpt_el_WZ2jet[6]=0.605022;
	v_Zpt_el_WZ2jet[7]=0.654072;
	v_Zpt_el_WZ2jet[8]=1.0467;
	v_Zpt_el_WZ2jet[9]=0.704575;

	v_Zpt_el_WZ3morejet[0]=1.20234;
	v_Zpt_el_WZ3morejet[1]=1.17959;
	v_Zpt_el_WZ3morejet[2]=0.8368;
	v_Zpt_el_WZ3morejet[3]=1.01174;
	v_Zpt_el_WZ3morejet[4]=0.932266;
	v_Zpt_el_WZ3morejet[5]=1.05073;
	v_Zpt_el_WZ3morejet[6]=0.652322;
	v_Zpt_el_WZ3morejet[7]=1.20203;
	v_Zpt_el_WZ3morejet[8]=1.11716;
	v_Zpt_el_WZ3morejet[9]=0.814289;



	v_Zpt_mu_tZq[0]=1.67468;
	v_Zpt_mu_tZq[1]=1.12136;
	v_Zpt_mu_tZq[2]=0.944309;
	v_Zpt_mu_tZq[3]=0.762743;
	v_Zpt_mu_tZq[4]=0.74524;
	v_Zpt_mu_tZq[5]=0.871755;
	v_Zpt_mu_tZq[6]=0.576436;
	v_Zpt_mu_tZq[7]=0.645049;
	v_Zpt_mu_tZq[8]=0.450624;
	v_Zpt_mu_tZq[9]=0.748974;

	v_Zpt_mu_ttZ[0]=1.78967;
	v_Zpt_mu_ttZ[1]=1.52187;
	v_Zpt_mu_ttZ[2]=0.945639;
	v_Zpt_mu_ttZ[3]=0.830342;
	v_Zpt_mu_ttZ[4]=0.903506;
	v_Zpt_mu_ttZ[5]=0.608179;
	v_Zpt_mu_ttZ[6]=0.553372;
	v_Zpt_mu_ttZ[7]=1.44008;
	v_Zpt_mu_ttZ[8]=0.637702;
	v_Zpt_mu_ttZ[9]=0.271906;

	v_Zpt_mu_WZ1jet[0]=3.07308;
	v_Zpt_mu_WZ1jet[1]=1.45012;
	v_Zpt_mu_WZ1jet[2]=0.714364;
	v_Zpt_mu_WZ1jet[3]=0.437299;
	v_Zpt_mu_WZ1jet[4]=0.288349;
	v_Zpt_mu_WZ1jet[5]=0.24043;
	v_Zpt_mu_WZ1jet[6]=0.193958;
	v_Zpt_mu_WZ1jet[7]=0.223087;
	v_Zpt_mu_WZ1jet[8]=0.1316;
	v_Zpt_mu_WZ1jet[9]=0.150708;

	v_Zpt_mu_WZ2jet[0]=1.84157;
	v_Zpt_mu_WZ2jet[1]=1.3925;
	v_Zpt_mu_WZ2jet[2]=1.02222;
	v_Zpt_mu_WZ2jet[3]=0.773407;
	v_Zpt_mu_WZ2jet[4]=0.641132;
	v_Zpt_mu_WZ2jet[5]=0.573534;
	v_Zpt_mu_WZ2jet[6]=0.36697;
	v_Zpt_mu_WZ2jet[7]=0.445029;
	v_Zpt_mu_WZ2jet[8]=0.265312;
	v_Zpt_mu_WZ2jet[9]=0.309481;

	v_Zpt_mu_WZ3morejet[0]=1.8712;
	v_Zpt_mu_WZ3morejet[1]=1.43556;
	v_Zpt_mu_WZ3morejet[2]=1.02624;
	v_Zpt_mu_WZ3morejet[3]=0.789222;
	v_Zpt_mu_WZ3morejet[4]=0.843388;
	v_Zpt_mu_WZ3morejet[5]=0.985938;
	v_Zpt_mu_WZ3morejet[6]=0.530604;
	v_Zpt_mu_WZ3morejet[7]=0.487662;
	v_Zpt_mu_WZ3morejet[8]=0.560811;
	v_Zpt_mu_WZ3morejet[9]=0.57125;

	return;
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

	cout<<endl<<BOLD(FGRN("-- USE ONLY ttZ / WZ / ZZ / tZq FOR TRAINING!"))<<endl;

	if(!use_ttZaMCatNLO) cout<<BOLD(FGRN("-- USE TTZ MADGRAPH SAMPLE !"))<<endl<<endl;
	else cout<<BOLD(FGRN("-- USE TTZ aMC@NLO SAMPLE !"))<<endl<<endl;


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
			if(sample_list[isample].Contains("tZq")) {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "fabs(Weight)" );}
			else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "fabs(Weight)" );} //CHANGED
			// if(sample_list[isample].Contains("tZq")) {factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "Weight" );}
			// else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "Weight" );}
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

//Boosted Decision Trees -- Choose method

	//NOTE : to be used for now
	factory->BookMethod(TMVA::Types::kBDT,method_title.Data(),"!H:!V:NTrees=200:nCuts=200:MaxDepth=2:BoostType=Grad:Shrinkage=0.4:IgnoreNegWeightsInTraining=True");


	//--- Adaptive Boost (old method)
	// if(isttZ) factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=15:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );
	// else factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=20:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );
	// factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=20:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );
	//--- Gradient Boost, used by ttH
	// factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:V:NTrees=200:nCuts=200:MaxDepth=2:BoostType=Grad:Shrinkage=0.10:IgnoreNegWeightsInTraining=True");


//--- TO TAKE NEGATIVE WEIGHTS INTO ACCOUNT (ttZ aMC@NLO) (From Mathias) - Need to use option IgnoreEventsWithNegWeightsInTraining ?
	//Adaboost
	// factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:BoostType=AdaBoost:AdaBoostBeta=0.4:PruneMethod=CostComplexity:PruneStrength=7:SeparationType=CrossEntropy:MaxDepth=3:nCuts=40:NodePurityLimit=0.5:NTrees=1000:MinNodeSize=1%:NegWeightTreatment=InverseBoostNegWeights" ); //Negweight treatment -> much longer


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
double theMVAtool::Compute_Fake_SF(TFile* f, TString channel)
{
	if(channel!="uuu" && channel!="eee" && channel!="eeu" && channel!="uue") {cout<<__LINE__<<" : "<<"Wrong channel name !"; return 0;}

	std::vector<TString> v_otherBackgrounds;
	for(int isample=0; isample<sample_list.size(); isample++)
	{
		if(sample_list[isample] == "Data" || sample_list[isample].Contains("Fake") ) {continue;} //Treated separately

		TString name_tmp = "mTW_uuu__"+sample_list[isample];
		if(!f ->GetListOfKeys()->Contains( name_tmp.Data() ) ) {cout<<name_tmp.Data()<<" not found !"<<endl;  continue;}

		v_otherBackgrounds.push_back(sample_list[isample]);
	}
	cout<<endl<<endl;

	f->cd();
	TH1F * hdata=0, *hsum=0, * hfake=0, *h_tmp=0;

	//Treat Data / Fakes / Other samples separately
	TString histo_name;

	//DATA
	if(combine_naming_convention) histo_name = "mTW_" + channel + "__data_obs";
	else histo_name = "mTW_" + channel + "__DATA";

	if ( !f->GetListOfKeys()->Contains( histo_name.Data() ) ) {cout<<histo_name.Data()<<" not found !"<<endl; return 0;}
	h_tmp = (TH1F*)f->Get( histo_name);
	hdata = (TH1F*) h_tmp->Clone();

	//FAKES -- sum contributions from muon & electron fakes
	histo_name = "mTW_" + channel + "__FakesElectron";
	if ( !f->GetListOfKeys()->Contains( histo_name.Data() ) )  {cout<<histo_name.Data()<<" not found !"<<endl; return 0;}
	h_tmp = (TH1F*)f->Get( histo_name);
	hfake = (TH1F*) h_tmp->Clone();
	histo_name = "mTW_" + channel + "__FakesMuon";
	if ( !f->GetListOfKeys()->Contains( histo_name.Data() ) )  {cout<<histo_name.Data()<<" not found !"<<endl; return 0;}
	h_tmp = (TH1F*)f->Get( histo_name);
	hfake->Add(h_tmp);

	//MC
	for (int i = 0; i<v_otherBackgrounds.size(); i++)
	{
		histo_name = "mTW_" + channel + "__" + v_otherBackgrounds[i];
		if ( !f->GetListOfKeys()->Contains( histo_name.Data() ) ) {cout<<histo_name.Data()<<" not found !"<<endl; return 0;}
		h_tmp = (TH1F*)f->Get( histo_name);
		if(i==0) {hsum = (TH1F*) h_tmp->Clone();}
		else {hsum->Add(h_tmp);}
	}

	if(!hdata || !hsum || !hfake) {cout<<"Warning : null histogram --> CAN NOT RESCALE FAKE HISTOGRAMS !"<<endl; return 0;}

	TObjArray *mc = new TObjArray(2); //Create array of MC samples -- differentiate fakes from rest
	mc->Add(hfake); //Param 0
	mc->Add(hsum); //Param 1

	TFractionFitter* fit = new TFractionFitter(hdata, mc, "Q"); //'Q' for quiet

	//Constrain backgrounds which are not fake (NB : because we're only interested in fitting the fakes to the data here!)
	double fracmc = hsum->Integral()/hdata->Integral() ;
	fit->Constrain(0, 0, 1); //Constrain param 0 (fakes integral) between 0 & 1
	fit->Constrain(1, fracmc*0.99, fracmc*1.01); //Constrain fraction of other backgrounds to prefit value, to ~fix it!

	// TFitResultPtr r = fit->Fit(); //Create smart ptr to TFitResult --> can access fit properties
	// double fakes_frac = r->Parameter(0);

	Int_t status = fit->Fit(); //Perform fit
	if(status != 0) {cout<<"ERROR ! Problem during fit !"<<endl; return 0;} //check status

	double fraction_fake=0, error_fake=0;
	double fraction_other=0, error_other=0;
	fit->GetResult(0, fraction_fake, error_fake);
	fit->GetResult(1, fraction_other, error_other);

	cout<<"Fraction_fakes = "<<fraction_fake<<" , error = "<<error_fake<<endl;
	cout<<"Fraction other = "<<fraction_other<<", error = "<<error_other<<endl;

	TCanvas* c1 = new TCanvas("c1");

	TH1F* result = (TH1F*) fit->GetPlot();
	hdata->Draw("Ep");
	result->Draw("same");

	mkdir("plots",0777); //Create directory if inexistant

	c1->SaveAs(("plots/ScaleFakes_"+channel+".png").Data()); //Save fit plot

	// cout<<endl<<"Fake lepton flavour "<<fakeLep_flavour<<" : Fraction of Fakes fitted from data = "<<fakes_postfit*100<<" %"<<endl;

	delete mc; delete fit; delete c1;

	return fraction_fake * (hdata->Integral() / hfake->Integral() ); //Return 'postfit fakes fraction' over 'prefit fakes fraction' ==> SF
}






/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


/**
 * Rescale Fake histograms with SFs computed with TFractionFitter
 * @param file_to_rescale_name [Name of file containing fake histograms to be rescaled]
 */
void theMVAtool::Rescale_Fake_Histograms(TString file_to_rescale_name)
{
	if(!Check_File_Existence(file_to_rescale_name) ) {cout<<file_to_rescale_name<<" doesn't exist ! Can't rescale Fakes ! "<<endl; return;}

	TFile * file_mTW_templates_unscaled = 0;

	TString file_mTW_templates_unscaled_PATH;
	Long_t *id,*size,*flags,*modtime;
	file_mTW_templates_unscaled_PATH = "outputs/Reader_mTW" + this->filename_suffix_noJet + "_NJetsMin0_NBJetsEq0_unScaled.root"; //mTW unrescaled Template file  => Used to compute the Fakes SFs

	if(!Check_File_Existence(file_mTW_templates_unscaled_PATH))
	{
		cout<<file_mTW_templates_unscaled_PATH<<" doesn't exist !"<<endl;
		file_mTW_templates_unscaled_PATH = "outputs/Reader_mTW_NJetsMin0_NBJetsEq0_unScaled.root";
		cout<<"Try using "<<file_mTW_templates_unscaled_PATH<<endl;
		if(!Check_File_Existence(file_mTW_templates_unscaled_PATH))
		{
			cout<<file_mTW_templates_unscaled_PATH<<" doesn't exist ! Can't rescale Fakes !"<<endl; return;
		}
	}

	file_mTW_templates_unscaled = TFile::Open(file_mTW_templates_unscaled_PATH, "READ"); //File containing the templates, from which can compute fake ratio
	if(!file_mTW_templates_unscaled) {cout<<FRED(<<file_mTW_templates_unscaled_PATH.Data()<<" not found! Can't compute Fake Ratio -- Abort")<<endl; return;}

	double SF_uuu = Compute_Fake_SF(file_mTW_templates_unscaled, "uuu");
	double SF_eee = Compute_Fake_SF(file_mTW_templates_unscaled, "eee");
	double SF_uue = Compute_Fake_SF(file_mTW_templates_unscaled, "uue");
	double SF_eeu = Compute_Fake_SF(file_mTW_templates_unscaled, "eeu");

	cout<<endl<<BOLD(FYEL("--- Re-scaling the Fake histograms ---"))<<endl;
	cout<<FYEL("uuu channel SF = "<<SF_uuu<<"")<<endl;
	cout<<FYEL("eeu channel SF = "<<SF_eeu<<"")<<endl;
	cout<<FYEL("eee channel SF = "<<SF_eee<<"")<<endl;
	cout<<FYEL("uue channel SF = "<<SF_uue<<"")<<endl;

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
	for(int isyst=0; isyst<syst_list.size(); isyst++)
	{
		if(!syst_list[isyst].Contains("Fake") && !syst_list[isyst].Contains("Zpt") ) {continue;}

		if(combine_naming_convention) syst_fakes.push_back(Combine_Naming_Convention(syst_list[isyst]));
		else syst_fakes.push_back(Theta_Naming_Convention(syst_list[isyst]));

		//"Fake" shape syst. renamed -- not present in this form in syst_list vector --> add it
	}

	if(combine_naming_convention)
	{
		syst_fakes.push_back("FakeShapeMuUp");
		syst_fakes.push_back("FakeShapeMuDown");
		syst_fakes.push_back("FakeShapeElUp");
		syst_fakes.push_back("FakeShapeElDown");
	}
	else  //Theta
	{
		syst_fakes.push_back("FakeShapeMu__plus");
		syst_fakes.push_back("FakeShapeMu__minus");
		syst_fakes.push_back("FakeShapeEl__plus");
		syst_fakes.push_back("FakeShapeEl__minus");
	}


	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		for(int isyst=0; isyst<syst_fakes.size(); isyst++)
		{
			TString systname = "";
			if(syst_fakes[isyst] != "") systname = "__" + syst_fakes[isyst];

			for(int ivar=0; ivar<total_var_list.size(); ivar++)
			{

				for(int isample=0; isample<sample_list[isample]; isample++)
				{
					if(!sample_list[isample].Contains("Fakes")) {continue;}

					TString histo_name = total_var_list[ivar] + "_" + channel_list[ichan] + "__" + sample_list[isample] + systname;
					if(!file_to_rescale->GetListOfKeys()->Contains(histo_name.Data()) ) {continue;}


					TH1F* h_tmp = (TH1F*) file_to_rescale->Get(histo_name);

					if(channel_list[ichan] == "uuu") {h_tmp->Scale(SF_uuu);}
					else if(channel_list[ichan] == "eeu") {h_tmp->Scale(SF_eeu);}
					else if(channel_list[ichan] == "eee") {h_tmp->Scale(SF_eee);}
					else if(channel_list[ichan] == "uue") {h_tmp->Scale(SF_uue);}


					file_to_rescale->cd();
					h_tmp->Write(histo_name, TObject::kOverwrite);

					delete h_tmp;
				}
			}
		}
	}

	delete file_to_rescale;

	return;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////












//---------------------------------------------------------------------------
// ######## ########  ########    ########  ######## ##      ## ######## ####  ######   ##     ## ########
//      ##  ##     ##    ##       ##     ## ##       ##  ##  ## ##        ##  ##    ##  ##     ##    ##
//     ##   ##     ##    ##       ##     ## ##       ##  ##  ## ##        ##  ##        ##     ##    ##
//    ##    ########     ##       ########  ######   ##  ##  ## ######    ##  ##   #### #########    ##
//   ##     ##           ##       ##   ##   ##       ##  ##  ## ##        ##  ##    ##  ##     ##    ##
//  ##      ##           ##       ##    ##  ##       ##  ##  ## ##        ##  ##    ##  ##     ##    ##
// ######## ##           ##       ##     ## ########  ###  ###  ######## ####  ######   ##     ##    ##
//---------------------------------------------------------------------------

/**
 * Extract properties of data-driven event, and reads class-member vectors to return corresponding Scale factor
 * This is used to add a "Zpt reweighting" systematic, in order to account for a cinematic bias in construction of data-driven Fakes sample
 */
 // double theMVAtool::Get_Zpt_Reweighting_SF(int event_ientry)
 double theMVAtool::Get_Zpt_Reweighting_SF(int Channel, int NJets, double Zpt)
{
	double SF=0;

	double tmp = Zpt/30; //Scale factors are computed for ranges of 30 GeV
	int index_vector = tmp;
	if(index_vector > 9) {index_vector=9;} //Overflow in last bin
	else if(index_vector<0) {cout<<"Error : wrong Zpt value ! Can't add systematic 'ZptReweight' !"<<endl;}


	if(Channel==0 || Channel==2) //uuu & eeu
	{
		if(!isWZ && !isttZ)
		{
			return v_Zpt_mu_tZq[index_vector];
		}
		else if(!isWZ && isttZ)
		{
			return v_Zpt_mu_ttZ[index_vector];
		}
		else if(isWZ && !isttZ)
		{
			if(NJets==1) {return v_Zpt_mu_WZ1jet[index_vector];}
			else if(NJets==2) {return v_Zpt_mu_WZ2jet[index_vector];}
			else {return v_Zpt_mu_WZ3morejet[index_vector];}
		}
		else {cout<<"ERROR in Zpt reweighting : wrong region !"<<endl; return 0;
		}
	}
	else if(Channel==1 || Channel==3) //uue & eee
	{
		if(!isWZ && !isttZ)
		{
			// cout<<"Zpt "<<Zpt<<" - i "<<index_vector<<" - SF = "<<v_Zpt_el_tZq[index_vector]<<endl;
			return v_Zpt_el_tZq[index_vector];
		}
		else if(!isWZ && isttZ)
		{
			return v_Zpt_el_ttZ[index_vector];
		}
		else if(isWZ && !isttZ)
		{
			if(NJets==1) {return v_Zpt_el_WZ1jet[index_vector];}
			else if(NJets==2) {return v_Zpt_el_WZ2jet[index_vector];}
			else {return v_Zpt_el_WZ3morejet[index_vector];}
		}
		else {cout<<"ERROR in Zpt reweighting : wrong region !"<<endl; return 0;
		}
	}

	else {cout<<"Error : wrong channel name !"<<endl; return 0;}

	return 0;
}









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
 * @param  cut_on_BDT            Cut value on BDT (-> cran create templates in CR)
 */
int theMVAtool::Read(TString template_name, bool fakes_from_data, bool real_data, bool cut_on_BDT, bool keep_high_BDT_events, double BDT_cut_value, bool combine_mTW_BDT_SR)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;}
	else {cout<<BOLD(FRED("--- ERROR : invalid template_name value ! Exit !"))<<endl; cout<<"Valid names are : BDT/BDTttZ/mTW !"<<endl; return 0;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
	else {cout<<FYEL("--- Using fakes from data ---")<<endl;}
	if(!real_data) {cout<<FYEL("--- Not using real data ---")<<endl<<endl;}
	else {cout<<FYEL("--- Using REAL data ---")<<endl<<endl;}
	if(cut_on_BDT) {cout<<FYEL("--- Creating templates WITH cut on BDT value // Cut value = "<<BDT_cut_value<<" ----")<<endl; if(BDT_cut_value==-99) {return 1;} }

	if(combine_mTW_BDT_SR) {cout<<endl<<endl<<endl<<BOLD(FYEL(" !!CREATING COMBINED TEMPLATES IN SR : USE BOTH MTW AND BDT DISTRIBUTIONS !!"))<<endl<<endl<<endl<<endl<<endl;}

	bool combine_mTW_BDT = false;
	if(combine_mTW_BDT_SR && template_name == "BDT") combine_mTW_BDT = true;

	mkdir("outputs",0777);

	TString output_file_name = "outputs/Reader_" + template_name + this->filename_suffix;
	if(cut_on_BDT) output_file_name+= "_CutBDT";
	//Cf. Fakes rescaling function : need to have an 'unscaled' version of the mTW template to compute the Fakes SFs
	if(template_name == "mTW") output_file_name+= "_unScaled.root"; //Add suffix to distinguish it : this file will be used to compute SFs
	else output_file_name+= ".root";

	TFile* file_output = TFile::Open( output_file_name, "RECREATE" );

	TH1::SetDefaultSumw2();

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

	TH1F *hist_uuu = 0, *hist_uue = 0, *hist_eeu = 0, *hist_eee = 0;

	//Store sum of weights of nominal template : needed to rescale Zpt & FakeShape systematics to nominal
	double integral_FakesElectron_nominal_uuu = 0;
	double integral_FakesElectron_nominal_uue = 0;
	double integral_FakesElectron_nominal_eeu = 0;
	double integral_FakesElectron_nominal_eee = 0;

	double integral_FakesMuon_nominal_uuu = 0;
	double integral_FakesMuon_nominal_uue = 0;
	double integral_FakesMuon_nominal_eeu = 0;
	double integral_FakesMuon_nominal_eee = 0;


	// --- Systematics loop
	for(int isyst=0; isyst<syst_list.size(); isyst++)
	{
		cout<<endl<<endl<<FGRN("Systematic "<<syst_list[isyst]<<" ("<<isyst+1<<"/"<<syst_list.size()<<") :")<<endl;

		//Loop on samples, syst., events ---> Fill histogram/channel --> Write()
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
			else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only


			if(!real_data && sample_list[isample] == "Data") {continue;} //Don't use the real data

			if( (sample_list[isample]=="Data" && syst_list[isyst]!="")
				|| ( (sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) && (syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") && !syst_list[isyst].Contains("Zpt") ) ) ) {continue;} //Only 'Fakes' syst. taken into account for Fakes

			if( (syst_list[isyst].Contains("Fakes") || syst_list[isyst].Contains("Zpt") ) && !sample_list[isample].Contains("Fakes") && !sample_list[isample].Contains("DY") && !sample_list[isample].Contains("TT") && !sample_list[isample].Contains("WW"))   {continue;} //"Fakes" syst only in fakes samples

			if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}

			if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

			TString inputfile = dir_ntuples + "/FCNCNTuple_" + sample_list[isample] + ".root";

			//For these systematics, info stored in separate ntuples ! (events are not the same)
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQup.root";}
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__minus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQdw.root";}
			else if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "Hadron__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqhwpp.root";} //For minus, take nominal -- verify


			if(!Check_File_Existence(inputfile) ) {cout<<inputfile.Data()<<" not found!"<<endl; continue;}
			file_input = TFile::Open( inputfile.Data() );
			if(dbgMode) std::cout << "--- Select "<<sample_list[isample]<<" sample" << __LINE__ <<std::endl;


			if(fakes_from_data || (!fakes_from_data && !sample_list[isample-1].Contains("DY") && !sample_list[isample-1].Contains("TT") && !sample_list[isample-1].Contains("WW")) ) //MC Fakes : Don't reinitialize histos -> sum !
			{
				// Book output histograms
				if(combine_mTW_BDT)
				{
					hist_uuu     = new TH1F( (template_name+"_uuu").Data(),           (template_name+"_uuu").Data(),           15, 0, 150);
					hist_uue     = new TH1F( (template_name+"_uue").Data(),           (template_name+"_uue").Data(),           15, 0, 150);
					hist_eeu     = new TH1F( (template_name+"_eeu").Data(),           (template_name+"_eeu").Data(),           15, 0, 150);
					hist_eee     = new TH1F( (template_name+"_eee").Data(),           (template_name+"_eee").Data(),           15, 0, 150);
					//
					// hist_uuu     = new TH1F( (template_name+"_uuu").Data(),           (template_name+"_uuu").Data(),           10, 0, 200);
					// hist_uue     = new TH1F( (template_name+"_uue").Data(),           (template_name+"_uue").Data(),           10, 0, 200);
					// hist_eeu     = new TH1F( (template_name+"_eeu").Data(),           (template_name+"_eeu").Data(),           10, 0, 200);
					// hist_eee     = new TH1F( (template_name+"_eee").Data(),           (template_name+"_eee").Data(),           10, 0, 200);
				}
				else if (template_name == "BDT" || template_name == "BDTttZ") //create histogram for each channel (-1 = bkg, +1 = signal)
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

					// hist_uuu     = new TH1F( "mTW_uuu",           "mTW_uuu",           20, 0., 150 ); //new binning
					// hist_uue     = new TH1F( "mTW_uue",           "mTW_uue",           20, 0., 150 );
					// hist_eeu     = new TH1F( "mTW_eeu",           "mTW_eeu",           20, 0., 150 );
					// hist_eee     = new TH1F( "mTW_eee",           "mTW_eee",           20, 0., 150 );
				}
			}

			tree = 0;

			//For JES & JER systematics, need a different tree (modify the variables distributions' shapes)
			if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") )
			{
				tree = (TTree*) file_input->Get(syst_list[isyst].Data());
			}
			else {tree = (TTree*) file_input->Get(t_name.Data());}

			if(!tree && syst_list[isyst]=="") {cout<<BOLD(FRED("ERROR : nominal tree not found ! Abort"))<<endl; return 0;}
			else if(!tree) {cout<<BOLD(FRED("ERROR : tree "<<syst_list[isyst]<<" not found ! Skip this systematic"))<<endl; break;}

//--- Prepare the event tree -- Set Branch Addresses
//WARNING : the last SetBranchAddress overrides the previous ones !! Be careful not to associate branches twice !

			Float_t *Zpt=0, *njets=0; //For ZptReweight syst., need to access Zpt and njet values for each fake event. However, can SetBranchAddress only once per variable, and this has to be done already for vectors for automation. So need to create dedicated pointers to access these values in the event loop.

			for(int i=0; i<v_add_var_names.size(); i++)
			{
				tree->SetBranchAddress(v_add_var_names[i].Data(), &v_add_var_floats[i]);

				if(v_add_var_names[i]=="Zpt") {Zpt = &v_add_var_floats[i];}
				else if(v_add_var_names[i]=="NJets") {njets = &v_add_var_floats[i];}
			}
			for(int i=0; i<var_list.size(); i++)
			{
				tree->SetBranchAddress(var_list[i].Data(), &var_list_floats[i]);

				if(var_list[i]=="Zpt") {Zpt = &var_list_floats[i];}
				else if(var_list[i]=="NJets") {njets = &var_list_floats[i];}
			}
			for(int i=0; i<v_cut_name.size(); i++)
			{
				tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);

				if(v_cut_name[i]=="Zpt") {Zpt = &v_cut_float[i];}
				else if(v_cut_name[i]=="NJets") {njets = &v_cut_float[i];}
			}

			//Dedicated variables, easier to access in event loop
			float mTW = -666; tree->SetBranchAddress("mTW", &mTW);
			float i_channel = 9; tree->SetBranchAddress("Channel", &i_channel);


			float weight;
			//For all other systematics, only the events weights change
			if(syst_list[isyst] == "" || syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") || syst_list[isyst].Contains("Fakes") || syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") || syst_list[isyst].Contains("Zpt"))	{tree->SetBranchAddress("Weight", &weight);}
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

				tree->GetEntry(ievt);

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
					if(v_cut_def[ivar] == "") {continue;}

					else if((sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW")) && (v_cut_name[ivar] == "AdditionalMuonIso" || v_cut_name[ivar] == "AdditionalEleIso") && v_cut_def[ivar].Contains("<") )
					{
						cout<<endl<<endl<<BOLD(FYEL("Not applying '<' cuts on isolation for Fakes samples !"))<<endl<<endl<<endl;

						continue;
					}

					//Can't set Branch address of same branch to 2 different variables (only last one will be filled)
					//Since I define myself a 'mTW' variable, it is a problem if one of the cuts is on mTW (because the associated float won't be filled)
					//---> If v_cut_name[i] == "mTW", need to make sure that we use the variable which is filled !
					if(v_cut_name[ivar] == "mTW") {v_cut_float[ivar] = mTW;}
					//Idem for channel value
					if(v_cut_name[ivar] == "Channel") {v_cut_float[ivar] = i_channel;}

					// cout<<v_cut_name[ivar]<<" "<<v_cut_float[ivar]<<endl;

					if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) //If cut contains only 1 condition
					{
						cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);
						if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break;}
						else if(v_cut_def[ivar].Contains("!=") && v_cut_float[ivar] == cut_tmp)  {pass_all_cuts = false; break;}
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
						else if(cut1.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_all_cuts = false; break;}
						//CUT 2
						cut_tmp = Find_Number_In_TString(cut2);
						if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
						else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
						else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
						else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
						else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
						else if(cut2.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_all_cuts = false; break;}
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
						else if(cut1.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_cut1 = false;}
						//CUT 2
						cut_tmp = Find_Number_In_TString(cut2);
						if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_cut2 = false;}
						else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_cut2 = false;}
						else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_cut2 = false;}
						else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_cut2 = false;}
						else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_cut2 = false;}
						else if(cut2.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_cut2 = false;}

						if(!pass_cut1 && !pass_cut2) {pass_all_cuts = false; break;}
					}
				}


				if(!pass_all_cuts) {continue;}

				TString chan_name = "uuu";
				if(i_channel == 1) chan_name = "uue";
				else if(i_channel == 2) chan_name = "eeu";
				else if(i_channel == 3) chan_name = "eee";

				TString MVA_method_name = template_name + "_" + chan_name + this->filename_suffix + " method";
				if(cut_on_BDT && !template_name.Contains("BDT")) {MVA_method_name = "BDT_" + chan_name + this->filename_suffix + " method";} //For mTW templates with cut on BDT

				// if(cut_on_BDT && reader->EvaluateMVA(MVA_method_name.Data() ) < 0.4) {continue;} //CHANGE HERE IF WANT TO KEEP HIGH BDT VALUES INSTEAD !!
				if(cut_on_BDT)
				{
					if(keep_high_BDT_events && reader->EvaluateMVA(MVA_method_name.Data() ) < BDT_cut_value) {continue;} //Cut on BDT value
					else if(!keep_high_BDT_events && reader->EvaluateMVA(MVA_method_name.Data() ) >= BDT_cut_value) {continue;} //Cut on BDT value
				}

//------------------------------------------------------------
//------------------------------------------------------------
//--- CREATE OUTPUT HISTOGRAMS (TEMPLATES) ----

				// --- Return the MVA outputs and fill into histograms

				//Add 1-sided systematic to reweight data-driven Fakes sample
				if(syst_list[isyst] == "ZptReweight__minus")
				{
					weight*= Get_Zpt_Reweighting_SF(i_channel, *njets, *Zpt); //Multiply by scale-factor ; njets & Zpt are dedicated variables, see where they are defined
				}

				//Same boundaries & nbins for all 4 histos

				double xmax_h = hist_uuu->GetXaxis()->GetXmax();
				int lastbin_h = hist_uuu->GetNbinsX();
				double mva_value = -999;

				if(syst_list[isyst]=="" && sample_list[isample]=="FakesElectron")
				{
					if(i_channel==0) {integral_FakesElectron_nominal_uuu+= weight;}
					else if(i_channel==1) {integral_FakesElectron_nominal_uue+= weight;}
					else if(i_channel==2) {integral_FakesElectron_nominal_eeu+= weight;}
					else if(i_channel==3) {integral_FakesElectron_nominal_eee+= weight;}
				}
				else if(syst_list[isyst]=="" && sample_list[isample]=="FakesMuon")
				{
					if(i_channel==0) {integral_FakesMuon_nominal_uuu+= weight;}
					else if(i_channel==1) {integral_FakesMuon_nominal_uue+= weight;}
					else if(i_channel==2) {integral_FakesMuon_nominal_eeu+= weight;}
					else if(i_channel==3) {integral_FakesMuon_nominal_eee+= weight;}
				}

				//CHANGED -- put overflow in last bin (error treatment ok ?)
				if (template_name == "BDT" || template_name == "BDTttZ")
				{
					if(i_channel == 0)
					{
						if(combine_mTW_BDT && mTW<50) {hist_uuu->Fill(mTW, weight);}
						else if(combine_mTW_BDT && mTW>50)
						{
							mva_value = reader->EvaluateMVA( template_name+"_uuu"+this->filename_suffix+ " method");
							mva_value = mva_value*50 + 50 + 50;
							hist_uuu->Fill(mva_value, weight);
						}
						else
						{
							mva_value = reader->EvaluateMVA( template_name+"_uuu"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_uuu->Fill(mva_value, weight);}
							else {Fill_Last_Bin_TH1F(hist_uuu, weight);} //Put overflow in last bin (no info lost)
						}
					}
					else if(i_channel == 1)
					{
						if(combine_mTW_BDT && mTW<50) {hist_uue->Fill(mTW, weight);}
						else if(combine_mTW_BDT && mTW>50)
						{
							mva_value = reader->EvaluateMVA( template_name+"_uue"+this->filename_suffix+ " method");
							mva_value = mva_value*50 + 50 + 50;
							hist_uue->Fill(mva_value, weight);
						}
						else
						{
							mva_value = reader->EvaluateMVA( template_name+"_uue"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_uue->Fill(mva_value, weight);}
							else {Fill_Last_Bin_TH1F(hist_uue, weight);} //Put overflow in last bin (no info lost)
						}
					}
					else if(i_channel == 2)
					{
						if(combine_mTW_BDT && mTW<50) {hist_eeu->Fill(mTW, weight);}
						else if(combine_mTW_BDT && mTW>50)
						{
							mva_value = reader->EvaluateMVA( template_name+"_eeu"+this->filename_suffix+ " method");
							mva_value = mva_value*50 + 50 + 50;
							hist_eeu->Fill(mva_value, weight);
						}
						else
						{
							mva_value = reader->EvaluateMVA( template_name+"_eeu"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_eeu->Fill(mva_value, weight);}
							else {Fill_Last_Bin_TH1F(hist_eeu, weight);} //Put overflow in last bin (no info lost)
						}
					}
					else if(i_channel == 3)
					{
						if(combine_mTW_BDT && mTW<50) {hist_eee->Fill(mTW, weight);}
						else if(combine_mTW_BDT && mTW>50)
						{
							mva_value = reader->EvaluateMVA( template_name+"_eee"+this->filename_suffix+ " method");
							mva_value = mva_value*50 + 50 + 50;
							hist_eee->Fill(mva_value, weight);
						}
						else
						{
							mva_value = reader->EvaluateMVA( template_name+"_eee"+this->filename_suffix+ " method");
							if(mva_value < xmax_h ) {hist_eee->Fill(mva_value, weight);}
							else {Fill_Last_Bin_TH1F(hist_eee, weight);} //Put overflow in last bin (no info lost)
						}
					}
					else  {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
				}
				// --- Return the MVA outputs and fill into histograms
				else if (template_name == "mTW")
				{
					if(i_channel == 0)
					{
						if(mTW < xmax_h ) {hist_uuu->Fill(mTW, weight);}
						else {Fill_Last_Bin_TH1F(hist_uuu, weight);} //Put overflow in last bin (no info lost)
					}
					else if(i_channel == 1)
					{
						if(mTW < xmax_h ) {hist_uue->Fill(mTW, weight);}
						else {Fill_Last_Bin_TH1F(hist_uue, weight);} //Put overflow in last bin (no info lost)
					}
					else if(i_channel == 2)
					{
						if(mTW < xmax_h ) {hist_eeu->Fill(mTW, weight);}
						else {Fill_Last_Bin_TH1F(hist_eeu, weight);} //Put overflow in last bin (no info lost)
					}
					else if(i_channel == 3)
					{
						if(mTW < xmax_h ) {hist_eee->Fill(mTW, weight);}
						else {Fill_Last_Bin_TH1F(hist_eee, weight);} //Put overflow in last bin (no info lost)
					}

					else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
				}

			} //end entries loop


//---------

			//Re-scale to desired luminosity, unless it's data
			if(sample_list[isample] != "Data")
			{
				hist_uuu->Scale(luminosity_rescale); hist_uue->Scale(luminosity_rescale); hist_eeu->Scale(luminosity_rescale); hist_eee->Scale(luminosity_rescale);
			}

			//ZptReweight & FakeShape systematics needs to have same norm. as nominal
			if(syst_list[isyst].Contains("Zpt") || syst_list[isyst].Contains("Fake"))
			{
				if(sample_list[isample] == "FakesElectron")
				{
					hist_uuu->Scale(integral_FakesElectron_nominal_uuu/hist_uuu->Integral()); hist_eeu->Scale(integral_FakesElectron_nominal_eeu/hist_eeu->Integral()); hist_uue->Scale(integral_FakesElectron_nominal_uue/hist_uue->Integral()); hist_eee->Scale(integral_FakesElectron_nominal_eee/hist_eee->Integral());
				}
				if(sample_list[isample] == "FakesMuon")
				{
					hist_uuu->Scale(integral_FakesMuon_nominal_uuu/hist_uuu->Integral()); hist_eeu->Scale(integral_FakesMuon_nominal_eeu/hist_eeu->Integral()); hist_uue->Scale(integral_FakesMuon_nominal_uue/hist_uue->Integral()); hist_eee->Scale(integral_FakesMuon_nominal_eee/hist_eee->Integral());
				}
			}

			// --- Write histograms
			file_output->cd();

			//NB : theta name convention = <observable>__<process>[__<uncertainty>__(plus,minus)]
			TString output_histo_name = "";
			TString syst_name = "";
			if(combine_naming_convention && syst_list[isyst] != "") {syst_name = "__" + Combine_Naming_Convention(syst_list[isyst]);} //Combine naming convention
			else if(syst_list[isyst] != "") {syst_name = "__" + Theta_Naming_Convention(syst_list[isyst]);} //Theta naming convention
			if(syst_list[isyst].Contains("Fake"))
			{
				syst_name = "__";

				if(combine_naming_convention)
				{
					if(sample_list[isample] == "FakesMuon") syst_name+= "FakeShapeMu";
					else if(sample_list[isample] == "FakesElectron") syst_name+= "FakeShapeEl";
					if(syst_list[isyst].Contains("plus")) syst_name+= "Up";
					else syst_name+= "Down";
				}
				else
				{
					if(sample_list[isample] == "FakesMuon") syst_name+= "FakeShapeMu";
					else if(sample_list[isample] == "FakesElectron") syst_name+= "FakeShapeEl";
					if(syst_list[isyst].Contains("plus")) syst_name+= "__plus";
					else syst_name+= "__minus";
				}
			}

			TString sample_name = sample_list[isample];
			// if(real_data && sample_list[isample] == "Data") {sample_name = "DATA";} //THETA CONVENTION
			if(real_data && sample_list[isample] == "Data")
			{
				if(combine_naming_convention) sample_name = "data_obs";
				else sample_name = "DATA";
			}


			if(!fakes_from_data && isample != sample_list.size()-1 && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) //MC Fakes : Don't reinitialize histos -> sum !
			// if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
			{
				continue;
			}
			else
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
			delete hist_uuu; delete hist_uue; delete hist_eeu; delete hist_eee; //Free memory
			delete file_input; //CHANGED -- free memory
		} //end sample loop

		if(dbgMode) cout<<"Done with syst : "<<syst_list[isyst]<<endl;
	} 	//end syst loop


	std::cout << "--- Created root file: "<<file_output->GetName()<<" containing the output histograms" << std::endl;
	delete file_output;
	std::cout << "==> Reader() is done!" << std::endl << std::endl;

	if(fakes_from_data) Rescale_Fake_Histograms(output_file_name);


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

	TString input_file_name = "outputs/Reader_BDT" + this->filename_suffix + ".root";

	if(!Check_File_Existence(input_file_name))
	{
		cout<<BOLD(FRED("outputs/Reader_BDT"<<this->filename_suffix<<".root not found ! Using outputs/Combine_Input.root instead ! MAKE SURE THERE IS NOT ALREADY A CUT ON BDT IN THAT FILE !"))<<endl;

		input_file_name = "outputs/Combine_Input.root";
	}

	TFile* f = 0;
	f = TFile::Open(input_file_name.Data());
	if(f == 0) {cout<<endl<<"--- No template file "<<input_file_name.Data()<<" found -- Can't determine BDT CR cut value !"<<endl<<endl; return 0;}

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
			else input_histo_name = "BDT_" + channel_list[ichan] + "__" + sample_list[isample];

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
 * @param cut_on_BDT      [If true, cut on BDT to go to a Control Region]
 * @param cut             [Cut value]
 * @param use_pseudodata  [If true, use pseudodata rather than real data]
 */
void theMVAtool::Create_Control_Trees(bool fakes_from_data, bool cut_on_BDT, double cut, bool use_pseudodata, bool keep_high_BDT_events=false)
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


	bool create_syst_histos = false; //For now ,simply disactivate systematics for control plots (too long)
/*	if(syst_list.size() != 1)
	{
		string answer = "";
		cout<<FYEL("Do you want to create histograms for the systematics also ? This will increase")<<BOLD(FYEL(" A LOT "))<<FYEL("the processing time ! -- Type yes/no")<<endl;
		cin>>answer;

		while(answer != "yes" && answer!= "no")
		{
			cout<<"Wrong answer -- Type 'yes' (with syst.) or 'no' (without syst.) !"<<endl;
			cin.clear();
			cin.ignore(1000, '\n');
			cin>>answer;
		}

		if(answer == "yes") create_syst_histos = true;
	}*/

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
				|| ( (sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) && syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") ) ) {continue;} //Data = no syst. -- Fakes = only fake syst.

			if( syst_list[isyst].Contains("Fakes") && !sample_list[isample].Contains("Fakes") && !sample_list[isample].Contains("DY") && !sample_list[isample].Contains("TT") && !sample_list[isample].Contains("WW") )   {continue;} //Fake syst. only for "fakes" samples

			if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only


			TString inputfile = dir_ntuples + "/FCNCNTuple_" + sample_list[isample] + ".root";

			//For these systematics, info stored in separate ntuples ! (events are not the same)
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQup.root";}
			if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "PSscale__minus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqQdw.root";}
			else if(sample_list[isample].Contains("tZq") && syst_list[isyst] == "Hadron__plus") {inputfile = dir_ntuples + "/FCNCNTuple_tZqhwpp.root";} //For minus, take nominal -- verify


			if(!Check_File_Existence(inputfile) ) {cout<<inputfile.Data()<<" not found!"<<endl; continue;}
			file_input = TFile::Open( inputfile.Data() );

			file_input->cd();
			tree = new TTree("tree", "input tree"); //CHANGED


			output_file->cd();
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
							else if(v_cut_def[ivar].Contains("!=") && v_cut_float[ivar] == cut_tmp)  {pass_all_cuts = false; break;}
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
							else if(cut1.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_all_cuts = false; break;}
							//CUT 2
							cut_tmp = Find_Number_In_TString(cut2);
							if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
							else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
							else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
							else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
							else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
							else if(cut2.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_all_cuts = false; break;}
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
							else if(cut1.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_cut1 = false;}
							//CUT 2
							cut_tmp = Find_Number_In_TString(cut2);
							if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_cut2 = false;}
							else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_cut2 = false;}
							else if(cut2.Contains(">") && !v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] <= cut_tmp)		 {pass_cut2 = false;}
							else if(cut2.Contains("<") && !v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] >= cut_tmp) 	 {pass_cut2 = false;}
							else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_cut2 = false;}
							else if(cut2.Contains("!=") && v_cut_float[ivar] == cut_tmp) 	 {pass_cut2 = false;}

							if(!pass_cut1 && !pass_cut2) {pass_all_cuts = false; break;}
						}
					}
				}


				if(!pass_all_cuts) {continue;}
//------------------------------------------------------------

//------------------------------------------------------------
				if(cut_on_BDT)
				{
					if(keep_high_BDT_events) //To keep only HIGH BDT values
					{
						if(i_channel == 0 && reader->EvaluateMVA( BDT_type + "_uuu"+this->filename_suffix+" method") < cut) 		{continue;}
						else if(i_channel == 1 && reader->EvaluateMVA( BDT_type + "_uue"+this->filename_suffix+" method") < cut) 	{continue;}
						else if(i_channel == 2 && reader->EvaluateMVA( BDT_type + "_eeu"+this->filename_suffix+" method") < cut) 	{continue;}
						else if(i_channel == 3 && reader->EvaluateMVA( BDT_type + "_eee"+this->filename_suffix+" method") < cut) 	{continue;}
					}
					else //keep low BDT values (blind)
					{
						if(i_channel == 0 && reader->EvaluateMVA( BDT_type + "_uuu"+this->filename_suffix+" method") > cut) 		{continue;}
						else if(i_channel == 1 && reader->EvaluateMVA( BDT_type + "_uue"+this->filename_suffix+" method") > cut) 	{continue;}
						else if(i_channel == 2 && reader->EvaluateMVA( BDT_type + "_eeu"+this->filename_suffix+" method") > cut) 	{continue;}
						else if(i_channel == 3 && reader->EvaluateMVA( BDT_type + "_eee"+this->filename_suffix+" method") > cut) 	{continue;}
					}
				}

				tree_control->Fill();
			} //end event loop

			// --- Write histograms

			output_file->cd();

			//NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
			TString output_tree_name = "Control_" + sample_list[isample];
			if (syst_list[isyst] != "")
			{
				// if(combine_naming_convention) output_tree_name+= "_" + Combine_Naming_Convention(syst_list[isyst]);
				// else output_tree_name+= "_" + Theta_Naming_Convention(syst_list[isyst]);

				output_tree_name+= "_" + Theta_Naming_Convention(syst_list[isyst]);
			}

			tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);

			delete tree_control; delete tree;
			delete file_input;

			cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
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
void theMVAtool::Create_Control_Histograms(bool fakes_from_data, bool use_pseudodata_CR_plots, bool cut_on_BDT)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Create Control Histograms ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;
	if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
	else {cout<<"--- Using fakes from data ---"<<endl<<endl;}


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
	int control_binning = 20;
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


	bool create_syst_histos = false; //For now ,simply disactivate systematics for control plots (too long)
/*	if(syst_list.size() != 1)
	{
		string answer = "";
		cout<<FYEL("Do you want to create histograms for the systematics also ? This will increase")<<BOLD(FYEL(" A LOT "))<<FYEL("the processing time ! -- Type yes/no")<<endl;
		cin>>answer;

		while(answer != "yes" && answer!= "no")
		{
			cout<<"Wrong answer -- Type 'yes' (with syst.) or 'no' (without syst.) !"<<endl;
			cin.clear();
			cin.ignore(1000, '\n');
			cin>>answer;
		}

		if(answer == "yes") create_syst_histos = true;
	}*/

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
			//Info contained in tree leaves. Need to create histograms first
			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				if(use_pseudodata_CR_plots && sample_list[isample] == "Data") {continue;}

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					if(!create_syst_histos && syst_list[isyst] != "") {continue;} //If don't want to create histos for the systematics

					if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
					else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

					if( (sample_list[isample] == "Data" && syst_list[isyst]!="")
						|| ( (sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("TT") ) && syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") ) ) {continue;} //Data = no syst. -- Fakes = only fake syst.

					if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}

					if(syst_list[isyst].Contains("Zpt") || syst_list[isyst].Contains("Fake")) {continue;} //would need to rescale these systematics to nominal...

					if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

					if(syst_list[isyst].Contains("Fakes") && !sample_list[isample].Contains("Fakes") && !sample_list[isample].Contains("DY") && !sample_list[isample].Contains("TT") && !sample_list[isample].Contains("WW") ) {continue;}

					if(i_hist_created%100==0 && i_hist_created != 0) {cout<<"--- "<<i_hist_created<<"/"<<nof_histos_to_create<<endl;}


					TH1F* h_tmp = 0;
					if(total_var_list[ivar] == "mTW") 								{h_tmp = new TH1F( "","", binning, 0, 150 );}
					else if(total_var_list[ivar] == "METpt")						{h_tmp = new TH1F( "","", control_binning, 0, 200 );}
					else if(total_var_list[ivar] == "m3l")							{h_tmp = new TH1F( "","", control_binning, 0., 450. );}
					else if(total_var_list[ivar] == "ZMass") 						{h_tmp = new TH1F( "","", control_binning, 75, 105 );}
					else if(total_var_list[ivar] == "dPhiAddLepB") 					{h_tmp = new TH1F( "","", control_binning, -3.15, 3.15 );}
					else if(total_var_list[ivar] == "Zpt") 							{h_tmp = new TH1F( "","", control_binning, 0, 300 );}
					// else if(total_var_list[ivar] == "Zpt") 							{h_tmp = new TH1F( "","", control_binning, 0, 500 );}
					else if(total_var_list[ivar] == "ZEta")			 				{h_tmp = new TH1F( "","", control_binning, -4, 4 );}
					else if(total_var_list[ivar] == "AddLepAsym") 					{h_tmp = new TH1F( "","", control_binning, -2.5, 2.5 );}
					else if(total_var_list[ivar] == "mtop") 						{h_tmp = new TH1F( "","", control_binning, 60, 500 );}
					else if(total_var_list[ivar] == "btagDiscri") 					{h_tmp = new TH1F( "","", 22, -0.1, 1.2 );}
					else if(total_var_list[ivar] == "etaQ")							{h_tmp = new TH1F( "","", control_binning, -4.5, 4.5 );}
					else if(total_var_list[ivar] == "NBJets")						{h_tmp = new TH1F( "","", 5, 0, 5 );}
					else if(total_var_list[ivar] == "AddLepPT")						{h_tmp = new TH1F( "","", control_binning, 0, 200 );}
					else if(total_var_list[ivar] == "AddLepETA")					{h_tmp = new TH1F( "","", control_binning, -2.5, 2.5 );}
					else if(total_var_list[ivar] == "LeadJetPT")					{h_tmp = new TH1F( "","", control_binning, 0, 300 );}
					else if(total_var_list[ivar] == "LeadJetEta")					{h_tmp = new TH1F( "","", control_binning, -4.5, 4.5 );}
					else if(total_var_list[ivar] == "dPhiZMET")						{h_tmp = new TH1F( "","", control_binning, -3.15, 3.15 );}
					else if(total_var_list[ivar] == "dPhiZAddLep")					{h_tmp = new TH1F( "","", control_binning, -3.15,3.15 );}
					else if(total_var_list[ivar] == "dRAddLepB")					{h_tmp = new TH1F( "","", control_binning, 0, 5 );}
					else if(total_var_list[ivar] == "dRZAddLep")					{h_tmp = new TH1F( "","", control_binning, 0, 5 );}
					else if(total_var_list[ivar] == "dRZTop")						{h_tmp = new TH1F( "","", control_binning, 0, 5 );}
					else if(total_var_list[ivar] == "TopPT")						{h_tmp = new TH1F( "","", control_binning, 0, 500 );}
					else if(total_var_list[ivar] == "NJets")						{h_tmp = new TH1F( "","", 6, 0, 6 );}
					else if(total_var_list[ivar] == "ptQ")							{h_tmp = new TH1F( "","", control_binning, 30, 250 );}
					else if(total_var_list[ivar] == "dRjj")							{h_tmp = new TH1F( "","", control_binning, 0.5, 4.5 );}
					else if(total_var_list[ivar] == "AdditionalEleIso")				{h_tmp = new TH1F( "","", control_binning, 0, 0.5 );}
					else if(total_var_list[ivar] == "AdditionalMuonIso")			{h_tmp = new TH1F( "","", control_binning, 0, 0.5 );}
					else if(total_var_list[ivar] == "AddLepPT")						{h_tmp = new TH1F( "","", control_binning, 0, 150 );}
					else if(total_var_list[ivar] == "dRAddLepQ")					{h_tmp = new TH1F( "","", control_binning, 0, 5 );}
					else if(total_var_list[ivar] == "dRAddLepClosestJet")			{h_tmp = new TH1F( "","", control_binning, 0, 5 );}
					else if(total_var_list[ivar] == "tZq_pT")						{h_tmp = new TH1F( "","", control_binning, 0, 300 );}
					else if(total_var_list[ivar] == "tZq_mass")						{h_tmp = new TH1F( "","", control_binning, 0, 300 );}
					else if(total_var_list[ivar] == "leadingLeptonPT")              {h_tmp = new TH1F( "","", control_binning, 0, 150 );}
					else if(total_var_list[ivar] == "MAddLepB")                     {h_tmp = new TH1F( "","", control_binning, 0, 500 );}

					else if(total_var_list[ivar] == "MEMvar_0")						{h_tmp = new TH1F( "","", binning, 0, 50 );}
					else if(total_var_list[ivar] == "MEMvar_6")						{h_tmp = new TH1F( "","", binning, 0, 15 );}
					else if(total_var_list[ivar] == "MEMvar_1")						{h_tmp = new TH1F( "","", binning, -60, -30 );}
					else if(total_var_list[ivar] == "MEMvar_2")						{h_tmp = new TH1F( "","", binning, -80, -20 );}
					else if(total_var_list[ivar] == "MEMvar_3")						{h_tmp = new TH1F( "","", binning, 0, 50 );}
					else if(total_var_list[ivar] == "MEMvar_4")						{h_tmp = new TH1F( "","", binning, -60, 10 );}
					else if(total_var_list[ivar] == "MEMvar_5")						{h_tmp = new TH1F( "","", binning, -60, 10 );}
					else if(total_var_list[ivar] == "MEMvar_7")						{h_tmp = new TH1F( "","", binning, 0, 15 );}
					else if(total_var_list[ivar] == "MEMvar_8")						{h_tmp = new TH1F( "","", binning, 0, 35 );}
					else if(total_var_list[ivar] == "tZ_mass")						{h_tmp = new TH1F( "","", binning, 0, 1500 );}
					else if(total_var_list[ivar] == "tZ_pT")						{h_tmp = new TH1F( "","", binning, 0, 500 );}
					else if(total_var_list[ivar] == "bj_mass_leadingJet")			{h_tmp = new TH1F( "","", 20, 0, 1000 );}
					else if(total_var_list[ivar] == "bj_mass_subleadingJet")		{h_tmp = new TH1F( "","", 20, 0, 400 );}
					else if(total_var_list[ivar] == "bj_mass_leadingJet_pT40")		{h_tmp = new TH1F( "","", 20, 0, 1000 );}
					else if(total_var_list[ivar] == "bj_mass_leadingJet_pT50")		{h_tmp = new TH1F( "","", 20, 0, 1000 );}
					else if(total_var_list[ivar] == "bj_mass_leadingJet_pTlight40")		{h_tmp = new TH1F( "","", 20, 0, 1000 );}
					else if(total_var_list[ivar] == "bj_mass_leadingJet_pTlight50")		{h_tmp = new TH1F( "","", 20, 0, 1000 );}
					else if(total_var_list[ivar] == "bj_mass_leadingJet_etaCut")		{h_tmp = new TH1F( "","", 20, 0, 1000 );}
					else if(total_var_list[ivar] == "LeadingJetCSV")		{h_tmp = new TH1F( "","", 22, -0.1, 1.2 );}
					else if(total_var_list[ivar] == "SecondJetCSV")		{h_tmp = new TH1F( "","", 22, -0.1, 1.2 );}
					else if(total_var_list[ivar] == "LeadingJetNonB_pT")		{h_tmp = new TH1F( "","", binning, 0, 200 );}
					else if(total_var_list[ivar] == "SecondJetNonB_pT")		{h_tmp = new TH1F( "","", binning, 0, 200 );}
					else if(total_var_list[ivar] == "ContainsBadJet")		{h_tmp = new TH1F( "","", binning, 0, 1 );}

					// else if(total_var_list[ivar] == "")							{h_tmp = new TH1F( "","", control_binning, 0, 150 );}

					else {cout<<endl<<"theMVAtool.cc - l."<<__LINE__<<BOLD(FRED(" --> !!Unknown variable!! ")) << total_var_list[ivar] << " Correct name or add it here" <<endl<<endl; break;}

					TString tree_name = "Control_" + sample_list[isample];
					if(syst_list[isyst] != "") {tree_name+= "_" + syst_list[isyst];}
					if( !f_input->GetListOfKeys()->Contains(tree_name.Data()) && sample_list[isample] != "Data" )
					{
						// if(isyst==0 && ivar==0) {cout<<__LINE__<<" : "<<tree_name<<" not found (missing sample?) -- [Stop error messages]"<<endl;}
						cout<<__LINE__<<" : "<<tree_name<<" not found -- Check Control_Tree file content !"<<endl;
						continue;
					}
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

						if(channel_list[ichan] == "uuu" && i_channel!= 0) {continue;}
						else if(channel_list[ichan] == "uue" && i_channel!= 1) {continue;}
						else if(channel_list[ichan] == "eeu" && i_channel!= 2) {continue;}
						else if(channel_list[ichan] == "eee" && i_channel!= 3) {continue;}
						else if(channel_list[ichan] == "9") {cout<<__LINE__<<" : problem !"<<endl;}

						//Avoid to get overflow because of inappropriate binning --> Put it into last bin instead! (underflow in first bin)
						if(tmp < h_tmp->GetXaxis()->GetXmax() && tmp > h_tmp->GetXaxis()->GetXmin() ) {h_tmp->Fill(tmp, weight);}
						else if(tmp > h_tmp->GetXaxis()->GetXmax() ) {Fill_Last_Bin_TH1F(h_tmp, weight);} //overflow
						else if(tmp < h_tmp->GetXaxis()->GetXmin() ) {Fill_First_Bin_TH1F(h_tmp, weight);} //underflow
					}

					//Re-scale to desired luminosity, unless it's data
					if(sample_list[isample] != "Data")
					{
						h_tmp->Scale(luminosity_rescale);
					}


					TString output_histo_name = total_var_list[ivar] + "_" + channel_list[ichan] + "__";

					if(sample_list[isample]=="Data")
					{
						if(combine_naming_convention) output_histo_name+= "data_obs";
						else output_histo_name+= "DATA";
					}
					else output_histo_name+= sample_list[isample];

					if(syst_list[isyst] != "")
					{
						if(combine_naming_convention) output_histo_name+= "__" + Combine_Naming_Convention(syst_list[isyst]);
						else output_histo_name+= "__" + Theta_Naming_Convention(syst_list[isyst]);
					}

					f_output->cd();
					h_tmp->Write(output_histo_name.Data());
					// cout<<"Wrote "<<output_histo_name<<endl;

					delete h_tmp; delete tree; //Free memory

					i_hist_created++;
				} //end syst loop
			} //end sample loop
		} //end var loop
	} //end channel loop


	delete f_input;
	delete f_output;

	if(fakes_from_data) Rescale_Fake_Histograms(output_file_name);

	return ;
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
 * @param  allchannels           If true, sum 4 channels
 * @param  postfit           	 Decide if produce prefit OR postfit plots (NB : different files/hist names)
 // style requirements from https://twiki.cern.ch/twiki/bin/view/CMS/Internal/FigGuidelines
 */
int theMVAtool::Draw_Control_Plots(TString channel, bool fakes_from_data, bool allchannels, bool postfit, bool cut_on_BDT)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Draw Control Plots ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

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
		// delete f; //CHANGED
		// delete t_mu; //Free memory

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

	stringv_list.resize( total_var_list.size() );

//---------------------
//Loop on var > chan > sample > syst
//Retrieve histograms, stack, compare, plot
	for(int ivar=0; ivar<total_var_list.size(); ivar++)
	{
		TH1F *h_tmp = 0, *h_data = 0;

		vector<TH1F*> v_MC_histo; //Store separately the histos for each MC sample --> stack them after loops

		TLegend* qw = new TLegend(.85,.7,0.99,0.99);
		// qw->SetLineColor(0);
		// TLegend* qw = new TLegend(.2,.4,0.34,0.69); //CHANGED
		qw->SetShadowColor(0);
		qw->SetFillColor(0);
		qw->SetLineColor(1);

		//---------------------------
		//ERROR VECTORS INITIALIZATION
		//---------------------------
		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)

		//Only call this 'random' histogram here in order to get the binning used for the current variable --> can initialize the error vectors !
		//CHANGED
		if (postfit && !f->GetDirectory((total_var_list[ivar] + "_uuu_postfit").Data()) )
		{
			cout<<total_var_list[ivar].Data()<<"_uuu_postfit not found !"<<endl;

			continue;
		}

		TString histo_name = "";
		if(postfit) histo_name =  total_var_list[ivar] + "_uuu_postfit/tZqmcNLO"; //For postfit file only !
		else histo_name = total_var_list[ivar] + "_uuu__tZqmcNLO"; //For prefit file

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

			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				h_tmp = 0; //Temporary storage of histogram

				// cout<<total_var_list[ivar]<<" / "<<thechannellist[ichan]<<" / "<<sample_list[isample]<<" / "<<endl;

				if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
				else if(fakes_from_data && (sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only

				if((sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) && (sample_list[isample-1].Contains("DY") || sample_list[isample-1].Contains("TT") || sample_list[isample-1].Contains("WW") ) ) {continue;} //Draw MC fakes only once

				if(sample_list[isample] == "FakesElectron" && thechannellist[ichan] == "uuu")
				{
					if(allchannels && niter_chan==0)
					{
						MC_samples_legend.push_back(sample_list[isample]);

						v_MC_histo.push_back(h_tmp);
					}
					continue;
				}
				else if(sample_list[isample] == "FakesMuon" && thechannellist[ichan] == "eee")
				{
					if(allchannels && niter_chan==0)
					{
						MC_samples_legend.push_back(sample_list[isample]);

						v_MC_histo.push_back(h_tmp);
					}
					continue;
				}


				bool isData = false; if(sample_list[isample] == "Data") isData = true;

				TH1F* histo_nominal = 0; //Nominal histogram <-> syst == "" //Create it here because it's also used in the (syst != "") loop


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
					else histo_name+= sample_list[isample];
				}

				//NOTE -- CAREFUL, no protection against missing histogram here ! Possible segfault if wrong samplename
				//(reason : GetListOfKeys doesn't work for subdirectories in TFiles)

				h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();

				// cout<<"h_tmp = "<<h_tmp<<endl;
				if(!h_tmp) continue;

				if(!isData && (!allchannels || ichan==0)) MC_samples_legend.push_back(sample_list[isample]); //Fill vector containing existing MC samples names -- do it only once ==> because sample_list also contains data

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
					if(isample-1 < 0)  {cout<<__LINE__<<BOLD(FRED(" : Try to access wrong address (need at least 2 samples)! Exit !"))<<endl; return 0;}

					if(postfit && sample_list[isample].Contains("tZq")) {h_tmp->Scale(sig_strength);} //Need to rescale signal manually, not done by CombineHarvester !!

					//Use color vector filled in main()
					h_tmp->SetFillStyle(1001);
					h_tmp->SetFillColor(colorVector[isample-1]);
					h_tmp->SetLineColor(kBlack);
					if( (isample+1) < sample_list.size())
					{
						if( (sample_list[isample] == "ttH" || sample_list[isample] == "ttW") && (sample_list[isample+1] == "ttH" || sample_list[isample+1] == "ttW") ) {h_tmp->SetLineColor(colorVector[isample-1]);} //No black lines b/w 'ttW/H' samples //CHANGED
					}

					if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
					else
					{
						// if(isample-1 >= v_MC_histo.size()) {cout<<__LINE__<<BOLD(FRED(" : ERROR -- wrong number of samples !"))<<endl; continue;}
						// else v_MC_histo[isample-1]->Add(h_tmp);

						if(v_MC_histo[isample-1] == 0) v_MC_histo[isample-1] = (TH1F*) h_tmp->Clone();
						else v_MC_histo[isample-1]->Add(h_tmp);
					}

					histo_nominal = (TH1F*) h_tmp->Clone();

					for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
					{
						v_eyl[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.025, 2); //Lumi error = 2.5% //CHANGED
						v_eyh[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.025, 2);
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
					if(isData || ( (sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("DY") || sample_list[isample].Contains("TT") || sample_list[isample].Contains("WW") ) && !syst_list[isyst].Contains("Fakes") && !syst_list[isyst].Contains("Zpt")) ) {continue;}
					if(syst_list[isyst] == "") {continue;} //Already done
					if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}
					if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

					// if(postfit) cout<<BOLD(FRED("POSTFIT PLOTS : NO SYST YET ! CHECK NAMING CONVENTIONS FIRST !"))<<endl;

					TH1F* histo_syst = 0; //Store the "systematic histograms"

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

		THStack *stack = 0;
		TH1F* histo_total_MC = 0; //Sum of all MC samples

		//CHANGED -- ARC request : plot tZq signal above stack
		int index_tZq_sample = -99;

		//Stack all the MC nominal histograms (contained in v_MC_histo)
		for(int i=0; i<v_MC_histo.size(); i++)
		{
			if(MC_samples_legend[i].Contains("tZq"))
			{
				index_tZq_sample = i;
			}
			else
			{
				if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
				else {stack->Add(v_MC_histo[i]);}
			}

			if(histo_total_MC == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
			// if(i == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
			else {histo_total_MC->Add(v_MC_histo[i]);}
			//cout << "**********************  "<< histo_total_MC->Integral() << endl;


		}

		//Data on top of legend
		if(h_data != 0) {qw->AddEntry(h_data, "Data" , "ep");}
		else {cout<<__LINE__<<BOLD(FRED(" : h_data is null"))<<endl;}

		//Signal on top of all MC legends
		if(index_tZq_sample >= 0) //Put tZq on top
		{
			stack->Add(v_MC_histo[index_tZq_sample]);
			qw->AddEntry(v_MC_histo[index_tZq_sample], "tZq" , "f");
		}
		else {cout<<__LINE__<<" : no signal histogram !"<<endl;}

		//Add other legend entries -- iterate backwards, so that last histo stacked is on top of legend
		for(int i=v_MC_histo.size()-1; i>=0; i--)
		{
			if(MC_samples_legend[i] == "ttH") {continue;} //same entry as ttW
			if(MC_samples_legend[i].Contains("FakesMuon")) {continue;} //same entry as FakesElectron
			if(MC_samples_legend[i] == "ttW" ) {qw->AddEntry(v_MC_histo[i], "ttH+ttW" , "f");} //Single entry for ttW+ttH
			else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttZ" , "f");} //Single entry for ttZ
			else if(MC_samples_legend[i] == "WZL") {qw->AddEntry(v_MC_histo[i], "WZ+light" , "f");}
			else if(MC_samples_legend[i] == "WZB") {qw->AddEntry(v_MC_histo[i], "WZ+b" , "f");}
			else if(MC_samples_legend[i] == "WZC") {qw->AddEntry(v_MC_histo[i], "WZ+c" , "f");}
			else if(MC_samples_legend[i] == "ZZ") {qw->AddEntry(v_MC_histo[i], "ZZ" , "f");}
			else if(MC_samples_legend[i].Contains("Fakes") ) {qw->AddEntry(v_MC_histo[i], "Non-prompt" , "f");}
			else if(MC_samples_legend[i].Contains("DY") ) {qw->AddEntry(v_MC_histo[i], "Non-prompt (MC)" , "f");}
			else if(MC_samples_legend[i] == "WZC") {qw->AddEntry(v_MC_histo[i], "WZ+c" , "f");}
			else if(MC_samples_legend[i].Contains("ST") ) {qw->AddEntry(v_MC_histo[i], "tWZ" , "f");}
			else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV/H" , "f");} //Single entry for ttZ+ttW+ttH
		}

		if(!stack) {cout<<__LINE__<<BOLD(FRED(" : stack is null"))<<endl;}

		//---------------------------
		//DRAW HISTOS
		//---------------------------

		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
		c1->SetBottomMargin(0.3); //Leave space for ratio plot

		c1->cd();

		//Set Yaxis maximum & min
		if(h_data != 0 && stack != 0)
		{
			if(h_data->GetMaximum() > stack->GetMaximum() ) {stack->SetMaximum(h_data->GetMaximum()+0.3*h_data->GetMaximum());}
			else stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum());
		}
		stack->SetMinimum(0); //CHANGED
		// stack->SetMinimum(0.0001);

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


		//-------------------
		//CAPTIONS
		//-------------------

/*

//--- old graphics style ---

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
		float lumi = 35.9 * luminosity_rescale; //CHANGED
		TString lumi_ts = Convert_Number_To_TString(lumi);
		lumi_ts+= " fb^{-1} at #sqrt{s} = 13 TeV";
		latex2->DrawLatex(0.87, 0.95, lumi_ts.Data());
		//------------------
*/


// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines //CHANGED
		//
		TString cmsText     = "CMS";
		float cmsTextFont   = 61;  // default is helvetic-bold

		bool writeExtraText = false;
		TString extraText   = "Preliminary";
		float extraTextFont = 52;  // default is helvetica-italics

		// text sizes and text offsets with respect to the top frame
		// in unit of the top margin size
		float lumiTextSize     = 0.6;
		float lumiTextOffset   = 0.2;
		float cmsTextSize      = 0.75;
		float cmsTextOffset    = 0.1;  // only used in outOfFrame version

		float relPosX    = 0.045;
		float relPosY    = 0.035;
		float relExtraDY = 1.2;

		// ratio of "CMS" and extra text size
		float extraOverCmsTextSize  = 0.76;

		float lumi = 35.9 * luminosity_rescale; //CHANGED
		TString lumi_13TeV = Convert_Number_To_TString(lumi);
		lumi_13TeV += " fb^{-1} (13 TeV)";

		TLatex latex;
		latex.SetNDC();
		latex.SetTextAngle(0);
		latex.SetTextColor(kBlack);

		float H = c1->GetWh();
		float W = c1->GetWw();
		float l = c1->GetLeftMargin();
		float t = c1->GetTopMargin();
		float r = c1->GetRightMargin();
		float b = c1->GetBottomMargin();

		float extraTextSize = extraOverCmsTextSize*cmsTextSize;

		latex.SetTextFont(42);
		latex.SetTextAlign(31);
		latex.SetTextSize(lumiTextSize*t);
		//	Change position w.r.t. CMS recommendation, only for control plots
		//      latex.DrawLatex(1-r,1-t+lumiTextOffset*t,lumi_13TeV);
		// latex.DrawLatex(0.7,1-t+lumiTextOffset*t,lumi_13TeV);
		latex.DrawLatex(0.8,1-t+lumiTextOffset*t,lumi_13TeV); //CHANGED

		latex.SetTextFont(cmsTextFont);
		latex.SetTextAlign(11);
		latex.SetTextSize(cmsTextSize*t);
		latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);

		//float posX_ =   l +  relPosX*(1-l-r);
		//float posY_ =   1-t+lumiTextOffset*t;

		latex.SetTextFont(extraTextFont);
		//latex.SetTextAlign(align_);
		latex.SetTextSize(extraTextSize*t);
		latex.DrawLatex(l+cmsTextSize*l, 1-t+lumiTextOffset*t, extraText);


//------------------

		TString info_data;
		if (channel=="eee")    info_data = "eee channel";
		else if (channel=="eeu")  info_data = "ee#mu channel";
		else if (channel=="uue")  info_data = "#mu#mu e channel";
		else if (channel=="uuu") info_data = "#mu#mu #mu channel";
		else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";
/*
		TLatex* text2 = new TLatex(0.45,0.98, info_data);
		text2->SetNDC();
		text2->SetTextAlign(13);
		text2->SetX(0.18);
		text2->SetY(0.92);
		text2->SetTextFont(42);
		text2->SetTextSize(0.0610687);
		//text2->SetTextSizePixels(24);// dflt=28
		text2->Draw();
*/

		TString extrainfo_data;
		if(isWZ) {extrainfo_data= "3l,>0j,0bj";}
		else if(isttZ) {extrainfo_data= "3l,>1j,>1bj";}
		else if(!isWZ && !isttZ) {extrainfo_data= "3l,2-3j,1bj";}

		TLatex text2 ; //= new TLatex(0, 0, info_data);
		text2.SetNDC();
		text2.SetTextAlign(13);
		text2.SetTextFont(42);
		text2.SetTextSize(0.04);
		text2.DrawLatex(0.195,0.91,info_data);

		text2.SetTextFont(62);
		text2.DrawLatex(0.63,0.9,extrainfo_data);

		qw->Draw(); //Draw legend



		//--------------------------
		//SET VARIABLES LABELS
		//--------------------------


		if(total_var_list[ivar] == "mTW")                       {       stringv_list[ivar] = "M_{T}_{W} [GeV]"; }
		else if(total_var_list[ivar] == "METpt")                {       stringv_list[ivar] = "E_{T}^{miss} [GeV]"; }
		else if(total_var_list[ivar] == "m3l")          {       stringv_list[ivar] = "Trilepton mass [GeV]"; }
		else if(total_var_list[ivar] == "ZMass")                {       stringv_list[ivar] = "M_{Z} [GeV]"; }
		else if(total_var_list[ivar] == "dPhiAddLepB")  {       stringv_list[ivar] = "#Delta#Phi(l_{W},b)"; }
		else if(total_var_list[ivar] == "Zpt")          {       stringv_list[ivar] = "Z p_{T} [GeV]"; }
		else if(total_var_list[ivar] == "ZEta")         {       stringv_list[ivar] = "Z #eta"; }
		else if(total_var_list[ivar] == "AddLepAsym")   {       stringv_list[ivar] = " q_{#font[12]{l}}|#eta(#font[12]{l})|"; }
		else if(total_var_list[ivar] == "mtop")                 {       stringv_list[ivar] = "M_{top} [GeV]"; }
		else if(total_var_list[ivar] == "btagDiscri")   {       stringv_list[ivar] = "CSVv2 discriminant"; }
		else if(total_var_list[ivar] == "etaQ")         {       stringv_list[ivar] = "#eta(#font[12]{j'})"; }
		else if(total_var_list[ivar] == "NBJets")               {       stringv_list[ivar] = "B Jets multiplicity"; }
		else if(total_var_list[ivar] == "AddLepPT")             {       stringv_list[ivar] = "l_{W} p_{T} [GeV]"; }
		else if(total_var_list[ivar] == "AddLepETA")            {       stringv_list[ivar] = "l_{W} #eta"; }
		else if(total_var_list[ivar] == "LeadJetPT")            {       stringv_list[ivar] = "p_{T}(#font[12]{j'}) [GeV]"; }
		else if(total_var_list[ivar] == "LeadJetEta")           {       stringv_list[ivar] = "Leading jet #eta"; }
		else if(total_var_list[ivar] == "dPhiZMET")             {       stringv_list[ivar] = "#Delta#Phi(Z,E_{T}^{miss})"; }
		else if(total_var_list[ivar] == "dPhiZAddLep")  {       stringv_list[ivar] = "#Delta#Phi(Z,l_{W})"; }
		else if(total_var_list[ivar] == "dRAddLepB")            {       stringv_list[ivar] = "#Delta R(b,l_{W})"; }
		else if(total_var_list[ivar] == "dRZAddLep")            {       stringv_list[ivar] = "#Delta R(Z,l_{W})"; }
		else if(total_var_list[ivar] == "dRZTop")               {       stringv_list[ivar] = "#Delta R(Z,top)"; }
		else if(total_var_list[ivar] == "TopPT")                {       stringv_list[ivar] = "top p_{T} [GeV]"; }
		else if(total_var_list[ivar] == "NJets")                {       stringv_list[ivar] = "Jets multiplicity"; }
		else if(total_var_list[ivar] == "ptQ")          {       stringv_list[ivar] = "p_{T}(#font[12]{j'}) [GeV]"; }
		else if(total_var_list[ivar] == "dRjj")         {       stringv_list[ivar] = "#Delta R(b,#font[12]{j'})"; }
		else if(total_var_list[ivar] == "AdditionalEleIso")     {       stringv_list[ivar] = "e_{W} isolation"; }
		else if(total_var_list[ivar] == "AdditionalMuonIso")    {       stringv_list[ivar] = "#mu_{W} isolation"; }
		else if(total_var_list[ivar] == "dRAddLepQ")            {       stringv_list[ivar] = "#Delta R(l_{W},#font[12]{j'})"; }
		else if(total_var_list[ivar] == "dRAddLepClosestJet")   {       stringv_list[ivar] = "#Delta R(closest jet, l_{W})"; }
		else if(total_var_list[ivar] == "tZq_pT")               {       stringv_list[ivar] = "tZq system p_{T} [GeV]"; }
		else if(total_var_list[ivar] == "tZq_mass")             {       stringv_list[ivar] = "tZq system mass [GeV]"; }
		else if(total_var_list[ivar] == "leadingLeptonPT")    {       stringv_list[ivar] = "Leading lepton p_{T} [GeV]"; }
		else if(total_var_list[ivar] == "MAddLepB")           {       stringv_list[ivar] = "l_{W}+b system mass "; }
		else if(total_var_list[ivar] == "MEMvar_0")             {       stringv_list[ivar] = "MEM LR tZq-ttZ"; }
		else if(total_var_list[ivar] == "MEMvar_1")             {       stringv_list[ivar] = "MEM Kin w_{tZq}"; }
		else if(total_var_list[ivar] == "MEMvar_2")             {       stringv_list[ivar] = "MEM Kin w_{ttZ}"; }
		else if(total_var_list[ivar] == "MEMvar_3")             {       stringv_list[ivar] = "MEM LR tZq-ttZ"; }
		else if(total_var_list[ivar] == "MEMvar_4")             {       stringv_list[ivar] = "MEM w_{WZ}"; }
		else if(total_var_list[ivar] == "MEMvar_5")             {       stringv_list[ivar] = "MEM Kin w_{WZ}"; }
		else if(total_var_list[ivar] == "MEMvar_6")             {       stringv_list[ivar] = "MEM LR tZq-ttZ-WZ"; }
		else if(total_var_list[ivar] == "MEMvar_7")             {       stringv_list[ivar] = "MEM LR tZq-WZ"; }
		else if(total_var_list[ivar] == "MEMvar_8")             {       stringv_list[ivar] = "MEM LR tZq-ttZ-WZ"; }
		else if(total_var_list[ivar] == "tZ_mass")             {       stringv_list[ivar] = "M_{tZ}"; }
		else if(total_var_list[ivar] == "tZ_pT")             {       stringv_list[ivar] = "tZ_{p_{T}} [GeV]"; }
		else {       stringv_list[ivar] = total_var_list[ivar] ; }



		//--------------------------
		//DRAW DATA/MC RATIO
		//--------------------------

		//Create Data/MC ratio plot (bottom of canvas)
		if(h_data == 0 || v_MC_histo.size() == 0) //Need both data and MC
		{
			cout<<"Problem : h_data = "<<h_data<<", v_MC_histo.size() = "<<v_MC_histo.size()<<endl;
			return 0;
		}


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

		// histo_ratio_data->GetXaxis()->SetTitle(total_var_list[ivar].Data());
		histo_ratio_data->GetXaxis()->SetTitle(stringv_list[ivar].Data());
		histo_ratio_data->GetYaxis()->SetTickLength(0.15);

		histo_ratio_data->GetXaxis()->SetTitleOffset(1.4);
		histo_ratio_data->GetYaxis()->SetTitleOffset(1.42);
		histo_ratio_data->GetXaxis()->SetLabelSize(0.05);
		histo_ratio_data->GetYaxis()->SetLabelSize(0.048);
		// histo_ratio_data->GetYaxis()->SetLabelSize(0.048);
		histo_ratio_data->GetXaxis()->SetLabelFont(42);
		histo_ratio_data->GetYaxis()->SetLabelFont(42);
		histo_ratio_data->GetXaxis()->SetTitleFont(42);
		histo_ratio_data->GetYaxis()->SetTitleFont(42);
		// histo_ratio_data->GetYaxis()->SetNdivisions(6);
		histo_ratio_data->GetYaxis()->SetNdivisions(503);
		// histo_ratio_data->GetYaxis()->SetNdivisions(303);
		histo_ratio_data->GetYaxis()->SetTitleSize(0.04);

		histo_ratio_data->GetYaxis()->SetTitle("Data/Prediction");

		histo_ratio_data->SetMinimum(0.0);
		histo_ratio_data->SetMaximum(1.999);
		histo_ratio_data->Draw("E1X0"); //Draw ratio points

		TH1F *h_line1 = new TH1F("","",this->nbin, h_data->GetXaxis()->GetXmin(), h_data->GetXaxis()->GetXmax());
		TH1F *h_line2 = new TH1F("","",this->nbin, h_data->GetXaxis()->GetXmin(), h_data->GetXaxis()->GetXmax());
		TH1F *h_line3 = new TH1F("","",this->nbin, h_data->GetXaxis()->GetXmin(), h_data->GetXaxis()->GetXmax());

		for(int ibin=1; ibin<this->nbin +1; ibin++)
		{
			h_line1->SetBinContent(ibin, 0.5);
			h_line2->SetBinContent(ibin, 1.5);
			h_line3->SetBinContent(ibin, 2.5);
		}

		h_line1->SetLineStyle(3);
		h_line2->SetLineStyle(3);
		h_line3->SetLineStyle(3);

		h_line1->Draw("hist same");
		h_line2->Draw("hist same");
		h_line3->Draw("hist same");




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

		//Yaxis title
		// if(stack!= 0) {stack->GetYaxis()->SetTitleSize(0.04); stack->GetYaxis()->SetTitle("Events");}
		if(stack!= 0)
		{
			stack->GetXaxis()->SetLabelFont(42);
			stack->GetYaxis()->SetLabelFont(42);
			stack->GetYaxis()->SetTitleFont(42);
			stack->GetYaxis()->SetTitleSize(0.045);
			stack->GetYaxis()->SetTickLength(0.04);
			stack->GetXaxis()->SetLabelSize(0.0);
			stack->GetYaxis()->SetLabelSize(0.048);
			stack->GetYaxis()->SetTitle("Events");
		}

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

		//Iximumge name
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


		delete thegraph_ratio;
		delete h_line1; delete h_line2; delete h_line3;
		delete thegraph_tmp;


		delete c1; //Must free dinamically-allocated memory
		// delete canvas_2;
		delete qw; delete stack; delete gr;

		delete h_tmp; delete h_data;
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

//CHANGED
// Mara: 25/April/2017 -- this method was modified to cope with the
// style requirements from https://twiki.cern.ch/twiki/bin/view/CMS/Internal/FigGuidelines
// and from the ARC

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Plot stacked MC templates .vs. Data PREFIT (from Combine !input! file) NB : systematics are not accounted for
 * @param  channel       Channel name
 * @param  template_name Template name : BDT / BDTttZ / mTW
 * @param  allchannels   If true, sum all channels
 */
int theMVAtool::Plot_Prefit_Templates(TString channel, TString template_name, bool allchannels, bool cut_on_BDT)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	// TString input_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root";
	TString input_name = "outputs/Combine_Input.root"; //FAKES RE-SCALED
	if(cut_on_BDT) input_name = "outputs/Combine_Input_CutBDT.root";
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

	vector<TString> MC_samples_legend; //List the MC samples which are actually used (to get correct legend)

	// TLegend* qw = new TLegend(.81,.63,.95,.91);
	// TLegend* qw = new TLegend(.85,.7,0.965,.915); //CHANGED
	// qw->SetShadowColor(0);
	// qw->SetFillColor(0);
	// qw->SetLineColor(0);
	TLegend* qw = new TLegend(.82,.65,.99,.99);
	qw->SetShadowColor(0);
	qw->SetFillColor(0);
	qw->SetLineColor(1); //CHANGED


	int niter_chan = 0;
	for(int ichan=0; ichan<thechannellist.size(); ichan++)
	{
		if(!allchannels && channel != thechannellist[ichan]) {continue;}

	//--- All MC samples but fakes
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			if(sample_list[isample].Contains("Data") ) {continue;}

			h_tmp = 0;

			//WARNING : in uuu channel --> FakesElectron histo will be null ; idem for FakesMuon in eee channel
			if(sample_list[isample] == "FakesElectron" && thechannellist[ichan] == "uuu")
			{
				if(allchannels && niter_chan==0)
				{
					v_MC_histo.push_back(h_tmp);
					MC_samples_legend.push_back(sample_list[isample]);
				}
				continue;
			}
			else if(sample_list[isample] == "FakesMuon" && thechannellist[ichan] == "eee")
			{
				if(allchannels && niter_chan==0)
				{
					v_MC_histo.push_back(h_tmp);
					MC_samples_legend.push_back(sample_list[isample]);
				}
				continue;
			}

			TString histo_name = template_name + "_" + thechannellist[ichan] + "__" + sample_list[isample];

			if(!file_input->GetListOfKeys()->Contains(histo_name.Data()) && !sample_list[isample].Contains("Fakes") && !sample_list[isample].Contains("DY") && !sample_list[isample].Contains("TT") && !sample_list[isample].Contains("WW") ) {cout<<histo_name<<" : not found"<<endl; continue;}
			else
			{
				h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();

				if(!sample_list[isample].Contains("Data") && (!allchannels || ichan==0)) MC_samples_legend.push_back(sample_list[isample]); //Fill vector containing existing MC samples names -- do it only once ==> because sample_list also contains data

				//Use color vector filled in main() (use -1 because first sample should be data)
				h_tmp->SetFillStyle(1001);
				h_tmp->SetFillColor(colorVector[isample-1]);
				h_tmp->SetLineColor(kBlack);
				if( (isample+1) < sample_list.size())
				{
					if( (sample_list[isample] == "ttH" || sample_list[isample] == "ttW") && (sample_list[isample+1] == "ttH" || sample_list[isample+1] == "ttW") ) {h_tmp->SetLineColor(colorVector[isample-1]);} //No black lines b/w 'ttW/H' samples //CHANGED
				}

				// cout<<"isample-1 "<<isample-1<<endl;
				// cout<<"v_MC_histo "<<v_MC_histo.size()<<endl;
				if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
				else if(v_MC_histo[isample-1]==0) {v_MC_histo[isample-1] = (TH1F*) h_tmp->Clone();} //For FakeEle and FakeMu
				else {v_MC_histo[isample-1]->Add(h_tmp);}
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


	// cout<<v_MC_histo[9]<<endl;
	// cout<<v_MC_histo[10]<<endl;
	// cout<<v_MC_histo.size()<<endl;

	//Make sure there are no negative bins
	for(int ibin = 1; ibin<h_sum_data->GetNbinsX()+1; ibin++)
	{
		for(int k=0; k<v_MC_histo.size(); k++)
		{
			if(!v_MC_histo[k]) {continue;} //Fakes templates can be null
			if(v_MC_histo[k]->GetBinContent(ibin) < 0) {v_MC_histo[k]->SetBinContent(ibin, 0);}
		}
		if(h_sum_data->GetBinContent(ibin) < 0) {h_sum_data->SetBinContent(ibin, 0);}
	}

	//--- CREATE STACK
	THStack* stack_MC = 0;
	TH1F* histo_total_MC = 0; //Sum of all MC samples

	//CHANGED -- ARC request : plot tZq signal above stack
	int index_tZq_sample = -99;

	//Stack all the MC nominal histograms (contained in v_MC_histo)
	for(int i=0; i<v_MC_histo.size(); i++)
	{
		if(!v_MC_histo[i]) {continue;} //Fakes templates can be null

		if(MC_samples_legend[i].Contains("tZq"))
		{
			index_tZq_sample = i;
		}
		else
		{
			if(stack_MC == 0) {stack_MC = new THStack; stack_MC->Add(v_MC_histo[i]);}
			else {stack_MC->Add(v_MC_histo[i]);}
		}

		if(histo_total_MC == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
		// if(i == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
		else {histo_total_MC->Add(v_MC_histo[i]);}
	}

	//Data on top of legend
	if(h_sum_data != 0) {qw->AddEntry(h_sum_data, "Data" , "ep");}
	else {cout<<__LINE__<<BOLD(FRED(" : h_sum_data is null"))<<endl;}

	//Signal on top of all MC legends
	if(index_tZq_sample >= 0) //Put tZq on top
	{
		stack_MC->Add(v_MC_histo[index_tZq_sample]);
		qw->AddEntry(v_MC_histo[index_tZq_sample], "tZq" , "f");
	}
	else {cout<<__LINE__<<" : no signal histogram !"<<endl;}

	//Add other legend entries -- iterate backwards, so that last histo stacked is on top of legend
	for(int i=v_MC_histo.size()-1; i>=0; i--)
	{
		if(!v_MC_histo[i]) {continue;} //Fakes templates can be null

		if(MC_samples_legend[i] == "ttH") {continue;}
		else if(MC_samples_legend[i].Contains("FakesMuon") && (allchannels || channel != "uuu") ) {continue;} //depends on cases
		else if(MC_samples_legend[i] == "ttW") {qw->AddEntry(v_MC_histo[i], "ttH+ttW" , "f");} //Single entry for ttW+ttH
		else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttZ" , "f");} //Single entry for ttZ
		else if(MC_samples_legend[i] == "WZL") {qw->AddEntry(v_MC_histo[i], "WZ+light" , "f");}
		else if(MC_samples_legend[i] == "WZB") {qw->AddEntry(v_MC_histo[i], "WZ+b" , "f");}
		else if(MC_samples_legend[i] == "WZC") {qw->AddEntry(v_MC_histo[i], "WZ+c" , "f");}
		else if(MC_samples_legend[i] == "ZZ") {qw->AddEntry(v_MC_histo[i], "ZZ" , "f");}
		else if(MC_samples_legend[i].Contains("Fakes") ) {qw->AddEntry(v_MC_histo[i], "Non-prompt" , "f");}
		else if(MC_samples_legend[i].Contains("DY") ) {qw->AddEntry(v_MC_histo[i], "Non-prompt (MC)" , "f");}
		else if(MC_samples_legend[i] == "WZC") {qw->AddEntry(v_MC_histo[i], "WZ+c" , "f");}
		else if(MC_samples_legend[i].Contains("ST") ) {qw->AddEntry(v_MC_histo[i], "tWZ" , "f");}
		else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV/H" , "f");} //Single entry for ttZ+ttW+ttH
	}

	//Set Yaxis maximum & minimum
	if(h_sum_data->GetMaximum() > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
	else stack_MC->SetMaximum(stack_MC->GetMaximum()+0.3*stack_MC->GetMaximum());
	stack_MC->SetMinimum(0);
	// stack_MC->SetMinimum(0.0001);


	//Canvas definition
	Load_Canvas_Style();
	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
	c1->SetBottomMargin(0.3);

	//w_caw stack & data
	stack_MC->Draw("hist");
	h_sum_data->SetMarkerStyle(20);
	h_sum_data->SetMinimum(0.) ;
	h_sum_data->Draw("e0psame");

	TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
	canvas_2->SetTopMargin(0.7);
	canvas_2->SetFillColor(0);
	canvas_2->SetFillStyle(0);
	canvas_2->SetGridy(1);
	canvas_2->Draw();
	canvas_2->cd(0);

	//CHANGED
	TH1F * histo_ratio_data = (TH1F*) h_sum_data->Clone();
	histo_ratio_data->Divide(histo_total_MC); //Ratio

	histo_ratio_data->GetXaxis()->SetTitleOffset(1.4);
	histo_ratio_data->GetYaxis()->SetTitleOffset(1.42);
	histo_ratio_data->GetXaxis()->SetLabelSize(0.05);
	histo_ratio_data->GetYaxis()->SetLabelSize(0.048);
	histo_ratio_data->GetXaxis()->SetLabelFont(42);
	histo_ratio_data->GetYaxis()->SetLabelFont(42);
	histo_ratio_data->GetXaxis()->SetTitleFont(42);
	histo_ratio_data->GetYaxis()->SetTitleFont(42);
	histo_ratio_data->GetYaxis()->SetNdivisions(503);
	histo_ratio_data->GetXaxis()->SetTitleSize(0.04);
	histo_ratio_data->GetYaxis()->SetTitleSize(0.04);
	histo_ratio_data->GetYaxis()->SetTickLength(0.15);
	histo_ratio_data->GetYaxis()->SetTitle("Data/Prediction");
	histo_ratio_data->SetMinimum(0.0);
	histo_ratio_data->SetMaximum(1.999);


	if (template_name == "BDT" ) histo_ratio_data->GetXaxis()->SetTitle("BDT output (1 tag)") ;
	else if ( template_name == "BDTttZ" ) histo_ratio_data->GetXaxis()->SetTitle("BDT output (2 tags)");
	else if ( template_name == "mTW")  histo_ratio_data->GetXaxis()->SetTitle("m_{T}^{W} [GeV]");


	histo_ratio_data->Draw("E1X0"); //Draw ratio points

	TH1F *h_line1 = new TH1F("","",this->nbin, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
	TH1F *h_line2 = new TH1F("","",this->nbin, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
	TH1F *h_line3 = new TH1F("","",this->nbin, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());

	for(int ibin=1; ibin<this->nbin +1; ibin++)
	{
		h_line1->SetBinContent(ibin, 0.5);
		h_line2->SetBinContent(ibin, 1.5);
		h_line3->SetBinContent(ibin, 2.5);
	}

	h_line1->SetLineStyle(3);
	h_line2->SetLineStyle(3);
	h_line3->SetLineStyle(3);

	h_line1->Draw("hist same");
	h_line2->Draw("hist same");
	h_line3->Draw("hist same");



	// stack_MC->GetXaxis()->SetTitle(template_name.Data());
	stack_MC->GetXaxis()->SetLabelFont(42);
	stack_MC->GetYaxis()->SetLabelFont(42);
	stack_MC->GetYaxis()->SetTitleFont(42);
	stack_MC->GetYaxis()->SetTitleSize(0.045);
	stack_MC->GetYaxis()->SetTickLength(0.04);
	stack_MC->GetXaxis()->SetLabelSize(0.0);
	stack_MC->GetYaxis()->SetLabelSize(0.048);
	stack_MC->GetYaxis()->SetTitle("Events");
	c1->Modified();


// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines

	TString cmsText     = "CMS";
	float cmsTextFont   = 61;  // default is helvetic-bold

	bool writeExtraText = false;
	TString extraText   = "Preliminary";
	float extraTextFont = 52;  // default is helvetica-italics

	// text sizes and text offsets with respect to the top frame
	// in unit of the top margin size
	float lumiTextSize     = 0.6;
	float lumiTextOffset   = 0.2;
	float cmsTextSize      = 0.75;
	float cmsTextOffset    = 0.1;  // only used in outOfFrame version

	float relPosX    = 0.045;
	float relPosY    = 0.035;
	float relExtraDY = 1.2;

	// ratio of "CMS" and extra text size
	float extraOverCmsTextSize  = 0.76;

	float lumi = 35.9 * luminosity_rescale; //CHANGED
	TString lumi_13TeV = Convert_Number_To_TString(lumi);
	lumi_13TeV += " fb^{-1} (13 TeV)";

	TLatex latex;
	latex.SetNDC();
	latex.SetTextAngle(0);
	latex.SetTextColor(kBlack);
	float H = c1->GetWh();
	float W = c1->GetWw();
	float l = c1->GetLeftMargin();
	float t = c1->GetTopMargin();
	float r = c1->GetRightMargin();
	float b = c1->GetBottomMargin();
	float extraTextSize = extraOverCmsTextSize*cmsTextSize;
	latex.SetTextFont(42);
	latex.SetTextAlign(31);
	latex.SetTextSize(lumiTextSize*t);
	latex.DrawLatex(0.8,1-t+lumiTextOffset*t,lumi_13TeV);
	latex.SetTextFont(cmsTextFont);
	latex.SetTextAlign(11);
	latex.SetTextSize(cmsTextSize*t);
	latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);
	latex.SetTextFont(extraTextFont);
	latex.SetTextSize(extraTextSize*t);
	latex.DrawLatex(l+cmsTextSize*l, 1-t+lumiTextOffset*t, extraText);


	//------------------


	TString info_data;
	if (channel=="eee")    info_data = "eee channel";
	else if (channel=="eeu")  info_data = "ee#mu channel";
	else if (channel=="uue")  info_data = "#mu#mu e channel";
	else if (channel=="uuu") info_data = "#mu#mu #mu channel";
	else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";

/*
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
*/

	TString extrainfo_data;
	if(isWZ) {extrainfo_data= "3l,>0j,0bj";}
	else if(isttZ) {extrainfo_data= "3l,>1j,>1bj";}
	else if(!isWZ && !isttZ) {extrainfo_data= "3l,2-3j,1bj";}

	TLatex text2 ; //= new TLatex(0, 0, info_data);
	text2.SetNDC();
	text2.SetTextAlign(13);
	text2.SetTextFont(42);
	text2.SetTextSize(0.04);
	text2.DrawLatex(0.195,0.91,info_data);

	text2.SetTextFont(62);
	text2.DrawLatex(0.63,0.9,extrainfo_data);
	//------------------


	qw->Draw("same");

	mkdir("plots",0777); //Create directory if inexistant
	mkdir("plots/templates",0777); //Create directory if inexistant
	mkdir("plots/templates/prefit",0777); //Create directory if inexistant

	//Output
	TString output_plot_name = "plots/templates/prefit/" + template_name +"_template_" + channel+ this->filename_suffix + this->format;
	if(channel == "" || allchannels) {output_plot_name = "plots/templates/prefit/" + template_name +"_template_all" + this->filename_suffix + this->format;}

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
int theMVAtool::Plot_Postfit_Templates(TString channel, TString template_name, bool allchannels, bool cut_on_BDT)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots from Combine Output ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	//In mlfit.root, data is given as a TGraph. Instead, we take it directly from the Template file given to Combine (un-changed!)
	TString input_name = "outputs/Combine_Input.root"; //FAKES RE-SCALED
	if(cut_on_BDT) input_name = "outputs/Combine_Input_CutBDT.root";
	TFile* file_data = 0;
	file_data = TFile::Open( input_name.Data() );
	if(file_data == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}

	input_name = "outputs/mlfit.root";
	TFile* file_input = 0;
	file_input = TFile::Open( input_name.Data() );
	if(file_input == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}


	mkdir("plots",0777);
	TH1::SetDefaultSumw2();

	TH1F *h_tmp = 0, *h_sum_data = 0;
	vector<TH1F*> v_MC_histo;

	vector<TString> MC_samples_legend; //List the MC samples which are actually used (to get correct legend)

	// TLegend* qw = new TLegend(.85,.7,0.965,.915);
	// qw->SetShadowColor(0);
	// qw->SetFillColor(0);
	// qw->SetLineColor(0);
	TLegend* qw = new TLegend(.82,.65,.99,.99); //CHANGED
   	qw->SetShadowColor(0);
   	qw->SetFillColor(0);
   	qw->SetLineColor(1);


	int niter_chan = 0;
	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		if(!allchannels && channel != channel_list[ichan]) {continue;}

	//--- All MC samples but fakes
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			bool isData = false;
			if(sample_list[isample].Contains("Data") ) {isData = true;}

			if(sample_list[isample] == "FakesElectron" && channel_list[ichan] == "uuu") {continue;}
			else if(sample_list[isample] == "FakesMuon" && channel_list[ichan] == "eee") {continue;}


			//--- DATA -- different file
			if(isData)
			{
				h_tmp = 0;
				TString histo_name;
				if(combine_naming_convention) histo_name = template_name + "_" + channel_list[ichan] + "__data_obs"; //Combine
				else histo_name = template_name + "_" + channel_list[ichan] + "__DATA"; //Theta

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
				TString dir_name = "shapes_fit_s/" + template_name + "_" + channel_list[ichan] + "/";

				h_tmp = 0;
				TString histo_name = sample_list[isample];

				h_tmp = (TH1F*) file_input->Get((dir_name+histo_name).Data())->Clone();
				if(!h_tmp) {cout<<__LINE__<<" : h_tmp is null ! "<<endl;}

				if(!sample_list[isample].Contains("Data") && (!allchannels || ichan==0)) MC_samples_legend.push_back(sample_list[isample]); //Fill vector containing existing MC samples names -- do it only once ==> because sample_list also contains data

				//Use color vector filled in main() (use -1 because first sample should be data)
				h_tmp->SetFillStyle(1001);
				h_tmp->SetFillColor(colorVector[isample-1]);
				h_tmp->SetLineColor(kBlack);
				if( (isample+1) < sample_list.size())
				{
					if( (sample_list[isample] == "ttH" || sample_list[isample] == "ttW") && (sample_list[isample+1] == "ttH" || sample_list[isample+1] == "ttW") ) {h_tmp->SetLineColor(colorVector[isample-1]);} //No black lines b/w 'ttW/H' samples //CHANGED
				}

				cout<<"sample "<<sample_list[isample]<<" / chan "<<channel_list[ichan]<<endl;


				if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
				else {v_MC_histo[isample-1]->Add(h_tmp);}
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


	//Canvas definition
	Load_Canvas_Style();
	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
	c1->SetBottomMargin(0.3);


	//Stack all the MC nominal histograms (contained in v_MC_histo)
	THStack* stack_MC = 0;
	TH1F* histo_total_MC = 0; //Sum of all MC samples

	//CHANGED -- ARC request : plot tZq signal above stack
	int index_tZq_sample = -99;

	//Stack all the MC nominal histograms (contained in v_MC_histo)
	for(int i=0; i<v_MC_histo.size(); i++)
	{
		if(MC_samples_legend[i].Contains("tZq"))
		{
			index_tZq_sample = i;
		}
		else
		{
			if(stack_MC == 0) {stack_MC = new THStack; stack_MC->Add(v_MC_histo[i]);}
			else {stack_MC->Add(v_MC_histo[i]);}
		}

		if(histo_total_MC == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
		// if(i == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
		else {histo_total_MC->Add(v_MC_histo[i]);}
	}

	stack_MC->Draw("hist"); //NOTE : need to draw stack here to access its Xaxis ; tZq not included yet (for ordering), so will redraw it after

	//--- Need to transform data histogram so that it's x-axis complies with combine's one
	double xmax_stack = stack_MC->GetXaxis()->GetXmax();
	double xmin_stack = stack_MC->GetXaxis()->GetXmin();
	double xmax_data = h_sum_data->GetXaxis()->GetXmax();
	double xmin_data = h_sum_data->GetXaxis()->GetXmin();

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

	qw->AddEntry(h_data_new, "Data" , "ep");

	//Signal on top of all MC legends
	if(index_tZq_sample >= 0) //Put tZq on top
	{
		stack_MC->Add(v_MC_histo[index_tZq_sample]);
		qw->AddEntry(v_MC_histo[index_tZq_sample], "tZq" , "f");
	}
	else {cout<<__LINE__<<" : no signal histogram !"<<endl;}

	//Add other legend entries -- iterate backwards, so that last histo stacked is on top of legend
	for(int i=v_MC_histo.size()-1; i>=0; i--)
	{
		if(MC_samples_legend[i] == "ttH") {continue;} //same entry as ttW
		else if(MC_samples_legend[i].Contains("FakesMuon") && (allchannels || channel != "uuu") ) {continue;} //same entry as FakesElectron
		else if(MC_samples_legend[i] == "ttW" ) {qw->AddEntry(v_MC_histo[i], "ttH+ttW" , "f");} //Single entry for ttW+ttH
		else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttZ" , "f");} //Single entry for ttZ
		else if(MC_samples_legend[i] == "WZL") {qw->AddEntry(v_MC_histo[i], "WZ+light" , "f");}
		else if(MC_samples_legend[i] == "WZB") {qw->AddEntry(v_MC_histo[i], "WZ+b" , "f");}
		else if(MC_samples_legend[i] == "WZC") {qw->AddEntry(v_MC_histo[i], "WZ+c" , "f");}
		else if(MC_samples_legend[i] == "ZZ") {qw->AddEntry(v_MC_histo[i], "ZZ" , "f");}
		else if(MC_samples_legend[i].Contains("Fakes") ) {qw->AddEntry(v_MC_histo[i], "Non-prompt" , "f");}
		else if(MC_samples_legend[i] == "WZC") {qw->AddEntry(v_MC_histo[i], "WZ+c" , "f");}
		else if(MC_samples_legend[i].Contains("ST") ) {qw->AddEntry(v_MC_histo[i], "tWZ" , "f");}
		else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "ttV/H" , "f");} //Single entry for ttZ+ttW+ttH
	}


	//Set Yaxis maximum & minimum
	if(h_sum_data->GetMaximum() > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
	else stack_MC->SetMaximum(stack_MC->GetMaximum()+0.3*stack_MC->GetMaximum());
	// stack_MC->SetMinimum(0.0001);


	//Draw stack & data
	stack_MC->Draw("hist"); //Re-draw
	c1->Modified();

	h_data_new->SetMarkerStyle(20);
	h_data_new->Draw("epsame");

	TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
	canvas_2->SetTopMargin(0.7);
	canvas_2->SetFillColor(0);
	canvas_2->SetFillStyle(0);
	canvas_2->SetGridy(1);
	canvas_2->Draw();
	canvas_2->cd(0);

	TH1F * histo_ratio_data = (TH1F*) h_data_new->Clone();

	cout<<"data bins = "<<histo_ratio_data->GetNbinsX()<<endl;
	cout<<"histo_total_MC bins = "<<histo_total_MC->GetNbinsX()<<endl;

	cout<<__LINE__<<endl;
	histo_ratio_data->Divide(histo_total_MC); //Ratio
	cout<<__LINE__<<endl;


	//NOTE : slightly different parameters than prefit function (need to add hand-made axis here)
	histo_ratio_data->GetXaxis()->SetLabelSize(0.0); //Make this axis invisible because we are going to draw a new one (below)
	histo_ratio_data->GetXaxis()->SetLabelOffset(999);
	histo_ratio_data->GetXaxis()->SetTickLength(0.0);


	histo_ratio_data->GetXaxis()->SetTitleOffset(1.4);
	histo_ratio_data->GetYaxis()->SetTitleOffset(1.42);
	histo_ratio_data->GetYaxis()->SetLabelSize(0.048);
	histo_ratio_data->GetXaxis()->SetLabelFont(42);
	histo_ratio_data->GetYaxis()->SetLabelFont(42);
	histo_ratio_data->GetXaxis()->SetTitleFont(42);
	histo_ratio_data->GetYaxis()->SetTitleFont(42);
	histo_ratio_data->GetYaxis()->SetNdivisions(503);
	histo_ratio_data->GetYaxis()->SetTitleSize(0.04);
	histo_ratio_data->GetYaxis()->SetTickLength(0.15);
	histo_ratio_data->GetYaxis()->SetTitle("Data/Prediction");
	histo_ratio_data->SetMinimum(0.0);
	histo_ratio_data->SetMaximum(1.999);
	histo_ratio_data->Draw("E1X0"); //Draw ratio points


	TH1F *h_line1 = new TH1F("","",this->nbin,xmin_stack,xmax_stack);
	TH1F *h_line2 = new TH1F("","",this->nbin,xmin_stack,xmax_stack);
	TH1F *h_line3 = new TH1F("","",this->nbin,xmin_stack,xmax_stack);

	for(int ibin=1; ibin<this->nbin +1; ibin++)
	{
		h_line1->SetBinContent(ibin, 0.5);
		h_line2->SetBinContent(ibin, 1.5);
		h_line3->SetBinContent(ibin, 2.5);
	}

	h_line1->SetLineStyle(3);
	h_line2->SetLineStyle(3);
	h_line3->SetLineStyle(3);

	h_line1->Draw("hist same");
	h_line2->Draw("hist same");
	h_line3->Draw("hist same");


	//SINCE WE MODIFIED THE ORIGINAL AXIS OF THE HISTOS, NEED TO DRAW AN INDEPENDANT AXIS REPRESENTING THE ORIGINAL x VALUES
	TGaxis *axis = new TGaxis(histo_ratio_data->GetXaxis()->GetXmin(),histo_ratio_data->GetMinimum(),histo_ratio_data->GetXaxis()->GetXmax(),histo_ratio_data->GetMinimum(),xmin_data,xmax_data, 510, "+"); // + : tick marks on positive side ; = : label on same side as marks
   // axis->SetLineColor(1);

	if (template_name == "BDT" ) axis->SetTitle("BDT output (1 tag)") ;
	else if ( template_name == "BDTttZ" ) axis->SetTitle("BDT output (2 tags)");
	else if ( template_name == "mTW")  axis->SetTitle("m_{T}^{W} [GeV]");
	axis->SetTitleSize(0.04);
	axis->SetTitleFont(42);
	axis->SetLabelFont(42);
	axis->SetTitleOffset(1.4);
	axis->SetLabelSize(0.05);
	axis->Draw("same");



	stack_MC->GetXaxis()->SetLabelFont(42);
	stack_MC->GetYaxis()->SetLabelFont(42);
	stack_MC->GetYaxis()->SetTitleFont(42);
	stack_MC->GetYaxis()->SetTitleSize(0.045);
	stack_MC->GetYaxis()->SetTickLength(0.04);
	stack_MC->GetXaxis()->SetLabelSize(0.0);
	stack_MC->GetYaxis()->SetLabelSize(0.048);
	stack_MC->GetYaxis()->SetTitle("Events");

	c1->Modified();


	//-------------------
	//CAPTIONS
	//-------------------

// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines //CHANGED
		//
		TString cmsText     = "CMS";
		float cmsTextFont   = 61;  // default is helvetic-bold

		bool writeExtraText = false;
		TString extraText   = "Preliminary";
		// TString extraText   = "Work in progress";
		float extraTextFont = 52;  // default is helvetica-italics

		// text sizes and text offsets with respect to the top frame
		// in unit of the top margin size
		float lumiTextSize     = 0.6;
		float lumiTextOffset   = 0.2;
		float cmsTextSize      = 0.75;
		float cmsTextOffset    = 0.1;  // only used in outOfFrame version

		float relPosX    = 0.045;
		float relPosY    = 0.035;
		float relExtraDY = 1.2;

		// ratio of "CMS" and extra text size
		float extraOverCmsTextSize  = 0.76;

		float lumi = 35.9 * luminosity_rescale; //CHANGED
		TString lumi_13TeV = Convert_Number_To_TString(lumi);
		lumi_13TeV += " fb^{-1} (13 TeV)";

		TLatex latex;
		latex.SetNDC();
		latex.SetTextAngle(0);
		latex.SetTextColor(kBlack);

		float H = c1->GetWh();
		float W = c1->GetWw();
		float l = c1->GetLeftMargin();
		float t = c1->GetTopMargin();
		float r = c1->GetRightMargin();
		float b = c1->GetBottomMargin();

		float extraTextSize = extraOverCmsTextSize*cmsTextSize;

		latex.SetTextFont(42);
		latex.SetTextAlign(31);
		latex.SetTextSize(lumiTextSize*t);
		//	Change position w.r.t. CMS recommendation, only for control plots
		latex.DrawLatex(0.8,1-t+lumiTextOffset*t,lumi_13TeV);
		latex.SetTextFont(cmsTextFont);
		latex.SetTextAlign(11);
		latex.SetTextSize(cmsTextSize*t);
		latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);

		//float posX_ =   l +  relPosX*(1-l-r);
		//float posY_ =   1-t+lumiTextOffset*t;

		latex.SetTextFont(extraTextFont);
		//latex.SetTextAlign(align_);
		latex.SetTextSize(extraTextSize*t);
		latex.DrawLatex(l+cmsTextSize*l, 1-t+lumiTextOffset*t, extraText);


		TString info_data;
		if (channel=="eee")    info_data = "eee channel";
		else if (channel=="eeu")  info_data = "ee#mu channel";
		else if (channel=="uue")  info_data = "#mu#mu e channel";
		else if (channel=="uuu") info_data = "#mu#mu #mu channel";
		else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";

		TString extrainfo_data;
		if(template_name=="mTW") {extrainfo_data= "3l,>0j,0bj";}
		else if(template_name == "BDTttZ") {extrainfo_data= "3l,>1j,>1bj";}
		else if(template_name == "BDT") {extrainfo_data= "3l,2-3j,1bj";}

		TLatex text2 ; //= new TLatex(0, 0, info_data);
		text2.SetNDC();
		text2.SetTextAlign(13);
		text2.SetTextFont(42);
		text2.SetTextSize(0.04);
		text2.DrawLatex(0.195,0.91,info_data);

		text2.SetTextFont(62);
		text2.DrawLatex(0.63,0.9,extrainfo_data);

//------------------

	qw->Draw("same");

	mkdir("plots",0777); //Create directory if inexistant
	mkdir("plots/templates",0777); //Create directory if inexistant
	mkdir("plots/templates/postfit",0777); //Create directory if inexistant

	//Output
	TString output_plot_name = "plots/templates/postfit/postfit_" + template_name +"_template_" + channel+ this->filename_suffix + this->format;
	if(channel == "" || allchannels) {output_plot_name = "plots/templates/postfit/postfit_" + template_name +"_template_all" + this->filename_suffix + this->format;}

	c1->SaveAs(output_plot_name.Data());

	delete file_input;
	delete file_data;
	delete c1; delete qw;
	delete stack_MC;

	return 0;
}

















//-----------------------------------------------------------------------------------------
// ##     ## ######## ##     ##    ##     ##          ######  ##     ## ########  ######  ##    ##  ######
// ###   ### ##       ###   ###     ##   ##          ##    ## ##     ## ##       ##    ## ##   ##  ##    ##
// #### #### ##       #### ####      ## ##           ##       ##     ## ##       ##       ##  ##   ##
// ## ### ## ######   ## ### ##       ###    ####### ##       ######### ######   ##       #####     ######
// ##     ## ##       ##     ##      ## ##           ##       ##     ## ##       ##       ##  ##         ##
// ##     ## ##       ##     ##     ##   ##          ##    ## ##     ## ##       ##    ## ##   ##  ##    ##
// ##     ## ######## ##     ##    ##     ##          ######  ##     ## ########  ######  ##    ##  ######
//-----------------------------------------------------------------------------------------








//-----------------------------------------------------------------------------------------
//  ######  ##     ## ########  ######## ########  ########   #######   ######  ########    ##     ## ######## ##     ##
// ##    ## ##     ## ##     ## ##       ##     ## ##     ## ##     ## ##    ## ##          ###   ### ##       ###   ###
// ##       ##     ## ##     ## ##       ##     ## ##     ## ##     ## ##       ##          #### #### ##       #### ####
//  ######  ##     ## ########  ######   ########  ########  ##     ##  ######  ######      ## ### ## ######   ## ### ##
//       ## ##     ## ##        ##       ##   ##   ##        ##     ##       ## ##          ##     ## ##       ##     ##
// ##    ## ##     ## ##        ##       ##    ##  ##        ##     ## ##    ## ##          ##     ## ##       ##     ##
//  ######   #######  ##        ######## ##     ## ##         #######   ######  ########    ##     ## ######## ##     ##
//-----------------------------------------------------------------------------------------

/**
 * Superpose prefit template distributions with or without MEM
 */
void theMVAtool::Superpose_With_Without_MEM_Templates(TString template_name, TString channel, bool normalized)
{
	if(channel != "allchan" && channel != "uuu" && channel != "uue" && channel != "eeu" && channel != "eee" )
	{
		cout<<"Wrong channel  Abort"<<endl; return;
	}

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mTW") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	//--- NO MEM
	TString input_name_noMEM = "outputs/files_noMEM/Combine_Input.root";
	if(!Check_File_Existence(input_name_noMEM) ) {input_name_noMEM = "outputs/files_noMEM/Reader_" + template_name + this->filename_suffix + ".root";}
	if(!Check_File_Existence(input_name_noMEM) ) {cout<<"No template files in dir. outputs/files_noMEM/ ! Abort"<<endl; return;}
	TFile* file_input_noMEM = 0;
	file_input_noMEM = TFile::Open( input_name_noMEM.Data() );
	if(file_input_noMEM == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

	//WITH MEM (nominal)
	TString input_name_MEM = "outputs/files_nominal/Combine_Input.root";
	if(!Check_File_Existence(input_name_MEM) ) {input_name_MEM = "outputs/files_nominal/Reader_" + template_name + this->filename_suffix + ".root";}
	if(!Check_File_Existence(input_name_MEM) ) {cout<<"No template files in dir. outputs/files_nominal/ ! Abort"<<endl; return;}
	TFile* file_input_MEM = 0;
	file_input_MEM = TFile::Open( input_name_MEM.Data() );
	if(file_input_MEM == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

	mkdir("plots",0777);
	TH1::SetDefaultSumw2();

	TH1F *h_tmp_noMEM = 0, *h_signal_noMEM  = 0, *h_sum_background_noMEM = 0;
	TH1F *h_tmp_MEM = 0, *h_signal_MEM = 0, *h_sum_background_MEM = 0;

	//TLegend* qw = new TLegend(.80,.60,.95,.90);
	TLegend* qw = new TLegend(.6,.77,0.85,.915);
	qw->SetShadowColor(0);
	qw->SetFillColor(0);
	qw->SetLineColor(0);
	// qw->SetLineWidth(3);
	qw->SetTextSize(0.03);

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		if(channel != "allchan" && channel_list[ichan] != channel) {continue;}

	//--- All MC samples but fakes
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			if(sample_list[isample].Contains("Data") ) {continue;}

			TString histo_name = template_name + "_" + channel_list[ichan] + "__" + sample_list[isample];
			if(!file_input_noMEM->GetListOfKeys()->Contains(histo_name.Data()) && !sample_list[isample].Contains("Fakes") ) {cout<<histo_name<<" : not found"<<endl; continue;}
			if(!file_input_MEM->GetListOfKeys()->Contains(histo_name.Data()) && !sample_list[isample].Contains("Fakes") ) {cout<<histo_name<<" : not found"<<endl; continue;}

			if(!sample_list[isample].Contains("Fakes") && !sample_list[isample].Contains("tZq") )
			{
				h_tmp_noMEM = (TH1F*) file_input_noMEM->Get(histo_name.Data())->Clone();
				h_tmp_MEM = (TH1F*) file_input_MEM->Get(histo_name.Data())->Clone();

				if(h_sum_background_noMEM == 0) {h_sum_background_noMEM = (TH1F*) h_tmp_noMEM->Clone();}
				else {h_sum_background_noMEM->Add(h_tmp_noMEM);}

				if(h_sum_background_MEM == 0) {h_sum_background_MEM = (TH1F*) h_tmp_MEM->Clone();}
				else {h_sum_background_MEM->Add(h_tmp_MEM);}
			}
			else if(sample_list[isample].Contains("Fakes") ) //fakes
			{
				if(!file_input_noMEM->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
				if(!file_input_MEM->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
				else
				{
					h_tmp_noMEM = (TH1F*) file_input_noMEM->Get(histo_name.Data())->Clone();
					h_tmp_MEM = (TH1F*) file_input_noMEM->Get(histo_name.Data())->Clone();

					h_sum_background_noMEM->Add(h_tmp_noMEM);
					h_sum_background_MEM->Add(h_tmp_MEM);
				}
			}
			else if(sample_list[isample].Contains("tZq") ) //Signal
			{
				h_tmp_noMEM = (TH1F*) file_input_noMEM->Get(histo_name.Data())->Clone();
				h_tmp_MEM = (TH1F*) file_input_MEM->Get(histo_name.Data())->Clone();

				if(h_signal_noMEM == 0) {h_signal_noMEM = (TH1F*) h_tmp_noMEM->Clone();}
				else {h_signal_noMEM->Add(h_tmp_noMEM);}

				if(h_signal_MEM == 0) {h_signal_MEM = (TH1F*) h_tmp_MEM->Clone();}
				else {h_signal_MEM->Add(h_tmp_MEM);}
			}
		} //end sample loop

	} //end channel loop

	if(normalized)
	{
		h_sum_background_noMEM->Scale(1./h_sum_background_noMEM->Integral() );
		h_sum_background_MEM->Scale(1./h_sum_background_MEM->Integral() );
		h_signal_noMEM->Scale(1./h_signal_noMEM->Integral() );
		h_signal_MEM->Scale(1./h_signal_MEM->Integral() );
	}

	h_sum_background_noMEM->SetLineColor(kBlue);
	h_sum_background_noMEM->SetLineWidth(2);

	h_sum_background_MEM->SetLineColor(kBlue);
	h_sum_background_MEM->SetLineStyle(2);
	h_sum_background_MEM->SetLineWidth(2);

	h_signal_noMEM->SetLineColor(kRed);
	h_signal_noMEM->SetLineWidth(2);

	h_signal_MEM->SetLineColor(kRed);
	h_signal_MEM->SetLineStyle(2);
	h_signal_MEM->SetLineWidth(2);

	if(!normalized) //Changes which histo should be drawn first
	{
		h_sum_background_MEM->GetXaxis()->SetTitleOffset(1.4);
		h_sum_background_MEM->GetXaxis()->SetLabelSize(0.045);
		h_sum_background_MEM->GetXaxis()->SetLabelFont(42);
		h_sum_background_MEM->GetXaxis()->SetTitleFont(42);

		h_sum_background_MEM->GetYaxis()->SetTitleOffset(1.42);
		h_sum_background_MEM->GetYaxis()->SetLabelSize(0.04);
		h_sum_background_MEM->GetYaxis()->SetLabelFont(42);
		h_sum_background_MEM->GetYaxis()->SetTitleFont(42);
		h_sum_background_MEM->GetYaxis()->SetTitleSize(0.04);

		h_sum_background_MEM->SetMinimum(0);
		h_sum_background_MEM->SetMaximum(h_sum_background_MEM->GetMaximum()*1.1);
		h_sum_background_MEM->GetYaxis()->SetTitle("Events");
		if(template_name=="BDT") h_sum_background_MEM->GetXaxis()->SetTitle("BDT output (1 tag)");
		else if(template_name=="BDTttZ") h_sum_background_MEM->GetXaxis()->SetTitle("BDT output (2 tags)");
		else if(template_name=="mTW") h_sum_background_MEM->GetXaxis()->SetTitle("mTW [GeV]");
	}
	else
	{
		h_signal_MEM->GetXaxis()->SetTitleOffset(1.4);
		h_signal_MEM->GetXaxis()->SetLabelSize(0.045);
		h_signal_MEM->GetXaxis()->SetLabelFont(42);
		h_signal_MEM->GetXaxis()->SetTitleFont(42);

		h_signal_MEM->GetYaxis()->SetTitleOffset(1.42);
		h_signal_MEM->GetYaxis()->SetLabelSize(0.04);
		h_signal_MEM->GetYaxis()->SetLabelFont(42);
		h_signal_MEM->GetYaxis()->SetTitleFont(42);
		h_signal_MEM->GetYaxis()->SetTitleSize(0.04);

		h_signal_MEM->SetMinimum(0);
		h_signal_MEM->SetMaximum(h_signal_MEM->GetMaximum()*1.4);
		h_signal_MEM->GetYaxis()->SetTitle("Events");
		if(template_name=="BDT") h_signal_MEM->GetXaxis()->SetTitle("BDT output (1 tag)");
		else if(template_name=="BDTttZ") h_signal_MEM->GetXaxis()->SetTitle("BDT output (2 tags)");
		else if(template_name=="mTW") h_signal_MEM->GetXaxis()->SetTitle("mTW [GeV]");
	}


	//Set Yaxis maximum & minimum
	// if(h_sum_data->GetMaximum() > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
	// else stack_MC->SetMaximum(stack_MC->GetMaximum()+0.3*stack_MC->GetMaximum());
	// stack_MC->SetMinimum(0);


	//Canvas definition
	Load_Canvas_Style();
	TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);

	if(!normalized)
	{
		h_sum_background_MEM->Draw("hist");
		h_signal_MEM->Draw("same hist");
	}
	else
	{
		h_signal_MEM->Draw("hist");
		h_sum_background_MEM->Draw("same hist");
	}
	h_sum_background_noMEM->Draw("same hist");
	h_signal_noMEM->Draw("same hist");

	qw->AddEntry(h_sum_background_noMEM, "Sum backgrounds" , "L");
	qw->AddEntry(h_sum_background_MEM, "Sum backgrounds (+MEM)" , "L");
	qw->AddEntry(h_signal_noMEM, "Signal" , "L");
	qw->AddEntry(h_signal_MEM, "Signal (+MEM)" , "L");

	//-------------------
	//CAPTIONS
	//-------------------

// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines //CHANGED
	//
	TString cmsText     = "CMS";
	float cmsTextFont   = 61;  // default is helvetic-bold

	bool writeExtraText = false;
	TString extraText   = "Preliminary";
	float extraTextFont = 52;  // default is helvetica-italics

	// text sizes and text offsets with respect to the top frame
	// in unit of the top margin size
	float lumiTextSize     = 0.6;
	float lumiTextOffset   = 0.2;
	float cmsTextSize      = 0.75;
	float cmsTextOffset    = 0.1;  // only used in outOfFrame version

	float relPosX    = 0.045;
	float relPosY    = 0.035;
	float relExtraDY = 1.2;

	// ratio of "CMS" and extra text size
	float extraOverCmsTextSize  = 0.76;

	float lumi = 35.9 * luminosity_rescale; //CHANGED
	TString lumi_13TeV = Convert_Number_To_TString(lumi);
	lumi_13TeV += " fb^{-1} (13 TeV)";

	TLatex latex;
	latex.SetNDC();
	latex.SetTextAngle(0);
	latex.SetTextColor(kBlack);

	float H = c1->GetWh();
	float W = c1->GetWw();
	float l = c1->GetLeftMargin();
	float t = c1->GetTopMargin();
	float r = c1->GetRightMargin();
	float b = c1->GetBottomMargin();

	float extraTextSize = extraOverCmsTextSize*cmsTextSize;

	latex.SetTextFont(42);
	latex.SetTextAlign(31);
	latex.SetTextSize(lumiTextSize*t);
	latex.DrawLatex(0.7,1-t+lumiTextOffset*t,lumi_13TeV);

	latex.SetTextFont(cmsTextFont);
	latex.SetTextAlign(11);
	latex.SetTextSize(cmsTextSize*t);
	latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);
	latex.SetTextFont(extraTextFont);
	latex.SetTextSize(extraTextSize*t);
	latex.DrawLatex(l+cmsTextSize*l, 1-t+lumiTextOffset*t, extraText);

//------------------

	TString info_data;
	if (channel=="eee")    info_data = "eee channel";
	else if (channel=="eeu")  info_data = "ee#mu channel";
	else if (channel=="uue")  info_data = "#mu#mu e channel";
	else if (channel=="uuu") info_data = "#mu#mu #mu channel";
	else if(channel=="allchannels") info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";

	TString extrainfo_data;
	if(isWZ) {extrainfo_data= "3l,>0j,0bj";}
	else if(isttZ) {extrainfo_data= "3l,>1j,>1bj";}
	else if(!isWZ && !isttZ) {extrainfo_data= "3l,2-3j,1bj";}

	TLatex text2 ; //= new TLatex(0, 0, info_data);
	text2.SetNDC();
	text2.SetTextAlign(13);
	text2.SetTextFont(42);
	text2.SetTextSize(0.04);
	text2.DrawLatex(0.195,0.91,info_data);

	text2.SetTextFont(62);
	// text2.DrawLatex(0.25,0.85,extrainfo_data);
	text2.DrawLatex(0.63,0.9,extrainfo_data);

	qw->Draw(); //Draw legend

//------------------

	mkdir("plots",0777); //Create directory if inexistant
	mkdir("plots/Superpose",0777); //Create directory if inexistant

	//Output
	TString output_plot_name = "plots/Superpose/SuperposeMEM_" + template_name +"_" + channel + this->filename_suffix + this->format;
	if(normalized) output_plot_name = "plots/Superpose/SuperposeMEM_" + template_name +"_" + channel + this->filename_suffix + "_norm" + this->format;

	c1->SaveAs(output_plot_name.Data());

	delete file_input_noMEM;
	delete file_input_MEM;

	delete c1;
	delete qw;

	return;
}








//-----------------------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######     ##     ##    ###    ########  ####    ###    ######## ####  #######  ##    ##
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##    ##     ##   ## ##   ##     ##  ##    ## ##      ##     ##  ##     ## ###   ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##          ##     ##  ##   ##  ##     ##  ##   ##   ##     ##     ##  ##     ## ####  ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######     ##     ## ##     ## ########   ##  ##     ##    ##     ##  ##     ## ## ## ##
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##     ##   ##  ######### ##   ##    ##  #########    ##     ##  ##     ## ##  ####
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##      ## ##   ##     ## ##    ##   ##  ##     ##    ##     ##  ##     ## ##   ###
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######        ###    ##     ## ##     ## #### ##     ##    ##    ####  #######  ##    ##
//-----------------------------------------------------------------------------------------

/**
 * Plot nominal templates with 1 systematics superposed
 * --> Can see visually it's amplitude/variation
 * (mostly taken from Draw CR plot function)
 */
void theMVAtool::Draw_Template_With_Systematic_Variation(TString channel, TString template_name, TString sample, TString systematic)
{

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Draw Template with Systematic "<<systematic<<" superimposed ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	TString filepath_noMEM = "./outputs/files_noMEM/Combine_Input.root";
	TString filepath_withMEM = "./outputs/files_nominal/Combine_Input.root";


	if(!Check_File_Existence(filepath_noMEM) ) {cout<<filepath_noMEM.Data()<<" not found ! Abort !"<<endl; return;}
	TFile* f_noMEM = TFile::Open( filepath_noMEM );

	if(!Check_File_Existence(filepath_withMEM) ) {cout<<filepath_withMEM.Data()<<" not found ! Abort !"<<endl; return;}
	TFile* f = TFile::Open( filepath_withMEM );

	TH1::SetDefaultSumw2();

	mkdir("plots",0777); //Create directory if inexistant

	TH1F *h_tmp;

	vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
	thechannellist.push_back("uuu");
	thechannellist.push_back("uue");
	thechannellist.push_back("eeu");
	thechannellist.push_back("eee");

	vector<TString> template_list;
	template_list.push_back("BDT");
	template_list.push_back("BDTttZ");
	template_list.push_back("BDTWZ");

	//Load Canvas definition
	Load_Canvas_Style();


//---------------------
//Loop on var > chan > sample > syst
//Retrieve histograms, stack, compare, plot
	for(int ivar=0; ivar<template_list.size(); ivar++)
	{
		if(template_list[ivar] != template_name) {continue;}

		TLegend* qw = 0;
		// TLegend* qw = new TLegend(.85,.7,0.99,0.99);
		qw = new TLegend(.2 ,.7,0.4,0.85);

		qw->SetShadowColor(0);
		// qw->SetFillColor(0);
		qw->SetLineColor(0);

		TH1F* histo_nominal = 0;
		TH1F* histo_noMEM = 0;

		TH1F* histo_nominal_syst = 0;
		TH1F* histo_noMEM_syst = 0;

		//---------------------------
		//ERROR VECTORS INITIALIZATION
		//---------------------------
		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
		vector<double> v_eyl_noMEM, v_eyh_noMEM, v_exl_noMEM, v_exh_noMEM, v_x_noMEM, v_y_noMEM; //Contain the systematic errors (used to create the TGraphError)


		TString histo_name = template_list[ivar] + "_uuu__tZqmcNLO"; //Get binning
		if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<__LINE__<<" : "<<histo_name<<" : not found ! Continue !"<<endl; continue;}

		h_tmp=0;
		h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
		if(!h_tmp) continue;

		//Initialize error vectors with height=0 and width = bin/2
		int nofbins = h_tmp->GetNbinsX();
		for(int ibin=0; ibin<nofbins; ibin++)
		{
			v_eyl.push_back(0); v_eyh.push_back(0);
			v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
			v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
			v_y.push_back(0);

			v_eyl_noMEM.push_back(0); v_eyh_noMEM.push_back(0);
			v_exl_noMEM.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh_noMEM.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
			v_x_noMEM.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
			v_y_noMEM.push_back(0);
		}


		//---------------------------
		//RETRIEVE & SUM HISTOGRAMS, SUM ERRORS QUADRATICALLY
		//---------------------------

		int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
		for(int ichan=0; ichan<thechannellist.size(); ichan++)
		{
			if(channel != thechannellist[ichan]) {continue;} //If plot single channel

			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				if(sample_list[isample] != sample) {continue;}
				if(sample_list[isample].Contains("Data")) {continue;}

				h_tmp = 0; //Temporary storage of histogram

				histo_name = template_list[ivar] + "_" + thechannellist[ichan] + "__" + sample_list[isample];


				h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();

				h_tmp->SetLineColor(kRed);


				histo_nominal = (TH1F*) h_tmp->Clone();
				histo_nominal->Scale(1./histo_nominal->Integral());


				h_tmp = (TH1F*) f_noMEM->Get(histo_name.Data())->Clone();


				h_tmp->SetLineColor(kBlack);


				histo_noMEM = (TH1F*) h_tmp->Clone();
				histo_noMEM->Scale(1./histo_noMEM->Integral());


				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					if(syst_list[isyst] == "" || !syst_list[isyst].Contains(systematic) ) {continue;}


					if((sample_list[isample].Contains("Fakes") && !syst_list[isyst].Contains("Fakes")) ) {continue;}
					if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}
					if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only


					TString histo_name_syst = histo_name;
					histo_name_syst+= "__" + Combine_Naming_Convention(syst_list[isyst]);

					if(!f->GetListOfKeys()->Contains(histo_name_syst.Data()) ) {cout<<"syst not found! "<<endl; continue;}
					if(!f_noMEM->GetListOfKeys()->Contains(histo_name_syst.Data()) ) {cout<<"syst not found! "<<endl; continue;}

					histo_nominal_syst = (TH1F*) f->Get(histo_name_syst.Data())->Clone();
					histo_nominal_syst->Scale(1./histo_nominal_syst->Integral());

					histo_noMEM_syst = (TH1F*) f_noMEM->Get(histo_name_syst.Data())->Clone();
					histo_noMEM_syst->Scale(1./histo_noMEM_syst->Integral());

					for(int ibin=0; ibin<nofbins; ibin++)
					{
						v_y[ibin] = histo_nominal->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)
						v_y_noMEM[ibin] = histo_noMEM->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)
					}

					//Add up here the different errors (quadratically), for each bin separately
					for(int ibin=0; ibin<nofbins; ibin++)
					{
						//NOTE : in TGraphAsymmErrors, should use fabs(errors) ; because it knows which are the "up" and "down" errors
						//--------------------------
						double tmp = histo_nominal_syst->GetBinContent(ibin+1) - histo_nominal->GetBinContent(ibin+1);
						if(tmp>0 && (fabs(tmp)>fabs(v_eyh[ibin])) ) v_eyh[ibin] = fabs(tmp);
						else if(tmp<0 && (fabs(tmp)>fabs(v_eyl[ibin])) ) v_eyl[ibin] = fabs(tmp);

						// cout<<syst_list[isyst]<<endl;
						// cout<<"tmp = "<<tmp<<endl;

						//--------------------------

						double tmp_noMEM = histo_noMEM_syst->GetBinContent(ibin+1) - histo_noMEM->GetBinContent(ibin+1);
						if(tmp_noMEM>0 && (fabs(tmp_noMEM)>fabs(v_eyh_noMEM[ibin])) ) v_eyh_noMEM[ibin] = fabs(tmp_noMEM);
						else if(tmp_noMEM<0 && (fabs(tmp_noMEM)>fabs(v_eyl_noMEM[ibin])) ) v_eyl_noMEM[ibin] = fabs(tmp_noMEM);
						//--------------------------

						if(ibin > 0) {continue;} //cout only first bin
						// cout<<"syst = "<<syst_list[isyst]<<endl;
						// cout<<"histo_nominal_syst->GetBinContent(ibin+1) : "<<histo_nominal_syst->GetBinContent(ibin+1)<<" , histo_nominal->GetBinContent(ibin+1) : "<<histo_nominal->GetBinContent(ibin+1)<<endl;

						// cout<<"tmp = "<<tmp<<endl;
						//
						// cout<<endl<<"ibin "<<ibin<<" "<<histo_nominal_syst->GetBinContent(ibin+1)<<" -- x = "<<v_x[ibin]<<", y = "<<v_y[ibin]<<endl;
						// cout<<"with MEM : eyl = "<<v_eyl[ibin]<<", eyh = "<<v_eyh[ibin]<<endl;
						// cout<<"no MEM : eyl = "<<v_eyl_noMEM[ibin]<<", eyh = "<<v_eyh_noMEM[ibin]<<endl;
					}

				} //end syst loop
			} //end sample loop

			niter_chan++;
		} //end channel loop




		TString sample_legend = sample;
		if(sample.Contains("tZq")) sample_legend = "tZq";
		else if(sample.Contains("Fakes")) sample_legend = "Non-prompt";

		qw->AddEntry(histo_nominal, (sample_legend + " with MEM").Data() , "l");
		qw->AddEntry(histo_noMEM, (sample_legend+" no MEM").Data() , "l");



		//---------------------------
		//DRAW HISTOS
		//---------------------------

		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
		c1->SetBottomMargin(0.3); //Leave space for ratio plot
		TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);

		c1->cd();
		histo_nominal->SetMinimum(0.0001);
		histo_nominal->SetMaximum(histo_nominal->GetMaximum()*1.2);
		if(sample.Contains("Fakes") ) histo_nominal->SetMaximum(histo_nominal->GetMaximum()*1.4);
		histo_nominal->Draw("HIST"); histo_nominal->GetXaxis()->SetLabelSize(0.0);
		histo_noMEM->Draw("HIST same");


		//---------------------------
		//DRAW SYST ERRORS ON PLOT
		//---------------------------

		//Need to take sqrt of total errors
		for(int ibin=0; ibin<nofbins; ibin++)
		{

			// v_eyh[ibin] = 0.02;
			// v_eyl[ibin] =  0.02;


			if(ibin>0) continue;
			// cout<<"--- chan "<<channel<<endl;
			// cout<<"Bin 0 : y = "<<v_y[ibin]<<endl;
			// cout<<"high = "<<v_eyh[ibin]<<" --> ratio = "<<v_eyh[ibin]/v_y[ibin]<<endl;
			// cout<<"low = "<<v_eyl_noMEM[ibin]<<" --> ratio = "<<v_eyl_noMEM[ibin]/v_y_noMEM[ibin]<<endl;

			// cout<<"histo_nominal->GetBinContent = "<<histo_nominal->GetBinContent(ibin+1)<<endl;
			// cout<<"v_eyl = "<<v_eyl[ibin]<<endl;
			// cout<<"v_eyh = "<<v_eyh[ibin]<<endl;
			//
			// cout<<endl<<"histo_noMEM->GetBinContent = "<<histo_noMEM->GetBinContent(ibin+1)<<endl;
			// cout<<"v_eyl_noMEM = "<<v_eyl_noMEM[ibin]<<endl;
			// cout<<"v_eyh_noMEM = "<<v_eyh_noMEM[ibin]<<endl;
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
		gr->SetFillStyle(3005); //Right strips
		gr->SetFillColor(2);
		gr->Draw("e2 same"); //Superimposes the systematics uncertainties on stack

		qw->AddEntry(gr, (systematic+" with MEM").Data(), "f");


		double* eyl_noMEM = &v_eyl_noMEM[0];
		double* eyh_noMEM = &v_eyh_noMEM[0];
		double* exl_noMEM = &v_exl_noMEM[0];
		double* exh_noMEM = &v_exh_noMEM[0];
		double* x_noMEM = &v_x_noMEM[0];
		double* y_noMEM = &v_y_noMEM[0];

		//Create TGraphAsymmErrors with the error vectors / (x,y) coordinates --> Can superimpose it on plot
		TGraphAsymmErrors* gr_noMEM = 0;
		gr_noMEM = new TGraphAsymmErrors(nofbins,x_noMEM,y_noMEM,exl_noMEM,exh_noMEM,eyl_noMEM,eyh_noMEM);
		gr_noMEM->SetFillStyle(3004); //left strips
		gr_noMEM->SetFillColor(1);
		gr_noMEM->Draw("e2 same"); //Superimposes the systematics uncertainties on stack

		qw->AddEntry(gr_noMEM, (systematic+" no MEM").Data(), "f");




		//-------------------
		//CAPTIONS
		//-------------------


// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines //CHANGED
		//
		TString cmsText     = "CMS";
		float cmsTextFont   = 61;  // default is helvetic-bold
		bool writeExtraText = false;
		TString extraText   = "Preliminary";
		float extraTextFont = 52;  // default is helvetica-italics
		// text sizes and text offsets with respect to the top frame
		// in unit of the top margin size
		float lumiTextSize     = 0.6;
		float lumiTextOffset   = 0.2;
		float cmsTextSize      = 0.75;
		float cmsTextOffset    = 0.1;  // only used in outOfFrame version
		float relPosX    = 0.045;
		float relPosY    = 0.035;
		float relExtraDY = 1.2;
		// ratio of "CMS" and extra text size
		float extraOverCmsTextSize  = 0.76;
		float lumi = 35.9 * luminosity_rescale; //CHANGED
		TString lumi_13TeV = Convert_Number_To_TString(lumi);
		lumi_13TeV += " fb^{-1} (13 TeV)";
		TLatex latex;
		latex.SetNDC();
		latex.SetTextAngle(0);
		latex.SetTextColor(kBlack);
		float H = c1->GetWh();
		float W = c1->GetWw();
		float l = c1->GetLeftMargin();
		float t = c1->GetTopMargin();
		float r = c1->GetRightMargin();
		float b = c1->GetBottomMargin();
		float extraTextSize = extraOverCmsTextSize*cmsTextSize;
		latex.SetTextFont(42);
		latex.SetTextAlign(31);
		latex.SetTextSize(lumiTextSize*t);
		latex.DrawLatex(0.7,1-t+lumiTextOffset*t,lumi_13TeV);
		latex.SetTextFont(cmsTextFont);
		latex.SetTextAlign(11);
		latex.SetTextSize(cmsTextSize*t);
		latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);
		latex.SetTextFont(extraTextFont);
		latex.SetTextSize(extraTextSize*t);
		latex.DrawLatex(l+cmsTextSize*l, 1-t+lumiTextOffset*t, extraText);


//------------------

		TString info_data;
		if (channel=="eee")    info_data = "eee channel";
		else if (channel=="eeu")  info_data = "ee#mu channel";
		else if (channel=="uue")  info_data = "#mu#mu e channel";
		else if (channel=="uuu") info_data = "#mu#mu #mu channel";
		// else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";

		TString extrainfo_data;
		if(template_name=="mTW") {extrainfo_data= "3l,>0j,0bj";}
		else if(template_name == "BDTttZ") {extrainfo_data= "3l,>1j,>1bj";}
		else if(template_name == "BDT") {extrainfo_data= "3l,2-3j,1bj";}

		TLatex text2 ; //= new TLatex(0, 0, info_data);
		text2.SetNDC();
		text2.SetTextAlign(13);
		text2.SetTextFont(42);
		text2.SetTextSize(0.04);
		text2.DrawLatex(0.195,0.91,info_data);

		text2.SetTextFont(62);
		text2.DrawLatex(0.63,0.9,extrainfo_data);

		qw->Draw(); //Draw legend



		//--------------------------
		//SET VARIABLES LABELS
		//--------------------------

		vector<TString> stringv_list;

		if(template_list[ivar] == "BDT") 			{	stringv_list.push_back("BDT discriminant"); }
		else if(template_list[ivar] == "BDTttZ") 			{	stringv_list.push_back("BDTttZ discriminant"); }
		else if(template_list[ivar] == "mTW") 			{	stringv_list.push_back("M_{T}_{W} [GeV]"); }



		//--------------------------
		//DRAW DATA/MC RATIO
		//--------------------------

	//Create Data/MC ratio plot (bottom of canvas)

		canvas_2->SetTopMargin(0.7);
		canvas_2->SetFillColor(0);
		canvas_2->SetFillStyle(0);
		canvas_2->SetGridy(1);
		canvas_2->Draw();
		canvas_2->cd(0);

		TH1::SetDefaultSumw2();


		TH1F * histo_ratio_data = 0;
		histo_ratio_data = (TH1F*) histo_nominal->Clone();

		if(!histo_ratio_data) {cout<<"histo_ratio_data is null !"<<endl; return;}

		for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
		{
			histo_ratio_data->SetBinContent(ibin, 1);
			histo_ratio_data->SetBinError(ibin, 0);
		}

		histo_ratio_data->GetXaxis()->SetTitle(stringv_list[ivar].Data());
		histo_ratio_data->GetYaxis()->SetTickLength(0.15);

		histo_ratio_data->GetXaxis()->SetTitleOffset(1.4);
		histo_ratio_data->GetYaxis()->SetTitleOffset(1.42);
		histo_ratio_data->GetXaxis()->SetLabelSize(0.045);
		histo_ratio_data->GetYaxis()->SetLabelSize(0.04); //CHANGED
		// histo_ratio_data->GetYaxis()->SetLabelSize(0.048);
		histo_ratio_data->GetXaxis()->SetLabelFont(42);
		histo_ratio_data->GetYaxis()->SetLabelFont(42);
		histo_ratio_data->GetXaxis()->SetTitleFont(42);
		histo_ratio_data->GetYaxis()->SetTitleFont(42);
		// histo_ratio_data->GetYaxis()->SetNdivisions(6);
		// histo_ratio_data->GetYaxis()->SetNdivisions(503);
		histo_ratio_data->GetYaxis()->SetNdivisions(303); //CHANGED
		histo_ratio_data->GetYaxis()->SetTitleSize(0.04);

		histo_ratio_data->GetYaxis()->SetTitle("Syst/Nominal");

		histo_ratio_data->SetMinimum(0.7);
		histo_ratio_data->SetMaximum(1.299);
		histo_ratio_data->Draw("psame"); //Draw ratio points

		TH1F *h_line1 = new TH1F("","",this->nbin, histo_nominal->GetXaxis()->GetXmin(), histo_nominal->GetXaxis()->GetXmax());
		TH1F *h_line2 = new TH1F("","",this->nbin, histo_nominal->GetXaxis()->GetXmin(), histo_nominal->GetXaxis()->GetXmax());

		for(int ibin=1; ibin<this->nbin+1; ibin++)
		{
			h_line1->SetBinContent(ibin, 0.95);
			h_line2->SetBinContent(ibin, 1.05);
			// h_line3->SetBinContent(ibin, 2.5);
		}

		h_line1->SetLineStyle(3);
		h_line2->SetLineStyle(3);
		// h_line3->SetLineStyle(3);

		h_line1->Draw("hist same");
		h_line2->Draw("hist same");
		// h_line3->Draw("hist same");




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
		  theErrorY_l[i] /= theY[i];
		  theErrorY_h[i] /= theY[i];
		//   if(i==0) cout<<"bin 0 : "<<theErrorY_l[i]<<" , "<<theY[i]<<", ratio = "<<theErrorY_l[i] / theY[i]<<endl;
		  theY[i]=1; //To center the filled area around "1"
		}

		//--> Create new TGraphAsymmErrors
		TGraphAsymmErrors *thegraph_ratio = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
		thegraph_ratio->SetFillStyle(3005);
		thegraph_ratio->SetFillColor(2);
		thegraph_ratio->Draw("e2 same"); //Syst. error for Data/MC ; drawn on canvas2 (Data/MC ratio)





		//Copy previous TGraphAsymmErrors
		TGraphAsymmErrors *thegraph_tmp_noMEM = (TGraphAsymmErrors*) gr_noMEM->Clone();
		double *theErrorX_h_noMEM = thegraph_tmp_noMEM->GetEXhigh();
		double *theErrorY_h_noMEM = thegraph_tmp_noMEM->GetEYhigh();
		double *theErrorX_l_noMEM = thegraph_tmp_noMEM->GetEXlow();
		double *theErrorY_l_noMEM = thegraph_tmp_noMEM->GetEYlow();
		double *theY_noMEM        = thegraph_tmp_noMEM->GetY() ;
		double *theX_noMEM        = thegraph_tmp_noMEM->GetX() ;

		//Divide error --> ratio
		for(int i=0; i<thegraph_tmp_noMEM->GetN(); i++)
		{
		  theErrorY_l_noMEM[i] /= theY_noMEM[i];
		  theErrorY_h_noMEM[i] /= theY_noMEM[i];
		  theY_noMEM[i]=1; //To center the filled area around "1"
		}

		//--> Create new TGraphAsymmErrors
		TGraphAsymmErrors *thegraph_ratio_noMEM = new TGraphAsymmErrors(thegraph_tmp_noMEM->GetN(), theX_noMEM , theY_noMEM ,  theErrorX_l_noMEM, theErrorX_h_noMEM, theErrorY_l_noMEM, theErrorY_h_noMEM);
		thegraph_ratio_noMEM->SetFillStyle(3004);
		thegraph_ratio_noMEM->SetFillColor(1);
		thegraph_ratio_noMEM->Draw("e2 same"); //Syst. error for Data/MC ; drawn on canvas2 (Data/MC ratio)


		histo_nominal->GetXaxis()->SetLabelFont(42);
		histo_nominal->GetYaxis()->SetLabelFont(42);
		histo_nominal->GetYaxis()->SetTitleFont(42);
		histo_nominal->GetYaxis()->SetTitleSize(0.045);
		histo_nominal->GetYaxis()->SetTickLength(0.04);
		histo_nominal->GetXaxis()->SetLabelSize(0.0);
		histo_nominal->GetYaxis()->SetLabelSize(0.048);
		histo_nominal->GetYaxis()->SetTitleOffset(1.3);
		histo_nominal->GetYaxis()->SetTitle("Events (norm.)");


		//-------------------
		//OUTPUT
		//------------------


		//Iximumge name
		TString outputname = "./plots/"+template_name;
		outputname+= "_"+channel;
		outputname+= "_"+sample;
		outputname+= "_"+systematic+"_Variation";
		outputname+= this->format;

		// cout << __LINE__ << endl;
		if(c1!= 0) {c1->SaveAs(outputname.Data() );}
		// cout << __LINE__ << endl;

		delete qw; delete gr;
		delete h_line1; delete h_line2;
		delete thegraph_ratio;
		delete canvas_2;
		delete c1; //Must free dinamically-allocated memory
		delete histo_nominal; delete histo_noMEM;
		delete histo_nominal_syst; delete histo_noMEM_syst;
	} //end var loop

	delete f;

	return;
}
































//-----------------------------------------------------------------------------------------
// ########    ###    ##    ## ########  ######     ##     ##          ######  ##     ## ########  ######  ##    ##  ######
// ##         ## ##   ##   ##  ##       ##    ##     ##   ##          ##    ## ##     ## ##       ##    ## ##   ##  ##    ##
// ##        ##   ##  ##  ##   ##       ##            ## ##           ##       ##     ## ##       ##       ##  ##   ##
// ######   ##     ## #####    ######    ######        ###    ####### ##       ######### ######   ##       #####     ######
// ##       ######### ##  ##   ##             ##      ## ##           ##       ##     ## ##       ##       ##  ##         ##
// ##       ##     ## ##   ##  ##       ##    ##     ##   ##          ##    ## ##     ## ##       ##    ## ##   ##  ##    ##
// ##       ##     ## ##    ## ########  ######     ##     ##          ######  ##     ## ########  ######  ##    ##  ######
//-----------------------------------------------------------------------------------------






//-----------------------------------------------------------------------------------------
//  ######  ##     ## ########  ######## ########  ########   #######   ######  ########    ########    ###    ##    ## ########  ######
// ##    ## ##     ## ##     ## ##       ##     ## ##     ## ##     ## ##    ## ##          ##         ## ##   ##   ##  ##       ##    ##
// ##       ##     ## ##     ## ##       ##     ## ##     ## ##     ## ##       ##          ##        ##   ##  ##  ##   ##       ##
//  ######  ##     ## ########  ######   ########  ########  ##     ##  ######  ######      ######   ##     ## #####    ######    ######
//       ## ##     ## ##        ##       ##   ##   ##        ##     ##       ## ##          ##       ######### ##  ##   ##             ##
// ##    ## ##     ## ##        ##       ##    ##  ##        ##     ## ##    ## ##          ##       ##     ## ##   ##  ##       ##    ##
//  ######   #######  ##        ######## ##     ## ##         #######   ######  ########    ##       ##     ## ##    ## ########  ######
//-----------------------------------------------------------------------------------------

/**
 * Superpose prefit templates or input variables for : signal / fakes / other backgrounds
 */
void theMVAtool::Superpose_Shapes_Fakes_Signal(TString template_name, TString channel, bool normalized, bool draw_templates)
{
	if(channel != "allchan" && channel != "uuu" && channel != "uue" && channel != "eeu" && channel != "eee" )
	{
		cout<<"Wrong channel  Abort"<<endl; return;
	}

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Producing comparison plots (Fakes .vs. Signal shapes) ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;


	TString input_name = "outputs/Combine_Input.root";
	if(!draw_templates)
	{
		input_name = "outputs/Control_Histograms" + this->filename_suffix + ".root";
	}
	cout<<"Reading file "<<input_name<<endl;
	TFile* file_input = 0;
	file_input = TFile::Open( input_name.Data() );
	if(file_input == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

	// TFile* file2 = 0;
	// file2 = TFile::Open( "outputs/files_noMEM/fakesNewNew/Combine_Input.root" );
	// if(file2 == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

	mkdir("plots",0777);
	TH1::SetDefaultSumw2();

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


	for(int ivar=0; ivar<total_var_list.size(); ivar++)
	{
		TH1F *h_tmp = 0, *h_signal  = 0, *h_fakes  = 0, *h_sum_background = 0, *h_fakes2=0;

		TString varname_tmp = total_var_list[ivar];

		//TLegend* qw = new TLegend(.80,.60,.95,.90);
		TLegend* qw = new TLegend(.6,.77,0.85,.915);
		qw->SetShadowColor(0);
		qw->SetFillColor(0);
		qw->SetLineColor(0);
		// qw->SetLineWidth(3);
		qw->SetTextSize(0.03);

		if(draw_templates) {varname_tmp = template_name;}

		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			if(channel != "allchan" && channel_list[ichan] != channel) {continue;}

		//--- All MC samples but fakes
			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				h_tmp = 0;

				if(sample_list[isample].Contains("Data") ) {continue;}

				TString histo_name = varname_tmp + "_" + channel_list[ichan] + "__" + sample_list[isample];
				if(!file_input->GetListOfKeys()->Contains(histo_name.Data()) && !sample_list[isample].Contains("Fakes") ) {cout<<histo_name<<" : not found"<<endl; continue;}

				if(!sample_list[isample].Contains("Fakes") && !sample_list[isample].Contains("tZq") )
				{
					h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();

					if(h_sum_background == 0) {h_sum_background = (TH1F*) h_tmp->Clone();}
					else {h_sum_background->Add(h_tmp);}
				}
				else if(sample_list[isample].Contains("Fakes") ) //fakes
				{
					if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
					else
					{
						h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();

						if(h_fakes == 0) {h_fakes = (TH1F*) h_tmp->Clone();}
						else {h_fakes->Add(h_tmp);}
					}
				}
				else if(sample_list[isample].Contains("tZq") ) //Signal
				{
					h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();

					if(h_signal == 0) {h_signal = (TH1F*) h_tmp->Clone();}
					else {h_signal->Add(h_tmp);}
				}
			} //end sample loop


		} //end channel loop

		if(normalized)
		{
			h_sum_background->Scale(1./h_sum_background->Integral() );
			h_signal->Scale(1./h_signal->Integral() );
			h_fakes->Scale(1./h_fakes->Integral() );
		}


		h_sum_background->SetLineColor(kViolet);
		h_sum_background->SetLineWidth(2);

		h_fakes->SetLineColor(kAzure-2);
		h_fakes->SetLineWidth(2);

		h_signal->SetLineColor(kGreen+2);
		h_signal->SetLineWidth(2);


		h_signal->GetXaxis()->SetTitleOffset(1.4);
		h_signal->GetXaxis()->SetLabelSize(0.045);
		h_signal->GetXaxis()->SetLabelFont(42);
		h_signal->GetXaxis()->SetLabelSize(0.05);
		h_signal->GetYaxis()->SetLabelSize(0.048);
		h_signal->GetXaxis()->SetTitleFont(42);
		h_signal->GetYaxis()->SetTitleOffset(1.42);
		h_signal->GetYaxis()->SetLabelFont(42);
		h_signal->GetYaxis()->SetTitleFont(42);
		h_signal->GetYaxis()->SetTitleSize(0.04);

		h_signal->SetMinimum(0);
		h_signal->SetMaximum(h_signal->GetMaximum()*1.3);
		h_signal->GetYaxis()->SetTitle("Events");

		TString title_tmp = "";
		if(varname_tmp=="BDT") title_tmp = "BDT output (1 tag)";
		else if(varname_tmp=="BDTttZ") title_tmp = "BDT output (2 tags)";
		else if(varname_tmp=="mTW") title_tmp = "mTW [GeV]";

		//--------------------------
		//SET VARIABLES LABELS
		//--------------------------

		if(varname_tmp == "mTW")                       {       title_tmp = "M_{T}_{W} [GeV]"; }
		else if(varname_tmp == "METpt")                {       title_tmp = "E_{T}^{miss} [GeV]"; }
		else if(varname_tmp == "m3l")          {       title_tmp = "Trilepton mass [GeV]"; }
		else if(varname_tmp == "ZMass")                {       title_tmp = "M_{Z} [GeV]"; }
		else if(varname_tmp == "dPhiAddLepB")  {       title_tmp = "#Delta#Phi(l_{W},b)"; }
		else if(varname_tmp == "Zpt")          {       title_tmp = "Z p_{T} [GeV]"; }
		else if(varname_tmp == "ZEta")         {       title_tmp = "Z #eta"; }
		else if(varname_tmp == "AddLepAsym")   {       title_tmp = " q_{#font[12]{l}}|#eta(#font[12]{l})|"; }
		else if(varname_tmp == "mtop")                 {       title_tmp = "M_{top} [GeV]"; }
		else if(varname_tmp == "btagDiscri")   {       title_tmp = "CSVv2 discriminant"; }
		else if(varname_tmp == "etaQ")         {       title_tmp = "#eta(#font[12]{j'})"; }
		else if(varname_tmp == "NBJets")               {       title_tmp = "B Jets multiplicity"; }
		else if(varname_tmp == "AddLepPT")             {       title_tmp = "l_{W} p_{T} [GeV]"; }
		else if(varname_tmp == "AddLepETA")            {       title_tmp = "l_{W} #eta"; }
		else if(varname_tmp == "LeadJetPT")            {       title_tmp = "p_{T}(#font[12]{j'}) [GeV]"; }
		else if(varname_tmp == "LeadJetEta")           {       title_tmp = "Leading jet #eta"; }
		else if(varname_tmp == "dPhiZMET")             {       title_tmp = "#Delta#Phi(Z,E_{T}^{miss})"; }
		else if(varname_tmp == "dPhiZAddLep")  {       title_tmp = "#Delta#Phi(Z,l_{W})"; }
		else if(varname_tmp == "dRAddLepB")            {       title_tmp = "#Delta R(b,l_{W})"; }
		else if(varname_tmp == "dRZAddLep")            {       title_tmp = "#Delta R(Z,l_{W})"; }
		else if(varname_tmp == "dRZTop")               {       title_tmp = "#Delta R(Z,top)"; }
		else if(varname_tmp == "TopPT")                {       title_tmp = "top p_{T} [GeV]"; }
		else if(varname_tmp == "NJets")                {       title_tmp = "Jets multiplicity"; }
		else if(varname_tmp == "ptQ")          {       title_tmp = "p_{T}(#font[12]{j'}) [GeV]"; }
		else if(varname_tmp == "dRjj")         {       title_tmp = "#Delta R(b,#font[12]{j'})"; }
		else if(varname_tmp == "AdditionalEleIso")     {       title_tmp = "e_{W} isolation"; }
		else if(varname_tmp == "AdditionalMuonIso")    {       title_tmp = "#mu_{W} isolation"; }
		else if(varname_tmp == "dRAddLepQ")            {       title_tmp = "#Delta R(l_{W},#font[12]{j'})"; }
		else if(varname_tmp == "dRAddLepClosestJet")   {       title_tmp = "#Delta R(closest jet, l_{W})"; }
		else if(varname_tmp == "tZq_pT")               {       title_tmp = "tZq system p_{T} [GeV]"; }
		else if(varname_tmp == "tZq_mass")             {       title_tmp = "tZq system mass [GeV]"; }
		else if(varname_tmp == "leadingLeptonPT")    {       title_tmp = "Leading lepton p_{T} [GeV]"; }
		else if(varname_tmp == "MAddLepB")           {       title_tmp = "l_{W}+b system mass "; }
		else if(varname_tmp == "MEMvar_0")             {       title_tmp = "MEM LR tZq-ttZ"; }
		else if(varname_tmp == "MEMvar_1")             {       title_tmp = "MEM Kin w_{tZq}"; }
		else if(varname_tmp == "MEMvar_2")             {       title_tmp = "MEM Kin w_{ttZ}"; }
		else if(varname_tmp == "MEMvar_3")             {       title_tmp = "MEM LR tZq-ttZ"; }
		else if(varname_tmp == "MEMvar_4")             {       title_tmp = "MEM w_{WZ}"; }
		else if(varname_tmp == "MEMvar_5")             {       title_tmp = "MEM Kin w_{WZ}"; }
		else if(varname_tmp == "MEMvar_6")             {       title_tmp = "MEM LR tZq-ttZ-WZ"; }
		else if(varname_tmp == "MEMvar_7")             {       title_tmp = "MEM LR tZq-WZ"; }
		else if(varname_tmp == "MEMvar_8")             {       title_tmp = "MEM LR tZq-ttZ-WZ"; }
		else if(varname_tmp == "tZ_mass")             {       title_tmp = "M_{tZ}"; }
		else if(varname_tmp == "tZ_pT")             {       title_tmp = "tZ_{p_{T}} [GeV]"; }
		else {       title_tmp = varname_tmp ; }

		h_signal->GetXaxis()->SetTitle(title_tmp);


		//Canvas definition
		Load_Canvas_Style();
		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);

		if(h_sum_background->GetMaximum() > h_signal->GetMaximum() && h_sum_background->GetMaximum() > h_fakes->GetMaximum())
		{
			h_signal->SetMaximum(h_sum_background->GetMaximum()*1.2);
		}
		else if(h_fakes->GetMaximum() > h_signal->GetMaximum() && h_fakes->GetMaximum() > h_sum_background->GetMaximum())
		{
			h_signal->SetMaximum(h_fakes->GetMaximum()*1.2);
		}

		if(draw_templates)
		{
			h_signal->Draw("hist E");
			h_sum_background->Draw("same hist E");
			h_fakes->Draw("same hist E");
			// h_fakes2->Draw("same hist E");
		}
		else
		{
			h_signal->Draw("hist");
			h_sum_background->Draw("same hist");
			h_fakes->Draw("same hist");
			// h_fakes2->Draw("same hist");
		}

		qw->AddEntry(h_signal, "tZq" , "L");
		qw->AddEntry(h_fakes, "Non-prompt" , "L");
		// qw->AddEntry(h_fakes2, "Non-prompt (new)" , "L");
		qw->AddEntry(h_sum_background, "Other backgrounds" , "L");

		//-------------------
		//CAPTIONS
		//-------------------

		// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines //CHANGED

		TString cmsText     = "CMS";
		float cmsTextFont   = 61;  // default is helvetic-bold

		bool writeExtraText = false;
		TString extraText   = "Preliminary";
		float extraTextFont = 52;  // default is helvetica-italics

		// text sizes and text offsets with respect to the top frame
		// in unit of the top margin size
		float lumiTextSize     = 0.6;
		float lumiTextOffset   = 0.2;
		float cmsTextSize      = 0.75;
		float cmsTextOffset    = 0.1;  // only used in outOfFrame version

		float relPosX    = 0.045;
		float relPosY    = 0.035;
		float relExtraDY = 1.2;

		// ratio of "CMS" and extra text size
		float extraOverCmsTextSize  = 0.76;

		float lumi = 35.9 * luminosity_rescale; //CHANGED
		TString lumi_13TeV = Convert_Number_To_TString(lumi);
		lumi_13TeV += " fb^{-1} (13 TeV)";

		TLatex latex;
		latex.SetNDC();
		latex.SetTextAngle(0);
		latex.SetTextColor(kBlack);

		float H = c1->GetWh();
		float W = c1->GetWw();
		float l = c1->GetLeftMargin();
		float t = c1->GetTopMargin();
		float r = c1->GetRightMargin();
		float b = c1->GetBottomMargin();

		float extraTextSize = extraOverCmsTextSize*cmsTextSize;

		latex.SetTextFont(42);
		latex.SetTextAlign(31);
		latex.SetTextSize(lumiTextSize*t);
		//	Change position w.r.t. CMS recommendation, only for control plots
		//      latex.DrawLatex(1-r,1-t+lumiTextOffset*t,lumi_13TeV);
		// latex.DrawLatex(0.7,1-t+lumiTextOffset*t,lumi_13TeV);
		latex.DrawLatex(0.8,1-t+lumiTextOffset*t,lumi_13TeV); //CHANGED

		latex.SetTextFont(cmsTextFont);
		latex.SetTextAlign(11);
		latex.SetTextSize(cmsTextSize*t);
		latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);

		//float posX_ =   l +  relPosX*(1-l-r);
		//float posY_ =   1-t+lumiTextOffset*t;

		latex.SetTextFont(extraTextFont);
		//latex.SetTextAlign(align_);
		latex.SetTextSize(extraTextSize*t);
		latex.DrawLatex(l+cmsTextSize*l, 1-t+lumiTextOffset*t, extraText);


		//------------------

		TString info_data;
		if (channel=="eee")    info_data = "eee channel";
		else if (channel=="eeu")  info_data = "ee#mu channel";
		else if (channel=="uue")  info_data = "#mu#mu e channel";
		else if (channel=="uuu") info_data = "#mu#mu #mu channel";
		else if(channel=="allchan") info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";

		TString extrainfo_data;
		if(isWZ) {extrainfo_data= "3l,>0j,0bj";}
		else if(isttZ) {extrainfo_data= "3l,>1j,>1bj";}
		else if(!isWZ && !isttZ) {extrainfo_data= "3l,2-3j,1bj";}

		TLatex text2 ; //= new TLatex(0, 0, info_data);
		text2.SetNDC();
		text2.SetTextAlign(13);
		text2.SetTextFont(42);
		text2.SetTextSize(0.04);
		text2.DrawLatex(0.195,0.91,info_data);

		text2.SetTextFont(62);
		text2.DrawLatex(0.63,0.9,extrainfo_data);

		qw->Draw(); //Draw legend

	//------------------

		mkdir("plots",0777); //Create directory if inexistant
		mkdir("plots/Superpose",0777); //Create directory if inexistant

		if(draw_templates)
		{
			mkdir("plots/Superpose/templates",0777); //Create directory if inexistant
			mkdir("plots/Superpose/templates/allchans",0777); //Create directory if inexistant
		}
		else
		{
			mkdir("plots/Superpose/input_vars",0777); //Create directory if inexistant
			mkdir("plots/Superpose/input_vars/allchans",0777); //Create directory if inexistant
		}

		//Output
		TString output_plot_name = "plots/Superpose/";
		if(draw_templates) {output_plot_name+= "templates/";}
		else {output_plot_name+= "input_vars/";}
		if(channel.Contains("all")) output_plot_name+= "allchans/";

		if(normalized) output_plot_name+= "SuperposeFakes_" + varname_tmp +"_" + channel + this->filename_suffix + "_norm" + this->format;
		else output_plot_name+= "SuperposeFakes_" + varname_tmp +"_" + channel + this->filename_suffix + this->format;

		c1->SaveAs(output_plot_name.Data());

		delete h_fakes; delete h_signal; delete h_sum_background; delete h_tmp;
		delete qw;
		delete c1;

		if(draw_templates) {break;} //don't look on variable if only want to plot template
	}

	delete file_input;

	return;
}









/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////



















//-----------------------------------------------------------------------------------------
//  #######  ######## ##     ## ######## ########
// ##     ##    ##    ##     ## ##       ##     ##
// ##     ##    ##    ##     ## ##       ##     ##
// ##     ##    ##    ######### ######   ########
// ##     ##    ##    ##     ## ##       ##   ##
// ##     ##    ##    ##     ## ##       ##    ##
//  #######     ##    ##     ## ######## ##     ##
//-----------------------------------------------------------------------------------------





//-----------------------------------------------------------------------------------------
//  ######   #######  ##     ## ########  #### ##    ## ########     ######  ####  ######   ##    ## #### ########
// ##    ## ##     ## ###   ### ##     ##  ##  ###   ## ##          ##    ##  ##  ##    ##  ###   ##  ##  ##
// ##       ##     ## #### #### ##     ##  ##  ####  ## ##          ##        ##  ##        ####  ##  ##  ##
// ##       ##     ## ## ### ## ########   ##  ## ## ## ######       ######   ##  ##   #### ## ## ##  ##  ######
// ##       ##     ## ##     ## ##     ##  ##  ##  #### ##                ##  ##  ##    ##  ##  ####  ##  ##
// ##    ## ##     ## ##     ## ##     ##  ##  ##   ### ##          ##    ##  ##  ##    ##  ##   ###  ##  ##
//  ######   #######  ##     ## ########  #### ##    ## ########     ######  ####  ######   ##    ## #### ##
//-----------------------------------------------------------------------------------------
/**
 * Run Combine on template file (arg), using datacard corresponding to tZq signal
 * @param  path_templatefile path of the templatefile on which to run Combine
 * @return                   computed significance value
 */
float theMVAtool::Compute_Combine_Significance_From_TemplateFile(TString path_templatefile, TString signal, TString channel, bool expected, bool use_syst)
{
	CopyFile(path_templatefile, "./COMBINE/templates/Combine_Input.root"); //Copy file to templates dir.

	TString datacard_path = "./COMBINE/datacards/COMBINED_datacard_TemplateFit_tZq";

	if(channel == "uuu" || channel == "eeu" || channel == "uue" || channel == "eee")
	{
		datacard_path+= "_"+channel;
	}
	else if(channel != "") {cout<<"Wrong channel name ! Abort"<<endl; return 0;}

	if(!use_syst) datacard_path+= "_noSyst";

	datacard_path+=".txt";

	TString f_tmp_name = "significance_expected_info_tmp.txt";

	if(expected) {system( ("combine -M ProfileLikelihood --significance -t -1 --expectSignal=1 "+datacard_path+" | tee "+f_tmp_name).Data() );} //Expected a-priori
	else {system( ("combine -M ProfileLikelihood --signif --cminDefaultMinimizerType=Minuit2 "+datacard_path+" | tee "+f_tmp_name).Data() );} //Observed


	ifstream file_in(f_tmp_name.Data()); //Store output from combine in text file, read it & extract value
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






//-----------------------------------------------------------------------------------------
// ########  ######## ########  #### ##    ##    ######## ######## ##     ## ########  ##          ###    ######## ########  ######
// ##     ## ##       ##     ##  ##  ###   ##       ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
// ##     ## ##       ##     ##  ##  ####  ##       ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
// ########  ######   ########   ##  ## ## ##       ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
// ##   ##   ##       ##     ##  ##  ##  ####       ##    ##       ##     ## ##        ##       #########    ##    ##             ##
// ##    ##  ##       ##     ##  ##  ##   ###       ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
// ##     ## ######## ########  #### ##    ##       ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//-----------------------------------------------------------------------------------------



/**
 * Takes input file, rebins all templates inside it, and store them in a separate output file
 */
void theMVAtool::Rebin_Template_File(TString filepath, int nbins)
{
	if(!Check_File_Existence(filepath)) {cout<<filepath<<" not found ! Abort"<<endl; return;}

	TFile* f = TFile::Open(filepath);

	TString output_filename = filepath;
	int i = output_filename.Index(".root"); //Find index of substring
	output_filename.Remove(i); //Remove substring
	output_filename+= "_"+Convert_Number_To_TString(nbins)+"Bins.root";

	TFile* f_output = TFile::Open( output_filename, "RECREATE");

	vector<TString> template_list;
	template_list.push_back("mTW");
	template_list.push_back("BDT");
	template_list.push_back("BDTttZ");

	TH1F* h = 0;
	TH1F* hnew = 0; //re-binned histo


	for(int itemp=0; itemp<template_list.size(); itemp++)
	{
		for(int isample = 0; isample<sample_list.size(); isample++)
		{
			for(int isyst=0; isyst<syst_list.size(); isyst++)
			{
				if(sample_list[isample] == "STtWll" && (syst_list[isyst].Contains("Q2") || syst_list[isyst].Contains("pdf") ) ) {continue;}
				if( (sample_list[isample] == "Data" && syst_list[isyst]!="")
					|| ( sample_list[isample].Contains("Fakes") && syst_list[isyst]!="" && !syst_list[isyst].Contains("Fakes") ) ) {continue;} //Data = no syst. -- Fakes = only fake syst.
				if( syst_list[isyst].Contains("Fakes") && !sample_list[isample].Contains("Fakes") )   {continue;} //Fake syst. only for "fakes" samples
				if( (syst_list[isyst].Contains("PSscale") || syst_list[isyst].Contains("Hadron") ) && !sample_list[isample].Contains("tZq") ) {continue;} //available for signal only

				for(int ichan=0; ichan<channel_list.size(); ichan++)
				{
					h=0; hnew=0;

					TString h_name = template_list[itemp]+"_"+channel_list[ichan]+"__";
					if(sample_list[isample].Contains("Data")) h_name+= "data_obs";
					else h_name+= sample_list[isample];

					if(syst_list[isyst] != "") {h_name+= "__"+Combine_Naming_Convention(syst_list[isyst]);}

					if(!f->GetListOfKeys()->Contains(h_name) ) {cout<<h_name.Data()<<" not found !"<<endl;  continue;}

					h = (TH1F*) f->Get(h_name);

					int current_binning = h->GetNbinsX();

					double rebin_ratio = (double) current_binning / nbins;

					hnew = (TH1F*) h->Clone();
					hnew->Rebin(rebin_ratio);

					for(int n = 1; n<=hnew->GetNbinsX(); n++)
					{
						if(hnew->GetBinContent(n) < 0) hnew->SetBinContent(n, 0);
					}


					f_output->cd();
					hnew->Write(h_name, TObject::kOverwrite);
				} //chan
			} //syst
		} //sample

		cout<<"--- Done with "<<template_list[itemp]<<"templates !"<<endl;

	} //template

	f->Close(); f_output->Close();
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

//  #######  ########   ######   #######  ##       ######## ######## ########
// ##     ## ##     ## ##    ## ##     ## ##       ##          ##    ##
// ##     ## ##     ## ##       ##     ## ##       ##          ##    ##
// ##     ## ########   ######  ##     ## ##       ######      ##    ######
// ##     ## ##     ##       ## ##     ## ##       ##          ##    ##
// ##     ## ##     ## ##    ## ##     ## ##       ##          ##    ##
//  #######  ########   ######   #######  ######## ########    ##    ########


// ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##  ######
// ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ## ##    ##
// ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ## ##
// ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##  ######
// ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##
// ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ### ##    ##
// ##        #######  ##    ##  ######     ##    ####  #######  ##    ##  ######

// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######
// ####### ####### ####### ####### ####### ####### ####### ####### ####### #######


//Place here all the functions which are being tested or which are needed only temporarily




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

void theMVAtool::Compare_Negative_Or_Absolute_Weight_Effect_On_Distributions(TString channel, bool allchannels)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Compare Input Variables using Weight or fabs(Weight) ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;


	//File containing histograms of input vars, w/ ALL events
	TString input_file_name = "outputs/files_nominal/Controlfiles_allEvents/";
	input_file_name+= "Control_Histograms" + this->filename_suffix + ".root";
	TFile* f_neg = 0;
	f_neg = TFile::Open( input_file_name );
	//File containing histograms of input vars, w/ ALL events but fabs(weight)
	input_file_name = "outputs/files_nominal/Controlfiles_fabsEvents/";
	input_file_name+= "Control_Histograms" + this->filename_suffix + ".root";
	TFile* f_abs = 0;
	f_abs = TFile::Open( input_file_name );

	if(!f_neg || !f_abs) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

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
	samples.push_back("tZqmcNLO");
	samples.push_back("WZL");
	samples.push_back("WZB");
	samples.push_back("WZC");
	samples.push_back("ttZ");

	//Load Canvas definition
	Load_Canvas_Style();

	TH1F *h_tmp_neg = 0, *h_tmp_abs = 0;
	TH1F *h_neg = 0, *h_abs = 0;

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

			h_neg = 0; h_abs = 0;

			//TLegend* qw = new TLegend(.80,.60,.95,.90);
			qw = new TLegend(.7,.7,0.965,.915);
			qw->SetShadowColor(0);
			qw->SetFillColor(0);
			qw->SetLineColor(0);

			int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
			for(int ichan=0; ichan<thechannellist.size(); ichan++)
			{
				if(!allchannels && channel != thechannellist[ichan]) {continue;} //If plot single channel


				h_tmp_neg = 0;
				h_tmp_abs = 0;

				TString histo_name = total_var_list[ivar] + "_" + thechannellist[ichan] + "__" + samples[isample];

				if(!f_neg->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
				h_tmp_neg = (TH1F*) f_neg->Get(histo_name.Data())->Clone();

				if(!f_abs->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
				h_tmp_abs = (TH1F*) f_abs->Get(histo_name.Data())->Clone();


				//Reference = distrib. with ALL events (relative weights)
				h_tmp_neg->SetFillStyle(3345);
				h_tmp_neg->SetFillColor(kRed);
				h_tmp_neg->SetLineColor(kRed);

				h_tmp_abs->SetLineColor(kGreen+2);

				if(niter_chan == 0) {h_neg = (TH1F*) h_tmp_neg->Clone();  h_abs = (TH1F*) h_tmp_abs->Clone();}
				else //Sum channels
				{
					h_neg->Add(h_tmp_neg); h_abs->Add(h_tmp_abs);
				}

			niter_chan++;
			} //end channel loop

			qw->AddEntry(h_neg, "ALL events", "F");
			qw->AddEntry(h_abs, "All events, fabs(weight)", "L");

			h_neg->Scale(1./h_neg->Integral());
			h_abs->Scale(1./h_abs->Integral());

			if(h_neg->GetMaximum() < h_abs->GetMaximum()) {h_neg->SetMaximum(h_abs->GetMaximum() * 1.2);}

			c1->cd();

			TString title = total_var_list[ivar] + " / " + samples[isample] ;
			h_neg->GetXaxis()->SetTitle((title.Data()));

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

	delete h_neg; delete h_abs;
	delete h_tmp_neg; delete h_tmp_abs;
	delete qw;
	delete f_neg; delete f_abs;

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


//Place here all the functions which are being tested or which are needed only temporarily




void theMVAtool::Fit_mTW_SR(TString channel)
{
	if(channel!="uuu" && channel!="eee" && channel!="eeu" && channel!="uue" && channel!="allchan") {cout<<__LINE__<<" : "<<"Wrong channel name !"; return;}

	TString file_name = "outputs/Control_Histograms"+this->filename_suffix_noJet+"_NJetsMin0_NBJetsEq0.root";
	if(!Check_File_Existence(file_name)) {cout<<file_name<<" doesn't exist !"<<endl; return;}
	TFile* f_WZ = TFile::Open(file_name);
	file_name = "outputs/Control_Histograms"+this->filename_suffix_noJet+"_NJetsMin1Max4_NBJetsEq1.root";
	if(!Check_File_Existence(file_name)) {cout<<file_name<<" doesn't exist !"<<endl; return;}
	TFile* f_tZq = TFile::Open(file_name);
	file_name = "outputs/Control_Histograms"+this->filename_suffix_noJet+"_NJetsMin1_NBJetsMin1.root";
	if(!Check_File_Existence(file_name)) {cout<<file_name<<" doesn't exist !"<<endl; return;}
	TFile* f_ttZ = TFile::Open(file_name);

	TH1F *h_tZq=0, *h_ttZ=0, *h_WZ=0, *h_tmp=0;

	cout<<endl<<endl<<"---------------------------"<<endl;

	cout<<"Fitting (mTW_WZ + mTW_ttZ) to mTW_tZq"<<endl<<endl;

	//Treat Data / Fakes / Other samples separately
	TString histo_name;

	vector<TString> samples;
	samples.push_back("FakesElectron");
	samples.push_back("FakesMuon");

	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		if(channel != "allchan" && channel_list[ichan] != channel) {continue;}

		for(int isample=0; isample<samples.size(); isample++)
		{
			h_tmp = 0;

			TString h_name = "mTW_" + channel_list[ichan] + "__" + samples[isample];

			h_tmp = (TH1F*) f_WZ->Get(h_name);
			if(h_WZ==0) {h_WZ = (TH1F*) h_tmp->Clone();}
			else {h_WZ->Add(h_tmp);}

			h_tmp = (TH1F*) f_tZq->Get(h_name);
			if(h_tZq==0) {h_tZq = (TH1F*) h_tmp->Clone();}
			else {h_tZq->Add(h_tmp);}

			h_tmp = (TH1F*) f_ttZ->Get(h_name);
			if(h_ttZ==0) {h_ttZ = (TH1F*) h_tmp->Clone();}
			else {h_ttZ->Add(h_tmp);}
		}
	}


	TObjArray *mc = new TObjArray(2);
	mc->Add(h_WZ); //Param 0
	mc->Add(h_ttZ); //Param 1

	TFractionFitter* fit = new TFractionFitter(h_tZq, mc, "Q"); //'Q' for quiet

	TFitResultPtr r = fit->Fit(); //Create smart ptr to TFitResult --> can access fit properties

	TCanvas* c1 = new TCanvas("c1");

	TH1F* result = (TH1F*) fit->GetPlot();
	h_tZq->Draw("Ep");
	result->Draw("same");

	mkdir("plots",0777); //Create directory if inexistant

	c1->SaveAs(("plots/ScaleMTW_"+channel+".png").Data()); //Save fit plot

	cout<<endl<<endl;

	cout<<BOLD(FYEL("RESULT FIT "<<channel<<" "))<<endl;
	cout<<"Fraction WZ : "<<r->Parameter(0)<<endl;
	cout<<"Fraction ttZ : "<<r->Parameter(1)<<endl;

	delete mc; delete fit; delete c1;
}
