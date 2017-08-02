#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <sstream>

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

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
#include "TRandom1.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TH2D.h"
#include "TLine.h"

#include "setTDRStyle.h"

using namespace std;

TH1D* GetHistoWeight(TTree* t, string variable, int nbins, double xmin, double xmax, string cut, string name)
{
	// cout<<"GetHistoWeight"<<endl;

	string sxmin, sxmax, snbins;
	stringstream ss[3];

	ss[0] << xmin;
	ss[0] >> sxmin;
	ss[1] << xmax;
	ss[1] >> sxmax;
	ss[2] << nbins;
	ss[2] >> snbins;

	string variablenew = variable + " >> h(" + snbins + "," + sxmin + "," + sxmax + ")";

	string cutnew = "weight * (" + cut + ")";

	if(!t) {cout<<"t is null!"<<endl; return 0;}

	// t->Print();

	t->Draw(variablenew.c_str(), cutnew.c_str());

	TH1D *histo = 0;
	histo = (TH1D*)gDirectory->Get("h");

	if (!histo) return histo;
	if(histo->GetEntries()==0) return histo;

	double underflow = histo->GetBinContent(0);
	// cout << "underflow="<<underflow<<endl;
	double val = 0;
	if (underflow>0) {
		val = histo->GetBinContent(1);
		histo->SetBinContent(1, val+underflow);
		histo->SetBinContent(0, 0);
	}

	double overflow = histo->GetBinContent(nbins+1);
	if (overflow>0) {
		val = histo->GetBinContent(nbins);
		histo->SetBinContent(nbins+1, 0);
		histo->SetBinContent(nbins, val+overflow);
	}

	// cout << "Area="<<histo->Integral()<<endl;
	// cout << "Nevents="<<histo->GetEntries()<<endl;
	histo->SetName(name.c_str());
	histo->SetTitle(name.c_str());

	return histo;
}

TGraph* GetEffSvsEffB_Optim(TTree* Signal, TTree* Background, string presel, string var, double valmin, double valmax, int npoints, string TitleGraph)
{

	cout << "GetEffSvsEffB_Optim"<<endl;

	int nbins = npoints;

	double* Eff_sg = new double[nbins];
	double* Eff_err_sg = new double[nbins];

	double* Eff_bg = new double[nbins];
	double* Eff_err_bg = new double[nbins];

	stringstream ss[nbins+1];
	string svalcut;
	string scut;

	string presel_sg = presel;
	string presel_bg = presel;

	TH1D* Histo_sg = GetHistoWeight(Signal, var, npoints, valmin, valmax, presel_sg, "sg");
	Histo_sg->SetName("sg");
	//cout << "Histo done"<<endl;
	double denom_sg = Histo_sg->Integral();
	// cout << "denom_sg=" << denom_sg<< endl;

	TH1D* Histo_bg = GetHistoWeight(Background, var, npoints, valmin, valmax, presel_bg, "bg");
	Histo_bg->SetName("bg");
	double denom_bg = Histo_bg->Integral();
	// cout << "denom_bg=" << denom_bg<< endl;


	for (int i=1; i<=nbins; i++){

		//double valcut = valmin*((double)i)/((double)nbins) + valmax*(1-((double)i)/((double)nbins));
		double valcut = valmax*((double)i)/((double)nbins) + valmin*(1-((double)i)/((double)nbins));

		ss[i] << valcut;
		ss[i] >> svalcut;
		double num_sg = Histo_sg->Integral(i, nbins);
		double num_bg = Histo_bg->Integral(i, nbins);

		Eff_sg[i] = num_sg/denom_sg;
		Eff_err_sg[i] = Eff_sg[i] * (sqrt(num_sg)/num_sg + sqrt(denom_sg)/denom_sg);

		Eff_bg[i] = num_bg/denom_bg;
		Eff_err_bg[i] = Eff_bg[i] * (sqrt(num_bg)/num_bg + sqrt(denom_bg)/denom_bg);

		// cout << "i="<<i << " "<<var<< "<" <<svalcut<<"   effS="<<Eff_sg[i]<<" +/- "<< Eff_err_sg[i]<<"   effB="<< Eff_bg[i]<<" +/- "<< Eff_err_bg[i]<<endl;

	}

	cout<<__LINE__<<endl;

	TGraph* GraphEff = new TGraph(nbins+1, Eff_bg, Eff_sg);
	GraphEff->SetName(TitleGraph.c_str());
	GraphEff->SetTitle(TitleGraph.c_str());

	return GraphEff;
}


