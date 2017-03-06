cp ../*.cc .
cp ../*.sh .
cp ../*.h .
#cp ../*.cxx .
#cp ../*.C .
cp ../Makefile .
cp ../*.txt .

cp -r ../COMBINE/ .

cp /home/nico/Bureau/these/tZq/MEM_Interfacing/MEM_NtupleMaker.* .

git add .
git commit -m "update"
git push origin master
