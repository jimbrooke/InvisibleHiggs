#include <TH2.h>
#include <iomanip>
// Macro to create TH2s from lepton weight text files, like for trigger weights
// They can then be used in Ntuple code
//
// Robin Aggleton 2013

// (Apologies for the use of "ele" here, and "el" in ntuple code - inherited ele from IC code...)

void makeHists(std::string filename, TFile* outputFile){
    ifstream infile;
    infile.open(filename.c_str());
    if (!infile.fail()) { //check if file opened correctly
        std::string line;
        double lastPtMin(0.), lastPtMax(0.), lastEtaMin(-1.), lastEtaMax(0.);
        std::vector<double> ptBins;
        std::vector<double> etaBins;
        std::vector<double> SF;
        std::vector<double> SFerrUp;
        std::vector<double> SFerrDown;
        int nPtBins(0),nEtaBins(0);

        // Read in each line of txt file and process
        // We want the pT and eta bins out, as well as storign the scale factors, and the +/- errors on it
        while (std::getline(infile, line))
        {
            std::stringstream ss(line);
            double ptMin, ptMax, etaMin, etaMax, sf, sfErrP, sfErrM;
            if (ss >> ptMin >> ptMax >> etaMin >> etaMax >> sf >> sfErrM >> sfErrP) {
                SF.push_back(sf);
                SFerrUp.push_back(sfErrP);
                SFerrDown.push_back(sfErrM);
                if (ptMin > lastPtMin) {
                    lastPtMin = ptMin;
                    nPtBins++;
                    ptBins.push_back(ptMin);
                }
                if (etaMin > lastEtaMin) {
                    lastEtaMin = etaMin;
                    nEtaBins++;
                    etaBins.push_back(etaMin);
                }
                lastPtMax  = ptMax;
                lastEtaMax = etaMax;
            }
        }    
        ptBins.push_back(lastPtMax);
        etaBins.push_back(lastEtaMax);
        infile.close();

        // Make a sensible hist name out of the filename - chop off the trailing .txt
        int lastindex = filename.find_last_of("."); 
        string histName = filename.substr(0, lastindex);

        // Now make 2D hists with the above pT and eta bins, and fill them with scale factors (central, error up and error down)
        TH2D histSF(histName.c_str(),";p_{T};#eta",nPtBins,&ptBins[0],nEtaBins,&etaBins[0]);
        TH2D histSFup((histName+"_errUp").c_str(),";p_{T};#eta",nPtBins,&ptBins[0],nEtaBins,&etaBins[0]);
        TH2D histSFdown((histName+"_errDown").c_str(),";p_{T};#eta",nPtBins,&ptBins[0],nEtaBins,&etaBins[0]);
        for (int a = 0; a < etaBins.size()-1 ; a++) {
            for (int b = 0; b < ptBins.size()-1; b++){
                // std::cout << SF[b+(a*(ptBins.size()-1))] << "   ";
                histSF.SetBinContent(b+1,a+1,SF[b+(a*nPtBins)]);
                histSFup.SetBinContent(b+1,a+1,SF[b+(a*nPtBins)]+SFerrUp[b+(a*nPtBins)]);
                histSFdown.SetBinContent(b+1,a+1,SF[b+(a*nPtBins)]-SFerrDown[b+(a*nPtBins)]);
            }
            // std::cout << std::endl;
        }

        // Uncomment the following 3 lines if you want a pdf of the scale factor
        // Makes it very slow though!
        // TCanvas c1;
        // histSF.Draw("TEXT90COLZ");
        // c1.SaveAs((histName+".pdf").c_str());

        // Save TH2s to file
        // TFile *file = new TFile((histName+".root").c_str(),"RECREATE");
        outputFile->cd();
        histSF.Write();
        histSFup.Write();
        histSFdown.Write();
        // file->Close();
    } else 
        std::cout << "ERROR: No such file " << filename << std::endl;
}

double calculateVetoEff(double eff_data, double eff_mc) {
    // Veto eff = 1-eff_data/1-eff_mc
    // Slightly complicated procedure here. Following IC, rules are:
        // if eff_data > 1, set = 1. Same with eff_mc
        // if eff_data < 0, set = 0. Same with eff_mc
        // When calculating the total eff, 1-eff_data / 1-eff_mc,
        // if you get 1/1 or 0/0, this = NaN, which we set = 1
        // All written & calcualted explicitly, so no ambiguity with NaN! 
    if(eff_data >= 1) {
        if(eff_mc >= 1)
            //set = 1/1 = NaN = 1
            return 1.;
        else
            //set = 0
            return 0.;
    } else if (eff_data < 0) {
        if(eff_mc >= 1)
            //set = 1 (1/0 =NaN) (this never occurs in practice)
            return 1.;
        else if (eff_mc < 0)
            //set = 1
            return 1.;
        else
            //set = 1/1-mc
            return 1./(1.-eff_mc);
    } else { // normal eff_data
        if(eff_mc >= 1)
            //set=1 (/0 =NaN)
            return 1.;
        else if (eff_mc < 0)
            //set = 1-hData/1
            return 1.-eff_data;
        else
            //set = 1-hData/1-mc
            return (1.-eff_data)/(1.-eff_mc);
    }
}

