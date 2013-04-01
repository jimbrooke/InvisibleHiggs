
#include "InvisibleHiggs/Analysis/interface/ProgramOptions.h"
#include "InvisibleHiggs/Analysis/interface/Cuts.h"
#include "InvisibleHiggs/Analysis/interface/Histogrammer.h"
#include "InvisibleHiggs/Analysis/interface/StackPlot.h"
#include "InvisibleHiggs/Analysis/interface/SumDatasets.h"
#include "InvisibleHiggs/Analysis/interface/Datasets.h"

#include "TTree.h"
#include "TMath.h"
#include "TH1D.h"

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
  TFile* ofile = TFile::Open( (options.oDir+std::string("/WBackground.root")).c_str(), "UPDATE");

  // cuts
  Cuts cuts;
  unsigned nCutsWMu = cuts.nCutsWMu();
  unsigned nCutsWEl = cuts.nCutsWEl();
  TCut puWeight("puWeight");

  // histograms
  double dphiEdges[4] = { 0., 1.0, 2.6, TMath::Pi() };

  TH1D* hWMu_MCC_DPhi = new TH1D("hWMu_MCC_DPhi", "", 3, dphiEdges);  // W+jets MC ctrl region
  TH1D* hWMu_MCS_DPhi = new TH1D("hWMu_MCS_DPhi", "", 3, dphiEdges);  // W+jets MC sgnl region
  TH1D* hWMu_BGC_DPhi = new TH1D("hWMu_BGC_DPhi", "", 3, dphiEdges);  // background MC ctrl region
  //  TH1D* hWMu_BGS_DPhi = new TH1D("hWMu_BGS_DPhi", "", 3, dphiEdges);  // background MC sgnl region
  TH1D* hWMu_DataC_DPhi = new TH1D("hWMu_DataC_DPhi", "", 3, dphiEdges);  // Data ctrl region

  TH1D* hWEl_MCC_DPhi = new TH1D("hWEl_MCC_DPhi", "", 3, dphiEdges);  // W+jets MC ctrl region
  TH1D* hWEl_MCS_DPhi = new TH1D("hWEl_MCS_DPhi", "", 3, dphiEdges);  // W+jets MC sgnl region
  TH1D* hWEl_BGC_DPhi = new TH1D("hWEl_BGC_DPhi", "", 3, dphiEdges);  // background MC ctrl region
  //  TH1D* hWEl_BGS_DPhi = new TH1D("hWEl_BGS_DPhi", "", 3, dphiEdges);  // background MC sgnl region
  TH1D* hWEl_DataC_DPhi = new TH1D("hWEl_DataC_DPhi", "", 3, dphiEdges);  // Data ctrl region

  // cutflow histograms
  TH1D* hDataWMu    = new TH1D("hWMu_CutFlow_Data", "", nCutsWMu, 0., nCutsWMu);
  TH1D* hWLNuWMu    = new TH1D("hWMu_CutFlow_WToLNu", "", nCutsWMu, 0., nCutsWMu);
  TH1D* hQCDWMu     = new TH1D("hWMu_CutFlow_QCD", "", nCutsWMu, 0., nCutsWMu);
  TH1D* hDYWMu      = new TH1D("hWMu_CutFlow_DY", "", nCutsWMu, 0., nCutsWMu);
  TH1D* hSingleTWMu = new TH1D("hWMu_CutFlow_SingleTSum", "", nCutsWMu, 0., nCutsWMu);
  TH1D* hDibosonWMu = new TH1D("hWMu_CutFlow_Diboson", "", nCutsWMu, 0., nCutsWMu);

  TH1D* hDataWEl    = new TH1D("hWEl_CutFlow_Data", "", nCutsWEl, 0., nCutsWEl);
  TH1D* hWLNuWEl    = new TH1D("hWEl_CutFlow_WToLNu", "", nCutsWEl, 0., nCutsWEl);
  TH1D* hQCDWEl     = new TH1D("hWEl_CutFlow_QCD", "", nCutsWEl, 0., nCutsWEl);
  TH1D* hDYWEl      = new TH1D("hWEl_CutFlow_DY", "", nCutsWEl, 0., nCutsWEl);
  TH1D* hSingleTWEl = new TH1D("hWEl_CutFlow_SingleTSum", "", nCutsWEl, 0., nCutsWEl);
  TH1D* hDibosonWEl = new TH1D("hWEl_CutFlow_Diboson", "", nCutsWEl, 0., nCutsWEl);
  

  // loop over MC datasets
  for (unsigned i=0; i<datasets.size(); ++i) {

    Dataset dataset = datasets.getDataset(i);
    
    // check it's  W+Jets
    bool isWJets = false;
    if (dataset.name == "WJets" ||
	dataset.name == "W1Jets" || 
	dataset.name == "W2Jets" || 
	dataset.name == "W3Jets" || 
	dataset.name == "W4Jets") {
      isWJets = true;
      std::cout << "Analysing W MC     : " << dataset.name << std::endl;
    }
    else if (dataset.isData) {
      std::cout << "Analysing Data     : " << dataset.name << std::endl;
    }
    else {
      std::cout << "Analysing BG MC    : " << dataset.name << std::endl;
    }

    // setup cuts
    TCut cutD = cuts.cutDataset(dataset.name);
    TCut cutWMu_Gen_C = puWeight * (cutD + cuts.wMuGen() + cuts.wMuVBF());
    TCut cutWMu_Gen_S = puWeight * (cutD + cuts.wMuGen() + cuts.allCutsNoDPhi());
    TCut cutWMu_C = puWeight * (cutD + cuts.wMuVBF());
    TCut cutWMu_S = puWeight * (cutD + cuts.allCutsNoDPhi());
    
    TCut cutWEl_Gen_C = puWeight * (cutD + cuts.wElGen() + cuts.wElVBF());
    TCut cutWEl_Gen_S = puWeight * (cutD + cuts.wElGen() + cuts.allCutsNoDPhi());
    TCut cutWEl_C = puWeight * (cutD + cuts.wElVBF());
    TCut cutWEl_S = puWeight * (cutD + cuts.allCutsNoDPhi());

    TFile* file = datasets.getTFile(dataset.name);
    TTree* tree = (TTree*) file->Get("invHiggsInfo/InvHiggsInfo");

    // tmp histograms
    TH1D* hWMu_C_DPhi = new TH1D("hWMu_C_DPhi", "", 3, dphiEdges);  // W+jets MC ctrl region
    TH1D* hWMu_S_DPhi = new TH1D("hWMu_S_DPhi", "", 3, dphiEdges);  // W+jets MC sgnl region
    TH1D* hWEl_C_DPhi = new TH1D("hWEl_C_DPhi", "", 3, dphiEdges);  // W+jets MC ctrl region
    TH1D* hWEl_S_DPhi = new TH1D("hWEl_S_DPhi", "", 3, dphiEdges);  // W+jets MC sgnl region

    double weight = lumi * dataset.sigma / dataset.nEvents;

    if (isWJets) {
      tree->Draw("vbfDPhi>>hWMu_C_DPhi", cutWMu_Gen_C);
      tree->Draw("vbfDPhi>>hWMu_S_DPhi", cutWMu_Gen_S);
      tree->Draw("vbfDPhi>>hWEl_C_DPhi", cutWEl_Gen_C);
      tree->Draw("vbfDPhi>>hWEl_S_DPhi", cutWEl_Gen_S);
      hWMu_MCC_DPhi->Add(hWMu_C_DPhi, weight);
      hWMu_MCS_DPhi->Add(hWMu_S_DPhi, weight);
      hWEl_MCC_DPhi->Add(hWEl_C_DPhi, weight);
      hWEl_MCS_DPhi->Add(hWEl_S_DPhi, weight);
    }
    else if (dataset.isData) {
      tree->Draw("vbfDPhi>>hWMu_C_DPhi", cutWMu_C);
      tree->Draw("vbfDPhi>>hWEl_C_DPhi", cutWEl_C);
      hWMu_DataC_DPhi->Add(hWMu_C_DPhi);
      hWEl_DataC_DPhi->Add(hWEl_C_DPhi);
    }
    else {  // must be a BG dataset
      tree->Draw("vbfDPhi>>hWMu_C_DPhi", cutWMu_C);
      //      tree->Draw("vbfDPhi>>hWMu_S_DPhi", cutWMu_S);
      tree->Draw("vbfDPhi>>hWEl_C_DPhi", cutWEl_C);
      //      tree->Draw("vbfDPhi>>hWEl_S_DPhi", cutWEl_S);
      hWMu_BGC_DPhi->Add(hWMu_C_DPhi, weight);
      //      hWMu_BGS_DPhi->Add(hWMu_S_DPhi, weight);
      hWEl_BGC_DPhi->Add(hWEl_C_DPhi, weight);
      //      hWEl_BGS_DPhi->Add(hWEl_S_DPhi, weight);
    }

    // debug output
    std::cout << "  N ctrl region (dphi<1) : " << hWMu_C_DPhi->GetBinContent(1) << " +/- " << hWMu_C_DPhi->GetBinError(1) << std::endl;
    
    delete hWMu_C_DPhi;
    delete hWMu_S_DPhi;
    delete hWEl_C_DPhi;
    delete hWEl_S_DPhi;

    // per-dataset control plots (just an example, add more later)
    ofile->cd();

    std::string hname = std::string("hWMu_WmT_")+dataset.name;
    TH1D* hWMu_WmT = new TH1D(hname.c_str(), "", 40, 0., 120.);
    std::string str = std::string("wMt>>")+hname;
    tree->Draw(str.c_str(), cutWMu_C);
    hWMu_WmT->Write("",TObject::kOverwrite);

    hname = std::string("hWEl_WmT_")+dataset.name;
    TH1D* hWEl_WmT = new TH1D(hname.c_str(), "", 40, 0., 120.);
    str = std::string("wMt>>")+hname;
    tree->Draw(str.c_str(), cutWEl_C);
    hWEl_WmT->Write("",TObject::kOverwrite);

    // per-dataset cutflow hists
    std::string hnameWMu = std::string("hWMu_CutFlow_")+dataset.name;
    std::string hnameWEl = std::string("hWEl_CutFlow_")+dataset.name;
    TH1D* hCutFlowWMu = new TH1D(hnameWMu.c_str(), "", nCutsWMu, 0., nCutsWMu);
    TH1D* hCutFlowWEl = new TH1D(hnameWEl.c_str(), "", nCutsWEl, 0., nCutsWEl);

    for (unsigned c=0; c<nCutsWMu; ++c) {

      TCut cut = puWeight * (cutD + cuts.cutflowWMu(c));
      TH1D* h = new TH1D("h","", 1, 0., 1.);
      tree->Draw("0.5>>h", cut);

      hCutFlowWMu->SetBinContent(c+1, h->GetBinContent(1));
      hCutFlowWMu->SetBinError(c+1, h->GetBinError(1));

      delete h;
    }

    for (unsigned c=0; c<nCutsWEl; ++c) {

      TCut cut = puWeight * (cutD + cuts.cutflowWEl(c));
      TH1D* h = new TH1D("h","", 1, 0., 1.);
      tree->Draw("0.5>>h", cut);

      hCutFlowWEl->SetBinContent(c+1, h->GetBinContent(1));
      hCutFlowWEl->SetBinError(c+1, h->GetBinError(1));

      delete h;
    }

    // sum histograms
    if (dataset.isData) {
      hDataWMu->Add(hCutFlowWMu, 1.);
      hDataWEl->Add(hCutFlowWEl, 1.);
    }
    if (isWJets) {
      hWLNuWMu->Add(hCutFlowWMu, weight);
      hWLNuWEl->Add(hCutFlowWEl, weight);
    }
    if (dataset.name.compare(0,3,"QCD")==0) {
      hQCDWMu->Add(hCutFlowWMu, weight);
      hQCDWEl->Add(hCutFlowWEl, weight);
    }
    if (dataset.name.compare(0,2,"DY")==0) {
      hDYWMu->Add(hCutFlowWMu, weight);
      hDYWEl->Add(hCutFlowWEl, weight);
    }
    if (dataset.name.compare(0,7,"SingleT")==0) {
      hSingleTWMu->Add(hCutFlowWMu, weight);
      hSingleTWEl->Add(hCutFlowWEl, weight);
    }
    if (dataset.name.compare(0,2,"WW")==0 ||
	dataset.name.compare(0,2,"WZ")==0 ||
	dataset.name.compare(0,2,"ZZ")==0 ) {
      hDibosonWMu->Add(hCutFlowWMu, weight);
      hDibosonWEl->Add(hCutFlowWEl, weight);
    }

    hCutFlowWMu->Write("",TObject::kOverwrite);
    hCutFlowWEl->Write("",TObject::kOverwrite);

    delete tree;
    file->Close();
   
  }

  // write out summed cutflow histograms
  hDataWMu->Write("",TObject::kOverwrite);  
  hWLNuWMu->Write("",TObject::kOverwrite);  
  hQCDWMu->Write("",TObject::kOverwrite);  
  hDYWMu->Write("",TObject::kOverwrite);  
  hSingleTWMu->Write("",TObject::kOverwrite);  
  hDibosonWMu->Write("",TObject::kOverwrite);  

  hDataWEl->Write("",TObject::kOverwrite);  
  hWLNuWEl->Write("",TObject::kOverwrite);  
  hQCDWEl->Write("",TObject::kOverwrite);  
  hDYWEl->Write("",TObject::kOverwrite);  
  hSingleTWEl->Write("",TObject::kOverwrite);  
  hDibosonWEl->Write("",TObject::kOverwrite);  


  // create histograms with the background estimate
  TH1D* hWMu_R_DPhi    = new TH1D("hWMu_R_DPhi", "", 3, dphiEdges);  // ratio of sngl/ctrl
  TH1D* hWMu_EstC_DPhi = new TH1D("hWMu_EstC_DPhi", "", 3, dphiEdges); // estimated W in ctrl region
  TH1D* hWMu_EstS_DPhi = new TH1D("hWMu_EstS_DPhi", "", 3, dphiEdges); // estimated W in signal region

  TH1D* hWEl_R_DPhi    = new TH1D("hWEl_R_DPhi", "", 3, dphiEdges);
  TH1D* hWEl_EstC_DPhi = new TH1D("hWEl_EstC_DPhi", "", 3, dphiEdges);
  TH1D* hWEl_EstS_DPhi = new TH1D("hWEl_EstS_DPhi", "", 3, dphiEdges);

  hWMu_R_DPhi->Divide(hWMu_MCS_DPhi, hWMu_MCC_DPhi, 1., 1.);
  hWMu_EstC_DPhi->Add(hWMu_DataC_DPhi, hWMu_BGC_DPhi, 1., -1.);
  hWMu_EstS_DPhi->Multiply(hWMu_EstC_DPhi, hWMu_R_DPhi, 1., 1.);

  hWEl_R_DPhi->Divide(hWEl_MCS_DPhi, hWEl_MCC_DPhi, 1., 1.);
  hWEl_EstC_DPhi->Add(hWEl_DataC_DPhi, hWEl_BGC_DPhi, 1., -1.);
  hWEl_EstS_DPhi->Multiply(hWEl_EstC_DPhi, hWEl_R_DPhi, 1., 1.);

  TH1D* hW_Est_S_DPhi = new TH1D("hW_Est_S_DPhi", "", 3, dphiEdges); 
  hW_Est_S_DPhi->Add(hWMu_EstS_DPhi, hWEl_EstS_DPhi, 1., 1.);


  std::cout << std::endl;
  std::cout << "W->mu channel (dphi>2.6)" << std::endl;
  std::cout << "  W+jets MC  ctrl region : " << hWMu_MCC_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  W+jets MC  sgnl region : " << hWMu_MCS_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  Background ctrl region : " << hWMu_BGC_DPhi->GetBinContent(3) << std::endl;
  //  std::cout << "  Background sgnl region : " << hWMu_BGS_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  Data ctrl region       : " << hWMu_DataC_DPhi->GetBinContent(3) << std::endl;
  std::cout << std::endl;
  std::cout << "  W in ctrl region       : " << hWMu_EstC_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  N_S(MC)/N_C(MC)        : " << hWMu_R_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  W in sgnl region       : " << hWMu_EstS_DPhi->GetBinContent(3) << std::endl;
  std::cout << std::endl << std::endl;
  std::cout << "W->el channel (dphi>2.6)" << std::endl;
  std::cout << "  W+jets MC  ctrl region : " << hWEl_MCC_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  W+jets MC  sgnl region : " << hWEl_MCS_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  Background ctrl region : " << hWEl_BGC_DPhi->GetBinContent(3) << std::endl;
  //  std::cout << "  Background sgnl region : " << hWEl_BGS_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  Data ctrl region       : " << hWEl_DataC_DPhi->GetBinContent(3) << std::endl;
  std::cout << std::endl;
  std::cout << "  W in ctrl region       : " << hWEl_EstC_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  N_S(MC)/N_C(MC)        : " << hWEl_R_DPhi->GetBinContent(3) << std::endl;
  std::cout << "  W in sgnl region       : " << hWEl_EstS_DPhi->GetBinContent(3) << std::endl;
  std::cout << std::endl << std::endl;
  std::cout << "W->mu channel (dphi<1.0)" << std::endl;
  std::cout << "  W+jets MC  ctrl region : " << hWMu_MCC_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  W+jets MC  sgnl region : " << hWMu_MCS_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  Background ctrl region : " << hWMu_BGC_DPhi->GetBinContent(1) << std::endl;
  //  std::cout << "  Background sgnl region : " << hWMu_BGS_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  Data ctrl region       : " << hWMu_DataC_DPhi->GetBinContent(1) << std::endl;
  std::cout << std::endl;
  std::cout << "  W in ctrl region       : " << hWMu_EstC_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  N_S(MC)/N_C(MC)        : " << hWMu_R_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  W in sgnl region       : " << hWMu_EstS_DPhi->GetBinContent(1) << std::endl;
  std::cout << std::endl << std::endl;
  std::cout << "W->el channel (dphi<1.0)" << std::endl;
  std::cout << "  W+jets MC  ctrl region : " << hWEl_MCC_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  W+jets MC  sgnl region : " << hWEl_MCS_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  Background ctrl region : " << hWEl_BGC_DPhi->GetBinContent(1) << std::endl;
  //  std::cout << "  Background sgnl region : " << hWEl_BGS_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  Data ctrl region       : " << hWEl_DataC_DPhi->GetBinContent(1) << std::endl;
  std::cout << std::endl;
  std::cout << "  W in ctrl region       : " << hWEl_EstC_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  N_S(MC)/N_C(MC)        : " << hWEl_R_DPhi->GetBinContent(1) << std::endl;
  std::cout << "  W in sgnl region       : " << hWEl_EstS_DPhi->GetBinContent(1) << std::endl;
  std::cout << std::endl;
  std::cout << "Total W (dphi<1.0)" << std::endl;
  std::cout << "  W in sgnl region       : " << hW_Est_S_DPhi->GetBinContent(1) << std::endl;

  // store histograms
  ofile->cd();
    
  hWMu_MCC_DPhi->Write("",TObject::kOverwrite);
  hWMu_MCS_DPhi->Write("",TObject::kOverwrite);
  hWMu_BGC_DPhi->Write("",TObject::kOverwrite);
  //  hWMu_BGS_DPhi->Write("",TObject::kOverwrite);
  hWMu_DataC_DPhi->Write("",TObject::kOverwrite);
  hWMu_R_DPhi->Write("",TObject::kOverwrite);
  hWMu_EstC_DPhi->Write("",TObject::kOverwrite);
  hWMu_EstS_DPhi->Write("",TObject::kOverwrite);

  hWEl_MCC_DPhi->Write("",TObject::kOverwrite);
  hWEl_MCS_DPhi->Write("",TObject::kOverwrite);
  hWEl_BGC_DPhi->Write("",TObject::kOverwrite);
  //  hWEl_BGS_DPhi->Write("",TObject::kOverwrite);
  hWEl_DataC_DPhi->Write("",TObject::kOverwrite);
  hWEl_R_DPhi->Write("",TObject::kOverwrite);
  hWEl_EstC_DPhi->Write("",TObject::kOverwrite);
  hWEl_EstS_DPhi->Write("",TObject::kOverwrite);

  hW_Est_S_DPhi->Write("",TObject::kOverwrite);

  // write the cutflow table
  std::cout << "Writing cut flow TeX file" << std::endl;

  ofstream effFile;
  effFile.open(options.oDir+std::string("/cutflowWMu.tex"));

  effFile << "Cut & N(data) & N($W\\rightarrow l\\nu$) & N(DY) & N(QCD) & N($t\\bar{t}$) & N(single $t$) & N(diboson) \\\\" << std::endl;

  TH1D* hTTbarWMu = (TH1D*) ofile->Get("hWMu_CutFlow_TTBar");

  // cutflow table
  for (unsigned i=0; i<nCutsWMu; ++i) {

    effFile << cuts.cutNameWMu(i) << " & ";
    effFile << "$" << hDataWMu->GetBinContent(i+1) << " \\pm " << hDataWMu->GetBinError(i+1) << "$ & ";
    effFile << "$" << hWLNuWMu->GetBinContent(i+1) << " \\pm " << hWLNuWMu->GetBinError(i+1) << "$ & ";
    effFile << "$" << hDYWMu->GetBinContent(i+1) << " \\pm " << hDYWMu->GetBinError(i+1) << "$ & ";
    effFile << "$" << hQCDWMu->GetBinContent(i+1) << " \\pm " << hQCDWMu->GetBinError(i+1) << "$ & ";
    effFile << "$" << hTTbarWMu->GetBinContent(i+1) << " \\pm " << hTTbarWMu->GetBinError(i+1) << "$ & ";
    effFile << "$" << hSingleTWMu->GetBinContent(i+1) << " \\pm " << hSingleTWMu->GetBinError(i+1) << "$ & ";
    effFile << "$" << hDibosonWMu->GetBinContent(i+1) << " \\pm " << hDibosonWMu->GetBinError(i+1) << "$ \\\\ ";
    effFile << std::endl;

  }

  effFile << std::endl << std::endl;
  effFile.close();

  //WEl cutflow
  effFile.open(options.oDir+std::string("/cutflowWEl.tex"));

  effFile << "Cut & N(data) & N($W\\rightarrow l\\nu$) & N(DY) & N(QCD) & N($t\\bar{t}$) & N(single $t$) & N(diboson) \\\\" << std::endl;

  TH1D* hTTbarWEl = (TH1D*) ofile->Get("hWEl_CutFlow_TTBar");

  // cutflow table
  for (unsigned i=0; i<nCutsWEl; ++i) {

    effFile << cuts.cutNameWEl(i) << " & ";
    effFile << "$" << hDataWEl->GetBinContent(i+1) << " \\pm " << hDataWEl->GetBinError(i+1) << "$ & ";
    effFile << "$" << hWLNuWEl->GetBinContent(i+1) << " \\pm " << hWLNuWEl->GetBinError(i+1) << "$ & ";
    effFile << "$" << hDYWEl->GetBinContent(i+1) << " \\pm " << hDYWEl->GetBinError(i+1) << "$ & ";
    effFile << "$" << hQCDWEl->GetBinContent(i+1) << " \\pm " << hQCDWEl->GetBinError(i+1) << "$ & ";
    effFile << "$" << hTTbarWEl->GetBinContent(i+1) << " \\pm " << hTTbarWEl->GetBinError(i+1) << "$ & ";
    effFile << "$" << hSingleTWEl->GetBinContent(i+1) << " \\pm " << hSingleTWEl->GetBinError(i+1) << "$ & ";
    effFile << "$" << hDibosonWEl->GetBinContent(i+1) << " \\pm " << hDibosonWEl->GetBinError(i+1) << "$ \\\\ ";
    effFile << std::endl;
  }

  effFile << std::endl << std::endl;
  effFile.close();



  ofile->Close();    

}
