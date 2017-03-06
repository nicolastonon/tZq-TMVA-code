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
#include <iostream>
#include <map>

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

double scaleFactor(TFile * f, int channel, vector<TString> sample_list)
{

  std::vector<TString> channelname;
  channelname.push_back("uuu");
  channelname.push_back("uue");
  channelname.push_back("eeu");
  channelname.push_back("eee");

  std::vector<TString> listSum;
  cout<<endl<<endl;
  for(int isample=0; isample<sample_list.size(); isample++)
  {
    if(sample_list[isample].Contains("data") || sample_list[isample].Contains("Fake")) {continue;}
    TString name_tmp = "mTW_uuu__"+sample_list[isample];
    if(!f ->GetListOfKeys()->Contains( name_tmp.Data() ) ) {cout<<name_tmp.Data()<<" not found !"<<endl;  continue;}

    listSum.push_back(sample_list[isample]);
  }
  cout<<endl<<endl;

  std::vector<TString> channelfake;
  channelfake.push_back("FakeMuMuMu");
  channelfake.push_back("FakeMuMuEl");
  channelfake.push_back("FakeElElMu");
  channelfake.push_back("FakeElElEl");


  f->cd();
  char myHist[100];

  TH1F * hdata, *hsum[listSum.size()], * hfake;

  //Treat Data / Fakes / Other samples separately
  sprintf(myHist,"mTW_%s__data_obs",channelname[channel].Data());
  if ( f ->GetListOfKeys()->Contains( myHist ) ) hdata = (TH1F*)f->Get( myHist) ;
  else {printf(" HAAAAALT %s\n",myHist); return 0;}

  sprintf(myHist,"mTW_%s__%s",channelname[channel].Data(), channelfake[channel].Data());
  if ( f ->GetListOfKeys()->Contains( myHist ) ) hfake = (TH1F*)f->Get( myHist) ;
  else {printf(" HAAAAALT %s\n",myHist); return 0;}

  for (int i = 0; i<listSum.size(); i++)
  {
    sprintf(myHist,"mTW_%s__%s",channelname[channel].Data(), listSum[i].Data());

    if ( f ->GetListOfKeys()->Contains( myHist ) ) hsum[i] = (TH1F*)f->Get( myHist) ;
    else {printf(" HAAAAALT %s\n",myHist); continue;}

    if (i>0) hsum[0]->Add( hsum[i] );
  }

  hfake->Print(); hsum[0]->Print(); hdata->Print();

  TObjArray *mc = new TObjArray(2);
  mc->Add(hfake);
  mc->Add(hsum[0]);

  TFractionFitter* fit = new TFractionFitter(hdata, mc);

  TFitResultPtr r = fit->Fit(); //Create smart ptr to TFitResult --> can access fit properties

  TCanvas* c1 = new TCanvas("c1");

  TH1F* result = (TH1F*) fit->GetPlot();
  hdata->Draw("Ep");
  result->Draw("same");

  // c1->SaveAs(("plots/ScaleFakes_"+channelname[channel]+".png").Data()); //Save fit plot
  delete c1;

  double fraction_fakes = r->Parameter(0); //Parameter 0 <--> Fitted Fraction of mc[0] == Fakes
  cout<<endl<<endl<<channelname[channel]<<" : Fraction of Fakes fitted from data = "<<fraction_fakes*100<<" %"<<endl;

  double integralQuotient = hdata->Integral()/hfake->Integral();

  // double mySF = vFit->GetParameter(0)*integralQuotient;
  double mySF = fraction_fakes*integralQuotient;

  return mySF; //returns (fakes/data)[FIT]  *  (data/fakes) --> Scale factor for Fakes samples !
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

int Scale_Fake_Histograms(TString file_to_rescale_name)
{
  TH1::SetDefaultSumw2();

  int rebin = 1;

  TFile * file_to_rescale = 0;
  file_to_rescale = TFile::Open(file_to_rescale_name.Data()); //File containing the templates, from which can compute fake ratio
  if(!file_to_rescale) {cout<<FRED(<<file_to_rescale_name.Data()<<" not found! Which file do you want to Rescale ? -- Abort")<<endl; return 0;}

  TString prefit_file_name = "outputs/Combine_Input_noScale.root";
  TFile * inputfile_prefit = 0;
  inputfile_prefit = TFile::Open(prefit_file_name.Data()); //File containing the templates, from which can compute fake ratio
  if(!inputfile_prefit) {cout<<FRED(<<prefit_file_name.Data()<<" not found! Can't compute Fake Ratio -- Abort")<<endl; return 0;}

  std::vector<TString> syst_list;
  syst_list.push_back(""); //KEEP this one -- nominal

  // syst_list.push_back("JERUp")  ;       syst_list.push_back("JERDown")  ;
  // syst_list.push_back("JESUp")  ;       syst_list.push_back("JESDown")  ;
  // syst_list.push_back("FakesUp");       syst_list.push_back("FakesDown");

  syst_list.push_back("Q2Up")      ;    syst_list.push_back("Q2Down");
  syst_list.push_back("PUUp")      ;    syst_list.push_back("PUDown");
  syst_list.push_back("MuEffUp")   ;    syst_list.push_back("MuEffDown");
  syst_list.push_back("EleEffUp")  ;    syst_list.push_back("EleEffDown");
  syst_list.push_back("pdfUp")     ;    syst_list.push_back("pdfDown");
  syst_list.push_back("LFcontUp")  ;    syst_list.push_back("LFcontDown");
  syst_list.push_back("HFstats1Up");    syst_list.push_back("HFstats1Down");
  syst_list.push_back("HFstats2Up");    syst_list.push_back("HFstats2Down");
  syst_list.push_back("CFerr1Up")  ;    syst_list.push_back("CFerr1Down");
  syst_list.push_back("CFerr2Up")  ;    syst_list.push_back("CFerr2Down");
  syst_list.push_back("HFcontUp")  ;    syst_list.push_back("HFcontDown");
  syst_list.push_back("LFstats1Up");    syst_list.push_back("LFstats1Down");
  syst_list.push_back("LFstats2Up");    syst_list.push_back("LFstats2Down");


  std::vector<TString> sample_list;
  sample_list.push_back("data_obs");
  sample_list.push_back("ttZ");
  sample_list.push_back("ttW");
  sample_list.push_back("WZjets");
  sample_list.push_back("tZq");
  sample_list.push_back("ttH");
  sample_list.push_back("ZZ");
  // sample_list.push_back("SingleTop");
  sample_list.push_back("FakeElElEl");
  sample_list.push_back("FakeMuMuEl");
  sample_list.push_back("FakeElElMu");
  sample_list.push_back("FakeMuMuMu");

  vector<TString> var_list;
  //Template names
  var_list.push_back("BDT");
  var_list.push_back("BDTttZ");
  var_list.push_back("mTW");

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

  //Other vars
  var_list.push_back("NJets");
  var_list.push_back("NBJets");


  std::vector<TString> channel_list;
  channel_list.push_back("uuu");
  channel_list.push_back("uue");
  channel_list.push_back("eeu");
  channel_list.push_back("eee");


  //Get SF for each channel
  double factor_uuu = scaleFactor(inputfile_prefit,0, sample_list);
  double factor_uue = scaleFactor(inputfile_prefit,1, sample_list);
  double factor_eeu = scaleFactor(inputfile_prefit,2, sample_list);
  double factor_eee = scaleFactor(inputfile_prefit,3, sample_list);


  // printf("\n\n ------- %f %f %f %f -----\n \n",factor_uuu,factor_uue,factor_eeu,factor_eee);

  cout<<endl<<"SCALE FACTORS VALUES : "<<endl;
  cout<<"--- SF uuu = "<<factor_uuu<<endl;
  cout<<"--- SF uue = "<<factor_uue<<endl;
  cout<<"--- SF eeu = "<<factor_eeu<<endl;
  cout<<"--- SF eee = "<<factor_eee<<endl<<endl;

  //output file
  TString output_name = file_to_rescale_name ;
  int index = 0;
  if(output_name.Contains("noScale") ) index = output_name.Index("_noScale.root"); //Find index of substring
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
    cout<<"Channel : "<<channel_list[ichan]<<endl;

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

          if(!file_to_rescale->GetListOfKeys()->Contains(histo_name.Data()) ) {continue;}

          histo = (TH1F*)file_to_rescale->Get( histo_name );

          //RE-SCALE ONLY FAKE HISTOS
          if(sample_list[isample] == "FakeElElEl")       { histo->Scale(factor_eee); }
          else if(sample_list[isample] == "FakeElElMu")  { histo->Scale(factor_eeu); }
          else if(sample_list[isample] == "FakeMuMuEl")  { histo->Scale(factor_uue); }
          else if(sample_list[isample] == "FakeMuMuMu")  { histo->Scale(factor_uuu); }

          //WRITE ALL HISTOS
          // histo->SetName( histo_name );
          outfile_post->cd();
          histo->Rebin(rebin);
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
  if(argc == 2)
  {
    Scale_Fake_Histograms(argv[1]);
  }

  else if(argc==1)
  {
    TString file_to_rescale = "";

    file_to_rescale = "outputs/Combine_Input_noScale.root";
    Scale_Fake_Histograms(file_to_rescale);

    file_to_rescale = "outputs/Control_Histograms_NJetsMin0_NBJetsEq0.root";
    Scale_Fake_Histograms(file_to_rescale);
  }

  else {cout<<"Wrong number of arguments !"<<endl; return 1;}


  return 0;
}
