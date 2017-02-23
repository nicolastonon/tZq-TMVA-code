cp ../*.cc .
cp ../*.h .
cp ../*.cxx .
cp ../*.C .
cp ../Makefile
cp ../README.md .

cp -r ../COMBINE/datacards ./COMBINE
cp ../COMBINE/*.C ./COMBINE
cp ../COMBINE/*.py ./COMBINE
cp ../COMBINE/*.txt ./COMBINE
cp ../COMBINE/*.sh ./COMBINE

git add .
git commit -m "update"
git push origin master
