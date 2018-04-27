#!/bin/sh

#~%~^~%~^~%~^~%~^~%~^~%~^~% HEADER ~%~^~%~^~%~^~%~^~%~^~%~^~%#

#IOS - OPERACNE SYSTEMY
#PROJ 1 - ANALYZA ADRESAROV

#MARTINA GRZYBOWSKA (xgrzyb00)

#dirstat [-i FILE_ERE] [DIR]

#[-i FILE_ERE] - extended-regex switcher
#[DIR] - path do directory which is to be analyzes

#~%~^~%~^~%~^~%~^~%~^~% END OF HEADER ~%~^~%~^~%~^~%~^~%~^~%#
#export LC_ALL=POSIX

#IF ARG IS NOT ZERO AND IT IS NOT A DIR OR -i
if [ $# -ne 0 ]; then
	if [ ! -d "$1" ] && [ "$1" != "-i" ]; then
		echo "INVALID FIRST ARGUMENT" >&2
		exit 1
	fi
fi

#CHANGING IFS FOR THE "FOR" LOOP
ORIGINAL_IFS=$IFS

#BECAUSE OF HIS ROYAL HIGHNESS, DASH, SINCE THE SNEAKY LITTLE ... POO WOULD NOT ACCEPT $'\n'
IFS='
'

while getopts ":i:" opt; do
	case $opt in
		i)
			OPT_ARG="$OPTARG"

			OPTION_ALIVE_AND_KICKING=true;;
		\?)
			echo "INVALID ARGUMENT OPTION -$OPTARG" >&2
			exit 1;;
		:)
			echo "OPTION -$OPTARG REQUIRES AN ARGUMENT" >&2
			exit 1;;
	esac
done

case "$#" in 
	0|2)
		DIR=$(pwd);;
	1)
		DIR=$1;;
	3)
		DIR=$3;;
	*) 
		echo "INVALID INPUT - TOO MANY ARGUMENTS" >&2
		exit 1;;
esac

#IF ROOT DIR MATCHES REGEX
if [ "$OPTION_ALIVE_AND_KICKING" = true ]; then
	ROOT_DIR=$(echo "$PWD" | sed 's/.*\///')
	COMPARE=$(echo "$ROOT_DIR" | grep "$OPT_ARG")
	if [ "$ROOT_DIR" = "$COMPARE" ]; then
    	echo "OPTION ARGUMENT MATCHES THE NAME OF ROOT DIRECTORY" >&2
    	exit 1
	fi
fi

#IF DIR EXISTS AND IS SAFE TO USE
if [ -d $DIR ]; then
	#CHANGE THE WORKING DIR FOR SCRIPT
	cd "$DIR"
else
	echo "DIRECTORY MAY NOT EXIST" >&2
	exit 1
fi

#~%~^~%~^~%~^~%~^~%~^~%~^~% PATHS SECTION ~%~^~%~^~%~^~%~^~%~^~%~^~%#

#SEARCHING THE ROOT FOR THE RESPECTIVE PATHS OF ALL SUBDIRS AND FILES
DEPTH_FIRST_SEARCH="$(find . -depth 2>/dev/null)"

#IF SEARCH RETURNED VALUE OTHER THAN ZERO
SEARCH_STATE="$?"
if [ $SEARCH_STATE = 1 ]; then
	echo "ACCESS DENIED, PROCESS IS TERMINATED" >&2
	exit 1
fi

#~%~^~%~^~%~^~%~^~%~^~%~^~% FILTER SECTION ~%~^~%~^~%~^~%~^~%~^~%~^~%#

