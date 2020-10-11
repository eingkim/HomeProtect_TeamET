export STREAMER_PATH=$HOME/project/mjpg/mjpg-streamer/mjpg-streamer-experimental
export LD_LIBRARY_PATH=$STREAMER_PATH
echo $$
echo $$				#확인을 위한 mjpg.sh의 프로세서id 출력
echo $$
echo $$
$STREAMER_PATH/mjpg_streamer -i "input_raspicam.so" -o "output_http.so -p 8091 -w $STREAMER_PATH/www" 
				#라즈베리파이 스트리밍 실행
