#include "CMS_lumi.h"

#include "Math/GenVector/LorentzVector.h"
#include "Math/GenVector/VectorUtil.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TROOT.h"
#include "TFile.h"
#include "TMath.h"
#include "TTreeReader.h"
#include "TLegend.h"
#include "TGraphAsymmErrors.h"

#include <vector>
#include <string>

using namespace ROOT::Math::VectorUtil;

float deltaPhi (float phi1, float phi2){
  if(fabs(phi1-phi2) < TMath::Pi())
    return fabs(phi1-phi2);
  else
    return 2*TMath::Pi()-fabs(phi1-phi2);
}



typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>> LorentzVector;

enum class analysisRegion {MET,SingleMuon,JetHT};

vector<float> pt_binning  = {30,40,50,65,80,100,125,150,175,200,225,250,275,300,400,500,600};
vector<float> ptem_binning  = {30,40,50,60,70,80,90,100,125,150,175,200,250,300,400};
vector<float> abseta_binning = {0.0,0.25,0.50,0.75,1.0,1.25,1.50,1.75,2.0,2.25,2.5,2.75,3.0,3.25,3.5,4.0};
vector<float> eta_binning = {-4.0,-3.5,-3.0,-2.5,-2.0,-1.5,-1.0,-0.5,0.0,0.50,1.0,1.50,2.0,2.5,3.0,3.5,4.0};
vector<float> phi_binning = {-3.14,-3.0,-2.75,-2.50,-2.25,-2.0,-1.75,-1.50,-1.25,-1.00,-0.75,-0.50,-0.25,0,0.25,0.50,0.75,1.00,1.25,1.50,1.75,2.00,2.25,2.50,2.75,3.0,3.14};

void plotEfficiencyMap(TCanvas* canvas, TH2F* efficiency, string xaxisName, string yaxisName, string zaxisname, string postfix, string outputDIR){

  canvas->SetRightMargin(0.18);
  efficiency->GetXaxis()->SetTitle(xaxisName.c_str());
  efficiency->GetXaxis()->SetTitleOffset(1.15);
  efficiency->GetYaxis()->SetTitle(yaxisName.c_str());
  efficiency->GetYaxis()->SetTitleOffset(1.15);
  efficiency->GetZaxis()->SetTitle(zaxisname.c_str());
  efficiency->GetZaxis()->SetTitleOffset(1.15);
  efficiency->GetZaxis()->SetRangeUser(0,1);
  efficiency->Draw("colz text");
  
  CMS_lumi(canvas,"");
  if(TString(xaxisName).Contains("p_{T}"))
    canvas->SetLogx(1);
  canvas->SaveAs((outputDIR+"/"+postfix+".png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/"+postfix+".pdf").c_str(),"pdf");
  canvas->SetLogx(0);
  
}

float jetpt_threshold = 100;

