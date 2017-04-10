sudo ipsec stop >/dev/null
git pull || exit 1
make clean >/dev/null
./configure --disable-ikev1 --enable-ha --enable-sha3 --enable-newhope --enable-ntru --enable-bliss --enable-openssl --enable-chapoly --enable-hybrid-quantum-safe --enable-curve25519 || exit 1
make || exit 1
sudo make install || exit 1



