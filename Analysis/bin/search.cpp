
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

namespace po = boost::program_options;

int main(int argc, char* argv[]) {

  // setup
  TH1::SetDefaultSumw2();

  ProgramOptions options(argc, argv);

  double lumi = options.lumi;
  std::string iDir = options.iDir;
  std::string oDir = options.oDir+std::string("/Signal");

  if (oDir!="") {
    boost::filesystem::path opath(oDir);
    if (!exists(opath)) {
      std::cout << "Creating output directory : " << oDir << std::endl;
      boost::filesystem::create_directory(opath);
    }
    std::cout << "Writing results to " << oDir << std::endl;
  }

  Datasets datasets(iDir);
  datasets.readFile(options.datasetFile);

  Cuts cuts;

  TFile* ofile = TFile::Open((oDir+std::string("/Search.root")).c_str(), "RECREATE");

  TH1D* hCounts = new TH1D("hCounts", "", 1, 0., 1.);

  // now do something
  for (unsigned i=0; i<datasets.size(); ++i) {
    
    Dataset dataset = datasets.getDataset(i);
    
    if (!dataset.isData) continue;

    TFile* ifile = datasets.getTFile(dataset.name);
    TTree* tree = (TTree*) ifile->Get("invHiggsInfo/InvHiggsInfo");

    TH1D* hTmp = new TH1D("hTmp", "", 1, 0., 1.);

    tree->Draw("0.5>>hTmp", cuts.allCuts());

    hCounts->Add(hTmp);
  }

  std::cout << "Yield in signal region : " << hCounts->GetBinContent(1);
  std::cout << std::endl;

  // write out histograms
  ofile->cd();
  hCounts->Write("",TObject::kOverwrite);

  // clean up
  ofile->Close();

}
