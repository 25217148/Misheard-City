# Image Search 2 — Google Images

**Misheard City · Day 2**

[Back to Day 2](../README.md) · [Openverse](../01a_Openverse/README.md) · [Dataset review](../02_Dataset_Review/README.md)

The notebook searches **Google Images** with the same labels and phrases as [01a](../01a_Openverse/README.md): the wider web's picture of each category. Google Images results are used **for training and in the film**.

[Open the notebook](01b_Google_Images.ipynb) · [Open in Colab](https://colab.research.google.com/github/25217148/Misheard-City/blob/main/Day_02_Datasets_and_Training/01b_Google_Images/01b_Google_Images.ipynb)

- [Part A — Get a SerpAPI key](#part-a--get-a-serpapi-key)
- [Part B — Collect images step by step](#part-b--collect-images-step-by-step)
- [Part C — Complete script](#part-c--complete-script)

---

## SerpAPI

[SerpAPI](https://serpapi.com/) runs the Google Images search for you and returns the results as JSON.

🔗 [Google Images API documentation](https://serpapi.com/google-images-api) · [Account API](https://serpapi.com/account-api)

---

## Part A — Get a SerpAPI key

The free plan needs an email address and a mobile phone number. It gives **250 searches per month**; each phrase uses one.

1. **Account:** [serpapi.com/users/sign_up](https://serpapi.com/users/sign_up), with an email address or a Google account.
2. **Email:** the confirmation link SerpAPI sends.
3. **Phone:** the code sent by SMS. The free plan starts after this step.
4. **Key:** copied from [Manage API Key](https://serpapi.com/manage-api-key).
5. **Colab Secrets** (key icon): **Add new secret**, name `SERPAPI_API_KEY`, the key as value, **Notebook access** on.

**VS Code:** an environment variable, see [VS Code on your own computer](../README.md#vs-code-on-your-own-computer).

> [!IMPORTANT]
> The key never goes into a cell, a README or a screenshot.

With three categories and two phrases each, the notebook uses **six searches**. Saved results in `api_cache/` cost no searches.

### Check

The cell shows the plan and the searches left, without using one.

```python
# SET-UP · checks the key and the searches left, without using one
import os
import requests


def read_secret(name):
    """Read a value from Colab Secrets, or else from an environment variable."""
    try:
        from google.colab import userdata
        return userdata.get(name)
    except Exception:
        return os.environ.get(name)


api_key = read_secret("SERPAPI_API_KEY")

if not api_key:
    print("No key found. Add SERPAPI_API_KEY.")
else:
    response = requests.get("https://serpapi.com/account.json",
                            params={"api_key": api_key}, timeout=30)
    if not response.ok:
        print("Key check failed:", response.status_code, response.text[:200])
    else:
        account = response.json()
        print("Plan:                    ", account.get("plan_name"))
        print("Searches per month:      ", account.get("searches_per_month"))
        print("Searches left this month:", account.get("total_searches_left"))
```

---

## Part B — Collect images step by step

![An image-search request and returned metadata](../images/api-image-flow.svg)

*A search phrase becomes a request, returned metadata and a local collection.*

Every code cell starts with its type: **SET-UP**, **COLLECT**, **CLEAN** or **CHECK**. Comments in capitals (`FILTER`, `CACHE`, `DEDUPLICATE`, `DOWNLOAD`) mark each operation.

| Step | Type | What the code does | After running |
|---|---|---|---|
| 1 | Set-up | Group folder, labels, phrases, country and language | `✓ Settings` |
| 2 | Set-up | Libraries, folders and a check of the label names | `✓ Folders ready` |
| 3 | Set-up | The SerpAPI key and the searches left | `✓ Key accepted` |
| 4 | Collect | The search function: one search per phrase; saved responses | `✓ search_google_images() is ready` |
| 5 | Check | The first result of one search | `✓ One search worked` |
| 6 | Collect, clean | Every result in one table; images found twice kept once | `✓ Collected`, `shared_between_categories_google.csv` |
| 7 | Collect, clean | Images downloaded; failed downloads marked in `download_ok` | `✓ Downloaded`, `google_*.jpg` in `candidates/<label>/`, `candidates_google.csv` |
| 8 | Check | The most frequent websites and the first 24 images of each category | `✓ Grids shown` |

Each step ends with a line that starts with **✓**. A step without its ✓ line stopped with an error; the last line of the error names the cause.

### Step 1 — Settings

The **same `GROUP`, labels and phrases** as in 01a.

- `RESULTS_PER_PHRASE`: images kept per phrase, about 100 per category with two phrases. One search returns up to 100.
- `GL` and `HL`: country and language of the Google search. Results change with both.

**After running:** `✓ Settings: Group_01, 3 categories, 6 search phrases = searches needed`.

```python
# SET-UP · group folder, labels, phrases and the Google search settings
# ---- Settings to change ----------------------------------------------------
GROUP = "Group_01"            # Same folder name as in notebook 01a
USE_GOOGLE_DRIVE = True       # Colab only: save files in Google Drive

CATEGORIES = {                # The same labels and phrases as in 01a
    "moss": ["moss on brick wall", "moss between paving stones"],
    "stain": ["water stain on concrete wall", "rust stain on wall"],
    "weathered_paint": ["peeling paint wall", "flaking paint door"],
}
RESULTS_PER_PHRASE = 50       # Images kept per phrase (one search returns up to 100)
GL = "uk"                     # Country of the Google search
HL = "en"                     # Language of the Google search
# -----------------------------------------------------------------------------

print(f"✓ Settings: {GROUP}, {len(CATEGORIES)} categories, "
      f"{sum(len(q) for q in CATEGORIES.values())} search phrases = searches needed")
```

### Step 2 — Libraries and folders

**After running:** In Colab, a Google permission window, then `Mounted at /content/drive`. Then `✓ Folders ready:` and the folder path.

```python
# SET-UP · libraries, the group folder and a check of the label names
import hashlib
import io
import json
import os
import pprint
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import requests
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
CACHE_DIR = PROJECT_DIR / "api_cache"
CANDIDATE_DIR = PROJECT_DIR / "candidates"
CACHE_DIR.mkdir(parents=True, exist_ok=True)

for label in CATEGORIES:
    if not re.fullmatch(r"[a-z0-9_]+", label):
        raise ValueError(f"Rename the label '{label}': use a-z, 0-9 and _ only.")

print("✓ Folders ready:", PROJECT_DIR.resolve())
```

### Step 3 — API key

The key check does not use a search.

**After running:** `✓ Key accepted. Searches left this month:` and a number. No search is used.

```python
# SET-UP · reads SERPAPI_API_KEY and checks it
SERPAPI = "https://serpapi.com"


def read_secret(name):
    """Read a value from Colab Secrets, or else from an environment variable."""
    try:
        from google.colab import userdata
        return userdata.get(name)
    except Exception:
        return os.environ.get(name)


API_KEY = read_secret("SERPAPI_API_KEY")
if not API_KEY:
    raise RuntimeError("No SerpAPI key found. Add SERPAPI_API_KEY (see Part A).")

account = requests.get(f"{SERPAPI}/account.json", params={"api_key": API_KEY}, timeout=30)
if not account.ok:
    raise RuntimeError(f"SerpAPI rejected the key ({account.status_code}). Check SERPAPI_API_KEY.")
print("✓ Key accepted. Searches left this month:", account.json().get("total_searches_left"))
```

### Step 4 — The search function

| Parameter | Value | Meaning |
|---|---|---|
| `engine` | `google_images` | Google Images, not the web search |
| `q` | your phrase | The search terms |
| `gl`, `hl` | `uk`, `en` | Country and language |
| `ijn` | `0` | First page of results |

**Each call uses one search.** Responses are saved in `api_cache/` and reused.

**After running:** `✓ search_google_images() is ready: nothing has been searched yet`. The cell only defines the function.

```python
# COLLECT · the search function: one search per phrase, saved in api_cache/
def search_google_images(query):
    """Return the Google Images results for one phrase. Each call uses one search."""
    # CACHE: a phrase searched before is read from api_cache/ and costs no search
    key = re.sub(r"[^a-z0-9]+", "_", query.lower()).strip("_")
    cache_file = CACHE_DIR / f"google_{key}_{GL}_{HL}.json"
    if cache_file.exists():
        return json.loads(cache_file.read_text())

    params = {
        "engine": "google_images",
        "q": query,
        "gl": GL,
        "hl": HL,
        "ijn": 0,              # First page of results
        "api_key": API_KEY,
    }
    response = requests.get(f"{SERPAPI}/search.json", params=params, timeout=60)
    if response.status_code == 401:
        raise RuntimeError("SerpAPI rejected the key. Check SERPAPI_API_KEY.")
    if response.status_code == 429:
        raise RuntimeError("SerpAPI limit reached: no searches left, or too many this hour.")
    response.raise_for_status()

    data = response.json()
    if "error" in data:
        print(f"SerpAPI for '{query}':", data["error"])
        return {"images_results": []}
    cache_file.write_text(json.dumps(data))
    return data


print("✓ search_google_images() is ready: nothing has been searched yet")
```

### Step 5 — Look at one response

Each result carries `title`, `source`, `link`, `thumbnail` and `original`; there is no creator and no licence.

**After running:** The number of results for the first phrase (up to 100), one result as a dictionary, and `✓ One search worked`. The cell uses one search, or none if the phrase is already in `api_cache/`.

```python
# CHECK · the first result of one search
first_label = next(iter(CATEGORIES))
first_query = CATEGORIES[first_label][0]
results = search_google_images(first_query).get("images_results", [])

print(f"'{first_query}': {len(results)} results")
pprint.pprint(results[0] if results else "No results", compact=True, width=100)
print("✓ One search worked: the first result is shown above")
```

### Step 6 — Collect every result, remove repeats

The columns match the Openverse table; `license` is `unknown`.

Each image gets an `id` made from its address (a short **hash**).

**After running:** One line per phrase with its number of results, `✓ Collected:` with the totals, and the number of images per category. New file: `shared_between_categories_google.csv`.

```python
# COLLECT + CLEAN · every result in one table; an image found twice is kept once
COLUMNS = [
    "label", "query", "id", "title", "creator", "license", "source",
    "foreign_landing_url", "url", "thumbnail", "width", "height", "collection",
]

rows = []
for label, queries in CATEGORIES.items():
    for query in queries:
        results = search_google_images(query).get("images_results", [])[:RESULTS_PER_PHRASE]
        for result in results:
            address = result.get("original") or result.get("thumbnail") or ""
            rows.append({
                "label": label,
                "query": query,
                "id": hashlib.md5(address.encode()).hexdigest()[:12],  # Same address, same id
                "title": result.get("title"),
                "creator": "",
                "license": "unknown",
                "source": result.get("source"),
                "foreign_landing_url": result.get("link"),
                "url": result.get("original"),
                "thumbnail": result.get("thumbnail"),
                "width": result.get("original_width"),
                "height": result.get("original_height"),
                "collection": "google_images",
            })
        print(f"{label:>18} | {query}: {len(results)} results")

found = pd.DataFrame(rows, columns=COLUMNS)

# CHECK: images found by more than one category, listed for the review
labels_per_image = found.groupby("id")["label"].nunique()
shared_ids = labels_per_image[labels_per_image > 1].index
shared = found[found["id"].isin(shared_ids)].sort_values("id")
shared.to_csv(PROJECT_DIR / "shared_between_categories_google.csv", index=False)

# DEDUPLICATE: an image found by several phrases or categories is kept once, under the first
candidates = found.drop_duplicates(subset="id", keep="first").reset_index(drop=True)
print(f"✓ Collected: {len(found)} results, {len(candidates)} different images, "
      f"{len(shared_ids)} found by more than one category")
print(candidates["label"].value_counts().to_string())
```

### Step 7 — Download images, mark failures

Images are saved as `candidates/<label>/google_<id>.jpg`, from the thumbnail or, if that fails, the original.

The table is saved as **`candidates_google.csv`**.

**After running:** A progress line every 25 images, a `Skipped …` line for each website that blocks downloads, and `✓ Downloaded: … images, … skipped`. New: `google_*.jpg` files in `candidates/<label>/`, and `candidates_google.csv`.

```python
# COLLECT + CLEAN · downloads the images; failed downloads are marked
def download_image(row):
    """Save the thumbnail; if that fails, try the original image."""
    folder = CANDIDATE_DIR / row.label
    folder.mkdir(parents=True, exist_ok=True)
    path = folder / row.file
    if path.exists():
        return True
    problem = "no image address"
    for address in (row.thumbnail, row.url):
        if not isinstance(address, str) or not address:
            continue
        try:
            response = requests.get(address, timeout=20,
                                    headers={"User-Agent": "Mozilla/5.0 (UCL teaching exercise)"})
            response.raise_for_status()
            image = Image.open(io.BytesIO(response.content)).convert("RGB")
            image.save(path, "JPEG", quality=90)
            return True
        except Exception as error:
            problem = error
    print("Skipped", row.file, "-", problem)
    return False


candidates["file"] = [f"google_{image_id}.jpg" for image_id in candidates["id"]]

# DOWNLOAD: one image per row; download_ok = False marks a failed download
ok = []
for number, row in enumerate(candidates.itertuples(), start=1):
    ok.append(download_image(row))
    if number % 25 == 0:
        print(f"{number} / {len(candidates)}")
candidates["download_ok"] = ok

candidates.to_csv(PROJECT_DIR / "candidates_google.csv", index=False)
print(f"✓ Downloaded: {sum(ok)} images, {len(ok) - sum(ok)} skipped. Saved", PROJECT_DIR / "candidates_google.csv")
```

### Step 8 — What came back?

The grids sit beside the Openverse grids from 01a.

**After running:** The five most frequent websites per category, one grid of images per category, and `✓ Grids shown`.

```python
# CHECK · the most frequent websites and the first 24 images of each category
print(candidates.groupby("label")["source"].value_counts().groupby(level=0).head(5))


def show_grid(label, count=24, columns=8):
    subset = candidates[(candidates["label"] == label) & candidates["download_ok"]].head(count)
    rows_needed = max(1, -(-len(subset) // columns))
    figure, axes = plt.subplots(rows_needed, columns, figsize=(columns * 1.6, rows_needed * 1.8))
    for axis in axes.flat:
        axis.axis("off")
    for axis, row in zip(axes.flat, subset.itertuples()):
        axis.imshow(Image.open(CANDIDATE_DIR / row.label / row.file))
        axis.set_title(str(row.source)[:18], fontsize=6)
    figure.suptitle(f"{label}: first {len(subset)} Google Images candidates", fontsize=10)
    plt.tight_layout()
    plt.show()


for label in CATEGORIES:
    show_grid(label)
print("✓ Grids shown: one per category, up to 24 images each")
```

---

## Part C — Complete script

Collection and download in one block.

<details>
<summary>Show the complete collection script</summary>

```python
# ===== 1. SET-UP · Settings ==================================================

# ---- Settings to change ----------------------------------------------------
GROUP = "Group_01"            # Same folder name as in notebook 01a
USE_GOOGLE_DRIVE = True       # Colab only: save files in Google Drive

CATEGORIES = {                # The same labels and phrases as in 01a
    "moss": ["moss on brick wall", "moss between paving stones"],
    "stain": ["water stain on concrete wall", "rust stain on wall"],
    "weathered_paint": ["peeling paint wall", "flaking paint door"],
}
RESULTS_PER_PHRASE = 50       # Images kept per phrase (one search returns up to 100)
GL = "uk"                     # Country of the Google search
HL = "en"                     # Language of the Google search
# -----------------------------------------------------------------------------

print(f"✓ Settings: {GROUP}, {len(CATEGORIES)} categories, "
      f"{sum(len(q) for q in CATEGORIES.values())} search phrases = searches needed")


# ===== 2. SET-UP · Libraries and folders =====================================

import hashlib
import io
import json
import os
import pprint
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import requests
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
CACHE_DIR = PROJECT_DIR / "api_cache"
CANDIDATE_DIR = PROJECT_DIR / "candidates"
CACHE_DIR.mkdir(parents=True, exist_ok=True)

for label in CATEGORIES:
    if not re.fullmatch(r"[a-z0-9_]+", label):
        raise ValueError(f"Rename the label '{label}': use a-z, 0-9 and _ only.")

print("✓ Folders ready:", PROJECT_DIR.resolve())


# ===== 3. SET-UP · API key ===================================================

SERPAPI = "https://serpapi.com"


def read_secret(name):
    """Read a value from Colab Secrets, or else from an environment variable."""
    try:
        from google.colab import userdata
        return userdata.get(name)
    except Exception:
        return os.environ.get(name)


API_KEY = read_secret("SERPAPI_API_KEY")
if not API_KEY:
    raise RuntimeError("No SerpAPI key found. Add SERPAPI_API_KEY (see Part A).")

account = requests.get(f"{SERPAPI}/account.json", params={"api_key": API_KEY}, timeout=30)
if not account.ok:
    raise RuntimeError(f"SerpAPI rejected the key ({account.status_code}). Check SERPAPI_API_KEY.")
print("✓ Key accepted. Searches left this month:", account.json().get("total_searches_left"))


# ===== 4. COLLECT · Search function ==========================================

def search_google_images(query):
    """Return the Google Images results for one phrase. Each call uses one search."""
    # CACHE: a phrase searched before is read from api_cache/ and costs no search
    key = re.sub(r"[^a-z0-9]+", "_", query.lower()).strip("_")
    cache_file = CACHE_DIR / f"google_{key}_{GL}_{HL}.json"
    if cache_file.exists():
        return json.loads(cache_file.read_text())

    params = {
        "engine": "google_images",
        "q": query,
        "gl": GL,
        "hl": HL,
        "ijn": 0,              # First page of results
        "api_key": API_KEY,
    }
    response = requests.get(f"{SERPAPI}/search.json", params=params, timeout=60)
    if response.status_code == 401:
        raise RuntimeError("SerpAPI rejected the key. Check SERPAPI_API_KEY.")
    if response.status_code == 429:
        raise RuntimeError("SerpAPI limit reached: no searches left, or too many this hour.")
    response.raise_for_status()

    data = response.json()
    if "error" in data:
        print(f"SerpAPI for '{query}':", data["error"])
        return {"images_results": []}
    cache_file.write_text(json.dumps(data))
    return data


print("✓ search_google_images() is ready: nothing has been searched yet")


# ===== 5. COLLECT + CLEAN · Every result, repeats removed ====================

COLUMNS = [
    "label", "query", "id", "title", "creator", "license", "source",
    "foreign_landing_url", "url", "thumbnail", "width", "height", "collection",
]

rows = []
for label, queries in CATEGORIES.items():
    for query in queries:
        results = search_google_images(query).get("images_results", [])[:RESULTS_PER_PHRASE]
        for result in results:
            address = result.get("original") or result.get("thumbnail") or ""
            rows.append({
                "label": label,
                "query": query,
                "id": hashlib.md5(address.encode()).hexdigest()[:12],  # Same address, same id
                "title": result.get("title"),
                "creator": "",
                "license": "unknown",
                "source": result.get("source"),
                "foreign_landing_url": result.get("link"),
                "url": result.get("original"),
                "thumbnail": result.get("thumbnail"),
                "width": result.get("original_width"),
                "height": result.get("original_height"),
                "collection": "google_images",
            })
        print(f"{label:>18} | {query}: {len(results)} results")

found = pd.DataFrame(rows, columns=COLUMNS)

# CHECK: images found by more than one category, listed for the review
labels_per_image = found.groupby("id")["label"].nunique()
shared_ids = labels_per_image[labels_per_image > 1].index
shared = found[found["id"].isin(shared_ids)].sort_values("id")
shared.to_csv(PROJECT_DIR / "shared_between_categories_google.csv", index=False)

# DEDUPLICATE: an image found by several phrases or categories is kept once, under the first
candidates = found.drop_duplicates(subset="id", keep="first").reset_index(drop=True)
print(f"✓ Collected: {len(found)} results, {len(candidates)} different images, "
      f"{len(shared_ids)} found by more than one category")
print(candidates["label"].value_counts().to_string())


# ===== 6. COLLECT + CLEAN · Images and candidates_google.csv =================

def download_image(row):
    """Save the thumbnail; if that fails, try the original image."""
    folder = CANDIDATE_DIR / row.label
    folder.mkdir(parents=True, exist_ok=True)
    path = folder / row.file
    if path.exists():
        return True
    problem = "no image address"
    for address in (row.thumbnail, row.url):
        if not isinstance(address, str) or not address:
            continue
        try:
            response = requests.get(address, timeout=20,
                                    headers={"User-Agent": "Mozilla/5.0 (UCL teaching exercise)"})
            response.raise_for_status()
            image = Image.open(io.BytesIO(response.content)).convert("RGB")
            image.save(path, "JPEG", quality=90)
            return True
        except Exception as error:
            problem = error
    print("Skipped", row.file, "-", problem)
    return False


candidates["file"] = [f"google_{image_id}.jpg" for image_id in candidates["id"]]

# DOWNLOAD: one image per row; download_ok = False marks a failed download
ok = []
for number, row in enumerate(candidates.itertuples(), start=1):
    ok.append(download_image(row))
    if number % 25 == 0:
        print(f"{number} / {len(candidates)}")
candidates["download_ok"] = ok

candidates.to_csv(PROJECT_DIR / "candidates_google.csv", index=False)
print(f"✓ Downloaded: {sum(ok)} images, {len(ok) - sum(ok)} skipped. Saved", PROJECT_DIR / "candidates_google.csv")
```

</details>

---

## Outputs

| File or folder | Contents |
|---|---|
| `candidates_google.csv` | One row per image: label, phrase, title, website, page link, image links, `download_ok` |
| `candidates/<label>/google_<id>.jpg` | The downloaded images |
| `shared_between_categories_google.csv` | Images returned for more than one category |
| `api_cache/google_*.json` | Saved SerpAPI responses |

**Next:** [Dataset review](../02_Dataset_Review/README.md).

## Troubleshooting

| Symptom | Cause |
|---|---|
| `No SerpAPI key found` | Secret name not `SERPAPI_API_KEY`, or **Notebook access** off |
| `SerpAPI rejected the key` (401) | Key incomplete or regenerated |
| `SerpAPI limit reached` (429) | No searches left this month |
| `Google hasn't returned any results` | Phrase too specific |
| Many skipped downloads | Websites blocking downloads; skipped images are left out |

[Continue to Dataset review](../02_Dataset_Review/README.md)
