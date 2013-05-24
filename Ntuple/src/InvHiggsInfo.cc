#include "InvisibleHiggs/Ntuple/interface/InvHiggsInfo.h"

#include <iostream>
#include <algorithm>


InvHiggsInfo::InvHiggsInfo() :
  id(0),
  bx(0),
  lb(0),
  run(0),
  l1Result(false),
  l1ET(-10.),
  l1MET(-10.),
  l1ETPhi(-10.),
  l1HT(-10.),
  l1MHT(-10.),
  l1HTPhi(-10.),
  hltResult1(false),
  hltResult2(false),
  hltResult3(false),
  hltResult4(false),
  metflag0(false),
  metflag1(false),
  metflag2(false),
  metflag3(false),
  metflag4(false),
  metflag5(false),
  metflag6(false), 
  metflag7(false),
  metflag8(false),
  jet1Index(-1),
  jet1Pt(-10.),
  jet1Eta(-10.),
  jet1Phi(-10.),
  jet1M(-10.),
  jet1PUMVA(-10.),
  jet1PUFlag(0),
  jet1unc(0.),
  jet2Index(-1),
  jet2Pt(-10.),
  jet2Eta(-10.),
  jet2Phi(-10.),
  jet2M(-10.),
  jet2PUMVA(-10.),
  jet2PUFlag(0),
  jet2unc(0.),
  //PFMET original
  met(-10.),
  metPhi(-10.),
  jetMETdPhi(-10.),
  metNoMuon(-10.),
  metNoMuonPhi(-10.),
  metNoElectron(-10.),
  metNoElectronPhi(-10.),
  metNo2Muon(-10.),
  metNo2MuonPhi(-10.),
  metNoWLepton(-10.),
  metNoWLeptonPhi(-10.),
  mht(-10.),
  mhtPhi(-10.),
  vbfEt(-10.),
  vbfEta(-10.),
  vbfPhi(-10.),
  vbfM(-10.),
  vbfDEta(-10.),
  vbfDPhi(-10.),
  numTrackerJet3(0),
  jet3Et(-10.),
  jet3Eta(-10.),
  jet3Phi(-10.),
  jet3M(-10.),
  jet3EtaStar(-10.),
  cenJetEt(-10.),
  cenJetEta(-10.),
  cenJetPhi(-10.),
  cenJetM(-10.),
  cenJetEtaStar(-10.),
  ele1Pt(-10.),
  ele1Eta(-10.),
  ele1Phi(-10.),
  ele1M(-10.),
  ele2Pt(-10.),
  ele2Eta(-10.),
  ele2Phi(-10.),
  ele2M(-10.),
  ele3Pt(-10.),
  ele3Eta(-10.),
  ele3Phi(-10.),
  ele3M(-10.),
  mu1Pt(-10.),
  mu1Eta(-10.),
  mu1Phi(-10.),
  mu1M(-10.),
  mu2Pt(-10.),
  mu2Eta(-10.),
  mu2Phi(-10.),
  mu2M(-10.),
  mu3Pt(-10.),
  mu3Eta(-10.),
  mu3Phi(-10.),
  mu3M(-10.),
  nW(0),
  wPt(-10.),
  wEta(-10.),
  wPhi(-10.),
  wMt(-10.),
  wChannel(-10.),
  wDaulPt(-10.),
  wDaulEta(-10.),
  wDaulPhi(-10.),
  wDaulM(-10.),
  wDaulCharge(0.),
  nZ(0),
  zPt(-10.),
  zEta(-10.),
  zPhi(-10.),
  zMass(-10.),
  zChannel(-10.),
  zDau1Pt(-10.),
  zDau1Eta(-10.),
  zDau1Phi(-10.),
  zDau1M(-10.),
  zDau1Charge(0.),
  zDau2Pt(-10.),
  zDau2Eta(-10.),
  zDau2Phi(-10.),
  zDau2M(-10.),
  zDau2Charge(0.),
  nVtx(0),
  puWeight(1.),
  trigCorrWeight(1.),
  mcHiggsMass(-10.),
  mcHiggsPt(-10.),
  mcHiggsEta(-10.),
  mcHiggsPhi(-10.),
  mcQ1Pt(-10.),
  mcQ1Eta(-10.),
  mcQ1Phi(-10.),
  mcQ1M(-10.),
  mcQ2Pt(-10.),
  mcQ2Eta(-10.),
  mcQ2Phi(-10.),
  mcQ2M(-10.),
  mcVBFEt(-10.),
  mcVBFEta(-10.),
  mcVBFPhi(-10.),
  mcVBFM(-10.),
  mcVBFDEta(-10.),
  mcVBFDPhi(-10.),
  
  // MC Info -- W
  wgennj(-4),
  wgenmass(-10.),
  wgenpt(-10.),
  wgenmt(-10.),
  wgeneta(-10.),
  wgenphi(-10.),
  wgene(-10.),
  wltype(0),
  wlpt(-10.),
  wleta(-10.),
  wlphi(-10.),
  wle(-10.),
  wtauhadron(0),
  wmetpt(-10.),
  wmeteta(-10.),
  wmetphi(-10.),
  wmete(-10.),

  // MC Info -- Z
  zgenmass(-10.),
  zgenpt(-10.),
  zgenmt(-10.),
  zgeneta(-10.),
  zgenphi(-10.),
  zgene(-10.),
  zltype(0),
  zlmpt(-10.),
  zlmeta(-10.),
  zlmphi(-10.),
  zlme(-10.),
  zlppt(-10.),
  zlpeta(-10.),
  zlpphi(-10.),
  zlpe(-10.)
{

}

InvHiggsInfo::~InvHiggsInfo() { }

void InvHiggsInfo::Dump() {

  using namespace std;

  cout << "Event Dump " << endl;
  cout << "Run          " << run << endl;
  cout << "Event        " << id << endl;
  cout << "LB           " << lb << endl;
}


//#if !defined(__CINT__)
//  ClassImp(Event)
//#endif
