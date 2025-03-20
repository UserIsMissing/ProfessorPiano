import math
import wave
import struct

# Parameters for WAV file generation
samplerate = 48000     # 48 kHz sample rate
duration_s = 0.5     # 0.025 seconds
num_frames = int(samplerate * duration_s)
# 
freq = 494           # B4
# freq = 466      	   # A4/B4
# freq = 440           # A4
# freq = 415           # G4/A4
# freq = 391           # G4
# freq = 370           # F4/G4
# freq = 349           # F4
# freq = 330           # E4
# freq = 311           # D4/E4
# freq = 293           # D4
# freq = 277           # C4/D4
# freq = 261           # C4

amplitude = 0.1        # amplitude scale [0..1]

# Convert freq to a string without the decimal point, for filenames:
freq_str = str(freq).replace('.', '_')

# Construct filenames that include frequency:
wav_file_name = f"beep_stereo_{freq_str}.wav"
header_file_name = f"beep_stereo_{freq_str}_array.h"

# --- Generate the WAV file ---
with wave.open(wav_file_name, 'wb') as wf:
    wf.setnchannels(2)       # stereo
    wf.setsampwidth(2)       # 16 bits per sample
    wf.setframerate(samplerate)
    for i in range(num_frames):
        t = i / samplerate
        sample_val = amplitude * math.sin(2 * math.pi * freq * t)
        # Scale to 16-bit range
        val_int = int(sample_val * 32767)
        # Pack as little-endian 16-bit stereo (L, R)
        data = struct.pack('<hh', val_int, val_int)
        wf.writeframesraw(data)

# --- Convert the entire WAV file into a C array and save to file ---

# Read the generated WAV file
with open(wav_file_name, "rb") as f:
    wav_data = f.read()

# Prepare the C array for the full WAV file
output_lines_full = []
output_lines_full.append(f"const uint8_t beep_stereo_wav_{freq_str}[] = {{")
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
output_lines_full.append(f"const uint32_t beep_stereo_wav_len_{freq_str} = {len(wav_data)};")
output_str_full = "\n".join(output_lines_full)

# Write the full array to a header file
with open(header_file_name, "w") as outfile:
    outfile.write(output_str_full)

# Optionally, print to the console to confirm
print(f"WAV file saved to: {wav_file_name}")
print(f"Array header saved to: {header_file_name}")
