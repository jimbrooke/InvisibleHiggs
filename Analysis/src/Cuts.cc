#include "InvisibleHiggs/Analysis/interface/Cuts.h"


Cuts::Cuts() {

  addCut("trigger",    "hltResult2>0.");
  addCut("metFilter",  "metflag0 && metflag1 && metflag2 && metflag3 && metflag4 && metflag5 && metflag6");
  addCut("EVeto",      "ele1Pt<10.");
  addCut("MuVeto",     "mu1Pt<10.");
  addCut("dijet",      "jet1Pt>50.&&abs(jet1Eta)<4.7&&jet2Pt>50.&&abs(jet2Eta)<4.7");
  addCut("sgnEtaJJ",   "(jet1Eta*jet2Eta)<0.");
  addCut("dEtaJJ",     "vbfDEta>4.2");
  addCut("MET",        "met>130.");
  addCut("Mjj",        "vbfM>1200.");
  addCut("dPhiJJ",     "vbfDPhi<1.0");

  addZMuMuCut("trigger",   "hltResult2>0. && metflag0 && metflag1 && metflag2 && metflag3 && metflag4 && metflag5 && metflag6");
  addZMuMuCut("z",         "zChannel==1 && zMass>60. && zMass<120.");
  addZMuMuCut("LVeto",     "ele1Pt<10. && mu3Pt<10.");
  addZMuMuCut("dijet",     "jet1Pt>50.&&abs(jet1Eta)<4.7&&jet2Pt>50.&&abs(jet2Eta)<4.7 && (jet1Eta*jet2Eta)<0.");
  addZMuMuCut("dEtaJJ",    "vbfDEta>4.2");
  addZMuMuCut("MET",       "met>130.");
  addZMuMuCut("Mjj",       "vbfM>1200.");

  addWMuCut("trigger",   "hltResult2>0. && metflag0 && metflag1 && metflag2 && metflag3 && metflag4 && metflag5 && metflag6");
  addWMuCut("wMu",       "nW>0 && wChannel==1 && wMt>40.");
  addWMuCut("lVeto",     "ele1Pt<10. && mu2Pt<10.");
  addWMuCut("dijet",     "jet1Pt>50.&&abs(jet1Eta)<4.7&&jet2Pt>50.&&abs(jet2Eta)<4.7 && (jet1Eta*jet2Eta)<0.");
  addWMuCut("dEtaJJ",    "vbfDEta>4.2");
  addWMuCut("MET",       "met>130.");
  addWMuCut("Mjj",       "vbfM>1200.");

  addWElCut("trigger",   "hltResult2>0. && metflag0 && metflag1 && metflag2 && metflag3 && metflag4 && metflag5 && metflag6");
  addWElCut("wEl",       "nW>0 && wChannel==2 && wMt>40.");
  addWElCut("lVeto",     "ele1Pt<10. && mu2Pt<10.");
  addWElCut("dijet",     "jet1Pt>50.&&abs(jet1Eta)<4.7&&jet2Pt>50.&&abs(jet2Eta)<4.7 && (jet1Eta*jet2Eta)<0.");
  addWElCut("dEtaJJ",    "vbfDEta>4.2");
  addWElCut("MET",       "met>130.");
  addWElCut("Mjj",       "vbfM>1200.");

}

Cuts::~Cuts() {


}


void Cuts::addCut(std::string name, std::string cut) {
  names_.push_back(name);
  TCut c(cut.c_str());
  cuts_.push_back(c);
}

void Cuts::addZMuMuCut(std::string name, std::string cut) {
  namesZMuMu_.push_back(name);
  TCut c(cut.c_str());
  cutsZMuMu_.push_back(c);
}

void Cuts::addWMuCut(std::string name, std::string cut) {
  namesWMu_.push_back(name);
  TCut c(cut.c_str());
  cutsWMu_.push_back(c);
}

void Cuts::addWElCut(std::string name, std::string cut) {
  namesWEl_.push_back(name);
  TCut c(cut.c_str());
  cutsWEl_.push_back(c);
}


// TCut Cuts::cut(unsigned i) {
//   return cuts_.at(i);
// }


TCut Cuts::cut(std::string s) {
  for (unsigned i=0; i<names_.size(); ++i) {
    if (names_.at(i) == s) return cuts_.at(i);
  }
  return TCut();
}


std::string Cuts::cutName(unsigned i) {
  return names_.at(i);
}


TCut Cuts::allCuts() {

  TCut tmp;
  for (std::vector<TCut>::const_iterator c=cuts_.begin(); c!=cuts_.end(); ++c) {
    tmp = tmp + (*c);
  }
  return tmp;

}

// TCut Cuts::nMinusOneCuts(unsigned i) {

//   TCut tmp;
//   for (unsigned j=0; j<cuts_.size(); ++j) {
//     if (j!=i) tmp = tmp && cuts_.at(j);
//   }
//   return tmp;  

// }

TCut Cuts::nMinusOneCuts(std::string name) {

  TCut tmp;
  for (unsigned j=0; j<cuts_.size(); ++j) {
    if (name!=names_.at(j)) tmp = tmp && cuts_.at(j);
  }
  return tmp;  

}


TCut Cuts::cutflow(unsigned i) {
  TCut tmp;
  for (unsigned j=0; j<i+1; ++j) {
    tmp += cuts_.at(j);
  }
  return tmp;
}


