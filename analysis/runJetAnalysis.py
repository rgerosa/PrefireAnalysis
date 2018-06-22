#!/usr/bin/env python
import ROOT
import sys
import os

def mkdirp(d):
    if not os.path.exists(d):
        os.makedirs(d)


ROOT.gStyle.SetOptDate(0)
ROOT.gStyle.SetHistLineWidth(2)
ROOT.gStyle.SetPalette(ROOT.kBird)
ROOT.gStyle.SetNumberContours(100)
ROOT.gStyle.SetPadRightMargin(0.15)
ROOT.gStyle.SetCanvasDefW(int(600*(1.1)))

# era = "2016H"
era = sys.argv[1]
hdr = "JetHT Run%s" % era
# hdr = "SingleMuon Run%s" % era
f = ROOT.TFile.Open("prefiringJet_Run%s.root" % era)
#f = ROOT.TFile.Open("prefiringJet_SingleMuon_Run%s.root" % era)
t = f.Get("ntuple/tree")

print t.GetEntries()

ROOT.gROOT.ProcessLineSync(".L JetAnalysis.C+")

sel = ROOT.JetAnalysis()
t.Process(sel)
hists = dict((h.GetName(), h) for h in sel.GetOutputList())


def header(text):
    l = ROOT.TLatex()
    ROOT.SetOwnership(l, False)
    l.SetNDC()
    l.SetTextSize(0.7*ROOT.gPad.GetTopMargin())
    l.SetTextFont(42)
    l.SetY(1-0.9*ROOT.gPad.GetTopMargin())
    l.SetTextAlign(31)
    l.SetX(1-ROOT.gPad.GetRightMargin())
    l.SetTitle(text)
    l.Draw()


bx = [(-2, 'bxm2'), (-1, 'bxm1'), (0, 'bx0'), (1, 'bx1'), (2, 'bx2')]


mkdirp("L1EGeff")
mkdirp("L1EGPosition")
mkdirp("JetPtFinOReff")

for ibx, bxn in bx:
    c = ROOT.TCanvas()
    eff = ROOT.TEfficiency(hists['num_'+bxn], hists['denom'])
    eff.Draw("colz")
    c.Paint()
    eff.GetPaintedHistogram().GetZaxis().SetTitle("L1IsoEG30 in BX %d Efficiency (#DeltaR<0.4)" % ibx)
    eff.GetPaintedHistogram().GetZaxis().SetRangeUser(0, 1)
    header(hdr)
    c.Print("L1EGeff/Jet_L1IsoEG30eff_%s_looseJet_%s.pdf" % (bxn, era))
    c.Print("L1EGeff/Jet_L1IsoEG30eff_%s_looseJet_%s.root" % (bxn, era))
    ROOT.SetOwnership(eff, False)
    ROOT.SetOwnership(c, False)

finorm1 = None
for ibx, bxn in bx:
    c2 = ROOT.TCanvas()
    eff2 = ROOT.TEfficiency(hists['numL1A_'+bxn], hists['denomL1A'])
    if ibx == -1:
        finorm1 = eff2.CreateGraph()
    eff2.GetTotalHistogram().GetYaxis().SetTitle("FinOR in BX %d Efficiency" % ibx)
    eff2.Draw("ap")
    header(hdr)
    l = ROOT.TLegend(0.2, 0.8, 0.5, 0.9)
    l.AddEntry(eff2, "Single jet 2.5 #leq |#eta^{jet}| < 3", "pe")
    l.Draw()
    c2.Paint()
    eff2.GetPaintedGraph().GetYaxis().SetRangeUser(0, 1)
    c2.Print("JetPtFinOReff/FinOReff_%s_looseJetEta2p5to3p0_%s.pdf" % (bxn, era))
    c2.Print("JetPtFinOReff/FinOReff_%s_looseJetEta2p5to3p0_%s.root" % (bxn, era))
    ROOT.SetOwnership(eff2, False)
    ROOT.SetOwnership(c2, False)
    ROOT.SetOwnership(l, False)


c2 = ROOT.TCanvas()
mg = ROOT.TMultiGraph("mgeffthr", ";Jet p_{T}^{EM} [GeV];BX -1 Trigger Efficiency")
efflow = ROOT.TEfficiency(hists['numJetEGthr_eglow'], hists['denomJetEGthr']).CreateGraph()
efflow.SetMarkerColor(ROOT.kRed)
mg.Add(efflow, "p")
effmed = ROOT.TEfficiency(hists['numJetEGthr_egmed'], hists['denomJetEGthr']).CreateGraph()
effmed.SetMarkerColor(ROOT.kBlue)
mg.Add(effmed, "p")
effhigh = ROOT.TEfficiency(hists['numJetEGthr_eghigh'], hists['denomJetEGthr']).CreateGraph()
effhigh.SetMarkerColor(ROOT.kGreen)
mg.Add(effhigh, "p")
mg.Add(finorm1, "p")
mg.Draw("a")
mg.GetYaxis().SetRangeUser(0, 1)
header(hdr)
l = ROOT.TLegend(0.2, 0.8, 0.5, 0.9)
l.AddEntry(efflow, "L1IsoEG20", "pe")
l.AddEntry(effmed, "L1IsoEG30", "pe")
l.AddEntry(effhigh, "L1IsoEG40", "pe")
l.AddEntry(finorm1, "FinOR", "pe")
l.Draw()
c2.Print("L1EGeff/L1EGthrEff_looseJetEta2p5to3p0_%s.pdf" % (era, ))
c2.Print("L1EGeff/L1EGthrEff_looseJetEta2p5to3p0_%s.root" % (era, ))
ROOT.SetOwnership(c2, False)
ROOT.SetOwnership(l, False)

c3 = ROOT.TCanvas()
egmap = hists['jet30EGEtaPhi']
egmap.Draw("colz")
header(hdr)
c3.Paint()
c3.GetFrame().SetFillStyle(1001)
c3.GetFrame().SetFillColor(ROOT.kBlack)
c3.Print("L1EGPosition/Jet30_L1EGbxm1Position_%s.pdf" % (era, ))
c3.Print("L1EGPosition/Jet30_L1EGbxm1Position_%s.root" % (era, ))
