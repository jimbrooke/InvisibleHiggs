#include "InvisibleHiggs/Ntuple/interface/InvHiggsInfo.h"

#include <iostream>
#include <algorithm>


InvHiggsInfo::InvHiggsInfo() :
  id(0),
  bx(0),
  lb(0),
  run(0),
  l1Result(false),
  hltResult1(false),
  hltResult2(false),
  jet1Pt(0.),
  jet1Eta(0.),
  jet1Phi(0.),
  jet1Mass(0.),
  jet2Pt(0.),
  jet2Eta(0.),
  jet2Phi(0.),
  jet2Mass(0.),
  met(0.),
  mht(0.),
  vbfEt(0.),
  vbfEta(0.),
  vbfPhi(0.),
  vbfM(0.),
  vbfJet3Phi(0.),
  vbfNearJetEt(0.),
  vbfNearJetDR(0.),
  vbfCenJetEt(0.),
  ele1Pt(0.),
  ele1Eta(0.),
  ele1Phi(0.),
  ele2Pt(0.),
  ele2Eta(0.),
  ele2Phi(0.),
  mEE(0.),
  mu1Pt(0.),
  mu1Eta(0.),
  mu1Phi(0.),
  mu2Pt(0.),
  mu2Eta(0.),
  mu2Phi(0.),
  mMuMu(0.),
  puWeight(0.),
  mcHiggsMass(0.),
  mcHiggsPt(0.),
  mcHiggsEta(0.),
  mcHiggsPhi(0.),
  mcQ1Pt(0.),
  mcQ1Eta(0.),
  mcQ1Phi(0.),
  mcQ1M(0.),
  mcQ2Pt(0.),
  mcQ2Eta(0.),
  mcQ2Phi(0.),
  mcQ2M(0.),
  mcVBFEt(0.),
  mcVBFEta(0.),
  mcVBFPhi(0.),
  mcVBFM(0.),
  mcVBFJet3Phi(0.),
  mcVBFNearJetEt(0.),
  mcVBFNearJetDR(0.),
  mcVBFCenJetEt(0.)
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
