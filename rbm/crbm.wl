(* ::Package:: *)

BeginPackage["CRBM`"]


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
      "w": weight [n_visible_x * n_visible_y * n_hidden_x * n_hidden_y],
      "b": bias of visible units [n_visible_x * n_visible_y],
      "c": bias of hidden units  [n_hidden_x  * n_hidden_y]
    v: [batch_size * n_visible_x * n_visible_y]
    h: [batch_size * n_hidden_x  * n_hidden_y]
  Returned values:
    propUp:   [batch_size * n_hidden_x  * n_hidden_y]
    propDown: [batch_size * n_visible_x * n_visible_y]
*)
propUp[rbm_, v_] :=
  Round @ LogisticSigmoid[(rbm["c"] + $$vDotw[#, rbm["w"]]) & /@ v]
propDown[rbm_, h_] :=
  Round @ LogisticSigmoid[(rbm["b"] + $$hDotw[#, rbm["w"]]) & /@ h]


(*
  !! We use `propUp` and `propDown` instead.
  Returned values:
    sampleHiddenGivenVisible: [batch_size * n_hidden]
    sampleVisibleGivenHidden: [batch_size * n_visible]
*)
(*
sampleHiddenGivenVisible[rbm_, v$sample_] :=
  Module[{$h$props = propUp[rbm, v$sample]},
    Ramp @ RealSign[$h$props - RandomReal[{0, 1}, Dimensions @ $h$props]]]
sampleVisibleGivenHidden[rbm_, h$sample_] :=
  Module[{$v$props = propDown[rbm, h$sample]},
    Ramp @ RealSign[$v$props - RandomReal[{0, 1}, Dimensions @ $v$props]]]
$sampleHiddenGivenVisible[rbm_, v$sample_] :=
  Module[{$h$props = propUp[rbm, v$sample]},
    ParallelMap[RandomVariate[BinomialDistribution[1, #]] &, $h$props, {2}, Method \[Rule] "CoarsestGrained"]]
$sampleVisibleGivenHidden[rbm_, h$sample_] :=
  Module[{$v$props = propDown[rbm, h$sample]},
    ParallelMap[RandomVariate[BinomialDistribution[1, #]] &, $v$props, {2}, Method \[Rule] "CoarsestGrained"]]
*)


(*
  Arguments:
    v: [batch_size * n_visible_x * n_visible_y]
    k: Integer
  Returned value:
    ~ RBM
*)
contrastiveDivergence[rbm_, v_, k_] :=
  Module[
    {$samples = Nest[
      Module[{$v = propDown[rbm, #["h"]]},
        <|
          (*
            "h": hidden samples  [batch_size * n_hidden_x  * n_hidden_y]
            "v": visible samples [batch_size * n_visible_x * n_visible_y]
          *)
          "h" -> propUp[rbm, $v],
          "v" -> $v
        |>
      ] &,
      <|"h" -> propUp[rbm, v], "v" -> v|>,
      k
    ]},
    (* Shape of `h`: [batch_size * n_hidden_x  * n_hidden_y] *)
    Module[{$h$props = propUp[rbm, v]},
      Module[
        {
          (*
            Both have the shape of [n_visible_x * n_visible_y * n_hidden_x * n_hidden_y],
            `batch_size` dimension will be contracted here.
          *)
          $w$pos$grad = Transpose[v, {3, 1, 2}] . $h$props,
          $w$neg$grad = Transpose[$samples["v"], {3, 1, 2}] . $samples["h"]
        },
        <|
          (*
            Shapes:
              "w": weight [n_visible_x * n_visible_y * n_hidden_x * n_hidden_y],
              "b": bias of visible units [n_visible_x * n_visible_y],
              "c": bias of hidden units  [n_hidden_x  * n_hidden_y]
            For "b" and "c", `batch_size` dimension will be eliminated via `Mean`.
          *)
          "w" -> ($w$pos$grad - $w$neg$grad) / Times @@ Dimensions[v],
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
    v: [batch_size * n_visible_x * n_visible_y]
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
    v: [batch_size * n_visible_x * n_visible_y]
    steps: Integer
  Returned value:
    [batch_size * n_visible_x * n_visible_y]
*)
sampler[rbm_, v_, steps_] :=
  Nest[propDown[rbm, propUp[rbm, #]] &,
    v, steps]


(*
  Arguments:
    data:       [data_size * n_visible_x * n_visible_y]
    batch$size: Integer
  Returned values:
    "data":  [batch_size * n_visible_x * n_visible_y]
    "index": Integer
*)
dataInitialize[data_, batch$size_Integer] :=
  <|
    "data"  -> data[[;; batch$size]],
    "index" -> 1
  |>


(*
  Arguments:
    data:       [data_size * n_visible_x * n_visible_y]
    batch$data:
      "data":  [batch_size * n_visible_x * n_visible_y]
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
      "data"  -> data[[$index * $batch$size + 1 ;; Min[($index + 1) * $batch$size, $data$size]]],
      "index" -> Mod[$index + 1, Quotient[$data$size, $batch$size]]
    |>
  ]


$$showWeightImages[w_, indices_] :=
  GraphicsRow @ Map[
    MatrixPlot[#, ImageSize -> 50, Frame -> False] &,
    Function[i, Transpose[w, {3, 4, 1, 2}][[##]] & @@ i] /@ indices]


(*
  Arguments:
    data:          [data_size * n_visible_x * n_visible_y]
    rbm:           ~ RBM
    init$velocity: ~ RBM
*)
train[data_, rbm_, init$velocity_,
    epoches_, batch$size_, k_, s_, m_, lr_] :=
  Module[
    {
      $batch$num    = Quotient[Length @ data, batch$size],
      (* `10` is the number of monitoring images. *)
      $sample$index = RandomSample[Tuples @ Range @ Dimensions @ rbm["c"], 6]
    },
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
              "Epoch " <> ToString @ Quotient[$iteration, $batch$num]];
            Echo[$$showWeightImages[#["rbm_union"]["variable"]["w"], $sample$index],
              "Weights\t"];
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
