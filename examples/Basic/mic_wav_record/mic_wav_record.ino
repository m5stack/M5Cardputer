
/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX@^0.2.3: https://github.com/m5stack/M5GFX
 * M5Cardputer@^1.0.3: https://github.com/m5stack/M5Cardputer
 */

#include <M5Cardputer.h>
#include <SPI.h>
#include <SD.h>

#define SD_SPI_SCK_PIN  (40)
#define SD_SPI_MISO_PIN (39)
#define SD_SPI_MOSI_PIN (14)
#define SD_SPI_CS_PIN   (12)

static constexpr const size_t record_number     = 512;
static constexpr const size_t record_length     = 240;
static constexpr const size_t record_size       = record_number * record_length;
static constexpr const size_t record_samplerate = 16000;

static int16_t prev_y[record_length];
static int16_t prev_h[record_length];
static size_t rec_record_idx  = 2;
static size_t draw_record_idx = 0;
static int16_t* rec_data;

static uint32_t file_counter     = 0;
static uint8_t selectedFileIndex = 0;
static std::vector<String> wavFiles;

// WAV文件头部定义
struct WAVHeader {
    char riff[4]           = {'R', 'I', 'F', 'F'};
    uint32_t fileSize      = 0;
    char wave[4]           = {'W', 'A', 'V', 'E'};
    char fmt[4]            = {'f', 'm', 't', ' '};
    uint32_t fmtSize       = 16;
    uint16_t audioFormat   = 1;
    uint16_t numChannels   = 1;
    uint32_t sampleRate    = record_samplerate;
    uint32_t byteRate      = record_samplerate * sizeof(int16_t);
    uint16_t blockAlign    = sizeof(int16_t);
    uint16_t bitsPerSample = 16;
    char data[4]           = {'d', 'a', 't', 'a'};
    uint32_t dataSize      = 0;
};

bool saveWAVToSD(int16_t* data, size_t dataSize);  // Save the WAV file to the SD card.
void scanAndDisplayWAVFiles(void);                 // Scan and display the WAV files on the SD card on the screen.
bool playWAVFileFromSD(void);                      // Play the WAV files on the SD card.
bool playSelectedWAVFile(const String& fileName);  // Play the selected WAV file.
void playWAV(void);                                // Play the WAV file.
void updateDisplay(std::vector<String> wavFiles, uint8_t selectedFileIndex);  // Update the displayed information.
void setup(void)
{
    auto cfg = M5.config();

    M5Cardputer.begin(cfg);
    Serial.begin(115200);
    M5Cardputer.Display.startWrite();
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextDatum(top_center);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setFont(&fonts::FreeSansBoldOblique12pt7b);

    // SD Card Initialization
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        printf("Card failed, or not present\r\n");
        while (1);
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        printf("No SD card attached\r\n");
        return;
    }
    printf("SD Card Type: ");
    if (cardType == CARD_MMC) {
        printf("MMC\r\n");
    } else if (cardType == CARD_SD) {
        printf("SDSC\r\n");
    } else if (cardType == CARD_SDHC) {
        printf("SDHC\r\n");
    } else {
        printf("UNKNOWN\r\n");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    printf("SD Card Size: %lluMB\r\n", cardSize);

    rec_data = (typeof(rec_data))heap_caps_malloc(record_size * sizeof(int16_t), MALLOC_CAP_8BIT);
    memset(rec_data, 0, record_size * sizeof(int16_t));
    M5Cardputer.Speaker.setVolume(255);
    M5Cardputer.Speaker.end();
    M5Cardputer.Mic.begin();

    scanAndDisplayWAVFiles();
    updateDisplay(wavFiles, selectedFileIndex);
}

