// -*- C++ -*-
//
// Package:    L1Trigger/PrefireAnalysis
// Class:      TriggerRulePrefireVetoFilter
// 
/**\class TriggerRulePrefireVetoFilter TriggerRulePrefireVetoFilter.cc L1Trigger/PrefireAnalysis/plugins/TriggerRulePrefireVetoFilter.cc

 Description: Filters events that have an L1A history such that L1As in the previous bunch crossing were not possible due to trigger rules.

*/
//
// Original Author:  Nicholas Charles Smith
//         Created:  Mon, 28 May 2018 15:39:05 GMT
//
//


#include <memory>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/Scalers/interface/L1AcceptBunchCrossing.h"


class TriggerRulePrefireVetoFilter : public edm::stream::EDFilter<> {
  public:
    explicit TriggerRulePrefireVetoFilter(const edm::ParameterSet&);
    ~TriggerRulePrefireVetoFilter();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    virtual void beginStream(edm::StreamID) override;
    virtual bool filter(edm::Event&, const edm::EventSetup&) override;
    virtual void endStream() override;


    edm::EDGetTokenT<L1AcceptBunchCrossingCollection> l1AcceptRecordToken_;
};

TriggerRulePrefireVetoFilter::TriggerRulePrefireVetoFilter(const edm::ParameterSet& iConfig) :
  l1AcceptRecordToken_(consumes<L1AcceptBunchCrossingCollection>(iConfig.getParameter<edm::InputTag>("l1AcceptRecordLabel")))
{
  produces<int>("ruleIndex");

}


TriggerRulePrefireVetoFilter::~TriggerRulePrefireVetoFilter()
{
 
}


bool
TriggerRulePrefireVetoFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<L1AcceptBunchCrossingCollection> l1AcceptRecordH;
  iEvent.getByToken(l1AcceptRecordToken_, l1AcceptRecordH);
  const auto& l1AcceptRecord = *l1AcceptRecordH.product();

  // History should contain this bx + last 4 at least
  if ( l1AcceptRecord.size() < 5 ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Unexpectedly small L1A history from l1AcceptRecord";
  }
  
  unsigned int thisEvent = l1AcceptRecord[0].bunchCrossing();
  unsigned int thisOrbit = l1AcceptRecord[0].orbitNumber();

  std::vector<unsigned int> eventHistory;
  for (size_t i=1; i<5; ++i) {
    if ( thisOrbit == l1AcceptRecord[i].orbitNumber() ) {
      eventHistory.push_back(thisEvent - l1AcceptRecord[i].bunchCrossing());
    } else {
      // It appears the trigger rules are reset each BC0
      // So ignore events crossing orbit boundary (only really applicable to 4 in 240 due to abort gap)
      eventHistory.push_back(9999);
    }
  }

  // Since history is sorted, we only need to check that the kth history item is exactly
  // N bunch crossings ago for the k in N trigger rule, as then BX -1 is excluded and BX 0 allowed

  bool prefireIsVetoed{false};
  std::unique_ptr<int> ruleIndex(new int{0});

  // No more than 1 L1A in 3 BX
  if ( eventHistory[0] < 3u ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (1 in 3)";
  }
  if ( eventHistory[0] == 3u ) {
    prefireIsVetoed = true;
    *ruleIndex = 1;
  }

  // No more than 2 L1As in 25 BX
  if ( eventHistory[1] < 25u ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (2 in 25)";
  }
  if ( eventHistory[1] == 25u ) {
    prefireIsVetoed = true;
    *ruleIndex = 2;
  }

  // No more than 3 L1As in 100 BX
  if ( eventHistory[2] < 100u ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (3 in 100)";
  }
  if ( eventHistory[2] == 100u ) {
    prefireIsVetoed = true;
    *ruleIndex = 3;
  }

  // No more than 4 L1As in 240 BX
  if ( eventHistory[3] < 240u ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (4 in 240)";
  }
  if ( eventHistory[3] == 240u ) {
    prefireIsVetoed = true;
    *ruleIndex = 4;
  }

  iEvent.put(std::move(ruleIndex), "ruleIndex");
  return prefireIsVetoed;
}

void
TriggerRulePrefireVetoFilter::beginStream(edm::StreamID)
{
}

void
TriggerRulePrefireVetoFilter::endStream() {
}

void
TriggerRulePrefireVetoFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(TriggerRulePrefireVetoFilter);
