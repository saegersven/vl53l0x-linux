/**
 * Most of the functionality of this library is based on the VL53L0X API provided by ST (STSW-IMG005)
 * and some of the explanatory comments are quoted or paraphrased from the API source code, API user manual (UM2039), and the VL53L0X datasheet.
 **/

#ifndef _VL53L0X_H
#define _VL53L0X_H

#include <cstdint>
#include <fstream>
#include <mutex>
#include <functional>

#include "VL53L0X_defines.hpp"

class VL53L0X {
	public:

		//////// MODIFIED SECTION

		/*** I2C wrapper methods ***/
		std::function<int8_t(uint8_t, uint8_t uint8_t*)> i2c_readByte;
		std::function<int8_t(uint8_t, uint8_t uint8_t, uint8_t*)> i2c_readBytes;
		std::function<bool(uint8_t, uint8_t, uint8_t)> i2c_writeByte;
		std::function<bool(uint8_t, uint8_t, uint8_t, uint8_t*)> i2c_writeBytes;
		std::function<bool(uint8_t, uint8_t, uint16_t)> i2c_writeWord;

		//// END MODIFIED SECTION

		
		/*** Constructors and destructors ***/

		/**
		 * \brief Initialize sensor using sequence Based on VL53L0X_DataInit(), VL53L0X_StaticInit(), and VL53L0X_PerformRefCalibration().
		 *
		 * \param xshutGPIOPin - host's GPIO pin used to toggle sensor on and off. Defaults to -1 (unused).
		 * \param ioMode2v8 - whether to configure the sensor for 2V8 mode (2.8V logic instead of 1.8V). Defaults to true.
		 * \param address - I2C bus address of the sensor. Defaults to sensor's default address, change only if sensor was initialized to another address beforehand.
		 *
		 * This function does not perform reference SPAD calibration (VL53L0X_PerformRefSpadManagement()),
		 * since the API user manual says that it is performed by ST on the bare modules;
		 * It seems like that should work well enough unless a cover glass is added.
		 */
		VL53L0X(const int16_t xshutGPIOPin = -1, bool ioMode2v8 = true, const uint8_t address = VL53L0X_ADDRESS_DEFAULT);

		/*** Public methods ***/
		/**
		 * \brief Initialize the sensor's hardware and, if needed, GPIO access on the host side.
		 *
		 * It's not part of the constructor as it can throw errors.
		 */
		void initialize();
		/**
		 * Power on the sensor by setting its XSHUT pin to high via host's GPIO.
		 */
		void powerOn();
		/**
		 * Power off the sensor by setting its XSHUT pin to low via host's GPIO.
		 */
		void powerOff();
		/**
		 * Change sensor's I2C address (sets both the address on the physical sensor and within sensor's object).
		 */
		void setAddress(uint8_t newAddress);
		/**
		 * Get sensor's I2C address as last set.
		 */
		inline uint8_t getAddress() {
			return this->address;
		}
		/**
		 * Set the return signal rate limit check value in units of MCPS (mega counts per second).
		 *
		 * "This represents the amplitude of the signal reflected from the target and detected by the device";
		 * setting this limit presumably determines the minimum measurement necessary for the sensor to report a valid reading.
		 * Setting a lower limit increases the potential range of the sensor but also seems to increase the likelihood
		 * of getting an inaccurate reading because of unwanted reflections from objects other than the intended target.
		 * Defaults to 0.25 MCPS as initialized by the ST API and this library.
		 */
		bool setSignalRateLimit(float limitMCPS);
		/**
		 * Get the return signal rate limit check value in MCPS
		 */
		float getSignalRateLimit();
		/**
		 * Set the measurement timing budget (in microseconds), which is the time allowed for one measurement.
		 *
		 * The ST API and this library take care of splitting the timing budget among the sub-steps in the ranging sequence.
		 * A longer timing budget allows for more accurate measurements.
		 * Increasing the budget by a factor of N decreases the range measurement standard deviation by a factor of sqrt(N).
		 * Defaults to about 33 milliseconds; the minimum is 20 ms.
		 * Based on VL53L0X_set_measurement_timing_budget_micro_seconds().
		 */
		bool setMeasurementTimingBudget(uint32_t budgetMicroseconds);
		/**
		 * Get the measurement timing budget (in microseconds)
		 *
		 * Based on VL53L0X_get_measurement_timing_budget_micro_seconds()
		 */
		uint32_t getMeasurementTimingBudget();
		/**
		 * Set the VCSEL (vertical cavity surface emitting laser) pulse period for the given period type (pre-range or final range) to the given value (in PCLKs).
		 *
		 * Longer periods seem to increase the potential range of the sensor.
		 * Valid values are (even numbers only): pre: 12 to 18 (initialized default: 14), final: 8 to 14 (initialized default: 10).
		 * Based on VL53L0X_set_vcsel_pulse_period().
		 */
		bool setVcselPulsePeriod(vl53l0xVcselPeriodType type, uint8_t periodPCLKs);
		/**
		 * Get the VCSEL pulse period in PCLKs for the given period type.
		 *
		 * Based on VL53L0X_get_vcsel_pulse_period().
		 */
		uint8_t getVcselPulsePeriod(vl53l0xVcselPeriodType type);
		/**
		 * Start continuous ranging measurements.
		 *
		 * If periodMilliseconds (optional) is 0 or not given, continuous back-to-back mode is used (the sensor takes measurements as often as possible);
		 * Otherwise, continuous timed mode is used, with the given inter-measurement period in milliseconds determining how often the sensor takes a measurement.
		 * Based on VL53L0X_StartMeasurement().
		 */
		void startContinuous(uint32_t periodMilliseconds = 0);
		/**
		 * Stop continuous measurements.
		 *
		 * Based on VL53L0X_StopMeasurement().
		 */
		void stopContinuous();
		/**
		 * Returns a range reading in millimeters when continuous mode is active.
		 * Warning: Blocking call!
		 *
		 * readRangeSingleMillimeters() also calls this function after starting a single-shot range measurement.
		 */
		uint16_t readRangeContinuousMillimeters();
		/**
		 * Performs a single-shot range measurement and returns the reading in millimeters.
		 * Warning: Blocking call!
		 *
		 * Based on VL53L0X_PerformSingleRangingMeasurement().
		 */
		uint16_t readRangeSingleMillimeters();
		/**
		 * Set value of timeout for measurements.
		 * 0 (dafault value) means no time limit for measurements (infinite wait).
		 */
		inline void setTimeout(uint16_t timeout) {
			this->ioTimeout = timeout;
		}
		/**
		 * Get value of timeout for measurements as last set.
		 */
		inline uint16_t getTimeout() {
			return this->ioTimeout;
		}
		/**
		 * Whether a timeout occurred in one of the read functions since the last call to timeoutOccurred().
		 */
		bool timeoutOccurred();
	private:
		/*** Private fields ***/

