
#include "InvisibleHiggs/Analysis/interface/ProgramOptions.h"
#include "InvisibleHiggs/Analysis/interface/Cuts.h"
#include "InvisibleHiggs/Analysis/interface/Histogrammer.h"
#include "InvisibleHiggs/Analysis/interface/StackPlot.h"
#include "InvisibleHiggs/Analysis/interface/SumDatasets.h"
#include "InvisibleHiggs/Analysis/interface/Datasets.h"
#include "InvisibleHiggs/Analysis/interface/Constants.h"


#include "TTree.h"
#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {

  TH1::SetDefaultSumw2();

  ProgramOptions options(argc, argv);

  double lumi = options.lumi;

  std::cout << "Integrated luminosity : " << lumi << " pb-1" << std::endl;

  Datasets datasets(options.iDir);
  datasets.readFile(options.datasetFile);

  // output file
  TFile* ofile = TFile::Open( (options.oDir+std::string("/WTauBackground.root")).c_str(), "RECREATE");

  // For control plots
  std::string oDirPlots = options.oDir+std::string("/WTauControlPlots");
  boost::filesystem::path opath(oDirPlots);
  if (!exists(opath)) {
    std::cout << "Creating output directory : " << oDirPlots << std::endl;
    boost::filesystem::create_directory(opath);
  }
  else std::cout << "Writing plots results to " << oDirPlots << std::endl;
  std::vector<std::string> hnames; // to hold hist names for control plots

  /////////////////////////
  // Define general cuts //
  /////////////////////////
  Cuts cuts;
  unsigned nCutsWTau = cuts.nCutsWTau();

  TCut puWeight("puWeight");
  TCut trigCorrWeight("trigCorrWeight");
  TCut trigCorrWeight2( "(trigCorrWeight>0) ? trigCorrWeight : 1." );
  TCut wWeight = cuts.wWeight();

  // Cuts for control plots
  // Get puWeight etc added below if necessary
  // standard TightMjj selection - essentially signal selection but no DPhiJJ and no cjv
  TCut cutTightMjj_basic = cuts.HLTandMETFilters() + cuts.cutWTau("lVeto") + cuts.cutWTau("dijet") + cuts.cutWTau("dEtaJJ") + cuts.cutWTau("MET") + cuts.cutWTau("Mjj"); 
  TCut cutTightMjj(""); // used to add in PU, trig corr, wWeight etc

  TCut cutDPhiSignalNoCJV_basic = cutTightMjj_basic + cuts.cutWTau("dPhiJJ"); //standard DPhiSignalNoCJV selection
  TCut cutDPhiSignalNoCJV(""); // used to add in PU, trig corr, wWeight etc
  
  ////////////////////////////////////
  // Define histograms for counting //
  ////////////////////////////////////

  // NOTE: Since we don't use QCD method 1 anymore, and we don't use data-driven BG for our QCD/Z/W estimates (yet?), we don't really need to bin by DPhiJJ anymore
  // Did it this way to match other codes, and incase we need a control region for closure test or something
  // Think about optimising & cleaning this up!
  double dphiEdges[4] = { 0., 1.0, 2.6, TMath::Pi() };
  
  // For N_data and N_BG
  TH1D* hWTau_BGC_DPhi       = new TH1D("hWTau_BGC_DPhi",   "", 3, dphiEdges);  // background MC ctrl region (ctrl region = require tau reconstructed)
  TH1D* hWTau_DataC_DPhi     = new TH1D("hWTau_DataC_DPhi", "", 3, dphiEdges);  // Data ctrl region
  
  // Plots for eff_tauID
  TH1D* hWTau_MCC_NoCJV_DPhi = new TH1D("hWTau_MCC_NoCJV_DPhi", "", 3, dphiEdges);  // W+jets MC at gen level in ctrl region, no CJV
  TH1D* hWTau_MCS_NoCJV_DPhi = new TH1D("hWTau_MCS_NoCJV_DPhi", "", 3, dphiEdges);  // W+jets MC at gen level in signal region, no CJV
  
  // Plots for eff_CJV
  TH1D* hWTau_MC_CJV_DPhi    = new TH1D("hWTau_MC_CJV_DPhi",   "", 3, dphiEdges); // W+jets MC at gen level with CJV
  TH1D* hWTau_MC_NoCJV_DPhi  = new TH1D("hWTau_MC_NoCJV_DPhi", "", 3, dphiEdges); // W+jets MC at gen level without CJV
  
  // cutflow histograms
  TH1D* hDataWTau            = new TH1D("hWTau_CutFlow_Data",        "", nCutsWTau, 0., nCutsWTau);
  TH1D* hWLNuWTau            = new TH1D("hWTau_CutFlow_WToLNu",      "", nCutsWTau, 0., nCutsWTau);
  TH1D* hQCDWTau             = new TH1D("hWTau_CutFlow_QCD",         "", nCutsWTau, 0., nCutsWTau);
  TH1D* hDYWTau              = new TH1D("hWTau_CutFlow_DY",          "", nCutsWTau, 0., nCutsWTau);
  TH1D* hSingleTWTau         = new TH1D("hWTau_CutFlow_SingleTSum",  "", nCutsWTau, 0., nCutsWTau);
  TH1D* hDibosonWTau         = new TH1D("hWTau_CutFlow_Diboson",     "", nCutsWTau, 0., nCutsWTau);

  ///////////////////////////
  // loop over MC datasets //
  ///////////////////////////
  for (unsigned i=0; i<datasets.size(); ++i) {

    Dataset dataset = datasets.getDataset(i);

    TFile* file = datasets.getTFile(dataset.name);
    TTree* tree = (TTree*) file->Get("invHiggsInfo/InvHiggsInfo");

    /////////////////////////////
    // setup cuts & temp hists //
    /////////////////////////////

    // For DY samples - ensure we don't double count for pT > 100 (2 DY smaples) 
    TCut cutD = cuts.cutDataset(dataset.name);

    TCut cutWTau_C(""); // Cut to calculate numbers of data and BG from MC 
    TCut cutWTau_MCC_NoCJV(""), cutWTau_MCS_NoCJV(""); // Cuts for Tau ID eff
    TCut cutWTau_MC_CJV(""), cutWTau_MC_NoCJV(""); // Cuts for CJV eff

    // tmp histograms - numbers of BG. DOn't need one for data, we can put that straight into hWTau_DataC_DPhi
    TH1D* hWTau_BGC_DPhi_tmp       = new TH1D("hWTau_BGC_DPhi_tmp","",3,dphiEdges);
    
    // tmp histograms - tau ID eff
    TH1D* hWTau_MCC_NoCJV_DPhi_tmp = new TH1D("hWTau_MCC_NoCJV_DPhi_tmp", "", 3, dphiEdges);  // W+jets MC ctrl region
    TH1D* hWTau_MCS_NoCJV_DPhi_tmp = new TH1D("hWTau_MCS_NoCJV_DPhi_tmp", "", 3, dphiEdges);  // W+jets MC sgnl region
    
    // tmp histogram - CJV eff
    TH1D* hWTau_MC_CJV_DPhi_tmp    = new TH1D("hWTau_MC_CJV_DPhi_tmp",    "", 3, dphiEdges);  // W+jets MC ctrl region
    TH1D* hWTau_MC_NoCJV_DPhi_tmp  = new TH1D("hWTau_MC_NoCJV_DPhi_tmp",  "", 3, dphiEdges);  // W+jets MC sgnl region

    // Weight to lumi
    double weight = (dataset.isData) ? 1. : (lumi * dataset.sigma / dataset.nEvents);

    ///////////////////////////////
    // Now do some hist filling! //
    ///////////////////////////////

    // check it's W+Jets as they are used for eff hists
    bool isWJets = false;
    if (dataset.name == "WJets" ||
        dataset.name == "W1Jets" || 
        dataset.name == "W2Jets" || 
        dataset.name == "W3Jets" || 
        dataset.name == "W4Jets") {

      isWJets = true;
      std::cout << "Analysing W MC     : " << dataset.name << std::endl;
      std::cout << "  weight : " << weight << std::endl;

      // Do Tau ID eff
      cutWTau_MCC_NoCJV = puWeight * trigCorrWeight2 * wWeight * (cutD + cuts.wTauGen() + cutTightMjj_basic + cuts.cutWTau("wTau"));
      cutWTau_MCS_NoCJV = puWeight * trigCorrWeight2 * wWeight * (cutD + cuts.wTauGen() + cutTightMjj_basic);

      tree->Draw("vbfDPhi>>hWTau_MCC_NoCJV_DPhi_tmp", cutWTau_MCC_NoCJV);
      tree->Draw("vbfDPhi>>hWTau_MCS_NoCJV_DPhi_tmp", cutWTau_MCS_NoCJV); 
      
      hWTau_MCC_NoCJV_DPhi_tmp->Scale(weight);
      hWTau_MCS_NoCJV_DPhi_tmp->Scale(weight);
      
      hWTau_MCC_NoCJV_DPhi->Add(hWTau_MCC_NoCJV_DPhi_tmp);
      hWTau_MCS_NoCJV_DPhi->Add(hWTau_MCS_NoCJV_DPhi_tmp);

      // Do CJV eff
      cutWTau_MC_CJV   = puWeight * trigCorrWeight2 * wWeight * (cutD + cuts.wTauGen() + cutTightMjj_basic + cuts.cutWTau("CJV"));
      cutWTau_MC_NoCJV = puWeight * trigCorrWeight2 * wWeight * (cutD + cuts.wTauGen() + cutTightMjj_basic);

      tree->Draw("vbfDPhi>>hWTau_MCC_NoCJV_DPhi_tmp", cutWTau_MC_CJV);
      tree->Draw("vbfDPhi>>hWTau_MCS_NoCJV_DPhi_tmp", cutWTau_MC_NoCJV); 

      hWTau_MC_CJV_DPhi_tmp->Scale(weight);  
      hWTau_MC_NoCJV_DPhi_tmp->Scale(weight);  

      hWTau_MC_CJV_DPhi->Add(hWTau_MC_CJV_DPhi_tmp);
      hWTau_MC_NoCJV_DPhi->Add(hWTau_MC_NoCJV_DPhi_tmp);
      
      // Setup control plot cuts
      cutTightMjj        = puWeight * trigCorrWeight2 * wWeight * cutTightMjj_basic;
      cutDPhiSignalNoCJV = puWeight * trigCorrWeight2 * wWeight * cutDPhiSignalNoCJV_basic;
      
    } else if (dataset.isData) {

      std::cout << "Analysing Data     : " << dataset.name << std::endl;

      // Count number of tau in control region in data
      cutWTau_C = puWeight * (cuts.cutWTau("wTau") + cutDPhiSignalNoCJV_basic);
      tree->Draw("vbfDPhi>>hWTau_DataC_DPhi", cutWTau_C);

      // Setup cuts for control plots 
      cutTightMjj        = puWeight * trigCorrWeight2 * cutTightMjj_basic;
      cutDPhiSignalNoCJV = puWeight * trigCorrWeight2 * cutDPhiSignalNoCJV_basic;

    } else { // All MC _BUT_ WJets. Need to go into hWTau_BGC_DPhi

      std::cout << "Analysing BG MC    : " << dataset.name << std::endl;
      std::cout << "  weight : " << weight << std::endl;

      // Count number of tau in control region in data
      cutWTau_C = puWeight * trigCorrWeight2 * (cutD && cuts.cutWTau("wTau") && cutDPhiSignalNoCJV_basic);
      tree->Draw("vbfDPhi>>hWTau_BGC_DPhi_tmp", cutWTau_C);
      
      hWTau_BGC_DPhi_tmp->Scale(weight);
      
      hWTau_BGC_DPhi->Add(hWTau_BGC_DPhi_tmp);

      // Setup control plot cuts
      cutTightMjj        = puWeight * trigCorrWeight2 * (cutD + cutTightMjj_basic);
      cutDPhiSignalNoCJV = puWeight * trigCorrWeight2 * (cutD + cutDPhiSignalNoCJV_basic);

    }
        
    // debug output
    std::cout << "  N ctrl region (dphi<1) : " << hWTau_BGC_DPhi_tmp->GetBinContent(1) << " +/- " << hWTau_BGC_DPhi_tmp->GetBinError(1) << std::endl;
    
    delete hWTau_BGC_DPhi_tmp;
    delete hWTau_MCC_NoCJV_DPhi_tmp;
    delete hWTau_MCS_NoCJV_DPhi_tmp;
    delete hWTau_MC_CJV_DPhi_tmp;    
    delete hWTau_MC_NoCJV_DPhi_tmp;    
    
    ofile->cd();
    
    ///////////////////////////////
    // per-dataset cutflow hists //
    ///////////////////////////////
    std::string hnameWTau = std::string("hWTau_CutFlow_")+dataset.name;
    TH1D* hCutFlowWTau = new TH1D(hnameWTau.c_str(), "", nCutsWTau, 0., nCutsWTau);

    for (unsigned c=0; c<nCutsWTau; ++c) {

      TCut cut;
      if(c == nCutsWTau-1) {
          cut = puWeight * trigCorrWeight * (cutD + cuts.cutflowWTau(c));
          if(isWJets) cut = puWeight * trigCorrWeight * wWeight * (cuts.cutflowWTau(c));
      } else {
          cut = puWeight * (cutD + cuts.cutflowWTau(c));
          if(isWJets) cut = puWeight * wWeight * (cuts.cutflowWTau(c));
      }
      TH1D* h = new TH1D("h","", 1, 0., 1.);
      tree->Draw("0.5>>h", cut);

      hCutFlowWTau->SetBinContent(c+1, h->GetBinContent(1));
      hCutFlowWTau->SetBinError(c+1, h->GetBinError(1));

      delete h;
    }

    hCutFlowWTau->Scale(weight);

    ////////////////////
    // sum histograms //
    ////////////////////
    if (dataset.isData) 
      hDataWTau->Add(hCutFlowWTau);
    
    if (isWJets)
      hWLNuWTau->Add(hCutFlowWTau);
    
    if (dataset.name.compare(0,3,"QCD")==0)
      hQCDWTau->Add(hCutFlowWTau);
    
    if (dataset.name.compare(0,2,"DY")==0) 
      hDYWTau->Add(hCutFlowWTau);
    
    if (dataset.name.compare(0,7,"SingleT")==0) 
      hSingleTWTau->Add(hCutFlowWTau);
    
    if (dataset.name.compare(0,2,"WW")==0 ||
        dataset.name.compare(0,2,"WZ")==0 ||
        dataset.name.compare(0,2,"ZZ")==0 ) 
      hDibosonWTau->Add(hCutFlowWTau);

    hCutFlowWTau->Write("",TObject::kOverwrite);

    delete hCutFlowWTau;

    ///////////////////////
    // Do control plots: //
    ///////////////////////
    // Tau1Pt, Tau1Eta, DPhiJJ, Tau1mT (post DPhiSIGNALnoCJV), numTaus, MinDR(Tau1,tag jet), NumTauMatched,  Tau1mT (TightMjj)

    TFile* ofilePlots = TFile::Open( (oDirPlots+std::string("/")+dataset.name+std::string(".root")).c_str(), "RECREATE");

    // Ones with DPhiSignalNoCJV cut
    std::string hname = "hWTau_tau1Pt";
    if (i==0) hnames.push_back(hname);
    TH1D* hTau1Pt = new TH1D(hname.c_str(), "", 20, 0., 200.);
    std::string str = "tau1Pt>>"+hname;
    tree->Draw(str.c_str(), cutDPhiSignalNoCJV);
    hTau1Pt->Scale(weight);
    hTau1Pt->Write("", TObject::kOverwrite); 

    hname = "hWTau_tau1Eta";
    if (i==0) hnames.push_back(hname);
    TH1D* hTau1Eta = new TH1D(hname.c_str(), "", 25, -5., 5.);
    str = "tau1Eta>>"+hname;
    tree->Draw(str.c_str(), cutDPhiSignalNoCJV);
    hTau1Eta->Scale(weight); 
    hTau1Eta->Write("", TObject::kOverwrite); 

    hname = "hWTau_dPhiJJ";
    if (i==0) hnames.push_back(hname);
    TH1D* hDPhiJJ = new TH1D(hname.c_str(), "", 50, 0., TMath::Pi());
    str = "vbfDPhi>>"+hname;
    tree->Draw(str.c_str(), cutDPhiSignalNoCJV);
    hDPhiJJ->Scale(weight); 
    hDPhiJJ->Write("", TObject::kOverwrite); 
    
    hname = "hWTau_mT_DPhiSignalNoCJV";
    if (i==0) hnames.push_back(hname);
    TH1D* hWmT_DPhiSignalNoCJV = new TH1D(hname.c_str(), "", 20, 0., 200.);
    str = "tau1mT>>"+hname;
    tree->Draw(str.c_str(), cutDPhiSignalNoCJV);
    hWmT_DPhiSignalNoCJV->Scale(weight); 
    hWmT_DPhiSignalNoCJV->Write("", TObject::kOverwrite); 

    // Plots with TightMjj (+tau)
    hname = "hNumberTaus";
    if (i==0) hnames.push_back(hname);
    TH1D* hNTaus = new TH1D(hname.c_str(), "", 5, 0., 5.);
    str = "nTaus_pass>>"+hname;
    tree->Draw(str.c_str(), cutTightMjj);
    hNTaus->Scale(weight); 
    hNTaus->Write("", TObject::kOverwrite); 

    hname = "hMinDR";
    if (i==0) hnames.push_back(hname);
    TH1D* hMinDR = new TH1D(hname.c_str(), "", 18, 0., 4.5);
    str = "tau1dR>>"+hname;
    tree->Draw(str.c_str(), cutTightMjj);
    hMinDR->Scale(weight); 
    hMinDR->Write("", TObject::kOverwrite); 

    hname = "hTauMatchJet";
    if (i==0) hnames.push_back(hname);
    TH1D* hTauMatchJet = new TH1D(hname.c_str(), "", 4, 0., 4);
    str = "nTaus_match>>"+hname;
    tree->Draw(str.c_str(), cutTightMjj);
    hTauMatchJet->Scale(weight); 
    hTauMatchJet->Write("", TObject::kOverwrite); 

    hname = "hWTau_mT_TightMjj";
    if (i==0) hnames.push_back(hname);
    TH1D* hWmT_TightMjj = new TH1D(hname.c_str(), "", 20, 0., 200);
    str = "tau1mT>>"+hname;
    tree->Draw(str.c_str(), cutTightMjj);
    hWmT_TightMjj->Scale(weight); 
    hWmT_TightMjj->Write("", TObject::kOverwrite); 

    delete tree;

    ofilePlots->Close();
    file->Close();
   
  } //end of datasets loop

  // create histograms with the background estimate
  TH1D* hWTau_TauIDEff_DPhi  = new TH1D("hWTau_TauIDEff_DPhi",  "", 3, dphiEdges); // tau ID eff
  TH1D* hWTau_CJVEff_DPhi    = new TH1D("hWTau_CJVEff_DPhi",    "", 3, dphiEdges); // CJV eff
  TH1D* hWTau_Est_NoCJV_DPhi = new TH1D("hWTau_Est_NoCJV_DPhi", "", 3, dphiEdges); // n^data - n^BG in Anne Marie's AN
  TH1D* hWTau_Est_DPhi       = new TH1D("hWTau_Est_DPhi",       "", 3, dphiEdges); // Final number of tau estimate

  hWTau_TauIDEff_DPhi->Divide(hWTau_MCC_NoCJV_DPhi, hWTau_MCS_NoCJV_DPhi, 1., 1.); // calculate tau ID eff
  hWTau_CJVEff_DPhi->Divide(hWTau_MC_CJV_DPhi, hWTau_MC_NoCJV_DPhi, 1., 1.); // calculate CJV eff

  hWTau_Est_NoCJV_DPhi->Add(hWTau_DataC_DPhi, hWTau_BGC_DPhi, 1., -1.); // n^data - n^BG
  hWTau_Est_DPhi->Divide(hWTau_Est_NoCJV_DPhi,hWTau_MCC_NoCJV_DPhi,1.,1.); // ( n^data - n^BG ) / tauID eff numerator
  hWTau_Est_DPhi->Multiply(hWTau_MC_CJV_DPhi); // * CJV eff numerator - use numerator as denominators in efficiencies cancel, and we want to get the errors right and not include error from denominator (as it cancels!)

  std::cout << std::endl << std::endl;
  std::cout << "W->tau channel (dphi<1.0)" << std::endl;
  std::cout << "  Data ctrl region                                       : " << hWTau_DataC_DPhi->GetBinContent(1) << " +/- " << hWTau_DataC_DPhi->GetBinError(1) << std::endl;
  std::cout << "  Background ctrl region                                 : " << hWTau_BGC_DPhi->GetBinContent(1) << " +/- " << hWTau_BGC_DPhi->GetBinError(1) << std::endl;
  std::cout << "  W+jets MC - gen level tau, no CJV, tau reco            : " << hWTau_MCC_NoCJV_DPhi->GetBinContent(1) << " +/- " << hWTau_MCC_NoCJV_DPhi->GetBinError(1) << std::endl;
  std::cout << "  W+jets MC - gen level tau, no CJV                      : " << hWTau_MCS_NoCJV_DPhi->GetBinContent(1) << " +/- " << hWTau_MCS_NoCJV_DPhi->GetBinError(1) << std::endl;
  std::cout << "  W+jets MC - gen level tau, standard selection w/CJV    : " << hWTau_MC_CJV_DPhi->GetBinContent(1) << " +/- " << hWTau_MC_CJV_DPhi->GetBinError(1) << std::endl;
  std::cout << "  W+jets MC - gen level tau, standard selection wout/CJV : " << hWTau_MC_NoCJV_DPhi->GetBinContent(1) << " +/- " << hWTau_MC_NoCJV_DPhi->GetBinError(1) << std::endl;
  std::cout << std::endl;
  std::cout << "  W in ctrl region                                       : " << hWTau_Est_NoCJV_DPhi->GetBinContent(1) << " +/- " << hWTau_Est_NoCJV_DPhi->GetBinError(1) << std::endl;
  std::cout << "  eff_tauID                                              : " << hWTau_TauIDEff_DPhi->GetBinContent(1) << " +/- " << hWTau_TauIDEff_DPhi->GetBinError(1) << std::endl;
  std::cout << "  eff_CJV                                                : " << hWTau_CJVEff_DPhi->GetBinContent(1) << " +/- " << hWTau_CJVEff_DPhi->GetBinError(1) << std::endl;
  std::cout << std::endl << std::endl;
  std::cout << "  W in sgnl region                                       : " << hWTau_Est_DPhi->GetBinContent(1) << " +/- " << hWTau_Est_DPhi->GetBinError(1) << std::endl;

  // write the cutflow table
  std::cout << "Writing cut flow TeX file" << std::endl;

  ofstream effFile;
  effFile.open(options.oDir+std::string("/cutflowWTau.tex"));

  effFile << "Cut & N(data) & N($W\\rightarrow l\\nu$) & N(DY) & N(QCD) & N($t\\bar{t}$) & N(single $t$) & N(diboson) \\\\" << std::endl;

  TH1D* hTTbarWTau = (TH1D*) ofile->Get("hWTau_CutFlow_TTBar");

  // cutflow table
  for (unsigned i=0; i<nCutsWTau; ++i) {

    effFile << cuts.cutNameWTau(i) << " & ";
    effFile << "$" << hDataWTau->GetBinContent(i+1) << " \\pm " << hDataWTau->GetBinError(i+1) << "$ & ";
    effFile << "$" << hWLNuWTau->GetBinContent(i+1) << " \\pm " << hWLNuWTau->GetBinError(i+1) << "$ & ";
    effFile << "$" << hDYWTau->GetBinContent(i+1) << " \\pm " << hDYWTau->GetBinError(i+1) << "$ & ";
    effFile << "$" << hQCDWTau->GetBinContent(i+1) << " \\pm " << hQCDWTau->GetBinError(i+1) << "$ & ";
    effFile << "$" << hTTbarWTau->GetBinContent(i+1) << " \\pm " << hTTbarWTau->GetBinError(i+1) << "$ & ";
    effFile << "$" << hSingleTWTau->GetBinContent(i+1) << " \\pm " << hSingleTWTau->GetBinError(i+1) << "$ & ";
    effFile << "$" << hDibosonWTau->GetBinContent(i+1) << " \\pm " << hDibosonWTau->GetBinError(i+1) << "$ \\\\ ";
    effFile << std::endl;

  }

  effFile << std::endl << std::endl;
  effFile.close();  

  // for (unsigned n = 0; n < hnames.size(); n++)
  //   std::cout << hnames[n] <<std::endl;

  //////////////////////
  // Do control plots //
  //////////////////////
  std::vector<std::string> wjetsDatasets;
  wjetsDatasets.push_back(std::string("WJets"));
  wjetsDatasets.push_back(std::string("W1Jets"));
  wjetsDatasets.push_back(std::string("W2Jets"));
  wjetsDatasets.push_back(std::string("W3Jets"));
  wjetsDatasets.push_back(std::string("W4Jets"));
  SumDatasets(oDirPlots, wjetsDatasets, hnames, "WNJets");

  std::vector<std::string> topDatasets;
  topDatasets.push_back(std::string("SingleT_t"));
  topDatasets.push_back(std::string("SingleTbar_t"));
  topDatasets.push_back(std::string("SingleT_s"));
  topDatasets.push_back(std::string("SingleTbar_s"));
  topDatasets.push_back(std::string("SingleT_tW"));
  topDatasets.push_back(std::string("SingleTbar_tW"));
  topDatasets.push_back(std::string("TTBar"));
  SumDatasets(oDirPlots, topDatasets, hnames, "SingleT+TTBar");

  // sum DY contributions
  std::vector<std::string> dyjets;
  dyjets.push_back("DYJetsToLL");
  dyjets.push_back("DYJetsToLL_PtZ-100");
  dyjets.push_back("DYJetsToLL_EWK");
  SumDatasets(oDirPlots,dyjets,hnames,"DYJets");

  // sum single top datasets
  std::vector<std::string> dibDatasets;
  dibDatasets.push_back(std::string("WW"));
  dibDatasets.push_back(std::string("WZ"));
  dibDatasets.push_back(std::string("ZZ"));
  SumDatasets(oDirPlots, dibDatasets, hnames, "DiBoson");

  // sum QCD histograms
  std::vector<std::string> qcdDatasets;
  qcdDatasets.push_back(std::string("QCD_Pt30to50"));
  qcdDatasets.push_back(std::string("QCD_Pt50to80"));
  qcdDatasets.push_back(std::string("QCD_Pt80to120"));
  qcdDatasets.push_back(std::string("QCD_Pt120to170"));
  qcdDatasets.push_back(std::string("QCD_Pt170to300"));
  qcdDatasets.push_back(std::string("QCD_Pt300to470"));
  qcdDatasets.push_back(std::string("QCD_Pt470to600"));
  qcdDatasets.push_back(std::string("QCD_Pt600to800"));
  qcdDatasets.push_back(std::string("QCD_Pt800to1000"));
  qcdDatasets.push_back(std::string("QCD_Pt1000to1400"));
  qcdDatasets.push_back(std::string("QCD_Pt1400to1800"));
  qcdDatasets.push_back(std::string("QCD_Pt1800"));
  SumDatasets(oDirPlots, qcdDatasets, hnames, "QCD");

  // make plots
  std::cout << "Making plots" << std::endl;
  StackPlot plots(oDirPlots);
  plots.setLegPos(0.70,0.60,0.93,0.89);

  plots.addDataset("DiBoson", kViolet-6, 0);
  plots.addDataset("QCD", kGreen+3, 0);
  plots.addDataset("DYJets", kPink-4,0);
  plots.addDataset("SingleT+TTBar", kAzure-2, 0);
  plots.addDataset("WNJets", kGreen-3, 0);
  plots.addDataset("METABCD", kBlack, 1);

  plots.setLabel("#tau channel");
  std::string hname = "hWTau_tau1Pt";
  plots.draw(hname.c_str(), "Leading #tau p_{T} [GeV]", "N_{events}", true, true);

  hname = "hWTau_tau1Eta";
  plots.draw(hname.c_str(), "Leading jet #eta", "N_{events}",true, true);

  hname = "hWTau_dPhiJJ";
  plots.draw(hname.c_str(), "#Delta #phi_{jj}", "N_{events}",true, true);

  hname = "hWTau_mT_DPhiSignalNoCJV";
  plots.draw(hname.c_str(), "m_{T}(#tau#nu) [GeV]", "N_{events}",true, true);

  hname = "hNumberTaus";
  plots.draw(hname.c_str(), "Number of #tau", "N_{events}",true, true);

  hname = "hMinDR";
  plots.draw(hname.c_str(), "#Delta R_{min}(#tau, leading jet)", "N_{events}",true, true);

  hname = "hTauMatchJet";
  plots.draw(hname.c_str(), "#tau matching with tagging jets", "N_{events}",true, true);
  
  hname = "hWTau_mT_TightMjj";
  plots.draw(hname.c_str(), "m_{T}(#tau#nu) [GeV]", "N_{events}",true, true);

  //////////////////////////////////////
  // store cutflow & other histograms //
  //////////////////////////////////////
  ofile->cd(); 
  
  // Store numbers for data, bg, efficiencies  
  hWTau_BGC_DPhi->Write("",TObject::kOverwrite);
  hWTau_DataC_DPhi->Write("",TObject::kOverwrite);
  hWTau_MCC_NoCJV_DPhi->Write("",TObject::kOverwrite);
  hWTau_MCS_NoCJV_DPhi->Write("",TObject::kOverwrite);
  hWTau_MC_CJV_DPhi->Write("",TObject::kOverwrite);
  hWTau_MC_NoCJV_DPhi->Write("",TObject::kOverwrite);
  // Store cutflow hists
  hDataWTau->Write("",TObject::kOverwrite);
  hWLNuWTau->Write("",TObject::kOverwrite);
  hQCDWTau->Write("",TObject::kOverwrite);
  hDYWTau->Write("",TObject::kOverwrite);
  hSingleTWTau->Write("",TObject::kOverwrite);
  // Store final efficiencies, overall numbers, etc
  hWTau_TauIDEff_DPhi->Write("",TObject::kOverwrite);
  hWTau_CJVEff_DPhi->Write("",TObject::kOverwrite);
  hWTau_Est_NoCJV_DPhi->Write("",TObject::kOverwrite);
  hWTau_Est_DPhi ->Write("",TObject::kOverwrite);

  ofile->Close();    

}