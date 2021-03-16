make clean
make all
time ./kiwi-bench write 5000 1 >> out.txt 
sleep 5 
time ./kiwi-bench read 20000 1 >> out.txt
sleep 5
time ./kiwi-bench read 20000 10 >> out.txt 
sleep 5
time ./kiwi-bench read 20000 50 >> out.txt
sleep 5
time ./kiwi-bench read 20000 100 >> out.txt 
sleep 5
time ./kiwi-bench read 20000 200 >> out.txt
sleep 10
time ./kiwi-bench read 20000 500 >> out.txt 
sleep 10
time ./kiwi-bench read 20000 1000 >> out.txt 


