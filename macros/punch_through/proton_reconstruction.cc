#include "SKNeuron.h"
#include "SKLayer.h"
#include "SKWeights.h"
#include "SKPropagator.h"
#include "SKModel.h"
#include "SKFancyPlots.h"



int main (int argc, char** argv) {


  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging("ProtonReconstruction");

  TApplication* theApp = new TApplication("reconstruction", 0, 0);


  clock_t start, end,real_start,real_end;

  LOG(INFO)<<"#============================================================#";
  LOG(INFO)<<"# Welcome to SoKAI (Some Kind of Artificial Intelligence) !! #";
  LOG(INFO)<<"#============================================================#";

  int seed            = 2022;
  int epochs          = stoi(argv[1]);
  int nSamples        = stoi(argv[2]);
  int nTrainingSize   = (7.0/10.0)*nSamples;
  int nTestSize       = (3.0/10.0)*nSamples;
  int nMiniBatchSize  = stoi(argv[4]);
  float fLearningRate = stoi(argv[3])/100.;

  real_start = clock();

  /*---- Input Data (Use this format!) ---- */
  vector<vector<double>> data_sample;
  vector<vector<double>> input_labels;

  vector<double> data_instance;
  vector<double> label_instance;

  /*---- For training results ----*/
  vector<double> loss_vec;
  vector<double> output_vec;
  vector<double> output_model;

  vector<double> x_vec;
  vector<double> target_vec;
  vector<double> epoch_vec;
  vector<double> reconstruction_resolution_vec;

  double loss = 0.0;

  TRandom3 gen(seed);

  float fCrystalMax = 50;
  float fPolarMax = TMath::Pi();
  float fAzimuthalMax =2*TMath::Pi();
  float fClusterEnergyMax = 400;
  float fSingleCrystalEnergyMax = 340;
  float fPrimEnergyMax = 600;
  float fCrystalTypeMax = 27;



  /* -------- Put this on a header or something..... ----------*/
  gStyle->SetOptStat(0);
  Double_t Red[5]    = { 0.06, 0.25, 0.50, 0.75, 1.0};
  Double_t Green[5]  = {0.01, 0.1, 0.15, 0.20, 0.8};
  Double_t Blue[5]   = { 0.00, 0.00, 0.00, 0.0, 0.0};
  Double_t Length[5] = { 0.00, 0.25, 0.50, 0.75, 1.00 };
  Int_t nb=250;

  TColor::CreateGradientColorTable(5,Length,Red,Green,Blue,nb);
   gStyle->SetCanvasColor(1);
   gStyle->SetTitleFillColor(1);
   gStyle->SetStatColor(1);

   gStyle->SetFrameLineColor(0);
   gStyle->SetGridColor(0);
   gStyle->SetStatTextColor(0);
   gStyle->SetTitleTextColor(0);
   gStyle->SetLabelColor(0,"xyz");
   gStyle->SetTitleColor(0,"xyz");
   gStyle->SetAxisColor(0,"xyz");
   /* --------------------------------------------------------*/


   /* ------- Reading Root Data -------- */
  TString fileList = "/home/gabri/Analysis/s455/simulation/punch_through/neural_tree_data_input_cut_punched.root";

  TFile *eventFile;
  TTree* eventTree;

  eventFile = TFile::Open(fileList);
  eventTree = (TTree*)eventFile->Get("evt");

  int rCrystalMultiplicity;
  TBranch  *mulBranch = eventTree->GetBranch("CrystalMultiplicity");
  mulBranch->SetAddress(&rCrystalMultiplicity);

  float rPolar;
  TBranch  *polarBranch = eventTree->GetBranch("Theta");
  polarBranch->SetAddress(&rPolar);

  float rAzimuthal;
  TBranch  *aziBranch = eventTree->GetBranch("Phi");
  aziBranch->SetAddress(&rAzimuthal);

  float rClusterEnergy;
  TBranch  *hitBranch = eventTree->GetBranch("ClusterEnergy");
  hitBranch->SetAddress(&rClusterEnergy);

  float rSingleCrystalEnergy;
  TBranch  *singleBranch = eventTree->GetBranch("SingleCrystalEnergy");
  singleBranch->SetAddress(&rSingleCrystalEnergy);

  float rPrimaryEnergy;
  TBranch  *primBranch = eventTree->GetBranch("PrimEnergy");
  primBranch->SetAddress(&rPrimaryEnergy);

  float rCrystalType;
  TBranch *typeBranch = eventTree->GetBranch("MotherCrystalType");
  typeBranch->SetAddress(&rCrystalType);

  int nEvents = eventTree->GetEntries();

  LOG(INFO)<<"Number of Samples : "<<nEvents<<endl;

  for (Int_t j = 0; j<nSamples; j++) {


    eventTree->GetEvent(j);
    if(!(j%100))
     LOG(INFO)<<"Reading event "<<j<<" out of "<<nEvents<<" ("<<100.0*Float_t(j)/Float_t(nSamples)<<" % ) "<<endl;

    data_instance.push_back(rClusterEnergy/fClusterEnergyMax);
    // data_instance.push_back(rSingleCrystalEnergy/fSingleCrystalEnergyMax);
    data_instance.push_back((rCrystalType + 1)/fCrystalTypeMax);

    label_instance.push_back(rPrimaryEnergy/fPrimEnergyMax);
    cout<<"Data Instance : "<<data_instance.at(0)<<" "<<data_instance.at(1)<<endl;
    data_sample.push_back(data_instance);
    input_labels.push_back(label_instance);

    data_instance.clear();
    label_instance.clear();

   }

  /*------- The Model Itself -------*/

  SKLayer   *layer_1 = new SKLayer(2,argv[6]);
  SKWeights *weights_12 = new SKWeights(2,stoi(argv[5]));
  SKWeights *gradients_12 = new SKWeights(2,stoi(argv[5]));

  SKLayer   *layer_2 = new SKLayer(stoi(argv[5]),argv[7]);
  SKWeights *weights_23 = new SKWeights(stoi(argv[5]),1);
  SKWeights *gradients_23 = new SKWeights(stoi(argv[5]),1);

  SKLayer   *layer_3 = new SKLayer(1,argv[8]);




  weights_12->Init(seed);
  gradients_12->InitGradients();

  weights_23->Init(seed);
  gradients_23->InitGradients();




  SKModel *model = new SKModel("Regression");

  model->AddLayer(layer_1);
  model->AddWeights(weights_12);
  model->AddGradients(gradients_12);

  model->AddLayer(layer_2);
  model->AddWeights(weights_23);
  model->AddGradients(gradients_23);

  model->AddLayer(layer_3);

  model->SetInputSample(&data_sample);
  model->SetInputLabels(&input_labels);

  model->Init();
  model->SetLearningRate(fLearningRate);
  model->SetLossFunction(argv[9]);

  /* ---- Number of processed inputs before updating gradients ---- */
  model->SetBatchSize(nMiniBatchSize);

  LOG(INFO)<<"Model Training Hyper Parameters. Epochs : "<<argv[1]<<" Samples : "<<argv[2]<<" Learning Rate : "<<stoi(argv[3])/100.0<<" Metric : "<<argv[9];
  LOG(INFO)<<"";
  LOG(INFO)<<"/* ---------- Model Structure -----------";
  LOG(INFO)<<"L1 : "<<argv[6]<<" "<<"3";
  LOG(INFO)<<"H1 : "<<argv[7]<<" "<<argv[5];
  LOG(INFO)<<"L4 : "<<argv[8]<<" "<<"1";

  /* ---------- Pass Data Through Model ----------*/

   for (int i = 0 ; i < epochs ; i++){
     for (int j = 0 ; j < nTrainingSize ; j++){


      // Using  7/10 of the dataset to train the network
      int sample_number = nTrainingSize*gen.Rndm();

      model->Train(j);

      loss =  model->QuadraticLoss();

      model->Clear();

   }

    if(i%100==0){

     LOG(INFO)<<" Loss : "<<loss<<" . Epoch : "<<i;
     loss_vec.push_back(loss);
     epoch_vec.push_back(i);

   }

}

real_end = clock();

cout<<"Total training time : "<<((float) real_end - real_start)/CLOCKS_PER_SEC<<" s"<<endl;

/* --------- Testing the model --------- */
TH1F *hReconstruction_results = new TH1F("hReconstruction_results","Reconstructed Energy",400,-400,400);
TH1F *hCluster_results = new TH1F("hCluster_results","Cluster Energy",400,-400,400);

TH2F *hCorrReconstruction_results = new TH2F("hCorrReconstruction_results","Reconstructed Energy Vs Primary Energy",400,-600,800,400,0,600);
TH2F *hCorrCluster_results = new TH2F("hCorrCluster_results","Cluster Energy Vs Primary Energy",400,-600,600,400,0,600);


  for (int j = 0 ; j < nTestSize ; j++){


    // Using only 3/10 of the dataset to test the network
    int sample_number = nTrainingSize + nTestSize*gen.Rndm();


    output_vec.clear();

    output_vec = model->Propagate(sample_number);

    hCluster_results->Fill(fClusterEnergyMax*data_sample.at(sample_number).at(0)- fPrimEnergyMax*input_labels.at(sample_number).at(0));
    reconstruction_resolution_vec.push_back(fPrimEnergyMax*(output_vec.at(0) - input_labels.at(sample_number).at(0)));
    hCorrReconstruction_results->Fill(fPrimEnergyMax*(output_vec.at(0)),fPrimEnergyMax*input_labels.at(sample_number).at(0));
    hCorrCluster_results->Fill(fClusterEnergyMax*data_sample.at(sample_number).at(0),fPrimEnergyMax*input_labels.at(sample_number).at(0));
    model->Clear();


}




/* --------- Plots and so on ....... ------*/

TCanvas *model_canvas = new TCanvas("model_canvas","Model");
model_canvas->Divide(2,1);

TCanvas *summary_canvas = new TCanvas("summary_canvas","Model");
summary_canvas->Divide(2,1);

TCanvas *weight_canvas = new TCanvas("weight_canvas","Weights");



for(int i = 0 ; i < reconstruction_resolution_vec.size() ; i++)
  hReconstruction_results->Fill(reconstruction_resolution_vec.at(i));

model_canvas->cd(1);
hReconstruction_results->Draw("");
hReconstruction_results->GetXaxis()->SetTitle("Reconstructed Energy - Primary Energy (MeV)");
hReconstruction_results->GetYaxis()->SetTitle("Counts");

summary_canvas->cd(1);
hCorrReconstruction_results->Draw("COLZ");
hCorrReconstruction_results->GetXaxis()->SetTitle("Reconstructed Energy");
hCorrReconstruction_results->GetYaxis()->SetTitle("Primary Energy (MeV)");

summary_canvas->cd(2);
hCorrCluster_results->Draw("COLZ");
hCorrCluster_results->GetXaxis()->SetTitle("Cluster Energy");
hCorrCluster_results->GetYaxis()->SetTitle("Primary Energy (MeV)");



TGraph *loss_graph = new TGraph(epoch_vec.size(),&epoch_vec[0],&loss_vec[0]);

model_canvas->cd(2);
hCluster_results->Draw("");
hCluster_results->GetXaxis()->SetTitle("Cluster Energy - Primary Energy (MeV)");
hCluster_results->GetYaxis()->SetTitle("Counts");

TH2F* model_histo;
model_histo = (TH2F*)model->ShowMe();

weight_canvas->cd();
 model_histo->Draw("COLZ");

TString name = "training_results_";
name = name + argv[10] + "_" + argv[11] + ".root";

TFile resultsFile(name,"RECREATE");

model_canvas->Write();
summary_canvas->Write();
weight_canvas->Write();

theApp->Run();

return 0;



}
