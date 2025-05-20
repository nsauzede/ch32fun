#include <stdint.h>
#define delay_us(us) Delay_Us(us)
void DQ_output(void) {
    GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
}
void DQ_input(void) {
    GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_I_floating, GPIO_Speed_In);
}
void DQ_low(void) {
    GPIO_digitalWrite_lo(GPIOv_from_PORT_PIN(GPIO_port_C, 4));
}
void DQ_high(void) {
    GPIO_digitalWrite_hi(GPIOv_from_PORT_PIN(GPIO_port_C, 4));
}
uint8_t DQ_read(void) {
    return GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, 4));
}
uint8_t ds18b20_reset(void) {
    DQ_output();
    DQ_low();
    delay_us(480);
    DQ_input();
    delay_us(70);
    uint8_t presence = !DQ_read();
    delay_us(410);
    return presence;
}
void ds18b20_write_bit(uint8_t bit) {
    DQ_output();
    DQ_low();
    if (bit)
        delay_us(6);
    else
        delay_us(60);
    DQ_input();
    if (bit)
        delay_us(64);
    else
        delay_us(10);
}
uint8_t ds18b20_read_bit(void) {
    uint8_t bit;
    DQ_output();
    DQ_low();
    delay_us(6);
    DQ_input();
    delay_us(9);
    bit = DQ_read();
    delay_us(55);
    return bit;
}
void ds18b20_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        ds18b20_write_bit(data & 0x01);
        data >>= 1;
    }
}
uint8_t ds18b20_read_byte(void) {
    uint8_t value = 0;
    for (int i = 0; i < 8; i++) {
        value >>= 1;
        if (ds18b20_read_bit())
            value |= 0x80;
    }
    return value;
}
void ds18b20_read_rom() {    // 28 f7
    if (!ds18b20_reset())
        return; // sensor not found
    uint8_t rom_code[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    ds18b20_write_byte(0x33); // READ ROM
    for (int i = 0; i < 8; i++) {
        rom_code[i] = ds18b20_read_byte();
        printf(" %02x", rom_code[i]);
    }
    printf("\n");
}
// Read temperature (in °C * 16)
int16_t ds18b20_read_temp(void) {
    if (!ds18b20_reset())
        return -1000; // sensor not found
    ds18b20_write_byte(0xCC); // Skip ROM
    ds18b20_write_byte(0x44); // Convert T
    // Wait for conversion
    // Instead of releasing line (DQ_input), actively drive HIGH
DQ_output();
DQ_high();  // Keep driving HIGH
delay_us(800000);  // Let conversion happen
DQ_input();     // Now release for scratchpad read
    if (!ds18b20_reset())
        return -2000; // sensor not found
    ds18b20_write_byte(0xCC); // Skip ROM
    ds18b20_write_byte(0xBE); // Read Scratchpad
    uint8_t temp_lsb = ds18b20_read_byte();
    uint8_t temp_msb = ds18b20_read_byte();
    printf(" %02x %02x", temp_lsb, temp_msb);
    int16_t temp_raw = (temp_msb << 8) | temp_lsb;
    return temp_raw; // Use temp_raw / 16.0 to get °C
}
