import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')
## GT to be used
options.register (
    'globalTag','80X_dataRun2_2016SeptRepro_v7',VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'gloabl tag to be uses');

options.parseArguments()

process = cms.Process("TEST")

process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(True),
    wantSummary = cms.untracked.bool(True),
)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 500

process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring([
        '/store/data/Run2016H/JetHT/MINIAOD/03Feb2017_ver3-v1/100000/C8E4E1EC-97EA-E611-BC0D-001E67792738.root',
    ]),
    secondaryFileNames = cms.untracked.vstring([
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/038/00000/E45D54F1-779F-E611-A271-02163E011CBF.root',
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/039/00000/DE1B6E0E-759F-E611-865D-02163E014719.root',
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/042/00000/E2E0BED8-6D9F-E611-825B-FA163E88EBC7.root',
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/043/00000/0433CA2C-9F9F-E611-AE55-02163E012007.root',
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/043/00000/4EDD51A4-CE9F-E611-9E9D-02163E014310.root',
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/043/00000/566A0223-C49F-E611-BF3B-02163E01230B.root',
        '/store/data/Run2016H/JetHT/AOD/PromptReco-v3/000/284/043/00000/88BE1171-C79F-E611-95CF-02163E01352A.root',
    ]),
)

### global-tag 
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag

from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import updatedPatJetCorrFactors,updatedPatJets
from PhysicsTools.PatAlgos.recoLayer0.jetCorrFactors_cfi import patJetCorrFactors
process.JECLevels = cms.PSet(labels = cms.vstring('L1FastJet', 'L2Relative', 'L3Absolute', 'L2L3Residual'))
process.patJetCorrFactorsReapplyJEC = updatedPatJetCorrFactors.clone(
    src     = cms.InputTag("slimmedJets"),
    levels  = process.JECLevels.labels,
    payload = "AK4PFchs");
process.slimmedJetsRecorrected = updatedPatJets.clone(
    jetSource = cms.InputTag("slimmedJets"),
    jetCorrFactorsSource = cms.VInputTag(cms.InputTag("patJetCorrFactorsReapplyJEC")))

### propagate to the MET
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
runMetCorAndUncFromMiniAOD(process,
                           isData=True
                           )

from PhysicsTools.PatUtils.tools.corMETFromMuonAndEG import corMETFromMuonAndEG
corMETFromMuonAndEG(process,pfCandCollection="",electronCollection="slimmedElectronsBeforeGSFix",photonCollection="slimmedPhotonsBeforeGSFix",
                        corElectronCollection="slimmedElectrons", corPhotonCollection="slimmedPhotons",
                        allMETEGCorrected=True,muCorrection=False,eGCorrection=True,runOnMiniAOD=True,postfix="MuEGClean")

process.slimmedMETsMuEGClean = process.slimmedMETs.clone()
process.slimmedMETsMuEGClean.src = cms.InputTag("patPFMetT1MuEGClean")
process.slimmedMETsMuEGClean.rawVariation =  cms.InputTag("patPFMetRawMuEGClean")
process.slimmedMETsMuEGClean.t1Uncertainties = cms.InputTag("patPFMetT1%sMuEGClean")
del process.slimmedMETsMuEGClean.caloMET

### Electron and photon IDs
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
dataFormat = DataFormat.MiniAOD
switchOnVIDElectronIdProducer(process, dataFormat);
ele_id_modules = [];
ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff');
for idmod in ele_id_modules:
    setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)

switchOnVIDPhotonIdProducer(process, dataFormat);
ph_id_modules = []
ph_id_modules.append('RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring16_V2p2_cff')

for idmod in ph_id_modules:
    setupAllVIDIdsInModule(process,idmod,setupVIDPhotonSelection)

### GoodVertex
process.goodVertices = cms.EDFilter("VertexSelector",
  src = cms.InputTag("offlineSlimmedPrimaryVertices"),
  cut = cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.Rho <= 2"),
  filter = cms.bool(True))

## Bad muon filter
process.load('RecoMET.METFilters.BadPFMuonFilter_cfi')
process.BadPFMuonFilter.muons = cms.InputTag("slimmedMuons")
process.BadPFMuonFilter.PFCandidates = cms.InputTag("packedPFCandidates")
process.BadPFMuonFilter.taggingMode = cms.bool(True) ## in order not to reject events but just keep collection

##Bad charged tracks
process.load("RecoMET.METFilters.BadChargedCandidateFilter_cfi")
process.BadChargedCandidateFilter.muons = cms.InputTag("slimmedMuons")
process.BadChargedCandidateFilter.PFCandidates = cms.InputTag("packedPFCandidates")
process.BadChargedCandidateFilter.taggingMode = cms.bool(True) ## in order not to reject events but just keep collection

process.metFilters = cms.Sequence(process.goodVertices*process.BadPFMuonFilter*process.BadChargedCandidateFilter)

### select un-prefirable events
process.prefireVetoFilter = cms.EDFilter("TriggerRulePrefireVetoFilter",
    l1AcceptRecordLabel = cms.InputTag("scalersRawToDigi"),
)


