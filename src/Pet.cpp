#include "Pet.h"
#include "sprites.h"

// ── Constants ─────────────────────────────────────────────────────────────────

static const int STAT_MAX        = 100;
static const int STAT_MIN        = 0;
static const int HAPPY_THRESHOLD = 60;
static const int SAD_THRESHOLD   = 40;

static const unsigned long BACKLIGHT_TIMEOUT = 30000;  // ms
static const unsigned long ALERT_COOLDOWN    = 15000;  // ms
static const unsigned long MSG_DURATION      = 2000;   // ms

// Alert tone — passed to the global playTone() defined in sound.cpp
static const int ALERT_FREQ     = 880;
static const int ALERT_DURATION = 150;

// Display pins
static const int PIN_DC  = 8;
static const int PIN_CS  = 9;
static const int PIN_RST = 12;
static const int PIN_BL  = 13;

// Sprite dimensions and layout
static const int SPRITE_W = 120;
static const int SPRITE_H = 120;
static const int SPRITE_X = (240 - SPRITE_W) / 2;
static const int SPRITE_Y = 20;
static const int TEXT_Y   = SPRITE_Y + SPRITE_H + 10;
static const int IND_Y    = 240 - 30;
static const int IND_R    = 8;

// Button indicator layout
static const int IND_X[3]        = { 40, 120, 200 };
static const uint16_t IND_COL[3] = { 0xF800, 0xFFE0, 0x07E0 }; // RED YELLOW GREEN
static const char* IND_LABEL[3]  = { "Food", "Water", "Move" };

// ── Weak stubs (overridden when participants add sound.cpp / leds.cpp) ───────
__attribute__((weak)) void playTone(int freq, int duration) { (void)freq; (void)duration; }
__attribute__((weak)) void setLed(int r, int g, int b) { (void)r; (void)g; (void)b; }

// ── Constructor ──────────────────────────────────────────────────────────────

Pet::Pet()
    : _lcd(&SPI1, PIN_CS, PIN_DC, PIN_RST),
      food(STAT_MAX), water(STAT_MAX), energy(STAT_MAX),
      _decayEnabled(false), _backlightOn(true),
      _lastMood(Mood::HAPPY), _currentMsg(""),
      _lastDecay(0), _lastActivity(0), _msgClearAt(0), _lastAlert(0)
{}

// ── Public API ───────────────────────────────────────────────────────────────

void Pet::begin() {
    PetConfig defaults;
    begin(defaults);
}

void Pet::begin(PetConfig cfg) {
    _cfg = cfg;

    pinMode(PIN_BL, OUTPUT);
    digitalWrite(PIN_BL, HIGH);

    SPI1.begin();
    _lcd.init(240, 240);
    _lcd.setRotation(2);
    _lcd.fillScreen(0x0000);

    _lastDecay    = millis();
    _lastActivity = millis();

    _drawIndicators();
    _redraw();
}

void Pet::update() {
    unsigned long now = millis();

    // Stat decay
    if (_decayEnabled && (now - _lastDecay >= (unsigned long)_cfg.decayInterval)) {
        _decayAll();
        _lastDecay = now;
    }

    // Clear message after MSG_DURATION
    if (_currentMsg.length() > 0 && now >= _msgClearAt) {
        _currentMsg = "";
        _clearTextZone();
    }

    // Backlight timeout
    if (_backlightOn && (now - _lastActivity >= BACKLIGHT_TIMEOUT)) {
        digitalWrite(PIN_BL, LOW);
        _backlightOn = false;
    }

    // Redraw if mood changed
    Mood newMood = _computeMood();
    if (newMood != _lastMood) {
        _redraw();
    }

    // Alert with cooldown
    if (needsAlert() && (now - _lastAlert >= ALERT_COOLDOWN)) {
        _lastAlert = now;
        _triggerAlert();
    }
}

void Pet::feed() {
    food = min(STAT_MAX, food + _cfg.feedAmount);
    _lastActivity = millis();
    if (!_backlightOn) { digitalWrite(PIN_BL, HIGH); _backlightOn = true; }

    String msg;
    switch (_computeMood()) {
        case Mood::HAPPY: { const char* p[] = {"Yum!", "So full!", "Mmm!", "Delicious!"}; msg = p[random(4)]; break; }
        case Mood::SAD:   { const char* p[] = {"Finally...", "Needed that", "More please..."}; msg = p[random(3)]; break; }
        case Mood::DEAD:  msg = "*munch*"; break;
        default:          { const char* p[] = {"Thanks!", "Nom!", "Tasty!"}; msg = p[random(3)]; break; }
    }
    _showMessage(msg);
    _redraw();
}

void Pet::drink() {
    water = min(STAT_MAX, water + _cfg.drinkAmount);
    _lastActivity = millis();
    if (!_backlightOn) { digitalWrite(PIN_BL, HIGH); _backlightOn = true; }

    String msg;
    switch (_computeMood()) {
        case Mood::HAPPY: { const char* p[] = {"Ahhh!", "Refreshing!", "Glug glug!", "Hydrated!"}; msg = p[random(4)]; break; }
        case Mood::SAD:   { const char* p[] = {"So thirsty...", "Needed that...", "*sip*"}; msg = p[random(3)]; break; }
        case Mood::DEAD:  msg = "*slurp*"; break;
        default:          { const char* p[] = {"Thanks!", "Glug!", "Nice!"}; msg = p[random(3)]; break; }
    }
    _showMessage(msg);
    _redraw();
}

