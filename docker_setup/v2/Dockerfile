# Dockerfile — yahboom_ros2_humble  (Phase B of docs/CONTAINER_REBUILD_PLAN.md)
#
# Reproduces the running yahboom_ros2_humble container from scratch.
# Source-of-truth replacement for the older Dockerfile.ros2.humble in
# yahboomcar_ros2_ws_new/docker_setup/Dockerfile.ros2.humble (which was
# ~50% complete and pre-dated the camera + voice + claude-agent-sdk
# additions). Snapshot inputs live at
# /home/jetson/yahboomcar_ros2_ws_new/docker_setup/snapshots/rosmaster_2026-05-15/.
#
# ## Design decisions (locked 2026-05-15)
#
# - **Build from ros:humble-ros-base-jammy.** Gets us ROS Humble + locales +
#   apt repo config baked in, slimmer than ros:humble-desktop. We add
#   desktop bits explicitly (rviz2 etc.) below since this is a development
#   robot.
# - **PyTorch / torchvision / ultralytics / mediapipe NOT included.**
#   These go in a separate ML overlay container (out of scope for Phase B).
#   The Jetson L4T wheel sourcing for torch is a build-system project of
#   its own and the ML pipeline can run in its own container talking ROS
#   over the network.
# - **claude-agent-sdk + mcp + pydantic + httpx kept in image.** User
#   confirmed intentional; agent-driven robot control runs inside this
#   container.
# - **Workspace bake controlled by BAKE_WORKSPACE build arg.**
#     - `--build-arg BAKE_WORKSPACE=false` (default, `-dev` variant) →
#       yahboomcar_ws stays as a bind mount; colcon build runs at first boot.
#     - `--build-arg BAKE_WORKSPACE=true` (`-prod` variant) → workspace
#       is colcon-built into the image. Adds ~1 GB, removes the need for
#       `colcon build` at customer-unit first boot.
# - **library_ws ydlidar driver built from source, NOT web_video_server.**
#   The library_ws web_video_server links against OpenCV 4.10 which
#   conflicts with the system OpenCV 4.5; we use the apt-installed
#   ros-humble-web-video-server at runtime instead. See
#   memory/project_web_video_server_clean_shell_required.md.
# - **astra_camera built from source.** No apt package exists; the source
#   tree lives in library_ws.
# - **Rosmaster_Lib installed as Python egg** (vendor uses setup.py
#   install; pip freeze can't see it).
# - **ROS_DOMAIN_ID=100 baked into entrypoint.** Aligns with fleet
#   standard (memory/MEMORY.md). The old image had =28 hardcoded; every
#   script overrode to 100. We bake 100 and drop the per-script
#   override.
# - **/dev symlinks moved to entrypoint, not Dockerfile CMD.** The
#   current container abuses CMD for `ln -sf /dev/ttyUSB1 /dev/myserial
#   && tail -f /dev/null` — fragile because /dev names are not stable
#   (the entire reason we have udev rules + the dual-CH340 devpath fix).
#   Better: entrypoint does symlinks defensively, then tail-keepalive.
#
# ## Layer ordering (cache-friendly: stable bottom, churn-prone top)
#
#   1. Base
#   2. Locale + tz
#   3. Apt packages (rarely change) — system libs
#   4. ROS apt packages (occasional changes)
#   5. Pip packages (more frequent)
#   6. Source builds — astra_camera, Rosmaster_Lib, library_ws/ydlidar
#   7. Workspace build (OPTIONAL via BAKE_WORKSPACE)
#   8. Entrypoint + env + udev rules
#   9. CMD
#
# Change order ONLY for cache reasons; deps don't actually reorder
# because each step builds independently.

# syntax=docker/dockerfile:1.6
#
# ── Multi-stage layout (converged with provision/ultra/ 2026-05-16) ──
#   base : every dependency (apt + pip + source builds + configs).
#          Validated once via the dev bringup. **VARIANT-INDEPENDENT** —
#          no ARG VARIANT is consumed anywhere in base, so dev and prod
#          share its cache. (Ultra hit a LABEL-variant cache-fork that
#          forced prod to recompile everything; we avoid it by simply
#          not having an ARG VARIANT at all and hardcoding the variant
#          LABEL per leaf stage.)
#   dev  : base + workspace bind-mounted at runtime (fast iteration).
#   prod : base + workspace COPY'd in + colcon-built into the image
#          (unit boots immediately functional; nothing bind-mounted, so
#          the baked /opt/ros/humble_overlays is actually used — prod
#          does NOT have the dev-only library_ws-shadow caveat).
#
# Build via build.sh:
#   ./build.sh dev  v1.0  → docker build --target dev  -t yahboom_ros2_humble:v1.0-dev
#   ./build.sh prod v1.0  → docker build --target prod -t yahboom_ros2_humble:v1.0-prod
# prod additionally needs ros2_ws_staging/src/ in the context (build.sh
# rsyncs it from the Jetson workspace).
FROM ros:humble-ros-base-jammy AS base

