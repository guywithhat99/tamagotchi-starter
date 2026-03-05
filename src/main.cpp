#include <Arduino.h>
#include "Pet.h"

Pet pet;

void setup() {
    Serial.begin(115200);
    Serial.println("Pet alive!");
    pet.begin();
}

void loop() {
    Serial.println("Updating pet...");
    pet.update();
    delay(50);
}
