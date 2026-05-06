#!/usr/bin/env python3
"""Terminal /joy recorder for Yahboom X3PLUS gamepad mapping (D5 Phase 1).

Live ANSI/curses display of every button + axis on the gamepad. Press
'r' to toggle recording (status flashes RECORDING when on), 'q' to quit.
While recording, every state CHANGE is logged with a timestamp to the
output file. On quit, the file is finalized and a summary printed.

This is the Yahboom-flavored port of Ultra's
hiwonder_ros2_control/scripts/joy_recorder.py (lives at /home/pi/joy_recorder.py
on the Ultra Pi). Adapts BUTTON_LABELS and AXIS_LABELS for the Yahboom
shipped 15-button gamepad (DragonRise 0079:181c, standard xpad layout —
matches the Ultra DuLingKer for cross-robot operator consistency).

Requires:
  - rclpy (ROS2 Humble)
  - /joy publisher already running (joy_node reading /dev/yahboom_joy)

Usage (inside the yahboom_ros2_humble container, /joy already publishing):
    ros2 run yahboom_ros2_control joy_recorder.py [output_filename]

Default output: /tmp/yahboom_joy_recording.log
Press 'r' to toggle recording, 'q' to quit, '?' for help.
"""
import curses
import datetime
import os
import sys
import threading
import time

import rclpy
from rclpy.qos import HistoryPolicy, QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Joy

DEFAULT_OUT = "/tmp/yahboom_joy_recording.log"
AXIS_ACTIVE_THRESH = 0.15  # below this, axis shown as idle

# Yahboom-shipped 15-button gamepad (DragonRise 0079:181c) — standard
# xpad-style HID layout, identical to Ultra's DuLingKer mapping. Verified
# live on the X3PLUS on (your D5 session date here). Keep these in sync
# with config/yahboom_gamepad_map.yaml.
#
# Buttons 2, 5, 8, 9, 12 are UNMAPPED on this gamepad class — they fire
# no events (vendor convention). Indices left undefined here so they
# show as "?(N)" if they ever do fire.
# Verified live D5 Phase 1 (2026-05-06):
# Buttons 2, 5, 12 confirmed silent (no events). Buttons 8 and 9 are
# DIGITAL trigger reports paired with the analog axis values — pressing
# LT fires both buttons[8]=1 and axes[5] = -1.0, pressing RT fires both
# buttons[9]=1 and axes[4] = -1.0. Trigger AXIS polarity is rest=+1.0,
# pressed=-1.0 (opposite of what we initially documented).
BUTTON_LABELS = {
    0: "A",
    1: "B",
    3: "X",
    4: "Y",
    6: "L1",
    7: "R1",
    8: "LT",      # digital, paired with axes[5] analog
    9: "RT",      # digital, paired with axes[4] analog
    10: "SELECT",
    11: "START",
    13: "LS-clk",
    14: "RS-clk",
}
AXIS_LABELS = {
    0: "LS LR",
    1: "LS UD",
    2: "RS LR",
    3: "RS UD",
    4: "RT",     # analog trigger: rest=+1.0, pressed=-1.0
    5: "LT",     # analog trigger: rest=+1.0, pressed=-1.0
    6: "Dpad LR",
    7: "Dpad UD",
}


