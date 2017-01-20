#!/bin/sh

LOCAL_DIR=$(pwd)
BUILDROOT_DIR=$LOCAL_DIR/buildroot
BUILD_OUTPUT_DIR=$LOCAL_DIR/output

DEFCONFIG_ARRAY=("mesongxb_p200" "mesongxb_p200_32" "mesongxb_p201" "mesongxb_p201_32" "mesongxl_p212" "mesongxl_p212_32" "mesongxl_p230" "mesongxl_p230_32" "mesongxm_q200" "mesongxm_q200_32" "meson8_k200" "meson8_k200b" "meson8b_m200" "meson8b_m201" "meson8m2_n200")
BOARD_ARRAY=("p200" "p200" "p201" "p201" "p212" "p212" "p230" "p230" "q200" "q200" "k200" "k200b" "m200" "m201" "n200")
BUILD_TYPE_ARRAY=("64" "32" "64" "32" "64" "32" "64" "32" "64" "32" "32" "32" "32" "32" "32")

DEFCONFIG_ARRAY_LEN=${#DEFCONFIG_ARRAY[@]}

while [ $i -lt $DEFCONFIG_ARRAY_LEN ]
do
	echo ${DEFCONFIG_ARRAY[$i]}
	let i++
done

function choose_info()
{
	echo 
	echo "You're building on Linux"
	echo "Lunch menu...pick a combo:"
	i=0
	while [ $i -lt $DEFCONFIG_ARRAY_LEN ]
	do
		echo "$((${i}+1)). ${DEFCONFIG_ARRAY[$i]}_release"
		let i++
	done
	echo
}

function choose_type()
{
	choose_info
	local DEFAULT_NUM DEFAULT_VALUE
	DEFAULT_NUM=5
	DEFAULT_VALUE="mesongxl_p212_release"
	
	export TARGET_BUILD_TYPE=
	local ANSWER
	while [ -z $TARGET_BUILD_TYPE ]
	do
		echo -n "Which would you like? ["$DEFAULT_NUM"] "
		if [ -z "$1" ]; then
			read ANSWER
		else
			echo $1
			ANSWER=$1
		fi

		if [ -z "$ANSWER" ]; then
			ANSWER="$DEFAULT_NUM"
		fi

		if [ -n "`echo $ANSWER | sed -n '/^[0-9][0-9]*$/p'`" ]; then	
			if [ $ANSWER -le $DEFCONFIG_ARRAY_LEN ] && [ $ANSWER -gt 0 ]; then
				index=$((${ANSWER}-1))
				TARGET_BUILD_CONFIG="${DEFCONFIG_ARRAY[$index]}_release"
				TARGET_DIR_NAME="${DEFCONFIG_ARRAY[$index]}"
				TARGET_BUILD_TYPE="${BUILD_TYPE_ARRAY[$index]}"
				TARGET_BOARD_TYPE="${BOARD_ARRAY[$index]}"
			else
				echo
				echo "number not in range. Please try again."
				echo
			fi
		else
			echo
			echo "I didn't understand your response.  Please try again."
			echo
		fi
		if [ -n "$1" ]; then
			break
		fi
	done
	export TARGET_OUTPUT_DIR="$BUILD_OUTPUT_DIR/$TARGET_DIR_NAME"
}

function lunch()
{
	mkdir -p $TARGET_OUTPUT_DIR
	if [ -n "$TARGET_BUILD_CONFIG" ]; then
		cd buildroot
		echo "==========================================="
		echo  
		echo "#TARGET_BOARD=${TARGET_BOARD_TYPE}"
		echo "#BUILD_TYPE=${TARGET_BUILD_TYPE}"
		echo "#OUTPUT_DIR=output/$TARGET_DIR_NAME"
		echo "#CONFIG=${TARGET_BUILD_CONFIG}_defconfig"
		echo
		echo "==========================================="
		make O="$TARGET_OUTPUT_DIR" "$TARGET_BUILD_CONFIG"_defconfig
	fi
	cd $LOCAL_DIR
}
function function_stuff()
{
    choose_type
    lunch
}
function_stuff
