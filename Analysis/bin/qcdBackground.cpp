#include "InvisibleHiggs/Analysis/interface/ProgramOptions.h"
#include "InvisibleHiggs/Analysis/interface/Cuts.h"
#include "InvisibleHiggs/Analysis/interface/Histogrammer.h"
#include "InvisibleHiggs/Analysis/interface/StackPlot.h"
#include "InvisibleHiggs/Analysis/interface/SumDatasets.h"
#include "InvisibleHiggs/Analysis/interface/Datasets.h"

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

  // input datasets
  Datasets datasets(options.iDir);
  datasets.readFile(options.datasetFile);

  std::string oDir_Plot = options.oDir+std::string("/QCDBackground");
  
  boost::filesystem::path opath(oDir_Plot);
  if (!exists(opath)) {
    std::cout << "Creating output directory : " << oDir_Plot << std::endl;
    boost::filesystem::create_directory(opath);
  }

  // output file
  TFile* ofile = TFile::Open( (options.oDir+std::string("/QCDBackground.root")).c_str(), "RECREATE");

  // cuts
  Cuts cuts;
  TCut puWeight("puWeight");
  TCut trigCorr( "(trigCorrWeight>0.) ? trigCorrWeight : 1." );
 
  // histograms
  double dphiEdges[4] = { 0., 1.0, 2.6, TMath::Pi() };
  double metEdges[14] = { 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 110., 120., 10000. };

  TH2D* hQCD_BG_METDPhi = new TH2D("hQCD_BG_METDPhi", "", 3, dphiEdges, 13, metEdges);  // BG in NoMET region
  TH1D* hQCD_BG_Tight_DPhi = new TH1D("hQCD_BG_Tight_DPhi", "", 3, dphiEdges);  // BG in tight region

  TH2D* hQCD_WTau_METDPhi = new TH2D("hQCD_WTau_METDPhi", "", 3, dphiEdges, 13, metEdges);  // BG in NoMET region
  TH1D* hQCD_WTau_Tight_DPhi = new TH1D("hQCD_WTau_Tight_DPhi", "", 3, dphiEdges);  // BG in tight region

  TH2D* hQCD_TTBar_METDPhi = new TH2D("hQCD_TTBar_METDPhi", "", 3, dphiEdges, 13, metEdges);  // BG in NoMET region
  TH1D* hQCD_TTBar_Tight_DPhi = new TH1D("hQCD_TTBar_Tight_DPhi", "", 3, dphiEdges);  // BG in tight region

  TH2D* hQCD_SingleTSum_METDPhi = new TH2D("hQCD_SingleTSum_METDPhi", "", 3, dphiEdges, 13, metEdges);  // BG in NoMET region
  TH1D* hQCD_SingleTSum_Tight_DPhi = new TH1D("hQCD_SingleTSum_Tight_DPhi", "", 3, dphiEdges);  // BG in tight region

  TH2D* hQCD_DY_METDPhi = new TH2D("hQCD_DY_METDPhi", "", 3, dphiEdges, 13, metEdges);  // BG in NoMET region
  TH1D* hQCD_DY_Tight_DPhi = new TH1D("hQCD_DY_Tight_DPhi", "", 3, dphiEdges);  // BG in tight region

  TH2D* hQCD_Diboson_METDPhi = new TH2D("hQCD_Diboson_METDPhi", "", 3, dphiEdges, 13, metEdges);  // BG in NoMET region
  TH1D* hQCD_Diboson_Tight_DPhi = new TH1D("hQCD_Diboson_Tight_DPhi", "", 3, dphiEdges);  // BG in tight region

  TH2D* hQCD_Data_METDPhi = new TH2D("hQCD_Data_METDPhi", "", 3, dphiEdges, 13, metEdges);  // Data in NoMET region
  TH1D* hQCD_Data_Tight_DPhi = new TH1D("hQCD_Data_Tight_DPhi", "", 3, dphiEdges);  // Data in tight region

  // loop over MC datasets
  for (unsigned i=0; i<datasets.size(); ++i) {

    Dataset dataset = datasets.getDataset(i);

    TCut cutD = cuts.cutDataset(dataset.name);
    
    // check if it's ZToNuNuJets
    bool isZvv = false;

    if (dataset.isData) {
      std::cout << "Analysing Data         : " << dataset.name << std::endl;
    }
    else if (dataset.name.compare(0,3,"Zvv")==0) {
      isZvv = true;
      std::cout << "Analysing Zvv MC       : " << dataset.name << std::endl;
    }
    else {
      std::cout << "Analysing BG MC        : " << dataset.name << std::endl;
    }

    // get file & tree
    TFile* file = datasets.getTFile(dataset.name);
    TTree* tree = (TTree*) file->Get("invHiggsInfo/InvHiggsInfo");

    // cuts
    TCut cutQCDNoMET = puWeight * trigCorr * cuts.qcdNoMET();
    TCut cutQCDTightHiDPhi = puWeight * trigCorr * cuts.qcdTightHiDPhi();
    
    TCut cutWQCDNoMET = puWeight * trigCorr * cuts.wWeight() * (cuts.wTauGen() + cuts.qcdNoMET());
    TCut cutWQCDTightHiDPhi = puWeight * trigCorr * cuts.wWeight() * (cuts.wTauGen() + cuts.qcdTightHiDPhi());

    // fill tmp histograms for BG estimation
    TH2D* hQCD_METDPhi  = new TH2D("hQCD_METDPhi", "", 3, dphiEdges, 13, metEdges);  // 
    TH1D* hQCD_Tight_DPhi  = new TH1D("hQCD_Tight_DPhi", "", 3, dphiEdges);  // this is for the actual BG estimation

    if (dataset.name=="WJets" ||
	dataset.name=="W1Jets" ||
	dataset.name=="W2Jets" ||
	dataset.name=="W3Jets" ||
	dataset.name=="W4Jets") {
      tree->Draw("met:vbfDPhi>>hQCD_METDPhi", cutWQCDNoMET);
      tree->Draw("vbfDPhi>>hQCD_Tight_DPhi", cutWQCDTightHiDPhi);
    }
    else {
      tree->Draw("met:vbfDPhi>>hQCD_METDPhi", cutQCDNoMET);
      tree->Draw("vbfDPhi>>hQCD_Tight_DPhi", cutQCDTightHiDPhi);
    }

    // weight  to lumi
    double weight = (dataset.isData ? 1. : lumi * dataset.sigma / dataset.nEvents);

    hQCD_METDPhi->Scale(weight);
    hQCD_Tight_DPhi->Scale(weight);

    // add to output histograms
    if (dataset.isData) {
      hQCD_Data_METDPhi->Add(hQCD_METDPhi);
      hQCD_Data_Tight_DPhi->Add(hQCD_Tight_DPhi);
    }
    else {
      if (!isZvv) hQCD_BG_Tight_DPhi->Add(hQCD_Tight_DPhi); // do not include Z->vv samples we use data-driven number
      hQCD_BG_METDPhi->Add(hQCD_METDPhi);
    }

    if (dataset.name=="WJets" ||
	dataset.name=="W1Jets" ||
	dataset.name=="W2Jets" ||
	dataset.name=="W3Jets" ||
	dataset.name=="W4Jets" ) {
      hQCD_WTau_METDPhi->Add(hQCD_METDPhi);
      hQCD_WTau_Tight_DPhi->Add(hQCD_Tight_DPhi);
    }

    if (dataset.name=="TTBar") {
      hQCD_TTBar_METDPhi->Add(hQCD_METDPhi);
      hQCD_TTBar_Tight_DPhi->Add(hQCD_Tight_DPhi);
    }

    if (dataset.name.compare(0,7,"SingleT")==0) {
      hQCD_SingleTSum_METDPhi->Add(hQCD_METDPhi);
      hQCD_SingleTSum_Tight_DPhi->Add(hQCD_Tight_DPhi);
    }

    if (dataset.name.compare(0,2,"DY")==0) {
      hQCD_DY_METDPhi->Add(hQCD_METDPhi);
      hQCD_DY_Tight_DPhi->Add(hQCD_Tight_DPhi);
    }

    if (dataset.name == "WW" ||
	dataset.name == "WZ" ||
	dataset.name == "ZZ") {
      hQCD_Diboson_METDPhi->Add(hQCD_METDPhi);
      hQCD_Diboson_Tight_DPhi->Add(hQCD_Tight_DPhi);
    }     

    std::cout << "  N (met>130, dphi>2.6) : " << hQCD_Tight_DPhi->GetBinContent(3) << std::endl;

    delete hQCD_METDPhi;
    delete hQCD_Tight_DPhi;

    // QCD Figure 13 in AN
    TCut cutPlots("");

    if (dataset.name == "WJets"  ||
        dataset.name == "W1Jets" ||
        dataset.name == "W2Jets" ||
        dataset.name == "W3Jets" ||
        dataset.name == "W4Jets") {
		cutPlots = puWeight * trigCorr * cuts.wWeight() * (cuts.HLTandMETFilters() + cuts.leptonVeto() + cuts.vbf());  //no MET and dPhijj
    }
    else 	cutPlots = puWeight * trigCorr * (cuts.HLTandMETFilters() + cuts.leptonVeto() + cuts.vbf());  //no MET and dPhijj

    TFile* ofile_Plot = TFile::Open( (oDir_Plot+std::string("/")+dataset.name+std::string(".root")).c_str(), "RECREATE");

    TH1D* QCD_DPhijj 	= new TH1D("QCD_DPhijj",    "", 50, 0.,  TMath::Pi());
    TH1D* QCD_MET	= new TH1D("QCD_MET",       "", 50, 0.,  1000.);

    tree->Draw("vbfDPhi>>QCD_DPhijj"	, cutPlots);
    tree->Draw("met>>QCD_MET"		, cutPlots);

    if (!dataset.isData) {
    QCD_DPhijj->Scale(weight);
    QCD_MET->Scale(weight);
    }

    ofile_Plot->cd();

    QCD_DPhijj->Write("",TObject::kOverwrite);
    QCD_MET->Write("",TObject::kOverwrite);

    ofile_Plot->Close();

    // clean up
    delete tree;
    file->Close();
   
  }

  // get data-driven background estimates
  std::cout << "Reading Z backgrounds from : " << options.oDir+std::string("/ZBackground.root") << std::endl;
  TFile* zfile = TFile::Open( (options.oDir+std::string("/ZBackground.root")).c_str(), "READ");
  TH2D* hQCD_Z_METDPhi  = (TH2D*) zfile->Get("hZ_Est_S_METDPhi");
  TH1D* hQCD_Z_Tight_DPhi  = (TH1D*) zfile->Get("hZ_Est_S_DPhi");

  if (hQCD_Z_Tight_DPhi == 0) {
    std::cerr << "Could not read Z background histogram" << std::endl;
    std::exit(1);
  }

  std::cout << "  N (met>130, dphi>2.6) : " << hQCD_Z_Tight_DPhi->GetBinContent(3) << std::endl;
  std::cout << std::endl;
  
  std::cout << "Reading W backgrounds from : " << options.oDir+std::string("/WBackground.root") << std::endl;
  TFile* wfile = TFile::Open( (options.oDir+std::string("/WBackground.root")).c_str(), "READ");
  TH2D* hQCD_W_METDPhi  = (TH2D*) wfile->Get("hW_Est_S_METDPhi");
  TH1D* hQCD_W_Tight_DPhi  = (TH1D*) wfile->Get("hW_Est_S_DPhi");

  if (hQCD_W_Tight_DPhi == 0) {
    std::cerr << "Could not read W background histogram" << std::endl;
    std::exit(1);
  }  

  std::cout << "  N (met>130, dphi>2.6) : " << hQCD_W_Tight_DPhi->GetBinContent(3) << std::endl;
  std::cout << std::endl;
  

  // create output histograms
  TH2D* hQCD_Est_METDPhi  = new TH2D("hQCD_Est_METDPhi", "", 3, dphiEdges, 13, metEdges);
  TH1D* hQCD_Est_S_DPhi      = new TH1D("hQCD_Est_S_DPhi",     "", 3, dphiEdges);

  // do the background estimation
  hQCD_Est_METDPhi->Add(hQCD_Data_METDPhi, hQCD_BG_METDPhi, 1., -1.);
  hQCD_Est_S_DPhi->Add(hQCD_Data_Tight_DPhi, hQCD_BG_Tight_DPhi, 1., -1.);

  //  hQCD_Est_METDPhi->Add(hQCD_Z_METDPhi, -1.);
  hQCD_Est_S_DPhi->Add(hQCD_Z_Tight_DPhi, -1.);

  //  hQCD_Est_METDPhi->Add(hQCD_W_METDPhi, -1.);

  hQCD_Est_S_DPhi->Add(hQCD_W_Tight_DPhi, -1.);

  // calculate ratios
