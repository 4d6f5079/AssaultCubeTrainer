#pragma once
#include <iostream>
#include <cstdint>
#include <vector>

// add this base address offset to the exe base address of the game
const uintptr_t SINGLEPLAYER_MULTIPLAYER_STATIC_OFFSET_ADDR = 0x10F4F4;

// health and armor offsets
const std::vector<uintptr_t> HEALTH_OFFSET = { 0xF8 };
const std::vector<uintptr_t> ARMOR_OFFSET = { 0xFC };

// weapons and grenades offsets
// example finding ammo address: base (static) addr -> offset 1 -> offset 2 -> ... -> ammo address -> value
const std::vector<uintptr_t> AR_AMMO_OFFSET = { 0x374, 0x14, 0x0 };
const std::vector<uintptr_t> AR_AMMO_OFFSET2 = { 0x148 };
const std::vector<uintptr_t> PISTOL_AMMO_OFFSET = { 0x13C };
const std::vector<uintptr_t> GRENADE_OFFSET = { 0x158 };

// coordinates of mouse
const std::vector<uintptr_t> X_MOUSE_COORDINATE_OFFSET = { 0x40 };
const std::vector<uintptr_t> Y_MOUSE_COORDINATE_OFFSET = { 0x44 };

