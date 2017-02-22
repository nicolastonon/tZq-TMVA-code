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

using namespace std;

//  ######   #######  ##     ## ########  ##     ## ######## ########        ######      ########
// ##    ## ##     ## ###   ### ##     ## ##     ##    ##    ##             ##    ##     ##
// ##       ##     ## #### #### ##     ## ##     ##    ##    ##             ##           ##
// ##       ##     ## ## ### ## ########  ##     ##    ##    ######          ######      ######
// ##       ##     ## ##     ## ##        ##     ##    ##    ##                   ##     ##
// ##    ## ##     ## ##     ## ##        ##     ##    ##    ##             ##    ## ### ##       ###
//  ######   #######  ##     ## ##         #######     ##    ########        ######  ### ##       ###

double scaleFactor(TFile * f, int channel, vector<TString> listSamples)
{

  std::vector<TString> channelname;
  channelname.push_back("uuu");
  channelname.push_back("uue");
  channelname.push_back("eeu");
  channelname.push_back("eee");

  std::vector<TString> listSum;
  cout<<endl<<endl;
  for(int isample=0; isample<listSamples.size(); isample++)
  {
    if(listSamples[isample].Contains("data") || listSamples[isample].Contains("Fake")) {continue;}
    TString name_tmp = "mTW_uuu__"+listSamples[isample];
    if(!f ->GetListOfKeys()->Contains( name_tmp.Data() ) ) {cout<<name_tmp.Data()<<" not found !"<<endl;  continue;}

    listSum.push_back(listSamples[isample]);
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

  // Int_t status = fit->Fit();               // perform the fit

  TFitResultPtr r = fit->Fit(); //Create smart ptr to TFitResult --> can access fit properties


  // if (status == 0)   // check on fit status
  {
    TCanvas* c1 = new TCanvas("c1");

    TH1F* result = (TH1F*) fit->GetPlot();
    hdata->Draw("Ep");
    result->Draw("same");

    // TVirtualFitter* vFit = 0;
    // vFit = (TVirtualFitter*) fit->GetFitter();
    // if(!vFit) {cout<<"vFit == 0 -- Exit"<<endl; return 0;}
    // printf("----- %f \n", vFit->GetParameter(0) );

    c1->SaveAs(("plots/ScaleFakes_"+channelname[channel]+".png").Data());
    delete c1;


    double fraction_fakes = r->Parameter(0); //Parameter 0 <--> Fitted Fraction of mc[0] == Fakes
    cout<<endl<<endl<<channelname[channel]<<" : Fraction of Fakes fitted from data = "<<fraction_fakes*100<<" %"<<endl;

    double integralQuotient = hdata->Integral()/hfake->Integral();

    // double mySF = vFit->GetParameter(0)*integralQuotient;
    double mySF = fraction_fakes*integralQuotient;

    return mySF; //returns (fakes/data)[FIT]  *  (data/fakes) --> Scale factor for Fakes samples !
  }
  // else return 1.;

}


//  ######   ######     ###    ##       ########    ########    ###    ##    ## ########    ######## ######## ##     ## ########
// ##    ## ##    ##   ## ##   ##       ##          ##         ## ##   ##   ##  ##             ##    ##       ###   ### ##     ##
// ##       ##        ##   ##  ##       ##          ##        ##   ##  ##  ##   ##             ##    ##       #### #### ##     ##
//  ######  ##       ##     ## ##       ######      ######   ##     ## #####    ######         ##    ######   ## ### ## ########
//       ## ##       ######### ##       ##          ##       ######### ##  ##   ##             ##    ##       ##     ## ##
// ##    ## ##    ## ##     ## ##       ##          ##       ##     ## ##   ##  ##             ##    ##       ##     ## ##        ###
//  ######   ######  ##     ## ######## ########    ##       ##     ## ##    ## ########       ##    ######## ##     ## ##        ###


int main()
{
  int rebin = 1;

  std::vector<TString> thesystlist;
  thesystlist.push_back("");
  // thesystlist.push_back("JERUp")  ;       thesystlist.push_back("JERDown")  ;
  // thesystlist.push_back("JESUp")  ;       thesystlist.push_back("JESDown")  ;
  // thesystlist.push_back("FakesUp");       thesystlist.push_back("FakesDown");
  // thesystlist.push_back("Q2Up")      ;    thesystlist.push_back("Q2Down");
  thesystlist.push_back("PUUp")      ;    thesystlist.push_back("PUDown");
  thesystlist.push_back("MuEffUp")   ;    thesystlist.push_back("MuEffDown");
  thesystlist.push_back("EleEffUp")  ;    thesystlist.push_back("EleEffDown");
  thesystlist.push_back("pdfUp")     ;    thesystlist.push_back("pdfDown");
  thesystlist.push_back("LFcontUp")  ;    thesystlist.push_back("LFcontDown");
  thesystlist.push_back("HFstats1Up");    thesystlist.push_back("HFstats1Down");
  thesystlist.push_back("HFstats2Up");    thesystlist.push_back("HFstats2Down");
  thesystlist.push_back("CFerr1Up")  ;    thesystlist.push_back("CFerr1Down");
  thesystlist.push_back("CFerr2Up")  ;    thesystlist.push_back("CFerr2Down");
  thesystlist.push_back("HFcontUp")  ;    thesystlist.push_back("HFcontDown");
  thesystlist.push_back("LFstats1Up");    thesystlist.push_back("LFstats1Down");
  thesystlist.push_back("LFstats2Up");    thesystlist.push_back("LFstats2Down");



  std::vector<TString> listSamples;
  listSamples.push_back("data_obs");
  listSamples.push_back("ttZ");
  listSamples.push_back("ttW");
  listSamples.push_back("WZjets");
  listSamples.push_back("tZq");
  listSamples.push_back("ttH");
  listSamples.push_back("ZZ");
  listSamples.push_back("FakeElElEl");
  listSamples.push_back("FakeMuMuEl");
  listSamples.push_back("FakeElElMu");
  listSamples.push_back("FakeMuMuMu");



  std::vector<TString> thechannel;
  thechannel.push_back("uuu");
  thechannel.push_back("uue");
  thechannel.push_back("eeu");
  thechannel.push_back("eee");



  TString input_file_name = "outputs/Combine_Input_noScale.root";
  // TString input_file_name = "outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root"; //File containing mTW templates in WZ CR
  TFile * inputfile_prefit = 0;
  inputfile_prefit = TFile::Open(input_file_name.Data());
  if(!inputfile_prefit) {cout<<input_file_name.Data()<<" not found! -- Abort"<<endl; return 0;}


  //Get SF for each channel
  double factor_uuu = scaleFactor(inputfile_prefit,0, listSamples);
  double factor_uue = scaleFactor(inputfile_prefit,1, listSamples);
  double factor_eeu = scaleFactor(inputfile_prefit,2, listSamples);
  double factor_eee = scaleFactor(inputfile_prefit,3, listSamples);


  // printf("\n\n ------- %f %f %f %f -----\n \n",factor_uuu,factor_uue,factor_eeu,factor_eee);

  cout<<endl<<"SCALE FACTORS VALUES : "<<endl;
  cout<<"--- SF uuu = "<<factor_uuu<<endl;
  cout<<"--- SF uue = "<<factor_uue<<endl;
  cout<<"--- SF eeu = "<<factor_eeu<<endl;
  cout<<"--- SF eee = "<<factor_eee<<endl<<endl;

  //output file
  TFile * outfile_post  = new TFile( "outputs/Combine_Input_ScaledFakes.root", "RECREATE");

  //Fake channels names
  const char *   elelel = "FakeElElEl";
  const char *   elelmu = "FakeElElMu";
  const char *   mumuel = "FakeMuMuEl";
  const char *   mumumu = "FakeMuMuMu";

  char thisHisto[100];

//--- RESCALE FAKE TEMPLATES, write all templates to output file

  // Loop over channels
  for(int ichan=0; ichan<thechannel.size(); ichan++){

    // Loop over systematics
    for (unsigned int isys=0; isys<  thesystlist.size(); isys++){

      // Loop over samples
      for(unsigned int isample = 0; isample < listSamples.size(); isample++){

      	inputfile_prefit->cd();

//--- Scale mTW Fake templates & write all templates to output file
      	if ( strcmp(thesystlist[isys].Data(),"") == 0) sprintf(thisHisto,"mTW_%s__%s",thechannel[ichan].Data() , listSamples[isample].Data() );  // Take central systematic histograms
      	else sprintf(thisHisto,"mTW_%s__%s__%s",thechannel[ichan].Data() , listSamples[isample].Data(), thesystlist[isys].Data()  ); // Take other

      	inputfile_prefit->cd();

    	   if ( inputfile_prefit ->GetListOfKeys()->Contains( thisHisto ) ) {

      	  TH1F * histo = (TH1F*)inputfile_prefit->Get( thisHisto );

      	  if(strcmp( listSamples[isample].Data(), elelel)==0) { histo->Scale(factor_eee); }
      	  else if(strcmp( listSamples[isample].Data(), elelmu)==0 ) { histo->Scale(factor_eeu); }
      	  else if(strcmp( listSamples[isample].Data(), mumuel)==0 ) { histo->Scale(factor_uue); }
      	  else if(strcmp( listSamples[isample].Data(), mumumu)==0 ) { histo->Scale(factor_uuu); }

      	  histo->SetName( thisHisto );
      	  outfile_post->cd();
      	  histo->Rebin(rebin);
      	  histo->Write();

      	}

//--- Scale BDT Fake templates & write all templates to output file
        if ( strcmp(thesystlist[isys].Data(),"") == 0) sprintf(thisHisto,"BDT_%s__%s",thechannel[ichan].Data() , listSamples[isample].Data() );
        else sprintf(thisHisto,"BDT_%s__%s__%s",thechannel[ichan].Data() , listSamples[isample].Data(), thesystlist[isys].Data()  );

        inputfile_prefit->cd();

	      if ( inputfile_prefit ->GetListOfKeys()->Contains( thisHisto ) )
        {
      	  TH1F * histo1 = (TH1F*)inputfile_prefit->Get( thisHisto);

      	  if(strcmp( listSamples[isample].Data(), elelel)==0) { histo1->Scale(factor_eee); }
      	  else if(strcmp( listSamples[isample].Data(), elelmu)==0 ) { histo1->Scale(factor_eeu); }
      	  else if(strcmp( listSamples[isample].Data(), mumuel)==0 ) { histo1->Scale(factor_uue); }
      	  else if(strcmp( listSamples[isample].Data(), mumumu)==0 ) { histo1->Scale(factor_uuu); }

      	  histo1->SetName( thisHisto );
      	  outfile_post->cd();
      	  histo1->Rebin(rebin);
      	  histo1->Write();
        }


//--- Scale BDTttZ Fake templates & write all templates to output file
        if ( strcmp(thesystlist[isys].Data(),"") == 0) sprintf(thisHisto,"BDTttZ_%s__%s",thechannel[ichan].Data() , listSamples[isample].Data() );
        else sprintf(thisHisto,"BDTttZ_%s__%s__%s",thechannel[ichan].Data() , listSamples[isample].Data(), thesystlist[isys].Data()  );
        inputfile_prefit->cd();

	      if ( inputfile_prefit ->GetListOfKeys()->Contains( thisHisto) )
        {
  	      TH1F * histo2 = (TH1F*)inputfile_prefit->Get( thisHisto) ;

      	  if(strcmp( listSamples[isample].Data(), elelel)==0) { histo2->Scale(factor_eee); }
      	  else if(strcmp( listSamples[isample].Data(), elelmu)==0 ) { histo2->Scale(factor_eeu); }
      	  else if(strcmp( listSamples[isample].Data(), mumuel)==0 ) { histo2->Scale(factor_uue); }
      	  else if(strcmp( listSamples[isample].Data(), mumumu)==0 ) { histo2->Scale(factor_uuu); }

      	  histo2->SetName( thisHisto );
      	  outfile_post->cd();
      	  histo2->Rebin(rebin);
      	  histo2->Write();
        }



// //FIXME -- test Histogram
//         TString name_test = "btagDiscri_"  + thechannel[ichan] + "__" + listSamples[isample];
//         cout<<"name_test = "<<name_test<<endl;
//
//         inputfile_prefit->cd();
//
// 	      if ( inputfile_prefit ->GetListOfKeys()->Contains( name_test) )
//         {
//           cout<<"btagDiscri histo found!!"<<endl;
//   	      TH1F * histotest = 0;
//           histotest = (TH1F*)inputfile_prefit->Get( name_test) ;
//
//       	  if(strcmp( listSamples[isample].Data(), elelel)==0) { histotest->Scale(factor_eee); }
//       	  else if(strcmp( listSamples[isample].Data(), elelmu)==0 ) { histotest->Scale(factor_eeu); }
//       	  else if(strcmp( listSamples[isample].Data(), mumuel)==0 ) { histotest->Scale(factor_uue); }
//       	  else if(strcmp( listSamples[isample].Data(), mumumu)==0 ) { histotest->Scale(factor_uuu); }
//
//       	  histotest->SetName( name_test );
//       	  outfile_post->cd();
//       	  histotest->Rebin(rebin);
//           cout<<"histotest "<<histotest<<endl;
//       	  histotest->Write(); cout<<"write"<<endl;
//         }

      } // end sample loop
    } // end syst loop
  } // end channel loop


  outfile_post->Close();

  return 0;

}
