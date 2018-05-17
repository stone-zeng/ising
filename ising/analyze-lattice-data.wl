(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]]


isingExe          = "x64\\Release\\ising.exe";


timeString        = DateString[Riffle[{"ISODate", "Hour", "Minute", "Second"}, "-"]];
isingSettingsFile = "lattice-settings-" <> timeString <> ".json"
isingResultFile   = "lattice-result-"   <> timeString <> ".json"


isingTc = N[2 / Log[Sqrt[2] + 1]]


(* ::Section:: *)
(*Write settings*)


settings =
  {
    "size.list" -> {5},
    "boundary" -> "periodic",
    "temperature.list" -> Table[isingTc + i, {i, 0, 0.03, 0.01}],
    "externalMagneticField.list" -> {0.0},
    "iterations" -> 1000,
    "repetitions" -> 6000
  };
Export[isingSettingsFile, settings, "JSON", "Compact" -> True];


(* ::Section:: *)
(*Run ising.exe*)


AbsoluteTiming[Run[isingExe
  <> " --lattice"
  <> " --settings=" <> isingSettingsFile
  <> " >" <> isingResultFile];]
data = Association /@ Import[isingResultFile, "RawJSON"];
Dimensions[data]
(*DeleteFile[{isingSettingsFile, isingResultFile}];*)


#["latticeData"] & /@ data;
(% + 1) / 2;
Dimensions[%]
flattenedData = Flatten[%%, {{1},{2},{3,4}}];
Dimensions[flattenedData]


export[type_, span_Span, index_] :=
  Export["lattice-data-" <> type <> "-t" <> ToString[index] <> ".dat",
    flattenedData[[index, span]],
    "Table", "FieldSeparators" -> None]


ParallelMap[export["train", ;;5000, #] &, Range[4]];
ParallelMap[export["test", -1000;;-1, #] &, Range[4]];


Grid @ Transpose @ ParallelMap[ArrayPlot[(# + 1) / 2, ImageSize->Tiny] &, #["latticeData"] & /@ data, {2}]
