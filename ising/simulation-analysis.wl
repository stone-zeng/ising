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
magneticDipolePlot = ErrorListPlot[cvData[[4;;]], Joined -> True, (*Mesh\[Rule]All, PlotRange\[Rule]{-20,32},*)
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


(* ::InheritFromParent:: *)
(*{"{1., -3.99409, 0.000442386}","{1.1, -3.98759, 0.000942414}","{1.2, -3.97683, 0.00179465}","{1.3, -3.95851, 0.003348}","{1.4, -3.93337, 0.00524016}","{1.5, -3.89796, 0.008386}","{1.6, -3.85219, 0.0130378}","{1.7, -3.78851, 0.0182465}","{1.8, -3.71012, 0.0269484}","{1.82, -3.68883, 0.0280604}","{1.84, -3.67072, 0.0291573}","{1.86, -3.65472, 0.0325744}","{1.88, -3.63171, 0.0332458}","{1.9, -3.61223, 0.0354055}","{1.92, -3.5882, 0.0388962}","{1.94, -3.56642, 0.0405337}","{1.96, -3.52831, 0.0456609}","{1.98, -3.50606, 0.0468534}","{2., -3.47959, 0.050426}","{2.02, -3.45681, 0.0552325}","{2.04, -3.40126, 0.0649599}","{2.06, -3.38561, 0.0637925}","{2.08, -3.34154, 0.0681494}","{2.1, -3.29823, 0.0748548}","{2.12, -3.27919, 0.0769096}","{2.14, -3.23594, 0.0829343}","{2.16, -3.1747, 0.0926518}","{2.18, -3.11569, 0.114633}","{2.2, -3.06287, 0.107142}","{2.22, -3.00159, 0.118297}","{2.24, -2.98083, 0.125156}","{2.26, -2.93654, 0.121567}","{2.28, -2.83677, 0.133719}","{2.3, -2.80789, 0.139019}","{2.32, -2.70859, 0.13222}","{2.34, -2.63576, 0.129408}","{2.36, -2.63189, 0.134568}","{2.38, -2.56286, 0.134995}","{2.4, -2.51488, 0.124714}","{2.42, -2.4126, 0.123968}","{2.44, -2.39763, 0.114632}","{2.46, -2.36025, 0.113704}","{2.48, -2.32111, 0.106364}","{2.5, -2.26063, 0.10546}","{2.52, -2.24734, 0.0978693}","{2.54, -2.1927, 0.0958167}","{2.56, -2.16818, 0.0984973}","{2.58, -2.12746, 0.0920209}","{2.6, -2.1052, 0.0876498}","{2.62, -2.07249, 0.0835097}","{2.64, -2.02301, 0.0767228}","{2.66, -1.99283, 0.0720059}","{2.68, -1.96813, 0.0735332}","{2.7, -1.94668, 0.0765975}","{2.72, -1.92753, 0.0734833}","{2.74, -1.8914, 0.0710404}","{2.76, -1.87146, 0.069938}","{2.78, -1.86644, 0.0688017}","{2.8, -1.83338, 0.0658541}","{2.9, -1.74906, 0.0622454}","{3., -1.65178, 0.0589005}","{3.1, -1.5738, 0.0553899}","{3.2, -1.51183, 0.052516}","{3.3, -1.44969, 0.049944}","{3.4, -1.39159, 0.0490467}","{3.5, -1.34776, 0.0477177}","{3.6, -1.28684, 0.0474278}"}*)


#/.{{T_, e_}, ErrorBar[de_]}->{T,e,de} & /@ cvData[[2;;]];
Flatten/@(%\[Transpose])/.{T_, e1_,de1_,_, e2_,de2_,_, e3_,de3_,_, e4_,de4_}->{T,e3,de3,e4,de4};
ScientificForm[#,4,NumberFormat->(SequenceForm[#1,"e",#3]&)]&/@%;
ToString/@%;
Export["3.dat",%,"Table"]
