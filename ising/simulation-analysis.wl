(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]]


isingExe          = "x64\\Release\\ising.exe";


timeString        = DateString[Riffle[{"ISODate", "Hour", "Minute", "Second"}, "-"]];
isingSettingsFile = "simulation-settings-" <> timeString <> ".json"
isingResultFile   = "simulation-result-"   <> timeString <> ".json"


isingTc = N[2 / Log[Sqrt[2] + 1]]


(* ::Section:: *)
(*Write settings*)


sizeList = {8, 16, 32, 64, 128};


settings =
  {
    "size.list" -> sizeList,
    "boundary" -> "periodic",
    "temperature.list" -> DeleteDuplicates @ Join[Range[1.0, 1.8, 0.1], Range[1.8, 2.8, 0.02], Range[2.8, 3.6, 0.1]],
    "externalMagneticField.list" -> {0.0},
    "iterations" -> 1000,
    "repetitions" -> 50
  };
Export[isingSettingsFile, settings, "JSON", "Compact" -> True];


(* ::Section:: *)
(*Run ising.exe*)


(* Much slower *)
(*
AbsoluteTiming[resultString =
  RunProcess[{isingExe, "--simulation", "--settings", isingSettingsFile}, "StandardOutput"];]
data = Association /@ Interpreter["JSON"][resultString];
Dimensions[data]
DeleteFile[isingSettingsFile];
*)


AbsoluteTiming[Run[isingExe
  <> " --simulation"
  <> " --settings=" <> isingSettingsFile
  <> " >" <> isingResultFile];]
data = Association /@ Import[isingResultFile, "RawJSON"];
Dimensions[data]
(*DeleteFile[{isingSettingsFile, isingResultFile}];*)


data = Association /@ Import["simulation-result-2018-05-23-00-27-06.json", "RawJSON"];
Dimensions[data]


(* ::Section:: *)
(*Plots*)


Needs["ErrorBarPlots`"]


errorBarFunc = Function[{coords, err},
  {Opacity[0.6], #} & @ Line @
    {
      {coords + {0, err[[2, 1]]}, coords + {0, err[[2, 2]]}},
      {coords + {-0.01, err[[2, 1]]}, coords + {0.01, err[[2, 1]]}},
      {coords + {-0.01, err[[2, 2]]}, coords + {0.01, err[[2, 2]]}}
    }
    (*{
      {coords + {0, err[[2, 1]]}, coords + {0, err[[2, 2]]}},
      {coords + {err[[1, 1]], err[[2, 1]]}, coords + {err[[1, 2]], err[[2, 1]]}},
      {coords + {err[[1, 1]], err[[2, 2]]}, coords + {err[[1, 2]], err[[2, 2]]}}
    }*)
  ];


energyData = Map[{{#["temperature"], Mean[#["energy"]]},
    ErrorBar @ (Mean[#["energy.Square"]] - Mean[#["energy"]]^2)} &,
  Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];
energyPlot = ErrorListPlot[%, Joined -> True,
  ErrorBarFunction -> errorBarFunc,
  PlotStyle -> Thin, PlotTheme -> "Scientific", GridLines -> {{isingTc}, None},
  FrameLabel -> {"Temperature", "Energy"},
  AspectRatio -> 0.8, ImageSize -> {Automatic, 600}]


magneticDipoleData = Map[{{#["temperature"], Mean[#["magneticDipole.Abs"]]},
    ErrorBar @ (Mean[#["magneticDipole.Square"]] - Mean[#["magneticDipole.Abs"]]^2)} &,
  Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];
magneticDipolePlot = ErrorListPlot[%, Joined -> True,
  ErrorBarFunction -> errorBarFunc,
  PlotStyle -> Thin, PlotTheme -> "Scientific", GridLines -> {{isingTc}, None},
  PlotLegends -> Map[(# <> "\[Times]" <> #) &, ToString /@ sizeList],
  FrameLabel -> {"Temperature", "Magnetic Dipole"},
  AspectRatio -> 0.8, ImageSize -> {Automatic, 600}]


$T  = Map[#["temperature"] &, Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];
$dT = Differences /@ $T;


$dE = Differences /@
  Map[Mean[#["energy"]] &, Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];
$dE$err = ListConvolve[{2, 2}, #, {-1, 1}, 0, #2^#1 &, Sqrt[#1 + #2] &] & /@
    Map[(Mean[#["energy.Square"]] - Mean[#["energy"]]^2) &, Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];


cvData = Map[{{#[[1]], #[[2]]}, ErrorBar @ #[[3]]} &,
(*cvData = Map[{#[[1]], #[[2]]} &,*)
  Transpose[{Most /@ $T, $dE/(2$dT), $dE$err/(2$dT)}, {3, 1, 2}],
  {2}];
magneticDipolePlot = ErrorListPlot[cvData, Joined -> True, (*Mesh\[Rule]All, PlotRange\[Rule]{-20,32},*)
  ErrorBarFunction -> errorBarFunc,
  PlotStyle -> Thin, PlotTheme -> "Scientific", GridLines -> {{isingTc}, None},
  PlotLegends -> Map[(# <> "\[Times]" <> #) &, ToString /@ sizeList],
  FrameLabel -> {"Temperature", "Magnetic Dipole"},
  AspectRatio -> 0.8, ImageSize -> {Automatic, 400}]
cvData[[5]];


#/.{{T_, e_}, ErrorBar[de_]}->{T,e,de} & /@ energyData[[2;;]];
Flatten/@(%\[Transpose])/.{T_, e1_,de1_,_, e2_,de2_,_, e3_,de3_,_, e4_,de4_}->{T,e1,de1,e2,de3,e3,de3,e4,de4};
Export["1.dat",%,"Table"]


NumberForm[{1.0002,2.},{3,4}]


#/.{{T_, e_}, ErrorBar[de_]}->{T,e,de} & /@ magneticDipoleData[[2;;]];
Flatten/@(%\[Transpose])/.{T_, e1_,de1_,_, e2_,de2_,_, e3_,de3_,_, e4_,de4_}->{T,e1,de1,e2,de2,e3,de3,e4,de4};
ScientificForm[#,4,NumberFormat->(SequenceForm[#1,"e",#3]&)]&/@%;
ToString/@%;
Export["2.dat",%,"Table"]


#/.{{T_, e_}, ErrorBar[de_]}->{T,e,de} & /@ cvData[[2;;]];
Flatten/@(%\[Transpose])/.{T_, e1_,de1_,_, e2_,de2_,_, e3_,de3_,_, e4_,de4_}->{T,e1,de1,e2,de2,e3,de3,e4,de4};
ScientificForm[#,4,NumberFormat->(SequenceForm[#1,"e",#3]&)]&/@%;
ToString/@%;
Export["3.dat",%,"Table"]
