import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing()
options.register(
    "is2017",
    True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "2017+ vs 2016 label name for trigger objects (slimmedPatTrigger vs. selectedPatTrigger)"
)
options.parseArguments()

process = cms.Process("TEST")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(2000) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring([
        '/store/data/Run2017F/SingleElectron/MINIAOD/31Mar2018-v1/100000/64244276-8937-E811-AD7C-0CC47A4C8EBA.root',
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
    ]),
)

process.prefireVetoFilter = cms.EDFilter("TriggerRulePrefireVetoFilter",
    tcdsRecordLabel = cms.InputTag("tcdsDigis:tcdsRecord"),
)

process.ntuple = cms.EDAnalyzer("PrefiringZAna",
    electronSrc = cms.InputTag("slimmedElectrons"),
    vertexSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    tagElectronCut = cms.string("pt > 30 && abs(eta) < 2.5 && electronID('cutBasedElectronID-Fall17-94X-V1-medium') && userInt('HLT_Ele32_WPTight_Gsf')"),
    l1egSrc = cms.InputTag("caloStage2Digis:EGamma"),
    triggerObjects = cms.InputTag("slimmedPatTrigger" if options.is2017 else "selectedPatTrigger"),
    triggerPrescales = cms.InputTag("patTrigger"),
)

process.skimPath = cms.Path(process.prefireVetoFilter+process.ntuple)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("ntuple.root"),
    closeFileFast = cms.untracked.bool(True)
)

