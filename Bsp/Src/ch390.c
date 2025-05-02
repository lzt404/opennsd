#include "ch390.h"

/**
 * @name ch390_get_link_status
 * @brief Get link status of the internal PHY
 * @return 0: Link failed  1: Link OK
 */
int ch390_get_link_status(CH390_DEVICE_T dev)
{
    uint8_t nsr = ch390_read_reg(dev,CH390_NSR);
    return !!(nsr & NSR_LINKST);
}

/**
 * @name ch390_get_int_status
 * @brief Get CH390 interrupt status and clear them
 * @return Interrupt status
 */
uint8_t ch390_get_int_status(CH390_DEVICE_T dev)
{
    uint8_t int_status = ch390_read_reg(dev, CH390_ISR);
    // Clear interrupt status by write 1
    ch390_write_reg(dev, CH390_ISR, int_status);
    return int_status;
}

/**
 * @name ch390_write_reg
 * @brief Write register (HAL库版本)
 * @param reg - Target register address
 * @param value - Value to be written
 */
void ch390_write_reg(CH390_DEVICE_T dev, uint8_t reg, uint8_t value)
{
    uint8_t buf[2];
    buf[0] = reg | OPC_REG_W;
    buf[1] = value;

    ch390_cs(dev,0); // CS = 0
    HAL_SPI_Transmit(&hspi2, buf, 2, 200);
    ch390_cs(dev,1); // CS = 1
}

void ch390_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

/**
 * @name ch390_software_reset
 * @brief Software reset CH390 by NCR
 */
void ch390_software_reset(CH390_DEVICE_T dev)
{
    ch390_cs(dev,0);
    ch390_write_reg(dev, CH390_NCR, NCR_RST);
    ch390_delay_us(10);
    ch390_write_reg(dev, CH390_NCR, 0);
    ch390_write_reg(dev, CH390_NCR, NCR_RST);
    ch390_delay_us(10);
    ch390_cs(dev,1);
}

// PHY registers
#define CH390_PHY          0x40
#define CH390_PHY_BMCR     0x00
#define CH390_PHY_BMSR     0x01
#define CH390_PHY_PHYID1   0x02
#define CH390_PHY_PHYID2   0x03
#define CH390_PHY_ANAR     0x04
#define CH390_PHY_ANLPAR   0x05
#define CH390_PHY_ANER     0x06
#define CH390_PHY_PAGE_SEL 0x1F

// Packet status
#define CH390_PKT_NONE  0x00    /* No packet received */
#define CH390_PKT_RDY   0x01    /* Packet ready to receive */
#define CH390_PKT_ERR   0xFE    /* Un-stable states */
#define CH390_PKT_MAX   1536    /* Received packet max size */
#define CH390_PKT_MIN   64

static uint8_t ch390_spi_exchange_byte(uint8_t byte)
{
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &rx, 1,200);
    return rx;
}
#define ch390_spi_dummy_read() ch390_spi_exchange_byte(0)

/**
 * @name ch390_read_reg
 * @brief Read register
 * @param reg - Target register address
 * @return Register value
 */
uint8_t ch390_read_reg(CH390_DEVICE_T dev ,uint8_t reg)
{
    uint8_t spi_data;
    ch390_cs(dev,0); // CS = 0
    ch390_spi_exchange_byte(reg | OPC_REG_R);
    spi_data = ch390_spi_dummy_read();
    ch390_cs(dev,1); // CS = 1
    return spi_data;
}

/**
 * @name ch390_get_vendor_id
 * @brief Get vendor ID
 * @return Vendor ID
 */
uint16_t ch390_get_vendor_id(CH390_DEVICE_T dev)
{
    uint16_t id;
    id = (ch390_read_reg(dev, CH390_VIDL) & 0xff);
    id |= ch390_read_reg(dev, CH390_VIDH) << 8;
    return id;
}

/**
 * @name ch390_get_product_id
 * @brief Get product ID
 * @return Product ID
 */
