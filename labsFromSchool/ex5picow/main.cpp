#include <stdio.h>
#include <string.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "uart/PicoUart.h"

#include "IPStack.h"
#include "Countdown.h"
#include "MQTTClient.h"
#include "ModbusClient.h"
#include "ModbusRegister.h"
#include "ssd1306.h"
#include "epd154.h"
#include "FreeMono12pt7b.h"
#include "st7789nobuf.h"

#include "PicoI2CDevice.h"
#include "PicoSPIBus.h"
#include "PicoSPIDevice.h"
#include "rgb_palette.h"

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#if 0
#define UART_NR 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#else
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#endif

#define BAUD_RATE 9600
#define STOP_BITS 1 // for simulator
//#define STOP_BITS 2 // for real system

//#define USE_MODBUS
#define USE_MQTT
//#define USE_SSD1306
//#define USE_EPD154
//#define USE_ST7789
//buttons and leds
const uint led_pin1 = 22;
const uint led_pin2 = 21;
const uint led_pin3 = 20;
const uint button1 = 12; //updated to pin 12 -hannu
const uint button2 = 8;
const uint button3 = 7;

float low_alarm = 20.0f;
float high_alarm = 30.0f;

#if defined(USE_SSD1306) || defined(USE_EPD154) || defined(USE_ST7789)
static const uint8_t raspberry26x32[] =
        {0x0, 0x0, 0xe, 0x7e, 0xfe, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xf8, 0xfc, 0xfe,
         0xfe, 0xff, 0xff,0xff, 0xff, 0xff, 0xfe, 0x7e,
         0x1e, 0x0, 0x0, 0x0, 0x80, 0xe0, 0xf8, 0xfd,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff,0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd,
         0xf8, 0xe0, 0x80, 0x0, 0x0, 0x1e, 0x7f, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0x7f, 0x1e, 0x0, 0x0,
         0x0, 0x3, 0x7, 0xf, 0x1f, 0x1f, 0x3f, 0x3f,
         0x7f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x3f,
         0x3f, 0x1f, 0x1f, 0xf, 0x7, 0x3, 0x0, 0x0 };
#endif
//Read temp function from rp2040 datasheet -hannu
float read_core_temp() {
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / (1 << 12);  // 12-bit ADC
    float temp_c = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temp_c;
}
void messageArrived(MQTT::MessageData &md) {
    MQTT::Message &message = md.message;
    std::string payload((char*)message.payload, message.payloadlen);
    /* Legacy code
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n",
           message.qos, message.retained, message.dup, message.id);
    printf("Payload %s\n", (char *) message.payload);
    */

    // Try JSON format first -hannu
    std::string msg;

    size_t msgPos = payload.find("\"msg\"");
    if (msgPos != std::string::npos) {
        // Find start of inner object
        size_t colonPos = payload.find(":", msgPos);
        if (colonPos != std::string::npos) {
            //first quote after colon
            size_t q1 = payload.find('"', colonPos); //find first pair of quotes
            size_t q2 = payload.find('"', q1 + 1);

            if (q1 != std::string::npos && q2 != std::string::npos) {
                msg = payload.substr(q1 + 1, q2 - q1 - 1);
            }
        }
    }

        if (msg.empty()) msg = payload;
    //trim whitespaces -hp
    msg.erase(0, msg.find_first_not_of(" \t\n\r"));
    msg.erase(msg.find_last_not_of(" \t\n\r") + 1);
        //handle messages -hp
        if (msg == "temp") {
            float temp = read_core_temp();
            printf("MQTT request temp -> %.2f C\n", temp);
            return;
        }

    if (msg.find("High;") == 0) { //start with high
            high_alarm = std::stof(msg.substr(5));
            printf("High alarm set: %.2f C\n", high_alarm);
        return;
        }
    if (msg.find("Low;") == 0) {
        low_alarm = std::stof(msg.substr(4));
        printf("Low alarm set: %.2f C\n", low_alarm);
        return;
    }
    printf("Unknown message : %s\n", msg.c_str());
}


        int main() {
    // Initialize LED pin
    //gpio_init(led_pin);
    gpio_init(led_pin1);
    gpio_init(led_pin2);
    gpio_init(led_pin3);
    gpio_set_dir(led_pin1, GPIO_OUT);
    gpio_set_dir(led_pin2, GPIO_OUT);
    gpio_set_dir(led_pin3, GPIO_OUT);

    gpio_init(button1); gpio_set_dir(button1, GPIO_IN); gpio_pull_up(button1);
    gpio_init(button2); gpio_set_dir(button2, GPIO_IN); gpio_pull_up(button2);
    gpio_init(button3); gpio_set_dir(button3, GPIO_IN); gpio_pull_up(button3);

    // Initialize chosen serial port
    stdio_init_all();

    //ADC setup -hannu
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); //internal temperature sensor = 4

    printf("\nBoot\n");
