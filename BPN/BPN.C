/******************************************************************************
                            D E C L A R A T I O N S
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


typedef int           BOOL;         /* logical variable                     */
typedef int           INT;          /* integer variable                     */
typedef double        REAL;         /* real variable                        */

#define FALSE         0             /* logical constants                     */
#define TRUE          1             /* logical constants                     */
#define NOT           !             /* logical operator                      */
#define AND           &&            /* logical operator                      */
#define OR            ||            /* logical operator                      */

#define MIN_REAL      -HUGE_VAL     /* smallest possible floating point no.  */
#define MAX_REAL      +HUGE_VAL     /* largest possible floating point no.   */
#define MIN(x,y)      ((x)<(y) ? (x) : (y)) /* minimum of x and y       */
#define MAX(x,y)      ((x)>(y) ? (x) : (y)) /* maximum of x and y       */

#define LO            0.1           /* lowest allowed weight or bias         */
#define HI            0.9           /* highest allowed weight or bias        */
#define BIAS          1             /* position of bias input unit           */

#define sqr(x)        ((x)*(x))     /* x squared                             */


typedef struct {                     /* A LAYER OF A NET:                     */
        INT           Units;         /* - number of units in this layer       */
        REAL*         Output;        /* - output of ith unit                  */
        REAL*         Error;         /* - error term of ith unit              */
        REAL**        Weight;        /* - connection weights to ith unit      */
        REAL**        WeightSave;    /* - saved weights for stopped training  */
        REAL**        dWeight;       /* - last weight deltas for momentum     */
} LAYER;

typedef struct {                     /* A NET:                                */
        LAYER**       Layer;         /* - layers of this net                  */
        LAYER*        InputLayer;    /* - input layer                         */
        LAYER*        OutputLayer;   /* - output layer                        */
        REAL          Alpha;         /* - momentum factor                     */
        REAL          Eta;           /* - learning rate                       */
        REAL          Gain;          /* - gain of sigmoid function            */
        REAL          Error;         /* - total net error                     */
} NET;

/******************************************************************************
        R A N D O M S   D R A W N   F R O M   D I S T R I B U T I O N S
 ******************************************************************************/

void InitializeRandoms() // Always call this before using random functions
{
  srand(4711); // The seed can be any number
}

// Returns a random integer between Low and High inclusive
INT RandomEqualINT(INT Low, INT High)
{
  return rand() % (High-Low+1) + Low;   // +1 makes it [Low,High], not [Low,High)
}      

// Returns a random real number between Low and High inclusive
REAL RandomEqualREAL(REAL Low, REAL High)
{
  return ((REAL) rand() / RAND_MAX) * (High-Low) + Low; // /RAND_MAX makes result [0,1)
}      


/******************************************************************************
               A P P L I C A T I O N - S P E C I F I C   C O D E
 ******************************************************************************/


#define NUM_LAYERS    3            /* number of layers incl. input layer    */
#define N             30           /* number of input units                 */
#define M             1            /* number of output units                */
INT                   Units[NUM_LAYERS] = {N, 10, M}; /* number of units in each layer */

#define FIRST_YEAR    1700         /* first year of data                    */
#define NUM_YEARS     280          /* number of years of data               */

#define TRAIN_LWB     (N)          /* lower bound of training set           */
#define TRAIN_UPB     (179)        /* upper bound of training set           */
#define TRAIN_YEARS   (TRAIN_UPB - TRAIN_LWB + 1) /* number of years in training set */
#define TEST_LWB      (180)       /* lower bound of test set               */
#define TEST_UPB      (259)       /* upper bound of test set               */
#define TEST_YEARS    (TEST_UPB - TEST_LWB + 1) /* number of years in test set     */
#define EVAL_LWB      (260)       /* lower bound of evaluation set         */
#define EVAL_UPB      (NUM_YEARS - 1) /* upper bound of evaluation set     */
#define EVAL_YEARS    (EVAL_UPB - EVAL_LWB + 1) /* number of years in evaluation set */

