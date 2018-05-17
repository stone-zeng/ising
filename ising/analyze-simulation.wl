(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]]


isingExe          = "x64\\Release\\ising.exe";
isingSettingsFile = "simulation-settings.json";
isingResultFile   = "simulation-result.json";


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
    "repetitions" -> 10
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
DeleteFile[{isingSettingsFile, isingResultFile}];


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


Map[{{#["temperature"], Mean[#["energy"]]},
    ErrorBar @ (Mean[#["energy.Square"]] - Mean[#["energy"]]^2)} &,
  Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];
energyPlot = ErrorListPlot[%, Joined -> True,
  ErrorBarFunction -> errorBarFunc,
  FrameTicksStyle -> 12, LabelStyle -> Directive@{Automatic,12},
  PlotStyle -> Thickness[0.002], PlotTheme -> "Scientific", GridLines -> {{isingTc}, None},
  FrameLabel -> {"Temperature", "Energy"},
  AspectRatio -> 0.8, ImageSize -> {Automatic, 200}]


Map[{{#["temperature"], Mean[#["magneticDipole.Abs"]]},
    ErrorBar @ (Mean[#["magneticDipole.Square"]] - Mean[#["magneticDipole.Abs"]]^2)} &,
  Function[size, Select[data, (#["size"] == size) &]] /@ sizeList, {2}];
magneticDipolePlot = ErrorListPlot[%, Joined -> True,
  ErrorBarFunction -> errorBarFunc,
  PlotStyle -> Thin, PlotTheme -> "Scientific", GridLines -> {{isingTc}, None},
  PlotLegends -> Map[(# <> "\[Times]" <> #) &, ToString /@ sizeList],
  FrameLabel -> {"Temperature", "Magnetic Dipole"},
  AspectRatio -> 0.8, ImageSize -> {Automatic, 200}]


Row[{energyPlot, magneticDipolePlot}, Spacer[15]]
Export["ising-simulation.pdf", %]