process.ntuple = cms.EDAnalyzer("PrefiringJetAna",
    #### MET
    metSrc =  cms.InputTag("slimmedMETsMuEGClean","","TEST"),                              
    metOriginalSrc = cms.InputTag("slimmedMETsUncorrected"),                                
    #### Vertex
    vertexSrc   = cms.InputTag("goodVertices"),
    ### muon collection and selection
    muonSrc    = cms.InputTag("slimmedMuons"),
    looseMuonCut = cms.string("pt > 10 && abs(eta) < 2.4 && isPFMuon && (isTrackerMuon || isGlobalMuon) && (pfIsolationR04().sumChargedHadronPt + max(pfIsolationR04().sumNeutralHadronEt + pfIsolationR04().sumPhotonEt - pfIsolationR04().sumPUPt/2.,0.0))/pt < 0.25"),
    tightMuonCut = cms.string("pt > 10 && abs(eta) < 2.4 && isGlobalMuon && isPFMuon && innerTrack.hitPattern.numberOfValidPixelHits > 0 && innerTrack.hitPattern.trackerLayersWithMeasurement > 5 && (pfIsolationR04().sumChargedHadronPt + max(pfIsolationR04().sumNeutralHadronEt + pfIsolationR04().sumPhotonEt - pfIsolationR04().sumPUPt/2.,0.0))/pt < 0.15 && globalTrack.hitPattern.numberOfValidMuonHits > 0 && globalTrack.normalizedChi2 < 10 && numberOfMatches > 1 && abs(userFloat('dxy')) < 0.5 && abs(userFloat('dz')) < 0.5"),                                
    ### electron collection and selections                                
    electronSrc = cms.InputTag("slimmedElectrons"),                                
    tightElectronCut = cms.string("pt > 10 && abs(eta) < 2.5 && userInt('passTightID') && ((abs(eta) < 1.5 && abs(userFloat('dxy')) < 0.05 && abs(userFloat('dz')) < 0.10) || (abs(eta) > 1.5 && abs(userFloat('dxy')) < 0.1 && abs(userFloat('dz')) < 0.2))"),                                       
    vetoElectronCut = cms.string("pt > 10 && abs(eta) < 2.5 && userInt('passVetoID') && ((abs(eta) < 1.5 && abs(userFloat('dxy')) < 0.05 && abs(userFloat('dz')) < 0.10) || (abs(eta) > 1.5 && abs(userFloat('dxy')) < 0.1 && abs(userFloat('dz')) < 0.2))"),                                
    vetoElectronValueMap = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-veto"),
    tightElectronValueMap = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-tight"),                                
    #### photon collection and selection
    photonSrc  = cms.InputTag("slimmedPhotons"),
    loosePhotonCut = cms.string("pt > 15 && abs(eta) < 2.5 && userInt('passLooseID') && passElectronVeto()"),                                
    tightPhotonCut = cms.string("pt > 15 && abs(eta) < 2.5 && userInt('passTightID') && passElectronVeto()"),                                
    loosePhotonValueMap = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-loose"),
    tightPhotonValueMap = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-tight"),                                
    #### jets 
    jetSrc    = cms.InputTag("slimmedJetsRecorrected"),
    tagJetCut = cms.string("pt > 30 && userInt('looseJetId') && abs(eta) < 4.7"),
    #### L1 objects
    l1egSrc  = cms.InputTag("caloStage2Digis:EGamma"),
    l1jetSrc = cms.InputTag("caloStage2Digis:Jet"),
    l1muSrc  = cms.InputTag("gmtStage2Digis:Muon"),
    l1METSrc = cms.InputTag("caloStage2Digis:EtSum"),
    l1GtSrc  = cms.InputTag("gtStage2Digis"),
    ####                                
    triggerResults = cms.InputTag("TriggerResults", "","HLT"),
    filterResults  = cms.InputTag("TriggerResults", "","RECO"),                                
    badChargedCandidate = cms.InputTag("BadChargedCandidateFilter"),
    badPFMuon           = cms.InputTag("BadPFMuonFilter"),
                                
)


process.bxm1_pass = cms.EDAnalyzer("L1EGCheck",
    l1egSrc = cms.InputTag("caloStage2Digis:EGamma"),
    bx = cms.int32(-1),
)

process.bxm1_fail = cms.EDAnalyzer("L1EGCheck",
    l1egSrc = cms.InputTag("caloStage2Digis:EGamma"),
    bx = cms.int32(-1),
)

### fix the task
process.edTask = cms.Task()
for key in process.__dict__.keys():
    if(type(getattr(process,key)).__name__=='EDFilter' or type(getattr(process,key)).__name__=='EDProducer') :
        process.edTask.add(getattr(process,key))
process.taskSeq = cms.Sequence(process.edTask)

process.skimPath = cms.Path(
    process.egmGsfElectronIDSequence+
    process.egmPhotonIDSequence+
    process.metFilters+
    process.taskSeq+
    process.prefireVetoFilter+
    process.bxm1_pass+
    process.ntuple)

process.checkPath = cms.Path(~process.prefireVetoFilter+
                              process.bxm1_fail)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("ntuple.root"),
    closeFileFast = cms.untracked.bool(True)
)

processDumpFile = open('processDump.py', 'w')
print >> processDumpFile, process.dumpPython()
