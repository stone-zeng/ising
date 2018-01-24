(* ::Package:: *)

(* ::Section:: *)
(*Simulation*)


initialLattice[size_Integer]:=RandomChoice[{-1,1},{size,size}]


deltaEnergy[lattice_,pos$i_,pos$j_,B_]:=2lattice[[pos$i,pos$j]](ListConvolve[({
 {0, 1, 0},
 {1, 0, 1},
 {0, 1, 0}
}),lattice[[pos$i-1;;pos$i+1,pos$j-1;;pos$j+1]]][[1,1]]+B)


flipProbability[dE_,\[Beta]_]:=Min[1,E^(-\[Beta] dE)]


isFlip[lattice_,pos$i_,pos$j_,\[Beta]_,B_]:=
  1-2UnitStep[flipProbability[deltaEnergy[lattice,pos$i,pos$j,B],\[Beta]]-RandomReal[]]


flipOnce[lattice_,\[Beta]_,B_]:=
  Module[{lattice$pad=ArrayPad[lattice,1],size=Length@lattice},
  Nest[{MapAt[Function[point,point*isFlip[#[[1]],#[[2,1]],#[[2,2]],\[Beta],B]],#[[1]],#[[2]]],
      {#[[2,1]]+Quotient[#[[2,2]]-1,size],Mod[#[[2,2]]-1,size]+2}}&,
    {lattice$pad,{2,2}},size^2-1][[1,2;;-2,2;;-2]]]


(* TEST *)
MatrixPlot[#,ImageSize->Tiny]&/@NestList[flipOnce[#,2,0]&,initialLattice[25],20]


(* ::Section:: *)
(*Physical quantities*)


calcM[size_Integer,\[Beta]_,B_,step_Integer,eval$step_Integer]:=
  1/(size^2*eval$step) Total[Part[Abs@Total[#,2]&/@
    NestList[flipOnce[#,\[Beta],B]&,initialLattice[size],step],;;eval$step]]


(* ::Section:: *)
(*Results*)


(* size=10, step=100, eval_step=10, batch=8 *)
ListLinePlot[result$10=ParallelTable[{T,Mean@Table[calcM[10,1/T,0,100,10],{i,8}]},{T,0.02,4,0.02}]]


Length[result$10]
ListLinePlot[{result$10\[Transpose][[1]],GaussianFilter[result$10\[Transpose][[2]],5]}\[Transpose]]


(* size=20, step=100, eval_step=10, batch=8 *)
ListLinePlot[result$20=ParallelTable[{T,Mean@Table[calcM[20,1/T,0,100,10],{i,8}]},{T,0.02,4,0.02}]]


ListLinePlot[]