// The following data is taken from:
//   http://www.ngdc.noaa.gov/stp/SOLAR/ftpsunspotnumber.html
// The data is the mean monthly sunspot numbers for each month from January
// 1749 to December 1995.  The data is divided by 200 to scale it to the
// range [0,1].
REAL                  Sunspots_[NUM_YEARS]; // The original data
REAL                  Sunspots [NUM_YEARS] = { // The scaled data

                        0.0262,  0.0575,  0.0837,  0.1203,  0.1883,  0.3033,  
                        0.1517,  0.1046,  0.0523,  0.0418,  0.0157,  0.0000,  
                        0.0000,  0.0105,  0.0575,  0.1412,  0.2458,  0.3295,  
                        0.3138,  0.2040,  0.1464,  0.1360,  0.1151,  0.0575,  
                        0.1098,  0.2092,  0.4079,  0.6381,  0.5387,  0.3818,  
                        0.2458,  0.1831,  0.0575,  0.0262,  0.0837,  0.1778,  
                        0.3661,  0.4236,  0.5805,  0.5282,  0.3818,  0.2092,  
                        0.1046,  0.0837,  0.0262,  0.0575,  0.1151,  0.2092,  
                        0.3138,  0.4231,  0.4362,  0.2495,  0.2500,  0.1606,  
                        0.0638,  0.0502,  0.0534,  0.1700,  0.2489,  0.2824,  
                        0.3290,  0.4493,  0.3201,  0.2359,  0.1904,  0.1093,  
                        0.0596,  0.1977,  0.3651,  0.5549,  0.5272,  0.4268,  
                        0.3478,  0.1820,  0.1600,  0.0366,  0.1036,  0.4838,  
                        0.8075,  0.6585,  0.4435,  0.3562,  0.2014,  0.1192,  
                        0.0534,  0.1260,  0.4336,  0.6904,  0.6846,  0.6177,  
                        0.4702,  0.3483,  0.3138,  0.2453,  0.2144,  0.1114,  
                        0.0837,  0.0335,  0.0214,  0.0356,  0.0758,  0.1778,  
                        0.2354,  0.2254,  0.2484,  0.2207,  0.1470,  0.0528,  
                        0.0424,  0.0131,  0.0000,  0.0073,  0.0262,  0.0638,  
                        0.0727,  0.1851,  0.2395,  0.2150,  0.1574,  0.1250,  
                        0.0816,  0.0345,  0.0209,  0.0094,  0.0445,  0.0868,  
                        0.1898,  0.2594,  0.3358,  0.3504,  0.3708,  0.2500,  
                        0.1438,  0.0445,  0.0690,  0.2976,  0.6354,  0.7233,  
                        0.5397,  0.4482,  0.3379,  0.1919,  0.1266,  0.0560,  
                        0.0785,  0.2097,  0.3216,  0.5152,  0.6522,  0.5036,  
                        0.3483,  0.3373,  0.2829,  0.2040,  0.1077,  0.0350,  
                        0.0225,  0.1187,  0.2866,  0.4906,  0.5010,  0.4038,  
                        0.3091,  0.2301,  0.2458,  0.1595,  0.0853,  0.0382,  
                        0.1966,  0.3870,  0.7270,  0.5816,  0.5314,  0.3462,  
                        0.2338,  0.0889,  0.0591,  0.0649,  0.0178,  0.0314,  
                        0.1689,  0.2840,  0.3122,  0.3332,  0.3321,  0.2730,  
                        0.1328,  0.0685,  0.0356,  0.0330,  0.0371,  0.1862,  
                        0.3818,  0.4451,  0.4079,  0.3347,  0.2186,  0.1370,  
                        0.1396,  0.0633,  0.0497,  0.0141,  0.0262,  0.1276,  
                        0.2197,  0.3321,  0.2814,  0.3243,  0.2537,  0.2296,  
                        0.0973,  0.0298,  0.0188,  0.0073,  0.0502,  0.2479,  
                        0.2986,  0.5434,  0.4215,  0.3326,  0.1966,  0.1365,  
                        0.0743,  0.0303,  0.0873,  0.2317,  0.3342,  0.3609,  
                        0.4069,  0.3394,  0.1867,  0.1109,  0.0581,  0.0298,  
                        0.0455,  0.1888,  0.4168,  0.5983,  0.5732,  0.4644,  
                        0.3546,  0.2484,  0.1600,  0.0853,  0.0502,  0.1736,  
                        0.4843,  0.7929,  0.7128,  0.7045,  0.4388,  0.3630,  
                        0.1647,  0.0727,  0.0230,  0.1987,  0.7411,  0.9947,  
                        0.9665,  0.8316,  0.5873,  0.2819,  0.1961,  0.1459,  
                        0.0534,  0.0790,  0.2458,  0.4906,  0.5539,  0.5518,  
                        0.5465,  0.3483,  0.3603,  0.1987,  0.1804,  0.0811,  
                        0.0659,  0.1428,  0.4838,  0.8127 

                      };

