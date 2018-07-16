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
#include "FWCore/Framework/interface/ESHandle.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/L1Trigger/interface/EGamma.h"
#include "DataFormats/L1Trigger/interface/Jet.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1Trigger/interface/EtSum.h"  
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"

#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"

#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmAlgorithm.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"
#include "DataFormats/L1TGlobal/interface/GlobalExtBlk.h"

#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

#include "TTree.h"

#include "Math/LorentzVector.h"
#include "Math/PxPyPzE4D.h"
#include "TPRegexp.h"

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

    std::vector<LorentzVector> electron_p4;
    std::vector<bool> electron_tight;
    std::vector<LorentzVector> photon_p4;
    std::vector<bool> photon_tight;
    std::vector<LorentzVector> muon_p4;
    std::vector<bool> muon_tight;
    
    std::vector<int> L1EG_bx;
    std::vector<LorentzVector> L1EG_p4;
    std::vector<int> L1EG_iso;

    std::vector<int> L1Jet_bx;
    std::vector<LorentzVector> L1Jet_p4;

    std::vector<int> L1Mu_bx;
    std::vector<LorentzVector> L1Mu_p4;

    std::vector<int> L1HT_bx;
    std::vector<LorentzVector> L1HT_p4;
    std::vector<int> L1HTMiss_bx;
    std::vector<LorentzVector> L1HTMiss_p4;
    std::vector<int> L1MET_bx;
    std::vector<LorentzVector> L1MET_p4;
    
    std::vector<int> L1GtBx;

    std::vector<int> L1EGSeeds_bx;
    std::vector<std::string> L1EGSeeds_name;
    std::vector<int> L1IsoEGSeeds_bx;
    std::vector<std::string> L1IsoEGSeeds_name;
    std::vector<int> L1ETMSeeds_bx;
    std::vector<std::string> L1ETMSeeds_name;

    float met;
    float calomet;
    float metphi;
    float calometphi;

    uint8_t flaghbhenoise;
    uint8_t flaghbheiso;
    uint8_t flageebadsc;
    uint8_t flagecaltp;
    uint8_t flaggoodvertices;
    uint8_t flagglobaltighthalo;
    uint8_t flagbadchpf;
    uint8_t flagbadpfmu;
    uint8_t flagbadmuon;
    uint8_t flagduplicatemuon;

  };
}

class PrefiringJetAna : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns>  {
public:
  explicit PrefiringJetAna(const edm::ParameterSet&);
  ~PrefiringJetAna();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:

  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void beginRun(const edm::Run &, const edm::EventSetup &) override;
  virtual void endRun(const edm::Run &, const edm::EventSetup &) override;
  
  PFJetIDSelectionFunctor looseJetIdSelector_{PFJetIDSelectionFunctor::WINTER16, PFJetIDSelectionFunctor::LOOSE};
  pat::strbitset hasLooseId_;
  
  edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
  edm::EDGetTokenT<pat::JetCollection> jetToken_;
  edm::EDGetTokenT<pat::MuonCollection> muonToken_;
  edm::EDGetTokenT<pat::ElectronCollection> electronToken_;
  edm::EDGetTokenT<pat::PhotonCollection> photonToken_;
  edm::EDGetTokenT<pat::METCollection> metToken_;
  edm::EDGetTokenT<pat::METCollection> metOriginalToken_;
  StringCutObjectSelector<pat::Jet> tagJetCut_;
  StringCutObjectSelector<pat::Muon> looseMuonCut_;
  StringCutObjectSelector<pat::Muon> tightMuonCut_;
  StringCutObjectSelector<pat::Electron> vetoElectronCut_;
  StringCutObjectSelector<pat::Electron> tightElectronCut_;
  StringCutObjectSelector<pat::Photon> loosePhotonCut_;
  StringCutObjectSelector<pat::Photon> tightPhotonCut_;
  edm::EDGetTokenT<edm::ValueMap<bool> > vetoElectronIdMapToken;
  edm::EDGetTokenT<edm::ValueMap<bool> > tightElectronIdMapToken;
  edm::EDGetTokenT<edm::ValueMap<bool> > loosePhotonIdMapToken;
  edm::EDGetTokenT<edm::ValueMap<bool> > tightPhotonIdMapToken;
  edm::EDGetTokenT<BXVector<l1t::EGamma>>  l1egToken_;
  edm::EDGetTokenT<BXVector<l1t::Jet>>     l1jetToken_;
  edm::EDGetTokenT<BXVector<l1t::Muon>>    l1muToken_;
  edm::EDGetTokenT<BXVector<l1t::EtSum>>    l1METToken_;
  edm::EDGetTokenT<BXVector<GlobalAlgBlk>> l1GtToken_;
  edm::EDGetTokenT<edm::TriggerResults> filterResultsToken;
  edm::EDGetTokenT<bool> badChargedCandidateToken;
  edm::EDGetTokenT<bool> badPFMuonToken;  
  edm::InputTag triggerResultsTag_;
  edm::InputTag filterResultsTag_;
  
