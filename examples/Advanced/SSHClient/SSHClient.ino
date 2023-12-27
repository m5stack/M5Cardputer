/**
 * @file SSHClient.ino
 * @author aat440hz (https://github.com/aat440hz/SSHClient-M5Cardputer)
 * @brief M5Cardputer SSHClient
 * @version 0.1
 * @date 2023-12-27
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.9
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * LibSSH-ESP32: https://github.com/ewpa/LibSSH-ESP32
 */

#include <WiFi.h>
#include <M5Cardputer.h>
#include "libssh_esp32.h"
#include <libssh/libssh.h>

const char* ssid     = "Your_SSID";      // Replace with your WiFi SSID
const char* password = "Your_Password";  // Replace with your WiFi password

// SSH server configuration (initialize as empty strings)
String ssh_host     = "";
String ssh_user     = "";
String ssh_password = "";

// M5Cardputer setup
M5Canvas canvas(&M5Cardputer.Display);
String commandBuffer              = "> ";
int cursorY                       = 0;
const int lineHeight              = 32;
unsigned long lastKeyPressMillis  = 0;
const unsigned long debounceDelay = 200;  // Adjust debounce delay as needed

ssh_session my_ssh_session;
ssh_channel channel;

void waitForInput(String& input);

bool filterAnsiSequences =
    true;  // Set to false to disable ANSI sequence filtering

void setup() {
    Serial.begin(115200);  // Initialize serial communication for debugging
    Serial.println("Starting Setup");

    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(1);  // Set text size

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    // Initialize the cursor Y position
    cursorY = M5Cardputer.Display.getCursorY();

    // Prompt for SSH host, username, and password
    M5Cardputer.Display.print("SSH Host: ");
    waitForInput(ssh_host);
    M5Cardputer.Display.print("\nSSH Username: ");
    waitForInput(ssh_user);
    M5Cardputer.Display.print("\nSSH Password: ");
    waitForInput(ssh_password);

    // Connect and authenticate with SSH server
    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        Serial.println("SSH Session creation failed.");
        return;
    }
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, ssh_host.c_str());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, ssh_user.c_str());

    if (ssh_connect(my_ssh_session) != SSH_OK) {
        Serial.println("SSH Connect error.");
        ssh_free(my_ssh_session);
        return;
    }

    if (ssh_userauth_password(my_ssh_session, NULL, ssh_password.c_str()) !=
        SSH_AUTH_SUCCESS) {
        Serial.println("SSH Authentication error.");
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    channel = ssh_channel_new(my_ssh_session);
    if (channel == NULL || ssh_channel_open_session(channel) != SSH_OK) {
        Serial.println("SSH Channel open error.");
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    if (ssh_channel_request_pty(channel) != SSH_OK) {
        Serial.println("SSH PTY request error.");
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    if (ssh_channel_request_shell(channel) != SSH_OK) {
        Serial.println("SSH Shell request error.");
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    Serial.println("SSH setup completed.");
}

void loop() {
    M5Cardputer.update();

    // Handle keyboard input with debounce
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastKeyPressMillis >= debounceDelay) {
            lastKeyPressMillis               = currentMillis;
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto i : status.word) {
                commandBuffer += i;
                M5Cardputer.Display.print(i);
                cursorY = M5Cardputer.Display.getCursorY();
            }

            if (status.del && commandBuffer.length() > 2) {
                commandBuffer.remove(commandBuffer.length() - 1);
                M5Cardputer.Display.setCursor(
                    M5Cardputer.Display.getCursorX() - 6,
                    M5Cardputer.Display.getCursorY());
                M5Cardputer.Display.print(" ");
                M5Cardputer.Display.setCursor(
                    M5Cardputer.Display.getCursorX() - 6,
                    M5Cardputer.Display.getCursorY());
                cursorY = M5Cardputer.Display.getCursorY();
            }

            if (status.enter) {
                commandBuffer.trim();  // Trim the command buffer to remove
                                       // accidental whitespaces/newlines
                String message = commandBuffer.substring(
                    2);  // Get the command part, exclude the "> "
                ssh_channel_write(channel, message.c_str(),
                                  message.length());  // Send the command
                ssh_channel_write(channel, "\r",
                                  1);  // Send exactly one carriage return (try
                                       // "\n" or "\r\n" if needed)

                commandBuffer = "> ";  // Reset command buffer
                M5Cardputer.Display.print(
                    '\n');  // Move to the next line on display
                cursorY =
                    M5Cardputer.Display.getCursorY();  // Update cursor position
            }
        }
    }

    // Check if the cursor has reached the bottom of the display
    if (cursorY > M5Cardputer.Display.height() - lineHeight) {
        M5Cardputer.Display.scroll(0, -lineHeight);
        cursorY -= lineHeight;
        M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX(),
                                      cursorY);
    }

    // Read data from SSH server and display it, handling ANSI sequences
    char buffer[128];  // Reduced buffer size for less memory usage
    int nbytes =
        ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
    bool isAnsiSequence =
        false;  // To track when we are inside an ANSI sequence

    if (nbytes > 0) {
        for (int i = 0; i < nbytes; ++i) {
            char c = buffer[i];
            if (filterAnsiSequences) {
                if (c == '\033') {
                    isAnsiSequence = true;  // Enter ANSI sequence mode
                } else if (isAnsiSequence) {
                    if (isalpha(c)) {
                        isAnsiSequence = false;  // Exit ANSI sequence mode at
                                                 // the end character
                    }
                } else {
                    if (c == '\r') continue;  // Ignore carriage return
                    M5Cardputer.Display.write(c);
                    cursorY = M5Cardputer.Display.getCursorY();
                }
            } else {
                if (c == '\r') continue;  // Ignore carriage return
                M5Cardputer.Display.write(c);
                cursorY = M5Cardputer.Display.getCursorY();
            }
        }
    }

    // Handle channel closure and other conditions
    if (nbytes < 0 || ssh_channel_is_closed(channel)) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        M5Cardputer.Display.println("\nSSH session closed.");
        return;  // Exit the loop upon session closure
    }
}

void waitForInput(String& input) {
    unsigned long startTime           = millis();
    unsigned long lastKeyPressMillis  = 0;
    const unsigned long debounceDelay = 200;  // Adjust debounce delay as needed
    String currentInput               = input;

    while (true) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            if (status.del && currentInput.length() > 0) {
                // Handle backspace key
                currentInput.remove(currentInput.length() - 1);
                M5Cardputer.Display.setCursor(
                    M5Cardputer.Display.getCursorX() - 6,
                    M5Cardputer.Display.getCursorY());
                M5Cardputer.Display.print(" ");
                M5Cardputer.Display.setCursor(
                    M5Cardputer.Display.getCursorX() - 6,
                    M5Cardputer.Display.getCursorY());
                cursorY            = M5Cardputer.Display.getCursorY();
                lastKeyPressMillis = millis();
            }

            for (auto i : status.word) {
                if (millis() - lastKeyPressMillis >= debounceDelay) {
                    currentInput += i;
                    M5Cardputer.Display.print(i);
                    cursorY            = M5Cardputer.Display.getCursorY();
                    lastKeyPressMillis = millis();
                }
            }

            if (status.enter) {
                M5Cardputer.Display.println();  // Move to the next line
                input = currentInput;
                break;
            }
        }

        if (millis() - startTime > 180000) {  // Timeout after 3 minutes
            M5Cardputer.Display.println("\nInput timeout. Rebooting...");
            delay(1000);    // Delay for 1 second to allow the message to be
                            // displayed
            ESP.restart();  // Reboot the ESP32
        }
    }
}
