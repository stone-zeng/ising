(* ::Package:: *)

Remove["Global`*"]


(*
  Arguments:
    rbm:
      <|
        "w": weight [n_visible * n_hidden],
        "b": bias of visible units [n_visible],
        "c": bias of hidden units  [n_hidden]
      |>
    v: [batch_size * n_visible]
    i: Integer
  Returned values:
    freeEnergy: [batch_size]
    pseudoLogLikelihood:
      "cost": Real
      "i":    Integer in [1, n_visible]
*)
freeEnergy[rbm_, v_] :=
  - v . rbm["b"] - Total /@ Log[1 + Exp[(rbm["c"] + #) & /@ (v . rbm["w"])]]
pseudoLogLikelihood[rbm_, v_] :=
  Module[{$n$visible = Length @ rbm["b"]},
    Module[{$index = RandomInteger[{1, $n$visible}]},
      (*
        Free energy of the i-th element of x    -> fe_xi
        Flip the i-th element in the 2nd layer  -> fe_xi_flip
        => cost = mean @ log @ sigmoid (fe_xi_flip - fe_xi)
        `batch_size` dimension will be eliminated via `Mean`
      *)
      $n$visible * Mean @ Log @ LogisticSigmoid @
        (
          freeEnergy[rbm, ReplacePart[#, $index -> 1 - #[[$index]]] & /@ v] -
          freeEnergy[rbm, v]
        )
    ]
  ]


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
  Module[{$h$props = propUp[rbm, v$sample]},
    Ramp @ RealSign[$h$props - RandomReal[{0, 1}, Dimensions @ $h$props]]]
sampleVisibleGivenHidden[rbm_, h$sample_] :=
  Module[{$v$props = propDown[rbm, h$sample]},
    Ramp @ RealSign[$v$props - RandomReal[{0, 1}, Dimensions @ $v$props]]]


(*
  Arguments:
    v: [batch_size * n_visible]
    k: Integer
  Returned value:
    ~ RBM
*)
contrastiveDivergence[rbm_, v_, k_] :=
  Module[
    {$samples = Nest[
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
    Module[{$h$props = propUp[rbm, v]},
      Module[
        {
          (*
            Both have the shape of [n_visible * n_hidden]
            `batch_size` dimension will be contracted here.
          *)
          $w$pos$grad = Transpose[v] . $h$props,
          $w$neg$grad = Transpose[$samples["v"]] . $samples["h"]
        },
        <|
          (*
            Shapes:
              "w": [n_visible * n_hidden]
              "b": [n_visible]
              "c": [n_hidden]
            For "b" and "c", `batch_size` dimension will be eliminated via `Mean`.
          *)
          "w" -> ($w$pos$grad - $w$neg$grad) / Length[v],
          "b" -> Mean[v - $samples["v"]],
          "c" -> Mean[$h$props - $samples["h"]]
        |>
      ]
    ]
  ]


(*
  Arguments:
    rbm$union:
      "variable": ~ rbm
      "velocity": ~ rbm
    v: [batch_size * n_visible]
    k: Integer
    m  = momentum: Real
    lr = learning rate: Real
  Returned value:
    ~ RBM
*)
learn[rbm$union_, v_, k_, m_, lr_] :=
  Module[{$rbm = rbm$union["variable"]},
    Module[
      {$velocity = m * rbm$union["velocity"] +
        lr * contrastiveDivergence[$rbm, v, k]},
      <|"variable" -> $rbm + $velocity, "velocity" -> $velocity|>
    ]
  ]


(*
  Arguments:
    v: [batch_size * n_visible]
    steps: Integer
  Returned value:
    [batch_size * n_visible]
*)
sampler[rbm_, v_, steps_] :=
  Nest[sampleVisibleGivenHidden[rbm, sampleHiddenGivenVisible[rbm, #]] &,
    v, steps]


(*
  Arguments:
    data:       [data_size * n_visible]
    batch$size: Integer
  Returned values:
    "data":  [batch_size * n_visible]
    "index": Integer
*)
dataInitialize[data_, batch$size_Integer] :=
  <|
    "data"  -> data[[;; batch$size]],
    "index" -> 1
  |>


(*
  Arguments:
    data:       [data_size * n_visible]
    batch$data:
      "data":  [batch_size * n_visible]
      "index": Integer
  Returned values:
    ~ batch$data
*)
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
    data:          [data_size * n_visible]
    rbm:           ~ RBM
    init$velocity: ~ RBM
*)
train[data_, rbm_, init$velocity_,
    epoches_, batch$size_, k_, m_, lr_] :=
  Nest[
    Module[
      {
        $rbm$union  = #["rbm_union"],
        (* Shape: [batch_size * n_visible] *)
        $batch$data = #["batch_data"]["data"],
        $iteration  = #["iteration"]
      },
      Module[{$cost = pseudoLogLikelihood[$rbm$union["variable"], $batch$data]},
        (* Monitor cost during evaluation *)
        If[Divisible[$iteration, batch$size],
          Echo["\tCost: " <> ToString[$cost, StandardForm],
            "Epoch " <> ToString @ Quotient[$iteration, batch$size]]
        ];
        (* The following is the function for `Nest` *)
        <|
          "rbm_union"  -> learn[$rbm$union, $batch$data, k, m, lr],
          "batch_data" -> nextBatch[data, #["batch_data"]],
          "cost_list"  -> Append[#["cost_list"], $cost],
          "iteration"  -> $iteration + 1
        |>
      ]
    ] &,
    (* Initial value *)
    <|
      "rbm_union"  -> <|"variable" -> rbm, "velocity" -> init$velocity|>,
      "batch_data" -> dataInitialize[data, batch$size],
      "cost_list"  -> {},
      "iteration"  -> 1
    |>,
    epoches * batch$size
  ]


(* ::Section:: *)
(*Training*)


dataPath = "E:\\Files\\Programs\\machine-learning\\data\\mnist\\";
{imageTrainRaw, imageTestRaw} =
  Import[dataPath <> #, "UnsignedInteger8", "HeaderBytes" -> 16] & /@
    {"train-images-idx3-ubyte", "t10k-images-idx3-ubyte"};
Dimensions /@ %


(* Binarize and reshape *)
{imageTrain, imageTest} = Round[1 - Partition[#, 784] / 255.0] & /@
  {imageTrainRaw, imageTestRaw};
Dimensions /@ %


(* Parameters *)
visibleNum   = 784;
hiddenNum    = 100;
epochNum     = 10;
batchSize    = 50;
kParameter   = 30;
momentum     = 0.5;
learningRate = 0.1;

(* Helper function *)
plotMNIST[data_] := ArrayPlot[ArrayReshape[data, {28, 28}],
  ImageSize -> 60, Frame -> False]

(* Initialize *)
rbm = AssociationThread[{"w", "b", "c"} ->
  Evaluate[RandomReal[{0, 1}, #] & /@
    {{visibleNum, hiddenNum}, visibleNum, hiddenNum}]];
rbmVelocity = AssociationThread[{"w", "b", "c"} ->
  Evaluate[ConstantArray[0.0, #] & /@
    {{visibleNum, hiddenNum}, visibleNum, hiddenNum}]];

(* Main training loop *)
trainingTime = First @ AbsoluteTiming[
  trained = train[imageTrain, rbm, rbmVelocity,
    epochNum, batchSize, kParameter, momentum, learningRate];];

(* Training time and cost *)
Echo[#, "Training time:"] & @ Quantity[trainingTime, "Seconds"];
Echo[#, "Memory used:"]   & @ Quantity[MemoryInUse[] / 2^20., "Megabytes"];
ListLogLogPlot[-trained["cost_list"],
  PlotRange -> All, Joined -> True,
  PlotTheme -> "Detailed", PlotLabel -> "Learning curve"]

(* Filters *)
trained$w = Transpose @ trained["rbm_union"]["variable"]["w"];
GraphicsGrid @ Partition[#, 10] & @ ParallelMap[plotMNIST, trained$w]

(* Generated samples *)
sample$num   = 20;
sample$step  = 100;
sample$index = RandomSample[Range @ Length @ imageTest, sample$num];
sample = NestList[
  <|
    "data"  -> sampler[trained["rbm_union"]["variable"], #["data"], sample$step],
    "index" -> #["index"] + 1
  |> &,
  <|
    "data"  -> imageTest[[sample$index]],
    "index" -> 0
  |>, 5];
Echo[Row[plotMNIST /@ #["data"]],
  "Sample steps: " <> ToString[#["index"] * sample$step]] & /@ sample;
