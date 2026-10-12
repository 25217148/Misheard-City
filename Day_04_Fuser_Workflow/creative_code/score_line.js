// Score line: the device's top score over a session, coloured by label.
// Each record: [minute in the session, label, top score], from the field log.
const records = [
  [0, "stain", 0.62],
  [2, "stain", 0.55],
  [4, "moss", 0.71],
  [6, "moss", 0.81],
  [8, "rust", 0.47],
  [10, "moss", 0.66],
  [12, "moss", 0.74],
];

// One colour per category, as red, green, blue.
const colours = {
  moss: [120, 200, 120],
  stain: [200, 160, 90],
  rust: [210, 90, 60],
  paint: [120, 160, 230],
};
const colourOf = (label) => colours[label] || [255, 255, 255]; // white for a label not listed

const seconds = 20; // time for the line to cross the frame

function setup() {
  createCanvas(1920, 1080);
  textFont("monospace");
  textSize(36);
}

function draw() {
  background(0); // black, for the Screen blend mode in the edit

  const shown = min(millis() / 1000 / seconds, 1) * records.length; // records drawn so far
  const lastMinute = records[records.length - 1][0];

  // Record → position: minute across, score up.
  const x = (r) => map(r[0], 0, lastMinute, 160, width - 160);
  const y = (r) => map(r[2], 0, 1, height - 160, 160);

  // The first label, then the line from record to record.
  noStroke();
  fill(colourOf(records[0][1]));
  text("heard as " + records[0][1], x(records[0]) + 16, y(records[0]) - 40);

  for (let i = 1; i < shown && i < records.length; i++) {
    const a = records[i - 1];
    const b = records[i];
    stroke(colourOf(b[1]));
    strokeWeight(6);
    line(x(a), y(a), x(b), y(b));

    // A new label is written where it changes.
    if (b[1] !== a[1]) {
      noStroke();
      fill(colourOf(b[1]));
      text("heard as " + b[1], x(b) + 16, y(b) - 40);
    }
  }

  // The latest record: its score in large type.
  const current = records[max(0, min(floor(shown) - 1, records.length - 1))];
  noStroke();
  fill(255);
  textSize(96);
  text(nf(current[2], 1, 2), 160, 200);
  textSize(36);
}