TGraph* GetEffSvsEffB_Optim_SameSample(TTree* Sample, string presel_sig, string presel_bkg, string var, double valmin, double valmax, int npoints, string TitleGraph)
{

	cout << "GetEffSvsEffB_Optim_SameSample"<<endl;

	int nbins = npoints;

	double* Eff_sg = new double[nbins+1];
	double* Eff_err_sg = new double[nbins+1];

	double* Eff_bg = new double[nbins+1];
	double* Eff_err_bg = new double[nbins+1];

	stringstream ss[nbins+1];
	string svalcut;
	string scut;

	string presel_sg = presel_sig;
	string presel_bg = presel_bkg;

	TH1D* Histo_sg = GetHistoWeight(Sample, var, npoints, valmin, valmax, presel_sg, "sg");
	Histo_sg->SetName("sg");
	//cout << "Histo done"<<endl;
	double denom_sg = Histo_sg->Integral();
	// cout << "denom_sg=" << denom_sg<< endl;

	TH1D* Histo_bg = GetHistoWeight(Sample, var, npoints, valmin, valmax, presel_bg, "bg");
	Histo_bg->SetName("bg");
	double denom_bg = Histo_bg->Integral();
	// cout << "denom_bg=" << denom_bg<< endl;

	for (int i=1; i<=nbins; i++){

		//double valcut = valmin*((double)i)/((double)nbins) + valmax*(1-((double)i)/((double)nbins));
		double valcut = valmax*((double)i)/((double)nbins) + valmin*(1-((double)i)/((double)nbins));

		ss[i] << valcut;
		ss[i] >> svalcut;

		double num_sg = Histo_sg->Integral(i, nbins);
		double num_bg = Histo_bg->Integral(i, nbins);

		Eff_sg[i] = num_sg/denom_sg;
		Eff_err_sg[i] = Eff_sg[i] * (sqrt(num_sg)/num_sg + sqrt(denom_sg)/denom_sg);

		Eff_bg[i] = num_bg/denom_bg;
		Eff_err_bg[i] = Eff_bg[i] * (sqrt(num_bg)/num_bg + sqrt(denom_bg)/denom_bg);

		// cout << "i="<<i << " "<<var<< "<" <<svalcut<<"   effS="<<Eff_sg[i]<<" +/- "<< Eff_err_sg[i]<<"   effB="<< Eff_bg[i]<<" +/- "<< Eff_err_bg[i]<<endl;

	}

	TGraph* GraphEff = new TGraph(nbins+1, Eff_bg, Eff_sg);

	GraphEff->SetName(TitleGraph.c_str());
	GraphEff->SetTitle(TitleGraph.c_str());

	return GraphEff;
}

TCanvas* Plot1RocCurves(TTree* ts1, TTree* tb1, string discrim1, string legend1, string selection, string legendtitle, string PicName)
{

	// cout << "Plot1RocCurves"<<endl;

	TGraph* Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim(ts1, tb1, selection, discrim1, -1, 701, 700, "EffSvsRejB_MEMdiscrim_1");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legendtitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}

