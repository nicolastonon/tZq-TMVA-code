#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <vector>
#include <sys/stat.h> // to be able to use mkdir
#include <sstream>

#include <TROOT.h>
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

/**
 * Take ntuples containing all MEM vars + analysis vars & outputs ntuple containing only vars needed for rest of the analysis (also take log of MEM vars, etc.)
 * @param sample     sample name
 * @param thevarlist list of vars NOT related to MEM to be in the output ntuple
 * @param MEMvarlist list of vars RELATED to MEM to be in the output ntuple
 */
void Modify_Ntuples(TString sample, vector<TString> thevarlist, vector<TString> MEMvarlist, vector<TString> MEMvarlist_new, vector<TString> weight_syst_list, vector<TString> tree_syst_list, TString MEM_or_WZ, TString tmp_dir)
{
	vector<float> v_floats; //Store all other variables as floats
	vector<double> v_double_MEM; //Store MEM variables as doubles
	vector<float> v_floats_MEM; //Needed to convert MEM variables from doubles to floats (needed by BDT code as floats)
	vector<float> v_floats_modif; //Store all other variables in floats
	vector<float> v_floats_syst; //Store syst weights

	//Input
	TString input_filename;
//
	// FIXME -- Path of Ntuples with MEM
	if(MEM_or_WZ == "MEM")
	{
		input_filename = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_withMEM/";
		// input_filename = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_readyForMEM/";
		if(tmp_dir != "") input_filename+= tmp_dir+"/";
		input_filename+= "FCNCNTuple_" + sample +  ".root";
	}


	//Path of Ntuples for WZ CR
	else if(MEM_or_WZ == "WZ")
	{
		input_filename = "/home/nico/Bureau/these/tZq/MEM_Interfacing/output_ntuples/ntuples_WZ/";
		if(tmp_dir != "") input_filename+= tmp_dir+"/";
		input_filename+= "FCNCNTuple_" + sample +  ".root";
	}

	TFile* f_input = 0;
  	f_input = new TFile(input_filename.Data()); if(!f_input || f_input->IsZombie() ) {cout<<"Can't find input file !"<<endl; return;}

	//Output

	if(tmp_dir != "")
	{
		TString create_dir = "input_ntuples/"+tmp_dir;
		mkdir(create_dir.Data(),0755);
		create_dir = "input_ntuples/"+tmp_dir+"/ntuples_MEM";
		mkdir(create_dir.Data(),0755);
		create_dir = "input_ntuples/"+tmp_dir+"/ntuples_WZ";
		mkdir(create_dir.Data(),0755);
	}
	else
	{
		TString create_dir = "input_ntuples/";
		mkdir(create_dir.Data(),0755);
		create_dir = "input_ntuples/ntuples_MEM";
		mkdir(create_dir.Data(),0755);
		create_dir = "input_ntuples/ntuples_WZ";
		mkdir(create_dir.Data(),0755);
	}

	TString output_filename;
	if(MEM_or_WZ == "MEM")
	{
		output_filename = "input_ntuples/";
		if(tmp_dir != "") output_filename+= tmp_dir+"/";
		output_filename+= "ntuples_MEM/FCNCNTuple_" + sample +  ".root";
		// if(sample=="tWZ") output_filename 	= "input_ntuples/ntuples_MEM/FCNCNTuple_STtWll.root";
	}
	else if(MEM_or_WZ == "WZ")
	{
		output_filename 	= "input_ntuples/";
		if(tmp_dir != "") output_filename+= tmp_dir+"/";
		output_filename+= "ntuples_WZ/FCNCNTuple_" + sample +  ".root";
		// if(sample=="tWZ") output_filename 	= "input_ntuples/ntuples_WZ/FCNCNTuple_STtWll.root";

	}
	else {cout<<"ERROR -- abort !"<<endl; return;}

	cout<<endl<<endl<<"--- Creating new Ntuple from "<<input_filename.Data()<<endl;
cout<<__LINE__<<endl;
	TFile* f_output = TFile::Open( output_filename, "RECREATE" );
cout<<__LINE__<<endl;
	for(int itreesyst = 0; itreesyst<tree_syst_list.size(); itreesyst++)
	{
		//Only 'Tree' tree for data and fakes (+'Fakes' syst tree for fakes)
	    if( (sample.Contains("Data") && tree_syst_list[itreesyst] != "Tree") || ( sample.Contains("Fakes") && tree_syst_list[itreesyst] != "Tree" && !tree_syst_list[itreesyst].Contains("Fakes") ) ) {continue;}
	    if( tree_syst_list[itreesyst].Contains("Fakes") && !sample.Contains("Fakes") ) {continue;} //'Fakes' syst. tree only for Fakes sample

	    cout<<FGRN("SYSTEMATIC TREE NAME : ")<<tree_syst_list[itreesyst]<<endl;

		TTree* tree_modif = 0;
		tree_modif = new TTree(tree_syst_list[itreesyst].Data(), "");


		//Allocate memory/elements to vectors
		//NOTE : don't do it in same loop as Branch() !! (push_back can change vector address -->segfault)
		for(int i=0; i<thevarlist.size(); i++)
		{
			v_floats.push_back(-999);
		}
		for(int i=0; i<MEMvarlist.size(); i++)
		{
			v_floats_MEM.push_back(-999);
			v_double_MEM.push_back(-999);
		}
		for(int i=0; i<MEMvarlist_new.size(); i++)
		{
			v_floats_modif.push_back(-999);
		}
		for(int i=0; i<weight_syst_list.size(); i++)
		{
			if(sample=="Data" || sample=="Fakes" || tree_syst_list[itreesyst] != "Tree") break;
			v_floats_syst.push_back(-999);
		}

		//Prepare output tree
		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			tree_modif->Branch(thevarlist[ivar].Data(),&v_floats[ivar],(thevarlist[ivar]+"/F").Data());
		}
		for(int ivar=0; ivar<MEMvarlist.size(); ivar++)
		{
			tree_modif->Branch(MEMvarlist[ivar].Data(),&v_floats_MEM[ivar],(MEMvarlist[ivar]+"/F").Data());
		}
		for(int ivar=0; ivar<MEMvarlist_new.size(); ivar++)
		{
			tree_modif->Branch(MEMvarlist_new[ivar].Data(),&v_floats_modif[ivar],(MEMvarlist_new[ivar]+"/F").Data());
		}
		//Systematics renaming for Combine
		for(int isyst=0; isyst<weight_syst_list.size(); isyst++)
		{
			if(sample.Contains("Data") || sample.Contains("Fakes") || tree_syst_list[itreesyst] != "Tree") break; //No weight syst

			tree_modif->Branch(weight_syst_list[isyst].Data(),&v_floats_syst[isyst],(weight_syst_list[isyst]+"/F").Data());
		}

		//Set Branch Addresses
		TTree* t_input = 0;
		// if(MEM_or_WZ == "MEM")
		{
			t_input = (TTree*) f_input->Get(tree_syst_list[itreesyst].Data());

			if(tree_syst_list[itreesyst] == "Fakes__plus") {t_input = (TTree*) f_input->Get("Tree"); } //Fakes__plus = nominal !
			if(!t_input) {cout<<"Tree "<<tree_syst_list[itreesyst]<<" not found !"<<endl; return;}
		}
		// else if(MEM_or_WZ == "WZ") {t_input = (TTree*) f_input->Get("Default"); if(!t_input) {cout<<"Tree 'Default' not found !"<<endl; return;} }

		TLorentzVector* v_MET = new TLorentzVector; int index_MET_float = -99;
		t_input->SetBranchAddress("multilepton_mET", &v_MET); //FIXME -- need to find MET somewhere else for now

		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			if(thevarlist[ivar] == "METpt")
			{
				index_MET_float = ivar;
				continue;
			}
			else t_input->SetBranchAddress(thevarlist[ivar].Data(), &v_floats[ivar]);
		}
		for(int ivar=0; ivar<MEMvarlist.size(); ivar++)
		{
			if(MEM_or_WZ == "MEM") t_input->SetBranchAddress(MEMvarlist[ivar].Data(), &v_double_MEM[ivar]); //For SetBranchAddress, need to use doubles
		}
		for(int ivar=0; ivar<weight_syst_list.size(); ivar++)
		{
			if(sample=="Data" || sample=="Fakes" || tree_syst_list[itreesyst] != "Tree")  break;

			t_input->SetBranchAddress(weight_syst_list[ivar].Data(), &v_floats_syst[ivar]); //NOTE : new syst branches have different names (but associated to same floats)
		}



		//Event loop : modify vars, fill output tree
		int nentries = t_input->GetEntries();
		for(int ientry=0; ientry<nentries; ientry++)
		{
			if(ientry%10000==0) {cout<<ientry<<" / "<<nentries<<endl;}

			// if(ientry%2==0) {continue;} //CUT SAMPLES IN HALVES

			t_input->GetEntry(ientry);

			v_floats[index_MET_float] = v_MET->Energy(); //MET stored in a TLorentzVector

			// cout<<"v_floats[0] / "<<thevarlist[0].Data()<<" = "<<v_floats[0]<<endl;

			//Retrieve number of jets/bjets to be able to determine to which region an event belongs (see protections below)
			int njets = 0, nbjets = 0;
			for(int k=0; k<thevarlist.size(); k++)
			{
				if(thevarlist[k] == "NJets") njets = v_floats[k];
				else if(thevarlist[k] == "NBJets") nbjets = v_floats[k];
			}


			if(MEM_or_WZ == "MEM")
			{
//-------------------
				//NOTE : v_double_MEM numbering scheme :
				// MEMvarlist.push_back("mc_mem_ttz_weight"); //0
				// MEMvarlist.push_back("mc_mem_tllj_weight"); //1
				// MEMvarlist.push_back("mc_mem_tllj_weight_kinmaxint"); //2
				// MEMvarlist.push_back("mc_mem_ttz_weight_kinmaxint"); //3
				// MEMvarlist.push_back("mc_mem_ttz_tllj_likelihood"); //4
				// MEMvarlist.push_back("mc_mem_wzjj_weight"); //5
				// MEMvarlist.push_back("mc_mem_wzjj_weight_kinmaxint"); //6

//-------------------
				//---  BDT variables numbering scheme :

				// thevarlist.push_back("-log((3.89464e-13*mc_mem_ttz_weight) / (3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"); //0
				// thevarlist.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //1
				// thevarlist.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //2

				// thevarlist.push_back("log(mc_mem_wzjj_weight)"); //4
			    // thevarlist.push_back("log(mc_mem_wzjj_weight_kinmaxint)"); //5
			    // thevarlist.push_back("-log((0.017*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight) / (0.017*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"); //6
				// thevarlist.push_back("-log((0.017*mc_mem_wzjj_weight) / (0.017*mc_mem_wzjj_weight + 0.17993*mc_mem_tllj_weight) )"); //7
				// discriminant_TZQ_TTZandWZ_SR = "-log((0.017e-10*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight) / (0.017e-10*mc_mem_wzjj_weight + 3.89464e-13*mc_mem_ttz_weight + 0.17993*mc_mem_tllj_weight))"; //8

				// thevarlist_ttZ.push_back("-log(mc_mem_ttz_tllj_likelihood)"); //3
				// thevarlist_ttZ.push_back("log(mc_mem_tllj_weight_kinmaxint)"); //1
				// thevarlist_ttZ.push_back("log(mc_mem_ttz_weight_kinmaxint)"); //2
//-------------------

				v_floats_modif[0] = -log( (3.89464e-13*v_double_MEM[0]) / (3.89464e-13*v_double_MEM[0] + 0.17993*v_double_MEM[1]) );
				v_floats_modif[1] = log(v_double_MEM[2]);
				v_floats_modif[2] = log(v_double_MEM[3]);
				v_floats_modif[3] = -log(v_double_MEM[4]);

				v_floats_modif[4] = log(v_double_MEM[5]);
				v_floats_modif[5] = log(v_double_MEM[6]);

				v_floats_modif[6] = -log( (0.017*v_double_MEM[5] + 3.89464e-13*v_double_MEM[0]) / (0.17*v_double_MEM[5] + 3.89464e-13*v_double_MEM[0] + 0.17993*v_double_MEM[1]) );

				v_floats_modif[7] = -log((0.017*v_double_MEM[5]) / (0.017*v_double_MEM[5] + 0.17993*v_double_MEM[1]) );

				v_floats_modif[8] = -log( (0.017e-10*v_double_MEM[5] + 3.89464e-13*v_double_MEM[0]) / (0.17e-10*v_double_MEM[5] + 3.89464e-13*v_double_MEM[0] + 0.17993*v_double_MEM[1]) );

				//FIXME -- seem to be an error bc of some events ~ -700
				if(v_floats_modif[4] < -600) v_floats_modif[4] = 0;
				if(v_floats_modif[5] < -600) v_floats_modif[5] = 0;

				//Protection against inf and nan
				bool event_problem = false;
				for(int k=0; k<v_floats_modif.size(); k++)
				{
					if(njets>1 && njets<4 && nbjets==1 && (k==0 || k==1 || k==2 || k==4 || k==5 || k==6 || k==7))
					{
						if(isinf(v_floats_modif[k]) || isnan(v_floats_modif[k]) )
						{
							cout<<BOLD(FRED("PROBLEM (tZq region event) !  : v_floats_modif["<<k<<"] = "))<<v_floats_modif[k]<<" -- Ignore event !"<<endl;

							event_problem = true;
							break;
						}
					}
					else if(njets>1 && nbjets>1 && (k==1 || k==2 || k==3) )
					{
						if(isinf(v_floats_modif[k]) || isnan(v_floats_modif[k]) )
						{
							cout<<BOLD(FRED("PROBLEM (ttZ region event) !  : v_floats_modif["<<k<<"] = "))<<v_floats_modif[k]<<" -- Ignore event !"<<endl;

							event_problem = true;
							break;
						}
					}
				}
				if(event_problem) {continue;}


				for(int i=0; i<v_double_MEM.size(); i++)
				{
					v_floats_MEM[i] = (float) v_double_MEM[i]; //Convert from doubles to float --> Store as floats in output ntuple, so it has same format as all BDT vars
				}

			}

			tree_modif->Fill();
		}


		f_output->cd();
	  	tree_modif->Write(); //Write output tree in output ntuple

	  	tree_modif->Delete();
	} //end tree syst loop


	f_input->Close(); f_input->Delete();
	f_output->Close(); f_output->Delete();

	return;
}




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

	vector<TString> sample_list;
	// sample_list.push_back("Data");
	// sample_list.push_back("tZq");
	// sample_list.push_back("tZqmcNLO");
	// sample_list.push_back("WZL");
	// sample_list.push_back("WZB");
	// sample_list.push_back("WZC");
	// sample_list.push_back("ttZ");
	// sample_list.push_back("ttW");
	// sample_list.push_back("ttH");
	// sample_list.push_back("ZZ");
	// sample_list.push_back("STtWll");
	// sample_list.push_back("tWZ");
	// sample_list.push_back("tZqQup");
	// sample_list.push_back("tZqQdw");
	sample_list.push_back("Fakes");



