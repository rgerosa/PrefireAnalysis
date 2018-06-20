import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing()
options.register(
    "is2016",
    True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "2016 vs 2017+ label name for trigger objects (selectedPatTrigger vs. slimmedPatTrigger)"
)
options.parseArguments()

process = cms.Process("TEST")

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring([
        '/store/data/Run2017F/SingleElectron/MINIAOD/31Mar2018-v1/100000/64244276-8937-E811-AD7C-0CC47A4C8EBA.root',
        #'/store/data/Run2016H/SingleElectron/MINIAOD/03Feb2017_ver3-v1/110000/2291F6A5-62EC-E611-A658-24BE05C63681.root',
    ]),
    secondaryFileNames = cms.untracked.vstring([
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/60002/D21DD65E-57E0-E711-A3B1-FA163E42C814.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/60002/FAE9BF62-57E0-E711-A80C-FA163E714FAE.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/04F04F9C-82E0-E711-8076-B083FED42A19.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/087E6FA3-79E0-E711-B1A6-141877343E6D.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/0C049678-76E0-E711-8CBF-001EC94BF6C8.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/20BD29AE-DFE0-E711-A962-0025905A48FC.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/7265E7AF-62E0-E711-BE0C-001EC94BF718.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/8A159106-77E0-E711-AB06-1866DAEA6520.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/A8D12096-82E0-E711-82FA-D4AE526567E0.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/B47537BF-79E0-E711-940B-842B2B17E3B8.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/BE2B461A-E6E0-E711-8B42-0CC47A4D760A.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50000/D2DEFF6F-E2E0-E711-8921-0025905B85B2.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/14805072-6BE0-E711-9323-1866DAEEB0A8.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/641E51D7-62E0-E711-B386-B083FED424C3.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/7E2E8077-69E0-E711-BA3F-1866DAEA6E00.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/9E948648-7AE0-E711-B455-782BCB20BDE3.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/A47C5B15-86E0-E711-93BC-1866DAEA79D4.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/B8E4ABA8-62E0-E711-AAC2-1866DAEA79D4.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/50001/E0D43C81-72E0-E711-A778-B083FED045EC.root',
        '/store/data/Run2017F/SingleElectron/AOD/17Nov2017-v1/60003/C04573A5-76E0-E711-BC32-FA163EBA1423.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/0C710287-5D9F-E611-8E10-02163E014514.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/181CB054-5D9F-E611-BDB3-02163E013940.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/2AE4BE84-5D9F-E611-9427-02163E014322.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/66742075-5D9F-E611-A3B9-FA163E963616.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/6AE34177-5D9F-E611-BF8B-FA163E88B627.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/7CB4F959-5D9F-E611-850F-FA163E9141E8.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/AE0FAC65-5D9F-E611-9B27-FA163EB16BB1.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/C09D6D8B-5D9F-E611-94F2-02163E014310.root',
        #'/store/data/Run2016H/SingleElectron/AOD/PromptReco-v3/000/284/036/00000/E636E86A-5D9F-E611-A60C-FA163E5BB86E.root',
    ]),
)

process.prefireVetoFilter = cms.EDFilter("TriggerRulePrefireVetoFilter",
    l1AcceptRecordLabel = cms.InputTag("scalersRawToDigi"),
)

# Latest 2016 is cutBasedElectronID-Summer16-80X-V1-medium but close enough
eleID = 'cutBasedElectronID-Spring15-25ns-V1-standalone-medium' if options.is2016 else 'cutBasedElectronID-Fall17-94X-V1-medium'

process.ntuple = cms.EDAnalyzer("PrefiringZAna",
    triggerRule = cms.InputTag("prefireVetoFilter:ruleIndex"),
    electronSrc = cms.InputTag("slimmedElectrons"),
    photonSrc = cms.InputTag("gedPhotons::RECO"),
    vertexSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    tagElectronCut = cms.string("pt > 25 && abs(eta) < 2.5 && electronID('%s') && userInt('pass%strigger')" % (eleID, '2016' if options.is2016 else '2017')),
    l1egSrc = cms.InputTag("caloStage2Digis:EGamma"),
    triggerObjects = cms.InputTag("selectedPatTrigger" if options.is2016 else "slimmedPatTrigger"),
    triggerPrescales = cms.InputTag("patTrigger"),
)

process.skimPath = cms.Path(process.prefireVetoFilter+process.ntuple)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("ntuple.root"),
    closeFileFast = cms.untracked.bool(True)
)

