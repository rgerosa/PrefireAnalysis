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

#include "DataFormats/L1Trigger/interface/EGamma.h"

#include "TH1D.h"

class L1EGCheck : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
  public:
    explicit L1EGCheck(const edm::ParameterSet&);
    ~L1EGCheck();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    // virtual void beginJob() override;
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    // virtual void endJob() override;

    edm::EDGetTokenT<BXVector<l1t::EGamma>> l1egToken_;

    int bx_;
    TH1D * L1EGPt_;
    TH1D * L1IsoEGPt_;
};

L1EGCheck::L1EGCheck(const edm::ParameterSet& iConfig):
  l1egToken_(consumes<BXVector<l1t::EGamma>>(iConfig.getParameter<edm::InputTag>("l1egSrc"))),
  bx_(iConfig.getParameter<int>("bx"))
{
  usesResource("TFileService");
  edm::Service<TFileService> fs;


  L1EGPt_ = fs->make<TH1D>("L1EGPt",";L1EG Pt;Counts", 250, 0, 250);
  L1IsoEGPt_ = fs->make<TH1D>("L1IsoEGPt",";L1IsoEG Pt;Counts", 250, 0, 250);
}


L1EGCheck::~L1EGCheck()
{
}

void
L1EGCheck::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  Handle<BXVector<l1t::EGamma>> l1egHandle;
  iEvent.getByToken(l1egToken_, l1egHandle);

  for (auto itL1=l1egHandle->begin(bx_); itL1!=l1egHandle->end(bx_); ++itL1) {
    L1EGPt_->Fill(itL1->p4().Pt());
    if ( itL1->hwIso() & 1 ) {
      L1IsoEGPt_->Fill(itL1->p4().Pt());
    }
  }

}


void
L1EGCheck::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}


DEFINE_FWK_MODULE(L1EGCheck);