uint16_t ch390_get_product_id(CH390_DEVICE_T dev)
{
    uint16_t id;
    id = (ch390_read_reg(dev, CH390_PIDL) & 0xff);
    id |= ch390_read_reg(dev, CH390_PIDH) << 8;
    return id;
}

/**
 * @name ch390_get_mac
 * @brief Get mac address
 * @param mac_addr - 6-byte length mac address output
 */
void ch390_get_mac(CH390_DEVICE_T dev, uint8_t *mac_addr)
{
    uint8_t i;
    for (i = 0; i < 6; i++)
    {
        mac_addr[i] = ch390_read_reg(dev,CH390_PAR + i);
    }
    ch390_read_reg(dev, 0);
}

/**
 * @name ch390_write_phy
 * @brief Write PHY register
 * @param reg - PHY register address
 * @param value - Value to be written
 */
void ch390_write_phy(CH390_DEVICE_T dev, uint8_t reg, uint16_t value)
{
    ch390_write_reg(dev,CH390_EPAR, CH390_PHY | reg);
    ch390_write_reg(dev, CH390_EPDRL, (value & 0xff));        // Low byte
    ch390_write_reg(dev, CH390_EPDRH, ((value >> 8) & 0xff)); // High byte
    // Chose PHY, send write command
    ch390_write_reg(dev, CH390_EPCR, 0x0A);
    while(ch390_read_reg(dev, CH390_EPCR) & 0x01);
    // Clear write command
    ch390_write_reg(dev, CH390_EPCR, 0x00);
}

/**
 * @name ch390_set_phy_mode
 * @brief Set PHY mode and enable PHY.
 *        PHY mode: Auto-negotiation, 10M/100M, full-duplex/half-duplex
 * @param mode - PHY mode
 */
void ch390_set_phy_mode(CH390_DEVICE_T dev, enum ch390_phy_mode mode)
{
    uint16_t BMCR_value = 0;
    uint16_t ANAR_value = 0;
    switch (mode)
    {
    case CH390_10MFD:
        BMCR_value = 0x1100;
        ANAR_value = 0x41;
        break;
    case CH390_100MFD:
        BMCR_value = 0x3100;
        ANAR_value = 0x101;
        break;
    case CH390_AUTO:
        BMCR_value = 0x1000;
        ANAR_value = 0x01E1;
        break;
    }
    ch390_write_phy(dev, CH390_PHY_BMCR, BMCR_value);
    ch390_write_phy(dev, CH390_PHY_ANAR, ANAR_value);
    ch390_write_reg(dev, CH390_GPR, 0x00);   // Enable PHY
}

/**
 * @name ch390_set_multicast
 * @brief Set multicast address hash table
 * @param multicast_addr - 8-byte length multicast address hash table array
 */
void ch390_set_multicast(CH390_DEVICE_T dev, uint8_t *multicast_hash)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        ch390_write_reg(dev, CH390_MAR + i, multicast_hash[i]);
    }
    ch390_read_reg(dev, 0);
}


/**
 * @name ch390_default_config
 * @brief Config CH390 with default options:
 *        LED mode 1;
 *        Enable transmit check sum generation;
 *        Enable RX;
 *        Enable all interrupt and PAR
 */
