#include "CMS_lumi.h"

void makeJetFractionComparison(string file1, string file2, string outputDIR){

  system(("mkdir -p "+outputDIR).c_str());
  setTDRStyle();
  gROOT->SetBatch(kTRUE);

  TFile* input1 = TFile::Open(file1.c_str());
  TFile* input2 = TFile::Open(file2.c_str());

  TH1F* jet_emfraction_2p25_2p5_1 = (TH1F*) input1->Get("jet_emfraction_2p25_2p5");
  TH1F* jet_emfraction_2p25_2p5_2 = (TH1F*) input2->Get("jet_emfraction_2p25_2p5");
  TH1F* jet_emfraction_2p5_2p75_1 = (TH1F*) input1->Get("jet_emfraction_2p5_2p75");
  TH1F* jet_emfraction_2p5_2p75_2 = (TH1F*) input2->Get("jet_emfraction_2p5_2p75");
  TH1F* jet_emfraction_2p75_3p0_1 = (TH1F*) input1->Get("jet_emfraction_2p75_3p0");
  TH1F* jet_emfraction_2p75_3p0_2 = (TH1F*) input2->Get("jet_emfraction_2p75_3p0");

  jet_emfraction_2p25_2p5_1->Scale(1./jet_emfraction_2p25_2p5_1->Integral());
  jet_emfraction_2p25_2p5_2->Scale(1./jet_emfraction_2p25_2p5_2->Integral());

  jet_emfraction_2p5_2p75_1->Scale(1./jet_emfraction_2p5_2p75_1->Integral());
  jet_emfraction_2p5_2p75_2->Scale(1./jet_emfraction_2p5_2p75_2->Integral());

  jet_emfraction_2p75_3p0_1->Scale(1./jet_emfraction_2p75_3p0_1->Integral());
  jet_emfraction_2p75_3p0_2->Scale(1./jet_emfraction_2p75_3p0_2->Integral());

  TCanvas* canvas = new TCanvas("canvas","canvas",600,600);
  canvas->cd();
  
  jet_emfraction_2p5_2p75_1->GetXaxis()->SetTitle("EM fraction");
  jet_emfraction_2p5_2p75_1->GetYaxis()->SetTitle("a.u.");
  jet_emfraction_2p5_2p75_1->GetXaxis()->SetTitleOffset(1.1);
  jet_emfraction_2p5_2p75_1->GetYaxis()->SetTitleOffset(1.1);

  jet_emfraction_2p5_2p75_1->SetLineColor(kRed);
  jet_emfraction_2p5_2p75_1->SetLineWidth(2);
  jet_emfraction_2p5_2p75_1->SetMarkerColor(kRed);
  jet_emfraction_2p5_2p75_1->SetMarkerStyle(20);
  jet_emfraction_2p5_2p75_1->SetMarkerSize(1.1);

  jet_emfraction_2p5_2p75_2->SetLineColor(kRed);
  jet_emfraction_2p5_2p75_2->SetMarkerColor(kRed);
  jet_emfraction_2p5_2p75_2->SetMarkerStyle(20);
  jet_emfraction_2p5_2p75_2->SetMarkerSize(1.1);

  jet_emfraction_2p5_2p75_1->Draw("hist");
  jet_emfraction_2p5_2p75_2->Draw("EPsame");

  jet_emfraction_2p25_2p5_1->SetLineColor(kBlack);
  jet_emfraction_2p25_2p5_1->SetMarkerColor(kBlack);
  jet_emfraction_2p25_2p5_1->SetMarkerStyle(20);
  jet_emfraction_2p25_2p5_1->SetMarkerSize(1.1);
  jet_emfraction_2p25_2p5_1->SetLineWidth(2);

  jet_emfraction_2p25_2p5_2->SetLineColor(kBlack);
  jet_emfraction_2p25_2p5_2->SetMarkerColor(kBlack);
  jet_emfraction_2p25_2p5_2->SetMarkerStyle(20);
  jet_emfraction_2p25_2p5_2->SetMarkerSize(1.1);

  jet_emfraction_2p25_2p5_1->Draw("hist same");
  jet_emfraction_2p25_2p5_2->Draw("EPsame");

  jet_emfraction_2p75_3p0_1->SetLineColor(kBlue);
  jet_emfraction_2p75_3p0_1->SetMarkerColor(kBlue);
  jet_emfraction_2p75_3p0_1->SetMarkerStyle(20);
  jet_emfraction_2p75_3p0_1->SetMarkerSize(1.1);
  jet_emfraction_2p75_3p0_1->SetLineWidth(2);

  jet_emfraction_2p75_3p0_2->SetLineColor(kBlue);
  jet_emfraction_2p75_3p0_2->SetMarkerColor(kBlue);
  jet_emfraction_2p75_3p0_2->SetMarkerStyle(20);
  jet_emfraction_2p75_3p0_2->SetMarkerSize(1.1);

  jet_emfraction_2p75_3p0_1->Draw("hist same");
  jet_emfraction_2p75_3p0_2->Draw("EPsame");


  jet_emfraction_2p5_2p75_1->GetYaxis()->SetRangeUser(0.,max(jet_emfraction_2p5_2p75_1->GetMaximum(),max(jet_emfraction_2p5_2p75_2->GetMaximum(),max(jet_emfraction_2p75_3p0_1->GetMaximum(),jet_emfraction_2p75_3p0_2->GetMaximum())))*1.75);
  
  CMS_lumi(canvas,"35.9");

  TLegend leg (0.55,0.65,0.9,0.9);
  leg.SetFillColor(0);
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);  
  leg.AddEntry(jet_emfraction_2p25_2p5_1,"Single-Muon 2.25 < |#eta| < 2.5","L"); 
  leg.AddEntry(jet_emfraction_2p25_2p5_2,"JetHT 2.25 < |#eta| < 2.5","EP"); 
  leg.AddEntry(jet_emfraction_2p5_2p75_1,"Single-Muon 2.5 < |#eta| < 2.75","L"); 
  leg.AddEntry(jet_emfraction_2p5_2p75_2,"JetHT 2.5 < |#eta| < 2.75","EP"); 
  leg.AddEntry(jet_emfraction_2p75_3p0_1,"Single-Muon 2.75 < |#eta| < 3.0","L"); 
  leg.AddEntry(jet_emfraction_2p75_3p0_2,"JetHT 2.75 < |#eta| < 3.0","EP"); 

  leg.Draw();

  canvas->SaveAs((outputDIR+"/jet_emfrac_comparison.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/jet_emfrac_comparison.pdf").c_str(),"pdf");

}