ARG CLAUDE_AGENT_SDK_VERSION=0.1.36

ENV DEBIAN_FRONTEND=noninteractive \
    TZ=America/Los_Angeles \
    LANG=en_US.UTF-8 \
    LC_ALL=en_US.UTF-8 \
    ROS_DISTRO=humble \
    ROS_DOMAIN_ID=100 \
    RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
    PYTHONDONTWRITEBYTECODE=1 \
    PIP_NO_CACHE_DIR=1

# ─────────────────────────────────────────────────────────────────────
# 2. Locale + tz
# ─────────────────────────────────────────────────────────────────────
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt-get update && apt-get install -y --no-install-recommends \
        locales tzdata && \
    locale-gen en_US.UTF-8 && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 && \
    rm -rf /var/lib/apt/lists/*

# ─────────────────────────────────────────────────────────────────────
# 3. System libraries + dev tools
# ─────────────────────────────────────────────────────────────────────
RUN apt-get update && apt-get install -y --no-install-recommends \
        # core dev
        build-essential cmake git pkg-config \
        ca-certificates curl wget gnupg2 lsb-release \
        sudo vim nano less file \
        # python
        python3-pip python3-dev python3-setuptools python3-wheel \
        python3-numpy python3-yaml python3-opencv python3-serial \
        # USB / serial / udev
        udev usbutils libusb-1.0-0 libusb-1.0-0-dev libudev-dev \
        # OpenCV + image libs (use system 4.5 from Ubuntu 22.04; do
        # NOT install opencv 4.10 from an overlay — see header note on
        # web_video_server)
        libopencv-dev libgl1-mesa-glx libglew-dev libepoxy-dev \
        # geometry / math
        libeigen3-dev libboost-all-dev \
        # camera + media
        libuvc-dev libv4l-dev v4l-utils ffmpeg \
        # ALSA / audio (for CI1302 voice module ALSA path)
        libasound2 libasound2-dev alsa-utils \
        # diag
        net-tools iputils-ping iproute2 jq htop lsof \
    && rm -rf /var/lib/apt/lists/*

# ─────────────────────────────────────────────────────────────────────
# 4. ROS packages
# ─────────────────────────────────────────────────────────────────────
# Strategy: install meta-packages where they exist, then top off with
# specific extras. ros-humble-desktop covers rviz2 + most demo bits.
# 450 ros-humble-* pkgs end up installed including transitive deps —
# we explicitly name only the ones we actually depend on; the rest
# come along.
RUN apt-get update && apt-get install -y --no-install-recommends \
        # bring-up / utility
        ros-humble-desktop \
        python3-colcon-common-extensions \
        python3-rosdep \
        python3-vcstool \
        # ros2_control + controllers we depend on
        ros-humble-ros2-control \
        ros-humble-ros2-controllers \
        ros-humble-controller-manager \
        ros-humble-forward-command-controller \
        ros-humble-joint-state-broadcaster \
        ros-humble-joint-trajectory-controller \
        ros-humble-mecanum-drive-controller \
        ros-humble-imu-sensor-broadcaster \
        # transforms / robot state
        ros-humble-robot-state-publisher \
        ros-humble-joint-state-publisher \
        ros-humble-xacro \
        ros-humble-urdf \
        ros-humble-tf2-ros ros-humble-tf2-tools ros-humble-tf2-geometry-msgs \
        # camera + image
        ros-humble-cv-bridge \
        ros-humble-image-transport ros-humble-image-transport-plugins \
        ros-humble-image-geometry \
        ros-humble-camera-info-manager \
        ros-humble-compressed-image-transport \
        ros-humble-web-video-server \
        # depthai (OAK-D Lite)
        ros-humble-depthai \
        ros-humble-depthai-bridge \
        ros-humble-depthai-descriptions \
        ros-humble-depthai-filters \
        ros-humble-depthai-ros \
        ros-humble-depthai-ros-driver \
        ros-humble-depthai-ros-msgs \
        ros-humble-depthimage-to-laserscan \
        # gamepad + teleop
        ros-humble-joy \
        ros-humble-teleop-twist-joy \
        ros-humble-teleop-twist-keyboard \
        # nav + slam
        ros-humble-nav2-bringup ros-humble-nav2-common \
        ros-humble-slam-toolbox ros-humble-cartographer-ros \
        ros-humble-laser-filters ros-humble-laser-geometry \
        # imu / EKF
        ros-humble-imu-filter-madgwick \
        ros-humble-robot-localization \
        # pcl
        ros-humble-pcl-ros ros-humble-pcl-conversions \
        # moveit (kept; some legacy demos use it)
        ros-humble-moveit \
        ros-humble-moveit-core \
        ros-humble-moveit-ros-planning-interface \
        ros-humble-moveit-ros-move-group \
        ros-humble-moveit-kinematics \
        ros-humble-moveit-planners \
        ros-humble-moveit-ros-visualization \
        ros-humble-moveit-simple-controller-manager \
        ros-humble-moveit-msgs \
        # rmw cyclonedds (fleet std)
        ros-humble-rmw-cyclonedds-cpp \
        # rosbridge (for any web UI bits)
        ros-humble-rosbridge-suite \
        # usb_cam (for legacy arm camera path before OAK-D Lite)
        ros-humble-usb-cam \
    && rm -rf /var/lib/apt/lists/*

# ─────────────────────────────────────────────────────────────────────
# 5. Python pip packages
# ─────────────────────────────────────────────────────────────────────
# Top-level packages only. Their deps (pydantic, anyio, httpx, mcp,
# annotated-types, etc.) come along transitively.
#
# NOT installed here (intentional — go in ML overlay container):
#   torch, torchvision, ultralytics, mediapipe
# ros:humble-ros-base-jammy ships Ubuntu 22.04's pip (~22.x) which:
#   - does NOT support --break-system-packages (added in pip 23.0)
#   - does NOT need it either (Ubuntu 22.04 is not PEP-668
#     externally-managed; that started in 23.04 / Debian 12)
# Upgrade pip first (better resolver for claude-agent-sdk's dep tree),
# then install plain. No --break-system-packages.
RUN python3 -m pip install --no-cache-dir --upgrade pip && \
    pip3 install --no-cache-dir \
        "claude-agent-sdk==${CLAUDE_AGENT_SDK_VERSION}" \
        depthai \
        pyserial

# ─────────────────────────────────────────────────────────────────────
# 6. Source-built dependencies
# ─────────────────────────────────────────────────────────────────────
# astra_camera (no apt pkg exists). At build time the source lives in
# the build context under context/astra_camera/. At runtime the
# vendor's source tree at /root/yahboomcar_ros2_ws_new/software/library_ws/
# is also bind-mounted — but baking the install into the image makes
# the container start without depending on the host source.
#
# Rosmaster_Lib (vendor Python lib via setup.py install — egg format).
# Same logic: source lives in context/Rosmaster_Lib/ at build, baked into
# image, vendor's host copy still bind-mounted for parity.
#
# library_ws ydlidar_ros2_driver — vendor's lidar driver. Built into a
# separate overlay so we can source it WITHOUT pulling in library_ws's
# broken web_video_server overlay. Final overlay path:
# /opt/ros/humble_overlays/ydlidar_ros2_driver/.
#
# Build context must include:
#   context/astra_camera/      ← copy of library_ws/src/astra_camera/
#   context/Rosmaster_Lib/     ← copy of yahboomcar_ros2_ws_new/software/Rosmaster_Lib/
#   context/ydlidar_driver/    ← copy of library_ws/src/ydlidar_ros2_driver/
# A separate prepare-context.sh script (Phase B helper) will scp these
# from the Jetson into the build context.

WORKDIR /tmp/source_builds

# Rosmaster_Lib. Vendor ships it three possible ways depending on which
# source tree prepare_context.sh found:
#   (a) py_install_V*/  → has setup.py  → `python3 setup.py install` (egg)
#   (b) package dir     → Rosmaster_Lib/__init__.py present → copy package
#   (c) bare module     → only Rosmaster_Lib.py → copy module
# Handle all three and FAIL the build if NONE produce an importable
# module — a silent no-op here cost a full build cycle on 2026-05-16.
COPY context/Rosmaster_Lib/ /tmp/source_builds/Rosmaster_Lib/
RUN set -e; \
    SITE=$(python3 -c 'import site,sys; print(site.getsitepackages()[0])'); \
    SRC=/tmp/source_builds/Rosmaster_Lib; \
    if [ -f "$SRC/setup.py" ]; then \
        echo "Rosmaster_Lib: setup.py install"; \
        cd "$SRC" && python3 setup.py install; \
    elif [ -f "$SRC/__init__.py" ]; then \
        echo "Rosmaster_Lib: package-dir copy → $SITE/Rosmaster_Lib"; \
        mkdir -p "$SITE/Rosmaster_Lib"; \
        cp -r "$SRC"/. "$SITE/Rosmaster_Lib/"; \
    elif [ -f "$SRC/Rosmaster_Lib.py" ]; then \
        echo "Rosmaster_Lib: bare-module copy → $SITE/"; \
        cp "$SRC/Rosmaster_Lib.py" "$SITE/Rosmaster_Lib.py"; \
    else \
        echo "FATAL: no usable Rosmaster_Lib in build context" >&2; exit 1; \
    fi; \
    cd /tmp && rm -rf /tmp/source_builds/Rosmaster_Lib; \
    python3 -c "from Rosmaster_Lib import Rosmaster; print('Rosmaster_Lib import OK')"