void ch390_default_config(CH390_DEVICE_T dev)
{
    // CH390 has built-in MAC, this is not necessary
    // uint8_t mac_addr[6] = { 0x50, 0x54, 0x7B, 0x84, 0x00, 0x73 };
    // Multicast address hash table
    uint8_t multicase_addr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    ch390_set_phy_mode(dev, CH390_AUTO);
    // Clear status
    ch390_write_reg(dev, CH390_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
    ch390_write_reg(dev, CH390_ISR, 0xFF);         // Clear interrupt status
    ch390_write_reg(dev, CH390_TCR2, 0x80);        // LED mode 1
    ch390_write_reg(dev, CH390_TCSCR, TCSCR_ALL);  // Enable check sum generation

    // ch390_set_mac_address(mac_addr);
    ch390_set_multicast(dev, multicase_addr);

    // Enable all interrupt and PAR
    ch390_write_reg(dev, CH390_IMR, IMR_ALL);
    // Enable RX
    ch390_write_reg(dev, CH390_RCR, RCR_DIS_CRC | RCR_RXEN);
}

/**
 * @name ch390_write_mem
 * @brief Write data to TX SRAM
 * @param data - Data buffer
 * @param length - Length to write
 */
void ch390_write_mem(CH390_DEVICE_T dev, uint8_t *data, int length)
{
    int i;
    ch390_cs(dev,0);
    ch390_spi_exchange_byte(OPC_MEM_WRITE);
    for (i = 0; i < length; i++)
    {
        ch390_spi_exchange_byte(data[i]);
    }
    ch390_cs(dev,1);
}

/**
 * @name ch390_send_request
 * @brief Issue transmit request
 */
void ch390_send_request(CH390_DEVICE_T dev)
{
    uint8_t tcr = ch390_read_reg(dev, CH390_TCR);
    ch390_write_reg(dev, CH390_TCR, tcr | TCR_TXREQ);
}

/**
 * @name ch390_send_packet
 * @brief Send packet
 * @param buff - Data to be sent
 * @param length - Less than 3k bytes.
 */
void ch390_send_packet(CH390_DEVICE_T dev, uint8_t *buff, uint16_t length)
{
    // Write data to SRAM
    ch390_write_mem(dev, buff, length);
    // Wait until last transmit complete
    while(ch390_read_reg(dev,CH390_TCR) & TCR_TXREQ);
    // Set current packet length
    ch390_write_reg(dev, CH390_TXPLL, length & 0xff);
    ch390_write_reg(dev, CH390_TXPLH, (length >> 8) & 0xff);
    // Issue transmit request
    ch390_send_request(dev);
}

/**
 * @name ch390_read_mem
 * @brief Read data from RX SRAM
 * @param data - Data buffer
 * @param length - Length to read
 */
void ch390_read_mem(CH390_DEVICE_T dev, uint8_t *data, int length)
{
    int i;
    ch390_cs(dev,0);
    ch390_spi_exchange_byte(OPC_MEM_READ);

    for (i = 0; i < length; i++)
    {
        data[i] = ch390_spi_dummy_read();
    }
    ch390_cs(dev,1);
}

/**
 * @name ch390_receive_packet
 * @brief Receive packet
 * @param buff - Size equal to CH390_PKT_MAX
 * @param rx_status - Output abnormal status while receiving packet.
 *                    It has the same meaning as RSR(06h).
 * @return Packet length
 */
uint32_t ch390_receive_packet(CH390_DEVICE_T dev, uint8_t *buff, uint8_t *rx_status)
{
    uint8_t rx_ready;
    uint16_t rx_len = 0;
	uint8_t ReceiveData[4];

    // Check packet ready or not
    ch390_read_reg(dev, CH390_MRCMDX);
    rx_ready = ch390_read_reg(dev, CH390_MRCMDX);

    // if rxbyte != 1 or 0 reset device
    if (rx_ready & CH390_PKT_ERR)
    {
        // Reset RX FIFO pointer
        uint8_t rcr = ch390_read_reg(dev, CH390_RCR);
        ch390_write_reg(dev, CH390_RCR, rcr & ~RCR_RXEN); //RX disable
        ch390_write_reg(dev, CH390_MPTRCR, 0x01);  //Reset RX FIFO pointer
        ch390_write_reg(dev, CH390_MRRH, 0x0c);
        ch390_delay_us(1000);
        ch390_write_reg(dev, CH390_RCR, rcr | RCR_RXEN); //RX Enable
        return 0;
    }
    if (!(rx_ready & CH390_PKT_RDY))
    {
        return 0;
    }

    ch390_read_mem(dev, ReceiveData, 4);

    *rx_status = ReceiveData[1];
    rx_len = ReceiveData[2] | (ReceiveData[3] << 8);

    if(rx_len <= CH390_PKT_MAX)
    {
        ch390_read_mem(dev, buff, rx_len);
    }

    if ((*rx_status & 0x3f) || (rx_len > CH390_PKT_MAX))
    {
        return 0;
    }
    return rx_len;
}

