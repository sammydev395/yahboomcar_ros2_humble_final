# yahboomcar_mediapipe/launch Launch File Catalog

---

## yahboomcar_mediapipe/launch/01_HandDetector.launch
**Purpose:** Hand detection using MediaPipe.

**Starts:**
- Hand detector node (`01_HandDetector.py`)

**Use:** For detecting hands in camera input using MediaPipe.

---

## yahboomcar_mediapipe/launch/02_PoseDetector.launch
**Purpose:** Pose detection using MediaPipe.

**Starts:**
- Pose detector node (`02_PoseDetector.py`)

**Use:** For detecting body poses in camera input using MediaPipe.

---

## yahboomcar_mediapipe/launch/03_Holistic.launch
**Purpose:** Holistic (full-body) detection using MediaPipe.

**Starts:**
- Holistic detection node (`03_Holistic.py`)

**Use:** For full-body detection (face, hands, pose) using MediaPipe.

---

## yahboomcar_mediapipe/launch/04_FaceMesh.launch
**Purpose:** Face mesh detection using MediaPipe.

**Starts:**
- Face mesh detection node (`04_FaceMesh.py`)

**Use:** For detecting face mesh landmarks using MediaPipe.

---

## yahboomcar_mediapipe/launch/05_FaceEyeDetection.launch
**Purpose:** Face and eye detection using MediaPipe.

**Starts:**
- Face and eye detection node (`05_FaceEyeDetection.py`)

**Use:** For detecting faces and eyes using MediaPipe.

---

## yahboomcar_mediapipe/launch/cloud_Viewer.launch
**Purpose:** Point cloud viewer for MediaPipe outputs.

**Starts:**
- MediaPipe point cloud viewer node (`mediapipe_point`)

**Use:** For visualizing point clouds generated from MediaPipe outputs. 