REAL                  Mean; // Mean of the sunspot data.
REAL                  TrainError; // Error on the training set.
REAL                  TrainErrorPredictingMean; // Error on the training set when predicting the mean.
REAL                  TestError; // Error on the test set.
REAL                  TestErrorPredictingMean; // Error on the test set when predicting the mean.

FILE*                 f; // File to write the results to.

// Normalize the sunspot data to the range [-1, 1], because this makes it easier for the network to learn.
// When predicting the mean, the error should be 0.
// The mean is also calculated.
// The normalized data is stored in Sunspots_.
// The original data is stored in Sunspots.
void NormalizeSunspots()
{
  INT  Year; // The current year.
  REAL Min, Max; // The minimum and maximum number of sunspots.
	
  Min = MAX_REAL; // Set the minimum to the maximum possible value.
  Max = MIN_REAL; // Set the maximum to the minimum possible value.

  // Find the minimum and maximum number of sunspots.
  // The minimum and maximum number of sunspots are used to normalize the data.
  for (Year=0; Year<NUM_YEARS; Year++)
  {
    Min = MIN(Min, Sunspots[Year]); // Find the minimum number of sunspots.
    Max = MAX(Max, Sunspots[Year]); // Find the maximum number of sunspots.
  }

  Mean = 0; // Initialize the mean to 0.

  // Normalize the data and calculate the mean.
  for (Year=0; Year<NUM_YEARS; Year++)
  {
    // Normalize the data to the range [-1, 1].
    // Sunspots_[Year] = ((Sunspots[Year]-Min) / (Max-Min)) * (1-(-1)) + (-1);
    Sunspots_[Year] = Sunspots [Year] = ((Sunspots[Year]-Min) / (Max-Min)) * (HI-LO) + LO;
    // Calculate the mean.
    Mean += Sunspots[Year] / NUM_YEARS;
  }
}

// Initialize the application.
// This function is called once when the program starts.
void InitializeApplication(NET* Net)
{
  INT  Year, i; // The current year.
  REAL Out, Err; // The output and error of the network.

  //Arrow operator is used to access the members of the structure.
  // Initialize the network.
  Net->Alpha = 0.5; // Set the momentum term.
  Net->Eta   = 0.05; // Set the learning rate.
  Net->Gain  = 1; // Set the gain of the sigmoid function.

  // Initialize the training and test sets.
  NormalizeSunspots();
  TrainErrorPredictingMean = 0;

  // Calculate the error on the test set when predicting the mean.
  for (Year=TRAIN_LWB; Year<=TRAIN_UPB; Year++)
  {
    // Calculate the output of the network.
    for (i=0; i<M; i++)
    {
      Out = Sunspots[Year+i]; // Get the current input.
      Err = Mean - Out; // Calculate the error.
      TrainErrorPredictingMean += 0.5 * sqr(Err); // Calculate the error.
    }
  }

  TestErrorPredictingMean = 0;

  // Calculate the error on the test set when predicting the mean.
  for (Year=TEST_LWB; Year<=TEST_UPB; Year++)
  {
    // Calculate the output of the network.
    for (i=0; i<M; i++)
    {
      Out = Sunspots[Year+i]; // Get the current input.
      Err = Mean - Out; // Calculate the error.
      TestErrorPredictingMean += 0.5 * sqr(Err); // Calculate the error.
    }
  }

  // Open the file to write the results to.
  f = fopen("BPN.txt", "w");
}

// Finalize the application.
void FinalizeApplication(NET* Net)
{
  // Close the file.
  fclose(f);
}


/******************************************************************************
                          I N I T I A L I Z A T I O N
 ******************************************************************************/