  TTree * tree_;
  EventStruct event_;
  
  std::unique_ptr<HLTPrescaleProvider> hltPrescaleProvider;
  
    // inner vectors
  std::vector<std::string>   filterPathsVector;
  std::map<std::string,int>  filterPathsMap;
  
  
};

PrefiringJetAna::PrefiringJetAna(const edm::ParameterSet& iConfig):
  vertexToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexSrc"))),
  jetToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jetSrc"))),
  muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muonSrc"))),
  electronToken_(consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("electronSrc"))),
  photonToken_(consumes<pat::PhotonCollection>(iConfig.getParameter<edm::InputTag>("photonSrc"))),
  metToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("metSrc"))),
  metOriginalToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("metOriginalSrc"))),
  tagJetCut_(iConfig.getParameter<std::string>("tagJetCut")),
  looseMuonCut_(iConfig.getParameter<std::string>("looseMuonCut")),
  tightMuonCut_(iConfig.getParameter<std::string>("tightMuonCut")),
  vetoElectronCut_(iConfig.getParameter<std::string>("vetoElectronCut")),
  tightElectronCut_(iConfig.getParameter<std::string>("tightElectronCut")),
  loosePhotonCut_(iConfig.getParameter<std::string>("loosePhotonCut")),
  tightPhotonCut_(iConfig.getParameter<std::string>("tightPhotonCut")),
  vetoElectronIdMapToken(consumes<edm::ValueMap<bool> > (iConfig.getParameter<edm::InputTag>("vetoElectronValueMap"))),
  tightElectronIdMapToken(consumes<edm::ValueMap<bool> > (iConfig.getParameter<edm::InputTag>("tightElectronValueMap"))),
  loosePhotonIdMapToken(consumes<edm::ValueMap<bool> > (iConfig.getParameter<edm::InputTag>("loosePhotonValueMap"))),
  tightPhotonIdMapToken(consumes<edm::ValueMap<bool> > (iConfig.getParameter<edm::InputTag>("tightPhotonValueMap"))),
  l1egToken_(consumes<BXVector<l1t::EGamma>>(iConfig.getParameter<edm::InputTag>("l1egSrc"))),
  l1jetToken_(consumes<BXVector<l1t::Jet>>(iConfig.getParameter<edm::InputTag>("l1jetSrc"))),
  l1muToken_(consumes<BXVector<l1t::Muon>>(iConfig.getParameter<edm::InputTag>("l1muSrc"))),
  l1METToken_(consumes<BXVector<l1t::EtSum>>(iConfig.getParameter<edm::InputTag>("l1METSrc"))),
  l1GtToken_(consumes<BXVector<GlobalAlgBlk>>(iConfig.getParameter<edm::InputTag>("l1GtSrc"))),
  filterResultsToken       (consumes<edm::TriggerResults> (iConfig.getParameter<edm::InputTag>("filterResults"))),
  badChargedCandidateToken (consumes<bool>(iConfig.getParameter<edm::InputTag>("badChargedCandidate"))),
  badPFMuonToken (consumes<bool>(iConfig.getParameter<edm::InputTag>("badPFMuon"))),
  triggerResultsTag_       (iConfig.getParameter<edm::InputTag>("triggerResults")),
  filterResultsTag_        (iConfig.getParameter<edm::InputTag>("filterResults"))
{
  usesResource("TFileService");
  edm::Service<TFileService> fs;

  hasLooseId_ = looseJetIdSelector_.getBitTemplate();

  tree_ = fs->make<TTree>("tree","Event Summary");
  tree_->Branch("run", &event_.run);
  tree_->Branch("lumi", &event_.lumi);
  tree_->Branch("event", &event_.event);
  tree_->Branch("bunchCrossing", &event_.bunchCrossing);

  tree_->Branch("jet_p4", &event_.jet_p4);
  tree_->Branch("jet_neutralEmFrac", &event_.jet_neutralEmFrac);
  tree_->Branch("jet_neutralHadFrac", &event_.jet_neutralHadFrac);

  tree_->Branch("electron_p4", &event_.electron_p4);
  tree_->Branch("electron_tight", &event_.electron_tight);

  tree_->Branch("muon_p4", &event_.muon_p4);
  tree_->Branch("muon_tight", &event_.muon_tight);

  tree_->Branch("photon_p4", &event_.photon_p4);
  tree_->Branch("photon_tight", &event_.photon_tight);  

  tree_->Branch("met", &event_.met);
  tree_->Branch("metphi", &event_.metphi);  
  tree_->Branch("calomet", &event_.calomet);  
  tree_->Branch("calometphi", &event_.calometphi);  

  tree_->Branch("flaghbhenoise", &event_.flaghbhenoise);  
  tree_->Branch("flaghbheiso", &event_.flaghbheiso);  
  tree_->Branch("flageebadsc", &event_.flageebadsc);  
  tree_->Branch("flagecaltp", &event_.flagecaltp);  
  tree_->Branch("flaggoodvertices", &event_.flaggoodvertices);  
  tree_->Branch("flagglobaltighthalo", &event_.flagglobaltighthalo);  
  tree_->Branch("flagbadchpf", &event_.flagbadchpf);  
  tree_->Branch("flagbadpfmu", &event_.flagbadpfmu);  
  tree_->Branch("flagbadmuon", &event_.flagbadmuon);  
  tree_->Branch("flagduplicatemuon", &event_.flagduplicatemuon);  

  tree_->Branch("L1EG_bx", &event_.L1EG_bx);
  tree_->Branch("L1EG_p4", &event_.L1EG_p4);
  tree_->Branch("L1EG_iso", &event_.L1EG_iso);
  tree_->Branch("L1Jet_bx", &event_.L1Jet_bx);
  tree_->Branch("L1Jet_p4", &event_.L1Jet_p4);
  tree_->Branch("L1Mu_bx", &event_.L1Mu_bx);
  tree_->Branch("L1Mu_p4", &event_.L1Mu_p4);
  tree_->Branch("L1HT_bx", &event_.L1HT_bx);
  tree_->Branch("L1HT_p4", &event_.L1HT_p4);
  tree_->Branch("L1HTMiss_bx", &event_.L1HTMiss_bx);
  tree_->Branch("L1HTMiss_p4", &event_.L1HTMiss_p4);
  tree_->Branch("L1MET_bx", &event_.L1MET_bx);
  tree_->Branch("L1MET_p4", &event_.L1MET_p4);
  tree_->Branch("L1GtBx", &event_.L1GtBx);
  tree_->Branch("L1EGSeeds_bx",&event_.L1EGSeeds_bx);
  tree_->Branch("L1EGSeeds_name",&event_.L1EGSeeds_name);
  tree_->Branch("L1IsoEGSeeds_bx",&event_.L1IsoEGSeeds_bx);
  tree_->Branch("L1IsoEGSeeds_name",&event_.L1IsoEGSeeds_name);
  tree_->Branch("L1ETMSeeds_bx",&event_.L1ETMSeeds_bx);
  tree_->Branch("L1ETMSeeds_name",&event_.L1ETMSeeds_name);
  
  hltPrescaleProvider.reset(new HLTPrescaleProvider(iConfig,consumesCollector(),*this));


}


