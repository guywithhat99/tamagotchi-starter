// ═══════════════════════════════════════════════════════════════════
// ═══ STEP 2 — Sound: buzzer setup, tones & melodies ══════════════
// ═══════════════════════════════════════════════════════════════════

#pragma once
#include "Pet.h"

void setupBuzzer();
void playTone(int freq, int duration);
void playMelody(const int notes[][2], int len);
void chirp(Mood m);

// Pre-made melodies — use these or compose your own!
extern const int WAKE_TUNE[][2];
extern const int WAKE_TUNE_LEN;

extern const int HAPPY_TUNE[][2];
extern const int HAPPY_TUNE_LEN;

extern const int SAD_TUNE[][2];
extern const int SAD_TUNE_LEN;

extern const int VICTORY_TUNE[][2];
extern const int VICTORY_TUNE_LEN;

extern const int DEATH_TUNE[][2];
extern const int DEATH_TUNE_LEN;
