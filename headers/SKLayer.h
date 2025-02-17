#ifndef SKLAYER_H
#define SKLAYER_H

#include "SKLibraries.h"
#include "SKNeuron.h"

using namespace std;

class SKLayer {

 public:

  /* ----- Standard Constructor ----- */
  SKLayer(int size, string activation);


  /* ----- Standard Destructor ----- */
  ~SKLayer();

  /* ----- Public Method Write Out ----- */
  void WriteOutput();


  /* ----- Public Method Print ----- */
  void Print();

  /* ----- Public Method Rearrange Softmax ----- */
  void RearrangeSoftmax();


  /* ----- Public Method Clear ----- */
  void Clear();

  /* ----- Derivatives ------- */
  double LayerDer(int neuron);

  inline double SigmoidDer(double arg) { return (1.0/(1.0 + exp(-1.0*arg)))*(1.0-1.0/(1.0 + exp(-1.0*arg)));}


 private:

  int fSize;
  vector<SKNeuron> vNeurons;
  vector<double> vLayerOutput;
  string sActivationFunction;

  friend class SKPropagator;
  friend class SKBackProp;
  friend class SKModel;


};

#endif
