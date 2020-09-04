#include <stdio.h>
#include <stdlib.h>
#include "radio.h"
#include "simvars.h"
#include "knobs.h"

radio::radio(int xPos, int yPos, int size) : instrument(xPos, yPos, size)
{
    setName("Radio");
    addVars();

#ifndef _WIN32
    // Only have hardware knobs on Raspberry Pi
    if (globals.hardwareKnobs) {
        addKnobs();
    }
#endif

    resize();
}

/// <summary>
/// Destroy and recreate all bitmaps as instrument has been resized
/// </summary>
void radio::resize()
{
    destroyBitmaps();

    // Create bitmaps scaled to correct size (original size is 800)
    scaleFactor = size / 1600.0f;

    // 0 = Original (loaded) bitmap
    ALLEGRO_BITMAP* orig = loadBitmap("radio.bmp");
    addBitmap(orig);

    if (bitmaps[0] == NULL) {
        return;
    }

    // 1 = Destination bitmap (all other bitmaps get assembled to here)
    ALLEGRO_BITMAP* bmp = al_create_bitmap(size, size / 4);
    addBitmap(bmp);

    // 2 = Main
    bmp = al_create_bitmap(size, size / 4);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 0, 0, 1600, 400, 0, 0, size, size / 4, 0);
    addBitmap(bmp);

    al_set_target_backbuffer(globals.display);
}

/// <summary>
/// Draw the instrument at the stored position
/// </summary>
void radio::render()
{
    if (bitmaps[0] == NULL) {
        return;
    }

    // Use normal blender
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // Draw stuff into dest bitmap
    al_set_target_bitmap(bitmaps[1]);

    // Add main
    al_draw_bitmap(bitmaps[2], 0, 0, 0);

    // Position dest bitmap on screen
    al_set_target_backbuffer(globals.display);
    al_draw_bitmap(bitmaps[1], xPos, yPos, 0);

    if (!globals.active) {
        dimInstrument();
    }
}

/// <summary>
/// Fetch flightsim vars and then update all internal variables
/// that affect this instrument.
/// </summary>
void radio::update()
{
    // Check for position or size change
    long *settings = globals.simVars->readSettings(name, xPos, yPos, size);

    xPos = settings[0];
    yPos = settings[1];

    if (size != settings[2]) {
        size = settings[2];
        resize();
    }

#ifndef _WIN32
    // Only have hardware knobs on Raspberry Pi
    if (globals.hardwareKnobs) {
        updateKnobs();
    }
#endif

    // Get latest FlightSim variables
    SimVars* simVars = &globals.simVars->simVars;

    // Calculate values
    angle = simVars->adiBank / 100.0;
}

/// <summary>
/// Add FlightSim variables for this instrument (used for simulation mode)
/// </summary>
void radio::addVars()
{
    //globals.simVars->addVar(name, "Value", false, 1, 0);
}

#ifndef _WIN32

void radio::addKnobs()
{
    // BCM GPIO 2 and 3
    calKnob = globals.hardwareKnobs->add(2, 3, -100, 100, 0);
}

void radio::updateKnobs()
{
    // Read knob for new instrument calibration
    int val = globals.hardwareKnobs->read(calKnob);

    if (val != INT_MIN) {
        // Convert knob value to new instrument value (adjust for desired sensitivity)
        double simVarVal = val / 10;

        // Update new instrument variable
        //globals.simVars->write("simvar", simVarVal);
    }
}

#endif // !_WIN32