# astra_camera + ydlidar_ros2_driver — built into a dedicated overlay
# at /opt/ros/humble_overlays/ to keep them separable from the user
# workspace.
# ── YDLidar-SDK ───────────────────────────────────────────────────────
# ydlidar_ros2_driver does find_package(ydlidar_sdk). That's a
# standalone CMake C++ library (not a ROS package) that must be built
# and installed to /usr/local BEFORE the overlay colcon build. Vendor
# built it in-source + make install'd system-wide; we do an
# out-of-source build for cleanliness.
COPY context/ydlidar_sdk/ /tmp/source_builds/ydlidar_sdk/
RUN if [ -f /tmp/source_builds/ydlidar_sdk/CMakeLists.txt ]; then \
        cmake -S /tmp/source_builds/ydlidar_sdk \
              -B /tmp/source_builds/ydlidar_sdk/_build \
              -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_INSTALL_PREFIX=/usr/local \
              -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF && \
        cmake --build /tmp/source_builds/ydlidar_sdk/_build -j"$(nproc)" && \
        cmake --install /tmp/source_builds/ydlidar_sdk/_build && \
        ldconfig && \
        rm -rf /tmp/source_builds/ydlidar_sdk; \
    else \
        echo "WARN: YDLidar-SDK source not in build context; ydlidar driver will fail" >&2; \
    fi

