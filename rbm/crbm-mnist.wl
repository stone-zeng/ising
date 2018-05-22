(* ::Package:: *)

LaunchKernels[]


Remove["Global`*"]
SetDirectory[NotebookDirectory[]];
<< CRBM`


dataPath = "E:\\Files\\Programs\\machine-learning\\data\\mnist\\";
{imageTrainRaw, imageTestRaw} =
  Import[dataPath <> #, "UnsignedInteger8", "HeaderBytes" -> 16] & /@
    {"train-images-idx3-ubyte", "t10k-images-idx3-ubyte"};
Dimensions @ imageTrainRaw
Dimensions @ imageTestRaw


(* Binarize and reshape *)
{imageTrain, imageTest} = Round[ArrayReshape[#, {28, 28}] & /@ Partition[#, 784] / 255.0] & /@
  {imageTrainRaw[[;; 784 * 1000]], imageTestRaw};
Dimensions @ imageTrain
Dimensions @ imageTest


(* Parameters *)
visibleNum   = {28, 28};
hiddenNum    = {5, 5};
epochNum     = 10;
batchSize    = 50;
kParameter   = 1;
sampleNum    = 1;
momentum     = 0.0;
learningRate = 0.1;

(* Helper function *)
plotMNIST[data_] := MatrixPlot[data, ImageSize -> 60, Frame -> False]

(* Initialize *)
rbm = AssociationThread[{"w", "b", "c"} ->
  Evaluate[RandomReal[{-1, 1}, #] & /@
    {Flatten @ {visibleNum, hiddenNum}, visibleNum, hiddenNum}]];
rbmVelocity = AssociationThread[{"w", "b", "c"} ->
  Evaluate[ConstantArray[0.0, #] & /@
    {Flatten @ {visibleNum, hiddenNum}, visibleNum, hiddenNum}]];

(* Main training loop *)
trainingTime = First @ AbsoluteTiming[
  trained = train[imageTrain, rbm, rbmVelocity,
    epochNum, batchSize, kParameter, sampleNum, momentum, learningRate];];

(* Training time and cost *)
Echo[#, "Training time:"] & @ Quantity[trainingTime, "Seconds"];
Echo[#, "Memory used:"]   & @ Quantity[MemoryInUse[] / 2^20., "Megabytes"];
ListLogLogPlot[-trained["cost_list"],
  PlotRange -> All, Joined -> True,
  PlotTheme -> "Detailed", PlotLabel -> "Learning curve"]

(* Filters *)
trained$w = trained["rbm_union"]["variable"]["w"];
Transpose[trained$w,{3, 4, 1, 2}];
GraphicsGrid @ ParallelMap[plotMNIST, %, {2}]

(* Generated samples *)
sample$num   = 20;
sample$step  = 100;
sample$index = RandomSample[Range @ Length @ imageTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["rbm_union"]["variable"], #["data"], sample$step],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> imageTest[[sample$index]],
    "index" -> 0
  |>, 5];
Echo[Row[plotMNIST /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]] & /@ sample;


(*
Export["data\\mnist-trained.json",
  KeyTake[trained, {"rbm_union", "cost_list"}],
  "Compact" -> True]
*)


GraphicsGrid @ ParallelMap[MatrixPlot[#, ImageSize -> 60, Frame -> False] &, Transpose[rbmVelocity["w"],{3,4,1,2}], {2}]


trained[[1,1,1]]//Dimensions
