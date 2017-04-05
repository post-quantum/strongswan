sudo ipsec stop >/dev/null
for c in nn-modp3072 nn-modp3072-newhope128 nn-modp3072-ntru128 nn-modp3072-ntruprime129
do
echo Measuring $c >tmp.txt
sudo ipsec start --nofork | grep "PQPERF:" >>tmp.txt &
for i in {1..10}
do
sudo ipsec up $c >/dev/null
sleep 0.25s
sudo ipsec down $c >/dev/null
done
sudo ipsec stop
cat tmp.txt
done