# context/astra_camera/ contains the ros2_astra_camera tree = TWO
# packages (astra_camera + astra_camera_msgs). colcon discovers both
# recursively under src/. astra_camera bundles its own OpenNI2
# redistributables (openni2_redist/arm64/) — only libuvc (apt,
# installed in layer 3) is needed at build time.
COPY context/astra_camera/ /tmp/source_builds/overlay_ws/src/ros2_astra_camera/
COPY context/ydlidar_driver/ /tmp/source_builds/overlay_ws/src/ydlidar_ros2_driver/

RUN if [ -d /tmp/source_builds/overlay_ws/src/astra_camera ] || \
       [ -d /tmp/source_builds/overlay_ws/src/ydlidar_ros2_driver ]; then \
        cd /tmp/source_builds/overlay_ws && \
        . /opt/ros/humble/setup.sh && \
        colcon build --merge-install \
            --install-base /opt/ros/humble_overlays/install \
            --cmake-args -DCMAKE_BUILD_TYPE=Release && \
        cd /tmp && rm -rf /tmp/source_builds/overlay_ws; \
    else \
        echo "WARN: no overlay sources in build context; skipping" >&2; \
    fi

# ─────────────────────────────────────────────────────────────────────
# 7. Entrypoint + env + udev rules  (still base — variant-independent)
# ─────────────────────────────────────────────────────────────────────
# role is constant across variants → safe in base. Placed LATE (after
# the expensive apt/pip/SDK/overlay layers) so adding it doesn't
# invalidate their cache. The variant LABEL lives in the leaf stages
# ONLY — base never references an ARG VARIANT (no cache-fork).
LABEL org.lumiai.role="rosmaster"
# Udev rules (devpath-pinned CH340 + ydlidar). These are copied to a
# staging dir; at container START the entrypoint links them into
# /etc/udev/rules.d/ if udev is actually being used inside the
# container, OR they're a no-op (host-side udev is the real source of
# truth — see provision/jetson/99-yahboom-stm32.rules). Bake them in
# so the image is self-describing.
# CycloneDDS config. WITHOUT this, every ROS node that has
# CYCLONEDDS_URI=file:///etc/cyclonedds/config.xml set (start_ros2_nodes.sh
# exports it) aborts with "can't open configuration file" →
# rmw_create_node fails → no cameras. The old image had this baked;
# the v1.0-dev first build (2026-05-16) missed it and Phase D caught
# it. Placed late so it doesn't invalidate the expensive overlay build
# cache. Same 9-line config the rest of the fleet uses
# (provision/jetson/cyclonedds_config.xml).
RUN mkdir -p /etc/cyclonedds
COPY cyclonedds_config.xml /etc/cyclonedds/config.xml

