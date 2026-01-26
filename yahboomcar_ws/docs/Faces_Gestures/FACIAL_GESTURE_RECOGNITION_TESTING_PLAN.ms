---
name: Facial Gesture Recognition Testing Plan
overview: Create a comprehensive testing system for facial and gesture recognition using LLM/VLM/MediaPipe/YOLOv8 with person tracking, working from 2-15 feet distance. Integrates existing MediaPipe face/hand detection, YOLOv8 person tracking, and VLM capabilities into a unified testing framework for Yahboom ROSMASTER X3PLUS 6DOF robot.
todos:
  - id: create-person-tracker
    content: Create person_tracker_node.py using YOLOv8 with tracking enabled, filtering for person class only, and publishing tracked persons with persistent IDs
    status: pending
  - id: create-custom-messages
    content: Create TrackedPerson.msg, TrackedPersons.msg, FaceDetection.msg, and GestureDetection.msg custom message types
    status: pending
  - id: create-face-detection
    content: Create face detection node that subscribes to person tracker, crops person regions, and applies MediaPipe face detection with person ID association
    status: pending
    dependencies:
      - create-person-tracker
      - create-custom-messages
  - id: create-gesture-recognition
    content: Create gesture recognition node that subscribes to person tracker, crops person regions, and applies MediaPipe hand detection with gesture classification
    status: pending
    dependencies:
      - create-person-tracker
      - create-custom-messages
  - id: create-vlm-integration
    content: Create VLM analysis node that integrates with agent_process service to provide natural language descriptions of faces and gestures
    status: pending
    dependencies:
      - create-face-detection
      - create-gesture-recognition
  - id: create-test-framework
    content: Create unified test node that combines all detections, provides visualization, logging, and metrics calculation
    status: pending
    dependencies:
      - create-face-detection
      - create-gesture-recognition
      - create-vlm-integration
  - id: create-launch-file
    content: Create launch file to start all nodes in correct order for testing
    status: pending
    dependencies:
      - create-test-framework
  - id: test-distance-range
    content: Test system at 2, 5, 10, and 15 feet distances and verify detection accuracy and tracking persistence
    status: pending
    dependencies:
      - create-launch-file
  - id: create-x3plus-tracking-flow-doc
    content: Create Yahboom_X3PLUS_Object_Tracking_Flow.md document documenting the complete flow from camera to robot arm movement for X3PLUS system, including camera topics, depth integration, YOLOv8 integration, etc.
    status: pending
    dependencies:
      - test-distance-range
---

# Facial and Gesture Recognition Testing Plan

## Overview

This plan creates a comprehensive testing system that combines facial recognition, gesture recognition, and person tracking using multiple technologies (LLM/VLM/MediaPipe/YOLOv8) to work across a 2-15 foot distance range.

## Architecture

The system will integrate:

- **MediaPipe** for real-time face and hand gesture detection
- **YOLOv8** for person detection and tracking with persistent IDs
- **VLM/LLM** for enhanced scene understanding and natural language descriptions
- **Multi-modal fusion** to combine detections across technologies

## System Components

### 1. Person Tracking Node (YOLOv8-based)

**File:** `/home/jetson/yahboomcar_ws/src/yolov8_detect/yolov8_detect/person_tracker_node.py` (new)

**Functionality:**

- Use YOLOv8 with tracking enabled (`model.track()`) to detect and track persons
- Filter for person class only (`classes=[0]`)
- Maintain persistent track IDs across frames
- Publish person bounding boxes with track IDs
- Track person position history for trajectory analysis
- Handle distance-based detection (2-10 feet range with Astra Pro, depth range 0.15-3m)

**Key Features:**

- Subscribe to camera topic: `/color/image_raw` (Astra Pro depth camera - native to X3PLUS)
- Publish tracked persons: `~/tracked_persons` (custom message with boxes, IDs, confidence)
- Publish annotated image: `~/tracked_image`
- Use `persist=True` for continuous tracking
- Store track history per person ID

**Reference:** Existing `yolov8_node.py` structure, but add tracking capability using `ultralytics` YOLO model with `.track()` method

