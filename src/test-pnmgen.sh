#!/bin/sh

#
# Generate images using the generating function: 
# f(x,y) = x binop y
#
#for binop in "greyand" "rgband" "greyior" "rgbior" "greyxor" "rgbxor" \
#  "greyadd" "rgbadd" "greymul" "rgbmul" "dekchaos" \
#  "gradrg" "gradgb" "gradrb" "addsubxor" "muladdsub" "greydiv" "divrg" \
#  "poly" "eda"
#for binop in "poly"
#for binop in "eda"
for binop in "greymaxsq" "greyminsq" "greyavgsq" "greymax" "greymin" "greyavg" \
  "maxminavgsq" "maxminavg" "greyavgcb" "greyavgdiv"
do
  ./pnmgen.exe -xy${binop} 240 -x 256 -y 256 -par 11
#  ./pnmgen.exe -xy${binop} 255 -x 256 -y 256 -par 11
done

#for f in "rwalk"
#do
#  for steps in "000001000" "000010000" "000100000" "000200000" "000500000" "001000000" "002000000" "005000000" "010000000" "020000000" "050000000" "100000000" "1000000000"
#  do 
#    ./pnmgen.exe -${f} 255 -x 256 -y 256 -par ${steps}
#    ./pnmgen.exe -${f} 240 -x 256 -y 256 -par ${steps}
#  done
#done

echo "We are done."

exit 0
