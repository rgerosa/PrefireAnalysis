import os, sys
from multiprocessing import Process
from WMCore.Configuration import Configuration

#### Basic crab config structure
config = Configuration()

pyCfgParams = []

config.section_('General')
config.General.transferLogs = False
config.General.workArea     = 'crab_projects_DATA_v4'  # Make sure you set this parameter

config.section_('JobType')
config.JobType.psetName         = 'testPrefiringJet_cfg.py'
config.JobType.pluginName       = 'Analysis'
config.JobType.outputFiles      = ['ntuple.root']


config.section_('Data')    
config.Data.inputDBS      = 'global'
config.Data.splitting     = 'FileBased'
config.Data.outLFNDirBase = '/store/group/phys_higgs/HiggsExo/rgerosa/PrefiringStudies_v4/'
config.Data.lumiMask      = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/ReReco/Final/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt'
config.Data.publication   = False
config.Data.unitsPerJob = 1
config.Data.useParent = True

config.section_('Site')
config.Site.storageSite = 'T2_CH_CERN'

config.JobType.pyCfgParams = list(pyCfgParams)
