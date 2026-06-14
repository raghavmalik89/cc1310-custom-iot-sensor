#ifndef LIS2DW12_PORT_TI_H_
#define LIS2DW12_PORT_TI_H_

#include <stdint.h>

#include "third_party/st/lis2dw12_reg.h"

int32_t lis2dw12_ti_read_reg(void *handle,
                             uint8_t reg,
                             uint8_t *bufp,
                             uint16_t len);
int32_t lis2dw12_ti_write_reg(void *handle,
                              uint8_t reg,
                              const uint8_t *bufp,
                              uint16_t len);
void lis2dw12_ti_ctx_init(stmdev_ctx_t *ctx, uint8_t i2c_addr);

#endif /* LIS2DW12_PORT_TI_H_ */
