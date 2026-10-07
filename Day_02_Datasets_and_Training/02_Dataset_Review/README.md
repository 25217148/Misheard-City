# Dataset Review

**Misheard City · Day 2**

[Back to Day 2](../README.md) · [Openverse](../01a_Openverse/README.md) · [Google Images](../01b_Google_Images/README.md) · [Edge Impulse training](../03_Edge_Impulse_Training/README.md)

The review turns the candidates of 01a and 01b into a dataset. The code removes damaged images and repeats; the group keeps or rejects the rest; the export holds one folder per category.

[Open the notebook](02_Dataset_Review.ipynb) · [Open in Colab](https://colab.research.google.com/github/25217148/Misheard-City/blob/main/Day_02_Datasets_and_Training/02_Dataset_Review/02_Dataset_Review.ipynb)

---

## What each step does

Every code cell starts with its type: **SET-UP**, **CLEAN**, **CHECK** or **EXPORT**. Comments in capitals (`MERGE`, `FILTER`, `DEDUPLICATE`, `SORT`…) mark each operation inside a cell.

| Step | Type | What the code does | After running |
|---|---|---|---|
| 1 | Set-up | Group folder and cleaning limits | `✓ Settings` |
| 2 | Set-up, clean | Merges the two candidate tables; drops failed downloads and exact repeats | `✓ Loaded` |
| 3 | Clean | Removes unreadable files, images below 96 px and near-duplicates | `✓ Cleaned`, `removed.csv` |
| 4 | Check | Numbered contact sheets per category | `✓ Contact sheets saved`, `contact_sheets/` |
| 5 | Clean | The group's decisions: keep or reject | `✓ Decisions saved`, `review.csv` |
| 6 | Check | Kept images per category and collection | Chart and table |
| 7 | Export | Copies the kept images into one folder per category; zips the dataset | `✓ Exported`, `dataset/<label>/`, `dataset_summary.txt`, `dataset_upload.zip` |
| 8 | Export | Downloads the ZIP | `✓ Download started` |

Each step ends with a line that starts with **✓**. A step without its ✓ line stopped with an error; the last line of the error names the cause.

## Keep or reject?

| Keep | Reject |
|---|---|
| The main subject matches the definition | Illustrations, diagrams, maps, collages, screenshots |
| The subject fills a good part of the frame | Large text, logos or watermarks |
| Distance, light and angle the XIAO could also see | Wrong subject: the phrase only matched the title or tags |
| | Near-duplicates that Step 3 missed |
| | An identifiable person as the main subject |
| | Fits more than one category, or the group cannot agree |

Rejected images that fit more than one category carry the note `ambiguous` in `review.csv`. They are the test cases on Day 3.

## Balance

Categories need similar numbers of images. Duplicated images do not count.

---

## The notebook step by step

### Step 1 — Settings

The **same `GROUP`** as in 01a and 01b. `MIN_SIZE` and `NEAR_DUPLICATE_LIMIT` set the automatic cleaning in Step 3.

**After running:** `✓ Settings:` with the group and the two limits.

```python
# SET-UP · group folder and limits of the automatic cleaning
# ---- Settings to change ----------------------------------------------------
GROUP = "Group_01"            # Same name as in 01a and 01b
USE_GOOGLE_DRIVE = True       # Colab only: files are in Google Drive
MIN_SIZE = 96                 # CLEAN: images smaller than this (shorter side, pixels) are removed
NEAR_DUPLICATE_LIMIT = 4      # CLEAN: fingerprints this close (out of 64) count as one photograph
# -----------------------------------------------------------------------------

print(f"✓ Settings: {GROUP}, MIN_SIZE {MIN_SIZE}, NEAR_DUPLICATE_LIMIT {NEAR_DUPLICATE_LIMIT}")
```

### Step 2 — Load and merge the candidates

`candidates_openverse.csv` and `candidates_google.csv`, whichever exist, become one table. Failed downloads and exact repeats are dropped.

**After running:** In Colab, a Google permission window, then `Mounted at /content/drive`. `Loaded candidates_openverse.csv` and/or `Loaded candidates_google.csv`, a table of images per category and collection, and `✓ Loaded:` with the number of images and the category names.

```python
# SET-UP + CLEAN · loads both candidate tables, drops failed downloads and repeats
import shutil
from datetime import date
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
from PIL import Image

try:
    from google.colab import drive
    IN_COLAB = True
except ImportError:
    IN_COLAB = False

if IN_COLAB and USE_GOOGLE_DRIVE:
    drive.mount("/content/drive")
    BASE_DIR = Path("/content/drive/MyDrive/Misheard_City_Data")
else:
    BASE_DIR = Path.home() / "Misheard_City_Data"   # VS Code: in the home folder

PROJECT_DIR = BASE_DIR / GROUP
CANDIDATE_DIR = PROJECT_DIR / "candidates"

# MERGE: both candidate tables, Openverse rows first
tables = []
for name in ("candidates_openverse.csv", "candidates_google.csv"):
    if (PROJECT_DIR / name).exists():
        tables.append(pd.read_csv(PROJECT_DIR / name))
        print("Loaded", name)
if not tables:
    raise FileNotFoundError("No candidates file found. Run notebook 01a or 01b first.")
candidates = pd.concat(tables, ignore_index=True)

# CLEAN: failed downloads and exact repeats (same id) are dropped
candidates = candidates[candidates["download_ok"] == True]
candidates = candidates.drop_duplicates(subset="id").copy()
candidates["path"] = [str(CANDIDATE_DIR / label / file)
                      for label, file in zip(candidates["label"], candidates["file"])]

LABELS = list(dict.fromkeys(candidates["label"]))
print(pd.crosstab(candidates["label"], candidates["collection"], margins=True))
print(f"✓ Loaded: {len(candidates)} images in {len(LABELS)} categories: {', '.join(LABELS)}")
```

### Step 3 — Automatic cleaning

Unreadable files, images below `MIN_SIZE` and **near-duplicates** are removed and listed in `removed.csv`. A near-duplicate has almost the same **fingerprint** (average hash: 8 × 8 grey pixels, each brighter or darker than the mean) as an image already kept; the first copy stays, and Openverse rows come first.

**After running:** `✓ Cleaned:` with the images kept and removed, a count per reason, then up to 8 image pairs marked `kept` and `removed`. New file: `removed.csv`.

```python
# CLEAN · removes unreadable files, small images and near-duplicates; numbers the rest
def fingerprint(image, size=8):
    """Average hash: 8 × 8 grey pixels, each brighter (1) or darker (0) than the mean, as one number."""
    pixels = list(image.convert("L").resize((size, size)).tobytes())
    mean = sum(pixels) / len(pixels)
    return sum(1 << i for i, p in enumerate(pixels) if p > mean)


# FILTER: unreadable files and images below MIN_SIZE
reasons, hashes = [], []
for path in candidates["path"]:
    try:
        with Image.open(path) as image:
            image.load()
            hashes.append(fingerprint(image))
            reasons.append(f"smaller than {MIN_SIZE} px" if min(image.size) < MIN_SIZE else "")
    except Exception:
        hashes.append(None)
        reasons.append("unreadable")

# DEDUPLICATE: an image whose fingerprint is close to a kept one is removed; the first copy stays
duplicate_of = [""] * len(hashes)
kept_so_far = []
for k, fp in enumerate(hashes):
    if reasons[k]:
        continue
    for other_fp, other_path in kept_so_far:
        if bin(fp ^ other_fp).count("1") <= NEAR_DUPLICATE_LIMIT:
            reasons[k] = "near-duplicate"
            duplicate_of[k] = other_path
            break
    else:
        kept_so_far.append((fp, candidates["path"].iloc[k]))

candidates["removed"] = reasons
candidates["duplicate_of"] = duplicate_of
removed = candidates[candidates["removed"] != ""]
removed.to_csv(PROJECT_DIR / "removed.csv", index=False)

images = candidates[candidates["removed"] == ""].reset_index(drop=True)
images["n"] = images.groupby("label").cumcount()  # Numbers shown on the contact sheets
print(f"✓ Cleaned: {len(images)} images kept, {len(removed)} removed (list in removed.csv):")
print(removed["removed"].value_counts().to_string())

# CHECK: the first 8 near-duplicates beside the copy that was kept
pairs = removed[removed["removed"] == "near-duplicate"].head(8)
for row in pairs.itertuples():
    figure, (left, right) = plt.subplots(1, 2, figsize=(4, 2))
    for axis, path, title in ((left, row.duplicate_of, "kept"), (right, row.path, "removed")):
        axis.imshow(Image.open(path))
        axis.set_title(title, fontsize=8)
        axis.axis("off")
    plt.show()
```

### Step 4 — Contact sheets

30 numbered images at a time, saved as PNG in `contact_sheets/`; **G** marks Google Images and `show_sheet("moss", page=1)` shows the next 30. After Step 5, rejected images are faded.

**After running:** One numbered sheet of up to 30 images per category and `✓ Contact sheets saved`. New folder: `contact_sheets/` with one PNG per sheet.

```python
# CHECK · numbered contact sheets per category, also saved as PNG in contact_sheets/
SHEET_DIR = PROJECT_DIR / "contact_sheets"
SHEET_DIR.mkdir(exist_ok=True)


def show_sheet(label, page=0, per_page=30, columns=10):
    subset = images[images["label"] == label].iloc[page * per_page:(page + 1) * per_page]
    if subset.empty:
        print(f"No images on page {page} of '{label}'.")
        return
    rows_needed = -(-len(subset) // columns)
    figure, axes = plt.subplots(rows_needed, columns, figsize=(columns * 1.5, rows_needed * 1.7))
    for axis in axes.flat:
        axis.axis("off")
    for axis, row in zip(axes.flat, subset.itertuples()):
        decision = getattr(row, "decision", "keep")
        axis.imshow(Image.open(row.path), alpha=0.25 if decision == "reject" else 1.0)
        marker = " G" if row.collection == "google_images" else ""
        axis.set_title(f"{row.n}{marker}", fontsize=8)
    total = (images["label"] == label).sum()
    figure.suptitle(f"{label}: images {page * per_page}-{page * per_page + len(subset) - 1} of {total}")
    plt.tight_layout()
    figure.savefig(SHEET_DIR / f"{label}_page{page}.png", dpi=120)
    plt.show()


for label in LABELS:
    show_sheet(label, page=0)
print("✓ Contact sheets saved in", SHEET_DIR)
```

### Step 5 — Record your decisions

Numbers under `REJECT` are excluded; everything else is kept. `NOTES` holds short reasons, e.g. `ambiguous`, and the cell saves **`review.csv`** each time it runs.

**After running:** A table of keep and reject per category and `✓ Decisions saved`. A misspelt label prints `'…' is not one of your labels`. New file: `review.csv`.

```python
# CLEAN · the group's decisions: keep or reject
# ---- Your decisions: image numbers from the contact sheets -----------------
REJECT = {
    "moss": [],
    "stain": [],
    "weathered_paint": [],
}
NOTES = {
    # ("moss", 12): "ambiguous: lichen or moss?",
    # ("stain", 8): "illustration",
}
# -----------------------------------------------------------------------------

# CHECK: label names in the decisions
for label in REJECT:
    if label not in LABELS:
        print(f"'{label}' is not one of your labels {LABELS}: check the spelling.")

# CLEAN: every image is keep or reject; saved in review.csv
images["decision"] = "keep"
images["note"] = ""
for label, numbers in REJECT.items():
    images.loc[(images["label"] == label) & images["n"].isin(numbers), "decision"] = "reject"
for (label, number), note in NOTES.items():
    images.loc[(images["label"] == label) & (images["n"] == number), "note"] = note

images.to_csv(PROJECT_DIR / "review.csv", index=False)
print(pd.crosstab(images["label"], images["decision"], margins=True))
print("✓ Decisions saved in", PROJECT_DIR / "review.csv")
```

### Step 6 — Balance

Kept images per category, split into Openverse and Google Images.

**After running:** A bar chart and a table of kept images per category, split into Openverse and Google Images.

```python
# CHECK · kept images per category and collection
training = images[(images["decision"] == "keep") & (images["label"].isin(LABELS))]
counts = pd.crosstab(training["label"], training["collection"])
colours = {"openverse": "#9aa5b1", "google_images": "#d9a441"}

axis = counts.plot(kind="barh", stacked=True, figsize=(6, 0.6 * len(counts) + 1),
                   color=[colours[c] for c in counts.columns])
axis.set_xlabel("kept images")
axis.set_ylabel("")
plt.tight_layout()
plt.show()
print(counts)
```

### Step 7 — One folder per category

Kept images are copied into `dataset/<label>/`, one folder per category, recreated every time. The cell also saves `dataset_summary.txt` and zips `dataset/` as `dataset_upload.zip`.

**After running:** The dataset summary, `✓ Exported:` and `✓ ZIP created:`. New: one folder per category in `dataset/`, `dataset_summary.txt` and `dataset_upload.zip`.

```python
# EXPORT · one folder per category, dataset_summary.txt and the ZIP
DATASET_DIR = PROJECT_DIR / "dataset"
if DATASET_DIR.exists():
    shutil.rmtree(DATASET_DIR)

# SORT: kept images into dataset/<label>/, one folder per category
for row in images[images["decision"] == "keep"].itertuples():
    folder = DATASET_DIR / row.label
    folder.mkdir(parents=True, exist_ok=True)
    shutil.copy(row.path, folder / row.file)

# SUMMARY: counts per label and collection, removed and rejected images, search phrases
training = images[images["decision"] == "keep"]
summary = [
    f"Misheard City dataset - {GROUP} - {date.today().isoformat()}",
    "",
    "Training images per label:",
    training["label"].value_counts().to_string(),
    "",
    "Training images per collection:",
    training["collection"].value_counts().to_string(),
    "",
    f"Removed automatically: {len(removed)}",
    f"Rejected in review: {(images['decision'] == 'reject').sum()}",
    "",
    "Search phrases:",
]
for label in LABELS:
    phrases = images.loc[images["label"] == label, "query"]
    summary.append(f"  {label}: " + "; ".join(dict.fromkeys(phrases.dropna())))
(PROJECT_DIR / "dataset_summary.txt").write_text("\n".join(summary) + "\n")

# ZIP: dataset/ with its category folders, as one file for the download
archive = shutil.make_archive(str(PROJECT_DIR / "dataset_upload"), "zip", DATASET_DIR)
print("\n".join(summary))
print("\n✓ Exported: one folder per category in", DATASET_DIR)
print("✓ ZIP created:", archive)
```

### Step 8 — Download

In Colab, the cell downloads `dataset_upload.zip`, which is unzipped before upload. In VS Code, the folders are already on the computer.

**After running:** Colab: the browser downloads `dataset_upload.zip` and the cell prints `✓ Download started`. VS Code: `✓ VS Code:` and the path of the `dataset` folder.

```python
# EXPORT · dataset_upload.zip to the computer
if IN_COLAB:
    from google.colab import files
    files.download(str(PROJECT_DIR / "dataset_upload.zip"))
    print("✓ Download started: dataset_upload.zip")
else:
    print("✓ VS Code: the category folders are in", DATASET_DIR.resolve())
```

---

## Complete script

Settings, decisions, cleaning and export in one block, without the contact sheets. The script overwrites the previous export.

<details>
<summary>Show the complete review and export script</summary>

```python
# ===== 1. SET-UP · Settings ==================================================

# ---- Settings to change ----------------------------------------------------
GROUP = "Group_01"            # Same name as in 01a and 01b
USE_GOOGLE_DRIVE = True       # Colab only: files are in Google Drive
MIN_SIZE = 96                 # CLEAN: images smaller than this (shorter side, pixels) are removed
NEAR_DUPLICATE_LIMIT = 4      # CLEAN: fingerprints this close (out of 64) count as one photograph
# -----------------------------------------------------------------------------

print(f"✓ Settings: {GROUP}, MIN_SIZE {MIN_SIZE}, NEAR_DUPLICATE_LIMIT {NEAR_DUPLICATE_LIMIT}")


# ===== 2. CLEAN · Decisions from the contact sheets ==========================

# ---- Your decisions: image numbers from the contact sheets -----------------
REJECT = {
    "moss": [],
    "stain": [],
    "weathered_paint": [],
}
NOTES = {
    # ("moss", 12): "ambiguous: lichen or moss?",
    # ("stain", 8): "illustration",
}
# -----------------------------------------------------------------------------


# ===== 3. SET-UP + CLEAN · Load and merge the candidates =====================

import shutil
from datetime import date
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
from PIL import Image

try:
    from google.colab import drive
    IN_COLAB = True
except ImportError:
    IN_COLAB = False

if IN_COLAB and USE_GOOGLE_DRIVE:
    drive.mount("/content/drive")
    BASE_DIR = Path("/content/drive/MyDrive/Misheard_City_Data")
else:
    BASE_DIR = Path.home() / "Misheard_City_Data"   # VS Code: in the home folder

PROJECT_DIR = BASE_DIR / GROUP
CANDIDATE_DIR = PROJECT_DIR / "candidates"

# MERGE: both candidate tables, Openverse rows first
tables = []
for name in ("candidates_openverse.csv", "candidates_google.csv"):
    if (PROJECT_DIR / name).exists():
        tables.append(pd.read_csv(PROJECT_DIR / name))
        print("Loaded", name)
if not tables:
    raise FileNotFoundError("No candidates file found. Run notebook 01a or 01b first.")
candidates = pd.concat(tables, ignore_index=True)

# CLEAN: failed downloads and exact repeats (same id) are dropped
candidates = candidates[candidates["download_ok"] == True]
candidates = candidates.drop_duplicates(subset="id").copy()
candidates["path"] = [str(CANDIDATE_DIR / label / file)
                      for label, file in zip(candidates["label"], candidates["file"])]

LABELS = list(dict.fromkeys(candidates["label"]))
print(pd.crosstab(candidates["label"], candidates["collection"], margins=True))
print(f"✓ Loaded: {len(candidates)} images in {len(LABELS)} categories: {', '.join(LABELS)}")


# ===== 4. CLEAN · Remove unreadable, small and near-duplicate images =========

def fingerprint(image, size=8):
    """Average hash: 8 × 8 grey pixels, each brighter (1) or darker (0) than the mean, as one number."""
    pixels = list(image.convert("L").resize((size, size)).tobytes())
    mean = sum(pixels) / len(pixels)
    return sum(1 << i for i, p in enumerate(pixels) if p > mean)


# FILTER: unreadable files and images below MIN_SIZE
reasons, hashes = [], []
for path in candidates["path"]:
    try:
        with Image.open(path) as image:
            image.load()
            hashes.append(fingerprint(image))
            reasons.append(f"smaller than {MIN_SIZE} px" if min(image.size) < MIN_SIZE else "")
    except Exception:
        hashes.append(None)
        reasons.append("unreadable")

# DEDUPLICATE: an image whose fingerprint is close to a kept one is removed; the first copy stays
duplicate_of = [""] * len(hashes)
kept_so_far = []
for k, fp in enumerate(hashes):
    if reasons[k]:
        continue
    for other_fp, other_path in kept_so_far:
        if bin(fp ^ other_fp).count("1") <= NEAR_DUPLICATE_LIMIT:
            reasons[k] = "near-duplicate"
            duplicate_of[k] = other_path
            break
    else:
        kept_so_far.append((fp, candidates["path"].iloc[k]))

candidates["removed"] = reasons
candidates["duplicate_of"] = duplicate_of
removed = candidates[candidates["removed"] != ""]
removed.to_csv(PROJECT_DIR / "removed.csv", index=False)

images = candidates[candidates["removed"] == ""].reset_index(drop=True)
images["n"] = images.groupby("label").cumcount()  # Numbers shown on the contact sheets
print(f"✓ Cleaned: {len(images)} images kept, {len(removed)} removed (list in removed.csv):")
print(removed["removed"].value_counts().to_string())


# ===== 5. CLEAN · Apply the decisions and save review.csv ====================

# CHECK: label names in the decisions
for label in REJECT:
    if label not in LABELS:
        print(f"'{label}' is not one of your labels {LABELS}: check the spelling.")

# CLEAN: every image is keep or reject; saved in review.csv
images["decision"] = "keep"
images["note"] = ""
for label, numbers in REJECT.items():
    images.loc[(images["label"] == label) & images["n"].isin(numbers), "decision"] = "reject"
for (label, number), note in NOTES.items():
    images.loc[(images["label"] == label) & (images["n"] == number), "note"] = note

images.to_csv(PROJECT_DIR / "review.csv", index=False)
print(pd.crosstab(images["label"], images["decision"], margins=True))
print("✓ Decisions saved in", PROJECT_DIR / "review.csv")


# ===== 6. EXPORT · One folder per category and the ZIP =======================

DATASET_DIR = PROJECT_DIR / "dataset"
if DATASET_DIR.exists():
    shutil.rmtree(DATASET_DIR)

# SORT: kept images into dataset/<label>/, one folder per category
for row in images[images["decision"] == "keep"].itertuples():
    folder = DATASET_DIR / row.label
    folder.mkdir(parents=True, exist_ok=True)
    shutil.copy(row.path, folder / row.file)

# SUMMARY: counts per label and collection, removed and rejected images, search phrases
training = images[images["decision"] == "keep"]
summary = [
    f"Misheard City dataset - {GROUP} - {date.today().isoformat()}",
    "",
    "Training images per label:",
    training["label"].value_counts().to_string(),
    "",
    "Training images per collection:",
    training["collection"].value_counts().to_string(),
    "",
    f"Removed automatically: {len(removed)}",
    f"Rejected in review: {(images['decision'] == 'reject').sum()}",
    "",
    "Search phrases:",
]
for label in LABELS:
    phrases = images.loc[images["label"] == label, "query"]
    summary.append(f"  {label}: " + "; ".join(dict.fromkeys(phrases.dropna())))
(PROJECT_DIR / "dataset_summary.txt").write_text("\n".join(summary) + "\n")

# ZIP: dataset/ with its category folders, as one file for the download
archive = shutil.make_archive(str(PROJECT_DIR / "dataset_upload"), "zip", DATASET_DIR)
print("\n".join(summary))
print("\n✓ Exported: one folder per category in", DATASET_DIR)
print("✓ ZIP created:", archive)
```

</details>

---

## Outputs

| File or folder | Contents |
|---|---|
| `removed.csv` | Images removed in Step 3, with the reason and, for near-duplicates, the copy kept |
| `contact_sheets/` | The contact sheets as PNG |
| `review.csv` | Every remaining image with its decision and note; `ambiguous` cases are found by their note |
| `dataset/<label>/` | Training images, one folder per category |
| `dataset_summary.txt` | Date, counts per label and per collection, search phrases |
| `dataset_upload.zip` | `dataset/` with its category folders, as one file |

Each category folder is uploaded to Edge Impulse with its folder name as the label; Edge Impulse resizes the images itself.

**Next:** [Edge Impulse training](../03_Edge_Impulse_Training/README.md).

## Troubleshooting

| Symptom | Cause |
|---|---|
| `No candidates file found` | Different `GROUP`, Drive not mounted, or no search finished |
| `'…' is not one of your labels` | Misspelt label in `REJECT` |
| Different photographs removed as near-duplicates | `NEAR_DUPLICATE_LIMIT` too high for plain surfaces; 2 is stricter |
| Many images `smaller than 96 px` | Small Google thumbnails; `RESULTS_PER_PHRASE` in 01b adds more |
| The ZIP does not download in Colab | Browser blocking downloads; the ZIP is also on Drive |

[Continue to Edge Impulse training](../03_Edge_Impulse_Training/README.md)
