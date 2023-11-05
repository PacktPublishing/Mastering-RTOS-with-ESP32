/**
 * @file peripheral_gpio.h
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-10-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __PERIPHERAL_GPIO_H__
#define __PERIPHERAL_GPIO_H__

void peripheral_gpio_setup(void);
void peripheral_gpio_toggle(void);
void peripheral_gpio_loop(void);

void peripheral_gpio_off(void);
void peripheral_gpio_on(void);

#endif // __PERIPHERAL_GPIO_H__