TCut Cuts::cutDataset(std::string name) {

  TCut tmp;
  if (name == "WJets") return TCut("wgennj==0");
  else if (name == "DYJetsToLL") return TCut("zgenpt<100.");
  else return tmp;

}


// combinations
TCut Cuts::HLTandMETFilters() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  return tmp;
}

TCut Cuts::vbf() {
  TCut tmp = cut("dijet");
  tmp += cut("sgnEtaJJ");
  tmp += cut("dEtaJJ");
  tmp += cut("Mjj");
  return tmp;
}

TCut Cuts::allCutsNoDPhi() {
  TCut tmp = nMinusOneCuts("dPhiJJ");
  return tmp;
}

TCut Cuts::allCutsHiDPhi() {
  TCut tmp = allCutsNoDPhi();
  tmp += TCut("vbfDPhi>2.6");
  return tmp;
}

TCut Cuts::allCutsLoDPhi() {
  TCut tmp = allCutsNoDPhi();
  tmp += TCut("vbfDPhi<1.0");
  return tmp;
}


// control regions
TCut Cuts::zMuMuGen() {
  TCut tmp("zltype==2");
  return tmp;
}

TCut Cuts::zMuMuGenPt100() {
  TCut tmp("zltype==2 && zgenpt<100.");
  return tmp;
}

TCut Cuts::zMuMuGenMass() {
  TCut tmp("zltype==2 && zgenmass>60. && zgenmass<120.");
  return tmp;
}

TCut Cuts::zMuMuReco() {
  TCut tmp("zChannel==1 && zMass>60. && zMass<120.");
  return tmp;
}

TCut Cuts::zMuMuVBF() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += zMuMuReco();
  tmp += TCut("mu3Pt<10");
  tmp += cut("EVeto");
  tmp += vbf();
  tmp += TCut("metNo2Muon>130.");
  return tmp;
}

TCut Cuts::zMuMuGenPt100VBF() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += zMuMuGenPt100();
  tmp += zMuMuReco();
  tmp += TCut("mu3Pt<10");
  tmp += cut("EVeto");
  tmp += vbf();
  tmp += TCut("metNo2Muon>130.");
  return tmp;
}


TCut Cuts::cutflowZMuMu(unsigned i) {
  TCut tmp("");
  for (unsigned j=0; j<i+1; ++j) {
    tmp += cutsZMuMu_.at(i);
  }
  return tmp;
}



// W -> mu
TCut Cuts::wMuGen() {
  TCut tmp("wltype==2");
  return tmp;
}

TCut Cuts::wMuGen0P() {
  TCut tmp("wltype==2 && wgennj==0");  // need to add single parton selection
  return tmp;
}

TCut Cuts::wMuReco() {
  TCut tmp("nW>0 && wChannel==1 && sqrt(2*wDaulPt*met*(1-wDaulPhi))>40. && sqrt(2*wDaulPt*met*(1-wDaulPhi))>120");
  return tmp;
}

TCut Cuts::wMuVBF() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += wMuReco();
  tmp += TCut("mu2Pt<10");
  tmp += cut("EVeto");
  tmp += vbf();
  tmp += TCut("metNoWLepton>130.");
  return tmp;
}

TCut Cuts::cutflowWMu(unsigned i) {
  TCut tmp("");
  for (unsigned j=0; j<i+1; ++j) {
    tmp += cutsWMu_.at(i);
  }
  return tmp;
}


// W -> e
TCut Cuts::wElGen() {
  TCut tmp("wltype==1");
  return tmp;
}

TCut Cuts::wElGen0P() {
  TCut tmp("wltype==1 && wgennj==0");  // need to add single parton selection
  return tmp;
}

TCut Cuts::wElReco() {
  TCut tmp("nW>0 && wChannel==2 && sqrt(2*wDaulPt*met*(1-wDaulPhi))>40. && sqrt(2*wDaulPt*met*(1-wDaulPhi))>120");
  return tmp;
}


TCut Cuts::wElVBF() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += wElReco();
  tmp += TCut("ele2Pt<10");
  tmp += cut("MVeto");
  tmp += vbf();
  tmp += TCut("metNoWLepton>130.");
  return tmp;
}

TCut Cuts::cutflowWEl(unsigned i) {
  TCut tmp("");
  for (unsigned j=0; j<i+1; ++j) {
    tmp += cutsWEl_.at(i);
  }
  return tmp;
}


// QCD regions
TCut Cuts::qcdNoMET() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += cut("EVeto");
  tmp += cut("MVeto");
  tmp += vbf();
  return tmp;
}

TCut Cuts::qcdLoose() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += cut("EVeto");
  tmp += cut("MVeto");
  tmp += vbf();
  tmp += TCut("met>70.");
  return tmp;
}

TCut Cuts::qcdTightHiDPhi() {
  TCut tmp = cut("trigger");
  tmp += cut("metFilter");
  tmp += cut("EVeto");
  tmp += cut("MVeto");
  tmp += vbf();
  tmp += cut("met");
  tmp += TCut("vbfDPhi>2.6");
  return tmp;
}

TCut Cuts::cutflowQCD(unsigned i) {
  TCut tmp("");
  for (unsigned j=0; j<i+1; ++j) {
    tmp += cutsQCD_.at(i);
  }
  return tmp;
}

