(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]];
<< RBM`


(* ::Section:: *)
(*MNIST*)


SetDirectory[StringRiffle[StringSplit[NotebookDirectory[], "\\"][[;;-2]], "\\"] <> "\\ising\\data\\lattice-data\\2"]


dataTrainRaw = ToCharacterCode /@ Import["lattice-data-train-t4.dat", "Lines"] - 48;
dataTestRaw  = ToCharacterCode /@ Import["lattice-data-test-t4.dat",  "Lines"] - 48;


dataTrain = Flatten[#[[;;16, ;;16]]] & /@ (ArrayReshape[#, {64, 64}] & /@ dataTrainRaw);
dataTest  = Flatten[#[[;;16, ;;16]]] & /@ (ArrayReshape[#, {64, 64}] & /@ dataTestRaw);
Echo[#, "Train data dimension:"] & @ Dimensions[dataTrain];
Echo[#, "Test data dimension:"] & @ Dimensions[dataTest];
Echo[Quantity[N @ #, "Megabytes"], "Train data size:"] & @
  (ByteCount[dataTrain] / 2^20);
Echo[Quantity[N @ #, "Megabytes"], "Test data size:"] & @
  (ByteCount[dataTest] / 2^20);


(* Parameters *)
visibleNum   = Length @ First @ dataTrain;
hiddenNum    = 64;
epochNum     = 40;
batchSize    = 20;
kParameter   = 10;
sampleNum    = 1;
momentum     = 0;
learningRate = 0.1;

(* Helper function *)
plotMNIST[data_] := MatrixPlot[ArrayReshape[data, Sqrt @ {visibleNum, visibleNum}],
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
  trained = train[dataTrain, rbm, rbmVelocity,
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
sample$index = RandomSample[Range @ Length @ dataTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["rbm_union"]["variable"], #["data"], sample$step],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> dataTest[[sample$index]],
    "index" -> 0
  |>, 5];
Echo[Row[plotMNIST /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]] & /@ sample;


1/2.30