PrefiringJetAna::~PrefiringJetAna(){}

void PrefiringJetAna::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup){

  bool changedHLTPSP = false;
  hltPrescaleProvider->init(iRun, iSetup, triggerResultsTag_.process(), changedHLTPSP); //ND   

  HLTConfigProvider fltrConfig;
  bool changedConfig = false;
  fltrConfig.init(iRun, iSetup, filterResultsTag_.process(), changedConfig);
 
  // MET filter Paths
  filterPathsVector.push_back("Flag_HBHENoiseFilter");
  filterPathsVector.push_back("Flag_eeBadScFilter");
  filterPathsVector.push_back("Flag_HBHENoiseIsoFilter");
  filterPathsVector.push_back("Flag_EcalDeadCellTriggerPrimitiveFilter");
  filterPathsVector.push_back("Flag_goodVertices");
  filterPathsVector.push_back("Flag_globalTightHalo2016Filter");
  filterPathsVector.push_back("Flag_badMuons");
  filterPathsVector.push_back("Flag_duplicateMuons");

  for (size_t i = 0; i < filterPathsVector.size(); i++)
    filterPathsMap[filterPathsVector[i]] = -1;

  for(size_t i = 0; i < filterPathsVector.size(); i++){
    TPRegexp pattern(filterPathsVector[i]);
    for(size_t j = 0; j < fltrConfig.triggerNames().size(); j++){
      std::string pathName = fltrConfig.triggerNames()[j];
      if(TString(pathName).Contains(pattern)){
	filterPathsMap[filterPathsVector[i]] = j;
      }
    }
  }
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

  Handle<pat::MuonCollection> muonHandle;
  iEvent.getByToken(muonToken_, muonHandle);

  Handle<pat::ElectronCollection> electronHandle;
  iEvent.getByToken(electronToken_, electronHandle);

  Handle<edm::ValueMap<bool> > vetoElectronIdMapHandle;
  Handle<edm::ValueMap<bool> > tightElectronIdMapHandle;
  iEvent.getByToken(vetoElectronIdMapToken,vetoElectronIdMapHandle);
  iEvent.getByToken(tightElectronIdMapToken,tightElectronIdMapHandle);

  Handle<pat::PhotonCollection> photonHandle;
  iEvent.getByToken(photonToken_, photonHandle);

  Handle<edm::ValueMap<bool> > loosePhotonIdMapHandle;
  Handle<edm::ValueMap<bool> > tightPhotonIdMapHandle;
  iEvent.getByToken(loosePhotonIdMapToken,loosePhotonIdMapHandle);
  iEvent.getByToken(tightPhotonIdMapToken,tightPhotonIdMapHandle);

  Handle<pat::METCollection> metHandle;
  iEvent.getByToken(metToken_,metHandle);

  Handle<pat::METCollection> metOriginalHandle;
  iEvent.getByToken(metOriginalToken_,metOriginalHandle);

  Handle<BXVector<l1t::EGamma>> l1egHandle;
  iEvent.getByToken(l1egToken_, l1egHandle);

  Handle<BXVector<l1t::Jet>> l1jetHandle;
  iEvent.getByToken(l1jetToken_, l1jetHandle);

  Handle<BXVector<l1t::Muon>> l1muHandle;
  iEvent.getByToken(l1muToken_, l1muHandle);

  Handle<BXVector<l1t::EtSum> >  l1etmHandle;
  iEvent.getByToken(l1METToken_,l1etmHandle);
  
  Handle<BXVector<GlobalAlgBlk>> l1GtHandle;
  iEvent.getByToken(l1GtToken_, l1GtHandle);

  Handle<reco::VertexCollection> vertexHandle;
  iEvent.getByToken(vertexToken_, vertexHandle);
  const auto& pv = vertexHandle->at(0);

  edm::Handle<bool> filterBadChCandH;
  iEvent.getByToken(badChargedCandidateToken,filterBadChCandH);      
  edm::Handle<bool> filterBadPFMuonH;
  iEvent.getByToken(badPFMuonToken,filterBadPFMuonH);
   
  // MET filters
  Handle<TriggerResults> filterResultsH;
  iEvent.getByToken(filterResultsToken, filterResultsH);
     
  event_.jet_p4.clear();
  event_.jet_neutralEmFrac.clear();
  event_.jet_neutralHadFrac.clear();
  event_.muon_p4.clear();
  event_.muon_tight.clear();
  event_.electron_p4.clear();
  event_.electron_tight.clear();
  event_.photon_p4.clear();
  event_.photon_tight.clear();
  event_.L1EG_bx.clear();
  event_.L1EG_p4.clear();
  event_.L1EG_iso.clear();
  event_.met = metHandle->front().corPt();
  event_.metphi  = metHandle->front().corPhi();
  event_.calomet = metOriginalHandle->front().caloMETPt();
  event_.calometphi = metOriginalHandle->front().caloMETPhi();

  // MET filter info
  event_.flaghbhenoise = 0; event_.flaghbheiso   = 0;
  event_.flageebadsc   = 0; event_.flagecaltp    = 0; 
  event_.flagglobaltighthalo = 0; event_.flaggoodvertices = 0;
  event_.flagbadchpf = 0; event_.flagbadpfmu = 0;
  event_.flagbadmuon = 0; event_.flagduplicatemuon = 0;

  event_.flagbadchpf = *filterBadChCandH;
  event_.flagbadpfmu = *filterBadPFMuonH;
     
  // Which MET filters passed
  if(filterResultsH.isValid()){
    for (size_t i = 0; i < filterPathsVector.size(); i++) {
      if (filterPathsMap[filterPathsVector[i]] == -1) continue;
      if (filterResultsH->accept(filterPathsMap[filterPathsVector[i]]))
      if (i == 0  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flaghbhenoise = 1; // HBHENoiseFilter
      if (i == 1  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flageebadsc   = 1; // eeBadScFilter
      if (i == 2  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flaghbheiso   = 1; // HBHENoiseIsoFilter
      if (i == 3  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flagecaltp    = 1; // Dead Ecal TP
      if (i == 4  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flaggoodvertices = 1; // Good vertices
      if (i == 5  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flagglobaltighthalo = 1; // 2016 global halo filter
      if (i == 6  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flagbadmuon = 1;
      if (i == 7  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) event_.flagduplicatemuon = 1;
    }
  }
  

  /// dump muon info
  for(size_t iMu=0; iMu<muonHandle->size(); ++iMu) {
    // Writeable so cut string can access new data
    pat::Muon mu(muonHandle->at(iMu));

    mu.addUserFloat("dxy", mu.muonBestTrack()->dxy(pv.position()));
    mu.addUserFloat("dz", mu.muonBestTrack()->dz(pv.position()));

    if(not looseMuonCut_(mu)) continue;    
    event_.muon_p4.push_back(mu.p4());

    if(tightMuonCut_(mu)){
      event_.muon_tight.push_back(1);
    }
    else
      event_.muon_tight.push_back(0);
  }

  /// dump electrons
  for(size_t iEl=0; iEl<electronHandle->size(); ++iEl) {
    // Writeable so cut string can access new data
    pat::Electron el(electronHandle->at(iEl));
    const Ptr<pat::Electron> electronPtr(electronHandle,iEl);

    el.addUserFloat("dxy", el.gsfTrack()->dxy(pv.position()));
    el.addUserFloat("dz", el.gsfTrack()->dz(pv.position()));
    el.addUserInt("passTightID",(*tightElectronIdMapHandle)[electronPtr]);
    el.addUserInt("passVetoID",(*vetoElectronIdMapHandle)[electronPtr]);

    if(not vetoElectronCut_(el)) continue;    
    event_.electron_p4.push_back(el.p4());

    if(tightElectronCut_(el))
      event_.electron_tight.push_back(1);
    else
      event_.electron_tight.push_back(0);
  }

  /// dump photons
  for(size_t iPh=0; iPh<photonHandle->size(); ++iPh) {
    // Writeable so cut string can access new data
    pat::Photon ph(photonHandle->at(iPh));
    const Ptr<pat::Photon> photonPtr(photonHandle,iPh);

    ph.addUserInt("passTightID",(*tightPhotonIdMapHandle)[photonPtr]);
    ph.addUserInt("passLooseID",(*loosePhotonIdMapHandle)[photonPtr]);

    if(not loosePhotonCut_(ph)) continue;    
    event_.photon_p4.push_back(ph.p4());

    if(tightPhotonCut_(ph))
      event_.photon_tight.push_back(1);
    else
      event_.photon_tight.push_back(0);
  }

  /// dump jets
  for(const auto& jet : *jetHandle) {

    pat::Jet jetNew(jet);
    jetNew.addUserInt("looseJetId", looseJetIdSelector_(jetNew, hasLooseId_));
    if ( not tagJetCut_(jetNew) ) continue;

    bool good_jet = true;
    for(auto muon : event_.muon_p4){ if(deltaR(jetNew.eta(),jetNew.phi(),muon.eta(),muon.phi()) < 0.4) good_jet = false; }
    for(auto ele : event_.electron_p4){ if(deltaR(jetNew.eta(),jetNew.phi(),ele.eta(),ele.phi()) < 0.4) good_jet = false; }
    for(auto pho : event_.photon_p4){ if(deltaR(jetNew.eta(),jetNew.phi(),pho.eta(),pho.phi()) < 0.4) good_jet = false; }    
    if(not good_jet) continue;

    event_.jet_p4.push_back( jetNew.p4() );
    event_.jet_neutralEmFrac.push_back( jetNew.neutralEmEnergyFraction() );
    event_.jet_neutralHadFrac.push_back( jetNew.neutralHadronEnergyFraction() );
  }
  
  // dump L1 EG

  for (auto bx=l1egHandle->getFirstBX(); bx<l1egHandle->getLastBX()+1; ++bx) {
    for (auto itL1=l1egHandle->begin(bx); itL1!=l1egHandle->end(bx); ++itL1) {
      event_.L1EG_bx.push_back(bx);
      event_.L1EG_p4.push_back(itL1->p4());
      event_.L1EG_iso.push_back(itL1->hwIso());
    }
  }
      
  // dump L1 Jet
  event_.L1Jet_bx.clear();
  event_.L1Jet_p4.clear();

  for (auto bx=l1jetHandle->getFirstBX(); bx<l1jetHandle->getLastBX()+1; ++bx) {
    for (auto itL1=l1jetHandle->begin(bx); itL1!=l1jetHandle->end(bx); ++itL1) {
      event_.L1Jet_bx.push_back(bx);
      event_.L1Jet_p4.push_back(itL1->p4());
    }
  }

  // dump L1 Mu
  event_.L1Mu_bx.clear();
  event_.L1Mu_p4.clear();

  for (auto bx=l1muHandle->getFirstBX(); bx<l1muHandle->getLastBX()+1; ++bx) {
    for (auto itL1=l1muHandle->begin(bx); itL1!=l1muHandle->end(bx); ++itL1) {
      event_.L1Mu_bx.push_back(bx);
      event_.L1Mu_p4.push_back(itL1->p4());
    }
  }

  // dump L1 Mu
  event_.L1HT_bx.clear();
  event_.L1HT_p4.clear();
  event_.L1HTMiss_bx.clear();
  event_.L1HTMiss_p4.clear();
  event_.L1MET_bx.clear();
  event_.L1MET_p4.clear();

  for (auto bx=l1etmHandle->getFirstBX(); bx<l1etmHandle->getLastBX()+1; ++bx) {
    for (auto itL1=l1etmHandle->begin(bx); itL1!=l1etmHandle->end(bx); ++itL1) {
      int sumType = static_cast<int>(itL1->getType());
      if(sumType == l1t::EtSum::kTotalHt){
	event_.L1HT_bx.push_back(bx);
	event_.L1HT_p4.push_back(itL1->p4());
      }
      else if(sumType == l1t::EtSum::kMissingEt){
	event_.L1MET_bx.push_back(bx);
	event_.L1MET_p4.push_back(itL1->p4());
      }
      else if(sumType == l1t::EtSum::kMissingHt){
	event_.L1HTMiss_bx.push_back(bx);
	event_.L1HTMiss_p4.push_back(itL1->p4());
      }
    }
  }
  
  // Get L1 Menu and utils
  event_.L1EGSeeds_bx.clear(); 
  event_.L1EGSeeds_name.clear(); 
  event_.L1IsoEGSeeds_bx.clear(); 
  event_.L1IsoEGSeeds_name.clear(); 
  event_.L1ETMSeeds_bx.clear(); 
  event_.L1ETMSeeds_name.clear(); 

  edm::ESHandle<L1TUtmTriggerMenu> menu;
  iSetup.get<L1TUtmTriggerMenuRcd>().get(menu);
  int iErrorCode = -1;
  L1GtUtils::TriggerCategory trigCategory = L1GtUtils::AlgorithmTrigger;
  // Get L1 utils for prescales
  L1GtUtils const & l1GtUtils = hltPrescaleProvider->l1GtUtils();
  l1GtUtils.prescaleFactorSetIndex(iEvent, trigCategory, iErrorCode);

  // Get the bit/name association //
  const UInt_t nBits = 512;
  std::string algoBitToName[nBits];
  for (auto const & keyval: menu->getAlgorithmMap()) { 
    std::string const & trigName  = keyval.second.getName(); 
    unsigned int index = keyval.second.getIndex(); 
    if(index < nBits) algoBitToName[index] = trigName;
  } // end algo Map

  event_.L1GtBx.clear();
  for (auto bx=l1GtHandle->getFirstBX(); bx<l1GtHandle->getLastBX()+1; ++bx) {
    event_.L1GtBx.push_back(l1GtHandle->begin(bx)->getFinalOR());
  }

  for(auto bx=l1GtHandle->getFirstBX(); bx<l1GtHandle->getLastBX()+1; ++bx) {
    if(bx > 0) continue;
    if(l1GtHandle->begin(bx)->getFinalOR()){
      // Get the L1 decision per algo //
      GlobalAlgBlk const & result = l1GtHandle->at(bx,0);
      for (unsigned int itrig = 0; itrig < result.maxPhysicsTriggers; ++itrig) {
	// Check decision for this bit
	bool myflag = result.getAlgoDecisionFinal(itrig) ; 
	if(myflag ) { 
	  if(TString(algoBitToName[itrig]).Contains("L1_SingleIsoEG") and not 
	     TString(algoBitToName[itrig]).Contains("er")){
	    event_.L1IsoEGSeeds_bx.push_back(bx);
	    event_.L1IsoEGSeeds_name.push_back(algoBitToName[itrig]);
	  }
	  if(TString(algoBitToName[itrig]).Contains("L1_SingleEG") and not 
	     TString(algoBitToName[itrig]).Contains("er")){
	    event_.L1EGSeeds_bx.push_back(bx);
	    event_.L1EGSeeds_name.push_back(algoBitToName[itrig]);
	  }
	  if(TString(algoBitToName[itrig]).Contains("L1_ETM") and not 
	     TString(algoBitToName[itrig]).Contains("er")){
	    event_.L1ETMSeeds_bx.push_back(bx);
	    event_.L1ETMSeeds_name.push_back(algoBitToName[itrig]);
	  }
	}
      }
    }
  }
  tree_->Fill();
}

void PrefiringJetAna::endRun(const edm::Run &, const edm::EventSetup &) {}

void
PrefiringJetAna::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}


DEFINE_FWK_MODULE(PrefiringJetAna);