### 2. Facial Recognition Node (MediaPipe-based)

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_visual/app/face_gesture_tracker.py` (new, or extend existing face tracking)

**Functionality:**

- Use MediaPipe Face Detection for face detection
- Detect faces within tracked person bounding boxes (crop person region first)
- Extract facial landmarks for expression analysis
- Associate face detections with person track IDs
- Handle multiple faces per person (if applicable)
- Work at varying distances (adjust detection confidence thresholds)

**Key Features:**

- Subscribe to: `person_tracker/tracked_persons` and camera image (`/color/image_raw` - Astra Pro)
- Crop person regions from full image before face detection
- Publish face detections: `~/face_detections` (with associated person ID)
- Use existing `mp.solutions.face_detection.FaceDetection` from `face_tracker.py`
- Adjust `min_detection_confidence` based on distance (lower for far, higher for close)

**Reference:** Existing `face_tracker.py` and `face_tracking.py` implementations

### 3. Gesture Recognition Node (MediaPipe-based)

**File:** `/home/ubuntu/ros2_ws/src/app/app/face_gesture_tracker.py` (integrated with face tracker)

**Functionality:**

- Use MediaPipe Hands for hand gesture detection
- Detect hands within tracked person bounding boxes
- Recognize gestures: fist, gun, hand_heart, one, two, three, OK, four, five, six, none
- **Additional gestures available in action groups** (may need recognition logic added): give, give3, give9, give_2
- Associate gestures with person track IDs
- Handle distance-based gesture recognition (may need to crop/zoom person region)

**Key Features:**

- Subscribe to: `person_tracker/tracked_persons` and camera image (`/color/image_raw` - Astra Pro)
- Crop person regions and detect hands within those regions
- Use existing gesture recognition logic from `finger_trace.py`
- Publish gestures: `~/gesture_detections` (gesture type + person ID)
- Adjust `min_detection_confidence` and `min_tracking_confidence` for distance

**Reference:** Existing `finger_trace.py` and `hand_gesture.py` implementations with `h_gesture()` function

**Note:** Action groups directory (if applicable) may contain additional gesture action files:
- Currently recognized: `fist.d6a`, `gun.d6a`, `hand_heart.d6a`, `one.d6a`, `two.d6a`, `three.d6a`, `OK.d6a`, `four.d6a`, `five.d6a`, `six.d6a`
- Available but not yet recognized: `give.d6a`, `give3.d6a`, `give9.d6a`, `give_2.d6a`
- May need to extend `h_gesture()` function to recognize "give" gestures (checking thumb and finger positions for giving/handoff gesture)

### 4. VLM/LLM Integration Node

**File:** `/home/jetson/yahboomcar_ws/src/large_models/large_models/vllm_face_gesture_analysis.py` (new, to be migrated from HiWonder Ultra)

**Functionality:**

- Use VLM to analyze detected faces and gestures
- Provide natural language descriptions of person actions
- Answer queries about detected persons, faces, and gestures
- Enhance detection confidence with semantic understanding
- Optional: Use LLM for reasoning about person behavior

**Key Features:**

- Subscribe to: face detections, gesture detections, tracked persons, camera image
- Use existing `agent_process/set_vllm_content` service pattern
- Publish VLM descriptions: `~/vllm_analysis`
- Integrate with existing VLM infrastructure (`vllm_with_camera.py`, `vllm_track.py`)
- Use prompts to describe: "What gestures is the person making?", "Describe the person's facial expression"

**Reference:** Existing `vllm_track.py` and `vllm_with_camera.py` patterns

### 5. Unified Testing Node

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_visual/app/face_gesture_test_node.py` (new)

**Functionality:**

- Combine all detections (person, face, gesture, VLM analysis)
- Visualize all detections on single annotated image
- Log test results with timestamps
- Provide test metrics (detection rate, accuracy, distance performance)
- Handle distance-based testing scenarios

**Key Features:**

- Subscribe to all detection topics
- Create comprehensive visualization with:
  - Person bounding boxes with track IDs
  - Face bounding boxes
  - Hand landmarks and gesture labels
  - VLM descriptions (optional overlay)
- Publish combined results: `~/test_results`
- Log to file: `/home/jetson/yahboomcar_ws/logs/face_gesture_test_<timestamp>.log`
- Calculate metrics: detection rate, false positives, distance performance

## Message Types

