cp ../*.cc .
cp ../*.sh .
cp ../*.h .
#cp ../*.cxx .
#cp ../*.C .
cp ../Makefile .
cp ../*.txt .

cp -r ../COMBINE/ .


git add .
git commit -m "update"
git push origin master
