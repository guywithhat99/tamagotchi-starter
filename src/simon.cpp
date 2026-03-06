// ═══════════════════════════════════════════════════════════════════
// ═══ STEP 3 — Simon Says mini-game (provided code — copy as-is) ══
// ═══════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include "simon.h"
#include "leds.h"
#include "sound.h"

int playSimon() {
    const int MAX_SEQ = 10;

    int colours[3][3] = {
        {1, 0, 0},   // red
        {0, 1, 0},   // green
        {1, 1, 0},   // yellow
    };
    int buttons[3] = {22, 19, 21};    // red, green, yellow
    int tones[3]   = {440, 523, 659}; // A4, C5, E5

    int seq[MAX_SEQ];
    int rounds = 0;

    while (rounds < MAX_SEQ) {
        // Add one new step to the sequence
        seq[rounds] = random(3);
        int seqLen = rounds + 1;

        // Flash the full sequence
        delay(500);
        for (int i = 0; i < seqLen; i++) {
            int c = seq[i];
            setLed(colours[c][0], colours[c][1], colours[c][2]);
            playTone(tones[c], 400);
            setLed(0, 0, 0);
            delay(200);
        }
        delay(300);

        // Read player input — 5-second window per step
        bool passed = true;
        for (int i = 0; i < seqLen; i++) {
            uint32_t start   = millis();
            bool     waiting = true;
            while (waiting && millis() - start < 5000) {
                for (int b = 0; b < 3; b++) {
                    if (digitalRead(buttons[b])) {
                        setLed(colours[b][0], colours[b][1], colours[b][2]);
                        playTone(tones[b], 200);
                        setLed(0, 0, 0);
                        delay(100);
                        if (b != seq[i]) { passed = false; waiting = false; break; }
                        waiting = false;
                        break;
                    }
                }
                delay(10);
            }
            if (waiting) { passed = false; }
            if (!passed) break;
        }

        if (!passed) break;

        // Round complete — quick success flash
        setLed(1, 1, 1);
        delay(150);
        setLed(0, 0, 0);

        rounds++;
    }

    // Victory flash if maxed out
    if (rounds == MAX_SEQ) {
        for (int i = 0; i < 5; i++) {
            setLed(1, 1, 1); delay(80);
            setLed(0, 0, 0); delay(80);
        }
    }

    return rounds;
}
