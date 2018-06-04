// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/transform.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/L1Trigger/interface/EGamma.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"

#include "TTree.h"

#include "Math/LorentzVector.h"
#include "Math/PxPyPzE4D.h"
// == reco::LorentzVector
typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>> LorentzVector;

namespace {
  struct EventStruct {
    Long64_t run;
    Long64_t lumi;
    Long64_t event;
    int bunchCrossing;
    
    std::vector<LorentzVector> jet_p4;
    std::vector<float> jet_neutralEmFrac;
    std::vector<float> jet_neutralHadFrac;

    std::vector<int> L1EG_bx;
    std::vector<LorentzVector> L1EG_p4;
    std::vector<int> L1EG_iso;
  };

  std::vector<const pat::TriggerObjectStandAlone*> getMatchedObjs(const float eta, const float phi, const std::vector<pat::TriggerObjectStandAlone>& trigObjs, const float maxDeltaR=0.1)
  {
    std::vector<const pat::TriggerObjectStandAlone*> matchedObjs;
    const float maxDR2 = maxDeltaR*maxDeltaR;
    for(auto& trigObj : trigObjs){
      const float dR2 = reco::deltaR2(eta,phi,trigObj.eta(),trigObj.phi());
      if(dR2<maxDR2) matchedObjs.push_back(&trigObj);
    }
    return matchedObjs;
  }
}

class PrefiringJetAna : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
  public:
    explicit PrefiringJetAna(const edm::ParameterSet&);
    ~PrefiringJetAna();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    // virtual void beginJob() override;
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    // virtual void endJob() override;

    edm::EDGetTokenT<pat::JetCollection> jetToken_;
    StringCutObjectSelector<pat::Jet> tagJetCut_;
    edm::EDGetTokenT<BXVector<l1t::EGamma>> l1egToken_;
    edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;
    edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescalesToken_;

    TTree * tree_;
    EventStruct event_;
};

PrefiringJetAna::PrefiringJetAna(const edm::ParameterSet& iConfig):
  jetToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jetSrc"))),
  tagJetCut_(iConfig.getParameter<std::string>("tagJetCut")),
  l1egToken_(consumes<BXVector<l1t::EGamma>>(iConfig.getParameter<edm::InputTag>("l1egSrc"))),
  triggerObjectsToken_(consumes<pat::TriggerObjectStandAloneCollection>(iConfig.getParameter<edm::InputTag>("triggerObjects"))),
  triggerPrescalesToken_(consumes<pat::PackedTriggerPrescales>(iConfig.getParameter<edm::InputTag>("triggerPrescales")))
{
  usesResource("TFileService");
  edm::Service<TFileService> fs;

  tree_ = fs->make<TTree>("tree","Event Summary");
  tree_->Branch("run", &event_.run);
  tree_->Branch("lumi", &event_.lumi);
  tree_->Branch("event", &event_.event);
  tree_->Branch("bunchCrossing", &event_.bunchCrossing);
  tree_->Branch("jet_p4", &event_.jet_p4);
  tree_->Branch("jet_neutralEmFrac", &event_.jet_neutralEmFrac);
  tree_->Branch("jet_neutralHadFrac", &event_.jet_neutralHadFrac);
  tree_->Branch("L1EG_bx", &event_.L1EG_bx);
  tree_->Branch("L1EG_p4", &event_.L1EG_p4);
  tree_->Branch("L1EG_iso", &event_.L1EG_iso);
}


PrefiringJetAna::~PrefiringJetAna()
{
}

void
PrefiringJetAna::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  event_.run = iEvent.run();
  event_.lumi = iEvent.luminosityBlock();
  event_.event = iEvent.id().event();
  event_.bunchCrossing = iEvent.bunchCrossing();

  Handle<pat::JetCollection> jetHandle;
  iEvent.getByToken(jetToken_, jetHandle);

  Handle<BXVector<l1t::EGamma>> l1egHandle;
  iEvent.getByToken(l1egToken_, l1egHandle);

  Handle<pat::TriggerObjectStandAloneCollection> triggerObjectsHandle;
  iEvent.getByToken(triggerObjectsToken_, triggerObjectsHandle);

  Handle<pat::PackedTriggerPrescales> triggerPrescalesHandle;
  iEvent.getByToken(triggerPrescalesToken_, triggerPrescalesHandle);
  const edm::TriggerResults& triggerResults = triggerPrescalesHandle->triggerResults();


  event_.jet_p4.clear();
  event_.jet_neutralEmFrac.clear();
  event_.jet_neutralHadFrac.clear();
  for(const auto& jet : *jetHandle) {
    if ( not tagJetCut_(jet) ) continue;
    event_.jet_p4.push_back( jet.p4() );
    event_.jet_neutralEmFrac.push_back( jet.neutralEmEnergyFraction() );
    event_.jet_neutralHadFrac.push_back( jet.neutralHadronEnergyFraction() );
  }

  event_.L1EG_bx.clear();
  event_.L1EG_p4.clear();
  event_.L1EG_iso.clear();

  auto readBx = [&] (const BXVector<l1t::EGamma>& egVect, int bx) {
    for (auto itL1=l1egHandle->begin(bx); itL1!=l1egHandle->end(bx); ++itL1) {
      event_.L1EG_bx.push_back(bx);
      event_.L1EG_p4.push_back(itL1->p4());
      event_.L1EG_iso.push_back(itL1->hwIso());
    }
  };

  readBx(*l1egHandle, -2);
  readBx(*l1egHandle, -1);
  readBx(*l1egHandle,  0);
  readBx(*l1egHandle,  1);
  readBx(*l1egHandle,  2);

  tree_->Fill();
}


void
PrefiringJetAna::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}


DEFINE_FWK_MODULE(PrefiringJetAna);
