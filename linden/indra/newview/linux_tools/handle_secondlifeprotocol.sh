#!/bin/bash

# Send a URL of the form secondlife://... to Second Life.
#

URL="$1"

if [ -z "$URL" ]; then
    echo Usage: $0 secondlife://...
    exit
fi

RUN_PATH=`dirname "$0" || echo .`
cd "${RUN_PATH}"

if [ `pidof do-not-directly-run-imprudence-bin` ]; then
	exec dbus-send --type=method_call --dest=com.secondlife.ViewerAppAPIService /com/secondlife/ViewerAppAPI com.secondlife.ViewerAppAPI.GoSLURL string:"$1"
else
	exec ./imprudence -url \'"${URL}"\'
fi

