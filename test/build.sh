#!/system/bin/bash

function compile() {
	local file="$1"
	local bin="${file%%.cc}"
	
	if [[ ! -f "$file" ]];then
		return
	fi
	
	echo "    CXX   $file"
	g++ "$file" -fPIC --pie -std=c++11 -I. -I../include -L../src -L.. -lupk -o "$bin"
}


if [[ "$1x" == ""x ]];then
	eval set -- $(ls *.cc 2>/dev/null)
fi

while [[ "$1"x != ""x ]];do
	f="$1"
	if [[ ! -f "$f" ]];then
		f="${f}.cc"
	fi
	
	compile "$f"
	shift
done

