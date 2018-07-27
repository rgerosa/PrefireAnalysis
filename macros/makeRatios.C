#include "CMS_lumi.h"

void makeRatios(string inputFile_1, string inputFile_2, string outputDIR, string postfix){

  gROOT->SetBatch(kTRUE);
  setTDRStyle();
  system(("mkdir -p "+outputDIR).c_str());

  TFile* input_1 = TFile::Open(inputFile_1.c_str(),"READ");
  TFile* input_2 = TFile::Open(inputFile_2.c_str(),"READ");

  TGraphAsymmErrors* eff_eta_2p25_2p5_file1  = NULL;
  TGraphAsymmErrors* eff_eta_2p25_2p5_file2  = NULL;
  TGraphAsymmErrors* eff_eta_2p5_2p75_file1  = NULL;
  TGraphAsymmErrors* eff_eta_2p5_2p75_file2  = NULL;
  TGraphAsymmErrors* eff_eta_2p75_3p0_file1  = NULL;
  TGraphAsymmErrors* eff_eta_2p75_3p0_file2  = NULL;

  eff_eta_2p25_2p5_file1  = ( (TGraphAsymmErrors*) input_1->Get("efficiency_UGT_ptem_eta_2p25_2p5"));
  eff_eta_2p25_2p5_file2  = ( (TGraphAsymmErrors*) input_2->Get("efficiency_UGT_ptem_eta_2p25_2p5"));
  eff_eta_2p5_2p75_file1  = ( (TGraphAsymmErrors*) input_1->Get("efficiency_UGT_ptem_eta_2p5_2p75"));
  eff_eta_2p5_2p75_file2  = ( (TGraphAsymmErrors*) input_2->Get("efficiency_UGT_ptem_eta_2p5_2p75"));
  eff_eta_2p75_3p0_file1  = ( (TGraphAsymmErrors*) input_1->Get("efficiency_UGT_ptem_eta_2p75_3p0"));
  eff_eta_2p75_3p0_file2  = ( (TGraphAsymmErrors*) input_2->Get("efficiency_UGT_ptem_eta_2p75_3p0"));    

  TGraphAsymmErrors* ratio_2p25_2p5 = new TGraphAsymmErrors();
  TGraphAsymmErrors* ratio_2p5_2p75 = new TGraphAsymmErrors();
  TGraphAsymmErrors* ratio_2p75_3p0 = new TGraphAsymmErrors();

  for(int iPoint = 0; iPoint < eff_eta_2p25_2p5_file1->GetN(); iPoint++){
    double x_num, y_num = 0;
    double x_den, y_den = 0;
    eff_eta_2p25_2p5_file1->GetPoint(iPoint,x_num,y_num);
    eff_eta_2p25_2p5_file2->GetPoint(iPoint,x_den,y_den);
    ratio_2p25_2p5->SetPoint(iPoint,x_num,y_num/y_den);
    ratio_2p25_2p5->SetPoint(iPoint,x_num,y_num/y_den);
    ratio_2p25_2p5->SetPointError(iPoint,0,0,sqrt(pow(eff_eta_2p25_2p5_file1->GetErrorYlow(iPoint),2)/pow(y_den,2)+pow(y_num,2)/(pow(y_den,4))*pow(eff_eta_2p25_2p5_file2->GetErrorYlow(iPoint),2)),
				  sqrt(pow(eff_eta_2p25_2p5_file1->GetErrorYhigh(iPoint),2)/pow(y_den,2)+pow(y_num,2)/(pow(y_den,4))*pow(eff_eta_2p25_2p5_file2->GetErrorYhigh(iPoint),2)));
  }

  for(int iPoint = 0; iPoint < eff_eta_2p5_2p75_file1->GetN(); iPoint++){
    double x_num, y_num = 0;
    double x_den, y_den = 0;
    eff_eta_2p5_2p75_file1->GetPoint(iPoint,x_num,y_num);
    eff_eta_2p5_2p75_file2->GetPoint(iPoint,x_den,y_den);
    ratio_2p5_2p75->SetPoint(iPoint,x_num,y_num/y_den);
    ratio_2p5_2p75->SetPointError(iPoint,0,0,sqrt(pow(eff_eta_2p5_2p75_file1->GetErrorYlow(iPoint),2)/pow(y_den,2)+pow(y_num,2)/(pow(y_den,4))*pow(eff_eta_2p5_2p75_file2->GetErrorYlow(iPoint),2)),
				  sqrt(pow(eff_eta_2p5_2p75_file1->GetErrorYhigh(iPoint),2)/pow(y_den,2)+pow(y_num,2)/(pow(y_den,4))*pow(eff_eta_2p5_2p75_file2->GetErrorYhigh(iPoint),2)));
  }

  for(int iPoint = 0; iPoint < eff_eta_2p75_3p0_file1->GetN(); iPoint++){
    double x_num, y_num = 0;
    double x_den, y_den = 0;
    eff_eta_2p75_3p0_file1->GetPoint(iPoint,x_num,y_num);
    eff_eta_2p75_3p0_file2->GetPoint(iPoint,x_den,y_den);
    ratio_2p75_3p0->SetPoint(iPoint,x_num,y_num/y_den);
    ratio_2p75_3p0->SetPoint(iPoint,x_num,y_num/y_den);
    ratio_2p75_3p0->SetPointError(iPoint,0,0,sqrt(pow(eff_eta_2p75_3p0_file1->GetErrorYlow(iPoint),2)/pow(y_den,2)+pow(y_num,2)/(pow(y_den,4))*pow(eff_eta_2p75_3p0_file2->GetErrorYlow(iPoint),2)),
				  sqrt(pow(eff_eta_2p75_3p0_file1->GetErrorYhigh(iPoint),2)/pow(y_den,2)+pow(y_num,2)/(pow(y_den,4))*pow(eff_eta_2p75_3p0_file2->GetErrorYhigh(iPoint),2)));
  }


  TCanvas* canvas = new TCanvas("canvas","canvas",650,600);
  
  ratio_2p25_2p5->GetXaxis()->SetTitle("p_{T} [GeV]");
  ratio_2p25_2p5->GetYaxis()->SetTitle("Ratio");
  ratio_2p25_2p5->GetXaxis()->SetRangeUser(80.,500);
  ratio_2p25_2p5->GetYaxis()->SetRangeUser(0.5,2.5);  
  ratio_2p25_2p5->SetMarkerColor(kBlack);
  ratio_2p25_2p5->SetLineColor(kBlack);
  ratio_2p25_2p5->SetMarkerSize(1.2);
  ratio_2p25_2p5->SetMarkerStyle(20);
  ratio_2p25_2p5->Draw("AEPL");

  ratio_2p5_2p75->SetMarkerColor(kRed);
  ratio_2p5_2p75->SetLineColor(kRed);
  ratio_2p5_2p75->SetMarkerSize(1.2);
  ratio_2p5_2p75->SetMarkerStyle(20);
  ratio_2p5_2p75->Draw("EPLsame");

  ratio_2p75_3p0->SetMarkerColor(kBlue);
  ratio_2p75_3p0->SetLineColor(kBlue);
  ratio_2p75_3p0->SetMarkerSize(1.2);
  ratio_2p75_3p0->SetMarkerStyle(20);
  ratio_2p75_3p0->Draw("EPLsame");

  TLegend leg (0.2,0.6,0.55,0.86);
  leg.SetFillColor(0);
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);
  leg.AddEntry(ratio_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(ratio_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(ratio_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");
  
  CMS_lumi(canvas,"");
  
  canvas->SaveAs((outputDIR+"/ratio_"+postfix+".png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/ratio_"+postfix+".pdf").c_str(),"pdf");

}
