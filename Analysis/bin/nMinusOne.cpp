#include "InvisibleHiggs/Analysis/interface/ProgramOptions.h"
#include "InvisibleHiggs/Analysis/interface/Cuts.h"
#include "InvisibleHiggs/Analysis/interface/Histogrammer.h"
#include "InvisibleHiggs/Analysis/interface/StackPlot.h"
#include "InvisibleHiggs/Analysis/interface/SumDatasets.h"
#include "InvisibleHiggs/Analysis/interface/Datasets.h"
#include "InvisibleHiggs/Analysis/interface/Constants.h"

// #include "TROOT.h"
#include "TTree.h"
#include "TMath.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

int main(int argc, char* argv[]) {

  TH1::SetDefaultSumw2();

  ProgramOptions options(argc, argv);

  double lumi = options.lumi;
  std::string oDir = options.oDir+std::string("/NMinusOne");

  std::cout << "Integrated luminosity : " << lumi << " pb-1" << std::endl;

  boost::filesystem::path opath(oDir);
  if (!exists(opath)) {
    std::cout << "Creating output directory : " << oDir << std::endl;
    boost::filesystem::create_directory(opath);
  }
  else std::cout << "Writing results to " << oDir << std::endl;

  Datasets datasets(options.iDir);
  datasets.readFile(options.datasetFile);

  // cuts
  Cuts cuts;
  TCut puWeight("puWeight");
  //TCut trigCorr("trigCorrWeight");
  TCut trigCorr("(trigCorrWeight>0) ? trigCorrWeight : 1.");

  // For lepton weights
  TCut elVetoWeight = cuts.elVetoWeight(options.leptCorr);
  TCut muVetoWeight = cuts.muVetoWeight(options.leptCorr);

  TCut trig      = cuts.nMinusOneCuts("trigger");
  TCut metFilt   = cuts.nMinusOneCuts("metFilter");
  TCut eVeto     = cuts.nMinusOneCuts("EVeto");
  TCut muVeto    = cuts.nMinusOneCuts("MuVeto");
  TCut dijet     = cuts.nMinusOneCuts("dijet");
  TCut sgnEtaJJ  = cuts.nMinusOneCuts("sgnEtaJJ");
  TCut dEtaJJ    = cuts.nMinusOneCuts("dEtaJJ");
  TCut met       = cuts.nMinusOneCuts("MET");
  TCut mJJ       = cuts.nMinusOneCuts("Mjj");
  TCut dPhiJJ    = cuts.nMinusOneCuts("dPhiJJ");
  TCut cjv       = cuts.nMinusOneCuts("CJV");

  // loop over datasets
  for (unsigned i=0; i<datasets.size(); ++i) {

    Dataset dataset = datasets.getDataset(i);

    if (dataset.isData) continue;
    // if (dataset.name.compare(0,3,"QCD") == 0) continue;

    TFile* ifile = datasets.getTFile(dataset.name);

    if (ifile==0) {
      std::cerr << "No file for " << dataset.name << std::endl;
      continue;
    }

    std::cout << "Making histograms for " << dataset.name << std::endl;
    
    TTree* tree = (TTree*) ifile->Get("invHiggsInfo/InvHiggsInfo");
    TFile* ofile = TFile::Open( (oDir+std::string("/")+dataset.name+std::string(".root")).c_str(), "RECREATE");

    // create histograms
    TH1D* hTrig                 = new TH1D("hTrigNM1",                   "", 2,   0.,  2.);
    TH1D* hMETfilt              = new TH1D("hMETFiltNM1",                "", 2,   0.,  2.);
    TH1D* hDijet                = new TH1D("hDijetNM1",                  "", 50,  0.,  250.);
    TH1D* hSgnEtaJJ             = new TH1D("hSgnEtaJJNM1",               "", 2,   -1., 1.);
    TH1D* hDEtaJJ               = new TH1D("hDEtaJJNM1",                 "", 50,  0.,  8.);
    TH1D* hMjj                  = new TH1D("hMjjNM1",                    "", 40,  0.,  4000.);
    TH1D* hMET                  = new TH1D("hMETNM1",                    "", 50,  0.,  500.);
    TH1D* hDPhiJMet             = new TH1D("hDPhiJMetNM1",               "", 50,  0.,  TMath::Pi());
    TH1D* hDPhiJMetNoDPhiJJ     = new TH1D("hDPhiJMetNM1NoDPhiJJ",       "", 50,  0.,  TMath::Pi());
    TH1D* hDPhiJMetNorm         = new TH1D("hDPhiJMetNormNM1",           "", 50,  0.,  200);
    TH1D* hDPhiJMetNormNoDPhiJJ = new TH1D("hDPhiJMetNormNM1NoDPhiJJ",   "", 50,  0.,  200);
    TH1D* hDPhiJJ               = new TH1D("hDPhiJJNM1",                 "", 50,  0.,  TMath::Pi());
    TH1D* hEVeto                = new TH1D("hEVetoNM1",                  "", 50,  0.,  50.);
    TH1D* hMuVeto               = new TH1D("hMuVetoNM1",                 "", 50,  0.,  50.);
    TH1D* hCenEt                = new TH1D("hCenEtNM1",                  "", 30,  0.,  150.); 
    TH1D* hCenEta               = new TH1D("hCenEtaNM1",                 "", 50, -5.,  5.);

    // Additional cuts specific to DYJetsToLL (cut on Zpt <100 to avoid double counting with the PtZ-100 sample)
    TCut cutD = cuts.cutDataset(dataset.name);

    // Additional WJets corrections (so can use inclusive & exclusive samples)
    TCut wWeight("");
    bool isWJets = false;
    bool isEwkW  = false;
    if (dataset.name == "WJets" ||
        dataset.name == "W1Jets" || 
        dataset.name == "W2Jets" || 
        dataset.name == "W3Jets" || 
        dataset.name == "W4Jets" ||
        dataset.name == "EWK_Wp2Jets" ||
        dataset.name == "EWK_Wm2Jets") {

      if (dataset.name == "EWK_Wp2Jets" || dataset.name == "EWK_Wm2Jets") isEwkW = true;
      else isWJets = true;

      if(isWJets) wWeight =  cuts.wWeight();
    } 

    // Add in lepton weights
    TCut leptonWeight("");
    if (!(dataset.isData)) leptonWeight = elVetoWeight*muVetoWeight;

    TCut otherCuts = puWeight * trigCorr * wWeight * leptonWeight;

    // fill histograms
    int n = tree->Draw("hltResult2>>hTrigNM1", (trig + cutD) * otherCuts);
    std::cout << "   " << n << " events passing trigger" << std::endl;

    tree->Draw("(metflag0 && metflag1 && metflag2 && metflag3 && metflag4 && metflag5 && metflag6 && metflag7 && metflag8)>>hMETFiltNM1", (metFilt + cutD) * otherCuts);
    tree->Draw("jet2Pt>>hDijetNM1", (dijet + cutD) * otherCuts);
    tree->Draw("TMath::Sign(1., jet1Eta*jet2Eta)>>hSgnEtaJJNM1", (sgnEtaJJ + cutD) * otherCuts);
    tree->Draw("abs(jet1Eta-jet2Eta)>>hDEtaJJNM1", (dEtaJJ + cutD) * otherCuts);
    tree->Draw("vbfM>>hMjjNM1", (mJJ  + cutD) * otherCuts);
    tree->Draw("met>>hMETNM1", (met + cutD) * otherCuts);
    // tree->Draw("min(abs(abs(abs(metPhi-jet1Phi)-TMath::Pi())-TMath::Pi()), abs(abs(abs(metPhi-jet2Phi)-TMath::Pi())-TMath::Pi()))>>hDPhiJMetNM1", (cuts.allCuts() + cutD) * otherCuts);
    tree->Draw("jmDPhi>>hDPhiJMetNM1", (cuts.allCuts() + cutD) * otherCuts);
    tree->Draw("jmDPhi>>hDPhiJMetNM1NoDPhiJJ", (dPhiJJ + cutD) * otherCuts);
    tree->Draw("jmDPhiNMin>>hDPhiJMetNormNM1", (cuts.allCuts() + cutD) * otherCuts);
    tree->Draw("jmDPhiNMin>>hDPhiJMetNormNM1NoDPhiJJ", (dPhiJJ + cutD) * otherCuts);
    tree->Draw("vbfDPhi>>hDPhiJJNM1", (dPhiJJ + cutD) * otherCuts);
    tree->Draw("ele1Pt>>hEVetoNM1", (eVeto + cutD) * otherCuts);
    tree->Draw("mu1Pt>>hMuVetoNM1", (muVeto + cutD) * otherCuts);
    tree->Draw("cenJetEt>>hCenEtNM1", (cjv + cutD) * otherCuts);
    tree->Draw("cenJetEta>>hCenEtaNM1", (cjv + cutD) * otherCuts);

    // Scale MC according to luminosity
    if (!dataset.isData) {
      double weight = lumi * dataset.sigma / dataset.nEvents;
      if(dataset.name == "EWK_ZvvFake") weight *= constants::ratioZToNuNuZToLL;
      hTrig->Scale(weight);
      hMETfilt->Scale(weight);
      hDijet->Scale(weight);
      hSgnEtaJJ->Scale(weight);
      hDEtaJJ->Scale(weight);
      hMjj->Scale(weight);
      hMET->Scale(weight);
      hDPhiJMet->Scale(weight);
      hDPhiJMetNoDPhiJJ->Scale(weight);
      hDPhiJMetNorm->Scale(weight);
      hDPhiJMetNormNoDPhiJJ->Scale(weight);
      hDPhiJJ->Scale(weight);
      hEVeto->Scale(weight);
      hMuVeto->Scale(weight);
      hCenEt->Scale(weight);
      hCenEta->Scale(weight);
    }    

    // write histograms

    hTrig->Write("",TObject::kOverwrite);
    hMETfilt->Write("",TObject::kOverwrite);
    hDijet->Write("",TObject::kOverwrite);
    hSgnEtaJJ->Write("",TObject::kOverwrite);
    hDEtaJJ->Write("",TObject::kOverwrite);
    hMjj->Write("",TObject::kOverwrite);
    hMET->Write("",TObject::kOverwrite);
    hDPhiJMet->Write("",TObject::kOverwrite);
    hDPhiJMetNoDPhiJJ->Write("",TObject::kOverwrite);
    hDPhiJMetNorm->Write("",TObject::kOverwrite);
    hDPhiJMetNormNoDPhiJJ->Write("",TObject::kOverwrite);
    hDPhiJJ->Write("",TObject::kOverwrite);
    hEVeto->Write("",TObject::kOverwrite);
    hMuVeto->Write("",TObject::kOverwrite);
    hCenEt->Write("",TObject::kOverwrite);
    hCenEta->Write("",TObject::kOverwrite);

    ofile->Close();
    
    ifile->Close();

  }

  // list of histograms for summing
  std::vector<std::string> hists;
  hists.push_back("hTrigNM1");
  hists.push_back("hMETFiltNM1");
  hists.push_back("hDijetNM1");
  hists.push_back("hSgnEtaJJNM1");
  hists.push_back("hDEtaJJNM1");
  hists.push_back("hMjjNM1");
  hists.push_back("hMETNM1");
  hists.push_back("hDPhiJMetNM1");
  hists.push_back("hDPhiJMetNM1NoDPhiJJ");
  hists.push_back("hDPhiJMetNormNM1");
  hists.push_back("hDPhiJMetNormNM1NoDPhiJJ");
  hists.push_back("hDPhiJJNM1");
  hists.push_back("hEVetoNM1");
  hists.push_back("hMuVetoNM1");
  hists.push_back("hCenEtNM1");
  hists.push_back("hCenEtaNM1");

  // sum Z+jets
  std::cout << "Summing histograms for Z+Jets" << std::endl;
  std::vector<std::string> zJets;
  zJets.push_back("Zvv_50to100");
  zJets.push_back("Zvv_100to200");
  zJets.push_back("Zvv_200to400");
  zJets.push_back("Zvv_400toinf");
  zJets.push_back("EWK_ZvvFake");  
  SumDatasets(oDir, zJets, hists, "ZJets+EWK");

  // sum W+jets datasets
  std::cout << "Summing histograms for W+Jets" << std::endl;
  std::vector<std::string> wJets;
  wJets.push_back(std::string("WJets"));
  wJets.push_back(std::string("W1Jets"));
  wJets.push_back(std::string("W2Jets"));
  wJets.push_back(std::string("W3Jets"));
  wJets.push_back(std::string("W4Jets"));
  wJets.push_back(std::string("EWK_Wp2Jets"));
  wJets.push_back(std::string("EWK_Wm2Jets"));
  SumDatasets(oDir, wJets, hists, "WNJets+EWK");

  // sum QCD
  std::cout << "Summing histograms for QCD" << std::endl;
  std::vector<std::string> qcd;
  qcd.push_back("QCD_Pt30to50");
  qcd.push_back("QCD_Pt50to80");
  qcd.push_back("QCD_Pt80to120");
  qcd.push_back("QCD_Pt120to170");
  qcd.push_back("QCD_Pt170to300");
  qcd.push_back("QCD_Pt300to470");
  qcd.push_back("QCD_Pt470to600");
  qcd.push_back("QCD_Pt600to800");
  qcd.push_back("QCD_Pt800to1000");
  qcd.push_back("QCD_Pt1000to1400");
  qcd.push_back("QCD_Pt1400to1800");
  qcd.push_back("QCD_Pt1800");
  SumDatasets(oDir, qcd, hists, "QCD");

  // Sum Single t and tbar
  std::cout << "Summing histograms for Single T and TTbar" << std::endl;
  std::vector<std::string> singleT;
  singleT.push_back("TTBar");
  singleT.push_back("SingleT_t");
  singleT.push_back("SingleT_s");
  singleT.push_back("SingleT_tW");
  singleT.push_back("SingleTbar_t");
  singleT.push_back("SingleTbar_s");
  singleT.push_back("SingleTbar_tW");
  SumDatasets(oDir, singleT, hists, "SingleT+TTbar");

  // Sum diboson
  std::cout << "Summing histograms for Diboson" << std::endl;
  std::vector<std::string> diboson;
  diboson.push_back("WW");
  diboson.push_back("WZ");
  diboson.push_back("ZZ");
  diboson.push_back("WG");
  SumDatasets(oDir, diboson, hists, "Diboson");

  //Sum DYJetsToLL
  std::cout << "Summing histograms for DYJetsToLL" << std::endl;
  std::vector<std::string> dyjets;
  dyjets.push_back("DYJetsToLL");
  dyjets.push_back("DYJetsToLL_PtZ-100");
  dyjets.push_back("DYJetsToLL_EWK");
  SumDatasets(oDir, dyjets, hists, "DYJets+EWK");

  // Signal
  std::vector<std::string> signal;
  signal.push_back("SignalM125_POWHEG");
  SumDatasets(oDir, signal, hists, "Signal");

  // make plots
  std::cout << "Making plots" << std::endl;
  StackPlot plots(oDir);
  // plots.setLegPos(0.69,0.70,0.98,0.97);
  plots.setLegPos(0.63,0.58,0.89,0.89);

  plots.addDataset("Diboson", kViolet-6, 0);
  plots.addDataset("DYJets+EWK", kPink-4,0);
  plots.addDataset("SingleT+TTbar", kAzure-2, 0);
  plots.addDataset("QCD", kGreen+3, 0);
  plots.addDataset("ZJets+EWK", kOrange-2, 0);
  plots.addDataset("WNJets+EWK", kGreen-3, 0);
  plots.addDataset("Signal","Signal 100% BR", kRed, 2);

  plots.setLumi(19.5); // doens't rescale anything, just adds a bit of text on the plot
  std::cout << "plotting hTrigNM1" << std::endl;
  plots.draw("hTrigNM1", "", "",1,"NM1");
  plots.draw("hMETFiltNM1", "", "",1,"NM1");
  plots.draw("hDijetNM1", "Sub-leading jet p_{T} [GeV]", "Events / 5 GeV",1,"NM1");
  // plots.draw("hSgnEtaNM1", "#eta_{1}#times#eta_{2}", "N_{events}",1,"NM1");
  plots.setYMax(5E4);
  plots.draw("hDEtaJJNM1", "#Delta #eta_{jj}", "Events / 0.16",1,"NM1");
  plots.draw("hMjjNM1", "M_{jj} [GeV]", "Events / 100 GeV",1,"NM1");
  plots.draw("hMETNM1", "E_{T}^{miss} [GeV]", "Events / 10 GeV",1,"NM1");
  plots.draw("hDPhiJMetNM1", "#Delta #phi_{j-#slash{E}_{T}}", "N_{events}",1,"NM1");
  plots.draw("hDPhiJMetNM1NoDPhiJJ", "#Delta #phi_{j-#slash{E}_{T}}, no #Delta #phi_{jj} cut", "N_{events}",1,"NM1");
  plots.setYMax(5E5);
  plots.draw("hDPhiJJNM1", "#Delta #phi_{jj}", "Events / (#pi/50)",1,"NM1");
  plots.setYMax(1E5);
  plots.draw("hCenEtNM1", "Central Jet E_{T} [GeV]", "Events / 5 GeV",1,"NM1");
  plots.draw("hCenEtaNM1", "Central Jet #eta", "N_{events}",1,"NM1");

  plots.setYMin(1e-1);
  // plots.setYMax(1e2);
  plots.draw("hEVetoNM1", "E_{T} [GeV]", "N_{events}",1,"NM1");
  plots.draw("hMuVetoNM1", "p_{T} [GeV]", "N_{events}",1,"NM1");

  // plots.setYMin(1e-2);
  plots.draw("hDPhiJMetNormNM1", "#Delta #phi_{N}^{min}", "N_{events}",1,"NM1");
  plots.draw("hDPhiJMetNormNM1NoDPhiJJ", "#Delta #phi_{N}^{min}, no #Delta #phi_{jj} cut", "N_{events}",1,"NM1");

}
