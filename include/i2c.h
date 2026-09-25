#ifndef I2C_H
#define I2C_H

int i2c_init(int bus, int addr);
int i2c_write_color(int r, int g, int b);
int i2c_write_bright(int bright);
void i2c_close(void);

#endif /* I2C_H */