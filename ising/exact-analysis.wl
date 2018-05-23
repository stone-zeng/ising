(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]]


isingExe          = "x64\\Release\\ising.exe";


timeString        = DateString[Riffle[{"ISODate", "Hour", "Minute", "Second"}, "-"]];
isingSettingsFile = "simulation-settings-" <> timeString <> ".json"
isingResultFile   = "simulation-result-"   <> timeString <> ".csv"


isingTc = N[2 / Log[Sqrt[2] + 1]]


(* ::Section:: *)
(*Write settings*)


sizeList = {4, 8, 16, 32, 64, 128, 256, 512};
tList    = Sort @ DeleteDuplicates @ Join[
  Range[1.0, 2.0, 0.1],   Range[2.6, 3.6, 0.1],
  Range[1.0, 2.6, 0.001]];


settings =
  {
    "size.list" -> sizeList,
    "temperature.list" -> tList
  };
Export[isingSettingsFile, settings, "JSON", "Compact" -> True];


data=Rest @ Import["simulation-result-2018-05-23-11-10-03.csv"];
Dimensions[%]


(* ::Section:: *)
(*Run ising.exe*)


AbsoluteTiming[Run[isingExe
  <> " --exact"
  <> " --settings=" <> isingSettingsFile
  <> " >" <> isingResultFile];]
data = Import[isingResultFile][[2;;]];
Dimensions[data]
(*DeleteFile[{isingSettingsFile, isingResultFile}];*)


(* ::Section:: *)
(*Plot*)


cvDataExact = Transpose[Tuples@{{First[#]},Rest[#]}&/@data];
ListPlot[cvDataExact,Joined->True,PlotRange->All]


MaximalBy[#,Last]&/@cvDataExact;
$data = {sizeList,1/(First/@Mean/@%-isingTc)}\[Transpose];
lm=LinearModelFit[$data/.{x_,y_}->Log@{x,y}, x,x]
lm["AdjustedRSquared"]
Show[ListLogLogPlot[$data], LogLogPlot[Exp@lm[Log@s],{s,1,500}]]
Show[ListLogLogPlot[$data/.{x_,y_}->Log@{x,y}], LogLogPlot[lm[s],{s,1,1000}]]


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
