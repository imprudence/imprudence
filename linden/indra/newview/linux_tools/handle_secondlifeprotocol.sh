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

#yeah, why just send a dbus message  if we can spawn a whole new instance of the viewer just for sending the message?
#exec ./imprudence -url \'"${URL}"\'
#</sarcasm>
exec dbus-send --type=method_call --dest=com.secondlife.ViewerAppAPIService /com/secondlife/ViewerAppAPI com.secondlife.ViewerAppAPI.GoSLURL string:"$1"
