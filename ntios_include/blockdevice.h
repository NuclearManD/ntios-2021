//
// Created by nuclaer on 12/13/21.
//

#ifndef NTIOS_2021_BLOCKDEVICE_H
#define NTIOS_2021_BLOCKDEVICE_H

#include "drivers.h"

#include <stdint.h>
#include <stddef.h>


/**
 * \class FsBlockDeviceInterface
 * \brief FsBlockDeviceInterface class.
 */
class BlockDevice: public Device {
public:
	virtual ~BlockDevice() {}

	/** start the device, check if operational */
	virtual bool begin() = 0;

	/** end use of device */
	virtual void end() {}

	/**
	 * Check for FsBlockDevice busy.
	 *
	 * \return true if busy else false.
	 */
	virtual bool isBusy() = 0;
	/**
	 * Read a sector.
	 *
	 * \param[in] sector Logical sector to be read.
	 * \param[out] dst Pointer to the location that will receive the data.
	 * \return true for success or false for failure.
	 */
	virtual bool readSector(uint32_t sector, uint8_t* dst) = 0;

	/**
	 * Read multiple sectors.
	 *
	 * \param[in] sector Logical sector to be read.
	 * \param[in] ns Number of sectors to be read.
	 * \param[out] dst Pointer to the location that will receive the data.
	 * \return true for success or false for failure.
	 */
	virtual bool readSectors(uint32_t sector, uint8_t* dst, size_t ns) = 0;

	/** \return device size in sectors. */
	virtual uint32_t sectorCount() = 0;

	/** End multi-sector transfer and go to idle state.
	 * \return true for success or false for failure.
	 */
	virtual bool syncDevice() = 0;

	/**
	 * Writes a sector.
	 *
	 * \param[in] sector Logical sector to be written.
	 * \param[in] src Pointer to the location of the data to be written.
	 * \return true for success or false for failure.
	 */
	virtual bool writeSector(uint32_t sector, const uint8_t* src) = 0;

	/**
	 * Write multiple sectors.
	 *
	 * \param[in] sector Logical sector to be written.
	 * \param[in] ns Number of sectors to be written.
	 * \param[in] src Pointer to the location of the data to be written.
	 * \return true for success or false for failure.
	 */
	virtual bool writeSectors(uint32_t sector, const uint8_t* src, size_t ns) = 0;

	int getType() { return DEV_TYPE_BLOCK_DEVICE; }
};

class DeviceSdCard: public BlockDevice {
public:
	/** CMD6 Switch mode: Check Function Set Function.
	 * \param[in] arg CMD6 argument.
	 * \param[out] status return status data.
	 *
	 * \return true for success or false for failure.
	 */
	virtual bool cardCMD6(uint32_t arg, uint8_t* status) = 0;
	/** end use of card */
	virtual void end() = 0;
	/** Erase a range of sectors.
	*
	* \param[in] firstSector The address of the first sector in the range.
	* \param[in] lastSector The address of the last sector in the range.
	*
	* \return true for success or false for failure.
	*/
	virtual bool erase(uint32_t firstSector, uint32_t lastSector) = 0;
	/** \return error code. */
	virtual uint8_t errorCode() const = 0;
	/** \return error data. */
	virtual uint32_t errorData() const = 0;
	/** \return true if card is busy. */
	virtual bool isBusy() = 0;
	/** \return false by default */
	virtual bool hasDedicatedSpi() {return false;}
	/** \return false by default */
	bool virtual isDedicatedSpi() {return false;}
	/** Set SPI sharing state
	 * \param[in] value desired state.
	 * \return false by default.
	 */
	virtual bool setDedicatedSpi(bool value) {
		(void)value;
		return false;
	}
	/**
	 * Read a card's CID register.
	 *
	 * \param[out] cid pointer to area for returned data.
	 *
	 * \return true for success or false for failure.
	 */
	//virtual bool readCID(cid_t* cid) = 0;
	/**
	* Read a card's CSD register.
	*
	* \param[out] csd pointer to area for returned data.
	*
	* \return true for success or false for failure.
	*/
	//virtual bool readCSD(csd_t* csd) = 0;
	/** Read OCR register.
	 *
	 * \param[out] ocr Value of OCR register.
	 * \return true for success or false for failure.
	 */
	//virtual bool readOCR(uint32_t* ocr) = 0;
	/** Read SCR register.
	 *
	 * \param[out] scr Value of SCR register.
	 * \return true for success or false for failure.
	 */
	//virtual bool readSCR(scr_t *scr) = 0;
	/**
	 * Determine the size of an SD flash memory card.
	 *
	 * \return The number of 512 byte data sectors in the card
	 *         or zero if an error occurs.
	 */
	virtual uint32_t sectorCount() = 0;
	/** \return card status. */
	virtual uint32_t status() {return 0XFFFFFFFF;}
	/** Return the card type: SD V1, SD V2 or SDHC/SDXC
	 * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC/SDXC.
	 */
	virtual uint8_t type() const = 0;
	/** Write one data sector in a multiple sector write sequence.
	 * \param[in] src Pointer to the location of the data to be written.
	 * \return true for success or false for failure.
	 */

	virtual bool writeData(const uint8_t* src) = 0;
	/** Start a write multiple sectors sequence.
	 *
	 * \param[in] sector Address of first sector in sequence.
	 *
	 * \return true for success or false for failure.
	 */
	virtual bool writeStart(uint32_t sector) = 0;
	/** End a write multiple sectors sequence.
	 * \return true for success or false for failure.
	 */
	virtual bool writeStop() = 0;

	int getType() { return DEV_TYPE_SD_CARD; }
};

#endif //NTIOS_2021_BLOCKDEVICE_H
