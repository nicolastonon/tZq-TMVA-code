#include "TString.h"
#include "TH1F.h"
#include "TStyle.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "THStack.h"
#include "TFractionFitter.h"
#include "TVirtualFitter.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TSystem.h"
#include <iostream>
#include <map>
#include <cstdlib>

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

//  ######   #######  ##     ## ########  ##     ## ######## ########        ######      ########
// ##    ## ##     ## ###   ### ##     ## ##     ##    ##    ##             ##    ##     ##
// ##       ##     ## #### #### ##     ## ##     ##    ##    ##             ##           ##
// ##       ##     ## ## ### ## ########  ##     ##    ##    ######          ######      ######
// ##       ##     ## ##     ## ##        ##     ##    ##    ##                   ##     ##
// ##    ## ##     ## ##     ## ##        ##     ##    ##    ##             ##    ## ### ##       ###
//  ######   #######  ##     ## ##         #######     ##    ########        ######  ### ##       ###

double scaleFactor(TFile * f, TString fakeLep_flavour, vector<TString> sample_list, TString Combine_or_Theta, bool verbose = true)
{
  if(fakeLep_flavour != "u" && fakeLep_flavour != "e") {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<"Wrong name for flavour of fake lepton ('e' or 'u') !"; return 0;}

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
    if(sample_list[isample] == "data_obs" || sample_list[isample] == "DATA" || sample_list[isample] == "DATA" || sample_list[isample].Contains("Fake") ) {continue;} //Treated separately
    TString name_tmp = "mTW_uuu__"+sample_list[isample];
    if(!f ->GetListOfKeys()->Contains( name_tmp.Data() ) ) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<name_tmp.Data()<<" not found !"<<endl;  continue;}

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
    if(Combine_or_Theta == "combine") myHist = "mTW_" + channels_same_flavour[ichan] + "__data_obs";
    else if(Combine_or_Theta == "theta") myHist = "mTW_" + channels_same_flavour[ichan] + "__DATA";
    if ( !f->GetListOfKeys()->Contains( myHist.Data() ) ) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<myHist.Data()<<" not found !"<<endl; return 0;}
    h_tmp = (TH1F*)f->Get( myHist);
    if(ichan==0) {hdata = (TH1F*) h_tmp->Clone();}
    else {hdata->Add(h_tmp);}

    //FAKES
    myHist = "mTW_" + channels_same_flavour[ichan] + "__" + channelfake[ichan];
    if ( !f->GetListOfKeys()->Contains( myHist.Data() ) )  {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<myHist.Data()<<" not found !"<<endl; return 0;}
    h_tmp = (TH1F*)f->Get( myHist);
    if(ichan==0) {hfake = (TH1F*) h_tmp->Clone();}
    else {hfake->Add(h_tmp);}

    //MC
    for (int i = 0; i<listSum.size(); i++)
    {
      myHist = "mTW_" + channels_same_flavour[ichan] + "__" + listSum[i];
      if ( !f->GetListOfKeys()->Contains( myHist.Data() ) ) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<myHist.Data()<<" not found !"<<endl; return 0;}
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

  //FIXME : constrain backgrounds which are not fake (NB : because we're only interested in fitting the fakes to the data here!)
  double fracmc = hsum->Integral()/hdata->Integral() ;
  fit->Constrain(0, 0.001, pow(10,6) ); //Constrain param 0 (fakes)
  if(fracmc>0) fit->Constrain(1, fracmc*0.99, fracmc*1.01); //Constrain param 1 (other MC samples)


  TFitResultPtr r = fit->Fit(); //Create smart ptr to TFitResult --> can access fit properties

  TCanvas* c1 = new TCanvas("c1");

  TH1F* result = (TH1F*) fit->GetPlot();
  hdata->Draw("Ep");
  result->Draw("same");

  c1->SaveAs(("plots/ScaleFakes_"+fakeLep_flavour+".png").Data()); //Save fit plot
  delete c1;

  double fakes_postfit = r->Parameter(0); //Parameter 0 <--> Fitted Fraction of mc[0] == Fakes
  if(verbose) cout<<endl<<"ScaleFakes.cc "<<__LINE__<<" : "<<"Fake lepton flavour "<<fakeLep_flavour<<" : Fraction of Fakes fitted from data = "<<fakes_postfit*100<<" %"<<endl;


  return (fakes_postfit/hfake->Integral() ) * hdata->Integral();
}