TCanvas* Plot2RocCurves(TTree* ts1, TTree* tb1, string discrim1, string legend1, TTree* ts2, TTree* tb2, string discrim2, string legend2, string selection, int windowtype, string legentitle, string PicName)
{

	TGraph* Graph_EffSvsRejB_MEMdiscrim_1;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2;

	Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim(ts1, tb1, selection, discrim1, -1, 701, 700, "EffSvsRejB_MEMdiscrim_1");

	if (windowtype==0) Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim(ts2, tb2, selection, discrim2, -1, 701, 700, "EffSvsRejB_MEMdiscrim_2");
	if (windowtype==1) Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim(ts2, tb2, selection, discrim2, -2, 2, 400, "EffSvsRejB_BDT_2");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legentitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}


TCanvas* Plot3RocCurves(TTree* ts1, TTree* tb1, string discrim1, string legend1, TTree* ts2, TTree* tb2, string discrim2, string legend2,TTree* ts3, TTree* tb3, string discrim3, string legend3, string selection, int windowtype, string legendtitle, string PicName)
{

	TGraph* Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim(ts1, tb1, selection, discrim1, -1, 701, 700, "EffSvsRejB_MEMdiscrim_1");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim(ts2, tb2, selection, discrim2, -1, 701, 700, "EffSvsRejB_MEMdiscrim_2");

	TGraph* Graph_EffSvsRejB_MEMdiscrim_3;
	if (windowtype==0) Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim(ts3, tb3, selection, discrim3, -1, 701, 700, "EffSvsRejB_MEMdiscrim_3");
	if (windowtype==1) Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim(ts3, tb3, selection, discrim3, -2, 2, 400, "EffSvsRejB_MEMdiscrim_3");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_3->SetMarkerColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_3->Draw("*same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legendtitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_3->GetName(), legend3.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}

TCanvas* Plot4RocCurves(TTree* ts1, TTree* tb1, string discrim1, string legend1, TTree* ts2, TTree* tb2, string discrim2, string legend2,TTree* ts3, TTree* tb3, string discrim3, string legend3, TTree* ts4, TTree* tb4, string discrim4, string legend4, string selection, string legendtitle, string PicName)
{
	TGraph* Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim(ts1, tb1, selection, discrim1, -1, 701, 700, "EffSvsRejB_MEMdiscrim_1");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim(ts2, tb2, selection, discrim2, -1, 701, 700, "EffSvsRejB_MEMdiscrim_2");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim(ts3, tb3, selection, discrim3, -1, 701, 700, "EffSvsRejB_MEMdiscrim_3");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_4 = GetEffSvsEffB_Optim(ts4, tb4, selection, discrim4, -1, 701, 700, "EffSvsRejB_MEMdiscrim_4");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_3->SetMarkerColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_3->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_4->SetMarkerColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_4->Draw("*same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legendtitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_3->GetName(), legend3.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_4->GetName(), legend4.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}


TCanvas* Plot5RocCurves(TTree* ts1, TTree* tb1, string discrim1, string legend1, TTree* ts2, TTree* tb2, string discrim2, string legend2,TTree* ts3, TTree* tb3, string discrim3, string legend3, TTree* ts4, TTree* tb4, string discrim4, string legend4, TTree* ts5, TTree* tb5, string discrim5, string legend5, string selection, string legendtitle, string PicName)
{
	TGraph* Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim(ts1, tb1, selection, discrim1, -1, 701, 700, "EffSvsRejB_MEMdiscrim_1");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim(ts2, tb2, selection, discrim2, -1, 701, 700, "EffSvsRejB_MEMdiscrim_2");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim(ts3, tb3, selection, discrim3, -1, 701, 700, "EffSvsRejB_MEMdiscrim_3");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_4 = GetEffSvsEffB_Optim(ts4, tb4, selection, discrim4, -1, 701, 700, "EffSvsRejB_MEMdiscrim_4");
	TGraph* Graph_EffSvsRejB_MEMdiscrim_5 = GetEffSvsEffB_Optim(ts5, tb5, selection, discrim5, -1, 701, 700, "EffSvsRejB_MEMdiscrim_5");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_3->SetMarkerColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_3->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_4->SetMarkerColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_4->Draw("*same");

	Graph_EffSvsRejB_MEMdiscrim_5->SetMarkerColor(kBlack);
	Graph_EffSvsRejB_MEMdiscrim_5->SetLineColor(kBlack);
	Graph_EffSvsRejB_MEMdiscrim_5->SetLineWidth(2);
	Graph_EffSvsRejB_MEMdiscrim_5->Draw("*same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legendtitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_3->GetName(), legend3.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_4->GetName(), legend4.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_5->GetName(), legend5.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}

//  #######     ########   #######   ######      ######  ##     ## ########  ##     ## ########  ######
// ##     ##    ##     ## ##     ## ##    ##    ##    ## ##     ## ##     ## ##     ## ##       ##    ##
//        ##    ##     ## ##     ## ##          ##       ##     ## ##     ## ##     ## ##       ##
//  #######     ########  ##     ## ##          ##       ##     ## ########  ##     ## ######    ######
// ##           ##   ##   ##     ## ##          ##       ##     ## ##   ##    ##   ##  ##             ##
// ##           ##    ##  ##     ## ##    ##    ##    ## ##     ## ##    ##    ## ##   ##       ##    ##
// #########    ##     ##  #######   ######      ######   #######  ##     ##    ###    ########  ######

//Add loop here to check if BDT_noMEM has some bins with higher efficiency than BDT_withMEM?
TCanvas* Plot2RocCurves_SameSample(TTree* t, string discrim1, string legend1, TTree* t2, string discrim2, string legend2, string selection, int windowtype, string legentitle, string PicName)
{
	// cout<<"t = "<<t<<endl;

	TGraph* Graph_EffSvsRejB_MEMdiscrim_1;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2;

	string sel_sg = selection + " && classID==0";
	string sel_bg = selection + " && classID==1";

	Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim_SameSample(t, sel_sg, sel_bg, discrim1, -2, 2, 100, "EffSvsRejB_MEMdiscrim_2");
	Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim_SameSample(t2, sel_sg, sel_bg, discrim2, -2, 2, 100, "EffSvsRejB_BDT_2");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("same L");

	//NOTE -- fix :
	//When drawing ROCs as lines, points (0,0) and (1,1) are linked together --> Superimpose white line to cover the original unwanted line b/w the points
	TLine* line = new TLine(0,0,1,1);
	line->SetLineColor(0);
	line->SetLineWidth(2);
	line->Draw("same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legentitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}


//  #######     ########   #######   ######      ######  ##     ## ########  ##     ## ########  ######
// ##     ##    ##     ## ##     ## ##    ##    ##    ## ##     ## ##     ## ##     ## ##       ##    ##
//        ##    ##     ## ##     ## ##          ##       ##     ## ##     ## ##     ## ##       ##
//  #######     ########  ##     ## ##          ##       ##     ## ########  ##     ## ######    ######
//        ##    ##   ##   ##     ## ##          ##       ##     ## ##   ##    ##   ##  ##             ##
// ##     ##    ##    ##  ##     ## ##    ##    ##    ## ##     ## ##    ##    ## ##   ##       ##    ##
//  #######     ##     ##  #######   ######      ######   #######  ##     ##    ###    ########  ######

TCanvas* Plot3RocCurves_SameSample(TTree* t, string discrim1, string legend1, TTree* t2, string discrim2, string legend2, TTree* t3, string discrim3, string legend3, string selection, int windowtype, string legentitle, string PicName){

	TGraph* Graph_EffSvsRejB_MEMdiscrim_1;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_3;

	string sel_sg = selection + " && classID==0";
	string sel_bg = selection + " && classID==1";

	//FIXME
	int npts = 150;

	Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim_SameSample(t, sel_sg, sel_bg, discrim1, -2, 2, npts, "Eff_1");
	Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim_SameSample(t2, sel_sg, sel_bg, discrim2, -2, 2, npts, "Eff_2");
	Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim_SameSample(t3, sel_sg, sel_bg, discrim3, -2, 2, npts, "Eff_3");


	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_3->SetMarkerColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_3->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_3->Draw("same L");

	//NOTE -- fix :
	//When drawing ROCs as lines, points (0,0) and (1,1) are linked together --> Superimpose white line to cover the original unwanted line b/w the points
	TLine* line = new TLine(0,0,1,1);
	line->SetLineColor(0);
	line->SetLineWidth(2);
	line->Draw("same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legentitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_3->GetName(), legend3.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}


// ##           ########   #######   ######      ######  ##     ## ########  ##     ## ########  ######
// ##    ##     ##     ## ##     ## ##    ##    ##    ## ##     ## ##     ## ##     ## ##       ##    ##
// ##    ##     ##     ## ##     ## ##          ##       ##     ## ##     ## ##     ## ##       ##
// ##    ##     ########  ##     ## ##          ##       ##     ## ########  ##     ## ######    ######
// #########    ##   ##   ##     ## ##          ##       ##     ## ##   ##    ##   ##  ##             ##
//       ##     ##    ##  ##     ## ##    ##    ##    ## ##     ## ##    ##    ## ##   ##       ##    ##
//       ##     ##     ##  #######   ######      ######   #######  ##     ##    ###    ########  ######

TCanvas* Plot4RocCurves_SameSample(TTree* t, string discrim1, string legend1, TTree* t2, string discrim2, string legend2, TTree* t3, string discrim3, string legend3, TTree* t4, string discrim4, string legend4, string selection, int windowtype, string legentitle, string PicName){

	TGraph* Graph_EffSvsRejB_MEMdiscrim_1;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_3;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_4;

	string sel_sg = selection + " && classID==0";
	string sel_bg = selection + " && classID==1";

	Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim_SameSample(t, sel_sg, sel_bg, discrim1, -2, 2, 100, "Eff_1");
	Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim_SameSample(t2, sel_sg, sel_bg, discrim2, -2, 2, 100, "Eff_2");
	Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim_SameSample(t3, sel_sg, sel_bg, discrim3, -2, 2, 100, "Eff_3");
	Graph_EffSvsRejB_MEMdiscrim_4 = GetEffSvsEffB_Optim_SameSample(t4, sel_sg, sel_bg, discrim4, -2, 2, 100, "Eff_4");

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_3->SetMarkerColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_3->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_3->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_4->SetMarkerColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_4->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_4->Draw("same L");

	//NOTE -- fix :
	//When drawing ROCs as lines, points (0,0) and (1,1) are linked together --> Superimpose white line to cover the original unwanted line b/w the points
	TLine* line = new TLine(0,0,1,1);
	line->SetLineColor(0);
	line->SetLineWidth(2);
	line->Draw("same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legentitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_3->GetName(), legend3.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_4->GetName(), legend4.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}



// ########    ########   #######   ######      ######  ##     ## ########  ##     ## ########  ######
// ##          ##     ## ##     ## ##    ##    ##    ## ##     ## ##     ## ##     ## ##       ##    ##
// ##          ##     ## ##     ## ##          ##       ##     ## ##     ## ##     ## ##       ##
// #######     ########  ##     ## ##          ##       ##     ## ########  ##     ## ######    ######
//       ##    ##   ##   ##     ## ##          ##       ##     ## ##   ##    ##   ##  ##             ##
// ##    ##    ##    ##  ##     ## ##    ##    ##    ## ##     ## ##    ##    ## ##   ##       ##    ##
//  ######     ##     ##  #######   ######      ######   #######  ##     ##    ###    ########  ######

TCanvas* Plot5RocCurves_SameSample(TTree* t, string discrim1, string legend1, TTree* t2, string discrim2, string legend2, TTree* t3, string discrim3, string legend3, TTree* t4, string discrim4, string legend4, TTree* t5, string discrim5, string legend5, string selection, int windowtype, string legentitle, string PicName){

cout<<__LINE__<<endl;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_1;
	cout<<__LINE__<<endl;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_2;
	cout<<__LINE__<<endl;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_3;
	cout<<__LINE__<<endl;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_4;
	cout<<__LINE__<<endl;
	TGraph* Graph_EffSvsRejB_MEMdiscrim_5;
	cout<<__LINE__<<endl;

	string sel_sg = selection + " && classID==0";
	string sel_bg = selection + " && classID==1";
	cout<<__LINE__<<endl;

	Graph_EffSvsRejB_MEMdiscrim_1 = GetEffSvsEffB_Optim_SameSample(t, sel_sg, sel_bg, discrim1, -2, 2, 100, "Eff_1");
	cout<<__LINE__<<endl;
	Graph_EffSvsRejB_MEMdiscrim_2 = GetEffSvsEffB_Optim_SameSample(t2, sel_sg, sel_bg, discrim2, -2, 2, 100, "Eff_2");
	Graph_EffSvsRejB_MEMdiscrim_3 = GetEffSvsEffB_Optim_SameSample(t3, sel_sg, sel_bg, discrim3, -2, 2, 100, "Eff_3");
	Graph_EffSvsRejB_MEMdiscrim_4 = GetEffSvsEffB_Optim_SameSample(t4, sel_sg, sel_bg, discrim4, -2, 2, 100, "Eff_4");
	Graph_EffSvsRejB_MEMdiscrim_5 = GetEffSvsEffB_Optim_SameSample(t5, sel_sg, sel_bg, discrim5, -2, 2, 100, "Eff_5");
	cout<<__LINE__<<endl;

	TCanvas* CanvasEffSvsRejB = new TCanvas("CanvasEffSvsRejB","EffSvsRejB");

	TH2D* hGrid = new TH2D("","",1000,0,1,1000,0,1);
	hGrid->Draw();
	hGrid->GetYaxis()->SetTitle("Signal Efficiency");
	hGrid->GetXaxis()->SetTitle("Background Efficiency");

	Graph_EffSvsRejB_MEMdiscrim_1->SetMarkerColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineColor(kBlue);
	Graph_EffSvsRejB_MEMdiscrim_1->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_1->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_1->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_2->SetMarkerColor(8); //green
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineColor(8);
	Graph_EffSvsRejB_MEMdiscrim_2->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_2->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_2->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_3->SetMarkerColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineColor(kOrange);
	Graph_EffSvsRejB_MEMdiscrim_3->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_3->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_3->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_4->SetMarkerColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineColor(kRed);
	Graph_EffSvsRejB_MEMdiscrim_4->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_4->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_4->Draw("same L");

	Graph_EffSvsRejB_MEMdiscrim_5->SetMarkerColor(kMagenta);
	Graph_EffSvsRejB_MEMdiscrim_5->SetLineColor(kMagenta);
	Graph_EffSvsRejB_MEMdiscrim_5->SetLineWidth(2);
	// Graph_EffSvsRejB_MEMdiscrim_()->Draw("*same");
	Graph_EffSvsRejB_MEMdiscrim_5->Draw("same L");

	//NOTE -- fix :
	//When drawing ROCs as lines, points (0,0) and (1,1) are linked together --> Superimpose white line to cover the original unwanted line b/w the points
	TLine* line = new TLine(0,0,1,1);
	line->SetLineColor(0);
	line->SetLineWidth(2);
	line->Draw("same");

	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();

	TLegend* legende = new TLegend(0.4, 0.3, 0.9, 0.5, legentitle.c_str());
	legende->SetFillColor(0);
	legende->SetBorderSize(0);
	legende->SetTextSize(0.03);
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_1->GetName(), legend1.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_2->GetName(), legend2.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_3->GetName(), legend3.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_4->GetName(), legend4.c_str(), "l");
	legende->AddEntry(Graph_EffSvsRejB_MEMdiscrim_5->GetName(), legend5.c_str(), "l");
	legende->Draw();
	CanvasEffSvsRejB->Update();
	CanvasEffSvsRejB->Modified();
	CanvasEffSvsRejB->Print(PicName.c_str());

	return CanvasEffSvsRejB;
}














// ##     ##    ###    #### ##    ##
// ###   ###   ## ##    ##  ###   ##
// #### ####  ##   ##   ##  ####  ##
// ## ### ## ##     ##  ##  ## ## ##
// ##     ## #########  ##  ##  ####
// ##     ## ##     ##  ##  ##   ###
// ##     ## ##     ## #### ##    ##


int main()
{
		gStyle->SetOptStat(0);

		setTDRStyle();

		vector<string> channel;
		channel.push_back("uuu");
		channel.push_back("eeu");
		channel.push_back("uue");
		channel.push_back("eee");

		TFile* file1[4]; string name1; string label1;
		TFile* file2[4]; string name2; string label2;
		TFile* file3[4]; string name3; string label3;
		TFile* file4[4]; string name4; string label4;
		TFile* file5[4]; string name5; string label5;
		TTree* tree1[4];
		TTree* tree2[4];
		TTree* tree3[4];
		TTree* tree4[4];
		TTree* tree5[4];

		string dir = "../outputs/";
		string name, picname, legendtitle;

		string selection = "1";

		name1 = dir + "ROCS/conf1"; label1 = "1";
		name2 = dir + "ROCS/conf2"; label2 = "2";
		name3 = dir + "ROCS/conf3"; label3 = "3";
		name4 = dir + "ROCS/conf4"; label4 = "4";
		name5 = dir + "ROCS/conf5"; label5 = "5";

		for (int i=0; i<channel.size(); i++)
		{
			//FIXME -- no cuts taken into account in filenames here -- (ex : MET10mTW10)
			string suffix_tZq = "/BDTfakeSR_"+ channel[i]+ "_badMuonEq0_ContainsBadJetEq0_fourthLep10Eq0_NJetsMin1Max4_NBJetsEq1.root";
			// string suffix_tZq = "/BDT_"+ channel[i]+ "_badMuonEq0_ContainsBadJetEq0_fourthLep10Eq0_NJetsMin1Max4_NBJetsEq1.root";
			// string suffix_tZq = "/BDT_"+ channel[i]+ "_NJetsMin1Max4_NBJetsEq1.root";
			string suffix_ttZ = "/BDTttZ_"+ channel[i]+ "_NJetsMin1_NBJetsMin1.root";

			//--- tZq SR
			name = name1 + suffix_tZq;
			file1[i] = 0;
			file1[i] = new TFile(name.c_str());  if(!file1[i]) cout<<"File not found !"<<endl;
			tree1[i] = 0;
			tree1[i] = (TTree*)file1[i]->Get("TestTree"); if(!tree1[i]) cout<<"Tree not found !"<<endl;

			name = name2 + suffix_tZq;
			file2[i] = 0;
			file2[i] = new TFile(name.c_str()); if(!file2[i]) cout<<"File not found !"<<endl;
			tree2[i] = 0;
			tree2[i] = (TTree*)file2[i]->Get("TestTree"); if(!tree2[i]) cout<<"Tree not found !"<<endl;

			name = name3 + suffix_tZq;
			file3[i] = 0;
			file3[i] = new TFile(name.c_str()); if(!file3[i]) cout<<"File not found !"<<endl;
			tree3[i] = 0;
			tree3[i] = (TTree*)file3[i]->Get("TestTree"); if(!tree3[i]) cout<<"Tree not found !"<<endl;

			name = name4 + suffix_tZq;
			file4[i] = 0;
			file4[i] = new TFile(name.c_str()); if(!file4[i]) cout<<"File not found !"<<endl;
			tree4[i] = 0;
			tree4[i] = (TTree*)file4[i]->Get("TestTree"); if(!tree4[i]) cout<<"Tree not found !"<<endl;

			name = name5 + suffix_tZq;
			file5[i] = 0;
			file5[i] = new TFile(name.c_str()); if(!file5[i]) cout<<"File not found !"<<endl;
			tree5[i] = 0;
			tree5[i] = (TTree*)file5[i]->Get("TestTree"); if(!tree5[i]) cout<<"Tree not found !"<<endl;

			//FIXME -- need to change variable 'name' accordingly here (additional cuts in filename, etc.)
			name = channel[i]+"_badMuonEq0_ContainsBadJetEq0_fourthLep10Eq0_NJetsMin1Max4_NBJetsEq1";
			// name = channel[i]+"_NJetsMin1Max4_NBJetsEq1";
			picname = "CompareBDT_"+ channel[i]+"_SignalRegion.png";
			legendtitle = "tZq Signal Region - "+ channel[i];

			// Plot2RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, selection, 0, legendtitle, picname);
			Plot3RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, tree3[i], name, label3, selection, 0, legendtitle, picname);
			// Plot4RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, tree3[i], name, label3, tree4[i], name, label4, selection, 0, legendtitle, picname);
			// Plot5RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, tree3[i], name, label3, tree4[i], name, label4, tree5[i], name, label5, selection, 0, legendtitle, picname);


			//--- ttZ CR
			name = name1 + suffix_ttZ;
			file1[i] = 0;
			file1[i] = new TFile(name.c_str());  if(!file1[i]) cout<<"File not found !"<<endl;
			tree1[i] = 0;
			tree1[i] = (TTree*)file1[i]->Get("TestTree"); if(!tree1[i]) cout<<"Tree not found !"<<endl;

			name = name2 + suffix_ttZ;
			file2[i] = 0;
			file2[i] = new TFile(name.c_str()); if(!file2[i]) cout<<"File not found !"<<endl;
			tree2[i] = 0;
			tree2[i] = (TTree*)file2[i]->Get("TestTree"); if(!tree2[i]) cout<<"Tree not found !"<<endl;

			name = name3 + suffix_ttZ;
			file3[i] = 0;
			file3[i] = new TFile(name.c_str()); if(!file3[i]) cout<<"File not found !"<<endl;
			tree3[i] = 0;
			tree3[i] = (TTree*)file3[i]->Get("TestTree"); if(!tree3[i]) cout<<"Tree not found !"<<endl;

			name = name4 + suffix_ttZ;
			file4[i] = 0;
			file4[i] = new TFile(name.c_str()); if(!file4[i]) cout<<"File not found !"<<endl;
			tree4[i] = 0;
			tree4[i] = (TTree*)file4[i]->Get("TestTree"); if(!tree4[i]) cout<<"Tree not found !"<<endl;

			name = name5 + suffix_ttZ;
			file5[i] = 0;
			file5[i] = new TFile(name.c_str()); if(!file5[i]) cout<<"File not found !"<<endl;
			tree5[i] = 0;
			tree5[i] = (TTree*)file5[i]->Get("TestTree"); if(!tree5[i]) cout<<"Tree not found !"<<endl;


			name = channel[i]+"_NJetsMin1_NBJetsMin1";
			picname = "CompareBDT_"+ channel[i]+"_ttZControlRegion.png";
			legendtitle = "ttZ Control Region - "+ channel[i];

			// Plot2RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, selection, 0, legendtitle, picname);
			// Plot3RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, tree3[i], name, label3, selection, 0, legendtitle, picname);
			// Plot4RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, tree3[i], name, label3, tree4[i], name, label4, selection, 0, legendtitle, picname);
			// Plot5RocCurves_SameSample(tree1[i], name, label1, tree2[i], name, label2, tree3[i], name, label3, tree4[i], name, label4, tree5[i], name, label5, selection, 0, legendtitle, picname);
		}

		return 0;
}








//Try to return smart pointers ??
/*// Use a smart, reference counted pointer that handles deallocation itself.
boost::shared_ptr<Foo> GetFoo2()
{
   boost::shared_ptr<Foo> f(new Foo);
   // Init f
   return f;
}*/

// Second point to remember is that, it is not good idea to return the address of a local variable to outside of the function, so you would have to define the local variable as static variable.
