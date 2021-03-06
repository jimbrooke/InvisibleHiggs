#ifndef Cuts_h
#define Cuts_h

#include "TCut.h"
#include <iostream>

class Cuts {
 public:

  Cuts();
  ~Cuts();

  // setters
  void addCut(std::string name, std::string cut);
  void addZvvCut(std::string name, std::string cut);
  void addZMuMuCut(std::string name, std::string cut);
  void addWMuCut(std::string name, std::string cut);
  void addWElCut(std::string name, std::string cut);
  void addWTauCut(std::string name, std::string cut);
  void addQCDCut(std::string name, std::string cut);

  // getters
  unsigned nCuts() { return cuts_.size(); }
  TCut cut(std::string name);
  std::string cutName(unsigned i);

  unsigned nZvvCuts() { return cutsZvv_.size(); }
  TCut cutZvv(std::string name);
  std::string cutNameZvv(unsigned i) { return namesZvv_.at(i); }

  unsigned nZMuMuCuts() { return cutsZMuMu_.size(); }
  TCut cutZMuMu(std::string name);
  std::string cutNameZMuMu(unsigned i) { return namesZMuMu_.at(i); }

  unsigned nWMuCuts() { return cutsWMu_.size(); }
  TCut cutWMu(std::string name);
  std::string cutNameWMu(unsigned i) { return namesWMu_.at(i); }

  unsigned nWElCuts() { return cutsWEl_.size(); }
  TCut cutWEl(std::string name);
  std::string cutNameWEl(unsigned i) { return namesWEl_.at(i); }

  unsigned nWTauCuts() { return cutsWTau_.size(); }
  TCut cutWTau(std::string name);
  std::string cutNameWTau(unsigned i) { return namesWTau_.at(i); }

  unsigned nQCDCuts() { return cutsQCD_.size(); }
  std::string cutNameQCD(unsigned i) { return namesQCD_.at(i); }

  TCut allCuts();

  //  TCut nMinusOneCuts(unsigned i);
  TCut nMinusOneCuts(std::string name);

  // cutflow
  TCut cutflow(unsigned i);
  TCut cutflowEWKZvv(unsigned i);

  // dataset-dependent cut
  TCut cutDataset(std::string name);

  // special weight for W MC
  TCut wWeight();
  
  // return correct lepton weights dep on user option
  TCut elTightWeight(std::string option);
  TCut elVetoWeight(std::string option);
  TCut muTightWeight(std::string option);
  TCut muVetoWeight(std::string option);

  // combinations
  TCut HLTandMETFilters();
  TCut leptonVeto();
  TCut vbf();   // note this does not include vetoes or MET!
  TCut vbfloose();

  TCut allCutsNoDPhi();
  TCut allCutsHiDPhi();
  TCut allCutsLoDPhi();

  // cuts for BG estimation

  // Z
  TCut zMuMuGen();   	  // there is a generator level Z->mumu
  TCut zMuMuGenMass();    // there is a generator level Z->mumu + mass cut
  TCut zMuMuReco();       // there is a reco level Z->mumu
  TCut zMuMuVBF();
  TCut zMuMuVBFLoose();
  TCut cutflowZMuMu(unsigned i);
  unsigned nCutsZMuMu()  { return cutsZMuMu_.size(); }

  // W-> mu
  TCut wMuGen();
  TCut wMuVBF();
  TCut cutflowWMu(unsigned i);
  unsigned nCutsWMu() { return cutsWMu_.size(); }

  // W-> e
  TCut wElGen();
  TCut wElVBF();
  TCut cutflowWEl(unsigned i);
  unsigned nCutsWEl() { return cutsWEl_.size(); }

  // W -> tau
  TCut wTauGen();
  TCut cutflowWTau(unsigned i);
  unsigned nCutsWTau() { return cutsWTau_.size(); }

  // QCD
  TCut qcdNoMET();
  TCut qcdLoose2();
  TCut qcdLoose();
  TCut qcdTightHiDPhi();
  TCut cutflowQCD(unsigned i);
  unsigned nCutsQCD() { return cutsQCD_.size(); }

 private:
  
  std::vector<std::string> names_;
  std::vector<TCut> cuts_;

  std::vector<std::string> namesZvv_;
  std::vector<TCut> cutsZvv_;

  std::vector<std::string> namesZMuMu_;
  std::vector<TCut> cutsZMuMu_;

  std::vector<std::string> namesWMu_;
  std::vector<TCut> cutsWMu_;

  std::vector<std::string> namesWEl_;
  std::vector<TCut> cutsWEl_;

  std::vector<std::string> namesWTau_;
  std::vector<TCut> cutsWTau_;

  std::vector<std::string> namesQCD_;
  std::vector<TCut> cutsQCD_;

};

#endif