//   double rNoMET      = hQCD_Est_METDPhi->GetBinContent(1) / hQCD_Est_METDPhi->GetBinContent(3);
//   double err_rNoMET  = rNoMET * sqrt(pow(hQCD_Est_METDPhi->GetBinError(1)/hQCD_Est_METDPhi->GetBinContent(1),2) + pow(hQCD_Est_METDPhi->GetBinError(3)/hQCD_Est_METDPhi->GetBinContent(3),2));
//   double rLoose2     = hQCD_Est_Loose2_DPhi->GetBinContent(1) / hQCD_Est_Loose2_DPhi->GetBinContent(3);
//   double err_rLoose2 = rLoose2 * sqrt(pow(hQCD_Est_Loose2_DPhi->GetBinError(1)/hQCD_Est_Loose2_DPhi->GetBinContent(1),2) + pow(hQCD_Est_Loose2_DPhi->GetBinError(3)/hQCD_Est_Loose2_DPhi->GetBinContent(3),2));
//   double rLoose      = hQCD_Est_Loose_DPhi->GetBinContent(1) / hQCD_Est_Loose_DPhi->GetBinContent(3);
//   double err_rLoose  = rLoose * sqrt(pow(hQCD_Est_Loose_DPhi->GetBinError(1)/hQCD_Est_Loose_DPhi->GetBinContent(1),2) + pow(hQCD_Est_Loose_DPhi->GetBinError(3)/hQCD_Est_Loose_DPhi->GetBinContent(3),2));
  
