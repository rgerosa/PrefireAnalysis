import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing

process = cms.Process("TEST")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(2000) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring([
        '/store/data/Run2017F/JetHT/MINIAOD/31Mar2018-v1/30000/54B9283C-C637-E811-A002-B496910A0554.root',
    ]),
    secondaryFileNames = cms.untracked.vstring([
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70006/E2AD6AFB-BDDF-E711-A932-0025905A6104.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/50000/46F6F25F-CBDE-E711-8525-02163E014624.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/50000/E62A295A-CBDE-E711-9F6B-02163E0118C5.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/50001/06999498-79DF-E711-9E0D-02163E01364E.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/50002/08E8F29F-AEDF-E711-B155-02163E01348C.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/50002/BE7A8FBE-B4DF-E711-824F-02163E01A46F.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/50002/E44994FE-BBDF-E711-818E-02163E0136F5.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70002/24B20F66-D6DF-E711-871A-008CFA1C6564.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70002/D4287C60-D5DF-E711-B716-A0369FD8FDB0.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70003/8E2E748B-1FE0-E711-9608-008CFAFC53C6.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70003/A2DA6C02-2CE0-E711-A436-A0369FC5D5A4.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70003/B2B65D7E-05E0-E711-A5A0-02163E012509.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70003/D4080733-06E0-E711-8E1D-02163E0129F1.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70004/CE523E1C-8DE0-E711-A6AC-008CFA197D38.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70004/DC645294-27E0-E711-8C0C-3417EBE64567.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70006/5260FD8C-BDDF-E711-BA7D-0CC47A7C34EE.root',
        '/store/data/Run2017F/JetHT/AOD/17Nov2017-v1/70006/FAFE3491-BDDF-E711-A5FC-0025905A6134.root',
    ]),
)

process.prefireVetoFilter = cms.EDFilter("TriggerRulePrefireVetoFilter",
    tcdsRecordLabel = cms.InputTag("tcdsDigis:tcdsRecord"),
)

process.ntuple = cms.EDAnalyzer("PrefiringJetAna",
    jetSrc = cms.InputTag("slimmedJets"),
    tagJetCut = cms.string("pt > 30 && userInt('looseJetId')"),
    l1egSrc = cms.InputTag("caloStage2Digis:EGamma"),
    l1GtSrc = cms.InputTag("gtStage2Digis"),
)

process.skimPath = cms.Path(process.prefireVetoFilter+process.ntuple)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("ntuple.root"),
    closeFileFast = cms.untracked.bool(True)
)