//  ######   ######     ###    ##       ########    ########    ###    ##    ## ########
// ##    ## ##    ##   ## ##   ##       ##          ##         ## ##   ##   ##  ##
// ##       ##        ##   ##  ##       ##          ##        ##   ##  ##  ##   ##
//  ######  ##       ##     ## ##       ######      ######   ##     ## #####    ######
//       ## ##       ######### ##       ##          ##       ######### ##  ##   ##
// ##    ## ##    ## ##     ## ##       ##          ##       ##     ## ##   ##  ##
//  ######   ######  ##     ## ######## ########    ##       ##     ## ##    ## ########

// ##     ## ####  ######  ########  #######   ######   ########     ###    ##     ##  ######
// ##     ##  ##  ##    ##    ##    ##     ## ##    ##  ##     ##   ## ##   ###   ### ##    ##
// ##     ##  ##  ##          ##    ##     ## ##        ##     ##  ##   ##  #### #### ##
// #########  ##   ######     ##    ##     ## ##   #### ########  ##     ## ## ### ##  ######
// ##     ##  ##        ##    ##    ##     ## ##    ##  ##   ##   ######### ##     ##       ##
// ##     ##  ##  ##    ##    ##    ##     ## ##    ##  ##    ##  ##     ## ##     ## ##    ##
// ##     ## ####  ######     ##     #######   ######   ##     ## ##     ## ##     ##  ######

int Scale_Fake_Histograms(TString file_to_rescale_name, TString Combine_or_Theta = "combine", bool verbose = true)
{
  if(Combine_or_Theta != "combine" && Combine_or_Theta != "theta" && verbose) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<BOLD(FRED("Error ! Need to choose b/w 'combine' or 'theta' in code !"))<<endl; return 0;}
  if(verbose) cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<BOLD(FRED("Creating re-scaled template file for : "<<Combine_or_Theta<<" !"))<<endl;

  TH1::SetDefaultSumw2();

  TFile * file_to_rescale = 0;
  file_to_rescale = TFile::Open(file_to_rescale_name.Data()); //File containing the templates, from which can compute fake ratio
  if(!file_to_rescale) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<FRED(<<file_to_rescale_name.Data()<<" not found! Which file do you want to Rescale ? -- Abort")<<endl; return 0;}


  TString prefit_file_name;
  if(Combine_or_Theta == "combine") prefit_file_name = "outputs/Combine_Input_noScale.root"; //Combine templates
  else if(Combine_or_Theta == "theta") prefit_file_name = "outputs/Theta_Input_noScale.root"; //Theta templates

  TFile * inputfile_prefit = 0;
  Long_t *id,*size,*flags,*modtime;

  inputfile_prefit = TFile::Open(prefit_file_name); //File containing the templates, from which can compute fake ratio
  if(!inputfile_prefit) //If file doesn't exist, check for the other one
  {
    if(Combine_or_Theta == "combine") prefit_file_name = "outputs/Theta_Input_noScale.root"; //Combine templates
    else if(Combine_or_Theta == "theta") prefit_file_name = "outputs/Combine_Input_noScale.root"; //Theta templates

    inputfile_prefit = TFile::Open(prefit_file_name);
    if(!inputfile_prefit) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<FRED(<<prefit_file_name.Data()<<" not found! Can't compute Fake Ratio -- Abort")<<endl; return 0;}
  }


  if(!inputfile_prefit) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<FRED(<<prefit_file_name.Data()<<" not found! Can't compute Fake Ratio -- Abort")<<endl; return 0;}


  std::vector<TString> syst_names;
  syst_names.push_back("JES")  ;
  syst_names.push_back("JER")  ;
  syst_names.push_back("Fakes");
  syst_names.push_back("Q2")      ;
  syst_names.push_back("PU")      ;
  syst_names.push_back("MuEff")   ;
  syst_names.push_back("EleEff")  ;
  syst_names.push_back("pdf")     ;
  syst_names.push_back("LFcont")  ;
  syst_names.push_back("HFstats1");
  syst_names.push_back("HFstats2");
  syst_names.push_back("CFerr1")  ;
  syst_names.push_back("CFerr2")  ;
  syst_names.push_back("HFcont")  ;
  syst_names.push_back("LFstats1");
  syst_names.push_back("LFstats2");

