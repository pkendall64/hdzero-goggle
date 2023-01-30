static char *module_standard_channels[] = {
    "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8",
    "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8",
    "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
    "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8",
    "L1", "L2", "L3", "L4", "L5", "L6", "L7", "L8",
};

char *module_standard_channel_name(int channel) {
    return module_standard_channels[channel];
}