(* ::Package:: *)

$HistoryLength=\[Infinity]


(* ::Section:: *)
(*Load Ising data*)


Remove["Global`*"]


SetDirectory[NotebookDirectory[] <> "\\data\\lattice-data\\3"]


ising$Tc   = N[2 / Log[Sqrt[2] + 1]]
T$list     = Range[1.0, 3.6, 0.1];
phase$list = Boole /@ GreaterThan[ising$Tc] /@ T$list;
size$list  = {8, 12, 16, 24, 32}(*{8, 12, 16, 24, 32, 48, 64}*);


data$train$filename = "lattice-result-2018-05-18-13-43-17.json";
data$test$filename  = "lattice-result-2018-05-18-13-43-17.json";


raw$data$train = Import[data$train$filename, "RawJSON"];
raw$data$test  = Import[data$test$filename,  "RawJSON"];


phaseQ[temperature_] :=
  Boole[temperature > ising$Tc];


(*
  Format:
    {1D lattice data} -> 0 or 1,
    {1D lattice data} -> 0 or 1,
    ...
  Size: equal to (number of training data) * (number of temperatures)
*)
importDataTrain[size_] :=
  RandomSample @ Flatten @ Map[
    Thread[(Flatten[#["latticeData"], {{1}, {2, 3}}] + 1) / 2 -> phaseQ[#["temperature"]]] &,
    Select[raw$data$train, (#["size"] == size) &]
  ]
(*
  Format:
    {1D lattice data},
    {1D lattice data},
    ...
  Size: equal to (number of test data) * (number of temperatures)
*)
importDataTest[size_] :=
  Map[
    Thread[(Flatten[#["latticeData"], {{1}, {2, 3}}] + 1) / 2 -> phaseQ[#["temperature"]]] &,
    Select[raw$data$test, (#["size"] == size) &]
  ]


data$train = #[[;;]] & /@ importDataTrain /@ size$list;
data$test  = importDataTest  /@ size$list;
Echo[#, "Train data dimension:"] & @ Dimensions[data$train];
Echo[#, "Test data dimension:"] & @ Dimensions[data$test];
Echo[Quantity[N @ #, "Megabytes"], "Train data size:"] & @
  (ByteCount[data$train] / 2^20);
Echo[Quantity[N @ #, "Megabytes"], "Test data size:"] & @
  (ByteCount[data$test] / 2^20);


(* ::Section:: *)
(*Train*)


trained = Map[
  NetTrain[NetChain[{LinearLayer[], LogisticSigmoid}], #, All, MaxTrainingRounds -> 50] &,
  data$train]


trained = Map[
  With[{size = Length @ #[[1, 1]]},
    NetTrain[NetChain[{LinearLayer[size / 4, "Input" -> size], LogisticSigmoid, LinearLayer[], LogisticSigmoid}], #, All, MaxTrainingRounds -> 100]] &,
  data$train]


(* ::Section:: *)
(*Plot*)


Needs["ErrorBarPlots`"]


$mean =  MapThread[Function[{t, d}, Mean /@ (t["TrainedNet"][#, None] & /@ d)],
  {trained, (# /. (x_ -> y_) -> x) & /@ data$test}];
$error = MapThread[Function[{t, d}, StandardDeviation /@ (t["TrainedNet"][#, None] & /@ d)],
  {trained, (# /. (x_ -> y_) -> x) & /@ data$test}];
MapThread[Function[{x,y,dy}, {{x,y}, ErrorBar[dy]}],
  {ConstantArray[T$list, Length@size$list], $mean, $error}, 2];
ErrorListPlot[%,PlotRange->{-0.25,1.25},Joined->True,PlotTheme->"Detailed",PlotLegends -> size$list]


result = AssociationThread[T$list -> #] & /@ 
  MapThread[Function[{t, d}, Mean /@ (t["TrainedNet"][#, None] & /@ d)],
    {trained, (# /. (x_ -> y_) -> x) & /@ data$test}];
ListPlot[result, PlotRange -> {-0.05,1.05}, Joined -> True,
  PlotTheme -> "Scientific", GridLines -> {{ising$Tc}, None},
  PlotLegends -> size$list]


T$scale[T_, size_] := (T - ising$Tc) * size


result$scale = MapThread[Function[{ass, size}, KeyMap[T$scale[#, size] &, ass]],
  {result, size$list}];
ListPlot[result$scale, PlotRange -> {-0.05,1.05}(*{{-20, 20}, All}*), Joined -> True,
  PlotTheme -> "Scientific", GridLines -> {{0}, None},
  PlotLegends -> size$list]


Interpolation /@ ({T$list, #}\[Transpose] & /@ Values /@ result)
Plot[#[x], {x,1,3.6}] & /@ %
FindRoot[#[x] == 0.5, {x, 2.2, 1.8, 3.0}] & /@ %%
ListLinePlot[{1/size$list,x/.%}\[Transpose][[;;5]], Mesh->All]



