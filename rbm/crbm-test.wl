(* ::Package:: *)

Remove["Global`*"]
SetDirectory[NotebookDirectory[]]


<< CRBM`


n$vx = 4;
n$vy = 5;
n$hx = 2;
n$hy = 3;
n$batch = 7;
n$data  = 24;


rbm =
  <|
    "w" -> RandomReal[{0, 1}, {n$vx, n$vy, n$hx, n$hy}],
    "b" -> RandomReal[{0, 1}, {n$vx, n$vy}],
    "c" -> RandomReal[{0, 1}, {n$hx, n$hy}]
  |>;
v = RandomReal[{0, 1}, {n$batch, n$vx, n$vy}];
h = RandomReal[{0, 1}, {n$batch, n$hx, n$hy}];
data = RandomReal[{0, 1}, {n$data, n$vx, n$vy}];


$$vDotb[#, rbm["b"]] & /@ v;
Echo[#, "$$vDotb: "] & @ Equal[Dimensions[%], {n$batch}];
$$vDotw[#, rbm["w"]] & /@ v;
Echo[#, "$$vDotw: "] & @ Equal[Dimensions[%], {n$batch, n$hx, n$hy}];
$$hDotw[#, rbm["w"]] & /@ h;
Echo[#, "$$hDotw: "] & @ Equal[Dimensions[%], {n$batch, n$vx, n$vy}];

freeEnergy[rbm, v];
Echo[#, "freeEnergy: "] & @ Equal[Dimensions[%], {n$batch}];

pseudoLogLikelihood[rbm, v, 2];
Echo[#, "pseudoLogLikelihood: "] & @ NumberQ[%];

propUp[rbm, v];
Echo[#, "propUp: "] & @ Equal[Dimensions[%], {n$batch, n$hx, n$hy}];
propDown[rbm, h];
Echo[#, "propDown: "] & @ Equal[Dimensions[%], {n$batch, n$vx, n$vy}];

contrastiveDivergence[rbm, v, 2];
Echo[#, "contrastiveDivergence: "] & @ (Equal @@ Map[Dimensions, {%, rbm}, {2}]);

rbmUnion = <|"variable" -> rbm, "velocity" -> rbm|>;
learn[rbmUnion, v, 10, 0.1, 0.1];
Echo[#, "learn: "] & @ (Equal @@ Map[Dimensions, {%, rbmUnion}, {2}]);

sampler[rbm, v, 10];
Echo[#, "sampler: "] & @ Equal[Dimensions[%], {n$batch, n$vx, n$vy}];

iniData = dataInitialize[data, n$batch];
Echo[#, "dataInitialize 1: "] & @ Equal[Dimensions[iniData["data"]], {n$batch, n$vx, n$vy}];
Echo[#, "dataInitialize 2: "] & @ NumberQ[iniData["index"]];

nextBatch[data, iniData];
Echo[#, "nextBatch 1: "] & @ (Equal @@ Map[Dimensions, {%, iniData}, {2}]);
Echo[#, "nextBatch 2: "] & @ Equal[%%["index"], iniData["index"] + 1];

RandomSample @ Tuples @ Range @ Dimensions @ rbm["c"];
Echo[#, "$$showWeightImages: "] & @ $$showWeightImages[rbm["w"], %];


trained = train[data, rbm, rbm,
  10, n$batch, 1, 1, 0, 0.1];
