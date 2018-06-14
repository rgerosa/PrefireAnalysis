#define JetAnalysis_cxx
// The class definition in JetAnalysis.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.


// The following methods are defined in this file:
//   Begin():      called every time a loop on the tree starts,
//              a convenient place to create your histograms.
//   SlaveBegin():  called after Begin(), when on PROOF called only on the
//              slave servers.
//   Process():    called for each event, in this function you decide what
//              to read and fill your histograms.
//   SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//              called only on the slave servers.
//   Terminate():   called at the end of the loop on the tree,
//              a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// root> T->Process("JetAnalysis.C")
// root> T->Process("JetAnalysis.C","some options")
// root> T->Process("JetAnalysis.C+")
//


#include "JetAnalysis.h"

void JetAnalysis::Begin(TTree * /*tree*/)
{
  // The Begin() function is called at the start of the query.
  // When running with PROOF Begin() is only called on the client.
  // The tree argument is deprecated (on PROOF 0 is passed).

  TString option = GetOption();

  hJetPtEtaEGeffDenom_ = newOutput<TH2D>("denom", ";Jet p_{T} [GeV];Jet |#eta|;Counts", 20, 30, 630, 50, 0, 5);;
  hJetPtEtaEGeffNum_bxm2_ = newOutput<TH2D>("num_bxm2", ";Jet p_{T} [GeV];Jet |#eta|;Counts", 20, 30, 630, 50, 0, 5);;
  hJetPtEtaEGeffNum_bxm1_ = newOutput<TH2D>("num_bxm1", ";Jet p_{T} [GeV];Jet |#eta|;Counts", 20, 30, 630, 50, 0, 5);;
  hJetPtEtaEGeffNum_bx0_ =  newOutput<TH2D>("num_bx0",  ";Jet p_{T} [GeV];Jet |#eta|;Counts", 20, 30, 630, 50, 0, 5);;
  hJetPtEtaEGeffNum_bx1_ =  newOutput<TH2D>("num_bx1",  ";Jet p_{T} [GeV];Jet |#eta|;Counts", 20, 30, 630, 50, 0, 5);;
  hJetPtEtaEGeffNum_bx2_ =  newOutput<TH2D>("num_bx2",  ";Jet p_{T} [GeV];Jet |#eta|;Counts", 20, 30, 630, 50, 0, 5);;

  hJet100EGEtaPhi_ = newOutput<TH2D>("jet100EGEtaPhi", ";L1EG #eta;L1EG #phi;Counts", 57, -3, 3, 72, -3.1415, 3.1415);

  hJetL1ADenom_ = newOutput<TH1D>("denomL1A", ";Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetL1ANum_bxm2_ = newOutput<TH1D>("numL1A_bxm2", ";Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetL1ANum_bxm1_ = newOutput<TH1D>("numL1A_bxm1", ";Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetL1ANum_bx0_  = newOutput<TH1D>("numL1A_bx0",  ";Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetL1ANum_bx1_  = newOutput<TH1D>("numL1A_bx1",  ";Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetL1ANum_bx2_  = newOutput<TH1D>("numL1A_bx2",  ";Jet p_{T} [GeV];Counts", 20, 30, 630);

  hJetEGm1thrDenom_ = newOutput<TH1D>("denomJetEGthr", ";Jet p_{T} [GeV];L1EG in BX -1 Efficiency", 20, 30, 630);
  hJetEGm1thrNum_EG22_ = newOutput<TH1D>("numJetEGthr_eg22", "L1IsoEG22;Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetEGm1thrNum_EG26_ = newOutput<TH1D>("numJetEGthr_eg26", "L1IsoEG26;Jet p_{T} [GeV];Counts", 20, 30, 630);
  hJetEGm1thrNum_EG30_ = newOutput<TH1D>("numJetEGthr_eg30", "L1IsoEG30;Jet p_{T} [GeV];Counts", 20, 30, 630);

}

void JetAnalysis::SlaveBegin(TTree * /*tree*/)
{
  // The SlaveBegin() function is called after the Begin() function.
  // When running with PROOF SlaveBegin() is called on each slave server.
  // The tree argument is deprecated (on PROOF 0 is passed).

  TString option = GetOption();

}

Bool_t JetAnalysis::Process(Long64_t entry)
{
  using namespace ROOT::Math::VectorUtil;
  auto bit = [] (int i) { return 1<<i; };
  fReader.SetEntry(entry);

  bool vetoMedJets{false};
  std::vector<LorentzVector> forwardJets;
  for(size_t iJet=0; iJet<jet_p4.GetSize(); ++iJet) {
    auto& jet = jet_p4[iJet];

    hJetPtEtaEGeffDenom_->Fill(jet.Pt(), std::abs(jet.Eta()));
    if ( std::abs(jet.Eta()) >= 2.5 and std::abs(jet.Eta()) < 3. ) {
      hJetEGm1thrDenom_->Fill(jet.Pt());
      forwardJets.emplace_back(jet);
    }
    else if ( std::abs(jet.Eta()) >= 2.0 and std::abs(jet.Eta()) < 2.5 ) {
      vetoMedJets = true;
    }


    int match_bx{0};
    int match_thr{0};
    for(size_t iEG=0; iEG<L1EG_p4.GetSize(); ++iEG) {
      if ( L1EG_p4[iEG].Pt() > 30. and (L1EG_iso[iEG] & 0x1) and DeltaR(L1EG_p4[iEG], jet) < 0.4 ) {
        match_bx |= bit(L1EG_bx[iEG]+2);
        if ( jet.Pt() > 10. and L1EG_bx[iEG] == -1 ) {
          hJet100EGEtaPhi_->Fill(L1EG_p4[iEG].Eta(), L1EG_p4[iEG].Phi());
        }
      }
      if ( std::abs(jet.Eta()) >= 2.5 and std::abs(jet.Eta()) < 3. and (L1EG_iso[iEG] & 0x1) and DeltaR(L1EG_p4[iEG], jet) < 0.4 ) {
        if ( L1EG_p4[iEG].Pt() > 22. ) match_thr |= bit(0);
        if ( L1EG_p4[iEG].Pt() > 26. ) match_thr |= bit(1);
        if ( L1EG_p4[iEG].Pt() > 30. ) match_thr |= bit(2);
      }
    }
    if ( match_bx & bit(-2+2) ) hJetPtEtaEGeffNum_bxm2_->Fill(jet.Pt(), std::abs(jet.Eta()));
    if ( match_bx & bit(-1+2) ) hJetPtEtaEGeffNum_bxm1_->Fill(jet.Pt(), std::abs(jet.Eta()));
    if ( match_bx & bit( 0+2) ) hJetPtEtaEGeffNum_bx0_ ->Fill(jet.Pt(), std::abs(jet.Eta()));
    if ( match_bx & bit( 1+2) ) hJetPtEtaEGeffNum_bx1_ ->Fill(jet.Pt(), std::abs(jet.Eta()));
    if ( match_bx & bit( 2+2) ) hJetPtEtaEGeffNum_bx2_ ->Fill(jet.Pt(), std::abs(jet.Eta()));

    if ( match_thr & bit(0) ) hJetEGm1thrNum_EG22_->Fill(jet.Pt());
    if ( match_thr & bit(1) ) hJetEGm1thrNum_EG26_->Fill(jet.Pt());
    if ( match_thr & bit(2) ) hJetEGm1thrNum_EG30_->Fill(jet.Pt());
  }

  if ( forwardJets.size() == 1 and not vetoMedJets ) {
    hJetL1ADenom_->Fill(forwardJets[0].Pt());
    if ( L1GtBx[0] ) hJetL1ANum_bxm2_->Fill(forwardJets[0].Pt());
    if ( L1GtBx[1] ) hJetL1ANum_bxm1_->Fill(forwardJets[0].Pt());
    if ( L1GtBx[2] ) hJetL1ANum_bx0_->Fill(forwardJets[0].Pt());
    if ( L1GtBx[3] ) hJetL1ANum_bx1_->Fill(forwardJets[0].Pt());
    if ( L1GtBx[4] ) hJetL1ANum_bx2_->Fill(forwardJets[0].Pt());
  }

  return kTRUE;
}

void JetAnalysis::SlaveTerminate()
{
  // The SlaveTerminate() function is called after all entries or objects
  // have been processed. When running with PROOF SlaveTerminate() is called
  // on each slave server.

}

void JetAnalysis::Terminate()
{
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.

}
