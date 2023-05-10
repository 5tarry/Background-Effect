# Background-Effect
### 임베디드시스템 팀프로젝트 (자유주제)

## Title

Pose classification에 따른 다양한 Background effect 적용

## Abstract

원하는 pose 이미지에 MoveNet을 적용하여 인체의 특징점을 추출한 뒤, 추출된 특징점을 바탕으로 pose label과 함께 pose classification 학습을 진행하였다. 학습된 model을 이용하여 라즈베리파이의 입출력(카메라, led, push button, 7 segment) 및 OpenCV 등 다양한 동작을 구현하였다.

## 참고
### MoveNet
신체의 17개 키포인트를 감지하는 매우 빠르고 정확한 모델이며, 본 프로젝트에서 Pose Estimation 모델을 생성하기 위해 사용하였다.

### Pose Classification
[오픈소스](https://www.tensorflow.org/lite/tutorials/pose_classification?hl=ko)를 본 프로젝트에 맞게 수정하여, movenet 결과 감지된 pose landmarks를 3가지 pose로 학습시켜 keras model을 생성하였다.

### [Grabcut](https://en.wikipedia.org/wiki/GrabCut)
GrabCut은 그래프 컷을 기반으로 하는 이미지 분할 방법이다. OpenCV에서 GrabCut을 사용하기 위해선, foreground와 background를 구분하는 mask 이미지를 직접 설정해주어야 한다.
