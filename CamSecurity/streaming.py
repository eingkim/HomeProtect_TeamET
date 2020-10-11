import subprocess
import sys
import RPi.GPIO as g	#GPIO핀 사용
import time


g.setmode(g.BCM)
g.setup(20, g.IN)		#GPIO20번 핀을 입력핀으로 설정
while g.input(20)==0:	#GPIO20번 핀에 HIGH가 들어올 떄 까지 sleep.
    time.sleep(0.00000001)	
    
subprocess.call('killall -9 mjpg_streamer', shell=True)	#GPIO20번 핀에 HIGH 입력 시, 스트리밍 프로세스 종료
print(g.input(20))
time.sleep(5)
