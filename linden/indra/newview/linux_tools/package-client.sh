#!/bin/sh
MANIFEST=$1
PACKAGE_NAME=$2
GRID=$3

# Check that the entire client manifest is there.
cd newview
echo Checking manifest...

# Strip out comment lines and empty lines
# Replace anything with a source,dest pairs with just source filename
if ! ls -d `cat "$MANIFEST" | \
	grep -v ^# | grep -v ^$ | \
	sed 's/,.*//'` 1>/dev/null
then
	echo Client manifest defined in newview/$MANIFEST is not complete.
	exit 1
fi
echo "Done."

# See if the package already exists.
BUILD_PACKAGE=YES
if [ -a $PACKAGE_NAME ]
then
	echo The directory "newview/$PACKAGE_NAME" already exists.
	echo Checking for complete client manifest...

	cd $PACKAGE_NAME

	# Strip out comment lines and empty lines
	# Replace source,dest pairs with just dest filename
	if ls -d `cat "../$MANIFEST" | \
		grep -v ^# | grep -v ^$ | \
		sed 's/.*,\(.*\)/\1/'` 1>/dev/null
	then
		echo "Done."
		echo Package at "newview/$PACKAGE_NAME" looks complete.
		cd ..
		BUILD_PACKAGE=NO
	else
		echo Incomplete package at "newview/$PACKAGE_NAME"!
		echo Removing corrupt package...
		cd ..
		rm -rf $PACKAGE_NAME
		echo Done.
	fi
fi

echo Building newview/$PACKAGE_NAME directory...
## First read all directories mentioned in the manifest, and create a package skeleton.

# Strip out comment lines and empty lines
# Replace source,dest pairs with just dest filename
# Strip out and line that does not include a directory in its path (ie contains a '/')
# Extract everything up to the last '/' and prefix with $PACKAGE_NAME
# Print out just the unique directores, and make them.
mkdir -p `cat $MANIFEST | \
	grep -v ^# | \
	grep -v ^$ | \
	sed 's/.*,\(.*\)/\1/' | \
	grep \/ | \
	sed "s/\(^.*\)\/[^\/]*/$PACKAGE_NAME\/\1/" | \
	sort | uniq`

## Copy the manifest.

# Strip out comment lines and empty lines
# Strip out empty directories
# Replace any line without a ',' with 'line,line'
for pair in `cat $MANIFEST | \
		grep -v ^# | \
		grep -v ^$ | \
		grep -v \/$ | \
		sed 's/\(^[^,]*$\)/\1,\1/' `
do
	# $pair is 'source,dest' ... split it up
	SOURCE=`echo "$pair" | awk -F, '{ print $1; }'`
	DEST=`echo "$pair" | awk -F, '{ print $2; }'`
	# If this is a wildcard copy (pair contains a '*'), then remove the wildcard from $DEST 
	# and make the copy recursive
	RECURSE=""
	if [ ! x == x`echo "$SOURCE" | grep \*$` ]
	then
		DEST=`echo "$DEST" | sed 's/\*$//'`
		RECURSE="-R"
	fi
	# The -a makes us copy links as links, plus timestamps etc.
	cp -a $RECURSE $SOURCE "$PACKAGE_NAME/$DEST"
done

echo Done.

## Clean up any CVS directories that might have been recursively included.
echo Pruning CVS directories from newview/$PACKAGE_NAME directory...
find $PACKAGE_NAME -type d -name CVS -exec rm -rf {} \; 2>/dev/null
echo "Done removing CVS directories."

## Clean up any SVN directories that might have been recursively included.
echo Pruning .svn directories from newview/$PACKAGE_NAME directory...
find $PACKAGE_NAME -type d -name \.svn -exec rm -rf {} \; 2>/dev/null
echo "Done removing .svn directories."

# Create an appropriate gridargs.dat for this package, denoting required grid.
if [ X$GRID == X'default' ]
then
    echo 'Default grid - creating empty gridargs.dat'
    echo " " > $PACKAGE_NAME/gridargs.dat
else
    echo "Creating gridargs.dat for package, grid $GRID"
    echo "--${GRID} -helperuri http://preview-${GRID}.secondlife.com/helpers/" > $PACKAGE_NAME/gridargs.dat
fi

TARBALL=$PACKAGE_NAME.tar.bz2

# See if the tarball already exists.
if [ -a $TARBALL ]
then
	echo Tarball "newview/$TARBALL" already exists.  Skipping tarball creation.
	exit 0
fi

echo Creating tarball "newview/$TARBALL"...
# --numeric-owner hides the username of the builder for security etc.
tar --numeric-owner -cjf $TARBALL $PACKAGE_NAME
echo Done.

