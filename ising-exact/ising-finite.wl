(* ::Package:: *)

Remove["Global`*"]


SetDirectory[NotebookDirectory[]];


gamma[q_Integer /; q > 0,  n_Integer, k_] := ArcCosh[Cosh[2 k] Coth[2 k] - Cos[Pi * q / n]]
gamma[q_Integer /; q == 0, n_Integer, k_] := Log[Exp[2 k] Tanh[k]]


Y1[n_Integer, m_Integer, k_] := Product[2 Cosh[m * gamma[2 q + 1, n, k] / 2], {q, 0, n - 1}]
Y2[n_Integer, m_Integer, k_] := Product[2 Sinh[m * gamma[2 q + 1, n, k] / 2], {q, 0, n - 1}]
Y3[n_Integer, m_Integer, k_] := Product[2 Cosh[m * gamma[2 q,     n, k] / 2], {q, 0, n - 1}]
Y4[n_Integer, m_Integer, k_] := Product[2 Sinh[m * gamma[2 q,     n, k] / 2], {q, 0, n - 1}]


PartitionFunctionQ[n_Integer, m_Integer, k_] :=
  (2 Sinh[2 k])^(n * m / 2) / 2 * Total[(#1 @@ {n, m, k} & ) /@ {Y1, Y2, Y3, Y4}]


EnergyE[size_Integer, T_] := T^2 * D[Log @ PartitionFunctionQ[size, size, 1 / $T], $T] /. $T -> T
SpecificHeatC[size_Integer, T_] :=
  T * D[$T * Log @ PartitionFunctionQ[size, size, 1 / $T], {$T, 2}] /. $T -> T


plotCapacity[size_Integer, range_] :=
  ParallelTable[{T, SpecificHeatC[size, T] / size^2}, {T, range}]


plotRange = DeleteDuplicates @ Join[Range[1, 2.1, 0.1], Range[2.1, 2.5, 0.02], Range[2.5, 4, 0.1]];
sizeList  = {2, 4, 8, 16, 32, 64};
AbsoluteTiming[plotData = plotCapacity[#, plotRange] & /@ sizeList;]


isingCriticalT = 2 / Log[1 + Sqrt[2]];
criticalLine = Line @ {{isingCriticalT, 0}, {isingCriticalT, 20}};


ListLinePlot[plotData, PlotRange -> {0, 2.5}, InterpolationOrder -> 3,
  Epilog -> {Gray, criticalLine},
  PlotTheme -> "Scientific", PlotStyle -> Thin,
  FrameLabel -> {Style["k T / J", Italic], Style["C / N k", Italic]},
  PlotLegends -> (ToString[#] <> "\[Times]" <> ToString[#] & /@ sizeList),
  ImageSize -> 500, AspectRatio -> 1.3]
(*Export["ising2d-specific-heat.pdf", %];*)