// Initialize the network.
// This function is called once when the program starts.
// The network is initialized with random weights.
// The momentum term is set to 0.
// The learning rate is set to 0.1.
// The gain of the sigmoid function is set to 1.
// The number of layers and units are set to the values in the global variables.
// The number of inputs and outputs are set to the values in the global variables.
// The memory for the layers, units, weights, outputs, errors, and weight updates is allocated.
void GenerateNetwork(NET* Net)
{
  // The network has 3 layers.
  // The input layer has 1 unit.
  // The hidden layer has 2 units.
  // The output layer has 1 unit.

  INT l,i;

  // Allocate memory for the layers.
  Net->Layer = (LAYER**) calloc(NUM_LAYERS, sizeof(LAYER*));

  // Allocate memory for the input layer.
  for (l=0; l<NUM_LAYERS; l++)
  {
    // Allocate memory for the layer.
    Net->Layer[l] = (LAYER*) malloc(sizeof(LAYER));

    // Initialize the layer.
    Net->Layer[l]->Units      = Units[l];
    // Allocate memory for the units.
    Net->Layer[l]->Output     = (REAL*)  calloc(Units[l]+1, sizeof(REAL));
    // Allocate memory for the errors.
    Net->Layer[l]->Error      = (REAL*)  calloc(Units[l]+1, sizeof(REAL));
    // Allocate memory for the errors.
    Net->Layer[l]->Weight     = (REAL**) calloc(Units[l]+1, sizeof(REAL*));
    // Allocate memory for the errors.
    Net->Layer[l]->WeightSave = (REAL**) calloc(Units[l]+1, sizeof(REAL*));
    // Allocate memory for the errors.
    Net->Layer[l]->dWeight    = (REAL**) calloc(Units[l]+1, sizeof(REAL*));
    // Set the bias.
    Net->Layer[l]->Output[0]  = BIAS;

    // Allocate memory for the weights.
    if (l != 0)
    {
      // Allocate memory for the units.
      for (i=1; i<=Units[l]; i++)
      {
        // Allocate memory for the weights.
        Net->Layer[l]->Weight[i]     = (REAL*) calloc(Units[l-1]+1, sizeof(REAL));
        // Allocate memory for the weights.
        Net->Layer[l]->WeightSave[i] = (REAL*) calloc(Units[l-1]+1, sizeof(REAL));
        // Allocate memory for the weights.
        Net->Layer[l]->dWeight[i]    = (REAL*) calloc(Units[l-1]+1, sizeof(REAL));
      }
    }
  }

  // Set the input layer.
  Net->InputLayer  = Net->Layer[0];
  // Set the output layer.
  Net->OutputLayer = Net->Layer[NUM_LAYERS - 1];
  // Set the target layer.
  Net->Alpha       = 0.9;
  // Set the target layer.
  Net->Eta         = 0.25;
  // Set the target layer.
  Net->Gain        = 1;
}

// Randomize the weights of the network.
// This function is called once when the program starts.
// The weights are randomized between -0.5 and 0.5.
// The seed of the random number generator is set based on the current time.
// This ensures that each time the program is run, the weights are different.
// The seed of the random number generator is also printed to the screen.
// This allows you to replicate the results of a run.
// The seed of the random number generator is also printed to a file.
void RandomWeights(NET* Net)
{
  // The weights are randomized between -0.5 and 0.5.
  INT l,i,j;

  // Initialize the weights between the input layer and the hidden layer.
  for (l=1; l<NUM_LAYERS; l++)
  {
    // Initialize the weights between the input layer and the hidden layer.
    for (i=1; i<=Net->Layer[l]->Units; i++)
    {
      // Initialize the weights between the input layer and the hidden layer.
      for (j=0; j<=Net->Layer[l-1]->Units; j++)
      {
        // Initialize the weights between the input layer and the hidden layer.
        Net->Layer[l]->Weight[i][j] = RandomEqualREAL(-0.5, 0.5);
      }
    }
  }
}

// Initialize the network.
// This function is called once when the program starts.
// The network is initialized with random weights.
// The momentum term is set to 0.
// The learning rate is set to 0.1.
// The gain of the sigmoid function is set to 1.
// The weights are read from a file.
// The weights are not trained.
// The weights are not saved to a file.
void SetInput(NET* Net, REAL* Input)
{
  INT i;

  // Set the input layer.
  for (i=1; i<=Net->InputLayer->Units; i++)
  {
    // Set the input layer.
    Net->InputLayer->Output[i] = Input[i-1];
  }
}

// Set the target output.
// This function is called once for every input.
// The target output is read from a file.
// The target output is copied to the output layer.
// The output layer is used to calculate the error of the network.
// The error is used to adjust the weights of the network.
// The error is not backpropagated to the previous layers.
// The error is not returned by the function.
// The error is used by the function TrainNetwork.
void GetOutput(NET* Net, REAL* Output)
{
  INT i;

  // Set the output layer.
  for (i=1; i<=Net->OutputLayer->Units; i++)
  {
    // Set the output layer.
    Output[i-1] = Net->OutputLayer->Output[i];
  }
}


