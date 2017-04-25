VER=0.1-3

# Ensure working directory is where this script is
SCRIPT=$(readlink -f "$0")
SCRIPTDIR=$(dirname "$SCRIPT")
cd $SCRIPTDIR

# Create the package build directory and initialise it with the control files
PACKAGEDIR=pqstrongswan_$VER
sudo rm -rf $PACKAGEDIR
mkdir $PACKAGEDIR
cp -r template/DEBIAN $PACKAGEDIR/
sed -i "s/\$VER/$VER/g" $PACKAGEDIR/DEBIAN/control

# Build the source with the necessary config for a release
CONFIGOPTS=`cat configopts`
cd ../..
./configure --prefix=/usr --sysconfdir=/etc $CONFIGOPTS
make

# Make install to the package build directory
sudo make DESTDIR=$SCRIPTDIR/$PACKAGEDIR/ install
cd $SCRIPTDIR

# Copy the template config files into place
sudo cp template/etc/*.* $PACKAGEDIR/etc/
sudo chown root:root $PACKAGEDIR/etc/ipsec.conf
sudo chown root:root $PACKAGEDIR/etc/ipsec.secrets
sudo chown root:root $PACKAGEDIR/etc/strongswan.conf

# Build the package
sudo dpkg-deb --build $PACKAGEDIR

# Upload to Bintray
if [ -z "$BINTRAY_USER" ] ; then
echo Package built but not uploaded. Rerun with BINTRAY_USER and BINTRAY_API_KEY set to automatically upload the .deb package at the end.
else
echo Uploading to Bintray...
curl -X PUT -T $PACKAGEDIR.deb -u$BINTRAY_USER:$BINTRAY_API_KEY https://api.bintray.com/content/post-quantum/pq-debian/pqstrongswan/$VER/$PACKAGEDIR.deb -H "X-Bintray-Debian-Distribution: xenial" -H "X-Bintray-Debian-Component: main" -H "X-Bintray-Debian-Architecture: amd64"
fi