void Pet::exercise(int boost) {
    energy = min(STAT_MAX, energy + constrain(boost, 0, _cfg.exerciseCap));
    _lastActivity = millis();
    if (!_backlightOn) { digitalWrite(PIN_BL, HIGH); _backlightOn = true; }
    _showMessage("Nice work!");
    _redraw();
}

void Pet::enableDecay() {
    _decayEnabled = true;
}

String Pet::catchphrase() {
    switch (_computeMood()) {
        case Mood::HAPPY: {
            const char* p[] = {"Yay!", "So happy!", "Wheee!", "Love this!", "Best day!"};
            return p[random(5)];
        }
        case Mood::SAD: {
            const char* p[] = {"Hungry...", "Feed me...", "So tired...", "Help..."};
            return p[random(4)];
        }
        case Mood::DEAD: {
            const char* p[] = {"x_x", "...", "zzz"};
            return p[random(3)];
        }
        default: {
            const char* p[] = {"Hey!", "Sup", "I'm ok", "...", "Hi there"};
            return p[random(5)];
        }
    }
}

void Pet::say(String text) {
    _showMessage(text);
}

Mood Pet::mood() {
    return _computeMood();
}

bool Pet::needsAlert() {
    return (food < _cfg.alertThreshold ||
            water < _cfg.alertThreshold ||
            energy < _cfg.alertThreshold);
}

// ── Private helpers ──────────────────────────────────────────────────────────

Mood Pet::_computeMood() {
    if (food <= STAT_MIN || water <= STAT_MIN || energy <= STAT_MIN)
        return Mood::DEAD;
    if (food  >= HAPPY_THRESHOLD &&
        water >= HAPPY_THRESHOLD &&
        energy >= HAPPY_THRESHOLD)
        return Mood::HAPPY;
    if (food  < SAD_THRESHOLD ||
        water < SAD_THRESHOLD ||
        energy < SAD_THRESHOLD)
        return Mood::SAD;
    return Mood::OKAY;
}

void Pet::_decayAll() {
    food   = max(STAT_MIN, food   - _cfg.decayRate);
    water  = max(STAT_MIN, water  - _cfg.decayRate);
    energy = max(STAT_MIN, energy - _cfg.decayRate);
}

void Pet::_redraw() {
    _lastMood = _computeMood();
    _drawSprite(_lastMood);
    if (_currentMsg.length() > 0) {
        _drawText(_currentMsg);
    }
}

void Pet::_drawSprite(Mood m) {
    const uint8_t* data;
    switch (m) {
        case Mood::HAPPY: data = SPRITE_HAPPY; break;
        case Mood::SAD:   data = SPRITE_SAD;   break;
        case Mood::DEAD:  data = SPRITE_DEAD;  break;
        default:          data = SPRITE_OKAY;  break;
    }

    _lcd.startWrite();
    _lcd.setAddrWindow(SPRITE_X, SPRITE_Y, SPRITE_W, SPRITE_H);
    uint32_t total = (uint32_t)SPRITE_W * SPRITE_H;
    for (uint32_t i = 0; i < total; i++) {
        uint8_t hi = pgm_read_byte(&data[i * 2]);
        uint8_t lo = pgm_read_byte(&data[i * 2 + 1]);
        _lcd.SPI_WRITE16((hi << 8) | lo);
    }
    _lcd.endWrite();
}

void Pet::_drawIndicators() {
    for (int i = 0; i < 3; i++) {
        _lcd.fillCircle(IND_X[i], IND_Y, IND_R, IND_COL[i]);
        int16_t x1, y1; uint16_t w, h;
        _lcd.getTextBounds(IND_LABEL[i], 0, 0, &x1, &y1, &w, &h);
        _lcd.setCursor(IND_X[i] - w / 2, IND_Y + IND_R + 4);
        _lcd.setTextColor(0xFFFF);
        _lcd.setTextSize(1);
        _lcd.print(IND_LABEL[i]);
    }
}

void Pet::_showMessage(String msg) {
    _currentMsg  = msg;
    _msgClearAt  = millis() + MSG_DURATION;
    _drawText(msg);
}

void Pet::_drawText(String msg) {
    _clearTextZone();
    _lcd.setTextColor(0xFFFF);
    _lcd.setTextSize(2);
    int16_t x1, y1; uint16_t w, h;
    _lcd.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
    _lcd.setCursor((240 - w) / 2, TEXT_Y);
    _lcd.print(msg);
}

void Pet::_clearTextZone() {
    _lcd.fillRect(0, TEXT_Y, 240, 30, 0x0000);
}

void Pet::_triggerAlert() {
    const char* msgs[] = {"Hey...", "I'm hungry...", "Don't forget me!", "Hellooo?"};
    _showMessage(msgs[random(4)]);
    playTone(ALERT_FREQ, ALERT_DURATION);
    setLed(1, 0, 0);
    delay(200);
    setLed(0, 0, 0);
    _drawSprite(Mood::SAD);
}
