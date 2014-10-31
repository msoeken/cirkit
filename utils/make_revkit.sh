#!/bin/bash

qa/check_header.py
qa/check_footer.py
qa/check_author.py

VERSION=`cat src/core/version.cpp | grep static | awk '{print $6}' | sed -e "s/[\";]//g"`

echo -e "\e[1;37m[I]\e[0m Create archive \e[0;32mrevkit-$VERSION.tar.gz\e[0m from working copy"

git archive --prefix=revkit-$VERSION/ --output=revkit-$VERSION.tar.gz HEAD

echo -e "\e[1;37m[I]\e[0m Unpacking archive \e[0;32mrevkit-$VERSION.tar.gz\e[0m"

tar xfz revkit-$VERSION.tar.gz

echo -e "\e[1;37m[I]\e[0m Deleting archive \e[0;32mrevkit-$VERSION.tar.gz\e[0m"

rm revkit-$VERSION.tar.gz

echo -e "\e[1;37m[I]\e[0m Renaming \e[0;32mcirkit\e[0m to \e[0;32mrevkit\e[0m from working copy"

find revkit-$VERSION -type f | xargs sed -i -e "s/cirkit/revkit/g"
find revkit-$VERSION -type f | xargs sed -i -e "s/CirKit/RevKit/g"

for d in `ls -1 revkit-$VERSION/addons/ | grep -e "^cirkit-addon" | sed -e "s/cirkit-addon-//g"`
do
    mv revkit-$VERSION/addons/cirkit-addon-$d revkit-$VERSION/addons/revkit-addon-$d
done

echo -e "\e[1;37m[I]\e[0m Deleting \e[0;32m.gitignore\e[0m and \e[0;32m.travis.yml\e[0m file"

rm -Rf revkit-$VERSION/.gitignore revkit-$VERSION/.travis.yml

echo -e "\e[1;37m[I]\e[0m Deleting \e[0;32mutils\e[0m, \e[0;32mqa\e[0m and \e[0;32mnotes\e[0m directory"

rm -Rf revkit-$VERSION/utils
rm -Rf revkit-$VERSION/qa
rm -Rf revkit-$VERSION/notes

echo -e "\e[1;37m[I]\e[0m Deleting GUI"

rm -Rf revkit-$VERSION/gui/programs/gui
sed -i "25,40d" revkit-$VERSION/gui/CMakeLists.txt

echo -e "\e[1;37m[I]\e[0m Repacking archive \e[0;32mrevkit-$VERSION.tar.gz\e[0m"

tar cfz revkit-$VERSION.tar.gz revkit-$VERSION

echo -e "\e[1;37m[I]\e[0m Deleting directory \e[0;32mrevkit-$VERSION\e[0m"

rm -Rf revkit-$VERSION

