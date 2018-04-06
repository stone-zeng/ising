(* ::Package:: *)

Remove["Global`*"]


$isingCriticalT = 2 / Log[1 + Sqrt[2]];


$path = NotebookDirectory[];
$name = "result.csv";


data = Import[$path <> $name];
Dimensions[data]


sizeList = data[[1, 2 ;;]];
tempList = N @ data[[2 ;;, 1]];


(* ::Section:: *)
(*Plot*)


plotData = Transpose[{tempList, data[[2 ;;, # + 1]]}] & /@ Range @ Length @ sizeList;


ListPlot[plotData, Joined -> True, PlotRange -> All,
  PlotTheme -> "Scientific",
  PlotLegends -> sizeList, GridLines -> {{$isingCriticalT}, None},
  AspectRatio -> 2, ImageSize -> 300]


(* ::Section:: *)
(*Fit*)


maxPos = Flatten[Ordering[data[[2 ;;, # + 1]], -1] & /@ Range @ Length @ sizeList];
maxPosVal = Transpose @ {sizeList, tempList[[maxPos]]};
ListPlot[maxPosVal,
  PlotTheme -> "Scientific", GridLines->Automatic]


lm = LinearModelFit[maxPosVal /. {x_, y_} -> {Log[x], Log[y - $isingCriticalT]}, x, x]
lm["ParameterTable"]
lm["AdjustedRSquared"]


Show[
  ListLogLogPlot[maxPosVal /. {x_, y_} -> {x, y - $isingCriticalT},
    PlotTheme -> "Scientific", GridLines->Automatic],
  Plot[lm[x], {x, 0, 1000}]
]


{$b, $k} = lm["BestFitParameters"];
Limit[Exp[$k * Log[L]+ $b] + $isingCriticalT, L -> Infinity]


Show[
  ListPlot[maxPosVal,
    PlotTheme -> "Scientific", GridLines->Automatic],
  Plot[Exp[$k * Log[L]+ $b] + $isingCriticalT, {L, 0, 1000}, PlotRange -> All]
]
