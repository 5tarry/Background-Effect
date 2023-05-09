import argparse
import logging
import sys
import socket
import threading
import cv2
from ml import Classifier
from ml import Movenet
from ml import MoveNetMultiPose
from ml import Posenet
import utils
import utils2
import numpy as np
# 사용할 movenet 모델
estimation_model = 'movenet_lightning'
tracker_type = 'bounding_box'
# 직접 학습시킨 model 파일명
classification_model='term_pose_classifier'
# 직접 학습시킨 model의 label 파일명
label_file = 'term_pose_labels.txt'
camera_id=0
width = 640
height = 480
TCP_IP = '000.000.000.000' # 라즈베리파이 IP 주소
TCP_PORT = 9000
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))
print('connect')
# Movenet model 실행
pose_detector = Movenet(estimation_model)
# 카메라로부터 이미지 캡쳐
cap = cv2.VideoCapture(camera_id)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
# Visualization parameters
row_size = 20 # pixels
left_margin = 24 # pixels
text_color = (0, 0, 255) # red
font_size = 1
font_thickness = 1
classification_results_to_show = 3
fps_avg_frame_count = 10
keypoint_detection_threshold_for_classifier = 0.1
classifier = None
# 직접 학습시킨 pose estimation model 실행 결과 초기값
estimation_result = ''
# pose estimation 결과의 신뢰도가 특정값 이상일 경우 check
estimation_check = 0
# 이미지를 저장할 때 파일명을 다르게 하기 위해 사용하는 변수
button_key = 0
prev_button_key = 0
# model 실행, 이미지 처리가 완료됐는지 check
while_check = 0
# 이미지 저장 여부
capture_check = 0
# 배경 제거 이후 대신 사용할 배경 이미지
background_image = cv2.imread('backgroundimage.jpeg')
background_image = cv2.resize(background_image, (width, height))
# Initialize the classification model (직접 학습시킨 model)
if classification_model:
  classifier = Classifier(classification_model, label_file)
  classification_results_to_show = min(classification_results_to_show,
  len(classifier.pose_class_names))
