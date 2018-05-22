(* ::Package:: *)

(*
  Arguments:
    v: [n_visible_x * n_visible_y]
    h: [n_hidden_x  * n_hidden_y]
    b: [n_visible_x * n_visible_y]
    w: [n_visible_x * n_visible_y * n_hidden_x * n_hidden_y]
  Returned values:
    $$vDotb: Real
    $$vDotw: [n_hidden_x  * n_hidden_y]
    $$hDotw: [n_visible_x * n_visible_y]
*)
$$vDotb[v_, b_] := Total @ Flatten[v * b]
$$vDotw[v_, w_] := Total @ Flatten[v * w, 1]
$$hDotw[h_, w_] := Total @ Flatten[h * Transpose[w, {3, 4, 1, 2}], 1]


(*
  Arguments:
    rbm:
      "w": weight [n_visible_x * n_visible_y * n_hidden_x * n_hidden_y],
      "b": bias of visible units [n_visible_x * n_visible_y],
      "c": bias of hidden units  [n_hidden_x  * n_hidden_y]
    v: [batch_size * n_visible_x * n_visible_y]
    s = sample numbers: Integer
  Returned values:
    freeEnergy:          [batch_size]
    pseudoLogLikelihood: Real
*)
freeEnergy[rbm_, v_] :=
  - $$vDotb[#, rbm["b"]] & /@ v -
    Total[Log[1 + Exp[(rbm["c"] + $$vDotw[#, rbm["w"]]) & /@ v]], {2, 3}]
pseudoLogLikelihood[rbm_, v_, s_] :=
  Module[{$n$visible = Dimensions @ rbm["b"]},
    (*
      Free energy of the i-th element of x    -> fe_xi
      Flip the i-th element in the 2nd layer  -> fe_xi_flip
      => cost = mean @ log @ sigmoid (fe_xi_flip - fe_xi)
      `batch_size` dimension will be eliminated via `Mean`
    *)
    Times @@ $n$visible / s * Mean @ Sum[
      Log @ LogisticSigmoid @
        (
          freeEnergy[rbm, $$getInverseVisibleUnit[#, i] & /@ v] -
          freeEnergy[rbm, v]
        ),
    {i, RandomSample[Tuples @ Range @ $n$visible, s]}]
  ]
(*
  Arguments:
    v$unit: [n_visible_x * n_visible_y]
    index:  [2] in [n_visible_x * n_visible_y]
*)
$$getInverseVisibleUnit[v$unit_, index_] :=
  ReplacePart[v$unit, i -> 1 - v$unit[[##]] & @@ index]


(*
  Arguments:
    rbm:
      "w": [n_visible * n_hidden]
      "b": [n_visible]
      "c": [n_hidden]
    v: [batch_size * n_visible]
    h: [batch_size * n_hidden]
  Intermediate variables:
    v.rbm["w"]:  [batch_size * n_hidden]
    h.rbm["w"]': [batch_size * n_visible]
  Returned values:
    propUp:   [batch_size * n_hidden]
    propDown: [batch_size * n_visible]
*)
propUp[rbm_, v_] :=
  LogisticSigmoid[(rbm["c"] + #) & /@ (v . rbm["w"])]
propDown[rbm_, h_] :=
  LogisticSigmoid[(rbm["b"] + #) & /@ (h . Transpose @ rbm["w"])]


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
        <|
          (*
            Shapes:
              "w": [n_visible * n_hidden]
              "b": [n_visible]
              "c": [n_hidden]
            For "b" and "c", `batch_size` dimension will be eliminated via `Mean`.
          *)
          "w" -> (w$pos$grad - w$neg$grad) / Length[v],
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
                  Transpose[$rbm["w"]][[$sample$index]]],
                "Weights\t"];
              Echo[Iconize[Transpose[$rbm["w"]][[$sample$index]]]];
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


imageTrain = Round[Partition[imageTrainRaw, 784] / 255.0];
imageTest  = Round[Partition[imageTestRaw,  784] / 255.0];
Dimensions /@ {imageTrain, imageTest}


(* Parameters *)
visibleNum   = 784;
hiddenNum    = 100;
epochNum     = 20;
batchSize    = 64;
kParameter   = 30;
learningRate = 0.1;

(* Helper function *)
plotMNIST[data_] := ArrayPlot[ArrayReshape[data, {28, 28}], ImageSize -> 60, Frame -> False]

(* Initialize features *)
rbm = AssociationThread[{"w", "b", "c"} ->
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
trainedW = Transpose @ trained["RBM_param"]["w"];
GraphicsGrid @ Partition[#, 10] & @ ParallelMap[plotMNIST, trainedW]

(* Generated samples *)
sample$num   = 20;
sample$step  = 100;
sample$index = RandomSample[Range @ Length @ imageTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["RBM_param"], #["data"], sample$step],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> imageTest[[sample$index]],
    "index" -> 0
  |>, 5];
Echo[Row[plotMNIST /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]]& /@ sample;
