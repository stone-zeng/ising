(* ::Package:: *)

Remove["Global`*"]


(*
  Arguments:
    n$visible: Integer
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
      "i":    Integer
*)
freeEnergy[rbm_, v_] :=
  - v . rbm["b"] - Total /@ Log[1 + Exp[(rbm["c"] + #) & /@ (v . rbm["W"])]]
pseudoLikelihood[n$visible_, rbm_, v_, i_] :=
  (* Binarize the input *)
  Module[{x = Round[v]},
    <|
      (*
        Free energy of the i-th element of x   -> fe_xi
        Flip the i-th element in the 2nd layer -> fe_xi_flip
        "cost" = mean @ log @ sigmoid (fe_xi_flip - fe_xi)
        `batch_size` dimension will be eliminated via `Mean`
      *)
      "cost" -> Mean[n$visible * Log @ LogisticSigmoid @ Subtract @@
          (Function[$v, freeEnergy[rbm, $v]] /@
            {ReplacePart[#, i -> 1 - #[[i]]] & @ x, x})],
      "i" -> Mod[i + 1, n$visible]
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
train[data_, rbm_,
    n$visible_Integer, epoches_Integer, batch$size_Integer, k_Integer, lr_Number] :=
  Nest[
    With[
      {
        $index      = First[#]["index"],
        $rbm        = First[#]["RBM_param"],
        (* Shape: [batch_size * n_visible] *)
        $batch$data = First[#]["batch_data"]["data"]
      },
      With[
        (*TODO*)
        {$likelihood = pseudoLikelihood[n$visible, $rbm, $batch$data, Last @ #]},
        (* Print cost during evaluation *)
        Print[$likelihood["cost"]];
        (* The following is the function for `Nest` *)
        {
          <|
            "index"      -> $index + 1,
            "RBM_param"  -> learn[$rbm, $batch$data, k, lr],
            "batch_data" -> nextBatch[data, First[#]["batch_data"]],
            "cost_list"  -> Join[First[#]["cost_list"], $likelihood["cost"]]
          |>,
          (*
            Note that the index for likelihood ($likelihood["i"], or the
            2nd term of `Nest`) is different from the batch index (`$index`)
          *)
          $likelihood["i"]
        }
      ]
    ] &,
    {
      (* Initial values *)
      <|
        "index"      -> 1,
        "RBM_param"  -> rbm,
        "batch_data" -> dataInitialize[data, batch$size],
        "cost_list"  -> {}
      |>,
      (* This is the index for likelihood *)
      1
    },
    epoches * batch$size
  ]


(* ::Section:: *)
(*Training*)


dataPath = "E:\\Files\\Programs\\machine-learning\\data\\mnist\\";


imageTrainRaw=Import[dataPath<>"train-images-idx3-ubyte","UnsignedInteger8","HeaderBytes"->16];
imageTestRaw=Import[dataPath<>"t10k-images-idx3-ubyte","UnsignedInteger8","HeaderBytes"->16];
Length/@{imageTrainRaw,imageTestRaw}


imageTrain=Partition[imageTrainRaw,784];
imageTest=Partition[imageTestRaw,784];
Dimensions/@{imageTrain,imageTest}


mnistData = dataInitialize[imageTrain[[;;100]], 16];


rand[shape_] := RandomReal[{0, 1}, shape]


(*trained = train[
  mnistData,
  <|"W" -> rand[{100, 784}], "b" -> rand[784], "c" -> rand[100]|>,
  784, 15, 30, 0.1]*)


data = mnistData;
rbm  = <|"W" -> rand[{100, 784}], "b" -> rand[784], "c" -> rand[100]|>;


trained=learn[rbm, data["batch_data"], 30, 0.1]


data["batch_data"]//Dimensions
