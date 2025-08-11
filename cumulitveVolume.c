// Assumes Profile Area tool configured
char* toolName = "Profile Area";
char* measurementName = "Area";
double area = 0;
double encoder_spacing = 3.0;
double volume;
double scale_factor = 764554857.984; // mm^3 per yd^3
double max_output_value = 2000000.0; // Max scaled volume in yd^3
double max_volume = max_output_value * scale_factor; // 2e15 mm^3

// Initialize memory on first frame or digital input 0 bit is high
if (Stamp_Frame() == 0 || (Stamp_Inputs() & 1) != 0) {
    Memory_Set64f(1, 0); // Volume
    Memory_Set64f(2, Stamp_Encoder()); // Initial encoder position
} else {
}

if (Measurement_NameExists(toolName, measurementName)) {
    int id = Measurement_Id(toolName, measurementName);
    int valid = Measurement_Valid(id); // Check if measurement is valid for this frame
   
    area = Measurement_Value(id);
    Output_SetAt(1, area, 1); // Debug: Output the raw area value
   
    if (valid == 1 && area != INVALID_VALUE) {
        // Get encoder direction
        double current_encoder = Stamp_Encoder();
        double prev_encoder = Memory_Get64f(2);
        double encoder_delta = -1 * (current_encoder - prev_encoder);
        double direction = 0; // -1=backward, 1=forward, 0=no movement
       
        if (encoder_delta > 0) {
            direction = 1; // Forward: count up
        } else if (encoder_delta < 0) {
            direction = -1; // Backward: count down
        }
       
        // Calculate tentative volume
        double tentative_volume = Memory_Get64f(1) + direction * area * encoder_spacing;
       
        // Clamp volume
        if (tentative_volume > max_volume) {
            volume = max_volume; // Clamp at max
        } else if (tentative_volume < 0) {
            volume = 0; // Clamp at 0
        } else {
            volume = tentative_volume; // Normal accumulation
        }
       
        Memory_Set64f(1, volume);
        Memory_Set64f(2, current_encoder); // Update encoder position
    } else {
        volume = Memory_Get64f(1); // Preserve previous volume
    }
} else {
    volume = INVALID_VALUE;
}

// Scale volume to m^3
float scaled_volume = volume / scale_factor; // mm^3 to m^3
Output_SetAt(0, scaled_volume, 1); // Output the scaled volume in m^3