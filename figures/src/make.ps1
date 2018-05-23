$PREAMBLE_COMMON = `
"\documentclass{standalone}
\usepackage{kvsetkeys,epstopdf-base,pgfplots,xcolor,xcolor-material,siunitx}
\usetikzlibrary{arrows,calc,graphs,arrows.meta,pgfplots.colorbrewer}`n"

$PREAMBLE_FONTS = `
"\usepackage[no-math]{fontspec}
\usepackage{unicode-math}
\setmainfont{XITS}
\setmathfont{XITS Math}[math-style=ISO, bold-style=ISO, StylisticSet=1]`n"

$PREAMBLE = $PREAMBLE_COMMON + $PREAMBLE_FONTS + `
"\def\Tc{T_{\symrm{c}}}
\def\bm#1{{\symbf{#1}}}`n"

$PREAMBLE_DUMPED = $PREAMBLE_COMMON + "\endofdump`n" + `
"\def\Tc{T_c}
\def\bm#1{#1}`n"

$TEXTEMP = "plot.tex"
$TEXFMT  = "plot-fmp"

function MakeFigure {
  Param ([string]$filename, [int]$flag)
  # flag:
  #   0: xelatex
  #   1: pdflatex (dumped)  -d
  #   2: pdftex (make dump) -D
  $texbody = "\begin{document}`n  \input{" + $filename + "}`n\end{document}"
  if ($flag -eq 0) {
    Out-File -FilePath $TEXTEMP -Encoding ascii -InputObject $PREAMBLE
    Out-File -FilePath $TEXTEMP -Encoding ascii -Append -InputObject $texbody
    & "xelatex.exe" ("-jobname=" + $filename) $TEXTEMP
  }
  elseif ($flag -eq 1) {
    Out-File -FilePath $TEXTEMP -Encoding ascii -InputObject $PREAMBLE_DUMPED
    Out-File -FilePath $TEXTEMP -Encoding ascii -Append -InputObject $texbody
    & "pdflatex.exe" ("&" + $TEXFMT) $TEXTEMP
  }
  elseif ($flag -eq 2) {
    Out-File -FilePath $TEXTEMP -Encoding ascii -InputObject $PREAMBLE_DUMPED
    Out-File -FilePath $TEXTEMP -Encoding ascii -Append -InputObject $texbody
    & "pdftex.exe" "-ini" ("-jobname=" + $TEXFMT) "&pdflatex" "mylatexformat.ltx" $TEXTEMP
  }
}

if ($args[0] -eq "-a") {
  $FILELIST =
    "boltzmann-machine",
    "gibbs-sampling",
    #"ising-cv-fit-i",
    #"ising-cv-fit-ii",
    #"ising-cv",
    #"ising-energy",
    #"ising-magnet",
    #"learning-curve",
    "rbm"
  foreach ($item in $FILELIST) {
    MakeFigure -filename $item -flag 0
  }
}
elseif ($args[0] -eq "-c") {
  foreach ($item in "*.aux", "*.log", "*.fmt", "*.pdf", "plot.tex") {
    Remove-Item $item
  }
}
elseif ($args[0] -eq "-f") {
  MakeFigure -filename $FILE -flag 2
}
else {
  if (Test-Path $args[0]) {
    $FILE = $args[0] -replace "\.\\", ""
    if ($args.Length -ge 2) {
      if ($args[1] -clike "-d") { MakeFigure -filename $FILE -flag 1 }
    }
    else { MakeFigure -filename $FILE -flag 0 }
  }
  else { Write-Output ("`"" + $args[0] + "`" does not exist!") }
}