#IF THE FILTER IS ON
if [ "$OPTION_ALIVE_AND_KICKING" = true ]; then
	
	#REMOVE ./ AND / AND ' ' FROM THE PATHS
	DEPTH_FIRST_SEARCH="$(echo "$DEPTH_FIRST_SEARCH" | grep -E -v '\.$' | sed 's/.\///' | tr '/' '\t')"
	FILTERED_SEARCH=false
	
	for INDEX in $DEPTH_FIRST_SEARCH; do
		#PUT EVERY WORD IN INDEX TO SEPARATE LINES
		INDEX="$(echo "$INDEX" | tr '\t' '\n')"
		OBJECTS="$(echo "$INDEX" | grep -Ev "$OPT_ARG")"
		#IF THERE WASN'T FILE_ERE IN INDEX, IT WILL BE ADDED TO THE LIST
		if [ "$INDEX" = "$OBJECTS" ]; then
			INDEX="$(echo "$INDEX" | tr '\n' '/' | sed 's|\/$||')"
			if [ "$FILTERED_SEARCH" = false ]; then
				FILTERED_SEARCH="$INDEX"
			else
				FILTERED_SEARCH="$FILTERED_SEARCH
$INDEX"
			fi
		fi
	done
	#ADD ./ TO THE BEGINNING OF EVERY LINE
	FILTERED_SEARCH="$(echo "$FILTERED_SEARCH" | sed 's/^/.\//')"
else 
	#IF THE FILTER WAS NOT ON
	FILTERED_SEARCH="$DEPTH_FIRST_SEARCH"
fi
#~%~^~%~^~%~^~%~^~%~^~%~^~% END OF FILTER SECTION ~%~^~%~^~%~^~%~^~%~^~%~^~%#

GREPPED_DIRS_SOURCE=false
GREPPED_FILES_SOURCE=false

#SORT FILES AND DIRS
for INDEX1 in $FILTERED_SEARCH; do
	if [ -d "$INDEX1" ] && [ ! -h "$INDEX1" ]; then
		if [ "$GREPPED_DIRS_SOURCE" = false ]; then
			GREPPED_DIRS_SOURCE="$INDEX1"
		else
			GREPPED_DIRS_SOURCE="$GREPPED_DIRS_SOURCE
$INDEX1"
		fi
	elif [ -f "$INDEX1" ] && [ ! -h "$INDEX1" ]; then
		if [ "$GREPPED_FILES_SOURCE" = false ]; then
			GREPPED_FILES_SOURCE="$INDEX1"
		else
			GREPPED_FILES_SOURCE="$GREPPED_FILES_SOURCE
$INDEX1"
		fi
	fi
done

#~%~^~%~^~%~^~%~^~%~^~%~^~% END OF PATHS SECTION ~%~^~%~^~%~^~%~^~%~^~%~^~%#

#DIRECTORIES, ND-number, DD-depth
#IF THERE ARE OTHER DIRECTORIES THAN ROOT OF THE RELATIVE PATH
if [ "$GREPPED_DIRS_SOURCE" != "./" ]; then
	DIRS_SOURCE="$(echo "$GREPPED_DIRS_SOURCE" | awk -F/ '{print NF} END{print NR}')"
	ND="$(echo "$DIRS_SOURCE" | tail -n -1 )"
	DD="$(echo "$DIRS_SOURCE" | tr '\n' ' ' | cut -d' ' -f1-$ND | tr ' ' '\n' | sort -n | tail -n -1)"
	if [ "$OPTION_ALIVE_AND_KICKING" = true ]; then
		ND=$((ND+1))
	fi
else
	#ELSE NUMBER OF DIRS AND DEPTH IS EQUAL TO 1
	ND=1
	DD=1
fi

#IF THERE ARE FILES IN THE DIR, THE VALUE IS OTHER THAN FALSE
if [ "$GREPPED_FILES_SOURCE" != false ]; then
	FILES_SOURCE="$(echo "$GREPPED_FILES_SOURCE" | tr '\n' '\0' | xargs -0 ls -l | awk '{print $5} {s+=$5} END{print NR, s}')"
	NF="$(echo "$FILES_SOURCE" | tail -n -1 | cut -d' ' -f1)"
	AF="$((NF/ND))"
	DIR_SIZE="$(echo "$FILES_SOURCE" | tail -n -1 | cut -d' ' -f2)"
	NF_ALL_SIZES="$(echo "$FILES_SOURCE" | tr '\n' ' ' | cut -d' ' -f1-$NF | tr ' ' '\n' | sort -n | tr '\n' ' ')"
