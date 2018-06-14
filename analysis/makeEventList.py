#!/usr/bin/env python
import ROOT
import sys

if len(sys.argv) != 3:
    print "Usage: %s ntuple.root outputlist.root" % sys.argv[0]
    exit(0)


fin = ROOT.TFile.Open(sys.argv[1])
tin = fin.Get("ntuple/tree")

tin.SetBranchStatus("*", 0)
tin.SetBranchStatus("run", 1)
tin.SetBranchStatus("lumi", 1)
tin.SetBranchStatus("event", 1)

fout = ROOT.TFile(sys.argv[2], "recreate")
tout = tin.CloneTree(-1, "fast")
tout.Write()
