#!/bin/bash

sed /^4096$/d input.txt | sort -g > out1.txt
./mt_sort input.txt out2.txt
diff out1.txt out2.txt

if [ $? = 0 ]; then
    echo OK!
fi

rm out1.txt out2.txt
