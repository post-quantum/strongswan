To run the tests (pqtest.sh) or measure key-exchange time and packet sizes (pqperf.sh)
build with PQPERF defined (e.g. add to compiler args via -DPQPERF) and ensure this
side (London) has the pq_ldn_ipsec.conf and the remote side (New York) is using 
the pq_nyc_ipsec.conf.

PQTest.sh should show a 'Connection succeeded' for every connection it tests. If
you get no output for a connection that implies a configuration issue... run ipsec
commands locally before retrying the script.
