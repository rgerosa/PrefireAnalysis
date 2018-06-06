from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.General.workArea = 'crab_prefireZ'
config.General.transferOutputs = True
config.General.transferLogs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'testPrefiringZ_cfg.py'

config.Data.splitting = 'FileBased'
# config.Data.splitting = 'Automatic'
config.Data.inputDBS = 'global'
config.Data.outLFNDirBase = '/store/user/%s/prefiringZ' % (getUsernameFromSiteDB())
config.Data.publication = False
#config.Data.allowNonValidInputDataset = True
config.Data.unitsPerJob = 1
#config.JobType.numCores = 4
config.Data.lumiMask ='/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/ReReco/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt'
config.Data.useParent = True

config.Site.storageSite = 'T2_US_Wisconsin'
#config.Site.storageSite = 'T2_CH_CERN'

pds = [
    '/SingleElectron/Run2017F-31Mar2018-v1/MINIAOD',
    # '/SingleElectron/Run2016H-03Feb2017_ver3-v1/MINIAOD',  # Need proper tag trigger
]

if __name__ == '__main__':
    from CRABAPI.RawCommand import crabCommand
    from CRABClient.ClientExceptions import ClientException
    from httplib import HTTPException

    def submit(config):
        try:
            crabCommand('submit', config = config)
        except HTTPException as hte:
            print "Failed submitting task: %s" % (hte.headers)
        except ClientException as cle:
            print "Failed submitting task: %s" % (cle)

    for i, pd in enumerate(pds):
        (_, primaryDS, conditions, dataTier) = pd.split('/')
        config.General.requestName = 'prefiringZ_%d_%s' % (i, primaryDS)
        config.Data.outputDatasetTag = conditions
        config.Data.inputDataset = pd
        if 'Run2016' in conditions:
            config.JobType.pyCfgParams = ['is2016=1']
        else:
            config.JobType.pyCfgParams = ['is2016=0']
        submit(config)
