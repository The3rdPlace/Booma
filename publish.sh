#!/bin/bash

# Get credentials
if [ -f .publish ]; then
  . .publish
else
  echo "Please create the file .publish with this content:"
  echo "PUBUSER=you"
  echo "PUBPASS=pass"
  echo "PUBHOST=ftp.where.com"
  echo "PUBROOT=put/here"
  exit 1
fi

MAJOR=$(cat booma/libbooma/include/booma.h | grep MAJORVERSION | cut -d ' ' -f 3)
MINOR=$(cat booma/libbooma/include/booma.h | grep MINORVERSION | cut -d ' ' -f 3)
BUILD=$(cat booma/libbooma/include/booma.h | grep BUILDNO | cut -d ' ' -f 3)

sed -i -e "s/{MAJORVERSION}/${MAJOR}/g" ../dist/site/index.html
sed -i -e "s/{MINORVERSION}/${MINOR}/g" ../dist/site/index.html
sed -i -e "s/{BUILD}/${BUILD}/g" ../dist/site/index.html
if [ ! -f "../dist/booma_${MAJOR}.${MINOR}-${BUILD}_amd64.deb" ]; then
  sed -i -e "s/DEBVISIBLE/DEBHIDDEN/g" ../dist/site/index.html
fi

# Archives
if [ -f "../dist/booma_${MAJOR}.${MINOR}-${BUILD}_amd64.deb" ]; then
  ncftpput -u $PUBUSER -p $PUBPASS $PUBHOST $PUBROOT/. ../dist/booma_${MAJOR}.${MINOR}-${BUILD}_amd64.deb
fi
ncftpput -u $PUBUSER -p $PUBPASS $PUBHOST $PUBROOT/. ../dist/booma_${MAJOR}.${MINOR}.orig.tar.gz


# Page(s)
ncftpput -u $PUBUSER -p  $PUBPASS -R $PUBHOST $PUBROOT/. ../dist/site/*
