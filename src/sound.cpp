// ═══════════════════════════════════════════════════════════════════
// ═══ STEP 2 — Sound: buzzer setup, tones & melodies ══════════════
// ═══════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include "sound.h"

void setupBuzzer() {
    // TODO: set pinMode for the buzzer pin (OUTPUT)
}

void playTone(int freq, int duration) {
    // TODO: call tone(), delay(), noTone() — or just delay() for rests (freq == 0)
}

void playMelody(const int notes[][2], int len) {
    // TODO: loop through notes and call playTone for each
}

void chirp(Mood m) {
    // TODO: play a short sound that matches the mood
}

// --- Pre-made melodies ------------------------------------------------
// Each melody is an array of {frequency, duration} pairs.
// Frequency 0 = rest (silence for that duration).

const int WAKE_TUNE[][2] = {
    {523, 150}, {523, 150}, {0, 50},
    {523, 150}, {0, 50}, {415, 150},
    {523, 200}, {659, 200}, {523, 200}
};
const int WAKE_TUNE_LEN = 9;

const int HAPPY_TUNE[][2] = {
    {523, 120}, {659, 120}, {784, 120},
    {880, 200}, {784, 120}, {880, 300}
};
const int HAPPY_TUNE_LEN = 6;

const int SAD_TUNE[][2] = {
    {440, 300}, {392, 300}, {349, 400}, {330, 500}
};
const int SAD_TUNE_LEN = 4;

const int VICTORY_TUNE[][2] = {
    {523, 150}, {523, 150}, {523, 150},
    {659, 400}, {587, 150}, {523, 150},
    {587, 150}, {659, 200}, {523, 200}
};
const int VICTORY_TUNE_LEN = 9;

const int DEATH_TUNE[][2] = {
    {294, 400}, {277, 400}, {262, 400}, {247, 600}
};
const int DEATH_TUNE_LEN = 4;
