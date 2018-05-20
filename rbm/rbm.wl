(* ::Package:: *)

BeginPackage["RBM`"]


(*
  Arguments:
    rbm:
      <|
        "w": weight [n_visible * n_hidden],
        "b": bias of visible units [n_visible],
        "c": bias of hidden units  [n_hidden]
      |>
    v: [batch_size * n_visible]
    s = sample numbers: Integer
  Returned values:
    freeEnergy:          [batch_size]
    pseudoLogLikelihood: Real
*)
freeEnergy[rbm_, v_] :=
  - v . rbm["b"] - Total /@ Log[1 + Exp[(rbm["c"] + #) & /@ (v . rbm["w"])]]
pseudoLogLikelihood[rbm_, v_, s_] :=
  Module[{$n$visible = Length @ rbm["b"]},
    (*
      Free energy of the i-th element of x    -> fe_xi
      Flip the i-th element in the 2nd layer  -> fe_xi_flip
      => cost = mean @ log @ sigmoid (fe_xi_flip - fe_xi)
      `batch_size` dimension will be eliminated via `Mean`
    *)
    $n$visible / s * Mean @ Sum[
      Log @ LogisticSigmoid @
        (
          freeEnergy[rbm, ReplacePart[#, i -> 1 - #[[i]]] & /@ v] -
          freeEnergy[rbm, v]
        ),
    {i, RandomSample[Range[$n$visible], s]}]
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
      Module[{$v = sampleVisibleGivenHidden[rbm, #["h"]]},
        <|
          (*
            "h": hidden samples  [batch_size * n_hidden]
            "v": visible samples [batch_size * n_visible]
          *)
          "h" -> sampleHiddenGivenVisible[rbm, $v],
          "v" -> $v
        |>
      ] &,
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
      $batch$size = Length @ batch$data["data"],
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
    epoches_, batch$size_, k_, s_, m_, lr_] :=
  Module[{$batch$num = Quotient[Length @ data, batch$size]},
    Nest[
      Module[
        {
          $rbm$union  = #["rbm_union"],
          (* Shape: [batch_size * n_visible] *)
          $batch$data = #["batch_data"]["data"],
          $iteration  = #["iteration"]
        },
        Module[{$cost = pseudoLogLikelihood[
              $rbm$union["variable"], $batch$data, s]},
          (* Monitor cost during evaluation *)
          If[Divisible[$iteration, $batch$num],
            Echo["\tCost: " <> ToString[$cost, StandardForm],
              "Epoch " <> ToString @ Quotient[$iteration, $batch$num]]
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
      epoches * $batch$num
    ]
  ]


EndPackage[]