//---------------------------------------------------------------------------
    //   ##     ##    ###    ########   ######
    //   ##     ##   ## ##   ##     ## ##    ##
    //   ##     ##  ##   ##  ##     ## ##
    //   ##     ## ##     ## ########   ######
    //    ##   ##  ######### ##   ##         ##
    //     ## ##   ##     ## ##    ##  ##    ##
    //      ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------

//NOTE : make sure the variable list is up-to-date compared to Ntuples' contents !!


	vector<TString> thevarlist;
	thevarlist.push_back("Weight");
	thevarlist.push_back("Channel");
	thevarlist.push_back("NJets");
	thevarlist.push_back("NBJets");
	thevarlist.push_back("mTW");
	thevarlist.push_back("METpt");

	thevarlist.push_back("btagDiscri");
	thevarlist.push_back("dRAddLepQ");
	thevarlist.push_back("dRAddLepClosestJet");
	thevarlist.push_back("dPhiAddLepB");
	thevarlist.push_back("ZEta");
	thevarlist.push_back("Zpt");
	thevarlist.push_back("mtop");
	thevarlist.push_back("AddLepAsym");
	thevarlist.push_back("etaQ");
	thevarlist.push_back("AddLepETA");
	thevarlist.push_back("LeadJetEta");
	thevarlist.push_back("dPhiZAddLep");
	thevarlist.push_back("dRZAddLep"); // --> little discrim --> to be included
	thevarlist.push_back("dRjj");
	thevarlist.push_back("ptQ"); // --> little discrim
	thevarlist.push_back("tZq_pT");
	thevarlist.push_back("dRAddLepB");
	thevarlist.push_back("TopPT"); // low discri power
	thevarlist.push_back("m3l");
	thevarlist.push_back("dRZTop");


	//--- MEM variables (weights, ...) which we modify into new variables in the code
	vector<TString> MEMvarlist; //FIXME add new MEM vars
	MEMvarlist.push_back("mc_mem_ttz_weight");
	MEMvarlist.push_back("mc_mem_tllj_weight");
	MEMvarlist.push_back("mc_mem_tllj_weight_kinmaxint");
	MEMvarlist.push_back("mc_mem_ttz_weight_kinmaxint");
	MEMvarlist.push_back("mc_mem_ttz_tllj_likelihood");

	MEMvarlist.push_back("mc_mem_wzjj_weight");
	MEMvarlist.push_back("mc_mem_wzjj_weight_kinmaxint");


	//--- Names of new MEM variable (used directly in BDT)
	vector<TString> MEMvarlist_new;
	MEMvarlist_new.push_back("MEMvar_0");
	MEMvarlist_new.push_back("MEMvar_1");
	MEMvarlist_new.push_back("MEMvar_2");
	MEMvarlist_new.push_back("MEMvar_3");

	MEMvarlist_new.push_back("MEMvar_4");
	MEMvarlist_new.push_back("MEMvar_5");
	MEMvarlist_new.push_back("MEMvar_6");
	MEMvarlist_new.push_back("MEMvar_7");
	MEMvarlist_new.push_back("MEMvar_8");



