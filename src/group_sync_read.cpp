#include "st3215/group_sync_read.h"

namespace st3215 {

GroupSyncRead::GroupSyncRead(ProtocolPacketHandler* ph, uint8_t start_address, uint8_t data_length)
    : ph_(ph), start_address_(start_address), data_length_(data_length),
      last_result_(false), is_param_changed_(false) {
    clearParam();
}

void GroupSyncRead::makeParam() {
    if (data_dict_.empty()) {
        return;
    }

    param_.clear();

    for (const auto& [sts_id, data] : data_dict_) {
        param_.push_back(sts_id);
    }
}

bool GroupSyncRead::addParam(uint8_t sts_id) {
    if (data_dict_.find(sts_id) != data_dict_.end()) {
        return false;
    }

    data_dict_[sts_id] = {};
    is_param_changed_ = true;
    return true;
}

void GroupSyncRead::removeParam(uint8_t sts_id) {
    if (data_dict_.find(sts_id) == data_dict_.end()) {
        return;
    }

    data_dict_.erase(sts_id);
    is_param_changed_ = true;
}

void GroupSyncRead::clearParam() {
    data_dict_.clear();
}

int GroupSyncRead::txPacket() {
    if (data_dict_.empty()) {
        return COMM_NOT_AVAILABLE;
    }

    if (is_param_changed_ || param_.empty()) {
        makeParam();
    }

    return ph_->syncReadTx(start_address_, data_length_, param_, data_dict_.size());
}

int GroupSyncRead::rxPacket() {
    last_result_ = true;
    int result = COMM_RX_FAIL;

    if (data_dict_.empty()) {
        return COMM_NOT_AVAILABLE;
    }

    auto [rx_result, rxpacket] = ph_->syncReadRx(data_length_, data_dict_.size());
    result = rx_result;

    if (rxpacket.size() >= static_cast<size_t>((data_length_ + 6))) {
        for (auto& [sts_id, stored_data] : data_dict_) {
            auto [data, read_result] = readRx(rxpacket, sts_id, data_length_);
            stored_data = data;
            if (read_result != COMM_SUCCESS) {
                last_result_ = false;
            }
        }
    } else {
        last_result_ = false;
    }

    return result;
}

int GroupSyncRead::txRxPacket() {
    int result = txPacket();
    if (result != COMM_SUCCESS) {
        return result;
    }
    return rxPacket();
}

std::tuple<std::vector<uint8_t>, int> GroupSyncRead::readRx(const std::vector<uint8_t>& rxpacket, uint8_t sts_id, uint8_t data_length) {
    std::vector<uint8_t> data;
    size_t rx_length = rxpacket.size();
    size_t rx_index = 0;

    while ((rx_index + 6 + data_length) <= rx_length) {
        uint8_t headpacket[3] = {0, 0, 0};
        while (rx_index < rx_length) {
            headpacket[2] = headpacket[1];
            headpacket[1] = headpacket[0];
            headpacket[0] = rxpacket[rx_index];
            rx_index++;
            if (headpacket[2] == 0xFF && headpacket[1] == 0xFF && headpacket[0] == sts_id) {
                break;
            }
        }

        if ((rx_index + 3 + data_length) > rx_length) {
            break;
        }

        if (rxpacket[rx_index] != static_cast<uint8_t>(data_length + 2)) {
            rx_index++;
            continue;
        }
        rx_index++;

        uint8_t error_byte = rxpacket[rx_index];
        rx_index++;

        uint8_t calSum = sts_id + (data_length + 2) + error_byte;
        data.clear();
        data.push_back(error_byte);
        for (uint8_t i = 0; i < data_length; ++i) {
            data.push_back(rxpacket[rx_index]);
            calSum += rxpacket[rx_index];
            rx_index++;
        }
        calSum = ~calSum & 0xFF;

        if (calSum != rxpacket[rx_index]) {
            return std::make_tuple(std::vector<uint8_t>(), COMM_RX_CORRUPT);
        }
        return std::make_tuple(data, COMM_SUCCESS);
    }

    return std::make_tuple(std::vector<uint8_t>(), COMM_RX_CORRUPT);
}

std::tuple<bool, uint8_t> GroupSyncRead::isAvailable(uint8_t sts_id, uint8_t address, uint8_t data_length) {
    if (data_dict_.find(sts_id) == data_dict_.end()) {
        return std::make_tuple(false, 0);
    }

    if (address < start_address_ || (start_address_ + data_length_ - data_length) < address) {
        return std::make_tuple(false, 0);
    }

    const auto& stored_data = data_dict_[sts_id];
    if (stored_data.empty()) {
        return std::make_tuple(false, 0);
    }

    if (stored_data.size() < static_cast<size_t>(data_length + 1)) {
        return std::make_tuple(false, 0);
    }

    return std::make_tuple(true, stored_data[0]);
}

uint32_t GroupSyncRead::getData(uint8_t sts_id, uint8_t address, uint8_t data_length) {
    const auto& stored_data = data_dict_[sts_id];
    uint8_t offset = address - start_address_ + 1;

    if (data_length == 1) {
        return stored_data[offset];
    } else if (data_length == 2) {
        return ph_->makeWord(stored_data[offset], stored_data[offset + 1]);
    } else if (data_length == 4) {
        return ph_->makeDWord(
            ph_->makeWord(stored_data[offset], stored_data[offset + 1]),
            ph_->makeWord(stored_data[offset + 2], stored_data[offset + 3]));
    }
    return 0;
}

}  // namespace st3215
