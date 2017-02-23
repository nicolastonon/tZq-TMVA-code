python generateDatacards.py uuu mTW ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py uue mTW ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py eeu mTW ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py eee mTW ../templates/Combine_Input_ScaledFakes.root 

python generateDatacards.py uuu BDT ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py uue BDT ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py eeu BDT ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py eee BDT ../templates/Combine_Input_ScaledFakes.root 

python generateDatacards.py uuu BDTttZ ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py uue BDTttZ ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py eeu BDTttZ ../templates/Combine_Input_ScaledFakes.root 
python generateDatacards.py eee BDTttZ ../templates/Combine_Input_ScaledFakes.root 


python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py BDT_eee=datacard_eee_BDT.txt BDT_uuu=datacard_uuu_BDT.txt BDT_eeu=datacard_eeu_BDT.txt BDT_uue=datacard_uue_BDT.txt BDTttZ_eee=datacard_eee_BDTttZ.txt BDTttZ_uuu=datacard_uuu_BDTttZ.txt BDTttZ_eeu=datacard_eeu_BDTttZ.txt BDTttZ_uue=datacard_uue_BDTttZ.txt mTW_eee=datacard_eee_mTW.txt mTW_uuu=datacard_uuu_mTW.txt mTW_eeu=datacard_eeu_mTW.txt mTW_uue=datacard_uue_mTW.txt > COMBINED_datacard.txt

mv datacard_*_*.txt datacards/.