### New Custom Messages

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_msgs/msg/TrackedPerson.msg`

```
uint32 track_id
float32[] bbox  # [x1, y1, x2, y2]
float32 confidence
float32 center_x
float32 center_y
```

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_msgs/msg/TrackedPersons.msg`

```
TrackedPerson[] persons
std_msgs/Header header
```

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_msgs/msg/FaceDetection.msg`

```
uint32 person_track_id
float32[] bbox  # [x1, y1, x2, y2]
float32 confidence
float32[] landmarks  # key facial landmarks
```

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_msgs/msg/GestureDetection.msg`

```
uint32 person_track_id
string gesture_type  # "fist", "gun", "one", "two", "three", "OK", "four", "five", "six", "hand_heart", "give", "give3", "give9", "give_2", "none"
float32 confidence
float32[] hand_landmarks
```

**Supported Gesture Types:**
- **Basic gestures:** fist, gun, hand_heart, one, two, three, OK, four, five, six, none
- **Extended gestures (action groups available):** give, give3, give9, give_2 (may require additional recognition logic)

## Implementation Steps

### Phase 1: Person Tracking with YOLOv8

1. **Create person tracker node**

   - Extend `yolov8_node.py` to use Ultralytics YOLO with `.track()` method
   - Implement person class filtering (`classes=[0]`)
   - Add track ID persistence
   - Test with camera feed at various distances

2. **Create custom messages**

   - Define `TrackedPerson.msg` and `TrackedPersons.msg`
   - Update `yahboomcar_msgs` package CMakeLists.txt and package.xml
   - Build and verify message compilation

3. **Test person tracking**

   - Verify tracking works at 2, 5, 10, 15 feet
   - Verify track IDs persist across frames
   - Measure FPS and CPU usage

### Phase 2: Face Detection Integration

1. **Create face detection node**

   - Subscribe to person tracker output
   - Crop person regions from full image
   - Apply MediaPipe face detection to cropped regions
   - Associate faces with person track IDs

2. **Test face detection**

   - Test at various distances (2-15 feet)
   - Verify face detection within person bounding boxes
   - Adjust confidence thresholds for distance

### Phase 3: Gesture Recognition Integration

1. **Create gesture recognition node**

   - Subscribe to person tracker output
   - Crop person regions (focus on upper body/hands area)
   - Apply MediaPipe Hands detection
   - Use existing gesture recognition logic (`h_gesture()`)

2. **Test gesture recognition**

   - Test all gesture types at various distances:
     - Basic gestures: fist, gun, hand_heart, one, two, three, OK, four, five, six
     - Extended gestures: give, give3, give9, give_2 (if recognition logic added)
   - Verify gesture recognition within person bounding boxes
   - Handle cases where hands are not visible
   - Test gesture-to-action mapping (verify action groups execute correctly)

### Phase 4: VLM/LLM Integration

1. **Create VLM analysis node**

   - Integrate with existing `agent_process` service
   - Send person crops with faces/gestures to VLM
   - Generate natural language descriptions
   - Optional: Use LLM for behavior reasoning

2. **Test VLM integration**

   - Verify VLM can describe detected faces and gestures
   - Test query responses about person actions
   - Measure latency and accuracy

### Phase 5: Unified Testing Framework

1. **Create test node**

   - Combine all detection streams
   - Create comprehensive visualization
   - Implement logging and metrics

2. **Test scenarios**

   - Single person, 2 feet away: face + gesture detection (✅ Expected: Reliable)
   - Single person, 5 feet away: face + gesture detection (✅ Expected: Reliable)
   - Single person, 10 feet away: face + gesture detection (⚠️ Expected: Face marginal, gesture difficult)
   - Single person, 15 feet away: face + gesture detection (❌ Expected: Face unreliable, gesture likely impossible)
   - Single person, 20 feet away: person detection only (❌ Expected: No face/gesture, person tracking only)
   - Person walking around: tracking persistence
   - Multiple persons: track ID separation
   - Occlusion handling: person temporarily hidden

**Note:** Astra Pro camera (640x480 @ 30fps, depth range 0.15-3m) limitations mean 15-20 feet face/gesture detection is likely not achievable. Consider higher resolution camera upgrade for long-range applications.

### Phase 6: Documentation

