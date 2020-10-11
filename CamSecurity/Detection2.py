import numpy as np
import cv2
import subprocess
import sys


from twilio.rest import Client


account_sid = "Twilio의 id"     				#Twilio의 sms 서비스 사용을 위한 id, token 입력
auth_token = "Twilio의 token"
client = Client(account_sid, auth_token)




faceCascade = cv2.CascadeClassifier('haarcascades/haarcascade_frontalface_default.xml')
bodyCascade = cv2.CascadeClassifier('haarcascades/haarcascade_fullbody.xml')
cap = cv2.VideoCapture(0)					#비디오 출력
cap.set(3,640) # set Width
cap.set(4,480) # set Height
while True:
    ret, img = cap.read()
    img = cv2.flip(img, 1) 
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    faces = faceCascade.detectMultiScale(			#얼굴 인식 시, 실행
        gray,
        scaleFactor=1.2,
        minNeighbors=5,
        minSize=(20, 20)
    )
    for (x,y,w,h) in faces:
        cv2.rectangle(img,(x,y),(x+w,y+h),(255,0,0),2)
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = img[y:y+h, x:x+w]
        cv2.imwrite('who.jpg',img)				#침입자의 얼굴을 라즈베리파이에 저장
        client.api.account.messages.create(				#얼굴 인식 시, Twilio로 sms를 전송(from->to) 
        to="+82사용자 휴대폰 번호",				
        from_="+Twilio계정의 임시 전화 번호",
        body="Warning! Intruder detected. Check application ")		#sms 메시지 내용
        cap.release()
        cv2.destroyAllWindows()	
        sys.exit()						#프로그램 종료
    
    body = bodyCascade.detectMultiScale(
        gray,
        scaleFactor=1.2,
        minNeighbors=5,
        minSize=(20, 20)
    )
    for (x,y,w,h) in faces:
        cv2.rectangle(img,(x,y),(x+w,y+h),(255,0,0),2)
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = img[y:y+h, x:x+w]
        client.api.account.messages.create(				#얼굴 인식 시, Twilio로 sms를 전송(from->to) 
        to="+82사용자 휴대폰 번호",
        from_="+Twilio계정의 임시 전화 번호",
        body="Warning! Intruder detected. Check application ")			#sms 메시지 내용
        cap.release()
        cv2.destroyAllWindows()
        sys.exit()						#프로그램 종료
        
        
    cv2.imshow('video',img) 
    
    
    k = cv2.waitKey(30) & 0xff
      	

    if k == 27: 
        break
cap.release()
cv2.destroyAllWindows()


