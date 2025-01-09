/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SRC_SUPLA_ELEMENT_H_
#define SRC_SUPLA_ELEMENT_H_

#include <supla/protocol/supla_srpc.h>

#include "channel.h"

class SuplaDeviceClass;

namespace Supla {

/**
 * Base class for all elements of SuplaDevice
 */
class Element {
 public:
  Element();
  virtual ~Element();
  /**
   * Returns first Element (based on creation order)
   *
   * @return first Element
   */
  static Element *begin();

  /**
   * Returns last Element (based on creation order)
   *
   * @return last Element
   */
  static Element *last();

  /**
   * Returns Element by channel number
   *
   * @param channelNumber of existing channel
   *
   * @return pointer to Element which handles this channel
   */
  static Element *getElementByChannelNumber(int channelNumber);

  /**
   * Returns Element which owns given subDeviceId
   *
   * @param subDeviceId id of subdevice
   *
   * @return pointer to Element which owns given subDeviceId
   */
  static Element *getOwnerOfSubDeviceId(int subDeviceId);

  /**
   * Checks if any element has pending update for remote server.
   *
   * This method is usually used by device which supports sleeping.
   * It will call it to check if there is any more update to be send to
   * the server before disconnecting and going to sleep.
   *
   * @return true if any Element has pending update
   */
  static bool IsAnyUpdatePending();

  /**
   * Notifies all elements about device config change
   *
   * @param fieldBit bit mask of changed field (see SUPLA_DEVICE_CONFIG_FIELD_
   *        in proto.h)
   */
  static void NotifyElementsAboutConfigChange(uint64_t fieldBit);

  /**
   * When "invalid pointer" is set, previously obtained Element pointer
   * may be invalid.
   *
   * In such case, new pointer should be generated by begin() method and
   * Element's iteration should be started again.
   *
   * @return true if "invalid pointer" is set
   */
  static bool IsInvalidPtrSet();

  /**
   * Clears "invalid pointer" flag
   */
  static void ClearInvalidPtr();

  /**
   * Returns next Element from the list
   *
   * @return pointer to the next Element
   */
  Element *next();

  /**
   * First method called on element in SuplaDevice.begin().
   *
   * Called only if Config Storage class is configured.
   * Element should read its configration in this method.
   *
   * @param sdc
   */
  virtual void onLoadConfig(SuplaDeviceClass *sdc);

  /**
   * Removes all configration data related to the element from Storage::Config
   */
  virtual void purgeConfig();

  /**
   * Second method called on element in SuplaDevice.begin().
   *
   * Method called during state Storage initialization (i.e. read from EEPROM,
   * FRAM). Called only if Storage class is configured.
   */
  virtual void onLoadState();

  /**
   * Third method called on element in SuplaDevice.begin()
   *
   * Method called during SuplaDevice initialization. I.e. load initial state,
   * initialize pins etc.
   */
  virtual void onInit();

  /**
   * Method called periodically during SuplaDevice iteration
   *
   * It should provide state saving for this elemnet to Storage
   */
  virtual void onSaveState();

  /**
   * Method called each time when device successfully registers to Supla server
   *
   * @param suplaSrpc
   */
  virtual void onRegistered(Supla::Protocol::SuplaSrpc *suplaSrpc = nullptr);

  /**
   * Method called on each SuplaDevice iteration.
   *
   * It should provide all actions that have to be executed periodically
   * regardless of network state.
   */
  virtual void iterateAlways();

  /**
   * Method called on each SuplaDevice iteration when device is connected and
   * registered to Supla server or other protocol.
   *
   * @return false when element didn't communicate with server and next element
   *         should be iterated instead.
   *         true otherwise.
   */
  virtual bool iterateConnected();
  /// deprecated
  virtual bool iterateConnected(void *ptr);

  /**
   * Method called on timer interupt.
   *
   * It should provide all actions that have to be executed periodically
   * regardless of other SuplaDevice activities. It is usually called every
   * 10 ms.
   */
  virtual void onTimer();

  /**
   * Method called on fast timer interupt.
   *
   * It should provide all actions that have to be executed periodically
   * regardless of other SuplaDevice activities. It is usually called every
   * 1 ms (or 0.5 ms for Arduino Mega).
   */
  virtual void onFastTimer();

  /**
   * Method called when device soft restart is triggered.
   */
  virtual void onSoftReset();

  /**
   * Method called when device config is changed.
   *
   * @param fieldBit bit mask of changed field
   */
  virtual void onDeviceConfigChange(uint64_t fieldBit);

  /**
   * Handles "new value" request from server.
   *
   * @param newValue pointer to TSD_SuplaChannelNewValue
   *
   * @return -1 - don't send reply to server
   *         0 - success==false
   *         1 - success==true
   */
  virtual int32_t handleNewValueFromServer(TSD_SuplaChannelNewValue *newValue);

