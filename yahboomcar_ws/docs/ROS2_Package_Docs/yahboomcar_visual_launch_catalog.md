# yahboomcar_visual/launch Launch File Catalog

---

## yahboomcar_visual/launch/opencv_apps.launch
**Purpose:** Bring up the USB camera and perform image processing (optionally flipping the image).

**Starts:**
- USB camera node (via usb_cam-test.launch)
- Image processing node (`pub_image.py`)

**Use:** For general camera bringup and image processing.

---

## yahboomcar_visual/launch/simple_AR.launch
**Purpose:** Simple augmented reality demo with camera input.

**Starts:**
- AR processing node (`simple_AR.py`)
- Web video server for streaming

**Use:** For AR demos and web streaming.

---

## yahboomcar_visual/launch/ar_track.launch
**Purpose:** AR marker tracking using the camera.

**Starts:**
- USB camera node (via usb_cam-test.launch)
- Static transform for camera
- AR marker tracking node (`ar_track_alvar`)
- Optionally RViz for visualization

**Use:** For AR marker tracking and visualization.

---

## yahboomcar_visual/launch/astra_calibration.launch
**Purpose:** Calibration for the Astra camera.

**Starts:**
- Astra camera node (via orbbec_camera/astraproplus.launch)
- IR transform node (`IR_transform`)

**Use:** For calibrating the Astra camera.

---

## yahboomcar_visual/launch/astra_get_depth.launch
**Purpose:** Get and process depth images from the Astra camera.

**Starts:**
- Astra camera node (via orbbec_camera/astraproplus.launch)
- Depth image processing node (Python or C++ version)

**Use:** For depth image acquisition and processing from Astra.

---

## yahboomcar_visual/launch/astra_get_rgb.launch
**Purpose:** Get and process RGB images from the Astra camera.

**Starts:**
- Astra camera node (via orbbec_camera/astraproplus.launch)
- RGB image processing node (Python or C++ version)

**Use:** For RGB image acquisition and processing from Astra.

---

## yahboomcar_visual/launch/astra_image_flip.launch
**Purpose:** Flip Astra camera images.

**Starts:**
- Astra camera node (via orbbec_camera/astraproplus.launch)
- Image flipping node (`astra_image_flip.py`)

**Use:** For flipping Astra camera images.

---

## yahboomcar_visual/launch/laser_to_image.launch
**Purpose:** Convert lidar data to image format.

**Starts:**
- Lidar node (via ydlidar_ros_driver/TG.launch)
- Lidar-to-image processing node (`laser_to_image.py`)

**Use:** For visualizing lidar data as images.

---

## yahboomcar_visual/launch/pointCloud_pub.launch
**Purpose:** Publish and visualize random point clouds.

**Starts:**
- Point cloud publisher node (`pointCloud_pub`)
- Optionally RViz for visualization

**Use:** For point cloud publishing and visualization demos.

---

## yahboomcar_visual/launch/pointCloud_visualize.launch
**Purpose:** Visualize point clouds from a specified topic.

**Starts:**
- Point cloud visualization node (`pointCloud_visualize`)

**Use:** For visualizing point clouds from various sources. 