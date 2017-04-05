sudo ipsec stop
echo Testing various tunnels... >tmp.txt
for c in nn-modp3072 nn-newhope128 nn-ntru128 nn-ntruprime129 nn-modp3072-newhope128 nn-modp3072-ntru128 nn-modp3072-ntruprime129
do
echo Testing $c >>tmp.txt
sudo ipsec start | grep "connection " >>tmp.txt
sleep 1.00s
sudo ipsec up $c | grep "connection " >>tmp.txt
sleep 2.00s
sudo ipsec down $c >/dev/null
sleep 1.00s
sudo ipsec stop
sleep 1.00s
done
cat tmp.txt
rm -f tmp.txt

