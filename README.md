# MISHEARD CITY
### Edge AI as an Instrument of Urban Research

![Misheard City](assets/misheard-city.jpg)
*Misheard City*

**B.Pro Intro Workshop · 5–16 October 2026 · Kunpeng Lei**

Misheard City investigates how **visual classification** shapes our perception and representation of urban life. When we train a model to recognise plants, infrastructure or weathered surfaces, we give particular details a place within our description of the city. The categories carry our interests; field encounters expose their limits and suggest other ways of seeing.

The workshop grows out of a **wearable visual-recognition device** developed and deployed in Venice, where classifications of the surroundings and bodily signals shaped a responsive soundscape. Over two weeks, each group develops its own enquiry through a **small device**, **field observations** and an **experimental film**. AI supports the reading of theoretical texts, programming, design and moving-image practice.

**No previous coding experience is required. All disciplinary backgrounds are welcome.** Each group has one Seeed Studio **XIAO ESP32S3 Sense** kit.

[Dates](#dates) · [Workshop modules](#workshop-modules) · [Support & resources](#support--resources)

# DATES

| Day | Date | Module | Topic | Time |
| --- | --- | --- | --- | --- |
| Introduction | Mon 05.10 | — | Project introduction and workshop selection | 10:25–10:35, online |
| **Day 1** | **Tue 06.10** | **1** | **Arduino and the XIAO: programming, AI-assisted iteration, image capture** | **09:30–13:00** |
| Day 2 | Thu 08.10 | 2 | Categories and datasets: image search, review, first training | 13:00–17:00 |
| Day 3 | Fri 09.10 | 2 | First model, deployment, field recording and storyboard | 13:00–17:00 |
| Day 4 | Mon 12.10 | 3 | Keyframes, Creative Code and image to video in Fuser | 10:00–13:00 |
| Day 5 | Tue 13.10 | 3 | Generated shots, editing and sound | 09:00–11:00 · 13:00–15:00 |
| Day 6 | Wed 14.10 | 4 | Film presentation 1: draft screenings | 09:00–13:00 |
| Day 7 | Thu 15.10 | 4 | Film presentation 2: revised screenings | 09:00–11:00 |
| Submission | Thu 15.10 | 4 | Final film | By 14:00 |
| Final presentation | Fri 16.10 | — | Screening of the workshop compilation | From 09:30 |

All times are London time. Teaching rooms will be announced separately.

# WORKSHOP MODULES

## Module 1: Programming and AI-assisted practice

We start with the **Arduino environment** and the **XIAO ESP32S3 Sense**: upload a program, change its behaviour, read serial messages and capture images. **AI-assisted coding** is a process of explaining, revising and checking: describe a small change, examine the suggested code and compare the result on the board.

### Day 1: Arduino and the XIAO ESP32S3 Sense

Each group introduces an interest in the city. We then program the board, make one change with AI assistance and save the first photographs to the microSD card. The **research task** for Day 2 is set at the end of the session.

[Introduction to Arduino](Day_01_Introduction/README.md) · [Camera and microSD](Day_01_Introduction/04_Camera_and_SD/README.md) · [Research task](Day_01_Introduction/05_Research/README.md)

## Module 2: Classification, datasets and edge AI

Develop a research question and a small set of **visual categories**. We collect candidate images through an **image-search API**, keep their sources and review them together: search results need judgement before they become training labels. With **Edge Impulse** we train a model and deploy it on the device, where classification decisions, ambiguous cases and errors become research material.

### Day 2: Categories and datasets

Groups present their proposals and refine their categories. We collect images with **Openverse** and **Google Images**, clean and review them in **Google Colab** and export a dataset for Edge Impulse.

[Categories and datasets](Day_02_Datasets_and_Training/README.md) · [Openverse](Day_02_Datasets_and_Training/01a_Openverse/README.md) · [Google Images](Day_02_Datasets_and_Training/01b_Google_Images/README.md) · [Dataset review](Day_02_Datasets_and_Training/02_Dataset_Review/README.md) · [Edge Impulse training](Day_02_Datasets_and_Training/03_Edge_Impulse_Training/README.md)

### Day 3: Model deployment, field recording and storyboard

The first model runs on the XIAO, and its readings meet the surfaces and objects in front of the camera. A **field recorder** saves each photograph with its classification scores. The first readings complete the group's AI-drafted **3-minute script**, which is drawn as a **storyboard** in Miro and listed shot by shot for field collection.

[Model deployment](Day_03_Deployment_and_Recording/README.md) · [Field recorder](Day_03_Deployment_and_Recording/README.md#field-recorder) · [AI script](Day_03_Deployment_and_Recording/README.md#ai-script) · [Storyboard](Day_03_Deployment_and_Recording/README.md#storyboard) · [Field collection](Day_03_Deployment_and_Recording/README.md#field-collection)

## Module 3: Fieldwork and interpretation

The device goes into an urban setting, and its records enter into dialogue with our observations. **Python notebooks** help organise and interpret the records: model scores and field descriptions are different kinds of evidence.

### Day 4: Fuser workflow and Creative Code

The storyboard and the field material continue in **Fuser**, a node-based canvas: keyframes for the shots the records cannot show, **Creative Code** sketches driven by the device records, then image to video for the shots the film turns on.

*Materials will be published here before the session on Mon 12.10.*

### Day 5: Moving image, editing and sound

The generated shots and data layers join the device records and phone footage in the edit, with field sound, voice-over and music. Creative Code recordings and a notebook draw the records as layers, timelines and contact sheets.

*Materials will be published here before the session on Tue 13.10.*

## Module 4: Moving image and discussion

Each group makes a **standalone film of 3 minutes**: see the [film requirements](#film-requirements).

### Days 6–7: Film presentations

The draft film is screened and discussed on 14 October, along the six points of the film requirements, and turned into a revision list. The revised film and its change note follow on 15 October, with the final export and submission.

*Materials will be published here before the session on Wed 14.10.*

**Submit the final film by 14:00 on 15 October.** The films are screened together on 16 October.

### Film requirements

**The film**

- A standalone film of **3 minutes**, understandable without a live presentation.
- It communicates the **research question**, the **categories** and how they were chosen, **something learned through the experiment**, and the group's **artistic interpretation**.
- It shows at least **two records from the device as they are**: the photograph with its label and score.
- Generated images and video are **marked as generated**, and the relationship between observations and generated imagery is understandable.
- Inputs for generated material are the group's own photographs and footage and the images collected on Day 2 through Openverse or SerpAPI. Identifiable people do not appear.

**Submission**

| Item | Format |
|---|---|
| Final film | MP4 (H.264), 1920 × 1080, file name `MisheardCity_<Group>_<Title>.mp4` |
| Credits | At the end of the film: title, members' names, music and sound sources |
| Still | One frame from the film, JPG |
| Synopsis | Up to 100 words: question, method, finding |
| Generation log | The completed generation log |

# SUPPORT & RESOURCES

## GitHub

This repository is the **central access point** for all workshop materials: each lesson is a README page with its code, notebooks and images. **Code → Download ZIP** gives a local copy; [GitHub Desktop](https://desktop.github.com/) keeps one up to date.

`.ino` files open in **Arduino IDE**, each sketch inside its matching folder. Python notebooks open in **Google Colab**; [VS Code](Day_02_Datasets_and_Training/README.md#vs-code-on-your-own-computer) runs them on a laptop. Group variations stay separate from the supplied examples, with a note of the code, data and model version behind each result.

## Readings

| Theme | Reading | Connection to the workshop |
| --- | --- | --- |
| Perception and position | Donna Haraway, [*Situated Knowledges*](https://commons.princeton.edu/hum583-f21/wp-content/uploads/sites/283/2021/08/Haraway-Situated-Knowledges.pdf), 1988 | The position and instruments through which an observation is made |
| Classification | Geoffrey C. Bowker and Susan Leigh Star, [*Sorting Things Out: Classification and Its Consequences*](https://mitpress.mit.edu/9780262024617/sorting-things-out/), 1999, introduction | The decisions and consequences involved in organising categories |
| Data and representation | Johanna Drucker, [*Humanities Approaches to Graphical Display*](https://digitalhumanities.org/dhq/vol/5/1/000091/000091.html), 2011 | How interpretation enters the construction and display of data |

## Resources

Software, documentation and image sources are listed on a separate [resources](Resources.md) page.