#ifdef USE_SSD1306
    auto bus = std::make_shared<PicoI2CBus>(1, 14, 15);
    auto dev = std::make_shared<PicoI2CDevice>(bus, 0x3C);
    ssd1306 display(dev);
    display.fill(0);
    //display.text("Hello", 0, 0);
    mono_vlsb rb(raspberry26x32, 26, 32);
    //display.blit(rb, 20, 20);
    //display.rect(15, 15, 35, 45, 1);
    //display.line(60, 5, 120, 60, 1);
    //display.line(60, 60, 120, 5, 1);
    display.show();
    /*
#if 1
            for(int i = 0; i < 128; ++i) {
                sleep_ms(50);
                display.scroll(1, 0);
                display.show();
            }*/
    //display.text("Done", 20, 30);
    //display.show();
#endif
    //display.setfont(&FreeMono12pt7b);
    //display.text("Free Mono", 0, 20,1);
    //display.show();
    //#endif

#ifdef USE_EPD154
    static const unsigned char binary_data[] = {
        // font edit begin : monohlsb : 48 : 60 : 48
        0x00, 0x3F, 0x00, 0x00, 0xFC, 0x00, 0x07, 0xFF,
        0xE0, 0x07, 0xFF, 0xE0, 0x1F, 0x85, 0x78, 0x1F,
        0x85, 0xF8, 0x3E, 0x00, 0x1C, 0x38, 0x00, 0x3C,
        0x10, 0x00, 0x06, 0x60, 0x00, 0x08, 0x38, 0x00,
        0x02, 0x60, 0x00, 0x1C, 0x30, 0x20, 0x03, 0xC0,
        0x04, 0x0C, 0x18, 0x0C, 0x01, 0xC0, 0x30, 0x18,
        0x18, 0x03, 0x01, 0x80, 0x40, 0x18, 0x1C, 0x00,
        0x81, 0x81, 0x80, 0x38, 0x0C, 0x00, 0x63, 0xC6,
        0x00, 0x10, 0x0E, 0x00, 0x37, 0xEC, 0x00, 0x70,
        0x06, 0x00, 0x1F, 0xF8, 0x00, 0x60, 0x03, 0x80,
        0x0F, 0xF8, 0x00, 0xC0, 0x03, 0x80, 0x1F, 0xF8,
        0x01, 0xC0, 0x00, 0xC0, 0x3F, 0xFC, 0x03, 0x00,
        0x00, 0xF9, 0xFF, 0xFF, 0x8F, 0x00, 0x00, 0x7F,
        0xF8, 0x1F, 0xFE, 0x00, 0x00, 0xF0, 0x60, 0x06,
        0x1F, 0x00, 0x01, 0xC0, 0xC0, 0x03, 0x03, 0x80,
        0x03, 0x80, 0xC0, 0x03, 0x81, 0xC0, 0x03, 0x01,
        0xC0, 0x03, 0xC0, 0xC0, 0x07, 0x07, 0xE0, 0x03,
        0xE0, 0x60, 0x06, 0x0F, 0xF8, 0x0F, 0xF0, 0x60,
        0x06, 0x1F, 0xFF, 0xF8, 0x78, 0x60, 0x06, 0x7C,
        0x0F, 0xF0, 0x1E, 0x60, 0x07, 0xF0, 0x07, 0xE0,
        0x0F, 0x70, 0x0F, 0xE0, 0x03, 0xC0, 0x07, 0xF0,
        0x1F, 0xE0, 0x01, 0xC0, 0x03, 0xB8, 0x39, 0xC0,
        0x01, 0xC0, 0x03, 0x8C, 0x71, 0xC0, 0x01, 0x80,
        0x01, 0x8E, 0x60, 0xC0, 0x01, 0xC0, 0x01, 0x06,
        0xE0, 0xC0, 0x01, 0xC0, 0x01, 0x07, 0xC1, 0xC0,
        0x03, 0xC0, 0x01, 0x83, 0xC1, 0xC0, 0x03, 0xE0,
        0x03, 0x83, 0xC1, 0xC0, 0x07, 0xF0, 0x03, 0x83,
        0xC1, 0xE0, 0x0F, 0xF8, 0x07, 0x83, 0xE1, 0xF0,
        0x1C, 0x1E, 0x0F, 0x87, 0x63, 0xFF, 0xF0, 0x0F,
        0xFF, 0xC6, 0x73, 0xFF, 0xE0, 0x03, 0xFE, 0xCE,
        0x3F, 0xFF, 0xC0, 0x03, 0xF8, 0x7C, 0x1C, 0x1F,
        0x80, 0x01, 0xF0, 0x38, 0x1C, 0x07, 0x80, 0x01,
        0xE0, 0x38, 0x1C, 0x03, 0x80, 0x01, 0xC0, 0x38,
        0x0C, 0x03, 0x80, 0x01, 0x80, 0x30, 0x0C, 0x01,
        0xC0, 0x01, 0x80, 0x30, 0x0C, 0x01, 0xC0, 0x03,
        0x00, 0x30, 0x0E, 0x00, 0xE0, 0x07, 0x00, 0x70,
        0x06, 0x00, 0xF0, 0x0F, 0x00, 0x60, 0x07, 0x00,
        0xFC, 0x3F, 0x00, 0xE0, 0x03, 0x80, 0xFF, 0xFF,
        0x01, 0xC0, 0x01, 0xE1, 0xFF, 0xFF, 0x07, 0x80,
        0x00, 0x7F, 0xF0, 0x07, 0xFE, 0x00, 0x00, 0x3F,
        0xC0, 0x01, 0xFC, 0x00, 0x00, 0x0F, 0x80, 0x01,
        0xF0, 0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x00,
        0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x00, 0x00,
        0x78, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8,
        0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00
        // font edit end
    };
