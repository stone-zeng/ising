(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]];
<< RBM`


SetDirectory["..\\ising\\data\\lattice-data"]


ising$Tc   = N[2 / Log[Sqrt[2] + 1]];
T$list     = Range[1.0, 3.5, 0.1];
phase$list = Boole /@ GreaterThan[ising$Tc] /@ T$list;
size$list  = {4, 8, 16, 32, 64, 128};

Echo[ising$Tc, "2D Ising model critical temperature:"];


data$train$size = 50;


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


data$train = importDataTrain /@ size$list;
Echo[Quantity[N @ #, "Megabytes"], "Train data size:"] & @
  (ByteCount[data$train] / 2^20);


data$train//Dimensions


data$train[[1]]