class JoyRecorder:
    def __init__(self, out_path: str):
        self.out_path = out_path
        self.recording = False
        self.events = []  # tuples: (t_offset, kind, index, value, prev_value)
        self.last_buttons = None
        self.last_axes = None
        self.msg_count = 0
        self.start_t = None
        self.lock = threading.Lock()

    def on_joy(self, msg: Joy):
        with self.lock:
            self.msg_count += 1
            buttons = list(msg.buttons)
            axes = list(msg.axes)
            now = time.monotonic()
            if self.last_buttons is None:
                self.last_buttons = buttons
                self.last_axes = axes
                return
            if self.recording:
                t_off = now - self.start_t
                for i, (prev, cur) in enumerate(zip(self.last_buttons, buttons)):
                    if prev != cur:
                        self.events.append((t_off, "button", i, cur, prev))
                for i, (prev, cur) in enumerate(zip(self.last_axes, axes)):
                    # Log axis changes that cross the active threshold either way
                    if abs(cur - prev) > 0.1 and (
                        abs(cur) > AXIS_ACTIVE_THRESH or abs(prev) > AXIS_ACTIVE_THRESH
                    ):
                        self.events.append((t_off, "axis", i, round(cur, 3), round(prev, 3)))
            self.last_buttons = buttons
            self.last_axes = axes

    def toggle_record(self):
        with self.lock:
            if not self.recording:
                self.recording = True
                self.start_t = time.monotonic()
                self.events.clear()
                return "started"
            else:
                self.recording = False
                self._save()
                return "stopped"

    def _save(self):
        ts = datetime.datetime.now().isoformat(timespec="seconds")
        # Derive a per-key summary
        button_press_counts = {}
        axis_max = {}
        for t_off, kind, idx, cur, prev in self.events:
            if kind == "button" and cur == 1:
                button_press_counts[idx] = button_press_counts.get(idx, 0) + 1
            elif kind == "axis":
                if idx not in axis_max or abs(cur) > abs(axis_max[idx]):
                    axis_max[idx] = cur

        with open(self.out_path, "w") as f:
            f.write("# yahboom joy_recorder.py session (X3PLUS, D5 Phase 1)\n")
            f.write(f"# saved: {ts}\n")
            f.write(f"# total events recorded: {len(self.events)}\n")
            f.write(f"# session duration: {self.events[-1][0]:.2f}s\n" if self.events else "# (empty session)\n")
            f.write("\n# === SUMMARY ===\n")
            f.write("# Button press counts:\n")
            for idx in sorted(button_press_counts):
                lbl = BUTTON_LABELS.get(idx, "?")
                f.write(f"#   {lbl:>8}({idx:2d}) pressed {button_press_counts[idx]} times\n")
            f.write("# Axis max-deflection observed (signed):\n")
            for idx in sorted(axis_max):
                lbl = AXIS_LABELS.get(idx, "?")
                f.write(f"#   {lbl:>8} (ax{idx}) max value: {axis_max[idx]:+.3f}\n")
            f.write("\n# === EVENT LOG ===\n")
            f.write("# t_offset_s,kind,index,label,value,prev_value\n")
            for t_off, kind, idx, cur, prev in self.events:
                lbl = (BUTTON_LABELS if kind == "button" else AXIS_LABELS).get(idx, "?")
                f.write(f"{t_off:.3f},{kind},{idx},{lbl},{cur},{prev}\n")


def render_bar(value: float, width: int = 20) -> str:
    """Render an axis value [-1..+1] as a horizontal bar centered at the midpoint."""
    half = width // 2
    bar = ["-"] * width
    bar[half] = "|"
    pos = int(round(value * half)) + half
    pos = max(0, min(width - 1, pos))
    bar[pos] = "#"
    return "[" + "".join(bar) + "]"


