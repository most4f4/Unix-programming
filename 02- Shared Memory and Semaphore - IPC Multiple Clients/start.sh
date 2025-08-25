./client1 &
sleep 1
./client2 &
sleep 1
./client3 &
wait
echo "All clients finished"