COPY udev_rules/ /opt/udev_rules.d/

COPY docker_entrypoint.sh /ros_entrypoint.sh
RUN chmod +x /ros_entrypoint.sh

# Default .bashrc helpers — but per
# memory/project_web_video_server_clean_shell_required.md, NEVER
# auto-source the workspace into root's .bashrc. Sourcing
# yahboomcar_ws chains library_ws via parent_prefix_path which breaks
# `ros2 pkg prefix web_video_server`. The entrypoint sets up the
# default environment selectively.
RUN { \
        echo '# Auto-source by /ros_entrypoint.sh handles ROS sourcing.'; \
        echo '# DO NOT add `source /root/yahboomcar_ros2_ws/install/setup.bash`'; \
        echo '# here unconditionally — see entrypoint comments.'; \
    } >> /root/.bashrc

# ─────────────────────────────────────────────────────────────────────
# 8. Default command  (inherited by both leaf stages)
# ─────────────────────────────────────────────────────────────────────
WORKDIR /root
ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["tail", "-f", "/dev/null"]

# ═════════════════════════════════════════════════════════════════════
#  LEAF STAGES — the ONLY place a variant marker appears.
#  base is complete + validated above; leaves only differ in how the
#  ROS2 workspace is delivered. Neither leaf references an ARG VARIANT,
#  so the base layer cache is shared between them (no cache-fork).
# ═════════════════════════════════════════════════════════════════════

# ── dev ──────────────────────────────────────────────────────────────
# Workspace is bind-mounted from the host at `docker run` time
# (run_container.sh -v .../yahboomcar_ws:/root/yahboomcar_ros2_ws).
# First colcon build happens after container start. Just provide the
# mount point so the bind has somewhere to land.
FROM base AS dev
LABEL org.lumiai.variant="dev"
RUN mkdir -p /root/yahboomcar_ros2_ws

# ── prod ─────────────────────────────────────────────────────────────
# Workspace baked: COPY src + colcon-build into the image. Unit boots
# immediately functional; no host bind-mount, so astra_camera /
# ydlidar_ros2_driver resolve from the baked /opt/ros/humble_overlays
# (the dev-only library_ws-shadow problem cannot occur here).
#
# build.sh stages the Jetson workspace src into ros2_ws_staging/src/
# before `docker build --target prod`. Source ONLY /opt/ros/humble +
# the baked overlay — NOT a host library_ws (absent in prod by design;
# this is the whole point). web_video_server stays the apt build in
# base; do not let the workspace chain a library_ws overlay.
FROM base AS prod
LABEL org.lumiai.variant="prod"
COPY ros2_ws_staging/src/ /root/yahboomcar_ros2_ws/src/
RUN set -e; \
    cd /root/yahboomcar_ros2_ws; \
    . /opt/ros/humble/setup.sh; \
    if [ -f /opt/ros/humble_overlays/install/setup.sh ]; then \
        . /opt/ros/humble_overlays/install/setup.sh; \
    fi; \
    colcon build --merge-install \
        --cmake-args -DCMAKE_BUILD_TYPE=Release \
        --event-handlers console_direct+; \
    rm -rf build log; \
    test -f /root/yahboomcar_ros2_ws/install/setup.bash
