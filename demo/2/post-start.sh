#!/bin/bash -

sleep 5

echo 

echo -n "Waiting for experiment completion.."

while (ps ax | grep -q "mge[n]"); do 
    echo -n "."
    sleep 1;
done

echo "done."

node1_sent=$(grep 'SEND proto>UDP flow>1' /tmp/lxc-node/2/1/var/log/mgen.out  | wc -l)

node3_sent=$(grep 'SEND proto>UDP flow>1' /tmp/lxc-node/2/3/var/log/mgen.out  | wc -l)


node1_recv=$(grep 'src>10.100.0.1/5001' /tmp/lxc-node/2/2/var/log/mgen.out  | wc -l)

node3_recv=$(grep 'src>10.100.0.3/5001' /tmp/lxc-node/2/2/var/log/mgen.out  | wc -l)

echo

printf "Flow   %-4s %-4s\n" 1 3

printf -- "----%-4s %-4s\n" \
    "------" "------" 

printf "Tx     %-4s %-4s\n" \
    $node1_sent $node3_sent

printf "Rx     %-4s %-4s\n" \
    $node1_recv $node3_recv

printf -- "----%-4s %-4s\n" \
    "------" "------" 

printf "Comp%%  %-4.3s %-4.3s\n" \
    $(echo "scale=9; $node1_recv/$node1_sent * 100.0;" | bc) \
    $(echo "scale=9; $node3_recv/$node3_sent * 100.0;" | bc)

echo