1. **Create Ultra Object Tracking Flow Document**

   - Document complete flow from camera to robot arm movement for X3PLUS system
   - Include camera topics (`/color/image_raw`, `/depth/image_raw` - Astra Pro)
     - Depth camera integration (Astra Pro - native support)
     - YOLOv8 integration for object detection
     - VLM/LLM integration capabilities (requires Jetson Orin NX SUPER or 16GB RPi5)
     - 6DOF bus servo robotic arm control
   - Document all ROS2 topics, services, and message types
   - Include step-by-step flow similar to FPV document
   - Add performance metrics and testing status
   - Reference existing tracking flow documentation as template

2. **Documentation Structure**

   - Include status updates and milestones
   - Document technical fixes and optimizations
   - Add hardware-specific details (Astra Pro camera, depth sensing)
   - Include integration with YOLOv8 and AI models
   - Document Jetson/Orin/RPi5 hardware differences

## Distance-Based Configuration

### Camera Comparison for 15-20 Feet Detection

#### Astra Pro (Yahboom X3PLUS Native Camera) - ⚠️ Limited Range

**Camera Specifications:**
- **Resolution:** 640x480 pixels @ 30fps
- **FOV:** ~60-70° (typical for Astra Pro)
- **Depth Range:** 0.15-3m (0.5-10 feet) - **NOT sufficient for 15-20 feet depth sensing**
- **Onboard AI:** ❌ No
- **Cost:** Included with X3PLUS

**At 20 feet distance:**
- **Person size in image:** ~160 pixels tall (6ft person) - **Better than Aurora 930, but limited**
- **Face size in image:** ~22 pixels tall (8 inch face) - **BELOW MediaPipe minimum (~50-80 pixels)**
- **Hand size in image:** ~11 pixels tall (4 inch hand) - **BELOW MediaPipe minimum (~30-50 pixels)**

**Verdict:** ⚠️ **Astra Pro CAN detect persons at 10-15 feet, but face/gesture detection at 15-20 feet is NOT RELIABLE**

**Advantages:**
- ✅ Native integration with X3PLUS
- ✅ Depth sensing (0.15-3m range)
- ✅ Good FPS (30fps)
- ✅ Point cloud support (`/depth/points`)

**Limitations:**
- ⚠️ Face detection at 15-20 feet: ~22 pixels (below MediaPipe minimum)
- ⚠️ Hand detection at 15-20 feet: ~11 pixels (below MediaPipe minimum)
- ❌ No onboard AI (CPU inference only)
- ⚠️ Limited depth range (3m max)

**Best for:** Person detection at 2-10 feet, face/gesture detection at 2-5 feet (reliable), 5-10 feet (marginal)

### Recommended Camera for 15-20 Feet Detection

**Camera Status for X3PLUS:**

**Current Camera: Astra Pro (Native)**
- ⚠️ **Limited range for 15-20 feet face/gesture detection**
- ✅ **Good for 2-10 feet range** (person, face, gesture detection)
- ⚠️ **10-15 feet range:** Person detection works, face/gesture marginal
- ❌ **15-20 feet range:** Person detection only, face/gesture not reliable

**Summary:**
- **For 2-10 feet face/gesture recognition:** ✅ **Astra Pro is sufficient**
- **For 10-15 feet:** ⚠️ **Astra Pro person detection works, face/gesture marginal**
- **For 15-20 feet:** ❌ **Astra Pro insufficient for face/gesture, person detection only**
- **Note:** X3PLUS comes with Astra Pro. For 15-20 feet face/gesture, consider upgrading to higher resolution camera (OAK-D Lite/S2 or similar)

### Detection Confidence Thresholds

**Note:** Thresholds differ based on camera used (Aurora 930 vs OAK-D Lite)

#### With Astra Pro (640x480) - Limited Range

**Close Range (2-5 feet):**
- Face detection: `min_detection_confidence=0.5`
- Hand detection: `min_detection_confidence=0.5`
- Person detection: `conf_threshold=0.3`
- **Status:** ✅ Reliable detection

**Medium Range (5-10 feet):**
- Face detection: `min_detection_confidence=0.4`
- Hand detection: `min_detection_confidence=0.4`
- Person detection: `conf_threshold=0.25`
- **Status:** ⚠️ Face detection marginal, hand detection difficult

