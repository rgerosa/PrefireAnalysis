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

#include "DataFormats/TCDS/interface/TCDSRecord.h"


class TriggerRulePrefireVetoFilter : public edm::stream::EDFilter<> {
  public:
    explicit TriggerRulePrefireVetoFilter(const edm::ParameterSet&);
    ~TriggerRulePrefireVetoFilter();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    virtual void beginStream(edm::StreamID) override;
    virtual bool filter(edm::Event&, const edm::EventSetup&) override;
    virtual void endStream() override;


    edm::EDGetTokenT<TCDSRecord> tcdsRecordToken_;
};

TriggerRulePrefireVetoFilter::TriggerRulePrefireVetoFilter(const edm::ParameterSet& iConfig) :
  tcdsRecordToken_(consumes<TCDSRecord>(iConfig.getParameter<edm::InputTag>("tcdsRecordLabel")))
{

}


TriggerRulePrefireVetoFilter::~TriggerRulePrefireVetoFilter()
{
 
}


bool
TriggerRulePrefireVetoFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<TCDSRecord> tcdsRecordH;
  iEvent.getByToken(tcdsRecordToken_, tcdsRecordH);
  const auto& tcdsRecord = *tcdsRecordH.product();

  uint64_t thisEvent = (tcdsRecord.getBXID()-1) + tcdsRecord.getOrbitNr()*3564ull;

  std::vector<uint64_t> eventHistory;
  for (auto&& l1a : tcdsRecord.getFullL1aHistory()) {
    eventHistory.push_back(thisEvent - ((l1a.getBXID()-1) + l1a.getOrbitNr()*3564ull));
  }

  // History should be last 16 according to TCDSRecord.h, we only care about the last 4
  if ( eventHistory.size() < 4 ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Unexpectedly small L1A history from TCDSRecord";
  }
  
  // Since history is sorted, we only need to check that the Nth history item is exactly
  // k bunch crossings ago for the k in N trigger rule, as then BX -1 is excluded and BX 0 allowed

  bool prefireIsVetoed{false};

  // No more than 1 L1A in 3 BX
  if ( eventHistory[0] < 3ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (1 in 3)";
  }
  if ( eventHistory[0] == 3ull ) prefireIsVetoed = true;

  // No more than 2 L1As in 25 BX
  if ( eventHistory[1] < 25ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (2 in 25)";
  }
  if ( eventHistory[1] == 25ull ) prefireIsVetoed = true;

  // No more than 3 L1As in 100 BX
  if ( eventHistory[2] < 100ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (3 in 100)";
  }
  if ( eventHistory[2] == 100ull ) prefireIsVetoed = true;

  // No more than 4 L1As in 240 BX
  if ( eventHistory[3] < 240ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (4 in 240)";
  }
  if ( eventHistory[3] == 240ull ) prefireIsVetoed = true;

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