else
	#ELSE EVERYTHING IS EQUAL TO ZERO
	NF=0
	AF=0
	DIR_SIZE=0
fi

#LARGEST FILE
if [ $NF = 0 ]; then 
	LF="N/A"
else
	LF="$(echo "$NF_ALL_SIZES" | tr '\n' ' ' | cut -d' ' -f$NF)"
	if [ "$LF" -eq 0 ]; then
		LF="N/A"
	fi
fi

#AVARAGE FILE SIZE
if [ "$DIR_SIZE" = 0 ] || [ "$NF" = 0 ]; then
	AS="N/A"
else
	AS="$((DIR_SIZE/NF))"
fi

F_MEDIAN (){
	if [ "$1" = 0 ]; then 
		MS_MEXT="N/A"
	else
		if [ "$(($1%2))" = 1 ]; then
			counter="$((($1+1)/2))"
			MS_MEXT="$(echo "$2" | cut -d' ' -f$counter)"
		else
			counter_1="$(($1/2+1))"
			counter_2="$(($1/2))"
			MS_MEXT="$(echo "$2" | awk '{median=($'$counter_1'+$'$counter_2')/2} {printf("%d", median)}')"
		fi
	fi
}
#CALLING MEDIAN FUNCTION FOR MS
F_MEDIAN "$NF" "$NF_ALL_SIZES"

#FILE EXTENSIONS 
EL="$(echo "$GREPPED_FILES_SOURCE" | sed 's/..//' | sed 's/.*\///' | sed 's/^\.//' | grep '\.' | sed 's/.*\.//' | grep -E '^[[:alnum:]]*$' | sort -u | tr '\n' ',' | sed 's/,$//')"

echo "Root directory: $DIR"
echo "Directories: $ND"
echo "Max depth: $DD"
echo "Average no. of files: $AF"
echo "All files: $NF"
echo "  Largest file: $LF"
echo "  Average file size: $AS"
echo "  File size median: $MS_MEXT"
echo "File extensions: $EL" 

IFS=' '
#FOR THE "FOR" LOOP
EL="$(echo $EL | tr ',' ' ')"

for EXT in $EL; do
	EXT_SOURCE="$(echo "$GREPPED_FILES_SOURCE" | grep "\.$EXT$" | tr '\n' '\0' | xargs -0 ls -l | awk '{print $5} {s+=$5} END{print NR, s}')"
	NEXT="$(echo "$EXT_SOURCE" | tail -n -1 | cut -d' ' -f1)"
	EXT_TOTAL_SIZE="$(echo "$EXT_SOURCE" | tail -n -1 | cut -d' ' -f2)"
	EXT_ALL_SIZES="$(echo "$EXT_SOURCE" | tr '\n' ' ' | cut -d' ' -f1-$NEXT | tr ' ' '\n' | sort -n)"
	LEXT="$(echo "$EXT_ALL_SIZES" | tr '\n' ' ' | cut -d' ' -f$NEXT)"
	EXT_ALL_SIZES="$(echo "$EXT_SOURCE" | tr '\n' ' ' | cut -d' ' -f1-$NEXT | tr ' ' '\n' | sort -n | tr '\n' ' ')"
	AEXT="$((EXT_TOTAL_SIZE/NEXT))"
	
	#CALLING MEDIAN FUNCTION FOR MEXT
	F_MEDIAN "$NEXT" "$EXT_ALL_SIZES"
	
	echo "Files .$EXT: $NEXT"
	echo "  Largest file .$EXT: $LEXT"
	echo "  Average file size .$EXT: $AEXT"
	echo "  File size median .$EXT: $MS_MEXT"
done

IFS=$ORIGINAL_IFS

exit 0