def send(sock):
  global estimation_result, estimation_check, while_check
  while True:
    if(while_check==1):
      if(estimation_check==1): # estimation 결과 신뢰도가 특정값 이상일 경우
        if(estimation_result=='flower'): # estimation 결과가 'flower' 포즈일 경우
          sock.send(str(0).encode('utf-8')
        elif(estimation_result=='heart'): # estimation 결과가 'heart' 포즈일 경우
          sock.send(str(1).encode('utf-8'))
        elif(estimation_result=='superman'): # estimation 결과가 'superman' 포즈일 경우
          sock.send(str(2).encode('utf-8'))
          estimation_check=0
      else:
        sock.send(str(3).encode('utf-8'))
      while_check = 0
def receive(sock):
  global button_key, while_check, capture_check
  while True:
    if(capture_check==0):
      data = sock.recv(65535).decode()
      if(data): # 사용자가 버튼을 눌렀을 경우
        capture_check = 1 # 이미지를 저장하기 위한 변수를 1로 변경
        button_key += 1
sender = threading.Thread(target=send, args=(sock,))
receiver = threading.Thread(target=receive, args=(sock,))
sender. start()
receiver. start()
# grabcut을 하기 위한 기본값
rect = [0,0,1,1]
bgdmodel = np.zeros((1,65),np.float64)
fgdmodel = np.zeros((1,65),np.float64)
########## for flower effect #############
img_fg = cv2.imread('flower_effect.png', cv2.IMREAD_UNCHANGED) # effect에 사용할 4채널 png 파일
img_fg = cv2.resize(img_fg, (int(680*0.9), int(480*0.9))) # 사이즈 조절
_, mask = cv2.threshold(img_fg[:,:,3], 1, 255, cv2.THRESH_BINARY) # 마스크, 역마스크 생성
mask_inv = cv2.bitwise_not(mask)
img_fg = cv2.cvtColor(img_fg, cv2.COLOR_BGRA2BGR)
h, w = img_fg.shape[:2]
masked_fg = cv2.bitwise_and(img_fg, img_fg, mask=mask)
########## for heart effect #############
img_fg2 = cv2.imread('heart_effect.png', cv2.IMREAD_UNCHANGED) # effect에 사용할 4채널 png 파일
img_fg2 = cv2.resize(img_fg2, (int(680*0.9), int(480*0.9))) # 사이즈 조절
_, mask2 = cv2.threshold(img_fg2[:,:,3], 1, 255, cv2.THRESH_BINARY) # 마스크, 역마스크 생성
mask_inv2 = cv2.bitwise_not(mask2)
img_fg2 = cv2.cvtColor(img_fg2, cv2.COLOR_BGRA2BGR)
masked_fg2 = cv2.bitwise_and(img_fg2, img_fg2, mask=mask2)
########## for light effect #############
img_fg3 = cv2.imread('light_effect.png', cv2.IMREAD_UNCHANGED) # effect에 사용할 4채널 png 파일
img_fg3 = cv2.resize(img_fg3, (int(680*0.9), int(480*0.9))) # 사이즈 조절
_, mask3 = cv2.threshold(img_fg3[:,:,3], 1, 255, cv2.THRESH_BINARY) # 마스크, 역마스크 생성
mask_inv3 = cv2.bitwise_not(mask3)
img_fg3 = cv2.cvtColor(img_fg3, cv2.COLOR_BGRA2BGR)
masked_fg3 = cv2.bitwise_and(img_fg3, img_fg3, mask=mask3)
#########################################
while cap.isOpened():
  success, image = cap.read()
  if not success: # 이미지 캡쳐에 실패했을 경우
    sys.exit(
    'ERROR: Unable to read from webcam. Please verify your webcam settings.'
  )
  image = cv2.flip(image, 1)
  # movenet 실행 결과
  list_persons = [pose_detector.detect(image)]
  ######## grabcut 동작을 위해 추가한 부분 ##########
  height, width, _ = image.shape
  image2 = np.ones((height, width), dtype = np.uint8)*cv2.GC_PR_BGD # 기본 마스크
  image2= utils.visualize(image2, list_persons) # movenet 실행 결과를 이용해 마스크 변경
  # 생성한 마스크를 이용해 grabcut
  cv2.grabCut(image, image2, tuple(rect), bgdmodel, fgdmodel, 1, cv2.GC_INIT_WITH_MASK)
  # grabcut 실행 결과를 이용해 배경 제거 및 대체
  image[(image2==cv2.GC_BGD) | (image2==cv2.GC_PR_BGD)] = background_image[(image2==cv2.GC_BGD)
  | (image2==cv2.GC_PR_BGD)]
  ################################################
  if classifier:
    person = list_persons[0] # single pose이므로 0번째 사람만 고려
    min_score = min([keypoint.score for keypoint in person.keypoints])
    if min_score < keypoint_detection_threshold_for_classifier:
      error_text = 'Some keypoints are not detected.'
      text_location = (left_margin, 2 * row_size)
      cv2.putText(image, error_text, text_location, cv2.FONT_HERSHEY_PLAIN,
      font_size, text_color, font_thickness)
      error_text = 'Make sure the person is fully visible in the camera.'
      text_location = (left_margin, 3 * row_size)
      cv2.putText(image, error_text, text_location, cv2.FONT_HERSHEY_PLAIN,
      font_size, text_color, font_thickness)
  else:
    # Run pose classification
    prob_list = classifier.classify_pose(person)
    # Show classification results on the image
    for i in range(classification_results_to_show): # pose estimation 결과 (각각의 label)
      class_name = prob_list[i].label
      probability = round(prob_list[i].score, 2)
      result_text = class_name + ' (' + str(probability) + ')'
      text_location = (left_margin, (i + 2) * row_size)
      cv2.putText(image, result_text, text_location, cv2.FONT_HERSHEY_PLAIN,
      font_size, text_color, font_thickness)
      if(i==0): # pose classification 결과 (신뢰도가 가장 높은 label)
        if(probability>0.8): # 신뢰도가 0.8 이상인 경우
          estimation_result = class_name # pose classification 결과의 label
          text_location = (left_margin, (7 + 2) * row_size)
          cv2.putText(image, result_text, text_location, cv2.FONT_HERSHEY_PLAIN,
          font_size, text_color, font_thickness)
          estimation_check=1
          ### add effect image (효과 이미지 합성)###
          if(estimation_result == 'flower'):
            roi = image[0:0+h, 10:10+w ]
            masked_bg = cv2.bitwise_and(roi, roi, mask=mask_inv)
            added = masked_fg + masked_bg
            image[0:0+h, 10:10+w] = added
          elif(estimation_result == 'heart'):
            roi = image[0:0+h, 10:10+w ]
            masked_bg = cv2.bitwise_and(roi, roi, mask=mask_inv2)
            added = masked_fg2 + masked_bg
            image[0:0+h, 10:10+w] = added
          elif(estimation_result == 'superman'):
            roi = image[0:0+h, 10:10+w ]
            masked_bg = cv2.bitwise_and(roi, roi, mask=mask_inv3)
            added = masked_fg3 + masked_bg
            image[0:0+h, 10:10+w] = added
  # Stop the program if the ESC key is pressed.
  if cv2.waitKey(1) == 27:
    break
  #image= utils2.visualize(image, list_persons) # 뼈대를 시각적으로 보기 위해 임시로 추가한 부분
  cv2.imshow(estimation_model, image)
  if(capture_check == 1):
    cv2.imwrite('result'+str(button_key)+'.png', image) # 버튼이 눌린 경우 현재 이미지 저장
    print('success capture')  
    capture_check = 0
  while_check=1
  cap.release()
  cv2.destroyAllWindows()