//Use the vector 'syst_names' to create a new vector containing the actual systematics names
  std::vector<TString> syst_list;
  syst_list.push_back(""); //KEEP this one -- nominal
  for(int isyst=0; isyst<syst_names.size(); isyst++)
  {
    if(Combine_or_Theta == "combine")
    {
      syst_list.push_back( (syst_names[isyst]+"Up") );
      syst_list.push_back( (syst_names[isyst]+"Down") );
    }
    else if(Combine_or_Theta == "theta")
    {
      syst_list.push_back( (syst_names[isyst]+"__plus") );
      syst_list.push_back( (syst_names[isyst]+"__minus") );
    }
  }



  std::vector<TString> sample_list;
  if(Combine_or_Theta == "combine") sample_list.push_back("data_obs");
  else if(Combine_or_Theta == "theta") sample_list.push_back("DATA");
  sample_list.push_back("data_obs");
  sample_list.push_back("ttZ");
  sample_list.push_back("ttW");
  // sample_list.push_back("WZjets");
  sample_list.push_back("WZl");
  sample_list.push_back("WZb");
  sample_list.push_back("WZc");
  sample_list.push_back("tZq");
  sample_list.push_back("ttH");
  sample_list.push_back("ZZ");
  sample_list.push_back("STtWll");
  sample_list.push_back("FakeElElEl");
  sample_list.push_back("FakeMuMuEl");
  sample_list.push_back("FakeElElMu");
  sample_list.push_back("FakeMuMuMu");

  vector<TString> var_list; //FIXME -- keep list of vars up-to-date !!
  //Template names
  var_list.push_back("BDT");
  var_list.push_back("BDTttZ");
  var_list.push_back("mTW");
  var_list.push_back("METpt");

  //BDT vars names
  var_list.push_back("btagDiscri");
  var_list.push_back("dRAddLepQ");
  var_list.push_back("dRAddLepClosestJet");
  var_list.push_back("dPhiAddLepB");
  var_list.push_back("ZEta");
  var_list.push_back("Zpt");
  var_list.push_back("mtop");
  var_list.push_back("AddLepAsym");
  var_list.push_back("etaQ");
  var_list.push_back("AddLepETA");
  var_list.push_back("LeadJetEta");
  var_list.push_back("dPhiZAddLep");
  var_list.push_back("dRZAddLep");
  var_list.push_back("dRjj");
  var_list.push_back("ptQ");
  var_list.push_back("tZq_pT");
  var_list.push_back("dRAddLepB");
  var_list.push_back("dPhiZAddLep");
  var_list.push_back("dRZAddLep"); // --> little discrim --> to be included
  var_list.push_back("dRjj");
  var_list.push_back("mtop");
  var_list.push_back("m3l");
  var_list.push_back("dRZTop");
  var_list.push_back("TopPT");

  //Other vars
  var_list.push_back("NJets");
  var_list.push_back("NBJets");


  std::vector<TString> channel_list;
  channel_list.push_back("uuu");
  channel_list.push_back("uue");
  channel_list.push_back("eeu");
  channel_list.push_back("eee");


  //Get SF for each channel
  double SF_FakeMu = scaleFactor(inputfile_prefit,"u", sample_list, Combine_or_Theta, verbose);
  double SF_FakeEl = scaleFactor(inputfile_prefit,"e", sample_list, Combine_or_Theta, verbose);


  if(verbose)
  {
    cout<<endl<<"SCALE FACTORS VALUES : "<<endl;
    cout<<"--- SF Fake Muon = "<<SF_FakeMu<<endl;
    cout<<"--- SF Fake Electron = "<<SF_FakeEl<<endl;

  }

  //output file
  TString output_name = file_to_rescale_name ;
  int index = 0;
  if(output_name.Contains("noScale.root") ) index = output_name.Index("_noScale.root"); //Find index of substring
  else index = output_name.Index(".root"); //Find index of substring
  output_name.Remove(index); //Remove substring
  output_name+= "_ScaledFakes.root"; //Add desired suffix

  TFile * outfile_post  = new TFile( output_name.Data(), "RECREATE");


