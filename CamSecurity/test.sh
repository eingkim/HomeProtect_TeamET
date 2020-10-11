#! /bin/bash
echo "Hello bash"

while [ true ]		#아래의 과정을 반복
do
python3 Detection2.py	#침임자 검출 프로그램 실행
echo "detection end"

sh mjpg.sh &		#스트리밍을 백그라운드 프로세스로 실행

sleep 5


python3 streaming.py 	#아두이노로부터 스트리밍 종료 신호 인식




echo "search end"

done