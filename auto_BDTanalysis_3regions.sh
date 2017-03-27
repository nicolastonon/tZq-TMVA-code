make -j5

#Train BDT & Create templates in 3 regions

./BDT_analysis.exe WZ #Need to run in CR WZ first, because needed for Fakes rescaling !

./BDT_analysis.exe tZq

./BDT_analysis.exe ttZ