//   // linear extrapolation using MET>80 and MET>100 bins
  double rTight = 0.0046;
  double err_rTight = 0.0046 * 1.5;

  // predict signal
  double nQCD_Est_S_HiDPhi = rTight * hQCD_Est_S_DPhi->GetBinContent(3);
  double err_nQCD_Est_S_HiDPhi = nQCD_Est_S_HiDPhi * sqrt(pow(err_rTight/rTight,2)+pow(hQCD_Est_S_DPhi->GetBinError(3)/hQCD_Est_S_DPhi->GetBinContent(3),2));

  // set bins in output histogram
  hQCD_Est_S_DPhi->SetBinContent(1, nQCD_Est_S_HiDPhi);
  hQCD_Est_S_DPhi->SetBinError(1, err_nQCD_Est_S_HiDPhi);

  // print results
  std::cout << std::endl;
  std::cout << "QCD Background estimate" << std::endl << std::endl;
  std::cout << std::endl;

  std::cout <<" Ratios" << std::endl;
//   std::cout << "   R(MET>80)   : " << rNoMET << " +/- " << err_rNoMET << std::endl;
//   std::cout << "   R(MET>90)   : " << rLoose2 << " +/- " << err_rLoose2 << std::endl;
//   std::cout << "   R(MET>100)   : " << rLoose << " +/- " << err_rLoose << std::endl;
//   std::cout << "   R(MET>130)  : " << rTight << " +/- " << err_rTight << std::endl;
//   std::cout << std::endl;

  std::cout << "Control region (MET>130, dphi>2.6)" << std::endl;
  std::cout << "   N data      : " << hQCD_Data_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_Data_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N Z (data)  : " << hQCD_Z_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_Z_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N W (data)  : " << hQCD_W_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_W_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N BG (MC)   : " << hQCD_BG_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_BG_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N WTau (MC)     : " << hQCD_WTau_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_WTau_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N TTbar (MC)    : " << hQCD_TTBar_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_TTBar_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N SingleT (MC)  : " << hQCD_SingleTSum_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_SingleTSum_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N DY (MC)       : " << hQCD_DY_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_DY_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N Diboson (MC)  : " << hQCD_Diboson_Tight_DPhi->GetBinContent(3) << " +/- " << hQCD_Diboson_Tight_DPhi->GetBinError(3) << std::endl;
  std::cout << "   N QCD (est) : " << hQCD_Est_S_DPhi->GetBinContent(3) << " +/- " << hQCD_Est_S_DPhi->GetBinError(3) << std::endl;
  std::cout << std::endl;
  std::cout << "Signal region (MET>130, dphi<1.0)" << std::endl;
  std::cout << "   N QCD (est): " << hQCD_Est_S_DPhi->GetBinContent(1) << " +/- " << hQCD_Est_S_DPhi->GetBinError(1) << std::endl;

  // list histograms for dataset summing
  std::vector<std::string> hists;
  hists.push_back("QCD_DPhijj");
  hists.push_back("QCD_MET");

  // sum Z+jets datasets
  std::vector<std::string> zjetsDatasets;
  zjetsDatasets.push_back(std::string("Zvv_50to100"));
  zjetsDatasets.push_back(std::string("Zvv_100to200"));
  zjetsDatasets.push_back(std::string("Zvv_200to400"));
  zjetsDatasets.push_back(std::string("Zvv_400toinf"));
  SumDatasets(oDir_Plot, zjetsDatasets, hists, "ZJets");

  // sum W+jets datasets
  std::vector<std::string> wjetsDatasets;
  wjetsDatasets.push_back(std::string("WJets"));
  wjetsDatasets.push_back(std::string("W1Jets"));
  wjetsDatasets.push_back(std::string("W2Jets"));
  wjetsDatasets.push_back(std::string("W3Jets"));
  wjetsDatasets.push_back(std::string("W4Jets"));
  SumDatasets(oDir_Plot, wjetsDatasets, hists, "WNJets");

  // sum single top datasets
  std::vector<std::string> topDatasets;
  topDatasets.push_back(std::string("SingleT_t"));
  topDatasets.push_back(std::string("SingleTbar_t"));
  topDatasets.push_back(std::string("SingleT_s"));
  topDatasets.push_back(std::string("SingleTbar_s"));
  topDatasets.push_back(std::string("SingleT_tW"));
  topDatasets.push_back(std::string("SingleTbar_tW"));
  topDatasets.push_back(std::string("TTBar"));
  SumDatasets(oDir_Plot, topDatasets, hists, "SingleT+TTbar");

  // sum DY contributions
  std::cout << "Summing histograms for DYJetsToLL" << std::endl;
  std::vector<std::string> dyjets;
  dyjets.push_back("DYJetsToLL");
  dyjets.push_back("DYJetsToLL_EWK");
  SumDatasets(oDir_Plot,dyjets,hists,"DYJets");

  // sum diboson datasets
  std::vector<std::string> dibDatasets;
  dibDatasets.push_back(std::string("WW"));
  dibDatasets.push_back(std::string("WZ"));
  dibDatasets.push_back(std::string("ZZ"));
  SumDatasets(oDir_Plot, dibDatasets, hists, "DiBoson");

  // make plots
  std::cout << "Making plots" << std::endl;
  StackPlot plots(oDir_Plot);
  plots.setLegPos(0.66,0.60,0.89,0.89);

  plots.addDataset("DiBoson", kViolet-6, 0);
  plots.addDataset("DYJets", kPink-4,0);
  plots.addDataset("SingleT+TTbar", kAzure-2, 0);
  plots.addDataset("ZJets", kOrange-2, 0);
  plots.addDataset("WNJets", kGreen-3, 0);
  plots.addDataset("METABCD",    kBlack, 1);

  plots.setYMax(1e+8); 
  plots.draw("QCD_DPhijj",	"#Delta #phi_{jj}",	"N_{events}"    ,1,"RATIO");
  plots.draw("QCD_MET",		"E_{T}^{miss} [GeV]",	"N_{events}"    ,1,"RATIO");

  // write out histograms
  ofile->cd();
  hQCD_Data_METDPhi->Write("", TObject::kOverwrite);
  hQCD_Data_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_BG_METDPhi->Write("", TObject::kOverwrite);
  hQCD_BG_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_Z_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_W_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_WTau_METDPhi->Write("", TObject::kOverwrite);
  hQCD_WTau_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_TTBar_METDPhi->Write("", TObject::kOverwrite);
  hQCD_TTBar_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_SingleTSum_METDPhi->Write("", TObject::kOverwrite);
  hQCD_SingleTSum_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_DY_METDPhi->Write("", TObject::kOverwrite);
  hQCD_DY_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_Diboson_METDPhi->Write("", TObject::kOverwrite);
  hQCD_Diboson_Tight_DPhi->Write("", TObject::kOverwrite);
  hQCD_Est_METDPhi->Write("", TObject::kOverwrite);
  hQCD_Est_S_DPhi->Write("", TObject::kOverwrite);

  // clean up
  ofile->Close();

}