void loop(void)
{
    M5Cardputer.update();

    // Press BtnA to start recording. After recording ends, the file will be saved to the SD card. If a file with the
    // same name already exists on the SD card, the new file will overwrite the existing one.
    if (M5Cardputer.BtnA.wasClicked()) {
        M5Cardputer.Display.clear();
        // Record
        if (M5Cardputer.Mic.isEnabled()) {
            M5Cardputer.Display.fillCircle(70, 15, 8, RED);
            M5Cardputer.Display.drawString("REC", 120, 3);
            static constexpr int shift = 6;
            for (uint16_t i = 0; i < record_number; i++) {
                auto data = &rec_data[i * record_length];
                if (M5Cardputer.Mic.record(data, record_length, record_samplerate)) {
                    if (i >= 2) {
                        data      = &rec_data[(i - 2) * record_length];
                        int32_t w = M5Cardputer.Display.width();
                        if (w > record_length - 1) {
                            w = record_length - 1;
                        }
                        for (int32_t x = 0; x < w; ++x) {
                            M5Cardputer.Display.writeFastVLine(x, prev_y[x], prev_h[x], TFT_BLACK);
                            int32_t y1 = (data[x] >> shift);
                            int32_t y2 = (data[x + 1] >> shift);
                            if (y1 > y2) {
                                int32_t tmp = y1;
                                y1          = y2;
                                y2          = tmp;
                            }
                            int32_t y = ((M5Cardputer.Display.height()) >> 1) + y1;
                            int32_t h = ((M5Cardputer.Display.height()) >> 1) + y2 + 1 - y;
                            prev_y[x] = y;
                            prev_h[x] = h;
                            M5Cardputer.Display.writeFastVLine(x, prev_y[x], prev_h[x], WHITE);
                        }
                    }
                }
                M5Cardputer.Display.display();
                M5Cardputer.Display.fillCircle(70, 15, 8, RED);
                M5Cardputer.Display.drawString("REC", 120, 3);
            }

            // Save the audio recorded by the microphone in WAV format to the SD card.
            if (saveWAVToSD(rec_data, record_size)) {
                printf("WAV file saved successfully.\n");
            } else {
                printf("Failed to save WAV file.\n");
            }
            M5Cardputer.Display.clear();
        }
        updateDisplay(wavFiles, selectedFileIndex);
    }
    scanAndDisplayWAVFiles();
}
void updateDisplay(std::vector<String> wavFiles, uint8_t selectedFileIndex)
{
    if (wavFiles.empty()) {
        printf("No WAV files found on SD card.\n");
        M5Cardputer.Display.fillScreen(BLACK);
        M5Cardputer.Display.setTextColor(RED);
        int xPos = (M5Cardputer.Display.width()) / 2;
        int yPos = M5Cardputer.Display.height() / 2 - 20;
        M5Cardputer.Display.drawString("No WAV files found", xPos, yPos);
        return;
    }
    const uint8_t maxVisibleFiles = 5;
    uint8_t startIndex            = 0;

    // Ensure the selected file is within the visible area.
    if (selectedFileIndex >= maxVisibleFiles) {
        startIndex = selectedFileIndex - (maxVisibleFiles - 1);
    }

    M5Cardputer.Display.fillScreen(BLACK);  // 清屏

    // Display the list of WAV files, centered. The selected file's font will be yellow, and the unselected files' font
    // will be white.
    for (size_t i = startIndex; i < startIndex + maxVisibleFiles && i < wavFiles.size(); i++) {
        uint16_t color = (i == selectedFileIndex) ? YELLOW : WHITE;
        M5Cardputer.Display.setTextColor(color);
        M5Cardputer.Display.drawString(wavFiles[i], M5Cardputer.Display.width() / 2, 3 + (i - startIndex) * 25);
    }
}
void scanAndDisplayWAVFiles()
{
    static std::vector<String> previousWavFiles;

    // Scan the WAV files on the SD card and store them in wavFiles.
    File dir = SD.open("/");
    if (!dir) {
        printf("Failed to open directory.\n");
        return;
    }
    wavFiles.clear();
    while (File entry = dir.openNextFile()) {
        if (!entry.isDirectory() && String(entry.name()).endsWith(".wav")) {
            wavFiles.push_back(String(entry.name()));  // Only add .wav files to the list.
        }
        entry.close();
    }
    dir.close();

    // If wavFiles changes, clear the interface and update the display.
    if (wavFiles != previousWavFiles) {
        previousWavFiles = wavFiles;  // Update the previous file list.
        updateDisplay(wavFiles, selectedFileIndex);
    }

    // Check if the Cardputer keyboard is pressed.
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            char data;
            for (auto i : status.word) {
                data += i;
            }
            // Use the up and down keys to select a file.
            if (data == ';') {
                selectedFileIndex = (selectedFileIndex == 0) ? wavFiles.size() - 1 : selectedFileIndex - 1;
                updateDisplay(wavFiles, selectedFileIndex);
            }
            if (data == '.') {
                selectedFileIndex = (selectedFileIndex + 1) % wavFiles.size();
                updateDisplay(wavFiles, selectedFileIndex);
            }

            // Delete the selected file.
            if (status.del) {
                String filePath = "/" + wavFiles[selectedFileIndex];
                if (SD.remove(filePath.c_str())) {
                    printf("Deleted file: %s\n", filePath.c_str());
                    wavFiles.erase(wavFiles.begin() + selectedFileIndex);
                    if (selectedFileIndex >= wavFiles.size() && !wavFiles.empty()) {
                        selectedFileIndex--;
                    }
                } else {
                    printf("Failed to delete file: %s\n", filePath.c_str());
                }
            }

            // Press the confirm key to play the selected file.
            if (status.enter) {
                playSelectedWAVFile(wavFiles[selectedFileIndex]);
            }
        }
    }
}