#if 0
    static uint8_t inv_raspberry26x32[sizeof(raspberry26x32)];
    for(size_t i = 0; i < sizeof(raspberry26x32); ++i) {
        inv_raspberry26x32[i] = ~raspberry26x32[i];
    }
#endif
    epd154 display;
    display.fill(1);
    display.show();
    display.text("Hello", 0, 0);
    mono_vlsb rb(raspberry26x32, 26, 32);
    mono_horiz rpi(binary_data,48,60);
    display.blit(rb, 20, 20);
    display.blit(rpi, 120, 120);
    display.rect(15, 15, 35, 45, 0);
    display.line(60, 5, 120, 60, 0);
    display.line(60, 60, 120, 5, 0);
    display.show();
#if 0
    for(int i = 0; i < 128; ++i) {
        sleep_ms(50);
        display.scroll(1, 0);
        //display.show();
    }
    display.text("Done", 20, 20);
    display.show();
#endif

#endif
#ifdef USE_ST7789
    static const unsigned char binary_data[] = {
        // font edit begin : monohlsb : 48 : 60 : 48
        0x00, 0x3F, 0x00, 0x00, 0xFC, 0x00, 0x07, 0xFF,
        0xE0, 0x07, 0xFF, 0xE0, 0x1F, 0x85, 0x78, 0x1F,
        0x85, 0xF8, 0x3E, 0x00, 0x1C, 0x38, 0x00, 0x3C,
        0x10, 0x00, 0x06, 0x60, 0x00, 0x08, 0x38, 0x00,
        0x02, 0x60, 0x00, 0x1C, 0x30, 0x20, 0x03, 0xC0,
        0x04, 0x0C, 0x18, 0x0C, 0x01, 0xC0, 0x30, 0x18,
        0x18, 0x03, 0x01, 0x80, 0x40, 0x18, 0x1C, 0x00,
        0x81, 0x81, 0x80, 0x38, 0x0C, 0x00, 0x63, 0xC6,
        0x00, 0x10, 0x0E, 0x00, 0x37, 0xEC, 0x00, 0x70,
        0x06, 0x00, 0x1F, 0xF8, 0x00, 0x60, 0x03, 0x80,
        0x0F, 0xF8, 0x00, 0xC0, 0x03, 0x80, 0x1F, 0xF8,
        0x01, 0xC0, 0x00, 0xC0, 0x3F, 0xFC, 0x03, 0x00,
        0x00, 0xF9, 0xFF, 0xFF, 0x8F, 0x00, 0x00, 0x7F,
        0xF8, 0x1F, 0xFE, 0x00, 0x00, 0xF0, 0x60, 0x06,
        0x1F, 0x00, 0x01, 0xC0, 0xC0, 0x03, 0x03, 0x80,
        0x03, 0x80, 0xC0, 0x03, 0x81, 0xC0, 0x03, 0x01,
        0xC0, 0x03, 0xC0, 0xC0, 0x07, 0x07, 0xE0, 0x03,
        0xE0, 0x60, 0x06, 0x0F, 0xF8, 0x0F, 0xF0, 0x60,
        0x06, 0x1F, 0xFF, 0xF8, 0x78, 0x60, 0x06, 0x7C,
        0x0F, 0xF0, 0x1E, 0x60, 0x07, 0xF0, 0x07, 0xE0,
        0x0F, 0x70, 0x0F, 0xE0, 0x03, 0xC0, 0x07, 0xF0,
        0x1F, 0xE0, 0x01, 0xC0, 0x03, 0xB8, 0x39, 0xC0,
        0x01, 0xC0, 0x03, 0x8C, 0x71, 0xC0, 0x01, 0x80,
        0x01, 0x8E, 0x60, 0xC0, 0x01, 0xC0, 0x01, 0x06,
        0xE0, 0xC0, 0x01, 0xC0, 0x01, 0x07, 0xC1, 0xC0,
        0x03, 0xC0, 0x01, 0x83, 0xC1, 0xC0, 0x03, 0xE0,
        0x03, 0x83, 0xC1, 0xC0, 0x07, 0xF0, 0x03, 0x83,
        0xC1, 0xE0, 0x0F, 0xF8, 0x07, 0x83, 0xE1, 0xF0,
        0x1C, 0x1E, 0x0F, 0x87, 0x63, 0xFF, 0xF0, 0x0F,
        0xFF, 0xC6, 0x73, 0xFF, 0xE0, 0x03, 0xFE, 0xCE,
        0x3F, 0xFF, 0xC0, 0x03, 0xF8, 0x7C, 0x1C, 0x1F,
        0x80, 0x01, 0xF0, 0x38, 0x1C, 0x07, 0x80, 0x01,
        0xE0, 0x38, 0x1C, 0x03, 0x80, 0x01, 0xC0, 0x38,
        0x0C, 0x03, 0x80, 0x01, 0x80, 0x30, 0x0C, 0x01,
        0xC0, 0x01, 0x80, 0x30, 0x0C, 0x01, 0xC0, 0x03,
        0x00, 0x30, 0x0E, 0x00, 0xE0, 0x07, 0x00, 0x70,
        0x06, 0x00, 0xF0, 0x0F, 0x00, 0x60, 0x07, 0x00,
        0xFC, 0x3F, 0x00, 0xE0, 0x03, 0x80, 0xFF, 0xFF,
        0x01, 0xC0, 0x01, 0xE1, 0xFF, 0xFF, 0x07, 0x80,
        0x00, 0x7F, 0xF0, 0x07, 0xFE, 0x00, 0x00, 0x3F,
        0xC0, 0x01, 0xFC, 0x00, 0x00, 0x0F, 0x80, 0x01,
        0xF0, 0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x00,
        0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x00, 0x00,
        0x78, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8,
        0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00
        // font edit end
    };
    auto spi = std::make_shared<PicoSPIBus>(0, 18, 19);
    auto dev = std::make_shared<PicoSPIDevice>(spi, 17);
    st7789nobuf display(dev, 27); // old 16
    display.fill(0xFFFF);
    display.show();
    display.text("Hello", 0, 0, 0xFFFF);
    mono_vlsb rb(raspberry26x32, 26, 32);
    mono_horiz rpi(binary_data,48,60);
    display.blit(rb, 20, 20, 0, rgb_palette(2, 0xFC00));
    display.blit(rpi, 120, 120, 0, rgb_palette(2, 0xF81F));
    display.rect(15, 15, 35, 45, 0x07E0);
    display.line(60, 5, 120, 60, 0xF800);
    display.line(60, 60, 120, 5, 0xF800);
    display.setfont(&FreeMono12pt7b);
    display.text("Free Mono", 10, 100,0xF800);
    display.show();

