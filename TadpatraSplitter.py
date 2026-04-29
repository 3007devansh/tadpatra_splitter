#!/usr/bin/env python3
"""
TadpatraSplitter - Desktop Application
Python/Tkinter port of the C++/MFC TadpatraSplitter.
Splits Tadpatra (palm-leaf manuscript) scanned images into individual folio strips,
then optionally deskews each strip via horizontal projection-profile analysis.
Version: 1.0.0.0
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import threading
import os
import shutil
import math
import time
import glob
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageTk

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

APP_VERSION = "2.0.0.0"
MIN_BORDER_PIXEL_MARGIN = 0
MAX_BORDER_PIXEL_MARGIN = 150


# ---------------------------------------------------------------------------
# Core image-processing helpers  (no logic changes from C++ original)
# ---------------------------------------------------------------------------

def _read_image(path: str):
    """Read an image file into a BGR NumPy array (equivalent to cv::imread IMREAD_COLOR)."""
    return cv2.imread(path, cv2.IMREAD_COLOR)


def _mat_to_photoimage(img_bgr, max_w: int = None, max_h: int = None):
    """Convert a BGR OpenCV image to a Tkinter-compatible PhotoImage, optionally scaled."""
    rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
    pil = Image.fromarray(rgb)
    ow, oh = pil.size
    if max_w or max_h:
        ratio = 1.0
        if max_w and ow > max_w:
            ratio = max_w / ow
        if max_h:
            nh_at_ratio = int(oh * ratio)
            if nh_at_ratio > max_h:
                ratio = ratio * (max_h / nh_at_ratio)
        if ratio < 1.0:
            pil = pil.resize(
                (max(1, int(ow * ratio)), max(1, int(oh * ratio))),
                Image.LANCZOS,
            )
    return ImageTk.PhotoImage(pil)


def _rotate_image(img, angle_deg: float):
    """
    Rotate image anticlockwise by angle_deg degrees, expanding the canvas so no
    content is clipped (replicates cv::warpAffine with BORDER_REPLICATE and the
    RotatedRect bounding-box expansion used in ImageAngleCorrectionDlg).
    Positive angle  → anticlockwise.
    Negative angle  → clockwise.
    """
    h, w = img.shape[:2]
    cx, cy = w / 2.0, h / 2.0
    rot = cv2.getRotationMatrix2D((cx, cy), angle_deg, 1.0)

    # Equivalent to cv::RotatedRect(cv::Point2f(), mainImg.size(), dAngle).boundingRect2f()
    ac = abs(math.cos(math.radians(angle_deg)))
    as_ = abs(math.sin(math.radians(angle_deg)))
    new_w = int(h * as_ + w * ac)
    new_h = int(h * ac + w * as_)

    # Adjust translation to keep image centred in the larger canvas
    rot[0, 2] += new_w / 2.0 - cx
    rot[1, 2] += new_h / 2.0 - cy

    return cv2.warpAffine(
        img, rot, (new_w, new_h),
        flags=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_REPLICATE,
    )


def _detect_strips(img):
    """
    Detect Tadpatra strips via Otsu threshold (binary-inverse) + external contours.
    Filters contours by area > 10 000 and sorts top-to-bottom.

    Returns (object_details, contours) where object_details is a list of
        {'rect': (x, y, w, h), 'index': int}
    sorted ascending by the rect's y coordinate.
    """
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    _, bw = cv2.threshold(gray, 0, 255, cv2.THRESH_OTSU | cv2.THRESH_BINARY_INV)
    contours, _ = cv2.findContours(bw, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

    details = []
    for i, cnt in enumerate(contours):
        if cv2.contourArea(cnt) > 10000:
            details.append({"rect": cv2.boundingRect(cnt), "index": i})

    details.sort(key=lambda d: d["rect"][1])
    return details, contours


def _is_broken(details) -> bool:
    """
    Detect whether the detected strips look like broken / torn Tadpatra.
    Replicates the nBrokenStripsPossibilityCount / nSimilarWidthCount logic
    from CBatchProcessingDlg::ImageSplittingThreadFunc() exactly.
    """
    n = len(details)

    n_broken = 0
    for i in range(1, n):
        wc = details[i]["rect"][2]
        wp = details[i - 1]["rect"][2]
        if abs(wc - wp) >= (wc * 10) // 100:
            n_broken += 1

    n_similar = 0
    if n <= 5:
        for i in range(n):
            for j in range(n):
                if i != j and abs(details[i]["rect"][2] - details[j]["rect"][2]) <= 200:
                    n_similar += 1

    return (n_similar == 0) and (
        n_broken >= 2
        or (n_broken > 0 and (n - n_broken) <= 2)
        or (n_broken > 0 and n_broken >= n // 2)
    )


def _extract_strip(img, detail: dict, contours, border: int):
    """
    Extract a single Tadpatra strip with convex-hull masking (white background
    outside the hull) and add a white border of `border` pixels on all sides.

    Replicates the convexHull / drawContours / bitwise_or logic from
    CBatchProcessingDlg::ImageSplittingThreadFunc() exactly.
    """
    x, y, w, h = detail["rect"]
    cnt = contours[detail["index"]]

    # Compute convex hull (returnPoints=True, clockwise=False)
    hull = cv2.convexHull(cnt, clockwise=False, returnPoints=True)

    # Determine bounding box of the hull, then adjust hull points to be
    # relative to that bounding box (matches C++: boundingRect + point shift)
    hx, hy = cv2.boundingRect(hull)[:2]
    hull_adj = hull.copy()
    hull_adj[:, 0, 0] -= hx
    hull_adj[:, 0, 1] -= hy

    # Create a black mask the size of the object rect; draw hull as white
    mask = np.zeros((h, w), dtype=np.uint8)
    cv2.drawContours(mask, [hull_adj], 0, 255, cv2.FILLED)

    # Invert: hull interior = 0 (black), outside = 255 (white)
    cv2.bitwise_not(mask, mask)

    # Crop colour image at object rect
    crop = img[y : y + h, x : x + w].copy()

    # OR with 3-channel mask → pixels outside hull become white
    mask3 = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    result = cv2.bitwise_or(crop, mask3)

    # Add white border
    return cv2.copyMakeBorder(
        result,
        border, border, border, border,
        cv2.BORDER_CONSTANT,
        value=(255, 255, 255),
    )


def _deskew_image(img, angle_range: float = 10.0, angle_step: float = 0.5):
    """
    Deskew a strip image using horizontal projection-profile analysis.

    Algorithm
    ---------
    1. Convert to binary (Otsu, foreground = 255).
    2. For every candidate angle θ in [-angle_range, +angle_range] at angle_step
       increments, rotate the *downsampled* binary image by θ and compute the
       variance of its row-wise projection (sum of foreground pixels per row).
    3. The angle that maximises variance corresponds to horizontal text lines
       → that is the correction angle.
    4. Apply the chosen rotation to the full-resolution colour image with a
       white background fill (matches the manuscript's white background).

    The binary image is downsampled to ≤800 px on the long side for speed;
    the found angle is then applied to the original full-resolution image.
    """
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    _, bw = cv2.threshold(gray, 0, 255, cv2.THRESH_OTSU | cv2.THRESH_BINARY_INV)

    h, w = bw.shape

    # Downsample for faster search (angle accuracy is unchanged)
    scale = min(1.0, 800.0 / max(w, h, 1))
    sw, sh = max(1, int(w * scale)), max(1, int(h * scale))
    bw_small = cv2.resize(bw, (sw, sh), interpolation=cv2.INTER_NEAREST)
    scx, scy = sw / 2.0, sh / 2.0

    best_angle = 0.0
    best_score = -1.0

    a = -angle_range
    while a <= angle_range + 1e-9:
        rot = cv2.getRotationMatrix2D((scx, scy), a, 1.0)
        rotated_bw = cv2.warpAffine(
            bw_small, rot, (sw, sh),
            flags=cv2.INTER_NEAREST,
            borderMode=cv2.BORDER_CONSTANT, borderValue=0,
        )
        proj = np.sum(rotated_bw, axis=1, dtype=np.float64)
        score = float(np.var(proj))
        if score > best_score:
            best_score = score
            best_angle = a
        a += angle_step

    # No meaningful skew detected
    if abs(best_angle) < angle_step * 0.5:
        return img.copy()

    # Apply correction to full-resolution colour image with expanded white canvas
    cx, cy = w / 2.0, h / 2.0
    rot_mat = cv2.getRotationMatrix2D((cx, cy), best_angle, 1.0)
    ac = abs(math.cos(math.radians(best_angle)))
    as_ = abs(math.sin(math.radians(best_angle)))
    nw = int(h * as_ + w * ac)
    nh = int(h * ac + w * as_)
    rot_mat[0, 2] += nw / 2.0 - cx
    rot_mat[1, 2] += nh / 2.0 - cy

    return cv2.warpAffine(
        img, rot_mat, (nw, nh),
        flags=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT,
        borderValue=(255, 255, 255),
    )


def _split_and_save(
    img,
    fpath: str,
    out_folder: str,
    manuscript: str,
    tile_num: int,
    front_side: bool,
    border: int,
    do_deskew: bool,
    overwrite_cb=None,
) -> None:
    """
    Shared per-image splitting logic used by BOTH Batch and Manual pipelines.

    Steps (identical to Batch):
      1. Detect strips via Otsu threshold + contours (_detect_strips).
      2. Single strip   → copy source file as-is, strip_num = 0.
      3. Broken strips  → copy source file as-is, strip_num = 0.
      4. Valid strips   → extract each with convex-hull masking (_extract_strip),
                          optionally deskew (_deskew_image), then save.

    Strip numbering:
      Front (A): 1, 2, 3, … (top → bottom)
      Back  (B): N, N-1, …  (top → bottom, so pairs match A by folio number)

    Output filename format (batch-compatible):
      {manuscript}-{tile_num:05d}-{strip_num}-{A|B}({stem}).jpg

    Parameters
    ----------
    overwrite_cb : callable(out_path: str) -> bool | None
        Called only when out_path already exists.
        Return True to overwrite, False to skip.
        None (default) means always overwrite (batch behaviour).
    """
    stem = Path(fpath).stem
    side = "A" if front_side else "B"

    details, contours = _detect_strips(img)
    n = len(details)

    def _can_write(out_path: str) -> bool:
        if not os.path.exists(out_path):
            return True
        return overwrite_cb is None or overwrite_cb(out_path)

    if n == 1 or (n > 1 and _is_broken(details)):
        # Single strip or broken Tadpatra → copy original image unchanged
        out_name = f"{manuscript}-{tile_num:05d}-0-{side}({stem}).jpg"
        out_path = os.path.join(out_folder, out_name)
        if _can_write(out_path):
            shutil.copy2(fpath, out_path)
    else:
        # Valid multi-strip image → extract each strip with hull masking
        n_rem = n
        for i, od in enumerate(details):
            stripped = _extract_strip(img, od, contours, border)

            if do_deskew:
                stripped = _deskew_image(stripped)

            if front_side:
                strip_num = i + 1        # A: 1, 2, 3, …
            else:
                strip_num = n_rem        # B: N, N-1, N-2, …
                n_rem -= 1

            out_name = f"{manuscript}-{tile_num:05d}-{strip_num}-{side}({stem}).jpg"
            out_path = os.path.join(out_folder, out_name)
            if _can_write(out_path):
                cv2.imwrite(out_path, stripped)


# ---------------------------------------------------------------------------
# Batch Processing Tab
# ---------------------------------------------------------------------------

class BatchProcessingTab(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self._stop = False
        self._thread: threading.Thread = None
        self._photo_ref = None          # prevent GC of the last preview image
        self._build_ui()

    # ------------------------------------------------------------------ UI --

    def _build_ui(self):
        # ---- Left: settings panel ----------------------------------------
        lf = ttk.LabelFrame(self, text="Settings", padding=10)
        lf.grid(row=0, column=0, sticky="nsew", padx=6, pady=6)

        r = 0
        ttk.Label(lf, text="Input Image Folder:").grid(row=r, column=0, sticky="w")
        r += 1
        self._v_input = tk.StringVar()
        ttk.Entry(lf, textvariable=self._v_input, width=44).grid(
            row=r, column=0, sticky="ew", pady=2
        )
        bf = ttk.Frame(lf)
        bf.grid(row=r, column=1, padx=4)
        ttk.Button(bf, text="Browse", command=self._browse_input).pack(side="left", padx=2)
        ttk.Button(bf, text="Open Folder", command=self._open_input).pack(side="left", padx=2)

        r += 1
        ttk.Label(lf, text="Output Image Folder:").grid(
            row=r, column=0, sticky="w", pady=(8, 0)
        )
        r += 1
        self._v_output = tk.StringVar()
        ttk.Entry(lf, textvariable=self._v_output, width=44).grid(
            row=r, column=0, sticky="ew", pady=2
        )
        bf2 = ttk.Frame(lf)
        bf2.grid(row=r, column=1, padx=4)
        ttk.Button(bf2, text="Browse", command=self._browse_output).pack(side="left", padx=2)
        ttk.Button(bf2, text="Open Folder", command=self._open_output).pack(side="left", padx=2)

        r += 1
        ttk.Label(lf, text="Image Type:").grid(row=r, column=0, sticky="w", pady=(8, 0))
        self._v_imgtype = tk.StringVar(value="JPG")
        ttk.Combobox(
            lf, textvariable=self._v_imgtype,
            values=["JPG", "PNG"], state="readonly", width=10,
        ).grid(row=r, column=1, sticky="w", pady=(8, 0))

        r += 1
        ttk.Label(lf, text="Output Border Pixel Margin:").grid(
            row=r, column=0, sticky="w", pady=(8, 0)
        )
        self._v_border = tk.IntVar(value=100)
        ttk.Spinbox(
            lf, textvariable=self._v_border,
            from_=MIN_BORDER_PIXEL_MARGIN, to=MAX_BORDER_PIXEL_MARGIN, width=8,
        ).grid(row=r, column=1, sticky="w", pady=(8, 0))

        r += 1
        self._v_preview = tk.BooleanVar(value=True)
        ttk.Checkbutton(
            lf, text="Preview Image", variable=self._v_preview,
        ).grid(row=r, column=0, sticky="w", pady=(8, 0))

        r += 1
        self._v_title = tk.BooleanVar(value=True)
        ttk.Checkbutton(
            lf, text="First Folio is Title Image", variable=self._v_title,
        ).grid(row=r, column=0, sticky="w", pady=2)

        r += 1
        self._v_deskew = tk.BooleanVar(value=True)
        ttk.Checkbutton(
            lf, text="Deskew After Splitting (projection profile)",
            variable=self._v_deskew,
        ).grid(row=r, column=0, sticky="w", pady=2)

        r += 1
        bf3 = ttk.Frame(lf)
        bf3.grid(row=r, column=0, columnspan=2, pady=10)
        ttk.Button(bf3, text="Start Splitting", command=self._start).pack(side="left", padx=8)
        ttk.Button(bf3, text="Stop Splitting", command=self._stop_split).pack(side="left", padx=8)

        r += 1
        ttk.Separator(lf, orient="horizontal").grid(
            row=r, column=0, columnspan=2, sticky="ew", pady=4
        )
        r += 1
        self._v_status = tk.StringVar(
            value="Select Input/Output folders and click 'Start Splitting'."
        )
        ttk.Label(
            lf, textvariable=self._v_status,
            wraplength=400, foreground="navy", justify="left",
        ).grid(row=r, column=0, columnspan=2, sticky="w")

        lf.columnconfigure(0, weight=1)

        # ---- Right: preview panel ----------------------------------------
        pf = ttk.LabelFrame(self, text="Preview", padding=4)
        pf.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
        self._preview_lbl = ttk.Label(pf, text="No preview", anchor="center")
        self._preview_lbl.pack(expand=True, fill="both")

        self.columnconfigure(0, weight=3)
        self.columnconfigure(1, weight=2)
        self.rowconfigure(0, weight=1)

    # -------------------------------------------------------- folder helpers --

    def _browse_input(self):
        d = filedialog.askdirectory(
            title="Please select Input image folder",
            initialdir=self._v_input.get() or None,
        )
        if d:
            self._v_input.set(d)
            self._v_output.set(d + "_Splitted")

    def _browse_output(self):
        d = filedialog.askdirectory(
            title="Please select Output image folder",
            initialdir=self._v_output.get() or None,
        )
        if d:
            self._v_output.set(d)

    def _open_input(self):
        p = self._v_input.get().strip()
        if p and os.path.isdir(p):
            os.startfile(p)

    def _open_output(self):
        p = self._v_output.get().strip()
        if p and os.path.isdir(p):
            os.startfile(p)

    # -------------------------------------------------------- action helpers --

    def _start(self):
        self._stop = False

        inp = self._v_input.get().strip()
        if not inp:
            messagebox.showwarning("TadpatraSplitter", "Select Input Image Folder.")
            return
        if not os.path.isdir(inp):
            messagebox.showerror(
                "TadpatraSplitter",
                "Input Image Folder doesn't exist. Please select valid folder.",
            )
            return

        out = self._v_output.get().strip()
        if not out:
            messagebox.showwarning("TadpatraSplitter", "Select Output Image Folder.")
            return

        b = self._v_border.get()
        if not (MIN_BORDER_PIXEL_MARGIN <= b <= MAX_BORDER_PIXEL_MARGIN):
            messagebox.showerror(
                "TadpatraSplitter",
                f"Border margin must be between {MIN_BORDER_PIXEL_MARGIN} "
                f"and {MAX_BORDER_PIXEL_MARGIN}.",
            )
            return

        if self._thread and self._thread.is_alive():
            return

        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _stop_split(self):
        self._stop = True
        self.after(0, lambda: self._v_status.set("Splitting stopped."))

    def _set_status(self, msg: str):
        self.after(0, lambda m=msg: self._v_status.set(m))

    def _show_preview(self, img_bgr):
        if img_bgr is None or img_bgr.size == 0:
            self.after(
                0,
                lambda: self._preview_lbl.config(image="", text="No preview"),
            )
            return
        try:
            photo = _mat_to_photoimage(img_bgr, max_w=260, max_h=380)

            def _upd(p=photo):
                self._photo_ref = p
                self._preview_lbl.config(image=p, text="")

            self.after(0, _upd)
        except Exception:
            pass

    # --------------------------------------------------------- worker thread --

    def _run(self):
        try:
            inp = self._v_input.get().strip()
            out = self._v_output.get().strip()
            ext = self._v_imgtype.get()            # "JPG" or "PNG"
            border = self._v_border.get()
            first_is_title = self._v_title.get()

            os.makedirs(out, exist_ok=True)
            manuscript = Path(inp).stem

            # Collect files (match both upper- and lower-case extensions)
            raw = glob.glob(os.path.join(inp, f"*.{ext}")) + \
                  glob.glob(os.path.join(inp, f"*.{ext.lower()}"))
            seen: set = set()
            files = []
            for f in sorted(raw, key=lambda p: p.lower()):
                k = f.lower()
                if k not in seen:
                    seen.add(k)
                    files.append(f)

            running_idx = 0
            tile_num = 1
            front_side = True
            title_done = False
            t0 = time.time()

            for fpath in files:
                stem = Path(fpath).stem

                # ---- Title image: copy as-is, do NOT toggle front_side ----
                if first_is_title and not title_done:
                    out_name = f"{manuscript}-{0:05d}-0-X({stem}).jpg"
                    shutil.copy2(fpath, os.path.join(out, out_name))
                    title_done = True
                    continue          # skip front_side toggle (matches C++ 'continue')

                # ---- Read image ------------------------------------------
                if not os.path.isfile(fpath):
                    front_side = not front_side
                    if self._stop and front_side:
                        break
                    continue

                img = _read_image(fpath)
                if img is None or img.size == 0:
                    front_side = not front_side
                    if self._stop and front_side:
                        break
                    continue

                self._set_status(f"Processing {fpath}")
                running_idx += 1

                # Tile number increments on odd running_idx >= 3
                if running_idx >= 3 and running_idx % 2 == 1:
                    tile_num += 1

                # Preview (checkbox is re-read each iteration, as in C++)
                if self._v_preview.get():
                    self._show_preview(img)
                else:
                    self._show_preview(None)

                # ---- Strip detection + save (shared logic) ---------------
                _split_and_save(
                    img, fpath, out, manuscript, tile_num, front_side,
                    border, self._v_deskew.get(),
                )

                front_side = not front_side

                # Stop only at a tile boundary (when next image would be front side)
                if self._stop and front_side:
                    break

            elapsed_ms = int((time.time() - t0) * 1000)
            self._set_status(
                f"Splitting completed! Total {running_idx} files processed "
                f"in [{elapsed_ms}] milli-seconds."
            )

        except Exception as exc:
            self._set_status(f"Error: {exc}")


# ---------------------------------------------------------------------------
# Manual Processing Tab
# ---------------------------------------------------------------------------

class ManualProcessingTab(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self._photo_front = None
        self._photo_back = None
        self._build_ui()

    # ------------------------------------------------------------------ UI --

    def _build_ui(self):
        # ---- Left: settings panel ----------------------------------------
        lf = ttk.LabelFrame(self, text="Settings", padding=10)
        lf.grid(row=0, column=0, sticky="nsew", padx=6, pady=6)

        r = 0
        ttk.Label(lf, text="Front Side Image File:").grid(row=r, column=0, sticky="w")
        r += 1
        self._v_front = tk.StringVar()
        ttk.Entry(lf, textvariable=self._v_front, width=44).grid(
            row=r, column=0, sticky="ew", pady=2
        )
        ttk.Button(lf, text="Browse…", command=self._browse_front).grid(
            row=r, column=1, padx=4
        )

        r += 1
        ttk.Label(lf, text="Back Side Image File:").grid(
            row=r, column=0, sticky="w", pady=(8, 0)
        )
        r += 1
        self._v_back = tk.StringVar()
        ttk.Entry(lf, textvariable=self._v_back, width=44).grid(
            row=r, column=0, sticky="ew", pady=2
        )
        ttk.Button(lf, text="Browse…", command=self._browse_back).grid(
            row=r, column=1, padx=4
        )

        r += 1
        ttk.Label(lf, text="Output Image Folder:").grid(
            row=r, column=0, sticky="w", pady=(8, 0)
        )
        r += 1
        self._v_output = tk.StringVar()
        ttk.Entry(lf, textvariable=self._v_output, width=44).grid(
            row=r, column=0, sticky="ew", pady=2
        )
        bf = ttk.Frame(lf)
        bf.grid(row=r, column=1, padx=4)
        ttk.Button(bf, text="Browse", command=self._browse_output).pack(side="left", padx=2)
        ttk.Button(bf, text="Open", command=self._open_output).pack(side="left", padx=2)

        r += 1
        ttk.Label(lf, text="Output ManuScript Number:").grid(
            row=r, column=0, sticky="w", pady=(8, 0)
        )
        r += 1
        self._v_msnum = tk.StringVar()
        ttk.Entry(lf, textvariable=self._v_msnum, width=22).grid(
            row=r, column=0, sticky="w", pady=2
        )

        r += 1
        ttk.Label(lf, text="Output Border Pixel Margin:").grid(
            row=r, column=0, sticky="w", pady=(8, 0)
        )
        self._v_border = tk.IntVar(value=100)
        ttk.Spinbox(
            lf, textvariable=self._v_border,
            from_=MIN_BORDER_PIXEL_MARGIN, to=MAX_BORDER_PIXEL_MARGIN, width=8,
        ).grid(row=r, column=1, sticky="w", pady=(8, 0))

        r += 1
        self._v_deskew = tk.BooleanVar(value=True)
        ttk.Checkbutton(
            lf, text="Deskew After Splitting (projection profile)",
            variable=self._v_deskew,
        ).grid(row=r, column=0, sticky="w", pady=(8, 2))

        r += 1
        ttk.Button(lf, text="Start Splitting", command=self._start).grid(
            row=r, column=0, pady=12, sticky="w"
        )

        r += 1
        ttk.Separator(lf, orient="horizontal").grid(
            row=r, column=0, columnspan=2, sticky="ew", pady=4
        )
        r += 1
        self._v_status = tk.StringVar(
            value="Manually select Front and Backside Image of Manuscript "
                  "and click Start Splitting."
        )
        ttk.Label(
            lf, textvariable=self._v_status,
            wraplength=400, foreground="navy", justify="left",
        ).grid(row=r, column=0, columnspan=2, sticky="w")

        lf.columnconfigure(0, weight=1)

        # ---- Right: preview panel ----------------------------------------
        pf = ttk.LabelFrame(self, text="Preview", padding=4)
        pf.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)

        ttk.Label(pf, text="Front Side:").pack(anchor="w")
        self._lbl_front = ttk.Label(pf, text="No image", anchor="center", relief="sunken")
        self._lbl_front.pack(expand=True, fill="both", pady=(0, 6))

        ttk.Label(pf, text="Back Side:").pack(anchor="w")
        self._lbl_back = ttk.Label(pf, text="No image", anchor="center", relief="sunken")
        self._lbl_back.pack(expand=True, fill="both")

        self.columnconfigure(0, weight=2)
        self.columnconfigure(1, weight=1)
        self.rowconfigure(0, weight=1)

    # -------------------------------------------------------- folder/file helpers --

    def _browse_front(self):
        f = filedialog.askopenfilename(
            title="JPG Files",
            filetypes=[("JPG Files", "*.jpg"), ("All files", "*.*")],
        )
        if f:
            self._v_front.set(f)
            # Auto-set output to parent directory + "_Splitted"
            self._v_output.set(str(Path(f).parent) + "_Splitted")
            self._show_preview(f, front=True)

    def _browse_back(self):
        f = filedialog.askopenfilename(
            title="JPG Files",
            filetypes=[("JPG Files", "*.jpg"), ("All files", "*.*")],
        )
        if f:
            self._v_back.set(f)
            self._show_preview(f, front=False)

    def _browse_output(self):
        d = filedialog.askdirectory(
            title="Please select output image folder",
            initialdir=self._v_output.get() or None,
        )
        if d:
            self._v_output.set(d)

    def _open_output(self):
        p = self._v_output.get().strip()
        if p and os.path.isdir(p):
            os.startfile(p)

    def _show_preview(self, fpath: str, front: bool):
        img = _read_image(fpath)
        if img is None:
            return
        photo = _mat_to_photoimage(img, max_w=220, max_h=200)
        if front:
            self._photo_front = photo
            self._lbl_front.config(image=photo, text="")
        else:
            self._photo_back = photo
            self._lbl_back.config(image=photo, text="")

    # --------------------------------------------------------- splitting logic --

    def _start(self):
        front = self._v_front.get().strip()
        if not front:
            messagebox.showwarning("TadpatraSplitter", "Select Manuscript Frontside image.")
            return
        if not os.path.isfile(front):
            messagebox.showerror(
                "TadpatraSplitter",
                "Manuscript Frontside image file doesn't exist. Please select valid file.",
            )
            return

        back = self._v_back.get().strip()
        if not back:
            messagebox.showwarning("TadpatraSplitter", "Select Manuscript Backside image.")
            return
        if not os.path.isfile(back):
            messagebox.showerror(
                "TadpatraSplitter",
                "Manuscript Backside image file doesn't exist. Please select valid file.",
            )
            return

        msnum = self._v_msnum.get().strip()
        if not msnum:
            messagebox.showwarning(
                "TadpatraSplitter", "Please enter proper Manuscript Number."
            )
            return

        out = self._v_output.get().strip()
        if not out:
            messagebox.showwarning("TadpatraSplitter", "Select Output Image Folder.")
            return

        try:
            os.makedirs(out, exist_ok=True)
            border = self._v_border.get()
            do_deskew = self._v_deskew.get()

            # Ask-before-overwrite callback used instead of silent overwrite
            def _ask(out_path: str) -> bool:
                return messagebox.askyesno(
                    "TadpatraSplitter",
                    f"Output file {out_path} already exists. "
                    "Are you sure you want to overwrite it?",
                )

            # Process exactly two images: front (A) then back (B).
            # tile_num = 1 (single tile; manual mode always processes one tile pair).
            for fpath, front_side in [(front, True), (back, False)]:
                img = _read_image(fpath)
                if img is None or img.size == 0:
                    continue

                self._v_status.set(f"Processing {fpath}")
                self.update_idletasks()

                _split_and_save(
                    img, fpath, out,
                    manuscript=msnum,
                    tile_num=1,
                    front_side=front_side,
                    border=border,
                    do_deskew=do_deskew,
                    overwrite_cb=_ask,
                )

            self._v_status.set("Manual processing completed.")
            messagebox.showinfo("TadpatraSplitter", "Manual processing completed.")

        except Exception as exc:
            messagebox.showerror("TadpatraSplitter", f"Error: {exc}")
            self._v_status.set("Error occurred while processing file.")


# ---------------------------------------------------------------------------
# Image Angle Correction Tab
# ---------------------------------------------------------------------------

class ImageAngleCorrectionTab(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self._files: list = []
        self._idx: int = 0
        self._photo_ref = None
        self._build_ui()

    # ------------------------------------------------------------------ UI --

    def _build_ui(self):
        # ---- Top: folder row ---------------------------------------------
        top = ttk.Frame(self)
        top.pack(fill="x", padx=6, pady=4)

        ttk.Label(top, text="Input Image Folder:").pack(side="left")
        self._v_folder = tk.StringVar()
        ttk.Entry(top, textvariable=self._v_folder, width=52).pack(
            side="left", padx=4
        )
        ttk.Button(top, text="Browse", command=self._browse).pack(side="left", padx=2)
        ttk.Button(top, text="Open Folder", command=self._open_folder).pack(
            side="left", padx=2
        )

        # ---- Current file display ----------------------------------------
        cf = ttk.Frame(self)
        cf.pack(fill="x", padx=6, pady=2)
        ttk.Label(cf, text="Current File:").pack(side="left")
        self._v_currfile = tk.StringVar()
        ttk.Entry(
            cf, textvariable=self._v_currfile, state="readonly", width=72,
        ).pack(side="left", padx=4, fill="x", expand=True)

        # ---- Image preview canvas ----------------------------------------
        pf = ttk.LabelFrame(self, text="Image Preview", padding=4)
        pf.pack(fill="both", expand=True, padx=6, pady=4)
        self._canvas = tk.Canvas(pf, bg="#c8c8c8", cursor="crosshair")
        self._canvas.pack(fill="both", expand=True)
        self._canvas.bind("<Configure>", lambda _e: self._refresh_preview())

        # ---- Bottom controls row ----------------------------------------
        bot = ttk.Frame(self)
        bot.pack(fill="x", padx=6, pady=4)

        # Navigation
        nav = ttk.LabelFrame(bot, text="Navigation", padding=6)
        nav.pack(side="left", padx=4)
        ttk.Button(nav, text="← Prev", command=self._prev).pack(
            side="left", padx=4
        )
        ttk.Button(nav, text="Next →", command=self._next).pack(
            side="left", padx=4
        )

        # Rotation
        rot = ttk.LabelFrame(bot, text="Rotation (Ctrl+↑ / Ctrl+↓)", padding=6)
        rot.pack(side="left", padx=4)

        ttk.Button(
            rot, text="↺ Anti-Clockwise", command=self._rotate_acw,
        ).pack(side="left", padx=4)
        self._v_acw = tk.IntVar(value=1)
        ttk.Spinbox(rot, textvariable=self._v_acw, from_=1, to=45, width=4).pack(
            side="left"
        )

        ttk.Label(rot, text="   ").pack(side="left")
        ttk.Button(
            rot, text="↻ Clockwise", command=self._rotate_cw,
        ).pack(side="left", padx=4)
        self._v_cw = tk.IntVar(value=1)
        ttk.Spinbox(rot, textvariable=self._v_cw, from_=1, to=45, width=4).pack(
            side="left"
        )

        # Options
        opts = ttk.LabelFrame(bot, text="Options", padding=6)
        opts.pack(side="left", padx=4)
        self._v_overwrite = tk.BooleanVar(value=True)
        ttk.Checkbutton(
            opts, text="Overwrite Rotated File", variable=self._v_overwrite,
        ).pack()

        # Keyboard bindings ← → Ctrl+↑ Ctrl+↓
        for widget in (self, self._canvas):
            widget.bind("<Left>",        lambda _e: self._prev(),        add=True)
            widget.bind("<Right>",       lambda _e: self._next(),        add=True)
            widget.bind("<Control-Up>",  lambda _e: self._rotate_acw(),  add=True)
            widget.bind("<Control-Down>",lambda _e: self._rotate_cw(),   add=True)

    # -------------------------------------------------------- public helpers --

    def activate(self):
        """Called when this tab is brought to front; ensures keyboard focus."""
        self._canvas.focus_set()

    # -------------------------------------------------------- folder helpers --

    def _browse(self):
        d = filedialog.askdirectory(
            title="Please select image folder",
            initialdir=self._v_folder.get() or None,
        )
        if d:
            self._v_folder.set(d)
            self._load_files(d)

    def _open_folder(self):
        p = self._v_folder.get().strip()
        if p and os.path.isdir(p):
            os.startfile(p)

    def _load_files(self, folder: str):
        """Load all JPG files from folder, sorted alphabetically (like CFileFind)."""
        raw = (
            glob.glob(os.path.join(folder, "*.JPG"))
            + glob.glob(os.path.join(folder, "*.jpg"))
        )
        seen: set = set()
        uniq = []
        for f in sorted(raw, key=lambda p: p.lower()):
            k = f.lower()
            if k not in seen:
                seen.add(k)
                uniq.append(f)
        self._files = uniq
        self._idx = 0
        self._v_currfile.set("")
        if self._files:
            self._show_file()
        self._canvas.focus_set()

    # -------------------------------------------------------- navigation -----

    def _show_file(self):
        if not self._files:
            return
        fpath = self._files[self._idx]
        self._v_currfile.set(f"Current File: {fpath}")
        self._refresh_preview()

    def _refresh_preview(self):
        if not self._files:
            return
        fpath = self._files[self._idx]
        img = _read_image(fpath)
        if img is None:
            return

        cw = self._canvas.winfo_width()
        ch = self._canvas.winfo_height()
        if cw < 4 or ch < 4:
            return

        h, w = img.shape[:2]
        # Scale to fit canvas width (maintaining aspect ratio), as in C++ PreviewImage()
        ratio = min(cw / w, ch / h)
        nw = max(1, int(w * ratio))
        nh = max(1, int(h * ratio))

        resized = cv2.resize(img, (nw, nh), interpolation=cv2.INTER_AREA)
        rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
        photo = ImageTk.PhotoImage(Image.fromarray(rgb))

        self._photo_ref = photo
        self._canvas.delete("all")
        x0 = (cw - nw) // 2
        y0 = (ch - nh) // 2
        self._canvas.create_image(x0, y0, anchor="nw", image=photo)

    def _prev(self):
        if not self._files:
            return
        if self._idx > 0:
            self._idx -= 1
        self._show_file()

    def _next(self):
        if not self._files:
            return
        if self._idx < len(self._files) - 1:
            self._idx += 1
        self._show_file()

    # -------------------------------------------------------- rotation ------

    def _do_rotate(self, angle_deg: float):
        """Rotate current image and optionally overwrite it on disk."""
        if not self._files:
            return
        fpath = self._files[self._idx]
        if not os.path.isfile(fpath):
            return

        img = _read_image(fpath)
        if img is None:
            return

        rotated = _rotate_image(img, angle_deg)

        if self._v_overwrite.get():
            cv2.imwrite(fpath, rotated)
        else:
            ans = messagebox.askyesno(
                "TadpatraSplitter",
                "Are you sure you want to overwrite the rotated file over original file?",
            )
            if ans:
                cv2.imwrite(fpath, rotated)

        self._refresh_preview()

    def _rotate_acw(self):
        """Rotate anticlockwise by the selected angle (Ctrl+Up)."""
        self._do_rotate(float(self._v_acw.get()))

    def _rotate_cw(self):
        """Rotate clockwise by the selected angle (Ctrl+Down)."""
        self._do_rotate(-float(self._v_cw.get()))


# ---------------------------------------------------------------------------
# Main Application Window
# ---------------------------------------------------------------------------

class TadpatraSplitterApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(f"TadpatraSplitter - {APP_VERSION}")
        self.geometry("950x660")
        self.minsize(820, 560)
        self._build_ui()

    def _build_ui(self):
        self._nb = ttk.Notebook(self)
        self._nb.pack(fill="both", expand=True, padx=4, pady=4)

        self._tab_batch   = BatchProcessingTab(self._nb)
        self._tab_manual  = ManualProcessingTab(self._nb)
        self._tab_rotate  = ImageAngleCorrectionTab(self._nb)

        self._nb.add(self._tab_batch,  text="  Batch Processing   ")
        self._nb.add(self._tab_manual, text="  Manual Processing   ")
        self._nb.add(self._tab_rotate, text="  Image Angle Correction    ")

        self._nb.bind("<<NotebookTabChanged>>", self._on_tab_change)

    def _on_tab_change(self, event):
        selected = self._nb.select()
        if selected == str(self._tab_rotate):
            self._tab_rotate.activate()


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    app = TadpatraSplitterApp()
    app.mainloop()