//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------
//--- Stored in different trees
	vector<TString> tree_syst_list;
	tree_syst_list.push_back("Tree"); //NOTE -- KEEP THIS LINE : nominal

	tree_syst_list.push_back("JER__plus"); tree_syst_list.push_back("JER__minus");
	tree_syst_list.push_back("JES__plus"); tree_syst_list.push_back("JES__minus");
	tree_syst_list.push_back("Fakes__plus"); tree_syst_list.push_back("Fakes__minus");


	vector<TString> weight_syst_list;
//--- Stored as separate weights
	weight_syst_list.push_back("Q2__plus"); weight_syst_list.push_back("Q2__minus");
	weight_syst_list.push_back("PU__plus"); weight_syst_list.push_back("PU__minus");
	weight_syst_list.push_back("MuEff__plus"); weight_syst_list.push_back("MuEff__minus");
	weight_syst_list.push_back("EleEff__plus"); weight_syst_list.push_back("EleEff__minus");
	weight_syst_list.push_back("pdf__plus"); weight_syst_list.push_back("pdf__minus");
	weight_syst_list.push_back("LFcont__plus"); weight_syst_list.push_back("LFcont__minus");
	weight_syst_list.push_back("HFstats1__plus"); weight_syst_list.push_back("HFstats1__minus");
	weight_syst_list.push_back("HFstats2__plus"); weight_syst_list.push_back("HFstats2__minus");
	weight_syst_list.push_back("CFerr1__plus"); weight_syst_list.push_back("CFerr1__minus");
	weight_syst_list.push_back("CFerr2__plus"); weight_syst_list.push_back("CFerr2__minus");
	weight_syst_list.push_back("HFcont__plus"); weight_syst_list.push_back("HFcont__minus");
	weight_syst_list.push_back("LFstats1__plus"); weight_syst_list.push_back("LFstats1__minus");
	weight_syst_list.push_back("LFstats2__plus"); weight_syst_list.push_back("LFstats2__minus");




