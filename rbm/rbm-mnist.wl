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
  (* Binarize the input *)
  (* TODO: `255` is only for MNIST *)
  Module[{x = Round[v / 255], n$visible = Length @ rbm["b"]},
    <|
      (*
        Free energy of the i-th element of x   -> fe_xi
        Flip the i-th element in the 2nd layer -> fe_xi_flip
        "cost" = mean @ log @ sigmoid (fe_xi_flip - fe_xi)
        `batch_size` dimension will be eliminated via `Mean`
      *)
      "cost" -> Mean[n$visible * Log @ LogisticSigmoid[
        Subtract @@ (Function[$v, freeEnergy[rbm, $v]] /@
          {ReplacePart[#, i -> 1 - #[[i]]] & /@ x, x})]],
      "i" -> Mod[i, n$visible] + 1
    |>
  ]

(* Test *)
freeEnergy[rbm, mnistData[[;;100]]]
pseudoLikelihood[rbm, mnistData[[;;100]], 1]


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


(*TODO: not used!*)
(* Returned value shapes: n_visible *)
sampler[rbm_, v_, steps_] :=
  Nest[
    sampleVisibleGivenHidden[rbm, sampleHiddenGivenVisible[rbm, #]] &,
    v,
    steps]


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
    Nest[
      With[
        {
          $rbm        = First[#]["RBM_param"],
          (* Shape: [batch_size * n_visible] *)
          $batch$data = First[#]["batch_data"]["data"]
        },
        With[{$likelihood = pseudoLikelihood[$rbm, $batch$data, #[[2]]]},
          (* Monitor cost during evaluation *)
          If[Divisible[Last[#], batch$size],
            Echo["Epoch: " <> ToString @ Quotient[Last[#], batch$size] <> "\t" <>
                 "Cost: "  <> ToString @ $likelihood["cost"]]
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
      epoches * batch$size
    ]
trained = train[mnistData, rbm, 10, 10, 30, 0.1];


ArrayPlot[rbm["W"]\[Transpose]]
ArrayPlot[(trainedW = trained[[1]]["RBM_param"]["W"])\[Transpose]]
ArrayPlot[ArrayReshape[#,{28,28}],ImageSize->Tiny]&/@(trainedW\[Transpose])


(* ::Section:: *)
(*Training*)


dataPath = "E:\\Files\\Programs\\machine-learning\\data\\mnist\\";


imageTrainRaw=Import[dataPath<>"train-images-idx3-ubyte","UnsignedInteger8","HeaderBytes"->16];
imageTestRaw=Import[dataPath<>"t10k-images-idx3-ubyte","UnsignedInteger8","HeaderBytes"->16];
Length/@{imageTrainRaw,imageTestRaw}


imageTrain=Partition[imageTrainRaw,784];
imageTest=Partition[imageTestRaw,784];
Dimensions/@{imageTrain,imageTest}


rand[shape_] := RandomReal[{0, 1}, shape]


mnistData = imageTrain;
rbm  = <|"W" -> rand[{784, 100}], "b" -> rand[784], "c" -> rand[100]|>;


(*trained = train[mnistData, rbm, 6, 10, 3, 0.01];*)


(*trained[[1]]["cost_list"]//ListPlot*)


(*trained=learn[rbm, mnistData["batch_data"], 30, 0.1];*)
