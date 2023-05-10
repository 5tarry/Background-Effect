import math
from typing import List, Tuple
import cv2
from data import Person
import numpy as np
import math
# 각 edge의 색깔 (해당 코드의 경우 edge 번호만 사용하고 색깔은 사용하지 않을 예정)
KEYPOINT_EDGE_INDS_TO_COLOR = {
  (0, 1): (147, 20, 255),
  (0, 2): (255, 255, 0),
  (1, 3): (147, 20, 255),
  (2, 4): (255, 255, 0),
  (0, 5): (147, 20, 255),
  (0, 6): (255, 255, 0),
  (5, 7): (147, 20, 255),
  (7, 9): (147, 20, 255),
  (6, 8): (255, 255, 0),
  (8, 10): (255, 255, 0),
  (5, 6): (0, 255, 255),
  (5, 11): (147, 20, 255),
  (6, 12): (255, 255, 0),
  (11, 12): (0, 255, 255),
  (11, 13): (147, 20, 255),
  (13, 15): (147, 20, 255),
  (12, 14): (255, 255, 0),
  (14, 16): (255, 255, 0)
}
def visualize(
  image: np.ndarray,
  list_persons: List[Person],
  keypoint_color: Tuple[int, ...] = None,
  keypoint_threshold: float = 0.05,
  instance_threshold: float = 0.1,
) -> np.ndarray:
  # 마스크 이미지 생성을 위해 landmark 및 edge 이용
  for person in list_persons:
    if person.score < instance_threshold:
      continue
    keypoints = person.keypoints
    bounding_box = person.bounding_box
    # movenet 결과 추출된 landmark들을 mask 이미지에서 Foreground로 지정
    for i in range(len(keypoints)):
      if keypoints[i].score >= keypoint_threshold:
        cv2.circle(image, keypoints[i].coordinate, 2, cv2.GC_FGD, 4)
    # movenet은 눈 위로 landmark를 고려하지 않기 때문에, 배경 제거를 원활히 하기 위하여
    # 귀의 landmark와 코의 landmark를 이용해서 머리 크기를 잡은 뒤, Prob Foreground로 지정
    if (keypoints[0].score >= keypoint_threshold and keypoints[3].score >= keypoint_threshold):
      radius = math.sqrt((keypoints[3].coordinate[1] - keypoints[0].coordinate[1])*(keypoints[3].coordinate[1] - keypoints[0].coordinate[1])+(keypoints[3].coordinate[0] - keypoints[0].coordinate[0])*(keypoints[3].coordinate[0] - keypoints[0].coordinate[0]))
      cv2.circle(image, keypoints[0].coordinate, int(radius), cv2.GC_FGD, -1)
      cv2.circle(image, (keypoints[0].coordinate[0], keypoints[1].coordinate[1]), int(radius), cv2.GC_PR_FGD, -1)
    # mask 이미지에서 edge에 해당하는 부분을 Foreground로 지정
    for edge_pair, edge_color in KEYPOINT_EDGE_INDS_TO_COLOR.items():
      if (keypoints[edge_pair[0]].score > keypoint_threshold and keypoints[edge_pair[1]].score > keypoint_threshold):
          cv2.line(image, keypoints[edge_pair[0]].coordinate, keypoints[edge_pair[1]].coordinate, cv2.GC_FGD, 10)
    # Draw bounding_box with multipose
    if bounding_box is not None:
      start_point = bounding_box.start_point
      end_point = bounding_box.end_point
  return image

def keep_aspect_ratio_resizer(
  image: np.ndarray, target_size: int) -> Tuple[np.ndarray, Tuple[int, int]]:
  # Resizes the image.
  height, width, _ = image.shape
  if height > width:
    scale = float(target_size / height)
    target_height = target_size
    scaled_width = math.ceil(width * scale)
    image = cv2.resize(image, (scaled_width, target_height))
    target_width = int(math.ceil(scaled_width / 32) * 32)
  else:
    scale = float(target_size / width)
    target_width = target_size
    scaled_height = math.ceil(height * scale)
    image = cv2.resize(image, (target_width, scaled_height))
    target_height = int(math.ceil(scaled_height / 32) * 32)
  padding_top, padding_left = 0, 0
  padding_bottom = target_height - image.shape[0]
  padding_right = target_width - image.shape[1]
  # add padding to image
  image = cv2.copyMakeBorder(image, padding_top, padding_bottom, padding_left,
  padding_right, cv2.BORDER_CONSTANT)
  return image, (target_height, target_width)
)
