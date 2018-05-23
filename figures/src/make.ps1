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

$TEMP    = "@plottemp"
$TEXTEMP = $TEMP + ".tex"
$TEXFMT  = "@plotformat"

function MakeFigure {
  Param ([string]$filename, [int]$flag)
  # flag:
  #   0: xelatex
  #   1: pdflatex (dumped)
  #   2: pdftex (make dump)
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
    & "pdftex.exe" "-ini" ("-jobname=" + $TEXFMT) "&pdflatex" "mylatexformat.ltx" $TEXTEMP
  }
}

# The "main" function
if ($args[0] -eq "-a") {
  $FILELIST =
    "ising-energy",
    "ising-magnet",
    "ising-cv",
    "ising-cv-exact",
    "ising-cv-fit-i",
    "ising-cv-fit-ii",
    "boltzmann-machine",
    "gibbs-sampling",
    "learning-curve",
    "rbm"
  foreach ($item in $FILELIST) {
    MakeFigure -filename $item -flag 0
    Move-Item -Force ($item + ".pdf") ".."
  }
}
elseif ($args[0] -clike "-c") {
  foreach ($item in "*.aux", "*.log", ($TEMP + ".*")) {
    Remove-Item $item
  }
}
elseif ($args[0] -clike "-C") {
  foreach ($item in "*.aux", "*.log", "*.fmt", "*.pdf", ($TEMP + ".*")) {
    Remove-Item $item
  }
}
elseif ($args[0] -eq "-f") {
  MakeFigure -flag 2
}
else {
  if (Test-Path $args[0]) {
    $file = $args[0] -replace "\.\\",  ""
    $file = $file    -replace "\.tex", ""
    if ($args.Length -ge 2) {
      if ($args[1] -clike "-d") { MakeFigure -filename $file -flag 1 }
    }
    else {
      MakeFigure -filename $file -flag 0
      Move-Item -Force ($file + ".pdf") ".."
    }
  }
  else { Write-Output ("`"" + $args[0] + "`" does not exist!") }
}
