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


sizeList = {4, 8, 16, 32, 64, 128, 256};
tList    = Sort @ DeleteDuplicates @ Join[
  Range[1.0, 1.8, 0.1],   Range[2.8, 3.6, 0.1],
  Range[1.8, 2.2, 0.05],   Range[2.5, 2.8, 0.05],
  Range[2.2, 2.26, 0.01],   Range[2.29, 2.5, 0.01],
  Range[2.26, 2.29, 0.001]];
Length[tList]


settings =
  {
    "size.list" -> sizeList,
    "temperature.list" -> tList
  };
Export[isingSettingsFile, settings, "JSON", "Compact" -> True];


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
ListPlot[cvDataExact,Joined->True,PlotRange->All,ImageSize->1000(*,Mesh\[Rule]All*)]


cvDataExact//Dimensions


Most@cvDataExact//Dimensions


$data//Dimensions


MaximalBy[#,Last]&/@cvDataExact;
$data = (Most/@{sizeList,1/(First/@Mean/@%-isingTc)})\[Transpose];
$data//TableForm
lm = NonlinearModelFit[$data,Exp[k + Log[x]], k, x]
lm["AdjustedRSquared"]
Show[ListPlot[$data], Plot[lm[s],{s,1,600}]]
Show[ListLogLogPlot[$data/.{x_,y_}->Log@{x,y}], LogLogPlot[lm[s],{s,1,1000}]]


MaximalBy[#,Last]&/@cvDataExact;
$data = (Most/@{sizeList,Last/@Mean/@%})\[Transpose];
$data//TableForm
lm = NonlinearModelFit[$data,k Log[x], k, x]
lm["AdjustedRSquared"]
Show[ListPlot[$data], Plot[lm[s],{s,0.1,600},PlotRange->All]]
Show[ListLogLinearPlot[$data], LogLinearPlot[lm[s],{s,0.1,600},PlotRange->All]]