**Far Range (10-15 feet):**
- Face detection: `min_detection_confidence=0.3`
- Hand detection: `min_detection_confidence=0.3`
- Person detection: `conf_threshold=0.2`
- **Status:** ⚠️ Face detection unreliable, hand detection very difficult

**Very Far Range (15-20 feet):**
- Face detection: `min_detection_confidence=0.2` (likely won't work)
- Hand detection: `min_detection_confidence=0.2` (likely won't work)
- Person detection: `conf_threshold=0.15`
- **Status:** ❌ Face/hand detection likely impossible, person detection only

#### With Higher Resolution Camera (OAK-D Lite/S2 or similar) - Extended Range ✅

**Note:** X3PLUS comes with Astra Pro. For 15-20 feet detection, consider upgrading to OAK-D Lite/S2 or similar high-resolution camera.

**Close Range (2-5 feet):**
- Face detection: `min_detection_confidence=0.5`
- Hand detection: `min_detection_confidence=0.5`
- Person detection: `conf_threshold=0.3`
- **Status:** ✅ Reliable detection

**Medium Range (5-10 feet):**
- Face detection: `min_detection_confidence=0.4`
- Hand detection: `min_detection_confidence=0.4`
- Person detection: `conf_threshold=0.25`
- **Status:** ✅ Reliable detection

**Far Range (10-15 feet):**
- Face detection: `min_detection_confidence=0.3`
- Hand detection: `min_detection_confidence=0.3`
- Person detection: `conf_threshold=0.2`
- **Status:** ✅ Face detection reliable, hand detection possible

**Very Far Range (15-20 feet):**
- Face detection: `min_detection_confidence=0.3` (1080p) or `0.4` (4K)
- Hand detection: `min_detection_confidence=0.3` (1080p) or `0.4` (4K)
- Person detection: `conf_threshold=0.2`
- **Status:** ✅ Face detection reliable (4K) or marginal (1080p), hand detection possible (4K) or difficult (1080p)

### Image Processing for Distance

#### With Astra Pro (640x480):
- **Close range (2-5 feet):** Use full resolution, minimal cropping
- **Medium range (5-10 feet):** Crop person region, maintain resolution
- **Far range (10-15 feet):** Crop person region, upscale 2x for face/hand detection
- **Very far range (15-20 feet):** Crop person region, upscale 3-4x (may still be insufficient for reliable face/gesture)

#### With Higher Resolution Camera (OAK-D Lite/S2 or similar):
- **Close range (2-5 feet):** Use full resolution (1080p or 4K), minimal cropping
- **Medium range (5-10 feet):** Crop person region, maintain resolution (sufficient pixels)
- **Far range (10-15 feet):** Crop person region, maintain resolution or slight upscale (1080p) or no upscale needed (4K)
- **Very far range (15-20 feet):** Crop person region, maintain resolution (4K) or upscale 1.5-2x (1080p)

## Testing Metrics

1. **Detection Rate:** Percentage of frames with successful detections
2. **Tracking Persistence:** Average track duration before ID loss
3. **Distance Performance:** Detection success rate by distance
4. **Latency:** End-to-end processing time per frame
5. **Accuracy:** Gesture recognition accuracy by distance
6. **FPS:** Frame processing rate

## Launch File

**File:** `/home/jetson/yahboomcar_ws/src/yahboomcar_visual/launch/face_gesture_test.launch.py`

Launch all nodes in correct order:

1. Camera node
2. Person tracker node
3. Face detection node
4. Gesture recognition node
5. VLM analysis node (optional)
6. Test visualization node

## Hardware Resource Requirements

### System Resource Usage (Jetson NANO 4GB/ORIN NX SUPER/ORIN NANO SUPER/Raspberry Pi 5)

**Base System (YOLOv8 Person Tracking + MediaPipe):**
```
✅ YOLOv8 Person Tracking:  ~2GB RAM, 40% CPU (RPi5 CPU) or ~1GB RAM, 20% CPU (Jetson/Orin TensorRT)
✅ MediaPipe Face Detection: ~200MB RAM, 15% CPU
✅ MediaPipe Hand Detection: ~200MB RAM, 15% CPU
✅ ROS2 nodes:              ~1GB RAM, 10% CPU
✅ System:                   ~1GB RAM
─────────────────────────────────────────────
Total (RPi5):               ~4.4GB RAM, 80% CPU
Total (Jetson/Orin):        ~3.4GB RAM, 60% CPU
Margin:                     Varies by board
```

**With VLM Integration (BLIP-2):**
```
✅ YOLOv8 Person Tracking:  ~2GB RAM, 40% CPU
✅ MediaPipe (Face+Hand):   ~400MB RAM, 30% CPU
✅ ROS2 nodes:              ~1GB RAM, 10% CPU
⚠️ BLIP-2 VLM:              ~4GB RAM, 30% CPU
✅ System:                   ~1GB RAM
─────────────────────────────────────────────
Total:                      ~8.4GB RAM, 110% CPU
Margin:                     NEGATIVE! (Tight fit)
```

**Verdict:**
- ✅ **Base system (YOLOv8 + MediaPipe) works on 8GB RPi5**
- ⚠️ **VLM integration requires 16GB RPi5 or careful optimization**
- ✅ **Python 3.8 compatible** (YOLOv8, MediaPipe, PyTorch all support 3.8)

### Performance Targets

- **Person Tracking:** ≥10 FPS (YOLOv8 with tracking)
- **Face Detection:** ≥15 FPS (MediaPipe on cropped regions)
- **Gesture Recognition:** ≥15 FPS (MediaPipe on cropped regions)
- **End-to-End Latency:** <100ms per frame
- **CPU Usage:** <80% total system load
- **Memory Usage:** <5GB RAM for base system

### Optimization Strategies

1. **YOLOv8 Optimization:**
   - Use OpenVINO models (`.xml` format) for faster inference
   - Consider YOLOv8n (nano) model for lower resource usage
   - Use `persist=True` for tracking to maintain IDs efficiently
   - Filter for person class only (`classes=[0]`) to reduce processing

2. **MediaPipe Optimization:**
   - Crop person regions before face/hand detection (reduces processing area)
   - Adjust confidence thresholds based on distance
   - Use lower resolution for far-range detection
   - Process face and hand detection in parallel threads

3. **VLM Optimization (if enabled):**
   - Use BLIP-2 (2GB model) instead of larger models
   - Process VLM queries asynchronously (not every frame)
   - Cache VLM responses for similar scenes
   - Consider cloud-based VLM API for heavy workloads

## Dependencies

### Python Packages

- `ultralytics` (for YOLOv8 tracking) - Python 3.10+ compatible
- `openvino` (for YOLOv8 optimization) - Python 3.10+ compatible
- `mediapipe` (for face and hand detection) - Python 3.10+ compatible
- `opencv-python` (for image processing)
- `cv_bridge` (for ROS2 image conversion)
- `transformers` (for VLM integration, if enabled) - Python 3.10+ compatible
- `torch` (for VLM models, if enabled) - Python 3.10+ compatible
- `tensorrt` (for Jetson/Orin acceleration, optional) - Python 3.10+ compatible

### ROS2 Packages

- `large_models` package (for VLM integration via agent_process service, to be migrated from HiWonder Ultra)
- `yahboomcar_msgs` package (for custom messages)
- `sensor_msgs` (for Image messages)
- `std_msgs` (for String and other standard messages)

### Installation Commands

```bash
# Core dependencies
pip3 install ultralytics openvino mediapipe opencv-python

# VLM dependencies (optional, only if using VLM)
pip3 install transformers torch pillow

# ROS2 packages (build with colcon)
cd ~/yahboomcar_ws
colcon build --packages-select yolov8_detect yahboomcar_msgs large_models
```

## YOLOv8 Tracking Implementation Details

### Tracking Method

Use Ultralytics YOLO with built-in tracking:
```python
from ultralytics import YOLO

# Load model
model = YOLO('yolov8n.pt')  # or yolov8n.xml for OpenVINO

# Track with person class filtering
results = model.track(
    source=frame,
    persist=True,           # Maintain track IDs across frames
    classes=[0],            # Person class only
    conf=0.25,             # Confidence threshold
    iou=0.45               # IoU threshold for NMS
)

# Extract tracking information
if results[0].boxes.id is not None:
    boxes = results[0].boxes.xyxy.cpu().numpy()
    track_ids = results[0].boxes.id.cpu().numpy()
    confidences = results[0].boxes.conf.cpu().numpy()
```

### Track ID Persistence

- Track IDs are maintained using BYTETracker (default in Ultralytics)
- IDs persist across frames even with temporary occlusions
- Track history can be stored per ID for trajectory analysis
- Maximum track age can be configured to handle lost tracks

### OpenVINO Model Optimization

For better performance on RPi5:
1. Convert PyTorch model to OpenVINO format:
   ```bash
   pip install openvino-dev
   mo --input_model yolov8n.onnx --output_dir models/
   ```
2. Use `.xml` model files instead of `.pt` files
3. Expected performance: 30-40% CPU vs 40-50% CPU with PyTorch

## VLM Integration Details

### Model Selection

**Recommended: BLIP-2 (Salesforce/blip2-opt-2.7b)**
- Size: 2GB
- RAM Required: 4GB
- Inference Time: ~500ms
- Accuracy: Good for robotics use cases
- Python 3.8 Compatible: ✅ Yes

**Alternative Options:**
- CLIP (350MB, 2GB RAM) - Faster but less accurate
- LLaVA-7B (7GB, 10GB RAM) - Better accuracy but too large for 8GB RPi5

### Integration Pattern

Use existing `agent_process/set_vllm_content` service pattern:
```python
from large_models_msgs.srv import SetContent

# Create service client
vllm_client = self.create_client(SetContent, 'agent_process/set_vllm_content')

# Prepare request
msg = SetContent.Request()
msg.model_type = 'vllm'
msg.query = "What gestures is the person making?"
msg.image = cv_bridge.cv2_to_imgmsg(person_crop, "bgr8")
msg.prompt = "Describe the person's gestures and facial expression."
msg.model = 'qwen-vl-max-latest'  # or other VLM model

# Call service
response = self.send_request(vllm_client, msg)
vlm_description = response.message
```

### VLM Usage Recommendations

- **Frequency:** Process VLM queries every 5-10 frames (not every frame)
- **Input:** Send cropped person regions (not full image) to reduce processing
- **Caching:** Cache VLM responses for similar scenes to reduce API calls
- **Fallback:** Make VLM optional - system should work without it

## Files to Create/Modify

**New Files:**

- `yolov8_detect/yolov8_detect/person_tracker_node.py`
- `yahboomcar_visual/app/face_gesture_tracker.py`
- `large_models/large_models/vllm_face_gesture_analysis.py` (to be migrated from HiWonder Ultra)
- `yahboomcar_visual/app/face_gesture_test_node.py`
- `yahboomcar_msgs/msg/TrackedPerson.msg`
- `yahboomcar_msgs/msg/TrackedPersons.msg`
- `yahboomcar_msgs/msg/FaceDetection.msg`
- `yahboomcar_msgs/msg/GestureDetection.msg`
- `yahboomcar_visual/launch/face_gesture_test.launch.py`
- `docs/Faces_Gestures/Yahboom_X3PLUS_Object_Tracking_Flow.md` (documentation)

**Modified Files:**

- `yolov8_detect/package.xml` (add dependencies)
- `yolov8_detect/setup.py` (add node entry point)
- `yahboomcar_msgs/CMakeLists.txt` (add new messages)
- `yahboomcar_msgs/package.xml` (if needed)

## Success Criteria

1. Person tracking works reliably from 2-15 feet (✅ Expected with YOLOv8 + Astra Pro)
2. Face detection works within tracked person bounding boxes:
   - **Astra Pro:** ✅ 2-5 feet, ⚠️ 5-10 feet, ⚠️ 10-15 feet (marginal), ❌ 15-20 feet
   - **Higher resolution camera (if upgraded):** ✅ 2-20 feet (4K) or ✅ 2-15 feet, ⚠️ 15-20 feet (1080p)
3. Gesture recognition works within tracked person bounding boxes:
   - **Astra Pro:** ✅ 2-5 feet, ⚠️ 5-10 feet, ❌ 10-20 feet
   - **Higher resolution camera (if upgraded):** ✅ 2-15 feet (4K) or ✅ 2-10 feet, ⚠️ 10-15 feet (1080p)
4. Track IDs persist as person walks around
5. System processes at ≥10 FPS (person tracking) and ≥15 FPS (face/gesture detection)
6. CPU usage <80% on Jetson/Orin/RPi5
7. Memory usage <5GB RAM for base system (Jetson NANO 4GB may be tight)
8. VLM can describe detected faces and gestures (if enabled, requires Jetson Orin NX SUPER or 16GB RPi5)
9. Test framework provides comprehensive logging and visualization
10. System remains stable for extended testing sessions (30+ minutes)

**⚠️ IMPORTANT:** Camera selection affects detection range:

**Astra Pro (640x480 resolution) - Native to X3PLUS:**
- **Realistic range for face detection:** 2-5 feet (reliable), 5-10 feet (marginal), 10-15 feet (marginal/unreliable), 15-20 feet (unreliable)
- **Realistic range for gesture detection:** 2-5 feet (reliable), 5-10 feet (marginal), 10-20 feet (unreliable)
- **15-20 feet range:** Person detection works (YOLOv8), face detection unreliable, gesture detection likely impossible
- **Note:** X3PLUS comes with Astra Pro. For 15-20 feet face/gesture, consider upgrading to higher resolution camera.

**Higher Resolution Camera (OAK-D Lite/S2 or similar) - Optional Upgrade:**
- **Realistic range for face detection:** 2-20 feet (reliable at 4K, marginal at 1080p for 15-20 feet)
- **Realistic range for gesture detection:** 2-15 feet (reliable at 4K), 2-10 feet (reliable at 1080p)
- **15-20 feet range:** ✅ Face detection reliable (4K) or marginal (1080p), gesture detection possible (4K)
- **Recommendation:** ✅ **Higher resolution camera can handle 15-20 feet detection requirements, especially at 4K resolution**

## Performance Benchmarks

### Expected Performance (Jetson NANO 4GB/ORIN NX SUPER/ORIN NANO SUPER/Raspberry Pi 5)

| Component | FPS | CPU % | RAM (MB) | Notes |
|-----------|-----|-------|----------|-------|
| YOLOv8 Person Tracking | 10-15 | 40% (RPi5) / 20% (Jetson/Orin) | 2000 (RPi5) / 1000 (Jetson/Orin) | OpenVINO/TensorRT optimized |
| MediaPipe Face Detection | 15-20 | 15% | 200 | On cropped regions |
| MediaPipe Hand Detection | 15-20 | 15% | 200 | On cropped regions |
| ROS2 Nodes | - | 10% | 1000 | All nodes combined |
| System Overhead | - | 10% | 1000 | OS and other processes |
| **Total (RPi5)** | **10-15** | **90%** | **4400** | **Base system** |
| **Total (Jetson/Orin)** | **15-20** | **70%** | **3400** | **Base system (TensorRT)** |

### With VLM (Jetson Orin NX SUPER or 16GB RPi5 Required)

| Component | FPS | CPU % | RAM (MB) | Notes |
|-----------|-----|-------|----------|-------|
| Base System | 10-15 | 90% | 4400 | As above |
| BLIP-2 VLM | 2-3 | 30% | 4000 | Async processing |
| **Total** | **8-12** | **120%** | **8400** | **With VLM** |

## Troubleshooting

### Common Issues

1. **Low FPS (<10 FPS)**
   - Check CPU usage - may need to reduce processing load
   - Use OpenVINO models instead of PyTorch
   - Reduce image resolution for person tracking
   - Optimize MediaPipe confidence thresholds

2. **High Memory Usage (>5GB)**
   - Check for memory leaks in tracking history
   - Limit track history size per person ID
   - Reduce image queue sizes
   - Disable VLM if not needed

3. **Track IDs Lost Frequently**
   - Increase `persist=True` confidence in YOLOv8 tracking
   - Adjust tracking parameters (track_buffer, match_thresh)
   - Check camera FPS - low FPS causes tracking issues
   - Verify person detection confidence threshold

4. **Face/Hand Detection Not Working at Distance**
   - Adjust confidence thresholds for distance ranges
   - Crop and upscale person regions for far-range detection
   - Check lighting conditions
   - Verify person bounding boxes are large enough

5. **VLM Integration Fails**
   - Verify Jetson Orin NX SUPER or 16GB RPi5 is available (Jetson NANO 4GB insufficient)
   - Check model download completed successfully
   - Verify agent_process service is running
   - Check API keys and base URLs if using cloud VLM
   - Consider TensorRT optimization for Jetson/Orin