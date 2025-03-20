import math
import wave
import struct

# Read the complete WAV file
with open("cast_away_fire.wav", "rb") as f:
    wav_data = f.read()

# Prepare the C array for the full WAV file
output_lines_full = []
output_lines_full.append("const uint8_t cast_away_fire[] = {")
line_length = 12
line = "   "
for i, b in enumerate(wav_data):
    line += f"0x{b:02X}, "
    if (i + 1) % line_length == 0:
        output_lines_full.append(line)
        line = "   "
if line.strip():
    output_lines_full.append(line)
output_lines_full.append("};")
output_lines_full.append(f"const uint32_t cast_away_fire_len = {len(wav_data)};")
output_str_full = "\n".join(output_lines_full)

# Write the full array to a text file
with open("cast_away_fire.h", "w") as outfile:
    outfile.write(output_str_full)


# Optionally, print the outputs to the console
print("Full WAV array saved to cast_away_fire.txt")
print("PCM data array saved to cast_away_fire.txt")
