(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]];
<< RBM`


dataPath = "E:\\Files\\Programs\\machine-learning\\data\\mnist\\";
{imageTrainRaw, imageTestRaw} =
  Import[dataPath <> #, "UnsignedInteger8", "HeaderBytes" -> 16] & /@
    {"train-images-idx3-ubyte", "t10k-images-idx3-ubyte"};
Dimensions /@ %


(* Binarize and reshape *)
{imageTrain, imageTest} = Round[Partition[#, 784] / 255.0] & /@
  {imageTrainRaw[[;; 784 * 1000]], imageTestRaw};
Dimensions /@ %


(* Parameters *)
visibleNum   = 784;
hiddenNum    = 100;
epochNum     = 20;
batchSize    = 20;
kParameter   = 10;
sampleNum    = 1;
momentum     = 0.0;
learningRate = 0.1;

(* Helper function *)
plotMNIST[data_] := MatrixPlot[ArrayReshape[data, {28, 28}],
  ImageSize -> 60, Frame -> False]

(* Initialize *)
rbm = AssociationThread[{"w", "b", "c"} ->
  Evaluate[RandomReal[{0, 1}, #] & /@
    {{visibleNum, hiddenNum}, visibleNum, hiddenNum}]];
rbmVelocity = AssociationThread[{"w", "b", "c"} ->
  Evaluate[ConstantArray[0.0, #] & /@
    {{visibleNum, hiddenNum}, visibleNum, hiddenNum}]];

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
trained$w = Transpose @ trained["rbm_union"]["variable"]["w"];
GraphicsGrid @ Partition[#, 10] & @ ParallelMap[plotMNIST, trained$w]

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