#endif


#ifdef USE_MQTT
    const char *topic = "hannu";
    //IPStack ipstack("SSID", "PASSWORD"); // example
    IPStack ipstack("C0rtnet", "Kev4trulla*"); // example
    //IPStack ipstack("SmartIotMQTT", "SmartIot"); // example
    auto client = MQTT::Client<IPStack, Countdown>(ipstack);

    int rc = ipstack.connect("192.168.8.100", 1883); // change this when at the class -hannu
    //int rc = ipstack.connect("192.168.50.122", 1883); // change this when at the class -hannu
    if (rc != 1) {
        printf("rc from TCP connect is %d\n", rc);
    }

    printf("MQTT connecting\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char *) "hannu";
    //data.username.cstring = (char *)"hannu"; //change user if needed -hannu
    //data.password.cstring = (char *)"emmi"; //change pass if needed -hannu
    rc = client.connect(data);
    if (rc != 0) {
        printf("rc from MQTT connect is %d\n", rc);
        while (true) {
            tight_loop_contents();
        }
    }
    printf("MQTT connected\n");

    const char *led_topic = "hp"; //used for subscribing
    // We subscribe QoS2. Messages sent with lower QoS will be delivered using the QoS they were sent with
    rc = client.subscribe(led_topic, MQTT::QOS2, messageArrived); //changed from topic to led_topic -hannu
    if (rc != 0) {
        printf("rc from MQTT subscribe is %d\n", rc);
    }
    printf("MQTT subscribed\n");

    int mqtt_qos = 0;
    int msg_count = 0;

#endif

#ifdef USE_MODBUS
    auto uart{std::make_shared<PicoUart>(UART_NR, UART_TX_PIN, UART_RX_PIN, BAUD_RATE, STOP_BITS)};
    auto rtu_client{std::make_shared<ModbusClient>(uart)};
    ModbusRegister rh(rtu_client, 241, 256);
    auto modbus_poll = make_timeout_time_ms(3000);
    ModbusRegister produal(rtu_client, 1, 0);
    produal.write(100);
    sleep_ms((100));
    produal.write(100);
#endif
#ifdef USE_MQTT
    static absolute_time_t next_temp_time = get_absolute_time();
    static absolute_time_t mqtt_send = get_absolute_time();
    static absolute_time_t last_button_press = {0};
    bool button_prev = false;
    const uint BUTTON_DEBOUNCE_MS = 50;
    while (true) {
        absolute_time_t now = get_absolute_time();
#ifdef USE_MODBUS
        if (time_reached(modbus_poll)) {
            gpio_put(led_pin, !gpio_get(led_pin)); // toggle  led
            modbus_poll = delayed_by_ms(modbus_poll, 3000);
            printf("RH=%5.1f%%\n", rh.read() / 10.0);
        }
#endif
        bool button_now = (gpio_get(button1) == 0);
        if (button_now && !button_prev &&
            absolute_time_diff_us(last_button_press, now) > BUTTON_DEBOUNCE_MS * 1000) {

            last_button_press = now;

            if (!client.isConnected()) {
                printf("Button: client not connected -> try reconnect\n");
                int rc = client.connect(data);
                printf("Reconnect rc=%d\n", rc);

            }
            //publish only if connection is fine. -hp
            if (client.isConnected()) {
                float temp = read_core_temp();
                char buf[256];
                int len = snprintf(buf, sizeof(buf), "{\"topic\":\"%s\",\"msg\":\"temp:%.2f\"}", topic, temp);

                MQTT::Message message;
                message.qos = MQTT::QOS0;
                message.retained = false;
                message.dup = false;
                message.payload = (void*)buf;
                message.payloadlen = strlen(buf);

                int rc = client.publish(topic, message);
                printf("Sent JSON MSG: %s\n", buf);
            } else {
                printf("Skipping publish: still not connected..\n");
            }
            }
        button_prev = button_now;

        //temp read 1hz
        if (time_reached(next_temp_time)) {
            next_temp_time = delayed_by_ms(next_temp_time, 1000);
            float temp = read_core_temp();
            printf("Core temp: %.2f C\n", temp);
            gpio_put(led_pin1, temp < low_alarm ? 1 : 0);
            gpio_put(led_pin3, temp > high_alarm ? 1 : 0);
            gpio_put(led_pin2, !gpio_get(led_pin2)); // 1hz heartbeat


            //oled here if still inspired -hp
#ifdef USE_SSD1306
            display.fill(0); // clear previous display
            char buf[16];
            sprintf(buf, "Temp: %.fC", temp);
            display.text(buf, 0, 15, 1);
            display.show();
#endif
            //
            // mqtt keepalive
            if (time_reached(mqtt_send)) {
                mqtt_send = delayed_by_ms(mqtt_send, 2000);
                if (!client.isConnected()) {
                    printf("Periodic reconnect attempt\n");
                    int rc = client.connect(data);
                    printf("Periodic reconnect rc=%d\n", rc);
                }
            }

                    /* legacy
                    char buf[100];
                    int rc = 0;
                    MQTT::Message message;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void *) buf;
                    switch (mqtt_qos) {
                        case 0:
                            // Send and receive QoS 0 message
                                sprintf(buf, "Msg nr: %d QoS 0 message", ++msg_count);
                        printf("%s\n", buf);
                        message.qos = MQTT::QOS0;
                        message.payloadlen = strlen(buf); //took out +1 -hannu
                        rc = client.publish(topic, message);
                        printf("Publish rc=%d\n", rc);
                        ++mqtt_qos;
                        break;
                        case 1:
                            // Send and receive QoS 1 message
                                sprintf(buf, "Msg nr: %d QoS 1 message", ++msg_count);
                        printf("%s\n", buf);
                        message.qos = MQTT::QOS1;
                        message.payloadlen = strlen(buf) + 1;
                        rc = client.publish(topic, message);
                        printf("Publish rc=%d\n", rc);
                        ++mqtt_qos;
                        break;

    #if MQTTCLIENT_QOS2
                        case 2:
                            // Send and receive QoS 2 message
                                sprintf(buf, "Msg nr: %d QoS 2 message", ++msg_count);
                        printf("%s\n", buf);
                        message.qos = MQTT::QOS2;
                        message.payloadlen = strlen(buf) + 1;
                        rc = client.publish(topic, message);
                        printf("Publish rc=%d\n", rc);
                        ++mqtt_qos;
                        break;
    #endif
                        default:
                            mqtt_qos = 0;
                        break;
                    }
                }
    */
                }
                cyw43_arch_poll(); // obsolete? - see below
                client.yield(100); // socket that client uses calls cyw43_arch_poll()
#endif
                tight_loop_contents();
            }
        }

