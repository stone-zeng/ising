(* ::Package:: *)

Remove["Global`*"]


(*
  Arguments:
    rbm:
      <|
        "W": weight [n_visible * n_hidden],
        "b": bias of visible units [n_visible],
        "c": bias of hidden units  [n_hidden]
      |>
    v: [batch_size * n_visible]
    i: Integer
  Returned values:
    freeEnergy: [batch_size]
    pseudoLikelihood:
      "cost": Real
      "i":    Integer in [1, n_visible]
*)
freeEnergy[rbm_, v_] :=
  - v . rbm["b"] - Total /@ Log[1 + Exp[(rbm["c"] + #) & /@ (v . rbm["W"])]]
pseudoLikelihood[rbm_, v_, i_] :=
  Module[{n$visible = Length @ rbm["b"]},
    <|
      (*
        Free energy of the i-th element of x   -> fe_xi
        Flip the i-th element in the 2nd layer -> fe_xi_flip
        "cost" = mean @ log @ sigmoid (fe_xi_flip - fe_xi)
        `batch_size` dimension will be eliminated via `Mean`
      *)
      "cost" -> Mean[n$visible * Log @ LogisticSigmoid[
        Subtract @@ (Function[$v, freeEnergy[rbm, $v]] /@
          {ReplacePart[#, i -> 1 - #[[i]]] & /@ v, v})]],
      "i" -> Mod[i, n$visible] + 1
    |>
  ]


(*
  Arguments:
    rbm:
      "W": [n_visible * n_hidden]
      "b": [n_visible]
      "c": [n_hidden]
    v: [batch_size * n_visible]
    h: [batch_size * n_hidden]
  Intermediate variables:
    v.rbm["W"]:  [batch_size * n_hidden]
    h.rbm["W"]': [batch_size * n_visible]
  Returned values:
    propUp:   [batch_size * n_hidden]
    propDown: [batch_size * n_visible]
*)
propUp[rbm_, v_] :=
  LogisticSigmoid[(rbm["c"] + #) & /@ (v . rbm["W"])]
propDown[rbm_, h_] :=
  LogisticSigmoid[(rbm["b"] + #) & /@ (h . Transpose @ rbm["W"])]


(*
  Returned values:
    sampleHiddenGivenVisible: [batch_size * n_hidden]
    sampleVisibleGivenHidden: [batch_size * n_visible]
*)
sampleHiddenGivenVisible[rbm_, v$sample_] :=
  Module[{h$props = propUp[rbm, v$sample]},
    Ramp @ Sign[h$props - RandomReal[{0, 1}, Dimensions @ h$props]]]
sampleVisibleGivenHidden[rbm_, h$sample_] :=
  Module[{v$props = propDown[rbm, h$sample]},
    Ramp @ Sign[v$props - RandomReal[{0, 1}, Dimensions @ v$props]]]


(*
  Arguments:
    v: [batch_size * n_visible]
    k: Integer
  Returned value:
    ~ rbm
*)
contrastiveDivergence[rbm_, v_, k_] :=
  Module[
    {samples = Nest[
      <|
        (*
          "h": hidden samples  [batch_size * n_hidden]
          "v": visible samples [batch_size * n_visible]
        *)
        "h" -> sampleHiddenGivenVisible[rbm, #["v"]],
        "v" -> sampleVisibleGivenHidden[rbm, #["h"]]
      |> &,
      <|"h" -> sampleHiddenGivenVisible[rbm, v], "v" -> v|>,
      k
    ]},
    (* Shape of `h`: [batch_size * n_hidden] *)
    Module[{h = propUp[rbm, v]},
      Module[
        {
          (*
            Both have the shape of [n_visible * n_hidden]
            `batch_size` dimension will be contracted here.
          *)
          w$pos$grad = Transpose[v] . h,
          w$neg$grad = Transpose[samples["v"]] . samples["h"]
        },
(*Echo[Dimensions@w$pos$grad];
Echo[ArrayPlot[ArrayReshape[w$pos$grad\[Transpose][[1]],{28,28}],ImageSize->50]];
Echo[ArrayPlot[ArrayReshape[w$neg$grad\[Transpose][[1]],{28,28}],ImageSize->50]];
Echo[samples["v"][[1]]];
Echo[samples["h"][[1]]];
Echo[ArrayReshape[w$pos$grad\[Transpose]\[LeftDoubleBracket]1\[RightDoubleBracket],{28,28}]//MatrixForm];
Echo[ArrayReshape[w$neg$grad\[Transpose]\[LeftDoubleBracket]1\[RightDoubleBracket],{28,28}]//MatrixForm];*)
        <|
          (*
            Shapes:
              "W": [n_visible * n_hidden]
              "b": [n_visible]
              "c": [n_hidden]
            For "b" and "c", `batch_size` dimension will be eliminated via `Mean`.
          *)
          "W" -> (w$pos$grad - w$neg$grad) / Length[v],
          "b" -> Mean[v - samples["v"]],
          "c" -> Mean[h - samples["h"]]
        |>
      ]
    ]
  ]


(* TODO: support momentum *)
(*
  Arguments:
    v: [batch_size * n_visible]
    k: Integer
    lr = learning rate: Real
  Returned value:
    ~ rbm
*)
learn[rbm_, v_, k_, lr_] :=
  rbm + lr * contrastiveDivergence[rbm, v, k]


(* Returned value shapes: n_visible *)
sampler[rbm_, v_, steps_] :=
  Nest[
    sampleVisibleGivenHidden[rbm, sampleHiddenGivenVisible[rbm, #]] &,
    v, steps]


(*
  Arguments:
    data:       [data_size * n_visible]
    batch$size: Integer
  Returned values:
    "batch_data": [batch_size * n_visible]
    "index":      Integer
*)
dataInitialize[data_, batch$size_Integer] :=
  <|
    "data"  -> data[[;; batch$size]],
    "index" -> 1
  |>


nextBatch[data_, batch$data_Association] :=
  Module[
    {
      $data$size  = Length @ data,
      $batch$size = Length @ batch$data["batch_data"],
      $index      = batch$data["index"]
    },
    <|
      "data"  -> data[[$index * $batch$size + 1 ;; ($index + 1) * $batch$size]],
      "index" -> Mod[$index + 1, Quotient[$data$size, $batch$size]]
    |>
  ]


(*
  Arguments:
    data: [data_size * n_visible]
    rbm: [RBM]
*)
train[data_, rbm_, epoches_, batch$size_, k_, lr_] :=
  Module[
    {
      $batch$num    = Quotient[Length @ data, batch$size],
      $sample$index = RandomSample[Range @ Length @ rbm["c"], 10]
    },
    Nest[
      Module[
        {
          $rbm        = First[#]["RBM_param"],
          (* Shape: [batch_size * n_visible] *)
          $batch$data = First[#]["batch_data"]["data"],
        },
        Module[{$likelihood = pseudoLikelihood[$rbm, $batch$data, #[[2]]]},
          (* Monitor cost during evaluation *)
          If[Divisible[Last[#], $batch$num],
            Echo["\tCost: " <> ToString[$likelihood["cost"], StandardForm],
              "Epoch " <> ToString @ Quotient[Last[#], $batch$num]];
              Echo[GraphicsRow @ Map[
                  Function[mat, ArrayPlot[ArrayReshape[mat, Sqrt[Length /@ {mat, mat}]],
                    ImageSize -> 50, Frame -> False]],
                  Transpose[$rbm["W"]][[$sample$index]]],
                "Weights\t"];
              Echo[Iconize[Transpose[$rbm["W"]][[$sample$index]]]];
          ];
          (* The following is the function for `Nest` *)
          {
            <|
              "RBM_param"  -> learn[$rbm, $batch$data, k, lr],
              "batch_data" -> nextBatch[data, First[#]["batch_data"]],
              "cost_list"  -> Append[First[#]["cost_list"], $likelihood["cost"]]
            |>,
            $likelihood["i"],
            Last[#] + 1
          }
        ]
      ] &,
      {
        (* Initial values *)
        <|
          "RBM_param"  -> rbm,
          "batch_data" -> dataInitialize[data, batch$size],
          "cost_list"  -> {}
        |>,
        (* This is the index for likelihood *)
        1,
        (* This is the index for `Nest` *)
        1
      },
      epoches * $batch$num
    ]
  ]


(* ::Section:: *)
(*Training*)


dataPath = "E:\\Files\\Programs\\machine-learning\\data\\mnist\\";


importMNIST[name_] :=
  Import[dataPath <> name, "UnsignedInteger8", "HeaderBytes" -> 16]
imageTrainRaw = importMNIST["train-images-idx3-ubyte"];
imageTestRaw  = importMNIST["t10k-images-idx3-ubyte"];
Dimensions /@ {imageTrainRaw,  imageTestRaw}


imageTrain = Round[Partition[imageTrainRaw, 784] / 255.0][[;;10000]];
imageTest  = Round[Partition[imageTestRaw,  784] / 255.0];
Dimensions /@ {imageTrain, imageTest}


(* Parameters *)
visibleNum   = 784;
hiddenNum    = 400;
epochNum     = 10;
batchSize    = 64;
kParameter   = 30;
learningRate = 0.1;

(* Helper function *)
plotSample[data_, size_] := ArrayPlot[ArrayReshape[data, {size, size}], ImageSize -> 60, Frame -> False]
plotVisible[data_] := plotSample[data, Round @ Sqrt @ visibleNum];
plotHidden[data_]  := plotSample[data, Round @ Sqrt @ hiddenNum];

(* Initialize features *)
rbm = AssociationThread[{"W", "b", "c"} ->
  Evaluate[RandomReal[{0, 1}, #] & /@
    {{visibleNum, hiddenNum}, visibleNum, hiddenNum}]];

(* Main training loop *)
trainingTime = First @ AbsoluteTiming[
  trained = First @ train[imageTrain, rbm, epochNum, batchSize, kParameter, learningRate];];

(* Training time and cost *)
Echo[#, "Training time:"] & @ Quantity[trainingTime, "Seconds"];
ListLogLogPlot[-trained["cost_list"],
  PlotRange -> All, Joined -> True, PlotTheme -> "Detailed", PlotLabel -> "Learning curve"]

(* Filters *)
trainedW = Transpose @ trained["RBM_param"]["W"];
GraphicsGrid @ Partition[#, 10] & @ ParallelMap[plotVisible, trainedW]

(* Generated samples *)
sample$num   = 20;
sample$step  = 100;
sample$index = RandomSample[Range @ Length @ imageTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["RBM_param"], #["data"], sample$step],
    "h"     -> sampleHiddenGivenVisible[trained["RBM_param"], #["data"]],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> imageTest[[sample$index]],
    "h"     -> {},
    "index" -> 0
  |>, 5];
Echo[Row[plotVisible /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ sample;
Echo[Row[plotHidden /@ #["h"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ Rest @ sample;


(* ::Section:: *)
(*Training Ising*)


SetDirectory[StringRiffle[StringSplit[NotebookDirectory[], "\\"][[;;-2]], "\\"] <> "\\ising\\data\\lattice-data\\2"]


dataTrainRaw = ToCharacterCode /@ Import["lattice-data-train-t4.dat", "Lines"] - 48;
dataTestRaw  = ToCharacterCode /@ Import["lattice-data-test-t4.dat",  "Lines"] - 48;


partSize = 28;
dataTrain = Flatten[#[[;;partSize, ;;partSize]]] & /@ (ArrayReshape[#, {64, 64}] & /@ dataTrainRaw);
dataTest  = Flatten[#[[;;partSize, ;;partSize]]] & /@ (ArrayReshape[#, {64, 64}] & /@ dataTestRaw);
Echo[#, "Train data dimension:"] & @ Dimensions[dataTrain];
Echo[#, "Test data dimension:"] & @ Dimensions[dataTest];
Echo[Quantity[N @ #, "Megabytes"], "Train data size:"] & @
  (ByteCount[dataTrain] / 2^20);
Echo[Quantity[N @ #, "Megabytes"], "Test data size:"] & @
  (ByteCount[dataTest] / 2^20);


(* Another dataset *)
partSize = 28;
rawData = Import["E:\\Files\\Programs\\ising\\ising\\data\\lattice-data\\2\\result-2018-05-25.json","RawJSON"][[2]];


dataTrain = (Flatten[rawData["latticeData"], {{1}, {2, 3}}][[;;-1001]] + 1) / 2;
dataTest  = (Flatten[rawData["latticeData"], {{1}, {2, 3}}][[-1000;;]] + 1) / 2;
Echo[#, "Train data dimension:"] & @ Dimensions[dataTrain];
Echo[#, "Test data dimension:"]  & @ Dimensions[dataTest];
Echo[Quantity[N @ #, "Megabytes"], "Train data size:"] & @
  (ByteCount[dataTrain] / 2^20);
Echo[Quantity[N @ #, "Megabytes"], "Test data size:"] & @
  (ByteCount[dataTest] / 2^20);


(* Parameters *)
dataNum      = All;
visibleNum   = partSize^2;
hiddenNum    = 100;
epochNum     = 50;
batchSize    = 64;
kParameter   = 30;
learningRate = 0.1;

(* Helper function *)
plotSample[data_, size_] := ArrayPlot[ArrayReshape[data, {size, size}], ImageSize -> 60, Frame -> False]
plotVisible[data_] := plotSample[data, Round @ Sqrt @ visibleNum];
plotHidden[data_]  := plotSample[data, Round @ Sqrt @ hiddenNum];

(* Initialize features *)
rbm = AssociationThread[{"W", "b", "c"} ->
  Evaluate[RandomReal[{0, 1}, #] & /@
    {{visibleNum, hiddenNum}, visibleNum, hiddenNum}]];

(* Main training loop *)
trainingTime = First @ AbsoluteTiming[
  trained = First @ train[dataTrain[[;;dataNum]], rbm, epochNum, batchSize, kParameter, learningRate];];

(* Training time and cost *)
Echo[#, "Training time:"] & @ Quantity[trainingTime, "Seconds"];
ListLogLogPlot[-trained["cost_list"],
  PlotRange -> All, Joined -> True, PlotTheme -> "Detailed", PlotLabel -> "Learning curve"]

(* Filters *)
trainedW = Transpose @ trained["RBM_param"]["W"];
GraphicsGrid @ Partition[#, 10] & @ ParallelMap[plotVisible, trainedW]

(* Generated samples *)
sample$num   = 20;
sample$step  = 100;
sample$index = RandomSample[Range @ Length @ dataTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["RBM_param"], #["data"], sample$step],
    "h"     -> sampleHiddenGivenVisible[trained["RBM_param"], #["data"]],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> dataTest[[sample$index]],
    "h"     -> {},
    "index" -> 0
  |>, 5];
Echo[Row[plotVisible /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ sample;
Echo[Row[plotHidden /@ #["h"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ Rest @ sample;


(* Generated samples *)
sample$num   = 20;
sample$step  = 100;
sample$index = RandomSample[Range @ Length @ dataTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["RBM_param"], #["data"], sample$step],
    "h"     -> sampleHiddenGivenVisible[trained["RBM_param"], #["data"]],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> dataTest[[sample$index]],
    "h"     -> {},
    "index" -> 0
  |>, 5];
Echo[Row[plotVisible /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ sample;
Echo[Row[plotHidden /@ #["h"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ Rest @ sample;


GraphicsGrid @ Partition[#, 10] & @ ParallelMap[plotVisible, trainedW]
