cp ../*.cc .
cp ../*.sh .
cp ../*.h .
#cp ../*.cxx .
#cp ../*.C .
cp ../Makefile .
cp ../*.txt .

cp ../other/getROC_curves.cc .

cp ../outputs/Reader* ./outputs/

cp ../COMBINE/README_Combine.txt ./COMBINE/

cp ../COMBINE/templates/Combine_Input.root ./COMBINE/templates/.

cp ../COMBINE/datacards/Create_Script_Datacard_Generation.cc ./COMBINE/datacards/.
cp ../COMBINE/datacards/AddBinByBin.py ./COMBINE/datacards/.
cp ../COMBINE/datacards/Draw_Canvas.C ./COMBINE/datacards/.
cp ../COMBINE/datacards/Generate_Datacards.py ./COMBINE/datacards/.
cp ../COMBINE/datacards/Template_Datacard.txt ./COMBINE/datacards/.
cp ../COMBINE/datacards/Makefile ./COMBINE/datacards/.

cp /home/nico/Bureau/these/tZq/MEM_Interfacing/MEM_NtupleMaker.h .
cp /home/nico/Bureau/these/tZq/MEM_Interfacing/MEM_NtupleMaker.cxx .

git add .
git commit -m "update"
git push origin master
