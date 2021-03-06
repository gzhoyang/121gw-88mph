/*****************************************************************************
 *  Copyright 2018 Thomas Watson                                             *
 *                                                                           *
 *  This file is a part of 88mph: https://github.com/tpwrules/121gw-88mph/   *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *****************************************************************************/

// this file handles the acquisition engine

#include <stdint.h>
#include <stdbool.h>
#include "stm32l1xx.h"

#include "acquisition/acquisition.h"

#include "acquisition/acq_modes.h"
#include "system/job.h"
#include "hardware/hy3131.h"
#include "hardware/gpio.h"

static acq_mode_func curr_acq_mode_func = 0;
static volatile uint8_t curr_int_mask = 0;

// turn on the acquisition engine
void acq_init(void) {
    // power up the digital supply for the measurement
    GPIO_PINSET(HW_PWR_CTL);
    // turn on the 4V analog supply
    GPIO_PINSET(HW_PWR_CTL2);
    // give the HY3131 a bit of time to power up
    // who knows if this is necessary, but it feels good
    HAL_Delay(10);
    // initialize it
    curr_int_mask = 0;
    hy_init();
    // switch to the 'off' mode manually
    // cause there should be no previous mode func to call
    curr_acq_mode_func = acq_mode_funcs[ACQ_MODE_MISC];
    curr_acq_mode_func(ACQ_EVENT_START, (int64_t)ACQ_MODE_MISC_SUBMODE_OFF);
    // the off mode tells the HY to not send us interrupts
}

// turn off the acquisition engine
void acq_deinit(void) {
    // stop the current acquisition by switching to 'off'
    acq_set_mode(ACQ_MODE_MISC, ACQ_MODE_MISC_SUBMODE_OFF);
    // and cancel out the acq function
    curr_acq_mode_func = 0;
    // stop the HY3131
    hy_deinit();
    // turn off analog supply
    GPIO_PINRST(HW_PWR_CTL2);
    // and then digital supply
    GPIO_PINRST(HW_PWR_CTL);
}

// do the acquisition job
// check the HY3131 and calculate new acquisitions
void acq_handle_job_acquisition(void) {
    uint8_t regbuf[5];

    // read which interrupts are pending
    // this also clears the pending interrupts
    uint8_t which_ints;
    hy_read_regs(HY_REG_INTF, 1, &which_ints);
    // only handle pending interrupts which are enabled
    which_ints &= curr_int_mask;
    if (which_ints & HY_REG_INT_AD1) {
        // read the 24 bit AD1 register
        hy_read_regs(HY_REG_AD1_DATA, 3, regbuf);
        int32_t val = regbuf[2] << 16 | regbuf[1] << 8 | regbuf[0];
        // sign extend 24 bits to 32
        if (val & 0x800000) {
            val |= 0xFF000000;
        }
        // tell the current acquisition mode about it
        curr_acq_mode_func(ACQ_EVENT_NEW_AD1, val);
    }
}

void acq_set_int_mask(uint8_t mask) {
    // disable the job around this so curr_int_mask isn't wrong
    bool acq_enabled = job_disable(JOB_ACQUISITION);
    curr_int_mask = mask;
    // the caller probably is changing the int mask because they've reconfigured
    // the chip, so clear pending interrupts from the chip first
    if (mask) {
        uint8_t whatever;
        hy_read_regs(HY_REG_INTF, 1, &whatever);
    }
    // now enable the interrupts the user wanted
    hy_write_regs(HY_REG_INTE, 1, &mask);
    // and turn on the job again so they get handled
    job_resume(JOB_ACQUISITION, acq_enabled);
}

void acq_set_mode(acq_mode_t mode, acq_submode_t submode) {
    // the acq job might try to interrupt us during this process, so pause it
    bool acq_enabled = job_disable(JOB_ACQUISITION);
    // turn off the current mode
    curr_acq_mode_func(ACQ_EVENT_STOP, 0);
    // figure out which mode func goes with this mode
    curr_acq_mode_func = acq_mode_funcs[mode];
    // and start it up
    curr_acq_mode_func(ACQ_EVENT_START, (int64_t)submode);
    job_resume(JOB_ACQUISITION, acq_enabled);
}

void acq_set_submode(acq_submode_t submode) {
    // the acq job might try to interrupt us during this process, so pause it
    bool acq_enabled = job_disable(JOB_ACQUISITION);
    curr_acq_mode_func(ACQ_EVENT_SET_SUBMODE, (int64_t)submode);
    job_resume(JOB_ACQUISITION, acq_enabled);
}

// must be power of 2!!
#define ACQ_READING_QUEUE_SIZE (8)

#define Q_MASK (ACQ_READING_QUEUE_SIZE-1)

static volatile reading_t queue[ACQ_READING_QUEUE_SIZE];
static volatile int q_head = 0;
static volatile int q_tail = 0;

// put a reading into the queue. if there is no space it's just dropped
void acq_put_reading(reading_t* reading) {
    // can be called from any job, so protect ourselves!
    __disable_irq();
    // buffer head cause it's volatile
    // but we know we can't get interrupted
    int q_h = q_head;
    if (((q_h+1)&Q_MASK) != q_tail) {
        // we have space
        queue[q_h] = *reading;
        q_head = (q_h+1) & Q_MASK;
    }
    __enable_irq();

    // the measurement engine is certainly interested in this new reading
    job_schedule(JOB_MEASUREMENT);
}

// get a reading from the queue. returns false if there is no reading to get.
// else puts the reading into reading and returns true
bool acq_get_reading(reading_t* reading) {
    // can be called from any job, so protect ourselves!
    __disable_irq();
    // buffer tail cause it's volatile
    // but we know we can't get interrupted
    int q_t = q_tail;
    bool there_is_a_reading = (q_head != q_t);
    if (there_is_a_reading) {
        *reading = queue[q_t];
        q_tail = (q_t+1) & Q_MASK;
    }
    __enable_irq();
    return there_is_a_reading;
}

// empty the queue of all readings
void acq_clear_readings(void) {
    // can be called from any job, so protect ourselves!
    __disable_irq();
    q_head = 0;
    q_tail = 0;
    __enable_irq();
}

// misc mode handler
void acq_mode_func_misc(acq_event_t event, int64_t value) {
    // for now, all this mode should be doing is turning off
    acq_set_int_mask(0);
    acq_clear_readings();
}