void makePreFiringAnalysisWithPrescaled(string inputDIR,
					string outputDIR,
					analysisRegion region,
					bool applyVBFcuts    = false,
					bool applyMuonCuts   = false,
					bool filterHotTowers = false,
					bool applyMinDphiCut = false
					){
  

  system(("mkdir -p "+outputDIR).c_str());
  setTDRStyle();
  gROOT->SetBatch(kTRUE);
  gStyle->SetPalette(kLightTemperature);

  ///// set input
  TChain* chain = new TChain("ntuple/tree","");
  chain->Add((inputDIR+"/*root").c_str());
  long int totalEvents = chain->GetEntries();

  chain->LoadTree(0);
  TTreeReader reader(chain);
  TTreeReaderValue<std::vector<LorentzVector> > jet_p4 (reader,"jet_p4");
  TTreeReaderValue<std::vector<float> > jet_neutralEmFrac (reader,"jet_neutralEmFrac");
  TTreeReaderValue<std::vector<LorentzVector> > electron_p4 (reader,"electron_p4");
  TTreeReaderValue<std::vector<bool> > electron_tight (reader,"electron_tight");
  TTreeReaderValue<std::vector<LorentzVector> > photon_p4 (reader,"photon_p4");
  TTreeReaderValue<std::vector<bool> > photon_tight (reader,"photon_tight");
  TTreeReaderValue<std::vector<LorentzVector> > muon_p4 (reader,"muon_p4");
  TTreeReaderValue<std::vector<bool> > muon_tight (reader,"muon_tight");

  TTreeReaderValue<std::vector<int> > L1EG_bx (reader,"L1EG_bx");
  TTreeReaderValue<std::vector<LorentzVector> > L1EG_p4 (reader,"L1EG_p4");
  TTreeReaderValue<std::vector<int> > L1EG_iso (reader,"L1EG_iso");

  TTreeReaderValue<std::vector<int> > L1Jet_bx (reader,"L1Jet_bx");
  TTreeReaderValue<std::vector<LorentzVector> > L1Jet_p4 (reader,"L1Jet_p4"); 
  TTreeReaderValue<std::vector<int> > L1Mu_bx (reader,"L1Mu_bx");
  TTreeReaderValue<std::vector<LorentzVector> > L1Mu_p4 (reader,"L1Mu_p4");
  TTreeReaderValue<std::vector<int> > L1MET_bx (reader,"L1MET_bx");
  TTreeReaderValue<std::vector<LorentzVector> > L1MET_p4 (reader,"L1MET_p4");
  
  TTreeReaderValue<std::vector<int> > L1GtBx (reader,"L1GtBx");

  TTreeReaderValue<std::vector<int> > L1EGSeeds_bx (reader,"L1EGSeeds_bx");
  TTreeReaderValue<std::vector<int> > L1IsoEGSeeds_bx (reader,"L1IsoEGSeeds_bx");
  TTreeReaderValue<std::vector<std::string> > L1EGSeeds_name (reader,"L1EGSeeds_name");
  TTreeReaderValue<std::vector<std::string> > L1IsoEGSeeds_name (reader,"L1IsoEGSeeds_name");

  TTreeReaderValue<uint8_t> flaghbhenoise (reader,"flaghbhenoise");
  TTreeReaderValue<uint8_t> flaghbheiso (reader,"flaghbheiso");
  TTreeReaderValue<uint8_t> flageebadsc (reader,"flageebadsc");
  TTreeReaderValue<uint8_t> flaggoodvertices (reader,"flaggoodvertices");
  TTreeReaderValue<uint8_t> flagglobaltighthalo (reader,"flagglobaltighthalo");
  TTreeReaderValue<uint8_t> flagbadchpf (reader,"flagbadchpf");
  TTreeReaderValue<uint8_t> flagbadpfmu (reader,"flagbadpfmu");

  TTreeReaderValue<float > met (reader,"met");
  TTreeReaderValue<float > metphi (reader,"metphi");
  TTreeReaderValue<float > calomet (reader,"calomet");

  // IsoEG efficiencies
  TH2F* denL1IsoEG_pt_eta = new TH2F("denL1IsoEG_pt_eta","denL1IsoEG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* denL1IsoEG_ptem_eta = new TH2F("denL1IsoEG_ptem_eta","denL1IsoEG_ptem_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1IsoEG_pt_eta_bx0  = new TH2F("L1IsoEG_pt_eta_bx0","L1IsoEG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1IsoEG_pt_eta_bxm1 = new TH2F("L1IsoEG_pt_eta_bxm1","L1IsoEG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1IsoEG_pt_eta_bxm2 = new TH2F("L1IsoEG_pt_eta_bxm2","L1IsoEG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1IsoEG_ptem_eta_bx0  = new TH2F("L1IsoEG_ptem_eta_bx0","L1IsoEG_ptem_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  denL1IsoEG_pt_eta->Sumw2();
  denL1IsoEG_ptem_eta->Sumw2();
  L1IsoEG_pt_eta_bx0->Sumw2();
  L1IsoEG_pt_eta_bxm1->Sumw2();
  L1IsoEG_pt_eta_bxm2->Sumw2();

  TH2F* denL1IsoEG_eta_phi = new TH2F("denL1IsoEG_eta_phi","denL1IsoEG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1IsoEG_eta_phi_bx0  = new TH2F("L1IsoEG_eta_phi_bx0","L1IsoEG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1IsoEG_eta_phi_bxm1 = new TH2F("L1IsoEG_eta_phi_bxm1","L1IsoEG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1IsoEG_eta_phi_bxm2 = new TH2F("L1IsoEG_eta_phi_bxm2","L1IsoEG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  denL1IsoEG_eta_phi->Sumw2();
  L1IsoEG_eta_phi_bx0->Sumw2();
  L1IsoEG_eta_phi_bxm1->Sumw2();
  L1IsoEG_eta_phi_bxm2->Sumw2();

  TH1F* L1IsoEG_pt_eta_2p25_2p5_denom =  new TH1F("L1IsoEG_pt_eta_2p25_2p5_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1IsoEG_pt_eta_2p5_2p75_denom =  new TH1F("L1IsoEG_pt_eta_2p5_2p75_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1IsoEG_pt_eta_2p75_3p0_denom =  new TH1F("L1IsoEG_pt_eta_2p75_3p0_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1IsoEG_pt_eta_2p25_2p5_bxm1 =  new TH1F("L1IsoEG_pt_eta_2p25_2p5_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1IsoEG_pt_eta_2p5_2p75_bxm1 =  new TH1F("L1IsoEG_pt_eta_2p5_2p75_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1IsoEG_pt_eta_2p75_3p0_bxm1 =  new TH1F("L1IsoEG_pt_eta_2p75_3p0_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  L1IsoEG_pt_eta_2p25_2p5_denom->Sumw2();
  L1IsoEG_pt_eta_2p5_2p75_denom->Sumw2();
  L1IsoEG_pt_eta_2p75_3p0_denom->Sumw2();
  L1IsoEG_pt_eta_2p25_2p5_bxm1->Sumw2();
  L1IsoEG_pt_eta_2p5_2p75_bxm1->Sumw2();
  L1IsoEG_pt_eta_2p75_3p0_bxm1->Sumw2();

  // EG efficiencies
  TH2F* denL1EG_pt_eta = new TH2F("denL1EG_pt_eta","denL1EG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1EG_pt_eta_bx0  = new TH2F("L1EG_pt_eta_bx0","L1EG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1EG_pt_eta_bxm1 = new TH2F("L1EG_pt_eta_bxm1","L1EG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1EG_pt_eta_bxm2 = new TH2F("L1EG_pt_eta_bxm2","L1EG_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  denL1EG_pt_eta->Sumw2();
  L1EG_pt_eta_bx0->Sumw2();
  L1EG_pt_eta_bxm1->Sumw2();
  L1EG_pt_eta_bxm2->Sumw2();

  TH2F* denL1EG_eta_phi = new TH2F("denL1EG_eta_phi","denL1EG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1EG_eta_phi_bx0  = new TH2F("L1EG_eta_phi_bx0","L1EG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1EG_eta_phi_bxm1 = new TH2F("L1EG_eta_phi_bxm1","L1EG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1EG_eta_phi_bxm2 = new TH2F("L1EG_eta_phi_bxm2","L1EG_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  denL1EG_eta_phi->Sumw2();
  L1EG_eta_phi_bx0->Sumw2();
  L1EG_eta_phi_bxm1->Sumw2();
  L1EG_eta_phi_bxm2->Sumw2();

  TH1F* L1EG_pt_eta_2p25_2p5_denom =  new TH1F("L1EG_pt_eta_2p25_2p5_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EG_pt_eta_2p5_2p75_denom =  new TH1F("L1EG_pt_eta_2p5_2p75_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EG_pt_eta_2p75_3p0_denom =  new TH1F("L1EG_pt_eta_2p75_3p0_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EG_pt_eta_2p25_2p5_bxm1 =  new TH1F("L1EG_pt_eta_2p25_2p5_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EG_pt_eta_2p5_2p75_bxm1 =  new TH1F("L1EG_pt_eta_2p5_2p75_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EG_pt_eta_2p75_3p0_bxm1 =  new TH1F("L1EG_pt_eta_2p75_3p0_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  L1EG_pt_eta_2p25_2p5_denom->Sumw2();
  L1EG_pt_eta_2p5_2p75_denom->Sumw2();
  L1EG_pt_eta_2p75_3p0_denom->Sumw2();
  L1EG_pt_eta_2p25_2p5_bxm1->Sumw2();
  L1EG_pt_eta_2p5_2p75_bxm1->Sumw2();
  L1EG_pt_eta_2p75_3p0_bxm1->Sumw2();

  // EG efficiencies
  TH2F* denL1EGOR_pt_eta = new TH2F("denL1EGOR_pt_eta","denL1EGOR_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1EGOR_pt_eta_bx0  = new TH2F("L1EGOR_pt_eta_bx0","L1EGOR_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1EGOR_pt_eta_bxm1 = new TH2F("L1EGOR_pt_eta_bxm1","L1EGOR_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  TH2F* L1EGOR_pt_eta_bxm2 = new TH2F("L1EGOR_pt_eta_bxm2","L1EGOR_pt_eta",pt_binning.size()-1,&pt_binning[0],abseta_binning.size()-1,&abseta_binning[0]);
  denL1EGOR_pt_eta->Sumw2();
  L1EGOR_pt_eta_bx0->Sumw2();
  L1EGOR_pt_eta_bxm1->Sumw2();
  L1EGOR_pt_eta_bxm2->Sumw2();

  TH2F* denL1EGOR_eta_phi = new TH2F("denL1EGOR_eta_phi","denL1EGOR_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1EGOR_eta_phi_bx0  = new TH2F("L1EGOR_eta_phi_bx0","L1EGOR_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1EGOR_eta_phi_bxm1 = new TH2F("L1EGOR_eta_phi_bxm1","L1EGOR_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  TH2F* L1EGOR_eta_phi_bxm2 = new TH2F("L1EGOR_eta_phi_bxm2","L1EGOR_eta_phi",eta_binning.size()-1,&eta_binning[0],phi_binning.size()-1,&phi_binning[0]);
  denL1EGOR_eta_phi->Sumw2();
  L1EGOR_eta_phi_bx0->Sumw2();
  L1EGOR_eta_phi_bxm1->Sumw2();
  L1EGOR_eta_phi_bxm2->Sumw2();

  TH1F* L1EGOR_pt_eta_2p25_2p5_denom =  new TH1F("L1EGOR_pt_eta_2p25_2p5_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EGOR_pt_eta_2p5_2p75_denom =  new TH1F("L1EGOR_pt_eta_2p5_2p75_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EGOR_pt_eta_2p75_3p0_denom =  new TH1F("L1EGOR_pt_eta_2p75_3p0_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EGOR_pt_eta_2p25_2p5_bxm1 =  new TH1F("L1EGOR_pt_eta_2p25_2p5_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EGOR_pt_eta_2p5_2p75_bxm1 =  new TH1F("L1EGOR_pt_eta_2p5_2p75_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* L1EGOR_pt_eta_2p75_3p0_bxm1 =  new TH1F("L1EGOR_pt_eta_2p75_3p0_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  L1EGOR_pt_eta_2p25_2p5_denom->Sumw2();
  L1EGOR_pt_eta_2p5_2p75_denom->Sumw2();
  L1EGOR_pt_eta_2p75_3p0_denom->Sumw2();
  L1EGOR_pt_eta_2p25_2p5_bxm1->Sumw2();
  L1EGOR_pt_eta_2p5_2p75_bxm1->Sumw2();
  L1EGOR_pt_eta_2p75_3p0_bxm1->Sumw2();

  /// ETM
  TH1F* ETM_pt_eta_2p25_2p5_denom =  new TH1F("ETM_pt_eta_2p25_2p5_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* ETM_pt_eta_2p5_2p75_denom =  new TH1F("ETM_pt_eta_2p5_2p75_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* ETM_pt_eta_2p75_3p0_denom =  new TH1F("ETM_pt_eta_2p75_3p0_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* ETM_pt_eta_2p25_2p5_bxm1 =  new TH1F("ETM_pt_eta_2p25_2p5_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* ETM_pt_eta_2p5_2p75_bxm1 =  new TH1F("ETM_pt_eta_2p5_2p75_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* ETM_pt_eta_2p75_3p0_bxm1 =  new TH1F("ETM_pt_eta_2p75_3p0_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  ETM_pt_eta_2p25_2p5_denom->Sumw2();
  ETM_pt_eta_2p5_2p75_denom->Sumw2();
  ETM_pt_eta_2p75_3p0_denom->Sumw2();
  ETM_pt_eta_2p25_2p5_bxm1->Sumw2();
  ETM_pt_eta_2p5_2p75_bxm1->Sumw2();
  ETM_pt_eta_2p75_3p0_bxm1->Sumw2();

  /// EGETM
  TH1F* EGETM_pt_eta_2p25_2p5_denom =  new TH1F("EGETM_pt_eta_2p25_2p5_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* EGETM_pt_eta_2p5_2p75_denom =  new TH1F("EGETM_pt_eta_2p5_2p75_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* EGETM_pt_eta_2p75_3p0_denom =  new TH1F("EGETM_pt_eta_2p75_3p0_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* EGETM_pt_eta_2p25_2p5_bxm1 =  new TH1F("EGETM_pt_eta_2p25_2p5_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* EGETM_pt_eta_2p5_2p75_bxm1 =  new TH1F("EGETM_pt_eta_2p5_2p75_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* EGETM_pt_eta_2p75_3p0_bxm1 =  new TH1F("EGETM_pt_eta_2p75_3p0_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  EGETM_pt_eta_2p25_2p5_denom->Sumw2();
  EGETM_pt_eta_2p5_2p75_denom->Sumw2();
  EGETM_pt_eta_2p75_3p0_denom->Sumw2();
  EGETM_pt_eta_2p25_2p5_bxm1->Sumw2();
  EGETM_pt_eta_2p5_2p75_bxm1->Sumw2();
  EGETM_pt_eta_2p75_3p0_bxm1->Sumw2();

  /// UGT
  TH1F* UGT_pt_eta_2p25_2p5_denom =  new TH1F("UGT_pt_eta_2p25_2p5_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* UGT_pt_eta_2p5_2p75_denom =  new TH1F("UGT_pt_eta_2p5_2p75_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* UGT_pt_eta_2p75_3p0_denom =  new TH1F("UGT_pt_eta_2p75_3p0_denom","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* UGT_pt_eta_2p25_2p5_bxm1 =  new TH1F("UGT_pt_eta_2p25_2p5_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* UGT_pt_eta_2p5_2p75_bxm1 =  new TH1F("UGT_pt_eta_2p5_2p75_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  TH1F* UGT_pt_eta_2p75_3p0_bxm1 =  new TH1F("UGT_pt_eta_2p75_3p0_bxm1","",pt_binning.size()-1,&pt_binning[0]);
  UGT_pt_eta_2p25_2p5_denom->Sumw2();
  UGT_pt_eta_2p5_2p75_denom->Sumw2();
  UGT_pt_eta_2p75_3p0_denom->Sumw2();
  UGT_pt_eta_2p25_2p5_bxm1->Sumw2();
  UGT_pt_eta_2p5_2p75_bxm1->Sumw2();
  UGT_pt_eta_2p75_3p0_bxm1->Sumw2();

  TH1F* UGT_ptem_eta_2p25_2p5_denom =  new TH1F("UGT_ptem_eta_2p25_2p5_denom","",ptem_binning.size()-1,&ptem_binning[0]);
  TH1F* UGT_ptem_eta_2p5_2p75_denom =  new TH1F("UGT_ptem_eta_2p5_2p75_denom","",ptem_binning.size()-1,&ptem_binning[0]);
  TH1F* UGT_ptem_eta_2p75_3p0_denom =  new TH1F("UGT_ptem_eta_2p75_3p0_denom","",ptem_binning.size()-1,&ptem_binning[0]);
  TH1F* UGT_ptem_eta_2p25_2p5_bxm1 =  new TH1F("UGT_ptem_eta_2p25_2p5_bxm1","",ptem_binning.size()-1,&ptem_binning[0]);
  TH1F* UGT_ptem_eta_2p5_2p75_bxm1 =  new TH1F("UGT_ptem_eta_2p5_2p75_bxm1","",ptem_binning.size()-1,&ptem_binning[0]);
  TH1F* UGT_ptem_eta_2p75_3p0_bxm1 =  new TH1F("UGT_ptem_eta_2p75_3p0_bxm1","",ptem_binning.size()-1,&ptem_binning[0]);
  UGT_ptem_eta_2p25_2p5_denom->Sumw2();
  UGT_ptem_eta_2p5_2p75_denom->Sumw2();
  UGT_ptem_eta_2p75_3p0_denom->Sumw2();
  UGT_ptem_eta_2p25_2p5_bxm1->Sumw2();
  UGT_ptem_eta_2p5_2p75_bxm1->Sumw2();
  UGT_ptem_eta_2p75_3p0_bxm1->Sumw2();

  TH1F* jet_emfraction_2p25_2p5 = new TH1F("jet_emfraction_2p25_2p5","jet_emfraction_2p25_2p5",100,0,1);
  TH1F* jet_emfraction_2p5_2p75 = new TH1F("jet_emfraction_2p5_2p75","jet_emfraction_2p5_2p75",100,0,1);
  TH1F* jet_emfraction_2p75_3p0 = new TH1F("jet_emfraction_2p75_3p0","jet_emfraction_2p75_3p0",100,0,1);
  
  long int events_with_metfiters = 0;
  long int events_with_calomet = 0;
  long int events_with_loose_muons = 0;
  long int events_with_loose_muons_ptcut = 0 ;
  long int events_with_tight_muons_ptcut = 0 ;
  long int events_passing_muon_veto = 0 ;
  long int events_passing_ele_veto = 0 ;
  long int events_passing_photon_veto = 0 ;
  long int events_passing_jet_veto = 0 ;
  long int events_passing_vbfcuts = 0 ;
  long int events_passing_minDphiCut = 0;

  while (reader.Next()){

    // un-prescaled thresholds in 2016
    float IsoEG_threshold = 30;
    float EG_threshold    = 40;
    float ETM_threshold   = 70;

    if(not *flaghbhenoise) continue;
    if(not *flaghbheiso) continue;
    if(not *flageebadsc) continue;
    if(not *flaggoodvertices) continue;
    if(not *flagglobaltighthalo) continue;
    if(not *flagbadchpf) continue;
    if(not *flagbadpfmu) continue;

    events_with_metfiters++;
    
    if(fabs(*met-*calomet)/(*met) > 0.5) continue;    
    events_with_calomet++;

    if(region == analysisRegion::SingleMuon){
      if(applyMuonCuts and muon_p4->size() == 0) continue; // ask for at least one loose muon
      events_with_loose_muons++;
      if(applyMuonCuts and muon_p4->at(0).Pt() < 10.) continue; // leading muon must have pt > 26 GeV
      events_with_loose_muons_ptcut++;
      bool tight_muon = false;
      for(auto mu : *muon_tight){
      	if(mu == 1) tight_muon = true;
      }
      if(applyMuonCuts and not tight_muon) continue;
      events_with_tight_muons_ptcut++;
      if(electron_p4->size() != 0) continue; // loose electron veto
      events_passing_ele_veto++;
      if(photon_p4->size() != 0) continue; // loose photon veto
      events_passing_photon_veto++;
    }
    else if(region == analysisRegion::JetHT){
      if(muon_p4->size() != 0) continue; // veto muons
      events_passing_muon_veto++;
      if(electron_p4->size() != 0) continue; // loose electron veto
      events_passing_ele_veto++;
      if(photon_p4->size() != 0) continue; // loose photon veto
      events_passing_photon_veto++;
    }
    else if(region == analysisRegion::MET){
      if(muon_p4->size() != 0) continue;
      events_passing_muon_veto++;
      if(electron_p4->size() != 0) continue; // loose electron veto
      events_passing_ele_veto++;
      if(photon_p4->size() != 0) continue; // loose photon veto
      events_passing_photon_veto++;
    }

    // ask at least one jet
    if(jet_p4->size() ==0) continue;
    events_passing_jet_veto++;
    
    // in case one want to apply VBF cuts
    if(applyVBFcuts){
      if(jet_p4->size() < 2) continue;
      if(jet_p4->at(0).Pt() < 70) continue;
      if(jet_p4->at(1).Pt() < 40) continue;
      if(jet_p4->at(0).Eta()*jet_p4->at(1).Eta() > 0) continue;
      if(fabs(jet_p4->at(0).Eta()-jet_p4->at(1).Eta()) < 1) continue;
      if((jet_p4->at(0)+jet_p4->at(1)).M() < 200) continue;
    }
    events_passing_vbfcuts++;

    if(applyMinDphiCut){
      float minDphi = 1000;
      for(size_t ijet = 0; ijet < jet_p4->size(); ijet++){
	float deltaphi = fabs(jet_p4->at(ijet).Phi()-*metphi);
	if(deltaphi > TMath::Pi()) deltaphi = 2*TMath::Pi()-deltaphi;
	if(deltaphi < minDphi)
	  minDphi = deltaphi;
      }
      if(minDphi < 0.5) continue;
    }
    events_passing_minDphiCut++;

    // check the threshold for the EG seeds in this event --> if fired by a lower threshold EG seed compared to what expected --> change threshold
    for(size_t iseed = 0; iseed < L1EGSeeds_bx->size(); iseed++){
      if(L1EGSeeds_bx->at(iseed) != -1) continue;
      TString name (L1EGSeeds_name->at(iseed));
      if(atof(name.ReplaceAll("L1_SingleEG","").Data()) < EG_threshold)
	EG_threshold = atof(name.ReplaceAll("L1_SingleEG","").Data());
    }

    // check the threshold for the IsoEG seeds in this event --> if fired by a lower threshold EG seed compared to what expected --> change threshold
    for(size_t iseed = 0; iseed < L1IsoEGSeeds_bx->size(); iseed++){
      if(L1IsoEGSeeds_bx->at(iseed) != -1) continue;
      TString name (L1IsoEGSeeds_name->at(iseed));
      if(atof(name.ReplaceAll("L1_SingleIsoEG","")) < IsoEG_threshold)
	IsoEG_threshold = atof(name.ReplaceAll("L1_SingleIsoEG","").Data());
    }

    // fill the efficiency maps for L1EG triggers
    for(size_t ijet = 0; ijet < jet_p4->size(); ijet++){

      if(filterHotTowers and fabs(jet_p4->at(ijet).Eta()+2.82) < 0.2 and fabs(deltaPhi(jet_p4->at(ijet).Phi(),2.0724)) < 0.2) continue; 

      // jet should not be matched with a L1Mu
      bool match_l1_mu_bx0 = false;
      for(size_t iMuCand = 0; iMuCand < L1Mu_p4->size(); iMuCand++){
	if(L1Mu_p4->at(iMuCand).Pt() > 22 and
	   DeltaR(L1Mu_p4->at(iMuCand),jet_p4->at(ijet)) < 0.4 and
	   L1Mu_bx->at(iMuCand) == 0)
	  match_l1_mu_bx0 = true;
      }
      if(match_l1_mu_bx0) continue;
      
      // loop on the EG candidates related to that BX
      int match_egiso_bx = 99;
      int match_eg_bx    = 99;
      for(size_t iEGCand = 0; iEGCand < L1EG_p4->size(); iEGCand++){
	if((L1EG_iso->at(iEGCand) & 1) == 1 and
	   L1EG_p4->at(iEGCand).Pt() > IsoEG_threshold and 
	   DeltaR(L1EG_p4->at(iEGCand),jet_p4->at(ijet)) < 0.4){	  
	  if(L1EG_bx->at(iEGCand) < match_egiso_bx){ // one should take the minimum since will be the one pre-firing
	    match_egiso_bx = L1EG_bx->at(iEGCand);
	  }
	}
	if(L1EG_p4->at(iEGCand).Pt() > EG_threshold and 
	   DeltaR(L1EG_p4->at(iEGCand),jet_p4->at(ijet)) < 0.4){	  
	  if(L1EG_bx->at(iEGCand) < match_eg_bx){ // one should take the minimum since will be the one pre-firing
	    match_eg_bx = L1EG_bx->at(iEGCand);
	  }
	}
      }
      
      // Denominators
      denL1IsoEG_pt_eta->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
      denL1EG_pt_eta->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
      denL1EGOR_pt_eta->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));

      if(jet_p4->at(ijet).Pt() > jetpt_threshold){
	denL1IsoEG_eta_phi->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	denL1EG_eta_phi->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	denL1EGOR_eta_phi->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
      }
      
      if(fabs(jet_p4->at(ijet).Eta()) >= 2.25 and fabs(jet_p4->at(ijet).Eta()) < 2.5){
	if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	  jet_emfraction_2p25_2p5->Fill(jet_neutralEmFrac->at(ijet));	
	L1IsoEG_pt_eta_2p25_2p5_denom->Fill(jet_p4->at(ijet).Pt());
	L1EG_pt_eta_2p25_2p5_denom->Fill(jet_p4->at(ijet).Pt());
	L1EGOR_pt_eta_2p25_2p5_denom->Fill(jet_p4->at(ijet).Pt());
      }
      else if(fabs(jet_p4->at(ijet).Eta()) >= 2.5 and fabs(jet_p4->at(ijet).Eta()) < 2.75){
	if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	  jet_emfraction_2p5_2p75->Fill(jet_neutralEmFrac->at(ijet));	
	L1IsoEG_pt_eta_2p5_2p75_denom->Fill(jet_p4->at(ijet).Pt());
	L1EG_pt_eta_2p5_2p75_denom->Fill(jet_p4->at(ijet).Pt());
	L1EGOR_pt_eta_2p5_2p75_denom->Fill(jet_p4->at(ijet).Pt());
      }
      else if(fabs(jet_p4->at(ijet).Eta()) >= 2.75 and fabs(jet_p4->at(ijet).Eta()) < 3.0){
	if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	  jet_emfraction_2p75_3p0->Fill(jet_neutralEmFrac->at(ijet));	
	L1IsoEG_pt_eta_2p75_3p0_denom->Fill(jet_p4->at(ijet).Pt());
	L1EG_pt_eta_2p75_3p0_denom->Fill(jet_p4->at(ijet).Pt());
	L1EGOR_pt_eta_2p75_3p0_denom->Fill(jet_p4->at(ijet).Pt());
      }

      // IsoEG decision	      
      if(match_egiso_bx != 99){ 
	if( match_egiso_bx == -2){
	  L1IsoEG_pt_eta_bxm2->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1IsoEG_eta_phi_bxm2->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	}
	if( match_egiso_bx == -1){
	  L1IsoEG_pt_eta_bxm1->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1IsoEG_eta_phi_bxm1->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	  
	  if(fabs(jet_p4->at(ijet).Eta()) >= 2.25 and fabs(jet_p4->at(ijet).Eta()) < 2.5){
	    L1IsoEG_pt_eta_2p25_2p5_bxm1->Fill(jet_p4->at(ijet).Pt());
	  }
	  else if(fabs(jet_p4->at(ijet).Eta()) >= 2.5 and fabs(jet_p4->at(ijet).Eta()) < 2.75){
	    L1IsoEG_pt_eta_2p5_2p75_bxm1->Fill(jet_p4->at(ijet).Pt());
	  }
	  else if(fabs(jet_p4->at(ijet).Eta()) >= 2.75 and fabs(jet_p4->at(ijet).Eta()) < 3.0){
	    L1IsoEG_pt_eta_2p75_3p0_bxm1->Fill(jet_p4->at(ijet).Pt());
	  }
	}
	if( match_egiso_bx == 0){
	  L1IsoEG_pt_eta_bx0->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1IsoEG_eta_phi_bx0->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	}
      }
      
      // EG decision
      if(match_eg_bx != 99){ 
	if( match_eg_bx == -2){
	  L1EG_pt_eta_bxm2->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1EG_eta_phi_bxm2->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	}
	if( match_eg_bx == -1){
	  L1EG_pt_eta_bxm1->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1EG_eta_phi_bxm1->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	  
	  if(fabs(jet_p4->at(ijet).Eta()) >= 2.25 and fabs(jet_p4->at(ijet).Eta()) < 2.5)
	    L1EG_pt_eta_2p25_2p5_bxm1->Fill(jet_p4->at(ijet).Pt());
	  else if(fabs(jet_p4->at(ijet).Eta()) >= 2.5 and fabs(jet_p4->at(ijet).Eta()) < 2.75)
	    L1EG_pt_eta_2p5_2p75_bxm1->Fill(jet_p4->at(ijet).Pt());
	  else if(fabs(jet_p4->at(ijet).Eta()) >= 2.75 and fabs(jet_p4->at(ijet).Eta()) < 3.0)
	    L1EG_pt_eta_2p75_3p0_bxm1->Fill(jet_p4->at(ijet).Pt());
	}
	if( match_eg_bx == 0){
	  L1EG_pt_eta_bx0->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1EG_eta_phi_bx0->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	}
      }


      // IsoEG OR EG decision
      if(match_eg_bx != 99 or match_egiso_bx){ 
	if(match_eg_bx == -2 or match_egiso_bx == -2){
	  L1EGOR_pt_eta_bxm2->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1EGOR_eta_phi_bxm2->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	}
	if(match_eg_bx == -1 or match_egiso_bx == -1){
	  L1EGOR_pt_eta_bxm1->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));	  
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1EGOR_eta_phi_bxm1->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	  
	  if(fabs(jet_p4->at(ijet).Eta()) >= 2.25 and fabs(jet_p4->at(ijet).Eta()) < 2.5)
	    L1EGOR_pt_eta_2p25_2p5_bxm1->Fill(jet_p4->at(ijet).Pt());
	  else if(fabs(jet_p4->at(ijet).Eta()) >= 2.5 and fabs(jet_p4->at(ijet).Eta()) < 2.75)
	    L1EGOR_pt_eta_2p5_2p75_bxm1->Fill(jet_p4->at(ijet).Pt());
	  else if(fabs(jet_p4->at(ijet).Eta()) >= 2.75 and fabs(jet_p4->at(ijet).Eta()) < 3.0)
	    L1EGOR_pt_eta_2p75_3p0_bxm1->Fill(jet_p4->at(ijet).Pt());
	}
	
	if( match_eg_bx == 0 or match_egiso_bx == 0){
	  L1EGOR_pt_eta_bx0->Fill(jet_p4->at(ijet).Pt(),fabs(jet_p4->at(ijet).Eta()));
	  if(jet_p4->at(ijet).Pt() > jetpt_threshold)
	    L1EGOR_eta_phi_bx0->Fill(jet_p4->at(ijet).Eta(),jet_p4->at(ijet).Phi());
	}
      }
    }    
    
    // Study the global trigger decision --> only events with 1-jet in the interesting region
    int njet = 0;
    vector<LorentzVector> jet4V_vec;
    vector<int> jet_pos;
    for(size_t ijet = 0; ijet < jet_p4->size(); ijet++){

      if(filterHotTowers and fabs(jet_p4->at(ijet).Eta()+2.82) < 0.2 and fabs(deltaPhi(jet_p4->at(ijet).Phi(),2.0724)) < 0.2) continue; 
      
      // inside the pre-firing region
      if(fabs(jet_p4->at(ijet).Eta()) >= 2.25 and fabs(jet_p4->at(ijet).Eta()) < 3.0){

	bool match_l1_mu_bx0 = false;
	for(size_t iMuCand = 0; iMuCand < L1Mu_p4->size(); iMuCand++){
	  if(L1Mu_p4->at(iMuCand).Pt() > 22 and
	     DeltaR(L1Mu_p4->at(iMuCand),jet_p4->at(ijet)) < 0.4 and
	     L1Mu_bx->at(iMuCand) == 0)
	    match_l1_mu_bx0 = true;
	}
	if(match_l1_mu_bx0) continue;

	njet++;
	jet4V_vec.push_back(jet_p4->at(ijet));
	jet_pos.push_back(ijet);
      }    
    }
    
    // only one jet that can pre-fire 
    if(njet == 1){      

      // Global trigger
      int match_ug_bx = 99;      
      for(size_t iUG = 0; iUG < L1GtBx->size(); iUG++){
	if(match_ug_bx == 99 and L1GtBx->at(iUG) == 1) match_ug_bx = iUG-2;
      }
      
      // ETM trigger
      int match_ETM_bx = 99;      
      for(size_t iETM = 0; iETM < L1MET_bx->size(); iETM++){
	if(match_ETM_bx == 99 and
           L1MET_p4->at(iETM).Pt() >= ETM_threshold)
          match_ETM_bx = iETM-2;
      }

      for(size_t ijet = 0; ijet < jet4V_vec.size(); ijet++){
	
	// loop on the EG candidates related to that BX
	int match_egiso_bx = 99;
	int match_eg_bx    = 99;
	for(size_t iEGCand = 0; iEGCand < L1EG_p4->size(); iEGCand++){
	  if((L1EG_iso->at(iEGCand) & 1) == 1 and
	     L1EG_p4->at(iEGCand).Pt() > IsoEG_threshold and 
	     DeltaR(L1EG_p4->at(iEGCand),jet4V_vec.at(ijet)) < 0.4){	  
	    if(L1EG_bx->at(iEGCand) < match_egiso_bx){ // one should take the minimum since will be the one pre-firing
	      match_egiso_bx = L1EG_bx->at(iEGCand);
	    }
	  }
	  if(L1EG_p4->at(iEGCand).Pt() > EG_threshold and 
	     DeltaR(L1EG_p4->at(iEGCand),jet4V_vec.at(ijet)) < 0.4){	  
	    if(L1EG_bx->at(iEGCand) < match_eg_bx){ // one should take the minimum since will be the one pre-firing
	      match_eg_bx = L1EG_bx->at(iEGCand);
	    }
	  }
	}
	      
	if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.25 and fabs(jet4V_vec.at(ijet).Eta()) < 2.50){
	  UGT_pt_eta_2p25_2p5_denom->Fill(jet4V_vec.at(ijet).Pt());
	  UGT_ptem_eta_2p25_2p5_denom->Fill(jet4V_vec.at(ijet).Pt()*jet_neutralEmFrac->at(jet_pos.at(ijet)));
	}
	else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.50 and fabs(jet4V_vec.at(ijet).Eta()) < 2.75){
	  UGT_pt_eta_2p5_2p75_denom->Fill(jet4V_vec.at(ijet).Pt());
	  UGT_ptem_eta_2p5_2p75_denom->Fill(jet4V_vec.at(ijet).Pt()*jet_neutralEmFrac->at(jet_pos.at(ijet)));
	}
	else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.75 and fabs(jet4V_vec.at(ijet).Eta()) < 3.00){
	  UGT_pt_eta_2p75_3p0_denom->Fill(jet4V_vec.at(ijet).Pt());      
	  UGT_ptem_eta_2p75_3p0_denom->Fill(jet4V_vec.at(ijet).Pt()*jet_neutralEmFrac->at(jet_pos.at(ijet)));
	}

	if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.25 and fabs(jet4V_vec.at(ijet).Eta()) < 2.50)
	  ETM_pt_eta_2p25_2p5_denom->Fill(jet4V_vec.at(ijet).Pt());
	else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.50 and fabs(jet4V_vec.at(ijet).Eta()) < 2.75)
	  ETM_pt_eta_2p5_2p75_denom->Fill(jet4V_vec.at(ijet).Pt());
	else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.75 and fabs(jet4V_vec.at(ijet).Eta()) < 3.00)
	  ETM_pt_eta_2p75_3p0_denom->Fill(jet4V_vec.at(ijet).Pt());      

	if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.25 and fabs(jet4V_vec.at(ijet).Eta()) < 2.50)
	  EGETM_pt_eta_2p25_2p5_denom->Fill(jet4V_vec.at(ijet).Pt());
	else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.50 and fabs(jet4V_vec.at(ijet).Eta()) < 2.75)
	  EGETM_pt_eta_2p5_2p75_denom->Fill(jet4V_vec.at(ijet).Pt());
	else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.75 and fabs(jet4V_vec.at(ijet).Eta()) < 3.00)
	  EGETM_pt_eta_2p75_3p0_denom->Fill(jet4V_vec.at(ijet).Pt());      

	/// UGT decision
	if(match_ug_bx == -1){
	  if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.25 and fabs(jet4V_vec.at(ijet).Eta()) < 2.5){
	    UGT_pt_eta_2p25_2p5_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	    UGT_ptem_eta_2p25_2p5_bxm1->Fill(jet4V_vec.at(ijet).Pt()*jet_neutralEmFrac->at(jet_pos.at(ijet)));
	  }
	  else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.5 and fabs(jet4V_vec.at(ijet).Eta()) < 2.75){
	    UGT_pt_eta_2p5_2p75_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	    UGT_ptem_eta_2p5_2p75_bxm1->Fill(jet4V_vec.at(ijet).Pt()*jet_neutralEmFrac->at(jet_pos.at(ijet)));
	  }
	  else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.75 and fabs(jet4V_vec.at(ijet).Eta()) < 3.0){
	    UGT_pt_eta_2p75_3p0_bxm1->Fill(jet4V_vec.at(ijet).Pt());	  
	    UGT_ptem_eta_2p75_3p0_bxm1->Fill(jet4V_vec.at(ijet).Pt()*jet_neutralEmFrac->at(jet_pos.at(ijet)));
	  }
	}	

	/// ETM decision
	if(match_ETM_bx == -1){
	  if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.25 and fabs(jet4V_vec.at(ijet).Eta()) < 2.5)
	    ETM_pt_eta_2p25_2p5_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	  else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.5 and fabs(jet4V_vec.at(ijet).Eta()) < 2.75)
	    ETM_pt_eta_2p5_2p75_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	  else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.75 and fabs(jet4V_vec.at(ijet).Eta()) < 3.0)
	    ETM_pt_eta_2p75_3p0_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	}	

	/// ETM decision
	if(match_ETM_bx == -1 or match_egiso_bx == -1 or match_eg_bx == -1){
	  if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.25 and fabs(jet4V_vec.at(ijet).Eta()) < 2.5)
	    EGETM_pt_eta_2p25_2p5_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	  else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.5 and fabs(jet4V_vec.at(ijet).Eta()) < 2.75)
	    EGETM_pt_eta_2p5_2p75_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	  else if(fabs(jet4V_vec.at(ijet).Eta()) >= 2.75 and fabs(jet4V_vec.at(ijet).Eta()) < 3.0)
	    EGETM_pt_eta_2p75_3p0_bxm1->Fill(jet4V_vec.at(ijet).Pt());
	}	

      }
    }    
  }

  cout<<"Total events "<<totalEvents<<endl;
  cout<<"Passing MET filters "<<events_with_metfiters<<" "<<float(events_with_metfiters)/totalEvents<<endl;
  cout<<"Passing Calo MET "<<events_with_calomet<<" "<<float(events_with_calomet)/totalEvents<<endl;

  if(region == analysisRegion::SingleMuon){
    cout<<"Loose muon tag "<<events_with_loose_muons<<" "<<float(events_with_loose_muons)/totalEvents<<endl;
    cout<<"Muon pt > 26 "<<events_with_loose_muons_ptcut<<" "<<float(events_with_loose_muons_ptcut)/totalEvents<<endl;
    cout<<"Tight muon tag "<<events_with_tight_muons_ptcut<<" "<<float(events_with_tight_muons_ptcut)/totalEvents<<endl;
  }
  else{
    cout<<"Passing muon veto "<<events_passing_muon_veto<<" "<<float(events_passing_muon_veto)/totalEvents<<endl;
  }

  cout<<"Passing electron veto "<<events_passing_ele_veto<<" "<<float(events_passing_ele_veto)/totalEvents<<endl;
  cout<<"Passing photon veto "<<events_passing_photon_veto<<" "<<float(events_passing_photon_veto)/totalEvents<<endl;
  cout<<"Passing jet veto "<<events_passing_jet_veto<<" "<<float(events_passing_jet_veto)/totalEvents<<endl;
  cout<<"Passing vbf cuts "<<events_passing_vbfcuts<<" "<<float(events_passing_vbfcuts)/totalEvents<<endl;
  cout<<"Passing minDphi cut "<<events_passing_minDphiCut<<" "<<float(events_passing_minDphiCut)/totalEvents<<endl;
  
  TH2F* eff_L1IsoEG_pt_eta_bx0 = (TH2F*) L1IsoEG_pt_eta_bx0->Clone("eff_L1IsoEG_pt_eta_bx0");
  eff_L1IsoEG_pt_eta_bx0->Divide(denL1IsoEG_pt_eta);
  TH2F* eff_L1IsoEG_pt_eta_bxm1 = (TH2F*) L1IsoEG_pt_eta_bxm1->Clone("eff_L1IsoEG_pt_eta_bxm1");
  eff_L1IsoEG_pt_eta_bxm1->Divide(denL1IsoEG_pt_eta);
  TH2F* eff_L1IsoEG_pt_eta_bxm2 = (TH2F*) L1IsoEG_pt_eta_bxm2->Clone("eff_L1IsoEG_pt_eta_bxm2");
  eff_L1IsoEG_pt_eta_bxm2->Divide(denL1IsoEG_pt_eta);

  TH2F* eff_L1IsoEG_eta_phi_bx0 = (TH2F*) L1IsoEG_eta_phi_bx0->Clone("eff_L1IsoEG_eta_phi_bx0");
  eff_L1IsoEG_eta_phi_bx0->Divide(denL1IsoEG_eta_phi);
  TH2F* eff_L1IsoEG_eta_phi_bxm1 = (TH2F*) L1IsoEG_eta_phi_bxm1->Clone("eff_L1IsoEG_eta_phi_bxm1");
  eff_L1IsoEG_eta_phi_bxm1->Divide(denL1IsoEG_eta_phi);
  TH2F* eff_L1IsoEG_eta_phi_bxm2 = (TH2F*) L1IsoEG_eta_phi_bxm2->Clone("eff_L1IsoEG_eta_phi_bxm2");
  eff_L1IsoEG_eta_phi_bxm2->Divide(denL1IsoEG_eta_phi);

  TH2F* eff_L1EG_pt_eta_bx0 = (TH2F*) L1EG_pt_eta_bx0->Clone("eff_L1EG_pt_eta_bx0");
  eff_L1EG_pt_eta_bx0->Divide(denL1EG_pt_eta);
  TH2F* eff_L1EG_pt_eta_bxm1 = (TH2F*) L1EG_pt_eta_bxm1->Clone("eff_L1EG_pt_eta_bxm1");
  eff_L1EG_pt_eta_bxm1->Divide(denL1EG_pt_eta);
  TH2F* eff_L1EG_pt_eta_bxm2 = (TH2F*) L1EG_pt_eta_bxm2->Clone("eff_L1EG_pt_eta_bxm2");
  eff_L1EG_pt_eta_bxm2->Divide(denL1EG_pt_eta);

  TH2F* eff_L1EG_eta_phi_bx0 = (TH2F*) L1EG_eta_phi_bx0->Clone("eff_L1EG_eta_phi_bx0");
  eff_L1EG_eta_phi_bx0->Divide(denL1EG_eta_phi);
  TH2F* eff_L1EG_eta_phi_bxm1 = (TH2F*) L1EG_eta_phi_bxm1->Clone("eff_L1EG_eta_phi_bxm1");
  eff_L1EG_eta_phi_bxm1->Divide(denL1EG_eta_phi);
  TH2F* eff_L1EG_eta_phi_bxm2 = (TH2F*) L1EG_eta_phi_bxm2->Clone("eff_L1EG_eta_phi_bxm2");
  eff_L1EG_eta_phi_bxm2->Divide(denL1EG_eta_phi);

  TH2F* eff_L1EGOR_pt_eta_bx0 = (TH2F*) L1EGOR_pt_eta_bx0->Clone("eff_L1EGOR_pt_eta_bx0");
  eff_L1EGOR_pt_eta_bx0->Divide(denL1EGOR_pt_eta);
  TH2F* eff_L1EGOR_pt_eta_bxm1 = (TH2F*) L1EGOR_pt_eta_bxm1->Clone("eff_L1EGOR_pt_eta_bxm1");
  eff_L1EGOR_pt_eta_bxm1->Divide(denL1EGOR_pt_eta);
  TH2F* eff_L1EGOR_pt_eta_bxm2 = (TH2F*) L1EGOR_pt_eta_bxm2->Clone("eff_L1EGOR_pt_eta_bxm2");
  eff_L1EGOR_pt_eta_bxm2->Divide(denL1EGOR_pt_eta);

  TH2F* eff_L1EGOR_eta_phi_bx0 = (TH2F*) L1EGOR_eta_phi_bx0->Clone("eff_L1EGOR_eta_phi_bx0");
  eff_L1EGOR_eta_phi_bx0->Divide(denL1EGOR_eta_phi);
  TH2F* eff_L1EGOR_eta_phi_bxm1 = (TH2F*) L1EGOR_eta_phi_bxm1->Clone("eff_L1EGOR_eta_phi_bxm1");
  eff_L1EGOR_eta_phi_bxm1->Divide(denL1EGOR_eta_phi);
  TH2F* eff_L1EGOR_eta_phi_bxm2 = (TH2F*) L1EGOR_eta_phi_bxm2->Clone("eff_L1EGOR_eta_phi_bxm2");
  eff_L1EGOR_eta_phi_bxm2->Divide(denL1EGOR_eta_phi);

  TCanvas* canvas = new TCanvas("canvas","canvas",650,600);
  canvas->cd();
  
  plotEfficiencyMap(canvas,eff_L1IsoEG_pt_eta_bxm2,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1IsoEG eff (bx = -2)","eff_L1IsoEG_pt_eta_bxm2",outputDIR);
  plotEfficiencyMap(canvas,eff_L1IsoEG_eta_phi_bxm2,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1IsoEG eff (bx = -2)","eff_L1IsoEG_eta_phi_bxm2",outputDIR);
  plotEfficiencyMap(canvas,eff_L1IsoEG_pt_eta_bxm1,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1IsoEG eff (bx = -1)","eff_L1IsoEG_pt_eta_bxm1",outputDIR);
  plotEfficiencyMap(canvas,eff_L1IsoEG_eta_phi_bxm1,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1IsoEG eff (bx = -1)","eff_L1IsoEG_eta_phi_bxm1",outputDIR);
  plotEfficiencyMap(canvas,eff_L1IsoEG_pt_eta_bx0,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1IsoEG eff (bx = 0)","eff_L1IsoEG_pt_eta_bx0",outputDIR);
  plotEfficiencyMap(canvas,eff_L1IsoEG_eta_phi_bx0,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1IsoEG eff (bx = 0)","eff_L1IsoEG_eta_phi_bx0",outputDIR);

  plotEfficiencyMap(canvas,eff_L1EG_pt_eta_bxm2,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1EG eff (bx = -2)","eff_L1EG_pt_eta_bxm2",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EG_eta_phi_bxm2,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1EG eff (bx = -2)","eff_L1EG_eta_phi_bxm2",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EG_pt_eta_bxm1,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1EG eff (bx = -1)","eff_L1EG_pt_eta_bxm1",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EG_eta_phi_bxm1,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1EG eff (bx = -1)","eff_L1EG_eta_phi_bxm1",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EG_pt_eta_bx0,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1EG eff (bx = 0)","eff_L1EG_pt_eta_bx0",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EG_eta_phi_bx0,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1EG eff (bx = 0)","eff_L1EG_eta_phi_bx0",outputDIR);

  plotEfficiencyMap(canvas,eff_L1EGOR_pt_eta_bxm2,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1EG OR L1IsoEG eff (bx = -2)","eff_L1EGOR_pt_eta_bxm2",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EGOR_eta_phi_bxm2,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1EG OR L1IsoEG eff (bx = -2)","eff_L1EGOR_eta_phi_bxm2",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EGOR_pt_eta_bxm1,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1EG OR L1IsoEG eff (bx = -1)","eff_L1EGOR_pt_eta_bxm1",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EGOR_eta_phi_bxm1,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1EG OR L1IsoEG eff (bx = -1)","eff_L1EGOR_eta_phi_bxm1",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EGOR_pt_eta_bx0,"p_{T}^{j,reco} [GeV]","|#eta_{j,reco}|","L1EG OR L1IsoEG eff (bx = 0)","eff_L1EGOR_pt_eta_bx0",outputDIR);
  plotEfficiencyMap(canvas,eff_L1EGOR_eta_phi_bx0,"#eta_{j,reco} [GeV]","#phi_{j,reco}","L1EG OR L1IsoEG eff (bx = 0)","eff_L1EGOR_eta_phi_bx0",outputDIR);
  
  canvas->SetLogx(1);
  canvas->SetRightMargin(0.05);

  TGraphAsymmErrors* efficiency_IsoEG_pt_eta_2p25_2p5 = new TGraphAsymmErrors(L1IsoEG_pt_eta_2p25_2p5_bxm1,L1IsoEG_pt_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_IsoEG_pt_eta_2p5_2p75 = new TGraphAsymmErrors(L1IsoEG_pt_eta_2p5_2p75_bxm1,L1IsoEG_pt_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_IsoEG_pt_eta_2p75_3p0 = new TGraphAsymmErrors(L1IsoEG_pt_eta_2p75_3p0_bxm1,L1IsoEG_pt_eta_2p75_3p0_denom);

  efficiency_IsoEG_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_IsoEG_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_IsoEG_pt_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_IsoEG_pt_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_IsoEG_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_IsoEG_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_IsoEG_pt_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_IsoEG_pt_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_IsoEG_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_IsoEG_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_IsoEG_pt_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_IsoEG_pt_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_IsoEG_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_IsoEG_pt_eta_2p25_2p5->GetYaxis()->SetTitle("L1IsoEG efficiency (bx=-1)");
  efficiency_IsoEG_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_IsoEG_pt_eta_2p25_2p5->Draw("AEP");
  efficiency_IsoEG_pt_eta_2p5_2p75->Draw("EPsame");
  efficiency_IsoEG_pt_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  TLegend leg (0.2,0.55,0.55,0.86);
  leg.SetFillColor(0);
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);
  leg.AddEntry(efficiency_IsoEG_pt_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_IsoEG_pt_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_IsoEG_pt_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_IsoEG_vs_pt.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_IsoEG_vs_pt.pdf").c_str(),"pdf");
  canvas->SetLogy(0);
  
  ////
  TGraphAsymmErrors* efficiency_EG_pt_eta_2p25_2p5 = new TGraphAsymmErrors(L1EG_pt_eta_2p25_2p5_bxm1,L1EG_pt_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_EG_pt_eta_2p5_2p75 = new TGraphAsymmErrors(L1EG_pt_eta_2p5_2p75_bxm1,L1EG_pt_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_EG_pt_eta_2p75_3p0 = new TGraphAsymmErrors(L1EG_pt_eta_2p75_3p0_bxm1,L1EG_pt_eta_2p75_3p0_denom);

  efficiency_EG_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_EG_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_EG_pt_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_EG_pt_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_EG_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_EG_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_EG_pt_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_EG_pt_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_EG_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_EG_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_EG_pt_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_EG_pt_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_EG_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_EG_pt_eta_2p25_2p5->GetYaxis()->SetTitle("L1_EG (bx=-1)");
  efficiency_EG_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_EG_pt_eta_2p25_2p5->Draw("AEP");
  efficiency_EG_pt_eta_2p5_2p75->Draw("EPsame");
  efficiency_EG_pt_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  leg.Clear();
  leg.AddEntry(efficiency_EG_pt_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_EG_pt_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_EG_pt_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_pt.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_pt.pdf").c_str(),"pdf");

  ////
  TGraphAsymmErrors* efficiency_EGOR_pt_eta_2p25_2p5 = new TGraphAsymmErrors(L1EGOR_pt_eta_2p25_2p5_bxm1,L1EGOR_pt_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_EGOR_pt_eta_2p5_2p75 = new TGraphAsymmErrors(L1EGOR_pt_eta_2p5_2p75_bxm1,L1EGOR_pt_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_EGOR_pt_eta_2p75_3p0 = new TGraphAsymmErrors(L1EGOR_pt_eta_2p75_3p0_bxm1,L1EGOR_pt_eta_2p75_3p0_denom);

  efficiency_EGOR_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_EGOR_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_EGOR_pt_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_EGOR_pt_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_EGOR_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_EGOR_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_EGOR_pt_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_EGOR_pt_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_EGOR_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_EGOR_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_EGOR_pt_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_EGOR_pt_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_EGOR_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_EGOR_pt_eta_2p25_2p5->GetYaxis()->SetTitle("L1_EG OR L1_IsoEG (bx=-1)");
  efficiency_EGOR_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_EGOR_pt_eta_2p25_2p5->Draw("AEP");
  efficiency_EGOR_pt_eta_2p5_2p75->Draw("EPsame");
  efficiency_EGOR_pt_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  leg.Clear();
  leg.AddEntry(efficiency_EGOR_pt_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_EGOR_pt_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_EGOR_pt_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EGOR_vs_pt.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EGOR_vs_pt.pdf").c_str(),"pdf");


  //////
  TGraphAsymmErrors* efficiency_UGT_pt_eta_2p25_2p5 = new TGraphAsymmErrors(UGT_pt_eta_2p25_2p5_bxm1,UGT_pt_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_UGT_pt_eta_2p5_2p75 = new TGraphAsymmErrors(UGT_pt_eta_2p5_2p75_bxm1,UGT_pt_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_UGT_pt_eta_2p75_3p0 = new TGraphAsymmErrors(UGT_pt_eta_2p75_3p0_bxm1,UGT_pt_eta_2p75_3p0_denom);

  efficiency_UGT_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_UGT_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_UGT_pt_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_UGT_pt_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_UGT_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_UGT_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_UGT_pt_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_UGT_pt_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_UGT_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_UGT_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_UGT_pt_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_UGT_pt_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_UGT_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_UGT_pt_eta_2p25_2p5->GetYaxis()->SetTitle("UGT efficiency (bx=-1)");
  efficiency_UGT_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_UGT_pt_eta_2p25_2p5->Draw("AEP");
  efficiency_UGT_pt_eta_2p5_2p75->Draw("EPsame");
  efficiency_UGT_pt_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  leg.Clear();
  leg.AddEntry(efficiency_UGT_pt_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_UGT_pt_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_UGT_pt_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_UGT_vs_pt.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_UGT_vs_pt.pdf").c_str(),"pdf");

  //////
  TGraphAsymmErrors* efficiency_UGT_ptem_eta_2p25_2p5 = new TGraphAsymmErrors(UGT_ptem_eta_2p25_2p5_bxm1,UGT_ptem_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_UGT_ptem_eta_2p5_2p75 = new TGraphAsymmErrors(UGT_ptem_eta_2p5_2p75_bxm1,UGT_ptem_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_UGT_ptem_eta_2p75_3p0 = new TGraphAsymmErrors(UGT_ptem_eta_2p75_3p0_bxm1,UGT_ptem_eta_2p75_3p0_denom);

  efficiency_UGT_ptem_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_UGT_ptem_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_UGT_ptem_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_UGT_ptem_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_UGT_ptem_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_UGT_ptem_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_UGT_ptem_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_UGT_ptem_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_UGT_ptem_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_UGT_ptem_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_UGT_ptem_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_UGT_ptem_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_UGT_ptem_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{EM} [GeV]");
  efficiency_UGT_ptem_eta_2p25_2p5->GetYaxis()->SetTitle("UGT efficiency (bx=-1)");
  efficiency_UGT_ptem_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_UGT_ptem_eta_2p25_2p5->Draw("AEP");
  efficiency_UGT_ptem_eta_2p5_2p75->Draw("EPsame");
  efficiency_UGT_ptem_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  leg.Clear();
  leg.AddEntry(efficiency_UGT_ptem_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_UGT_ptem_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_UGT_ptem_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_UGT_vs_ptem.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_UGT_vs_ptem.pdf").c_str(),"pdf");


  //////
  TGraphAsymmErrors* efficiency_ETM_pt_eta_2p25_2p5 = new TGraphAsymmErrors(ETM_pt_eta_2p25_2p5_bxm1,ETM_pt_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_ETM_pt_eta_2p5_2p75 = new TGraphAsymmErrors(ETM_pt_eta_2p5_2p75_bxm1,ETM_pt_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_ETM_pt_eta_2p75_3p0 = new TGraphAsymmErrors(ETM_pt_eta_2p75_3p0_bxm1,ETM_pt_eta_2p75_3p0_denom);

  efficiency_ETM_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_ETM_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_ETM_pt_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_ETM_pt_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_ETM_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_ETM_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_ETM_pt_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_ETM_pt_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_ETM_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_ETM_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_ETM_pt_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_ETM_pt_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_ETM_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_ETM_pt_eta_2p25_2p5->GetYaxis()->SetTitle("ETM efficiency (bx=-1)");
  efficiency_ETM_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_ETM_pt_eta_2p25_2p5->Draw("AEP");
  efficiency_ETM_pt_eta_2p5_2p75->Draw("EPsame");
  efficiency_ETM_pt_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  leg.Clear();
  leg.AddEntry(efficiency_ETM_pt_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_ETM_pt_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_ETM_pt_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_ETM_vs_pt.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_ETM_vs_pt.pdf").c_str(),"pdf");

  //////
  TGraphAsymmErrors* efficiency_EGETM_pt_eta_2p25_2p5 = new TGraphAsymmErrors(EGETM_pt_eta_2p25_2p5_bxm1,EGETM_pt_eta_2p25_2p5_denom);
  TGraphAsymmErrors* efficiency_EGETM_pt_eta_2p5_2p75 = new TGraphAsymmErrors(EGETM_pt_eta_2p5_2p75_bxm1,EGETM_pt_eta_2p5_2p75_denom);
  TGraphAsymmErrors* efficiency_EGETM_pt_eta_2p75_3p0 = new TGraphAsymmErrors(EGETM_pt_eta_2p75_3p0_bxm1,EGETM_pt_eta_2p75_3p0_denom);

  efficiency_EGETM_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_EGETM_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_EGETM_pt_eta_2p25_2p5->SetMarkerSize(1.2);
  efficiency_EGETM_pt_eta_2p25_2p5->SetMarkerStyle(20);

  efficiency_EGETM_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_EGETM_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_EGETM_pt_eta_2p5_2p75->SetMarkerSize(1.2);
  efficiency_EGETM_pt_eta_2p5_2p75->SetMarkerStyle(20);

  efficiency_EGETM_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_EGETM_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_EGETM_pt_eta_2p75_3p0->SetMarkerSize(1.2);
  efficiency_EGETM_pt_eta_2p75_3p0->SetMarkerStyle(20);

  efficiency_EGETM_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_EGETM_pt_eta_2p25_2p5->GetYaxis()->SetTitle("ETM OR EG efficiency (bx=-1)");
  efficiency_EGETM_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_EGETM_pt_eta_2p25_2p5->Draw("AEP");
  efficiency_EGETM_pt_eta_2p5_2p75->Draw("EPsame");
  efficiency_EGETM_pt_eta_2p75_3p0->Draw("EPsame");
  CMS_lumi(canvas,"");

  leg.Clear();
  leg.AddEntry(efficiency_EGETM_pt_eta_2p25_2p5,"2.25 < |#eta| < 2.50","PL");
  leg.AddEntry(efficiency_EGETM_pt_eta_2p5_2p75,"2.50 < |#eta| < 2.75","PL");
  leg.AddEntry(efficiency_EGETM_pt_eta_2p75_3p0,"2.75 < |#eta| < 3.00","PL");
  leg.Draw("same");

  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EGETM_vs_pt.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EGETM_vs_pt.pdf").c_str(),"pdf");

  //// Combined plot for comparison
  efficiency_UGT_pt_eta_2p25_2p5->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_UGT_pt_eta_2p25_2p5->SetLineColor(kBlack);
  efficiency_UGT_pt_eta_2p25_2p5->SetMarkerColor(kBlack);
  efficiency_UGT_pt_eta_2p25_2p5->SetMarkerStyle(21);
  efficiency_UGT_pt_eta_2p25_2p5->Draw("AEPL");  
  efficiency_UGT_pt_eta_2p25_2p5->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_UGT_pt_eta_2p25_2p5->GetYaxis()->SetTitle("Efficiency (bx=-1)");
  efficiency_IsoEG_pt_eta_2p25_2p5->SetMarkerColor(kRed);
  efficiency_IsoEG_pt_eta_2p25_2p5->SetLineColor(kRed);
  efficiency_IsoEG_pt_eta_2p25_2p5->Draw("EPLsame");
  efficiency_EG_pt_eta_2p25_2p5->SetMarkerColor(kBlue);
  efficiency_EG_pt_eta_2p25_2p5->SetLineColor(kBlue);
  efficiency_EG_pt_eta_2p25_2p5->Draw("EPLsame");
  // efficiency_EGOR_pt_eta_2p25_2p5->SetMarkerColor(kGreen+1);
  //  efficiency_EGOR_pt_eta_2p25_2p5->SetLineColor(kGreen+1);
  //  efficiency_EGOR_pt_eta_2p25_2p5->Draw("EPLsame");
  efficiency_ETM_pt_eta_2p25_2p5->SetMarkerStyle(24);
  efficiency_ETM_pt_eta_2p25_2p5->SetMarkerColor(kCyan);
  efficiency_ETM_pt_eta_2p25_2p5->SetLineColor(kCyan);
  efficiency_ETM_pt_eta_2p25_2p5->Draw("EPLsame");
  efficiency_EGETM_pt_eta_2p25_2p5->SetMarkerColor(kGreen+1);
  efficiency_EGETM_pt_eta_2p25_2p5->SetLineColor(kGreen+1);
  efficiency_EGETM_pt_eta_2p25_2p5->Draw("EPLsame");
  leg.Clear();
  leg.AddEntry((TObject*)(0),"2.25 < |#eta| < 2.50","");
  leg.AddEntry(efficiency_UGT_pt_eta_2p25_2p5,"Global Trigger","PL");
  leg.AddEntry(efficiency_IsoEG_pt_eta_2p25_2p5,"Iso-EGamma","PL");
  leg.AddEntry(efficiency_EG_pt_eta_2p25_2p5,"EGamma","PL");
  leg.AddEntry(efficiency_ETM_pt_eta_2p25_2p5,"ETM","PL");
  leg.AddEntry(efficiency_EGETM_pt_eta_2p25_2p5,"EG OR IsoEG OR ETM","PL");
  leg.Draw("same");
  CMS_lumi(canvas,"");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_UGT_eta_bin_2p25_2p5.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_UGT_eta_bin_2p25_2p5.pdf").c_str(),"pdf");

  efficiency_UGT_pt_eta_2p5_2p75->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_UGT_pt_eta_2p5_2p75->SetLineColor(kBlack);
  efficiency_UGT_pt_eta_2p5_2p75->SetMarkerColor(kBlack);
  efficiency_UGT_pt_eta_2p5_2p75->SetMarkerStyle(21);
  efficiency_UGT_pt_eta_2p5_2p75->Draw("AEPL");
  efficiency_UGT_pt_eta_2p5_2p75->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_UGT_pt_eta_2p5_2p75->GetYaxis()->SetTitle("Efficiency (bx=-1)");
  efficiency_IsoEG_pt_eta_2p5_2p75->SetMarkerColor(kRed);
  efficiency_IsoEG_pt_eta_2p5_2p75->SetLineColor(kRed);
  efficiency_IsoEG_pt_eta_2p5_2p75->Draw("EPLsame");
  efficiency_EG_pt_eta_2p5_2p75->SetMarkerColor(kBlue);
  efficiency_EG_pt_eta_2p5_2p75->SetLineColor(kBlue);
  efficiency_EG_pt_eta_2p5_2p75->Draw("EPLsame");
  //efficiency_EGOR_pt_eta_2p5_2p75->SetMarkerColor(kGreen+1);
  //efficiency_EGOR_pt_eta_2p5_2p75->SetLineColor(kGreen+1);
  //efficiency_EGOR_pt_eta_2p5_2p75->Draw("EPLsame");
  efficiency_ETM_pt_eta_2p5_2p75->SetMarkerStyle(24);
  efficiency_ETM_pt_eta_2p5_2p75->SetMarkerColor(kCyan);
  efficiency_ETM_pt_eta_2p5_2p75->SetLineColor(kCyan);
  efficiency_ETM_pt_eta_2p5_2p75->Draw("EPLsame");
  efficiency_EGETM_pt_eta_2p5_2p75->SetMarkerColor(kGreen+1);
  efficiency_EGETM_pt_eta_2p5_2p75->SetLineColor(kGreen+1);
  efficiency_EGETM_pt_eta_2p5_2p75->Draw("EPLsame");
  leg.Clear();
  leg.AddEntry((TObject*)(0),"2.50 < |#eta| < 2.75","");
  leg.AddEntry(efficiency_UGT_pt_eta_2p5_2p75,"Global Trigger","PL");
  leg.AddEntry(efficiency_IsoEG_pt_eta_2p5_2p75,"Iso-EGamma","PL");
  leg.AddEntry(efficiency_EG_pt_eta_2p5_2p75,"EGamma","PL");
  //  leg.AddEntry(efficiency_EGOR_pt_eta_2p5_2p75,"EG OR IsoEG","PL");
  leg.AddEntry(efficiency_ETM_pt_eta_2p5_2p75,"ETM","PL");
  leg.AddEntry(efficiency_EGETM_pt_eta_2p5_2p75,"EG OR IsoEG OR ETM","PL");
  leg.Draw("same");
  CMS_lumi(canvas,"");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_UGT_eta_bin_2p5_2p75.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_UGT_eta_bin_2p5_2p75.pdf").c_str(),"pdf");

  efficiency_UGT_pt_eta_2p75_3p0->GetYaxis()->SetRangeUser(0.,1.);
  efficiency_UGT_pt_eta_2p75_3p0->SetLineColor(kBlack);
  efficiency_UGT_pt_eta_2p75_3p0->SetMarkerColor(kBlack);
  efficiency_UGT_pt_eta_2p75_3p0->SetMarkerStyle(21);
  efficiency_UGT_pt_eta_2p75_3p0->Draw("AEPL");
  efficiency_UGT_pt_eta_2p75_3p0->GetXaxis()->SetTitle("p_{T}^{j,reco} [GeV]");
  efficiency_UGT_pt_eta_2p75_3p0->GetYaxis()->SetTitle("Efficiency (bx=-1)");
  efficiency_IsoEG_pt_eta_2p75_3p0->SetMarkerColor(kRed);
  efficiency_IsoEG_pt_eta_2p75_3p0->SetLineColor(kRed);
  efficiency_IsoEG_pt_eta_2p75_3p0->Draw("EPLsame");
  efficiency_EG_pt_eta_2p75_3p0->SetMarkerColor(kBlue);
  efficiency_EG_pt_eta_2p75_3p0->SetLineColor(kBlue);
  efficiency_EG_pt_eta_2p75_3p0->Draw("EPLsame");
  //  efficiency_EGOR_pt_eta_2p75_3p0->SetMarkerColor(kGreen+1);
  //  efficiency_EGOR_pt_eta_2p75_3p0->SetLineColor(kGreen+1);
  //  efficiency_EGOR_pt_eta_2p75_3p0->Draw("EPLsame");
  efficiency_ETM_pt_eta_2p75_3p0->SetMarkerStyle(24);
  efficiency_ETM_pt_eta_2p75_3p0->SetMarkerColor(kCyan);
  efficiency_ETM_pt_eta_2p75_3p0->SetLineColor(kCyan);
  efficiency_ETM_pt_eta_2p75_3p0->Draw("EPLsame");
  efficiency_EGETM_pt_eta_2p75_3p0->SetMarkerColor(kGreen+1);
  efficiency_EGETM_pt_eta_2p75_3p0->SetLineColor(kGreen+1);
  efficiency_EGETM_pt_eta_2p75_3p0->Draw("EPLsame");
  leg.Clear();
  leg.AddEntry((TObject*)(0),"2.75 < |#eta| < 3.0","");
  leg.AddEntry(efficiency_UGT_pt_eta_2p75_3p0,"Global Trigger","PL");
  leg.AddEntry(efficiency_IsoEG_pt_eta_2p75_3p0,"Iso-EGamma","PL");
  leg.AddEntry(efficiency_EG_pt_eta_2p75_3p0,"EGamma","PL");
  leg.AddEntry(efficiency_ETM_pt_eta_2p75_3p0,"ETM","PL");
  leg.AddEntry(efficiency_EGETM_pt_eta_2p75_3p0,"EG OR IsoEG OR ETM","PL");
  leg.Draw("same");
  CMS_lumi(canvas,"");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_UGT_eta_bin_2p75_3p0.png").c_str(),"png");
  canvas->SaveAs((outputDIR+"/prefiring_efficiency_EG_vs_UGT_eta_bin_2p75_3p0.pdf").c_str(),"pdf");

  ////
  TFile* output = NULL;
  if(region == analysisRegion::SingleMuon)
    output= new TFile((outputDIR+"/output_SingleMuon.root").c_str(),"RECREATE");
  else if(region == analysisRegion::JetHT)
    output= new TFile((outputDIR+"/output_JetHT.root").c_str(),"RECREATE");
  else if(region == analysisRegion::MET)
    output= new TFile((outputDIR+"/output_MET.root").c_str(),"RECREATE");

  denL1IsoEG_pt_eta->Write();
  denL1IsoEG_eta_phi->Write();
  denL1IsoEG_ptem_eta->Write();
  L1IsoEG_pt_eta_bx0->Write();
  L1IsoEG_eta_phi_bx0->Write();
  L1IsoEG_pt_eta_bxm1->Write();
  L1IsoEG_eta_phi_bxm1->Write();
  L1IsoEG_pt_eta_bxm2->Write();
  L1IsoEG_eta_phi_bxm2->Write();
  L1IsoEG_pt_eta_2p25_2p5_denom->Write();
  L1IsoEG_pt_eta_2p5_2p75_denom->Write();
  L1IsoEG_pt_eta_2p75_3p0_denom->Write();
  L1IsoEG_pt_eta_2p25_2p5_bxm1->Write();
  L1IsoEG_pt_eta_2p5_2p75_bxm1->Write();
  L1IsoEG_pt_eta_2p75_3p0_bxm1->Write();

  denL1EG_pt_eta->Write();
  denL1EG_eta_phi->Write();
  L1EG_pt_eta_bx0->Write();
  L1EG_eta_phi_bx0->Write();
  L1EG_pt_eta_bxm1->Write();
  L1EG_eta_phi_bxm1->Write();
  L1EG_pt_eta_bxm2->Write();
  L1EG_eta_phi_bxm2->Write();
  L1EG_pt_eta_2p25_2p5_denom->Write();
  L1EG_pt_eta_2p5_2p75_denom->Write();
  L1EG_pt_eta_2p75_3p0_denom->Write();
  L1EG_pt_eta_2p25_2p5_bxm1->Write();
  L1EG_pt_eta_2p5_2p75_bxm1->Write();
  L1EG_pt_eta_2p75_3p0_bxm1->Write();

  denL1EGOR_pt_eta->Write();
  denL1EGOR_eta_phi->Write();
  L1EGOR_pt_eta_bx0->Write();
  L1EGOR_eta_phi_bx0->Write();
  L1EGOR_pt_eta_bxm1->Write();
  L1EGOR_eta_phi_bxm1->Write();
  L1EGOR_pt_eta_bxm2->Write();
  L1EGOR_eta_phi_bxm2->Write();
  L1EGOR_pt_eta_2p25_2p5_denom->Write();
  L1EGOR_pt_eta_2p5_2p75_denom->Write();
  L1EGOR_pt_eta_2p75_3p0_denom->Write();
  L1EGOR_pt_eta_2p25_2p5_bxm1->Write();
  L1EGOR_pt_eta_2p5_2p75_bxm1->Write();
  L1EGOR_pt_eta_2p75_3p0_bxm1->Write();

  UGT_pt_eta_2p25_2p5_denom->Write();
  UGT_pt_eta_2p5_2p75_denom->Write();
  UGT_pt_eta_2p75_3p0_denom->Write();
  UGT_pt_eta_2p25_2p5_bxm1->Write();
  UGT_pt_eta_2p5_2p75_bxm1->Write();
  UGT_pt_eta_2p75_3p0_bxm1->Write();

  ETM_pt_eta_2p25_2p5_denom->Write();
  ETM_pt_eta_2p5_2p75_denom->Write();
  ETM_pt_eta_2p75_3p0_denom->Write();
  ETM_pt_eta_2p25_2p5_bxm1->Write();
  ETM_pt_eta_2p5_2p75_bxm1->Write();
  ETM_pt_eta_2p75_3p0_bxm1->Write();

  EGETM_pt_eta_2p25_2p5_denom->Write();
  EGETM_pt_eta_2p5_2p75_denom->Write();
  EGETM_pt_eta_2p75_3p0_denom->Write();
  EGETM_pt_eta_2p25_2p5_bxm1->Write();
  EGETM_pt_eta_2p5_2p75_bxm1->Write();
  EGETM_pt_eta_2p75_3p0_bxm1->Write();

  efficiency_IsoEG_pt_eta_2p25_2p5->SetName("efficiency_IsoEG_pt_eta_2p25_2p5");
  efficiency_IsoEG_pt_eta_2p5_2p75->SetName("efficiency_IsoEG_pt_eta_2p5_2p75");
  efficiency_IsoEG_pt_eta_2p75_3p0->SetName("efficiency_IsoEG_pt_eta_2p75_3p0");
  efficiency_IsoEG_pt_eta_2p25_2p5->Write();
  efficiency_IsoEG_pt_eta_2p5_2p75->Write();
  efficiency_IsoEG_pt_eta_2p75_3p0->Write();

  efficiency_EG_pt_eta_2p25_2p5->SetName("efficiency_EG_pt_eta_2p25_2p5");
  efficiency_EG_pt_eta_2p5_2p75->SetName("efficiency_EG_pt_eta_2p5_2p75");
  efficiency_EG_pt_eta_2p75_3p0->SetName("efficiency_EG_pt_eta_2p75_3p0");
  efficiency_EG_pt_eta_2p25_2p5->Write();
  efficiency_EG_pt_eta_2p5_2p75->Write();
  efficiency_EG_pt_eta_2p75_3p0->Write();

  efficiency_EGOR_pt_eta_2p25_2p5->SetName("efficiency_EGOR_pt_eta_2p25_2p5");
  efficiency_EGOR_pt_eta_2p5_2p75->SetName("efficiency_EGOR_pt_eta_2p5_2p75");
  efficiency_EGOR_pt_eta_2p75_3p0->SetName("efficiency_EGOR_pt_eta_2p75_3p0");
  efficiency_EGOR_pt_eta_2p25_2p5->Write();
  efficiency_EGOR_pt_eta_2p5_2p75->Write();
  efficiency_EGOR_pt_eta_2p75_3p0->Write();

  efficiency_UGT_pt_eta_2p25_2p5->SetName("efficiency_UGT_pt_eta_2p25_2p5");
  efficiency_UGT_pt_eta_2p5_2p75->SetName("efficiency_UGT_pt_eta_2p5_2p75");
  efficiency_UGT_pt_eta_2p75_3p0->SetName("efficiency_UGT_pt_eta_2p75_3p0");
  efficiency_UGT_pt_eta_2p25_2p5->Write();
  efficiency_UGT_pt_eta_2p5_2p75->Write();
  efficiency_UGT_pt_eta_2p75_3p0->Write();

  efficiency_UGT_ptem_eta_2p25_2p5->SetName("efficiency_UGT_ptem_eta_2p25_2p5");
  efficiency_UGT_ptem_eta_2p5_2p75->SetName("efficiency_UGT_ptem_eta_2p5_2p75");
  efficiency_UGT_ptem_eta_2p75_3p0->SetName("efficiency_UGT_ptem_eta_2p75_3p0");
  efficiency_UGT_ptem_eta_2p25_2p5->Write();
  efficiency_UGT_ptem_eta_2p5_2p75->Write();
  efficiency_UGT_ptem_eta_2p75_3p0->Write();

  efficiency_ETM_pt_eta_2p25_2p5->SetName("efficiency_ETM_pt_eta_2p25_2p5");
  efficiency_ETM_pt_eta_2p5_2p75->SetName("efficiency_ETM_pt_eta_2p5_2p75");
  efficiency_ETM_pt_eta_2p75_3p0->SetName("efficiency_ETM_pt_eta_2p75_3p0");
  efficiency_ETM_pt_eta_2p25_2p5->Write();
  efficiency_ETM_pt_eta_2p5_2p75->Write();
  efficiency_ETM_pt_eta_2p75_3p0->Write();

  efficiency_EGETM_pt_eta_2p25_2p5->SetName("efficiency_EGETM_pt_eta_2p25_2p5");
  efficiency_EGETM_pt_eta_2p5_2p75->SetName("efficiency_EGETM_pt_eta_2p5_2p75");
  efficiency_EGETM_pt_eta_2p75_3p0->SetName("efficiency_EGETM_pt_eta_2p75_3p0");
  efficiency_EGETM_pt_eta_2p25_2p5->Write();
  efficiency_EGETM_pt_eta_2p5_2p75->Write();
  efficiency_EGETM_pt_eta_2p75_3p0->Write();

  jet_emfraction_2p25_2p5->Write();
  jet_emfraction_2p5_2p75->Write();
  jet_emfraction_2p75_3p0->Write();
  
  output->Close();
}
