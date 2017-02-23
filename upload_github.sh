cp ../*.cc .
cp ../*.h .
cp ../*.cxx .
cp ../*.C .
cp ../Makefile .
cp ../README.md .

cp -r ../COMBINE/ .


git add .
git commit -m "update"
git push origin master