  /**
   * Fills TSD_SuplaChannelNewValue based on current state.
   *
   * @param value pointer to TSD_SuplaChannelNewValue which should be filled
   */
  virtual void fillSuplaChannelNewValue(TSD_SuplaChannelNewValue *value);

  // Handles "get channel state" request from server
  // channelState is prefilled with network and device status informations
  /**
   * Handles "get channel state (i)" request from server.
   *
   * @param channelState pointer to TDSC_ChannelState which should be filled.
   *                     channelState is prefilled with network and device info.
   */
  virtual void handleGetChannelState(TDSC_ChannelState *channelState);

  /**
   * Handles CALCFG requests from server
   *
   * @param request
   *
   * @return SUPLA_CALCFG_RESULT_* (see proto.h)
   */
  virtual int handleCalcfgFromServer(TSD_DeviceCalCfgRequest *request);

  // Returns SUPLA_RESULTCODE_
  /**
   * Handles "set channel config" request from server
   *
   * @param newChannelConfig pointer to new channel configuration
   * @param local true if request is local (issued by device itself), false
   *              when requests originates from server.
   *
   * @return SUPLA_RESULTCODE_* (see proto.h)
   */
  virtual uint8_t handleChannelConfig(TSD_ChannelConfig *newChannelConfig,
                                      bool local = false);

  /**
   * Handles "set channel config" with "weekly schedule" type request from
   * server.
   *
   * @param newWeeklySchedule pointer to new weekly schedule
   * @param altSchedule true if request is for alternative schedule (for
   * cooling)
   * @param local true if request is local (issued by device itself), false
   *              when requests originates from server.
   *
   * @return SUPLA_RESULTCODE_* (see proto.h)
   */
  virtual uint8_t handleWeeklySchedule(TSD_ChannelConfig *newWeeklySchedule,
                                       bool altSchedule = false,
                                       bool local = false);

  // handleSetChannelConfigResult should handle both standard channel config
  // and weekly schedule config
  /**
   * Handles "set channel config" reply from server.
   *
   * All types of channel config should be handled here (including weekly
   * schedule, default config, etc).
   *
   * @param result
   */
  virtual void handleSetChannelConfigResult(
      TSDS_SetChannelConfigResult *result);

  /**
   * Handles "channel config finished" message from server.
   *
   * This message informs device that server finished sending channel config
   * for this element.
   */
  virtual void handleChannelConfigFinished();

  /**
   * Returns channel number
   *
   * @return channel number related to main channel in element. Value -1 is
   * returned if element has no channel
   */
  int getChannelNumber() const;

  /**
   * Returns secondary channel number
   *
   * @return channel number related to secondary channel in element. Value -1 is
   * returned if element has no secondary channel
   */
  int getSecondaryChannelNumber() const;

  /**
   * Returns true if element is owner of subDeviceId
   *
   * @param subDeviceId
   *
   * @return true if element is owner of subDeviceId
   */
  virtual bool isOwnerOfSubDeviceId(int subDeviceId) const;

  /**
   * Returns pointer to main channel
   *
   * @return pointer to main channel, nullptr if element has no channel
   */
  virtual const Channel *getChannel() const;
  virtual Channel *getChannel();

  /**
   * Returns pointer to secondary channel
   *
   * @return pointer to secondary channel, nullptr if element has no
   * secondary channel
   */
  virtual const Channel *getSecondaryChannel() const;
  virtual Channel *getSecondaryChannel();

  /**
   * Generates key used for Config
   *
   * Adds "x_" prefix to the key where x is a channel number. Output is
   * written to output buffer and limitted to SUPLA_CONFIG_MAX_KEY_SIZE.
   *
   * @param output output buffer (should be at least SUPLA_CONFIG_MAX_KEY_SIZE)
   * @param key key
   */
  virtual void generateKey(char *output, const char *key) const;

  /**
   * Disables channel state (i) function
   *
   * @return reference to self
   */
  Element &disableChannelState();

  /**
   * Returns true if channel state (i) function is enabled
   *
   * @return true if channel state (i) function is enabled
   */
  bool isChannelStateEnabled() const;

  /**
   * Returns true if any update is pending
   *
   * @return true if any update is pending
   */
  virtual bool isAnyUpdatePending();

  /**
   * Sets initial caption.
   *
   * Initial caption is used as channel's caption when it is registered on
   * server for the first time.
   *
   * @param caption caption
   * @param secondaryChannel true if caption is for secondary channel, false
   *                         otherwise
   */
  void setInitialCaption(const char *caption, bool secondaryChannel = false);

  /**
   * Sets default channel's function
   *
   * @param defaultFunction SUPLA_CHANNELFNC_* see proto.h
   */
  void setDefaultFunction(int32_t defaultFunction);

 protected:
  static Element *firstPtr;
  static bool invalidatePtr;
  Element *nextPtr = nullptr;
};

};  // namespace Supla

#endif  // SRC_SUPLA_ELEMENT_H_
