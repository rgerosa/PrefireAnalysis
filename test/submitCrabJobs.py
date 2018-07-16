import os, sys, copy
from multiprocessing import Process
from WMCore.Configuration import Configuration

## multicrab section
if __name__ == '__main__':

    print "################################"
    print "#### Begin multicrab script ####"
    print "################################"

    samples = {}
    SamplesFile = sys.argv[1]
    ConfigFile  = sys.argv[2]

    print "SamplesFile = ", SamplesFile
    print "ConfigFile  = ", ConfigFile
    print "################################"

    command = "from "+ConfigFile.replace('.py','')+" import config"
    exec(command)

    # submit--> if found sample file --> open it
    if os.path.exists(SamplesFile) and not os.path.isdir(SamplesFile) :
        handle = open(SamplesFile,'r')
        exec(handle)
        handle.close()
        # samples to be analysed                   
        for key, value in samples.iteritems():
           print key, ' -> ', value

           config_temp = copy.deepcopy(config);
           config_temp.General.requestName = key
           ## set dataset
           config_temp.Data.inputDataset   = value[0]
           ## set list of python cfg pset parameters extending it
           config_temp.JobType.pyCfgParams.extend(value[1])
           ## redirect to a tmp file
           f = open('cfg_temp.py', 'w')
           print >> f, config_temp 
           f.close()

           os.system("crab submit -c cfg_temp.py")

    os.system("rm cfg_temp.py");
    os.system("rm cfg_temp.pyc");
