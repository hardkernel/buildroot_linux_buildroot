#!/bin/sh

LOCAL_DIR=$(pwd)
BUILDROOT_DIR=$LOCAL_DIR/buildroot
BUILD_OUTPUT_DIR=$LOCAL_DIR/output

DEFCONFIG_ARRAY=( "odroidn2" "odroidn2_bios" )

DEFCONFIG_ARRAY_LEN=${#DEFCONFIG_ARRAY[@]}

i=0
while [[ $i -lt $DEFCONFIG_ARRAY_LEN ]]
do
	let i++
done

function choose_info()
{
	echo
	echo "You're building on Linux"
	echo "Lunch menu...pick a combo:"
	i=0
	while [[ $i -lt $DEFCONFIG_ARRAY_LEN ]]
	do
		if [[ ${DEFCONFIG_ARRAY[$i]} =~ "debug" ]]; then
			echo "$((${i}+1)). ${DEFCONFIG_ARRAY[$i]}"
		else if [[ ${DEFCONFIG_ARRAY[$i]} =~ "release" ]]; then
			echo "$((${i}+1)). ${DEFCONFIG_ARRAY[$i]}"
		else
			echo "$((${i}+1)). ${DEFCONFIG_ARRAY[$i]}_release"
		fi
		fi
		let i++
	done
	echo
}

function get_index() {
	if [ $# -eq 0 ]; then
		return 0
	fi

	i=0
	while [[ $i -lt $DEFCONFIG_ARRAY_LEN ]]
	do
		if [[ "${DEFCONFIG_ARRAY[$i]}" =~ "debug" ]]; then
			if [ $1 = "${DEFCONFIG_ARRAY[$i]}" ]; then
				let i++
				return ${i}
			fi
		else if [[ "${DEFCONFIG_ARRAY[$i]}" =~ "release" ]]; then
			if [ $1 = "${DEFCONFIG_ARRAY[$i]}" ]; then
                                let i++
                                return ${i}
                        fi
		else
			if [ $1 = "${DEFCONFIG_ARRAY[$i]}_release" ]; then
				let i++
				return ${i}
			fi
		fi
		fi
		let i++
	done
	return 0
}

function get_target_board_type() {
	TARGET=$1
	RESULT="$(echo $TARGET | cut -d '_' -f 2)"
	echo "$RESULT"
}

function get_build_config() {
	TARGET=$1
	RESULT1="$(echo $TARGET | cut -d '_' -f 3)"
	RESULT2="$(echo $TARGET | cut -d '_' -f 4)"
	RESULT3="$(echo $TARGET | cut -d '_' -f 5)"
	if [[ $RESULT1 = "debug" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT1 = "release" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT2 = "debug" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT2 = "release" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT3 = "release" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	else
		echo "${DEFCONFIG_ARRAY[$index]}_release"
	fi


}
function get_target_build_type() {
	TARGET=$1
	TYPE="$(echo $TARGET | cut -d '_' -f 1)"
	if [[ $TYPE =~ "meson8" ]]; then
		echo "32"
		return
	fi

	LENGTH="$(echo $TARGET | awk -F '_' '{print NF}')"
	if [ $LENGTH -le 2 ]; then
		echo "64"
	else
		RESULT="$(echo $TARGET | cut -d '_' -f 3)"
		if [ $RESULT = "32" ]; then
			echo "32"
		elif [ $RESULT = "64" ]; then
			echo "64"
		else
			echo "64"
		fi
	fi
}

function choose_type()
{
	choose_info
	local DEFAULT_NUM DEFAULT_VALUE
	DEFAULT_NUM=1
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
				TARGET_BUILD_CONFIG=`get_build_config ${DEFCONFIG_ARRAY[$index]}`
				TARGET_DIR_NAME="${DEFCONFIG_ARRAY[$index]}"
				TARGET_BUILD_TYPE=`get_target_build_type ${DEFCONFIG_ARRAY[$index]}`
				TARGET_BOARD_TYPE=`get_target_board_type ${DEFCONFIG_ARRAY[$index]}`
			else
				echo
				echo "number not in range. Please try again."
				echo
			fi
		else
			get_index $ANSWER
			ANSWER=$?
			if [ $ANSWER -gt 0 ]; then
				index=$((${ANSWER}-1))
				TARGET_BUILD_CONFIG=`get_build_config ${DEFCONFIG_ARRAY[$index]}`
				TARGET_DIR_NAME="${DEFCONFIG_ARRAY[$index]}"
				TARGET_BUILD_TYPE=`get_target_build_type ${DEFCONFIG_ARRAY[$index]}`
				TARGET_BOARD_TYPE=`get_target_board_type ${DEFCONFIG_ARRAY[$index]}`
			else
				echo
				echo "I didn't understand your response.  Please try again."
				echo
			fi
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
	choose_type $@
	lunch
}
function_stuff $@