def draw_screen(stdscr, rec: "JoyRecorder", filename: str):
    stdscr.erase()
    h, w = stdscr.getmaxyx()

    title = " YAHBOOM JOY RECORDER  —  press 'r' to toggle record, 'q' to quit, '?' help "
    stdscr.addnstr(0, 0, title.center(w), w, curses.A_REVERSE)

    rec_status = "[RECORDING]" if rec.recording else "[ idle ]"
    rec_attr = curses.A_BOLD | (curses.color_pair(1) if rec.recording else curses.A_NORMAL)
    elapsed = (time.monotonic() - rec.start_t) if rec.recording and rec.start_t else 0
    line2 = f" status: {rec_status}  elapsed: {elapsed:6.1f}s   events: {len(rec.events)}   /joy msgs: {rec.msg_count}"
    stdscr.addnstr(1, 0, line2, w - 1, rec_attr)
    stdscr.addnstr(2, 0, f" output file: {filename}", w - 1)

    with rec.lock:
        buttons = list(rec.last_buttons or [])
        axes = list(rec.last_axes or [])

    stdscr.addnstr(4, 0, " BUTTONS  (highlighted = currently pressed; 2/5/8/9/12 expected silent)",
                   w - 1, curses.A_BOLD)
    if not buttons:
        stdscr.addnstr(5, 2, "(no /joy data yet — is joy_node running?)", w - 3)
    else:
        for i, b in enumerate(buttons):
            attr = curses.A_REVERSE if b else curses.A_DIM
            label = BUTTON_LABELS.get(i, f"?({i})")
            tag = f" {label:>7}({i:2d}):{'X' if b else '.'} "
            col = 2 + (i % 4) * 16
            row = 5 + (i // 4)
            try:
                stdscr.addnstr(row, col, tag, w - col - 1, attr)
            except curses.error:
                pass

    axes_row_start = 5 + max(1, (len(buttons) + 3) // 4) + 1
    stdscr.addnstr(axes_row_start, 0, " AXES", w - 1, curses.A_BOLD)
    for i, a in enumerate(axes):
        bar = render_bar(a, width=20)
        active = abs(a) > AXIS_ACTIVE_THRESH
        attr = curses.A_BOLD if active else curses.A_DIM
        label = AXIS_LABELS.get(i, f"ax{i}")
        line = f"  {label:>8} (ax{i}): {a:+.3f}  {bar}"
        try:
            stdscr.addnstr(axes_row_start + 1 + i, 0, line, w - 1, attr)
        except curses.error:
            pass

    help_row = h - 2
    if help_row > 0:
        stdscr.addnstr(help_row, 0, " keys: r = toggle record   q = quit   ? = help ", w - 1, curses.A_DIM)
    stdscr.refresh()


def show_help(stdscr):
    stdscr.erase()
    msg = [
        "yahboom joy_recorder.py — gamepad mapping helper for D5 Phase 1",
        "",
        "  r  toggle recording (events written to file when stopped)",
        "  q  quit (auto-saves any active recording)",
        "  ?  this help",
        "",
        "Live display shows the current state of every button and axis as",
        "the /joy publisher reports it. While RECORDING, every state change",
        "(button transitions, axis deflections > 0.15) is timestamped and",
        "appended to the in-memory log; on stop, written to the output file.",
        "",
        "Phase 1 procedure (per docs/YAHBOOM_GAMEPAD_INTEGRATION_PLAN.md):",
        "  1. Press each documented button once and watch the index light up.",
        "  2. Sweep each stick from rest to max in each direction.",
        "  3. Confirm 2/5/8/9/12 stay silent (unmapped on this gamepad).",
        "  4. If indices match expected → commit yahboom_gamepad_map.yaml.",
        "",
        "Press any key to return.",
    ]
    for i, line in enumerate(msg):
        try:
            stdscr.addnstr(i, 0, line, stdscr.getmaxyx()[1] - 1)
        except curses.error:
            pass
    stdscr.refresh()
    stdscr.nodelay(False)
    stdscr.getch()
    stdscr.nodelay(True)


def curses_main(stdscr, out_path: str):
    curses.curs_set(0)
    stdscr.nodelay(True)
    stdscr.timeout(50)
    if curses.has_colors():
        curses.start_color()
        curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)

    rclpy.init()
    node = rclpy.create_node("joy_recorder")
    rec = JoyRecorder(out_path)
    qos = QoSProfile(
        reliability=ReliabilityPolicy.RELIABLE,
        history=HistoryPolicy.KEEP_LAST,
        depth=20,
    )
    node.create_subscription(Joy, "/joy", rec.on_joy, qos)

    # Spin rclpy in a background thread so curses input remains responsive.
    stop_evt = threading.Event()

    def spinner():
        while not stop_evt.is_set():
            rclpy.spin_once(node, timeout_sec=0.05)

    spin_t = threading.Thread(target=spinner, daemon=True)
    spin_t.start()

    try:
        while True:
            draw_screen(stdscr, rec, out_path)
            try:
                ch = stdscr.getch()
            except curses.error:
                ch = -1
            if ch in (ord('q'), ord('Q'), 27):
                if rec.recording:
                    rec.toggle_record()
                break
            elif ch in (ord('r'), ord('R')):
                rec.toggle_record()
            elif ch == ord('?'):
                show_help(stdscr)
    finally:
        stop_evt.set()
        spin_t.join(timeout=1.0)
        try:
            node.destroy_node()
        except Exception:
            pass
        try:
            rclpy.shutdown()
        except Exception:
            pass


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_OUT
    try:
        curses.wrapper(curses_main, out_path)
    except KeyboardInterrupt:
        pass
    print()
    print(f"Recording (if any) saved to: {out_path}")
    if os.path.exists(out_path):
        print(f"File size: {os.path.getsize(out_path)} bytes")


if __name__ == "__main__":
    main()
