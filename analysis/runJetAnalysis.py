#!/usr/bin/env python
import ROOT
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

era = "2017F"
hdr = "JetHT Run%s" % era
# hdr = "SingleMuon Run%s" % era
f = ROOT.TFile.Open("prefiringJet_Run%s.root" % era)
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

doEGeff = True
doEGthrEff = True
doFinOReff = True
doEGMap = True

mkdirp("L1EGeff_dev")
mkdirp("JetPtFinOReff_dev")

if doEGeff:
    for ibx, bxn in bx:
        c = ROOT.TCanvas()
        eff = ROOT.TEfficiency(hists['num_'+bxn], hists['denom'])
        eff.Draw("colz")
        c.Paint()
        eff.GetPaintedHistogram().GetZaxis().SetTitle("L1IsoEG30 in BX %d Efficiency (#DeltaR<0.4)" % ibx)
        eff.GetPaintedHistogram().GetZaxis().SetRangeUser(0, 1)
        header(hdr)
        c.Print("L1EGeff_dev/Jet_L1IsoEG30eff_%s_looseJet_%s.pdf" % (bxn, era))
        c.Print("L1EGeff_dev/Jet_L1IsoEG30eff_%s_looseJet_%s.root" % (bxn, era))
        ROOT.SetOwnership(eff, False)
        ROOT.SetOwnership(c, False)

if doEGthrEff:
    c2 = ROOT.TCanvas()
    mg = ROOT.TMultiGraph("mgeffthr", ";Jet p_{T} [GeV];L1EG in BX -1 Efficiency (#DeltaR<0.4)")
    eff22 = ROOT.TEfficiency(hists['numJetEGthr_eg22'], hists['denomJetEGthr']).CreateGraph()
    mg.Add(eff22, "p")
    eff26 = ROOT.TEfficiency(hists['numJetEGthr_eg26'], hists['denomJetEGthr']).CreateGraph()
    eff26.SetMarkerColor(ROOT.kBlue)
    mg.Add(eff26, "p")
    eff30 = ROOT.TEfficiency(hists['numJetEGthr_eg30'], hists['denomJetEGthr']).CreateGraph()
    eff30.SetMarkerColor(ROOT.kGreen)
    mg.Add(eff30, "p")
    mg.Draw("a")
    mg.GetYaxis().SetRangeUser(0, 1)
    header(hdr)
    l = ROOT.TLegend(0.2, 0.8, 0.5, 0.9)
    l.AddEntry(eff22, "L1IsoEG22", "pe")
    l.AddEntry(eff26, "L1IsoEG26", "pe")
    l.AddEntry(eff30, "L1IsoEG30", "pe")
    l.Draw()
    c2.Print("L1EGeff_dev/L1EGthrEff_looseJetEta2p5to3p0_%s.pdf" % (era, ))
    c2.Print("L1EGeff_dev/L1EGthrEff_looseJetEta2p5to3p0_%s.root" % (era, ))
    ROOT.SetOwnership(c2, False)
    ROOT.SetOwnership(l, False)

if doFinOReff:
    for ibx, bxn in bx:
        c2 = ROOT.TCanvas()
        eff2 = ROOT.TEfficiency(hists['numL1A_'+bxn], hists['denomL1A'])
        eff2.GetTotalHistogram().GetYaxis().SetTitle("FinOR in BX %d Efficiency" % ibx)
        eff2.Draw("ap")
        header(hdr)
        l = ROOT.TLegend(0.2, 0.8, 0.5, 0.9)
        l.AddEntry(eff2, "Single jet 2.5 #leq |#eta^{jet}| < 3", "pe")
        l.Draw()
        c2.Paint()
        eff2.GetPaintedGraph().GetYaxis().SetRangeUser(0, 1)
        c2.Print("JetPtFinOReff_dev/FinOReff_%s_looseJetEta2p5to3p0_%s.pdf" % (bxn, era))
        c2.Print("JetPtFinOReff_dev/FinOReff_%s_looseJetEta2p5to3p0_%s.root" % (bxn, era))
        ROOT.SetOwnership(eff2, False)
        ROOT.SetOwnership(c2, False)
        ROOT.SetOwnership(l, False)

if doEGMap:
    c3 = ROOT.TCanvas()
    egmap = hists['jet100EGEtaPhi']
    egmap.Draw("colz")
    header(hdr)
    c3.Paint()
    c3.GetFrame().SetFillStyle(1001)
    c3.GetFrame().SetFillColor(ROOT.kBlack)
    c3.Print("Jet100_L1EGbxm1Position_%s.pdf" % (era, ))
    c3.Print("Jet100_L1EGbxm1Position_%s.root" % (era, ))
