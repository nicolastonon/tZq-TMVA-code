cp ../*.cc .
cp ../*.h .
cp ../*.cxx .
cp ../*.C .
cp ../Makefile .
cp ../README.txt .

cp -r ../COMBINE/ .


git add .
git commit -m "update"
git push origin master
