(* ::Package:: *)

(* ::Section:: *)
(*Load Ising data*)


Remove["Global`*"]


SetDirectory[NotebookDirectory[] <> "\\data\\lattice-data"]


ising$Tc   = N[2 / Log[Sqrt[2] + 1]]
T$list     = Range[1.0, 3.5, 0.1];
phase$list = Boole /@ GreaterThan[ising$Tc] /@ T$list;
size$list  = {4, 8, 16, 32, 64, 128};


data$train$size = 50;
data$test$size  = 10;


importData[type_String, size_, index_] :=
  Import[type <> "-" <> "size" <> ToString[size]
              <> "-" <> ToString[index] <> ".json", "RawJSON"]


(*
  Format:
    {1D lattice data} -> 0 or 1,
    {1D lattice data} -> 0 or 1,
    ...
  Size: equal to (number of training data) * (number of temperatures)
*)
importDataTrain[size_] :=
  RandomSample @ Flatten @ Map[
    Function[index,
      (#1 -> #2) & @@@ Transpose[{Flatten /@ (#["latticeData"] & /@
        importData["train", size, index]), phase$list}]],
    Range @ data$train$size];
(*
  Format:
    {1D lattice data},
    {1D lattice data},
    ...
  Size: equal to (number of test data) * (number of temperatures)
*)
importDataTest[size_] :=
  Function[index, Flatten /@ (#["latticeData"] & /@
    importData["train", size, index])] /@
  Range @ data$test$size;


data$train = importDataTrain /@ size$list;
data$test  = importDataTest  /@ size$list;
Echo[Quantity[N @ #, "Megabytes"], "Train data size:"] & @
  (ByteCount[data$train] / 2^20);
Echo[Quantity[N @ #, "Megabytes"], "Test data size:"] & @
  (ByteCount[data$test] / 2^20);


(* ::Section:: *)
(*Train*)


trained = ParallelMap[
  NetTrain[NetChain[{LinearLayer[], LogisticSigmoid}], #, All, TimeGoal -> 40] &,
  data$train,
  Method -> "FinestGrained"]


(* ::Section:: *)
(*Plot*)


result = AssociationThread[T$list -> #] & /@ Mean /@
  MapThread[Function[{t, d}, t["TrainedNet"][#, None] & /@ d],
    {trained, data$test}];
ListPlot[result, PlotRange -> All,
  PlotTheme -> "Scientific", GridLines -> {{ising$Tc}, None},
  PlotLegends -> size$list]


T$scale[T_, size_] := (T - ising$Tc) * size


result$scale = MapThread[Function[{ass, size}, KeyMap[T$scale[#, size] &, ass]],
  {result, size$list}];
ListPlot[result$scale, PlotRange -> All(*{{-20, 20}, All}*),
  PlotTheme -> "Scientific", GridLines -> {{ising$Tc}, None},
  PlotLegends -> size$list]
