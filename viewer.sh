# write a command that will display the contents of the files passed as an argument
# to the script. If the file does not exist, display an error message and exit with
# a status of 1.

#!/bin/bash

# place in a variable the return value of the decode program

# res=0

# /home/quentin/Programmation/C++/QOI/decode $@

res=$(/home/quentin/Programmation/C++/QOI/decode $@)

if [res==0]; then
    echo "Success"
else
    if [ res == 1 ]; then
        echo "Error: no file passed as argument"
        exit 1
    elif [ res == 2 ]; then
        echo "Error: $@ is not a .qoi file or does not exist"
        exit 1
    else
        echo "Error: unknown error"
        echo "Error code: $res"
        exit 1
    fi
fi

declare -a ppm_files
for file in "$@"
do
    if [[ -f $file && ${file: -4} == ".qoi" ]]; then
        ppm_files+=("${file%.qoi}.ppm")
    else
        echo "Error: $file is not a .qoi file or does not exist"
        exit 1
    fi
done

# Print the .ppm file names
feh ${ppm_files[@]}

rm -rf ${ppm_files[@]}

exit 0