//--- RESCALE FAKES HISTOGRAMS
//Run on all vars>chan>sample>syst names --> Write all histos to output, but rescale only fakes

  TString histo_name = "";
  TH1F * histo = 0;

  // Loop over channels
  for(int ichan=0; ichan<channel_list.size(); ichan++)
  {
    if(verbose) cout<<"Channel : "<<channel_list[ichan]<<endl;

    // Loop over samples
    for(unsigned int isample = 0; isample < sample_list.size(); isample++)
    {
      for(int ivar=0; ivar<var_list.size(); ivar++)
      {
        // Loop over systematics
        for (unsigned int isys=0; isys<syst_list.size(); isys++)
        {
          file_to_rescale->cd(); histo = 0;

          histo_name = var_list[ivar] + "_" + channel_list[ichan] + "__" + sample_list[isample];
          if(syst_list[isys] != "") histo_name+= "__" + syst_list[isys];

          // if(!file_to_rescale->GetListOfKeys()->Contains(histo_name.Data()) ) {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<histo_name.Data()<<" not found !"<<endl; continue;}
          if(!file_to_rescale->GetListOfKeys()->Contains(histo_name.Data()) ) {continue;}

          histo = (TH1F*)file_to_rescale->Get( histo_name );

          //RE-SCALE ONLY FAKE HISTOS
          if(sample_list[isample] == "FakeElElEl")       { histo->Scale(SF_FakeEl); }
          else if(sample_list[isample] == "FakeElElMu")  { histo->Scale(SF_FakeMu); }
          else if(sample_list[isample] == "FakeMuMuEl")  { histo->Scale(SF_FakeEl); }
          else if(sample_list[isample] == "FakeMuMuMu")  { histo->Scale(SF_FakeMu); }

          //WRITE ALL HISTOS
          // histo->SetName( histo_name );
          outfile_post->cd();
          histo->Write(histo_name, TObject::kOverwrite);
        } // end syst loop
      } // end var loop
    } //end sample loop
  } // end channel loop

  file_to_rescale->Close();
  inputfile_prefit->Close();
  outfile_post->Close();
  delete histo;

  return 0;
}


//DEFINE THE PATHS OF THE FILE WHERE THE HISTOGRAMS TO RESCALE ARE
int main(int argc, char **argv)
{
  bool verbose = true;

  TString Combine_or_Theta = "combine";

  if(argc>1) //Specify file(s) path(s) at execution
  {
    for(int ifile=1; ifile<argc; ifile++)
    {
      Scale_Fake_Histograms(argv[ifile], Combine_or_Theta, verbose);
    }
  }

  else if(argc==1) //Manually specify the file to rescale
  {
    TString file_to_rescale;

    file_to_rescale = "outputs/Combine_Input_noScale.root";
    Scale_Fake_Histograms(file_to_rescale, Combine_or_Theta, verbose);

    file_to_rescale = "outputs/Control_Histograms_NJetsMin0_NBJetsEq0.root";
    Scale_Fake_Histograms(file_to_rescale, Combine_or_Theta, verbose);
  }

  else {cout<<"ScaleFakes.cc "<<__LINE__<<" : "<<"Wrong number of arguments !"<<endl; return 1;}


  return 0;
}
