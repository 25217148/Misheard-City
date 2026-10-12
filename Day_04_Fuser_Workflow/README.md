# Fuser Workflow and Creative Code
## Misheard City — Day 4

**Monday 12 October · Kunpeng Lei**

[Workshop homepage](../README.md) · [Fuser](#fuser) · [Storyboard](#1-script-to-storyboard) · [Keyframes](#2-keyframes) · [Creative Code](#3-creative-code) · [Image to video](#4-image-to-video) · [Generated material](#generated-material)

**You need:** `script.md`, the Miro storyboard and the shot list from Day 3, the device records and phone footage, sorted as on the [field collection](../Day_03_Deployment_and_Recording/README.md#field-collection) page, `field/records_for_prompt.txt`, and access to [Fuser](https://fuser.studio/).

## Tutorial

Two device records and two phone clips from field collection, with the shots that were not captured. Every row of the shot list then carries a status: recorded, generated or to film.

---

## Fuser

**Fuser** is a node-based canvas for text, images, video and code. Each node holds content or runs a model; sockets connect one node's output to the next, their colour marking the type of data.

| Node | Input | Output |
|---|---|---|
| **Text** | — | Prompts, records, notes |
| **Image** | PNG, JPG or WebP upload | Image |
| Chat model, e.g. Claude, GPT | Text, image | Text |
| Image model, e.g. Flux | Text, reference image | Image |
| Video model, e.g. Kling, Veo, Runway | Image, text | Video clip |
| **Creative Code** | A description or JavaScript; upstream image, video, audio | A live sketch; a photo or a video |

Node names and the models on offer follow the current Fuser interface.

Every model run spends credits. A video run costs many times an image run, so each shot is settled as a keyframe or a sketch before it moves.

### Platforms

| Platform | Use |
|---|---|
| [Fuser](https://fuser.studio/) | Keyframes, Creative Code, image to video |
| [Figma Weave](https://weave.figma.com/) | Keyframes and video on a node canvas; free account: image models, 150 credits a month |
| [Kling AI](https://klingai.com/), [Hailuo AI](https://hailuoai.video/) | Image to video; free credits, watermarked clips |

---

## 1. Script to storyboard

```text
Text (script.md + shot list) ──► Chat model ──► one image prompt per shot
```

Prompt for the chat model:

> From this script and shot list, write one image prompt for every shot whose material is generated keyframe or generated video. Each prompt names the subject, the place, the light, the camera height and the lens in under 40 words. No style words, no identifiable people. Return one prompt per line, in shot order.

Each line of the answer goes into its own **Text** node, one node per shot.

The [storyboard](../Day_03_Deployment_and_Recording/README.md#storyboard) from Day 3 continues here. Four columns of the [shot list](../Day_03_Deployment_and_Recording/shot_list_template.csv) are filled:

| Column | Content |
|---|---|
| `status` | recorded, generated, to film |
| `keyframe` | The keyframe file for a generated shot |
| `on_screen_text` | Captions, labels, scores |
| `sound` | Field sound, voice-over, music |

---

## 2. Keyframes

A **keyframe** is the single image that fixes a generated shot: its framing, light and look.

```text
Text (shot prompt + style prompt) ──► Image model ──► keyframe
Image (device photograph) ─────────► (reference image)
```

One **style prompt** closes every shot prompt, and the film keeps one look:

> Overcast London daylight, muted colour, 35 mm lens, eye level, slight film grain, documentary photograph.

A device photograph as reference image ties a keyframe to a real record. A chat model given that photograph describes it in text, a starting point for its shot prompt.

Keyframes are saved as `K<shot>_<desc>.png`, for example `K03_wall_moss.png`.

### Keyframes from records

The records as text become image prompts: the surface turns into the heard category, further as the score rises.

```text
Text (records) ──► Chat model ──► image prompts ──► Image model ──► keyframes
Image (device photograph) ────────────────────────► (reference image)
```

Records as text, one per line, from `field/records_for_prompt.txt`:

```text
rec_0007 | weathered brick wall with dark water stains | heard as moss | 0.62
rec_0012 | concrete bench with lichen spots | heard as moss | 0.81
rec_0019 | painted railing with rust | heard as stain | 0.47
```

Prompt for the chat model:

> Each line is one record from our device: the record, what the photograph shows, what the device heard, and its score. Write one image prompt of under 40 words per record. The image starts from the real surface and turns towards the category the device heard. The higher the score, the further the change: at 0.4 a hint, at 0.9 the surface has become the category. No identifiable people. Return one prompt per line.

---

## 3. Creative Code

**Creative Code** is a Fuser node that runs a sketch in p5.js, Three.js or HTML5 Canvas. The sketch is described in plain language or written in JavaScript, and runs live on the canvas.

### Records into the sketch

The device records reach a sketch in two ways:

- **As text:** values and words pasted into the sketch or its description, from `field/records_for_prompt.txt` or the field log.
- **As parameters:** a value such as `score`, declared in the sketch and wired to an upstream node. One sketch then serves every record.

| Record | In the sketch |
|---|---|
| `top_score` | Size, speed, density, how far the surface changes |
| `top_label` | Colour, word, form |
| Time, `millis_ms` | Position across the frame, order of events |
| Photograph | The surface the sketch starts from, as upstream image |

### From sketch to video

```text
Text (records) ─────────┐
Image (device photo) ───┴─► Creative Code ─┬─► Video, up to 2 min ──► the edit, as shot or layer
                                           └─► Photo (one frame) ──► Video model ◄── Text (motion)
```

A **recording** goes into the edit as it is: a shot of its own, or a layer above the footage. A sketch on black sits above the footage in **Screen** blend mode, and the black drops out. A **frame** saved as Photo is an image like any keyframe and goes into [image to video](#4-image-to-video). A video-to-video model, e.g. Kling O3 Edit, restyles a recording.

Recordings are saved as `C<shot>_<desc>.mp4`, for example `C05_score_line.mp4`.

### Examples

Three starting points, each changed with the group's own records:

| Example | Input, pasted as text | Changeable |
|---|---|---|
| Score line | Minute, label, score per record | Records, colours, duration |
| Heard words | Lines of `records_for_prompt.txt` | Word size, drift, duration |
| Seen and heard | Device photograph; one line of `records_for_prompt.txt` | Layout, speed of change, captions |

**1 · Score line.** The top score over one session, coloured by label, on black. Code: [score_line.js](creative_code/score_line.js). The records and colours at its top are the group's own:

```js
// Each record: [minute in the session, label, top score], from the field log.
const records = [
  [0, "stain", 0.62],
  [2, "stain", 0.55],
  [4, "moss", 0.71],
  [6, "moss", 0.81],
  [8, "rust", 0.47],
];

// One colour per category, as red, green, blue.
const colours = {
  moss: [120, 200, 120],
  stain: [200, 160, 90],
  rust: [210, 90, 60],
  paint: [120, 160, 230],
};

const seconds = 20; // time for the line to cross the frame
```

**2 · Heard words.** A description for the node, with the records pasted in:

> A Three.js scene on black, 1920 × 1080. Each line below is one record: the record, what the photograph shows, what the device heard, the score. Each record becomes the heard word floating in space, its size set by the score. The camera drifts slowly through the words for 20 seconds. Records: [paste]

**3 · Seen and heard.** A description for the node, with a device photograph as upstream image:

> A p5.js sketch on black, 1920 × 1080. The input image is a photograph from our device. Left half: the photograph as it is, captioned with what it shows. Right half: the same photograph, slowly overtaken by the category the device heard; the higher the score, the further it goes. Record: [paste one line]

---

## 4. Image to video

```text
keyframe or Creative Code frame ──► Video model ◄── Text (motion prompt) ──► clip, 5–10 s
```

The motion prompt describes one movement only: of the camera or of the subject.

> The camera slowly moves closer to this weathered wall. The dark patches gradually turn into moss and then back into stain, as if the surface cannot decide what it is. No people.

Video carries the shots the film turns on. The other generated shots stay as keyframes, held as stills in the edit.

Clips are saved as `G<shot>_<desc>.mp4`. Figma Weave, Kling AI and Hailuo AI take the same keyframe and motion prompt. Free clips keep the watermark.

Keyframes, sketches and clips replace the sketches in the Miro storyboard: the whole film on one board.

---

## Generated material

- **Inputs:** the group's own photographs and footage, and the Day 2 images from Openverse or Google Images (SerpAPI).
- **People:** no identifiable people in inputs or prompts.
- **In the film:** generated shots carry a small caption, `Generated`.

### Generation log

Every generated file has a row in the [generation log](generation_log_template.csv), kept with the Fuser canvas.

| Column | Content |
|---|---|
| `output_file` | `K03_wall_moss.png`, `C05_score_line.mp4`, `G03_wall_moss.mp4` |
| `tool` | Fuser, Figma Weave, Kling AI, Hailuo AI |
| `model` | The model node, or Creative Code |
| `date` | Date of the run |
| `input_files` | Reference images, keyframes, records |
| `prompt` | The full prompt, style prompt included, or the sketch file |
| `used_in_shot` | Shot number in the storyboard |
| `note` | keyframe, video, rejected; for Creative Code, the changes made to the example |

## Sensors

Further sensors, such as galvanic skin response (GSR), can add a bodily signal to the records.

🔗 [Pervasive Urbanism — Sensors and Actuators](https://github.com/PervasiveUrbanism/PervasiveUrbanism_25-26/tree/main/Skills%20Module%202%20Prosthetic%20Clouds/Skills%202%20-%20Day%202%20Sensors%20and%20Actuators)

---

[Previous session: deployment and field recording](../Day_03_Deployment_and_Recording/README.md) · [Next session: moving image and editing](../Day_05_Moving_Image_and_Editing/README.md)