/******************************************************************************
            S U P P O R T   F O R   S T O P P E D   T R A I N I N G
 ******************************************************************************/

// Save the weights of the network.
// This function is called when the training is stopped.
// The weights are saved to a file.
// The file can be used later to resume the training.
void SaveWeights(NET* Net)
{
  INT l,i,j;

  // Save the weights.
  for (l=1; l<NUM_LAYERS; l++)
  {
    // Save the weights.
    for (i=1; i<=Net->Layer[l]->Units; i++)
    {
      // Save the weights.
      for (j=0; j<=Net->Layer[l-1]->Units; j++)
      {
        // Save the weights.
        Net->Layer[l]->WeightSave[i][j] = Net->Layer[l]->Weight[i][j];
      }
    }
  }
}

// Restore the weights of the network.
// This function is called when the training is stopped.
// The weights are read from a file.
// The file can be used later to resume the training.
// The weights are not saved to a file.
void RestoreWeights(NET* Net)
{
  INT l,i,j;

  // Restore the weights.
  // The weights are read from a file.
  for (l=1; l<NUM_LAYERS; l++)
  {
    // Restore the weights.
    for (i=1; i<=Net->Layer[l]->Units; i++)
    {
      // Restore the weights.
      for (j=0; j<=Net->Layer[l-1]->Units; j++)
      {
        // Restore the weights.
        Net->Layer[l]->Weight[i][j] = Net->Layer[l]->WeightSave[i][j];
      }
    }
  }
}


/******************************************************************************
                     P R O P A G A T I N G   S I G N A L S
 ******************************************************************************/

// Propagate the signals from the input layer to the output layer.
// This function is called once for every input.
// The signals are propagated from the input layer to the output layer.
// The signals are propagated one layer at a time.
// The signals are propagated from the input layer to the first hidden layer.
// The signals are propagated from one hidden layer to the next hidden layer.
void PropagateLayer(NET* Net, LAYER* Lower, LAYER* Upper)
{
  INT  i,j;
  REAL Sum;

  // Propagate the signals from the input layer to the output layer.
  for (i=1; i<=Upper->Units; i++)
  {
    Sum = 0;

    // Propagate the signals from the input layer to the output layer.
    for (j=0; j<=Lower->Units; j++)
    {
      // Propagate the signals from the input layer to the output layer.
      Sum += Upper->Weight[i][j] * Lower->Output[j];
    }

    // Propagate the signals from the input layer to the output layer.
    // The gain of the sigmoid function is set to 1.
    Upper->Output[i] = 1 / (1 + exp(-Net->Gain * Sum));
  }
}

// Propagate the signals from the input layer to the output layer.
void PropagateNet(NET* Net)
{
  INT l;


  // Propagate the signals from the input layer to the output layer.
  for (l=0; l<NUM_LAYERS-1; l++)
  {
    // Propagate the signals from the input layer to the output layer.
    PropagateLayer(Net, Net->Layer[l], Net->Layer[l+1]);
  }
}


/******************************************************************************
                  B A C K P R O P A G A T I N G   E R R O R S
 ******************************************************************************/

// Compute the error of the output layer.
// This function is called once for every input.
// The error of the output layer is computed.
// The error of the output layer is computed using the target output.
void ComputeOutputError(NET* Net, REAL* Target)
{
  INT  i;
  REAL Out, Err;
   
  Net->Error = 0;
  for (i=1; i<=Net->OutputLayer->Units; i++) {
    Out = Net->OutputLayer->Output[i];
    Err = Target[i-1]-Out;
    Net->OutputLayer->Error[i] = Net->Gain * Out * (1-Out) * Err;
    Net->Error += 0.5 * sqr(Err);
  }
}


void BackpropagateLayer(NET* Net, LAYER* Upper, LAYER* Lower)
{
  INT  i,j;
  REAL Out, Err;
   
  for (i=1; i<=Lower->Units; i++) {
    Out = Lower->Output[i];
    Err = 0;
    for (j=1; j<=Upper->Units; j++) {
      Err += Upper->Weight[j][i] * Upper->Error[j];
    }
    Lower->Error[i] = Net->Gain * Out * (1-Out) * Err;
  }
}


