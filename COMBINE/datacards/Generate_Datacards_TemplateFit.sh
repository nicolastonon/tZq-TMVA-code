python Generate_Datacards.py uuu BDT ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py eeu BDT ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py uue BDT ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py eee BDT ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ

python Generate_Datacards.py uuu BDTttZ ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py eeu BDTttZ ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py uue BDTttZ ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py eee BDTttZ ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ

python Generate_Datacards.py uuu mTW ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py eeu mTW ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py uue mTW ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ
python Generate_Datacards.py eee mTW ../templates/Combine_Input_ScaledFakes.root allSyst tZq&ttZ




mkdir datacards_TemplateFit



python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py BDT_uuu=datacard_uuu_BDT.txt BDT_eeu=datacard_eeu_BDT.txt BDT_uue=datacard_uue_BDT.txt BDT_eee=datacard_eee_BDT.txt BDTttZ_uuu=datacard_uuu_BDTttZ.txt BDTttZ_eeu=datacard_eeu_BDTttZ.txt BDTttZ_uue=datacard_uue_BDTttZ.txt BDTttZ_eee=datacard_eee_BDTttZ.txt mTW_uuu=datacard_uuu_mTW.txt mTW_eeu=datacard_eeu_mTW.txt mTW_uue=datacard_uue_mTW.txt mTW_eee=datacard_eee_mTW.txt > COMBINED_datacard_TemplateFit.txt
mv datacard_*.txt datacards_TemplateFit/