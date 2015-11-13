#include "ade7759.h"

u8  ade7759_tx[ADE7759_MAX_TX];
u8  ade7759_rx[ADE7759_MAX_RX];

static int ade7759_spi_write_reg_8(u8 reg_address,
                                   u8 val)
{
  int ret;
  ade7759_tx[0] = ADE7759_WRITE_REG(reg_address);
  ade7759_tx[1] = val;
  
  ret = spi_write(ade7759_tx, 2);  
  return ret;
}

static int ade7759_spi_write_reg_16(u8 reg_address,
                                    u16 value)
{
  int ret;
  ade7759_tx[0] = ADE7759_WRITE_REG(reg_address);
  ade7759_tx[1] = (value >> 8) & 0xFF;
  ade7759_tx[2] = value & 0xFF;
  ret = spi_write(ade7759_tx, 3);
  return ret;
}

static int ade7759_spi_read_reg_8(u8 reg_address,
                                  u8 *val)
{
  int ret;
  
  ret = spi_w8r8(ADE7759_READ_REG(reg_address));
  if (ret < 0) {
//    printf("problem when reading 8 bit register 0x%02X",
//            reg_address);
    return ret;
  }
  *val = ret;
  return 0;
}

static int ade7759_spi_read_reg_16(u8 reg_address,
                                   u16 *val)
{
  int ret;
  
  ret = spi_w8r16(ADE7759_READ_REG(reg_address));
  if (ret < 0) {
//    printf("problem when reading 16 bit register 0x%02X",
//            reg_address);
    return ret;
  }
  
  *val = ret;
//  *val = be16_to_cpup(val); // swap bytes if nessecsary
  
  return 0;
}

static int ade7759_spi_read_reg_40(u8 reg_address,
                                   u64 *val)
{
  struct spi_message msg;
  struct iio_dev *indio_dev = dev_get_drvdata(dev);
  struct ade7759_state *st = iio_priv(indio_dev);
  int ret;
  struct spi_transfer xfers[] = {
    {
      .tx_buf = st->tx,
      .rx_buf = st->rx,
      .bits_per_word = 8,
      .len = 6,
    },
  };
  
  mutex_lock(&st->buf_lock);
  st->tx[0] = ADE7759_READ_REG(reg_address);
  memset(&st->tx[1], 0 , 5);
  
  spi_message_init(&msg);
  spi_message_add_tail(xfers, &msg);
  ret = spi_sync(st->us, &msg);
  if (ret) {
    dev_err(&st->us->dev, "problem when reading 40 bit register 0x%02X",
            reg_address);
    goto error_ret;
  }
  *val = ((u64)st->rx[1] << 32) | (st->rx[2] << 24) |
    (st->rx[3] << 16) | (st->rx[4] << 8) | st->rx[5];
  
error_ret:
  mutex_unlock(&st->buf_lock);
  return ret;
}

int ade7759_reset(void)
{
  int ret;
  u16 val;
  ade7759_spi_read_reg_16(ADE7759_MODE,
                          &val);
  val |= 1 << 6; /* Software Chip Reset */
  ret = ade7759_spi_write_reg_16(ADE7759_MODE,
                                 val);
  
  return ret;
}

int ade7759_write_reset(const char *buf, size_t len)
{
  if (len < 1)
    return -1;
  switch (buf[0]) {
  case '1':
  case 'y':
  case 'Y':
    return ade7759_reset();
  }
  return -1;
}


int ade7759_set_irq(u8 enable)
{
  int ret;
  u8 irqen;
  ret = ade7759_spi_read_reg_8(ADE7759_IRQEN, &irqen);
  if (ret)
    goto error_ret;
  
  if (enable)
    irqen |= 1 << 3; /* Enables an interrupt when a data is
  present in the waveform register */
  else
    irqen &= ~(1 << 3);
  
  ret = ade7759_spi_write_reg_8(ADE7759_IRQEN, irqen);
  
error_ret:
  return ret;
}

/* Power down the device */
int ade7759_stop_device(void)
{
  u16 val;
  
  ade7759_spi_read_reg_16(ADE7759_MODE,
                          &val);
  val |= 1 << 4;  /* AD converters can be turned off */
  
  return ade7759_spi_write_reg_16(ADE7759_MODE, val);
}

int ade7759_initial_setup(void)
{
  int ret;  
  /* use low spi speed for init */
  st->us->mode = SPI_MODE_3;
  spi_setup(st->us);
  
  /* Disable IRQ */
  ret = ade7759_set_irq(dev, false);
  if (ret) {
//    printf("disable irq failed");
    goto err_ret;
  }
  
  ade7759_reset();
  delay_ms(ADE7759_STARTUP_DELAY);
  
err_ret:
  return ret;
}

static ssize_t ade7759_read_frequency(char *buf)
{
  int ret;
  u16 t;
  int sps;
  ret = ade7759_spi_read_reg_16(ADE7759_MODE,
                                &t);
  if (ret)
    return ret;
  
  t = (t >> 3) & 0x3;
  sps = 27900 / (1 + t);
  
  return sprintf(buf, "%d\n", sps);
}

int ade7759_write_frequency(const char *buf,
                                       size_t len)
{
  unsigned long val;
  int ret;
  u16 reg, t;
  
  ret = strict_strtol(buf, 10, &val); // convert to number
  if (ret)
    return ret;
    
  t = (27900 / val);
  if (t > 0)
    t--;
  
  if (t > 1)
    st->us->max_speed_hz = ADE7759_SPI_SLOW;
  else
    st->us->max_speed_hz = ADE7759_SPI_FAST;
  
  ret = ade7759_spi_read_reg_16(ADE7759_MODE, &reg);
  if (ret)
    goto out;
  
  reg &= ~(3 << 13);
  reg |= t << 13;
  
  ret = ade7759_spi_write_reg_16(ADE7759_MODE, reg);
  
out:
  
  return ret ? ret : len;
}