void BackpropagateNet(NET* Net)
{
  INT l;
   
  for (l=NUM_LAYERS-1; l>1; l--) {
    BackpropagateLayer(Net, Net->Layer[l], Net->Layer[l-1]);
  }
}


void AdjustWeights(NET* Net)
{
  INT  l,i,j;
  REAL Out, Err, dWeight;
   
  for (l=1; l<NUM_LAYERS; l++) {
    for (i=1; i<=Net->Layer[l]->Units; i++) {
      for (j=0; j<=Net->Layer[l-1]->Units; j++) {
        Out = Net->Layer[l-1]->Output[j];
        Err = Net->Layer[l]->Error[i];
        dWeight = Net->Layer[l]->dWeight[i][j];
        Net->Layer[l]->Weight[i][j] += Net->Eta * Err * Out + Net->Alpha * dWeight;
        Net->Layer[l]->dWeight[i][j] = Net->Eta * Err * Out;
      }
    }
  }
}


/******************************************************************************
                      S I M U L A T I N G   T H E   N E T
 ******************************************************************************/


void SimulateNet(NET* Net, REAL* Input, REAL* Output, REAL* Target, BOOL Training)
{
  SetInput(Net, Input);
  PropagateNet(Net);
  GetOutput(Net, Output);
   
  ComputeOutputError(Net, Target);
  if (Training) {
    BackpropagateNet(Net);
    AdjustWeights(Net);
  }
}


void TrainNet(NET* Net, INT Epochs)
{
  INT  Year, n;
  REAL Output[M];

  for (n=0; n<Epochs*TRAIN_YEARS; n++) {
    Year = RandomEqualINT(TRAIN_LWB, TRAIN_UPB);
    SimulateNet(Net, &(Sunspots[Year-N]), Output, &(Sunspots[Year]), TRUE);
  }
}


void TestNet(NET* Net)
{
  INT  Year;
  REAL Output[M];

  TrainError = 0;
  for (Year=TRAIN_LWB; Year<=TRAIN_UPB; Year++) {
    SimulateNet(Net, &(Sunspots[Year-N]), Output, &(Sunspots[Year]), FALSE);
    TrainError += Net->Error;
  }
  TestError = 0;
  for (Year=TEST_LWB; Year<=TEST_UPB; Year++) {
    SimulateNet(Net, &(Sunspots[Year-N]), Output, &(Sunspots[Year]), FALSE);
    TestError += Net->Error;
  }
  fprintf(f, "\nNMSE is %0.3f on Training Set and %0.3f on Test Set",
             TrainError / TrainErrorPredictingMean,
             TestError / TestErrorPredictingMean);
}


void EvaluateNet(NET* Net)
{
  INT  Year;
  REAL Output [M];
  REAL Output_[M];

  fprintf(f, "\n\n\n");
  fprintf(f, "Year    Sunspots    Open-Loop Prediction    Closed-Loop Prediction\n");
  fprintf(f, "\n");
  for (Year=EVAL_LWB; Year<=EVAL_UPB; Year++) {
    SimulateNet(Net, &(Sunspots [Year-N]), Output,  &(Sunspots [Year]), FALSE);
    SimulateNet(Net, &(Sunspots_[Year-N]), Output_, &(Sunspots_[Year]), FALSE);
    Sunspots_[Year] = Output_[0];
    fprintf(f, "%d       %0.3f                   %0.3f                     %0.3f\n",
               FIRST_YEAR + Year,
               Sunspots[Year],
               Output [0],
               Output_[0]);
  }
}


/******************************************************************************
                                    M A I N
 ******************************************************************************/


int main()
{
  NET  Net;
  BOOL Stop;
  REAL MinTestError;

  InitializeRandoms();
  GenerateNetwork(&Net);
  RandomWeights(&Net);
  InitializeApplication(&Net);

  Stop = FALSE;
  MinTestError = MAX_REAL;
  do {
    TrainNet(&Net, 10);
    TestNet(&Net);
    if (TestError < MinTestError) {
      fprintf(f, " - saving Weights ...");
      MinTestError = TestError;
      SaveWeights(&Net);
    }
    else if (TestError > 1.2 * MinTestError) {
      fprintf(f, " - stopping Training and restoring Weights ...");
      Stop = TRUE;
      RestoreWeights(&Net);
    }
  } while (NOT Stop);

  TestNet(&Net);
  EvaluateNet(&Net);
   
  FinalizeApplication(&Net);

  return 0;
}