//-----------------------------------------
 // ####### #     # #     #  #####  ####### ### ####### #     #     #####     #    #       #        #####
 // #       #     # ##    # #     #    #     #  #     # ##    #    #     #   # #   #       #       #     #
 // #       #     # # #   # #          #     #  #     # # #   #    #        #   #  #       #       #
 // #####   #     # #  #  # #          #     #  #     # #  #  #    #       #     # #       #        #####
 // #       #     # #   # # #          #     #  #     # #   # #    #       ####### #       #             #
 // #       #     # #    ## #     #    #     #  #     # #    ##    #     # #     # #       #       #     #
 // #        #####  #     #  #####     #    ### ####### #     #     #####  #     # ####### #######  #####
//-----------------------------------------

	bool do_MEM_regions = true;
	bool do_WZ_region = false;

	TString tmp_dir = ""; //Specific I/O dir. name (e.g. "medium_btag") if not nominal ntuples


//Need to differenciate ttZ/tZq & WZ, since MEM can't run in WZ region (not enough jets) ==> Different ntuples
	TString MEM_or_WZ;

//--- Produce ntuples for MEM (ttZ/tZq regions)
	if(do_MEM_regions)
	{
		MEM_or_WZ = "MEM";
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			Modify_Ntuples(sample_list[isample].Data(), thevarlist, MEMvarlist, MEMvarlist_new, weight_syst_list, tree_syst_list, MEM_or_WZ, tmp_dir);
		}
	}

//--- Interface ntuples for WZ CR study (mTW template fit)
	if(do_WZ_region)
	{
		MEM_or_WZ = "WZ";
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			Modify_Ntuples(sample_list[isample].Data(), thevarlist, MEMvarlist, MEMvarlist_new, weight_syst_list, tree_syst_list, MEM_or_WZ, tmp_dir);
		}
	}

	return 0;
}
