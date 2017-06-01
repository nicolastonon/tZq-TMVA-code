#Produce plot showing impact of each Systematic
#argument must be name of datacard without extension (e.g. "COMBINED_datacard_TemplateFit_tZq")

echo "--- Usage : ./Create_Syst_Impact_Plot.sh [datacard_name_without_extension]"
echo "[-- NB : avoid including MC stat. error (too long) ]"

text2workspace.py -m 125 $1
combineTool.py -M Impacts -m 125 --doInitialFit -d $1
combineTool.py -M Impacts -m 125 --doFits --parallel 4 -d $1
combineTool.py -M Impacts -m 125 -o impacts.json -d $1
plotImpacts.py -i impacts.json -o impacts
