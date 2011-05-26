(*

@file ConfigureDMG.scpt
@author Jacek Antonelli
@brief Script for configuring the Mac installer disk image.

Copyright (c) 2011, Jacek Antonelli

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-----

This AppleScript script configures the view options and icon layout of
the Mac installer disk image (DMG) as part of the packaging process.
See also scripts/package.py, which executes this script.

This script takes two required positional command line arguments:

  1: the name of the mounted volume (e.g. for "/Volumes/Imprudence Installer",
     the volume name is "Imprudence Installer").
  2: the name of the application file (e.g. "Imprudence.app").

Example usage:

  osascript ConfigureDMG.scpt "Imprudence Installer" "Imprudence.app"

Some preparation is necessary before running this script:

  * The target disk image must be currently attached as a volume, with
    the volume name specified by the first command line argument.
  * The volume must contain the application file, with the file
    name specified by the second command line argument.
  * The volume must contain the "background.png" image file.
  * The volume must not contain a file or folder named "Applications".
  * It might be necessary to "Enable access for assistive devices"
    in System Preferences > Universal Access.

*)

on run argv

  -- Read the first positional argument, the volume name.
  set volumeName to item 1 of argv

  -- Read the second positional argument, the app name.
  set appName to item 2 of argv

  tell application "Finder" to tell disk volumeName
    -- Open the volume in a Finder window.
    open 
    set theWindow to the container window

    -- Tweak some options.
    set current view of theWindow to icon view
    set toolbar visible of theWindow to false
    set statusbar visible of theWindow to false

    -- Set window to position {150,150}, size {+600,+420}.
    set bounds of theWindow to {150, 150, 750, 570}

    -- Tweak some more options.
    set viewOptions to the icon view options of theWindow
    set arrangement of viewOptions to not arranged
    set icon size of viewOptions to 128

    -- Make sure background.png is visible, so Finder can see it.
    set bgPicPath to the quoted form of (the POSIX path of (it as alias) & "background.png")
    do shell script ("SetFile -a v " & bgPicPath)
    update without registering applications

    -- Use background.png as the background picture.
    set background picture of viewOptions to file "background.png"

    -- Now set background.png to invisible, so the end user won't see it.
    do shell script ("SetFile -a V " & bgPicPath)

    -- Position the application file.
    set position of item appName of theWindow to {138, 260}

    -- Create and position an alias to the Applications folder.
    set appAlias to make new alias file at theWindow to POSIX file "/Applications"
    set name of appAlias to "Applications"
    set position of appAlias to {470, 260}

    -- Visually update the window so all the changes take effect.
    update without registering applications

    -- Pause briefly so we can admire the results.
    delay 2
  end tell

end run