		uint8_t address;
		int16_t xshutGPIOPin;
		bool ioMode2v8;
		std::string gpioFilename;
		std::mutex fileAccessMutex;
		bool gpioInitialized;

		uint32_t measurementTimingBudgetMicroseconds;
		uint64_t timeoutStartMilliseconds;
		uint64_t ioTimeout;
		bool didTimeout;
		// read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
		uint8_t stopVariable;

		/*** Private methods ***/

		void initHardware();
		void initGPIO();
		/**
		 * Get reference SPAD (single photon avalanche diode) count and type.
		 *
		 * Based on VL53L0X_get_info_from_device(), but only gets reference SPAD count and type.
		 */
		bool getSPADInfo(uint8_t* count, bool* typeIsAperture);
		/**
		 * Get sequence step enables.
		 *
		 * Based on VL53L0X_GetSequenceStepEnables().
		 */
		void getSequenceStepEnables(VL53L0XSequenceStepEnables* enables);
		/**
		 * Get sequence step timeouts.
		 *
		 * Based on get_sequence_step_timeout(), but gets all timeouts instead of just the requested one, and also stores intermediate values.
		 */
		void getSequenceStepTimeouts(const VL53L0XSequenceStepEnables* enables, VL53L0XSequenceStepTimeouts* timeouts);
		/**
		 * Decode sequence step timeout in MCLKs from register value.
		 *
		 * Based on VL53L0X_decode_timeout().
		 * Note: the original function returned a uint32_t, but the return value is always stored in a uint16_t.
		 */
		static uint16_t decodeTimeout(uint16_t registerValue);
		/**
		 * Encode sequence step timeout register value from timeout in MCLKs.
		 *
		 * Based on VL53L0X_encode_timeout().
		 * Note: the original function took a uint16_t, but the argument passed to it is always a uint16_t.
		 */
		static uint16_t encodeTimeout(uint16_t timeoutMCLKs);
		/**
		 * Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs.
		 *
		 * Based on VL53L0X_calc_timeout_us().
		 */
		static uint32_t timeoutMclksToMicroseconds(uint16_t timeoutPeriodMCLKs, uint8_t vcselPeriodPCLKs);
		/**
		 * Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs.
		 *
		 * Based on VL53L0X_calc_timeout_mclks().
		 */
		static uint32_t timeoutMicrosecondsToMclks(uint32_t timeoutPeriodMicroseconds, uint8_t vcselPeriodPCLKs);
		/**
		 * Based on VL53L0X_perform_single_ref_calibration().
		 */
		bool performSingleRefCalibration(uint8_t vhvInitByte);

		/**
		 * Write an 8-bit register.
		 */
		void writeRegister(uint8_t register, uint8_t value);
		/**
		 * Write a 16-bit register.
		 */
		void writeRegister16Bit(uint8_t register, uint16_t value);
		/**
		 * Write a 32-bit register.
		 *
		 * Based on VL53L0X_write_dword from VL53L0X kernel driver.
		 */
		void writeRegister32Bit(uint8_t register, uint32_t value);
		/**
		 * Write an arbitrary number of bytes from the given array to the sensor, starting at the given register.
		 */
		void writeRegisterMultiple(uint8_t register, const uint8_t* source, uint8_t count);
		/**
		 * Read an 8-bit register.
		 */
		uint8_t readRegister(uint8_t register);
		/**
		 * Read a 16-bit register.
		 */
		uint16_t readRegister16Bit(uint8_t register);
		/**
		 * Read a 32-bit register.
		 */
		uint32_t readRegister32Bit(uint8_t register);
		/**
		 * Read an arbitrary number of bytes from the sensor, starting at the given register, into the given array.
		 */
		void readRegisterMultiple(uint8_t register, uint8_t* destination, uint8_t count);

};

#endif