void MakeLeptonWeightHistos(){
    gStyle->SetOptStat("n");
    std::vector<std::string> fileList;
    TFile *outputFile = new TFile("leptonWeights.root","RECREATE");
    makeHists("ele_tight_id.txt",outputFile);
    makeHists("ele_veto_id_data_eff.txt",outputFile);
    makeHists("ele_veto_id_mc_eff.txt",outputFile);
    
    makeHists("mu_loose_id_data_eff.txt",outputFile);
    makeHists("mu_loose_id_mc_eff.txt",outputFile);
    makeHists("mu_loose_iso_data_eff.txt",outputFile);
    makeHists("mu_loose_iso_mc_eff.txt",outputFile);

    makeHists("mu_tight_id_SF.txt",outputFile);
    makeHists("mu_tight_iso_SF.txt",outputFile);
    
    // Multiply ID * ISO for mu and store
    // Also do 1-eff_data/1-eff_mc for loose mu and veto electrons and store
    std::string post[] = {"","_errUp","_errDown"};
    std::string dataMc[] = {"data","mc"};
    for (int k = 0; k < 3; k++){
        std::cout << " >>> " << post[k] << std::endl;

        for (int j =0; j<2; j++){
            // Loose mu: ID * ISO
            TH2D* temp = ((TH2D*) outputFile->Get(("mu_loose_id_"+dataMc[j]+"_eff"+post[k]).c_str())); 
            temp->Multiply((TH2D*) outputFile->Get(("mu_loose_iso_"+dataMc[j]+"_eff"+post[k]).c_str()));
            temp->SetName(("mu_loose_"+dataMc[j]+"_eff"+post[k]).c_str());
            temp->Write();
            delete temp;
        }

        // Loose mu eff
        std::cout << "Loose mu" << std::endl;
        TH2D* hDataMu  = (TH2D*) outputFile->Get(("mu_loose_data_eff"+post[k]).c_str());
        TH2D* hMcMu    = (TH2D*) outputFile->Get(("mu_loose_mc_eff"+post[k]).c_str());
        TH2D* hFinalMu = (TH2D*) hDataMu->Clone(("mu_loose_eff"+post[k]).c_str());
        for(int x = 1; x <= hDataMu->GetNbinsX();x++){
            for(int y = 1; y <= hDataMu->GetNbinsY(); y++){
                hFinalMu->SetBinContent(x,y,calculateVetoEff(hDataMu->GetBinContent(x,y),hMcMu->GetBinContent(x,y)));
                std::cout << std::fixed;
                std::cout << std::setprecision(15) << hDataMu->GetBinContent(x,y) << "   " << hMcMu->GetBinContent(x,y) << "   " << (1.-(hDataMu->GetBinContent(x,y)))/(1.-(hMcMu->GetBinContent(x,y))) << "   " << hFinalMu->GetBinContent(x,y) <<std::endl;
            }
        }
        hFinalMu->Write();
        delete hFinalMu;       

        // Tight mu ID * ISO
        // std::cout << "Tight Mu" << std::endl;
        TH2D* muTight = (TH2D*) outputFile->Get(("mu_tight_id_SF"+post[k]).c_str());
        muTight->Multiply((TH2D*) outputFile->Get(("mu_tight_iso_SF"+post[k]).c_str()));
        muTight->SetName(("mu_tight_eff"+post[k]).c_str());
        muTight->Write(); 

        // Veto electrons
        std::cout << "Veto ele" << std::endl;
        TH2D* hDataEl  = (TH2D*) outputFile->Get(("ele_veto_id_data_eff"+post[k]).c_str());
        TH2D* hMcEl    = (TH2D*) outputFile->Get(("ele_veto_id_mc_eff"+post[k]).c_str());
        TH2D* hFinalEl = (TH2D*) hDataEl->Clone(("ele_veto_eff"+post[k]).c_str());
        for(int x = 1; x <= hDataEl->GetNbinsX();x++){
            for(int y = 1; y <= hDataEl->GetNbinsY(); y++){
                hFinalEl->SetBinContent(x,y,calculateVetoEff(hDataEl->GetBinContent(x,y),hMcEl->GetBinContent(x,y)));
                std::cout << std::setprecision(15) << hDataEl->GetBinContent(x,y) << "   " << hMcEl->GetBinContent(x,y) << "   " << (1.-(hDataEl->GetBinContent(x,y)))/(1.-(hMcEl->GetBinContent(x,y))) << "   " << hFinalEl->GetBinContent(x,y)  << std::endl;
            }
        }
        hFinalEl->Write();
        delete hFinalEl;       
    }

    outputFile->ls();
    outputFile->Close();
}