bool playSelectedWAVFile(const String& fileName)
{
    String filePath = "/" + fileName;
    printf("Playing WAV file: %s\n", filePath.c_str());

    File file = SD.open(filePath.c_str());
    if (!file) {
        printf("Failed to open WAV file: %s\n", filePath.c_str());
        return false;
    }

    // Skip the header of the WAV file (usually 44 bytes).
    file.seek(44);

    size_t bytesRead = file.read((uint8_t*)rec_data, record_size * sizeof(int16_t));
    file.close();

    if (bytesRead == 0) {
        printf("Failed to read WAV file data.\n");
        return false;
    }
    playWAV();
    printf("Playback finished.\n");
    return true;
}

bool playWAVFileFromSD(void)
{
    File dir       = SD.open("/");
    String wavFile = "";

    // Search for the first WAV file
    while (File entry = dir.openNextFile()) {
        if (!entry.isDirectory() && String(entry.name()).endsWith(".wav")) {
            wavFile = "/" + String(entry.name());
            entry.close();
            break;
        }
        entry.close();
    }
    dir.close();

    if (wavFile == "") {
        printf("No WAV files found on SD card.\n");
        return false;
    }

    printf("Playing WAV file: %s\n", wavFile.c_str());
    File file = SD.open(wavFile.c_str());
    if (!file) {
        printf("Failed to open WAV file: %s\n", wavFile.c_str());
        return false;
    }

    // Skip WAV header (typically 44 bytes)
    file.seek(44);

    size_t bytesRead = file.read((uint8_t*)rec_data, record_size);
    file.close();

    if (bytesRead == 0) {
        printf("Failed to read WAV file data.\n");
        return false;
    }

    playWAV();
    printf("Playback finished.\n");
    return true;
}

void playWAV(void)
{
    M5Cardputer.Display.clear();
    M5Cardputer.Mic.end();
    M5Cardputer.Speaker.begin();
    M5Cardputer.Display.fillTriangle(70 - 8, 15 - 8, 70 - 8, 15 + 8, 70 + 8, 15, 0x1c9f);
    M5Cardputer.Display.drawString("PLAY", 120, 3);
    static constexpr int shift = 6;
    for (uint16_t i = 0; i < record_number; i++) {
        auto data = &rec_data[i * record_length];
        M5Cardputer.Speaker.playRaw(&rec_data[i * record_length], record_length, record_samplerate);
        do {
            delay(1);
            M5Cardputer.update();
        } while (M5Cardputer.Speaker.isPlaying());
        if (i >= 2) {
            data      = &rec_data[(i - 2) * record_length];
            int32_t w = M5Cardputer.Display.width();
            if (w > record_length - 1) {
                w = record_length - 1;
            }
            for (int32_t x = 0; x < w; ++x) {
                M5Cardputer.Display.writeFastVLine(x, prev_y[x], prev_h[x], TFT_BLACK);
                int32_t y1 = (data[x] >> shift);
                int32_t y2 = (data[x + 1] >> shift);
                if (y1 > y2) {
                    int32_t tmp = y1;
                    y1          = y2;
                    y2          = tmp;
                }
                int32_t y = ((M5Cardputer.Display.height()) >> 1) + y1;
                int32_t h = ((M5Cardputer.Display.height()) >> 1) + y2 + 1 - y;
                prev_y[x] = y;
                prev_h[x] = h;
                M5Cardputer.Display.writeFastVLine(x, prev_y[x], prev_h[x], WHITE);
            }
        }
        M5Cardputer.Display.fillTriangle(70 - 8, 15 - 8, 70 - 8, 15 + 8, 70 + 8, 15, 0x1c9f);
        M5Cardputer.Display.drawString("PLAY", 120, 3);
    }

    M5Cardputer.Speaker.end();
    M5Cardputer.Mic.begin();
    M5Cardputer.Display.clear();
    updateDisplay(wavFiles, selectedFileIndex);
}

bool saveWAVToSD(int16_t* data, size_t dataSize)
{
    // Generate a unique file name based on a counter. The counter defaults to 1 upon device reboot.
    char filename[32];
    snprintf(filename, sizeof(filename), "/recorded%lu.wav", file_counter++);  // Include the counter in the file name.
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        printf("Failed to open file for writing.\n");
        return false;
    }

    WAVHeader header;
    header.fileSize = 36 + dataSize * sizeof(int16_t);
    header.dataSize = dataSize * sizeof(int16_t);

    // Write the WAV header.
    file.write((uint8_t*)&header, sizeof(WAVHeader));

    // Write the audio data.
    file.write((uint8_t*)data, dataSize * sizeof(int16_t));
    file.close();

    return true;
}
